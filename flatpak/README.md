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


End-user usage:
---------------

Users might need to install flatpak, described in https://flatpak.org/setup/.
Using the provisionary beta-test directory:

    $ flatpak install --user \
          http://opencpn.duckdns.org/opencpn-beta/opencpn.flatpakref

Using the provisionary nigtly builds:

    $ flatpak install --user \
          http://opencpn.duckdns.org/opencpn/opencpn.flatpakref


In order to fix device permissions, a udev rule needs to be installed. Create
a file called *70-serial-opencpn.rules* like

    KERNEL=="ttyUSB*", MODE="0666"
    KERNEL=="ttyACM*", MODE="0666"
    KERNEL=="ttyS*", MODE="0666"

and drop it into */etc/udev/rules.d*. A reboot might be needed for it to
take effect. Normally, only one of the three lines is required depending
on the device used - the rest could be dropped.


Developer usage
---------------

The Makefile is capable of building and installing the opencpn flatpak
package locally. The first steps are about installing *flatpak* and
*flatpak-builder* as described at https://flatpak.org/setup/

Armed with these tools, initialize by installing the runtime and sdk:

    $ sudo flatpak install flathub org.freedesktop.Platform//20.08
    $ sudo flatpak install flathub org.freedesktop.Sdk//20.08

Review the `org.opencpn.OpenCPN.yaml` manifest file. In the very end
are the definitions for the opencpn source; the current setup is
to build the tip of the master branch. Update as required.

Build and install the opencpn flatpak package:

    $ cd build; make -f ../flatpak/Makefile build


Repo maintenance
----------------

The first steps are to build as described in Developer Usage above.

Then, install the results of the build into the website/ directory:

    $ cd build; make -f ../flatpak/Makefile install

In order to sign the repo a public gpg key should be available. The
urban wisdom seems to be to use a specific key created for this purpose.
Create and export one using something like:

    $ mkdir -m 700 ~/opencpn-gpg
    $ gpg2 --homedir=~/opencpn-gpg --quick-gen-key leamas@opencpn.org
    $ gpg2 --homedir=~/opencpn-gpg --export -a leamas@opencpn.org > opencpn.key

Sign contents + summary i. e., the result of the build:

    $ cd build; GPG_HOMEDIR=~/opencpn-gpg GPG_KEY=leamas@opencpn.org \
          make -f ../flatpak/Makefile sign

Given proper permissions, the result can be published using

    $ cd build; make -f ../flatpak/Makefile publish

A raw, rsync publish command might look like (beta repository)

    $ rsync -a  --info=stats website/ --delete-after \
        mumin.crabdance.com:/var/www/ocpn-flatpak-beta

There are multiple variables in the Makefile making it possible to tweak
this workflow in various ways.
