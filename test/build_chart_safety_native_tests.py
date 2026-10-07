#!/usr/bin/env python3
"""Build private safety fixtures against a completed Makefiles native build.

The harness replaces only the host's chart_safety_raster.cpp object, exercising
private production classifiers without adding exported test hooks to the ABI.
Run the resulting native-fixtures executable on a private Xvfb display.
GPL-2.0-or-later.
"""

import argparse
from pathlib import Path
import shlex
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    options = parser.parse_args()
    source = Path(__file__).resolve().parents[1]
    build = options.build_dir.resolve()
    out = options.output_dir.resolve()
    out.mkdir(parents=True, exist_ok=True)
    flags = dict(
        line.split(" = ", 1)
        for line in (build / "CMakeFiles/opencpn.dir/flags.make")
        .read_text()
        .splitlines()
        if " = " in line
    )
    obj = out / "native-fixtures.o"
    compile_command = [
        "c++",
        *shlex.split(flags["CXX_DEFINES"]),
        *shlex.split(flags["CXX_INCLUDES"]),
        *shlex.split(flags["CXX_FLAGS"]),
        "-c",
        str(source / "test/chart_safety_native_tests.cpp"),
        "-o",
        str(obj),
    ]
    link_command = shlex.split(
        (build / "CMakeFiles/opencpn.dir/link.txt").read_text()
    )
    link_command = [
        arg for arg in link_command if not arg.startswith("-Wl,--dependency-file=")
    ]
    link_command[
        link_command.index("CMakeFiles/opencpn.dir/gui/src/chart_safety_raster.cpp.o")
    ] = str(obj)
    link_command[link_command.index("-o") + 1] = str(out / "native-fixtures")
    link_command += ["-Wl,--wrap=main", "-lgtest"]
    log_path = out / "native-tests-build.log"
    with log_path.open("w") as log:
        for command, cwd in ((compile_command, source), (link_command, build)):
            result = subprocess.run(
                command, cwd=cwd, stdout=log, stderr=subprocess.STDOUT
            )
            if result.returncode:
                log.flush()
                print(log_path.read_text()[-7000:])
                raise SystemExit(result.returncode)
    print("Native fixture harness linked", flush=True)


if __name__ == "__main__":
    main()
