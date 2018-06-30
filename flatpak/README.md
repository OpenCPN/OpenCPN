OpenCPN flatpak packaging.
==========================

What's here?
------------
Necessary tools to create an OpenCPN flatpak. A flatpak is a distro-agnostic 
format which basically can be installed on all major Linux distros. 

Packaging in general needs to live in a separate branch. This is also
true for this directory.

Pros:
  
  - Works an all major linux distros.
  - Easy installation
  - Can be hosted by opencpn, no central repo required.
  - Maintenance better than e. g., snap and appimage due to runtimes, not
    much is actually bundled.

Cons:

  - Device access requires installing udev rules.
  - One single package without options do drop docs, plugins etc.


Usage:
------

Users might need to install flatpak, described in https://flatpak.org/setup/.
Using the provisionary fedorapeople repo, opencpn can be installed using: 

    $ sudo flatpak install https://leamas.fedorapeople.org/opencpn.flatpakref
    $ sudo flatpak install https://leamas.fedorapeople.org/opencpn-plugin-base.flatpakref

Same thing can be achieved pointing the browser to 
https://leamas.fedorapeople.org/opencpn/

In order to fix device permissions, a udev rule needs to be installed. Create
a file called *70-serial-opencpn.rules* like

    KERNEL=="ttyUSB*", MODE="0666"
    KERNEL=="ttyACM*", MODE="0666"
    KERNEL=="ttyS*", MODE="0666"

and drop it into */etc/udev/rules.d*. A reboot might be needed for it to 
take effect. Normally, only one of the three lines is required depending
on the device used - the rest could be removed.


Repo maintenance
----------------

The first steps is about installing *flatpak* and *flatpak-builder* as
described at https://flatpak.org/setup/

In order to sign the repo you need to have a public gpg key available. The
urban wisdom seems to be to use a specific key created for this purpose.
Create and export one using something like:

    $  mkdir gpg
    $  gpg2 --homedir=gpg --quick-gen-key leamas@opencpn.org
    $  gpg2 --homedir=gpg --export -a leamas@opencpn.org > opencpn.key


Armed with these tools, initialize by installing the runtime and sdk:

     $ sudo flatpak remote-add --if-not-exists flathub \
           https://dl.flathub.org/repo/flathub.flatpakrepo
     $ sudo flatpak install flathub org.freedesktop.Platform//1.6
     $ sudo flatpak install flathub org.freedesktop.Sdk//1.6

Review the org.opencpn.OpenCPN.yaml manifest file. In the very end
are the definitions for the opencpn source; normally, it should just be
to update the tag to make a new build. Then build the packages in *opencpn/*
and *base*

    $ BRANCH=stable make build

Build the repo stable branch and sign contents + summary:

    $ GPG_HOMEDIR=gpg GPG_KEY=leamas@opencpn.org make sign

Create the website repo directory 

    $ make DESTDIR=~/opencpn-website§ install


Packaging plugins
-----------------

The file system used by flatpak apps is not available in a form where
users could just drop a file. However, packaging plugins is normally 
trivial. An example from the shipdriver plugin, 
https://github.com/Rasbats/shipdriver_pi.git.

Use a unique id on the form *org.opencpn.OpenCPN.Plugin.MyPlugin*. Ensure 
that the plugin builds in a normal context. Then, create the plugin 
manifest with the name *org.opencpn.OpenCPN.Plugin.PluginName.yaml* like:

    id: org.opencpn.OpenCPN.Plugin.ShipDriver
    runtime: org.opencpn.OpenCPN
    runtime-version: stable
    sdk: org.freedesktop.Sdk//1.6
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
*org.opencpn.OpenCPN* and *org.opencpn.Plugin.Base*. Then, build package 
in directory *app* using:

    $ flatpak-builder app org.opencpn.OpenCPN.Plugin.PluginName.yaml

Export the built package to a repo directory (here named *repo*) on the stable 
branch using:

    $ flatpak build-export repo app stable

Add a local repository pointing to the repo dir:

    $ sudo flatpak remote-add my-repo $PWD/repo

List the repo contents:

    $ flatpak remote-ls my-repo

Install and test the plugin:

    $ sudo flatpak install my-repo org.opencpn.OpenCPN.Plugin.MyPLugin
