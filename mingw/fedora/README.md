What's here?
============

Tools to build a windows 32-bit executable using the mingw toolchain.

How?
====
    $ sudo  dnf copr enable leamas/opencpn-mingw
    $ sudo dnf builddep opencpn-deps.spec
    $ cd ../..
    $ rm -rf build; mkdir build
    $ cd build;
    $ cmake -DCMAKE_TOOLCHAIN_FILE=../mingw/fedora/toolchain.cmake ..
    $ make 
    $ make package

Notes:
=====
The build requires updated packages, notably wx3GTK, available at
https://copr.fedorainfracloud.org/coprs/leamas/opencpn-mingw.
The copr enable command above sets up access to this repo.
