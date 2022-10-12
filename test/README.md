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

Unfortunately, things are a little more convoluted on Windows. To work
%PATH% must include both the cmake binary directory and the path to
_buildwin_

The cmake binary directory contains cmake.exe. If cmake can be invoked
on the command line, this means %PATH% already contains this directory.
If not it can be added on a standard install using

    > set PATH=%PATH%;C:\Program Files\CMake\bin

The _buildwin_ directory can be added either as an absolute path or a
relative one. If relative, it is rooted in _build\test_ and added using

    > set PATH=%PATH%;..\..\buildwin

Once %PATH% is set up tests are run using

    > cd build
    > cmake --build . --target=run-tests --config RelWithDebInfo

Common error is test.exe failing with message `Result: Exit code 0xc0000135`.
This is usually caused by test.exe not being able to load the shared
libraries in _buildwin_. Check that _buildwin_ is part pf %PATH%, see above.
