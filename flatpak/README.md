OpenCPN flatpak packaging.
==========================

What's here?
------------
Here is the necessary tools to create an OpenCPN flatpak. A flatpak is a 
distro-agnostic format which basically can be installed on all major Linux
distros. From a user perspective, the installation is about adding a repo
and then installing the application from that repo, like:

    $ flatpak --user remote-add opencpn https://url/to/repo
    $ flatpak --user install opencpn org.opencpn.OpenCPN

Seen as provider, publishing a flatpak is a way to provide a binary package
to a much larger user-base than for example an Ubuntu PPA.

From the developer perspective, flatpak has some advantages compared to
other tools like Appimage and snap, notably the distributed approach with
no mandatory central hub, large shared low-level runtime libs and the
sandbox implementation.

Packaging in general needs to live in a separate branch. This is also
true for this directory.

How to use
----------

The first steps is about installing *flatpak* and *flatpak-builder*. How
to do this varies depending on platform and is described at 
https://flatpak.org/setup/

Armed with these tools, inititalize by installing the runtime and sdk :

     $ flatpak remote-add --if-not-exists flathub \
           https://dl.flathub.org/repo/flathub.flatpakrepo
     $ flatpak install flathub org.freedesktop.Platform//1.6
     $ flatpak install flathub org.freedesktop.Sdk//1.6

Review the org.opencpn.OpenCPN.json manifest file. In the very end
are the definitions for the opencpn source; normally, it should just be
to update the tag to make a new build. Then build the package in *opencpn/*:

    $ flatpak-builder --force-clean opencpn org.opencpn.OpenCPN.json

In order to sign the repo you need to have a public gpg key available. The
urban wisdom seems to be to use a specific key created for this purpose.
Create  and export one using something like:

    $  mkdir gpg
    $  gpg2 --homedir=gpg --quick-gen-key leamas@opencpn.org
    $  gpg2 --homedir=gpg --export leamas@opencpn.org > leamas@opencpn.key

Build the repo stable branch and sign contents + summary:

    $ flatpak build-export repo opencpn stable
    $ flatpak build-sign --gpg-sign=leamas@opencpn.org --gpg-homedir=gpg repo
    $ flatpak build-update-repo --gpg-sign=leamas@opencpn.org --gpg-homedir=gpg repo

Now the repo is ready and can be published under an url, and used by 
```flatpak remote-add``` using the exporeted key:

    $  sudo flatpak remote-add  --gpg-import=leamas@opencpn.key   opencpn $PWD/repo
