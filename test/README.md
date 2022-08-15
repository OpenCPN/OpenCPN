Provisionary Unittest README
============================

This directory contains a very small set of unit tests. They are hooked up
in the overall OpenCPN build.

Running the tests requires the ctest runner which comes with cmake. Other
runners are available, but ctest seems to do the job.

Build and run is basically about first running the ordinary build. After a
successful build, tests are run using

    $ cd build
    $ cmake --build . --target=run-tests

On non-windows platforms, `make run-tests `can be used instead.

Running tests on Windows
-------------------------

Unfortunately, things are a little more convoluted on windows. Steps as
follows after completing a regular build:

    > cd build
    > cmake --build . --config RelWithDebInfo --target tests
    > cd test
    > set PATH=%PATH%..\..\buildwin
    > ctest -C RelWithDebInfo
