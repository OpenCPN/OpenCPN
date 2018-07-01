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
  - Adding plugins is not supported ATM.
  - One single package without options do drop docs, plugins etc.


Usage:
------

Users might need to install flatpak, described in https://flatpak.org/setup/.
Using the provisionary github repo, opencpn can be installed using: 

    $ flatpak --user install https://leamas.github.io/opencpn.flatpakref

Same thing can be achieved pointing the browser to https://leamas.github.io.

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

Armed with these tools, initialize by installing the runtime and sdk:

     $ sudo flatpak remote-add --if-not-exists flathub \
           https://dl.flathub.org/repo/flathub.flatpakrepo
     $ sudo flatpak install flathub org.freedesktop.Platform//1.6
     $ sudo flatpak install flathub org.freedesktop.Sdk//1.6

Review the org.opencpn.OpenCPN.json manifest file. In the very end
are the definitions for the opencpn source; normally, it should just be
to update the tag to make a new build. Then build the package in *opencpn/*:

    $ flatpak-builder --force-clean opencpn org.opencpn.OpenCPN.json

In order to sign the repo you need to have a public gpg key available. The
urban wisdom seems to be to use a specific key created for this purpose.
Create and export one using something like:

    $  mkdir gpg
    $  gpg2 --homedir=gpg --quick-gen-key leamas@opencpn.org
    $  gpg2 --homedir=gpg --export -a leamas@opencpn.org > leamas@opencpn.key

Build the repo stable branch and sign contents + summary:

    $ flatpak build-export repo opencpn stable
    $ flatpak build-sign --gpg-sign=leamas@opencpn.org --gpg-homedir=gpg repo
    $ flatpak build-update-repo --gpg-sign=leamas@opencpn.org --gpg-homedir=gpg repo

Now the repo is ready and can be published under an url and used as described 
above.
