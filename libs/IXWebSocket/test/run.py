#!/usr/bin/env python2.7

from __future__ import print_function

import os
import sys
import platform
import argparse
import multiprocessing
import tempfile
import time
import datetime
import threading
import subprocess
import re
import xml.etree.ElementTree as ET
from xml.dom import minidom

hasClick = True
try:
    import click
except ImportError:
    hasClick = False

BUILD_TYPE = 'Debug'
XML_OUTPUT_FILE = 'ixwebsocket_unittest.xml'
TEST_EXE_PATH = None

class Command(object):
    """Run system commands with timeout

    From http://www.bo-yang.net/2016/12/01/python-run-command-with-timeout
    Python3 might have a builtin way to do that.
    """
    def __init__(self, cmd):
        self.cmd = cmd
        self.process = None

    def run_command(self):
        self.process = subprocess.Popen(self.cmd, shell=True)
        self.process.communicate()

    def run(self, timeout=None):
        '''5 minutes default timeout'''

        if timeout is None:
            timeout = 5 * 60

        thread = threading.Thread(target=self.run_command, args=())
        thread.start()
        thread.join(timeout)

        if thread.is_alive():
            print('Command timeout, kill it: ' + self.cmd)
            self.process.terminate()
            thread.join()
            return False, 255
        else:
            return True, self.process.returncode


def runCommand(cmd, abortOnFailure=True, timeout=None):
    '''Small wrapper to run a command and make sure it succeed'''

    if timeout is None:
        timeout = 30 * 60 # 30 minute default timeout

    print('\nRunning', cmd)
    command = Command(cmd)
    succeed, ret = command.run(timeout)

    if not succeed or ret != 0:
        msg = 'cmd {}\nfailed with error code {}'.format(cmd, ret)
        print(msg)
        if abortOnFailure:
            sys.exit(-1)


def runCMake(sanitizer, buildDir):
    '''Generate a makefile from CMake.
    We do an out of dir build, so that cleaning up is easy
    (remove build sub-folder).
    '''

    sanitizersFlags = {
        'asan': '-DSANITIZE_ADDRESS=On',
        'ubsan': '-DSANITIZE_UNDEFINED=On',
        'tsan': '-DSANITIZE_THREAD=On',
        'none': ''
    }
    sanitizerFlag = sanitizersFlags.get(sanitizer, '')

    # CMake installed via Self Service ends up here.
    cmakeExecutable = '/Applications/CMake.app/Contents/bin/cmake'
    if not os.path.exists(cmakeExecutable):
        cmakeExecutable = 'cmake'

    if platform.system() == 'Windows':
        #generator = '"NMake Makefiles"'
        #generator = '"Visual Studio 16 2019"'
        generator = '"Visual Studio 15 2017"'
        USE_VENDORED_THIRD_PARTY = 'ON'
    else:
        generator = '"Unix Makefiles"'
        USE_VENDORED_THIRD_PARTY = 'ON'

    CMAKE_BUILD_TYPE = BUILD_TYPE

    fmt = '{cmakeExecutable} -H. \
    {sanitizerFlag} \
    -B"{buildDir}" \
    -DCMAKE_BUILD_TYPE={CMAKE_BUILD_TYPE} \
    -DUSE_TLS=1 \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DUSE_VENDORED_THIRD_PARTY={USE_VENDORED_THIRD_PARTY} \
    -G{generator}'

    cmakeCmd = fmt.format(**locals())
    runCommand(cmakeCmd)


def runTest(args, buildDir, xmlOutput, testRunName):
    '''Execute the unittest.
    '''
    if args is None:
        args = ''

    testCommand = '{} -o {} -n "{}" -r junit "{}"'.format(TEST_EXE_PATH, xmlOutput, testRunName, args)

    runCommand(testCommand,
               abortOnFailure=False)


