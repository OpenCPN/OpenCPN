# Source: https://github.com/scivision/cmake-patch-file

#[==[

MIT License

Copyright (c) 2021 Michael

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
]==]

find_package(Git REQUIRED)

set(CMAKE_EXECUTE_PROCESS_COMMAND_ECHO STDOUT)

cmake_path(GET patch FILENAME patch_name)

execute_process(
  COMMAND ${GIT_EXECUTABLE} --git-dir= apply -p1 --ignore-whitespace "${patch}"
  RESULT_VARIABLE ret
  ERROR_VARIABLE err
  TIMEOUT 5
)
# if patch already applied - will fail

if (NOT ret EQUAL 0)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} --git-dir=
        apply -p1 --ignore-whitespace --check -R "${patch}"
    RESULT_VARIABLE ret1
    ERROR_VARIABLE err1
    TIMEOUT 5
  )
  # if succeeds - patch sucessfully applied - conf OK

  if (NOT ret1 EQUAL 0)
    message(
      FATAL_ERROR "Patch ${patch} failed to apply:
      ${ret} ${err}
      ${ret1} ${err1}"
    )
  endif ()
endif ()
