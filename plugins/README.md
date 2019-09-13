Downloadable plugins README
===========================

The subdirectores metadata and icons provides support for plugins
which can be downloaded and installed by the new plugin UI. Note
that the previous way to install using installation packages is
still supported.

Plugins which could be downloaded and installed needs to have:

  - A downloadable installation tarball available at an URI.
  - Plugin metadata merged into the common file ocpn-plugins.xml.
  - An optional icon available in the plugins/icons directory.


Tarball
-------

The installable plugin tarball is basically the result of *make install*
packed into a tar archive. Directory layout is platform-dependent,
see below.


ocpn-plugins.xml
----------------
Creating a new metadata file should be straight-forward using existing
examples. Patching existing xml files is of course also possible.
BEWARE: The name is as returned by the `common_name` function,
typically without the *_pi* suffix.

Once a new metadata file is created or an existing is patched a new file
*ocpn-plugins.xml* should be created.  By default, a system-wide
*ocpn-plugins.xml* file is installed in the common data directory.
There should be no reason to patch this.

Instead, create a new file by first dropping the new or modified file
into the *plugins/metadata* directory and run something like:

    plugins/metadata/metadata-builder plugins/metadata/* >
        ~/.opencpn/ocpn-plugins.xml

The path to ocpn-plugins.xml is platform-dependent, see below. A
*ocpn-plugins.xml* file installed in the private directory
overrides the system-wide default one.


Icons
-----

The plugin downloader supports png and svg icons.The basename of
the icon should be the same as the name in the xml metadata.

Icons are rescaled when displayed. For this reason svg icons are
preferred. Png icons should be reasonable large e. g. 64 x 64.


Platform notes - linux.
-----------------------

System-wide metadata: Typically  */usr/share/opencpn/ocpn-plugins.xml*,
possibly  distribution-dependent.

User metadata (overrides system if existing): *~/.opencpn/ocpn-plugins.xml*

Tarball layout (top directory could have any name):

   - .so dynamic libraries goes into *top-dir/usr/lib/opencpn* or
     *top_dir/usr/local/lib/opencpn*.

   - Optional helper binaries goes into *top-dir/usr/bin* or
     *top-dir/usr/local/bin*.

   - Plugin data files: *top-dir/usr/share/opencpn/plugins/<name>*
     where <name> is as in the plugin metadata.


Platform notes - Windows
------------------------

System-wide metadata: *C:\Program Files (x86)\OpenCPN\ocpn-plugins.xml*

User metadata (overrides system if existing):
    *%LOCALAPPDATA%\opencpn\ocpn-plugins.xml*, normally expanded to
    *:C:\ProgramData\opencpn\ocpn-plugins.xml*

Tarball layout (top directory could have any name):

   - .dll dynamic libraries and .exe binaries goes to
     *top-dir/plugins/\*.exe*
   - plugin data files goes to *top-dir/plugins/plugin-name*


Platform notes - MacOS
----------------------

System-wide metadata:
    *~/Desktop/OpenCPN.app/Contents/SharedSupport/ocpn-plugins.xml*

User metadata (overrides system if existing):
    *~/Library/Preferences/opencpn/ocpn-plugins.xml*

Tarball layout (top directory could have any name):

  - dylib dynamic libraries and binaries:
    *top-dir/OpenCPN.app/Contents/PlugIns/*

  - Data files:
    *top-dir/OpenCPN.app/Contents/SharedSupport/plugins/plugin-name*

NOTE: A deprecated prefix *topdir/tmp/bin* is still supported.


Platform notes - Flatpak
------------------------

System-wide metadata: */app/share/opencpn/ocpn-plugins.xml*

User metadata (overrides system if existing): *~/.opencpn/ocpn-plugins.xml*

Tarball layout: identical to linux with a manifest.json in the top directory
added.
