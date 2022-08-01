Provisionary Unittest README
============================

This directory contains a very small set of unit tests. They are not hooked
up in the overall OpenCPN build, just a separate directory.

The tests requires wxWidgets. At the time of writing there are no provisions
for using pre-compiled binaries for Windows and MacOS which means that the
tests most likely only builds on Linux and Flatpak.

Running the tests requires the ctest runner which comes with cmake. Other
runners are available, but ctest seems to do the job.

Build and run:

    $ rm -rf build && mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ ctest