def validateTestSuite(xmlOutput):
    '''
    Parse the output XML file to validate that all tests passed.

    Assume that the XML file contains only one testsuite.
    (which is true when generate by catch2)
    '''
    tree = ET.parse(xmlOutput)
    root = tree.getroot()
    testSuite = root[0]
    testSuiteAttributes = testSuite.attrib

    tests = testSuiteAttributes['tests']

    success = True

    for testcase in testSuite:
        if testcase.tag != 'testcase':
            continue

        testName = testcase.attrib['name']
        systemOutput = None

        for child in testcase:
            if child.tag == 'system-out':
                systemOutput = child.text

            if child.tag == 'failure':
                success = False

                print("Testcase '{}' failed".format(testName))
                print(' ', systemOutput)

    return success, tests


def log(msg, color):
    if hasClick:
        click.secho(msg, fg=color)
    else:
        print(msg)


def isSuccessFullRun(output):
    '''When being run from lldb, we cannot capture the exit code
    so we have to parse the output which is produced in a
    consistent way. Whenever we'll be on a recent enough version of lldb we
    won't have to do this.
    '''
    pid = None
    matchingPids = False
    exitCode = -1

    # 'Process 279 exited with status = 1 (0x00000001) ',
    exitPattern = re.compile('^Process (?P<pid>[0-9]+) exited with status = (?P<exitCode>[0-9]+)')

    # "Process 99232 launched: '/Users/bse...
    launchedPattern = re.compile('^Process (?P<pid>[0-9]+) launched: ')

    for line in output:
        match = exitPattern.match(line)
        if match:
            exitCode = int(match.group('exitCode'))
            pid = match.group('pid')

        match = launchedPattern.match(line)
        if match:
            matchingPids = (pid == match.group('pid'))

    return exitCode == 0 and matchingPids


def testLLDBOutput():
    failedOutputWithCrashLines = [
        '    frame #15: 0x00007fff73f4d305 libsystem_pthread.dylib`_pthread_body + 126',
        '    frame #16: 0x00007fff73f5026f libsystem_pthread.dylib`_pthread_start + 70',
        '    frame #17: 0x00007fff73f4c415 libsystem_pthread.dylib`thread_start + 13',
        '(lldb) quit 1'
    ]

    failedOutputWithFailedUnittest = [
        '===============================================================================',
        'test cases:  1 |  0 passed | 1 failed', 'assertions: 15 | 14 passed | 1 failed',
        '',
        'Process 279 exited with status = 1 (0x00000001) ',
        '',
        "Process 279 launched: '/Users/bsergeant/src/foss/ixwebsocket/test/build/Darwin/ixwebsocket_unittest' (x86_64)"
    ]

    successLines = [
        '...',
        '...',
        'All tests passed (16 assertions in 1 test case)',
        '',
        'Process 99232 exited with status = 0 (0x00000000) ',
        '',
        "Process 99232 launched: '/Users/bsergeant/src/foss/ixwebsocket/test/build/Darwin/ixwebsocket_unittest' (x86_64)"
    ]

    assert not isSuccessFullRun(failedOutputWithCrashLines)
    assert not isSuccessFullRun(failedOutputWithFailedUnittest)
    assert isSuccessFullRun(successLines)


def executeJob(job):
    '''Execute a unittest and capture info about it (runtime, success, etc...)'''

    start = time.time()

    sys.stderr.write('.')
    # print('Executing ' + job['cmd'] + '...')

    # 2 minutes of timeout for a single test
    timeout = 2 * 60
    command = Command(job['cmd'])
    timedout, ret = command.run(timeout)

    job['exit_code'] = ret
    job['success'] = ret == 0
    job['runtime'] = time.time() - start

    # Record unittest console output
    job['output'] = ''
    path = job['output_path']

    if os.path.exists(path):
        with open(path) as f:
            output = f.read()
            job['output'] = output

        outputLines = output.splitlines()

        if job['use_lldb']:
            job['success'] = isSuccessFullRun(outputLines)

        # Cleanup tmp file now that its content was read
        os.unlink(path)

    return job


