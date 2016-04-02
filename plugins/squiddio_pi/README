Squiddio for OpenCPN
=======================================
Squiddio is a global user-sourced and maintained repository of sailing destinations. Plugin enables downloading and viewing destinations as waypoints from within OpenCPN. More information and object libraries available at http://squidd.io/squiddio_pi 

Compiling
=========
You have to be able to compile OpenCPN itself - Get the info at http://opencpn.org/ocpn/developers_manual

* git clone git://github.com/mauroc/squiddio_pi.git

Note: the plugin requires libcurl. To install the libraries in Linux Debian: apt-get install libcurl4-gnutls-dev. 
To install in Windows, see the "" below.

Build:

* cd squiddio_pi
* mkdir build
* cd build
* cmake ..

On Unix:
* make

On Windows:
You need a corresponding opencpn.lib file in your build directory. If you don't want to build one yourself as part of the OpenCPN core, several different are available at https://sourceforge.net/projects/opencpnplugins/files/opencpn_lib/. For this plugin, the 3.3.1824 or newer is fine.
* cmake --build .

Generate packages:

On Unix:
To build RPM packages, you need the respective tools, so on Debian/Ubuntu, do "sudo apt-get install rpm"
* make package

On Windows:
cmake --build . --target package --config release


Installing libcurl in Windows
=============================
Download all the cotents from: http://sourceforge.net/projects/opencpnplugins/files/weatherfax_pi/wf_depends.7z/download
When using Mingw, only the single file "libcurl.dll" is needed. In visual studio, copy all the downloaded files to the ... directory.
On Windows, also the CA certificates bundle must be part of the package. Get it from http://sourceforge.net/projects/opencpnplugins/files/chartdldr_pi/curl-ca-bundle.crt/download and place into the buildwin directory

License
=======
The plugin code is licensed under the terms of the GPL v3 or, at your will, later.
