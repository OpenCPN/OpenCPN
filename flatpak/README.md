OpenCPN flatpak packaging.
==========================

What's here?
------------
Here is the necessary tools to create an OpenCPN flatpak. A flatpak is a 
distro-agnostic format which basically can be installed on all major Linux
distros. From a user perspective, the installation is about adding a repo
and then installing the application from that repo, like:

    $ flatpak --user remote-add opencpn https://url/to/repo
    $ flatpak --user install org.opencpn.OpenCPN

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

First, review the org.opencpn.OpenCPN.json manifest file. In the very end
are the definitions for the opencpn source; normally, it should just be
to update the tag to make a new build. Then build the package in opencpn/:

    $ flatpak-builder --force-clean opencpn org.opencpn.OpenCPN.json

In order to sign the repo you need to have a public gpg key available. The
urban wisdom seems to be to use a specific key created for this purpose.
In any case, the public key is represented like below (your key is
obviously something else).

Create a repository and sign it:

    $ flatpak-builder --export-only --repo=repo --gpg-sign=4E068B4C \
          opencpn org.opencpn.OpenCPN.json

Now the repo is ready and can be published under an url, and used 
by ```flatpak remote-add``` as described above.