def executeJobs(jobs, cpuCount):
    '''Execute a list of job concurrently on multiple CPU/cores'''

    print('Using {} cores to execute the unittest'.format(cpuCount))

    pool = multiprocessing.Pool(cpuCount)
    results = pool.map(executeJob, jobs)
    pool.close()
    pool.join()

    return results


def computeAllTestNames(buildDir):
    '''Compute all test case names, by executing the unittest in a custom mode'''

    cmd = '"{}" --list-test-names-only'.format(TEST_EXE_PATH)
    names = os.popen(cmd).read().splitlines()
    names.sort()  # Sort test names for execution determinism
    return names


def prettyPrintXML(root):
    '''Pretty print an XML file. Default writer write it on a single line
    which makes it hard for human to inspect.'''

    serializedXml = ET.tostring(root, encoding='utf-8')
    reparsed = minidom.parseString(serializedXml)
    prettyPrinted = reparsed.toprettyxml(indent="  ")
    return prettyPrinted


def generateXmlOutput(results, xmlOutput, testRunName, runTime):
    '''Generate a junit compatible XML file

    We prefer doing this ourself instead of letting Catch2 do it.
    When the test is crashing (as has happened on Jenkins), an invalid file
    with no trailer can be created which trigger an XML reading error in validateTestSuite.

    Something like that:
    ```
    <testsuite>
      <foo>
    ```
    '''

    root = ET.Element('testsuites')
    testSuite = ET.Element('testsuite', {
        'name': testRunName,
        'tests': str(len(results)),
        'failures': str(sum(1 for result in results if not result['success'])),
        'time': str(runTime),
        'timestamp': datetime.datetime.utcnow().isoformat(),
    })
    root.append(testSuite)

    for result in results:
        testCase = ET.Element('testcase', {
            'name': result['name'],
            'time': str(result['runtime'])
        })

        systemOut = ET.Element('system-out')
        systemOut.text = result['output'].decode('utf-8', 'ignore')
        testCase.append(systemOut)

        if not result['success']:
            failure = ET.Element('failure')
            testCase.append(failure)

        testSuite.append(testCase)

    with open(xmlOutput, 'w') as f:
        content = prettyPrintXML(root)
        f.write(content.encode('utf-8'))


def run(testName, buildDir, sanitizer, xmlOutput,
        testRunName, buildOnly, useLLDB, cpuCount, runOnly):
    '''Main driver. Run cmake, compiles, execute and validate the testsuite.'''

    # gen build files with CMake
    if not runOnly:
        runCMake(sanitizer, buildDir)

        if platform.system() == 'Linux':
            # build with make -j
            runCommand('make -C {} -j 2'.format(buildDir))
        elif platform.system() == 'Darwin':
            # build with make
            runCommand('make -C {} -j 8'.format(buildDir))
        else:
            # build with cmake on recent
            runCommand('cmake --build --parallel {}'.format(buildDir))

    if buildOnly:
        return

    # A specific test case can be provided on the command line
    if testName:
        testNames = [testName]
    else:
        # Default case
        testNames = computeAllTestNames(buildDir)

    # This should be empty. It is useful to have a blacklist during transitions
    # We could add something for asan as well.
    blackLists = {
        'ubsan': []
    }
    blackList = blackLists.get(sanitizer, [])

    # Run through LLDB to capture crashes
    lldb = ''
    if useLLDB:
        lldb = "lldb --batch -o 'run' -k 'thread backtrace all' -k 'quit 1'"

    # Jobs is a list of python dicts
    jobs = []

    for testName in testNames:
        outputPath = tempfile.mktemp(suffix=testName + '.log')

        if testName in blackList:
            log('Skipping blacklisted test {}'.format(testName), 'yellow')
            continue

        # testName can contains spaces, so we enclose them in double quotes
        cmd = '{} "{}" "{}" > "{}" 2>&1'.format(lldb, TEST_EXE_PATH, testName, outputPath)

        jobs.append({
            'name': testName,
            'cmd': cmd,
            'output_path': outputPath,
            'use_lldb': useLLDB
        })

    start = time.time()
    results = executeJobs(jobs, cpuCount)
    runTime = time.time() - start
    generateXmlOutput(results, xmlOutput, testRunName, runTime)

    # Validate and report results
    print('\nParsing junit test result file: {}'.format(xmlOutput))
    log('## Results', 'blue')
    success, tests = validateTestSuite(xmlOutput)

    if success:
        label = 'tests' if int(tests) > 1 else 'test'
        msg = 'All test passed (#{} {})'.format(tests, label)
        color = 'green'
    else:
        msg = 'unittest failed'
        color = 'red'

    log(msg, color)
    log('Execution time: %.2fs' % (runTime), 'blue')
    sys.exit(0 if success else 1)


