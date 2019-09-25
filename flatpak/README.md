OpenCPN flatpak packaging.
==========================

What's here?
------------
Necessary tools to create an OpenCPN flatpak. A flatpak is a distro-agnostic 
format which basically can be installed on all major Linux distros. 

Pros:
  
  - Works on all major linux distros.
  - Easy installation
  - Can be hosted by opencpn, no central repo required.
  - Maintenance better than e. g., snap and appimage due to runtimes, not
    much is actually bundled.

Cons:

  - Device access requires installing udev rules.
  - One single package without options do drop docs etc.
  - Plugins need to be patched and packaged to be usable.


Usage:
------

Users might need to install flatpak, described in https://flatpak.org/setup/.
Using the provisionary beta-test directory

    $ flatpak remote-add --if-not-exists \
       flathub https://flathub.org/repo/flathub.flatpakrepo

    $ flatpak install --user \
       http://opencpn.duckdns.org/opencpn-beta/website/opencpn.flatpakref


In order to fix device permissions, a udev rule needs to be installed. Create
a file called *70-serial-opencpn.rules* like

    KERNEL=="ttyUSB*", MODE="0666"
    KERNEL=="ttyACM*", MODE="0666"
    KERNEL=="ttyS*", MODE="0666"

and drop it into */etc/udev/rules.d*. A reboot might be needed for it to 
take effect. Normally, only one of the three lines is required depending
on the device used - the rest could be dropped.


Repo maintenance
----------------

The first steps is about installing *flatpak* and *flatpak-builder* as
described at https://flatpak.org/setup/

In order to sign the repo a public gpg key should be available. The
urban wisdom seems to be to use a specific key created for this purpose.
Create and export one using something like:

    $ mkdir -m 700 ~/opencpn-gpg
    $ gpg2 --homedir=~/opencpn-gpg --quick-gen-key leamas@opencpn.org
    $ gpg2 --homedir=~/opencpn-gpg --export -a leamas@opencpn.org > opencpn.key


Armed with these tools, initialize by installing the runtime and sdk:

     $ sudo flatpak remote-add --if-not-exists flathub \
           https://dl.flathub.org/repo/flathub.flatpakrepo
     $ sudo flatpak install flathub org.freedesktop.Platform//18.08
     $ sudo flatpak install flathub org.freedesktop.Sdk//18.08

Review the org.opencpn.OpenCPN.yaml manifest file. In the very end
are the definitions for the opencpn source; the current setup is
to build the tip of the plug-mgr branch.  Review and update as
rquired. Then build the packages in *opencpn/* and *base*

    $ BRANCH=stable make build

Build the repo stable branch and sign contents + summary:

    $ GPG_HOMEDIR=~/opencpn-gpg GPG_KEY=leamas@opencpn.org make sign

Create the website repo directory 

    $ make DESTDIR=~/opencpn-website install


Packaging plugins
-----------------

The file system used by flatpak apps is not available in a form where
users could just drop a file. Note that once the plugin installer
(PR #1457) is merged all these plugin can be used .

The alternative is to package the plugin which normally is
trivial. An example from the shipdriver plugin, 
https://github.com/lea~/opencpn-gpgmas/shipdriver\_pi.

The first step is to update the API. Copy the file ocpn\_plugin.h
from the opencpn package to shipdriver src directory. Then, patch
the plugin to use *GetPluginDataDir* instead of *GetpSharedDataLocation()*,
for  the shipdriver this looks like:

    --- a/src/icons.cpp
    +++ b/src/icons.cpp

         wxFileName fn;
    -    fn.SetPath(*GetpSharedDataLocation());
    -    fn.AppendDir(_T("plugins"));
    -    fn.AppendDir(_T("ShipDriver_pi"));
    +    fn.SetPath(GetPluginDataDir("ShipDriver_pi"));
         fn.AppendDir(_T("data"));
         //* The argument to GetPluginDataDir() is the name of the plugin
         //  data directory, as used to compute it in the old code.
 
The part(s) to patch could usually be found by searching for
GetpSharedDataLocation(). Not all plugins uses a data location though.

Pick a unique id on the form *org.opencpn.OpenCPN.Plugin.MyPlugin*. Ensure 
that the plugin builds in a normal context. Create the plugin manifest with 
the name *org.opencpn.OpenCPN.Plugin.MyPlugin.yaml* like:

    id: org.opencpn.OpenCPN.Plugin.ShipDriver
    runtime: org.opencpn.OpenCPN
    runtime-version: stable
    sdk: org.freedesktop.Sdk//18.08
    build-extension: true
    separate-locales: false
    appstream-compose: false
    modules: 
        - name: shipdriver
          no-autogen: true
          cmake: true
          config-opts: 
              - -DCMAKE_INSTALL_PREFIX:PATH=/app/extensions/ShipDriver
          sources: 
              - type: git
                url: https://github.com/Rasbats/shipdriver_pi.git
                tag: v0.5

Refer to the flatpak docs how to modify this. Note the *id* and 
*CMAKE_INSTALL_PREFIX*. The manifest supports all kinds of sources, patches 
and simple scripting required to build the package.  See
http://docs.flatpak.org/en/latest/getting-started.html. The crash-course:

See above for how to initialize the build environment. Install 
*org.opencpn.OpenCPN*. Then, build package in directory *app* using:

    $ flatpak-builder app org.opencpn.OpenCPN.Plugin.PluginName.yaml

Export the built package to a repo directory (here named *repo*) on the stable 
branch using:

    $ flatpak build-export repo app stable

Add a local remote pointing to the repo dir:

    $ sudo flatpak remote-add my-repo $PWD/repo

List the repo contents:

    $ flatpak remote-ls my-repo

Install and test the plugin:

    $ sudo flatpak install my-repo org.opencpn.OpenCPN.Plugin.MyPLugin
