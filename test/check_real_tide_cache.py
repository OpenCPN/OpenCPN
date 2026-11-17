"""Linux/GNU Make integration check using the real tide engine and shipped data.

First build opencpn with CMake's Unix Makefiles generator. Then run:
  xvfb-run -a python3 test/check_real_tide_cache.py /path/to/build
A working display can be used instead of xvfb-run. wx-config must be on PATH,
or pass its absolute path as the second argument. This is not a GUI paint test.
"""
from pathlib import Path
import os
import re
import shlex
import subprocess
import sys
import tempfile

source = Path(__file__).resolve().parents[1]
build = Path(sys.argv[1]).resolve()
wx_config = sys.argv[2] if len(sys.argv) > 2 else "wx-config"
flags = (build / "CMakeFiles/opencpn.dir/flags.make").read_text()
args = []
for key in ("CXX_DEFINES", "CXX_INCLUDES"):
    args += shlex.split(re.search(r"^" + key + r" = (.*)$", flags, re.M)[1])
args += ["-std=c++17", "-O1", "-g", "-ffunction-sections", "-fdata-sections"]
compiler = os.environ.get("CXX", "c++")
with tempfile.TemporaryDirectory() as directory:
    directory = Path(directory)
    objects = []
    for name in ("tcmgr", "tc_data_factory", "tc_data_source",
                 "tcds_ascii_harmonic", "tcds_binary_harmonic",
                 "idx_entry", "station_data"):
        obj = directory / (name + ".o")
        subprocess.run([compiler, *args, "-c", str(source / "gui/src" / (name + ".cpp")),
                        "-o", str(obj)], check=True)
        objects.append(str(obj))
    for name in ("logger", "georef"):
        objects.append(str(build / "model/CMakeFiles/_model_src.dir/src" / (name + ".cpp.o")))
    wx_flags = shlex.split(subprocess.check_output([wx_config, "--libs"], text=True))
    executable = directory / "tide-cache-test"
    subprocess.run([compiler, *args, str(source / "test/tide_extrema_real.cpp"),
                    *objects, "-Wl,--gc-sections", *wx_flags, "-o", str(executable)], check=True)
    subprocess.run([str(executable), str(source / "data/tcdata/HARMONICS_NO_US.IDX")],
                   check=True, timeout=30)
