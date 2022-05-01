OpenCPN flatpak packaging.
==========================

What's here?
------------
Necessary tools to create and use the nightly, flatpak builds. This
is **not** about the official distribution on flathub, this lives in
https://github.com/flathub/org.opencpn.OpenCPN


End-user usage:
---------------

Users might need to install flatpak, described in https://flatpak.org/setup/.
To install the latest nightly build:

    $ flatpak install --user \
          https://flatpak.ocpnci.kalian.cz/opencpn.flatpakref


See https://opencpn.org/wiki/dokuwiki/doku.php?id=opencpn:opencpn_user_manual:getting_started:opencpn_installation:flatpak 
for more info on the Flatpak installation.

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
