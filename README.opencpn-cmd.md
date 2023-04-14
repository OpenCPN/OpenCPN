opencpn-cmd README
==================

This is a temporary file used under development of opencpn-cmd. It will 
be replaced by a manual page. Please don't link to it.

opencpn-cmd provides basic commandline access to some OpenCPN functions.
At the time of writing it is mostly about plugins: list, install, remove,
update, etc.

The basic command for getting help is `opencpn-cmd --help` which should give
a clue to what's possible.


Debian/Ubuntu
-------------

opencpn-cmd is available direct after build in the build directory and could
be run directly:

    $ cd build
    $ cmake ...
    $ make ...
    $ ./opencpn-cmd --help

After `sudo make install` it is globally available. The plan is to incorporate 
opencpn-cmd in upcoming .deb packages.

Flatpak
-------

Make a local Flatpak build as described in 
https://opencpn-manuals.github.io/main/ocpn-dev-manual/od-compile-linux.html
This will also make a local installation.

opencpn-cmd needs to run in the Flatpak sandbox. Do:

    $ flatpak run --command=bash --devel org.opencpn.OpenCPN
    📦 $ opencpn-cmd --help

MacOS 
-----
After a local build followed by `sudo make install` opencpn-cmd is available in
/usr/local/bin/ Run using:

    $ /usr/local/bin/OpenCPN.app/Contents/MacOS/opencpn-cmd --help

The installer installs it under Applications. Run using:

    $ /Applications/OpenCPN.app/Contents/MacOS/opencpn-cmd --help

Windows
-------

The NSIS installer installs opencpn-cmd in the installation directory.
In a standard install do:

    > cd "\Program Files (x86)\OpenCPN"
    > opencpn-cmd --help