def main():
    root = os.path.dirname(os.path.realpath(__file__))
    os.chdir(root)

    buildDir = os.path.join(root, 'build', platform.system())
    if not os.path.exists(buildDir):
        os.makedirs(buildDir)

    parser = argparse.ArgumentParser(description='Build and Run the engine unittest')

    sanitizers = ['tsan', 'asan', 'ubsan', 'none']

    parser.add_argument('--sanitizer', choices=sanitizers,
                        help='Run a clang sanitizer.')
    parser.add_argument('--test', '-t', help='Test name.')
    parser.add_argument('--list', '-l', action='store_true',
                        help='Print test names and exit.')
    parser.add_argument('--no_sanitizer', action='store_true',
                        help='Do not execute a clang sanitizer.')
    parser.add_argument('--validate', action='store_true',
                        help='Validate XML output.')
    parser.add_argument('--build_only', '-b', action='store_true',
                        help='Stop after building. Do not run the unittest.')
    parser.add_argument('--run_only', '-r', action='store_true',
                        help='Only run the test, do not build anything.')
    parser.add_argument('--output', '-o', help='Output XML file.')
    parser.add_argument('--lldb', action='store_true',
                        help='Run the test through lldb.')
    parser.add_argument('--run_name', '-n',
                        help='Name of the test run.')
    parser.add_argument('--cpu_count', '-j', type=int, default=multiprocessing.cpu_count(),
                        help='Number of cpus to use for running the tests.')

    args = parser.parse_args()

    # Windows does not play nice with multiple files opened by different processes
    # "The process cannot access the file because it is being used by another process"
    if platform.system() == 'Windows':
        args.cpu_count = 1

    # Default sanitizer is tsan
    sanitizer = args.sanitizer

    if args.no_sanitizer:
        sanitizer = 'none'
    elif args.sanitizer is None:
        sanitizer = 'tsan'

    # Sanitizers display lots of strange errors on Linux on CI,
    # which looks like false positives
    if platform.system() != 'Darwin':
        sanitizer = 'none'

    defaultRunName = 'ixengine_{}_{}'.format(platform.system(), sanitizer)

    xmlOutput = args.output or XML_OUTPUT_FILE
    testRunName = args.run_name or os.getenv('IXENGINE_TEST_RUN_NAME') or defaultRunName

    global TEST_EXE_PATH

    if platform.system() == 'Windows':
        TEST_EXE_PATH = os.path.join(buildDir, BUILD_TYPE, 'ixwebsocket_unittest.exe')
    else:
        TEST_EXE_PATH = '../build/test/ixwebsocket_unittest'

    if args.list:
        # catch2 exit with a different error code when requesting the list of files
        try:
            runTest('--list-test-names-only', buildDir, xmlOutput, testRunName)
        except AssertionError:
            pass
        return

    if args.validate:
        validateTestSuite(xmlOutput)
        return

    if platform.system() != 'Darwin' and args.lldb:
        print('LLDB is only supported on Apple at this point')
        args.lldb = False

    return run(args.test, buildDir, sanitizer, xmlOutput,
               testRunName, args.build_only, args.lldb, args.cpu_count, args.run_only)


if __name__ == '__main__':
    main()
