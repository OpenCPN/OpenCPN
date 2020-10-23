#!/bin/sh  -xe
#
# Build and publish flatpak to the beta repo inside the Fedora container.
#
# Uses the FLATPAK_KEY environment variable to decrypt 
#  - ggp signing key -- gpg.tar.gz 
#  - ssh deployment key -- amazon-ec2.pem
# 

if [ -z "$FLATPAK_KEY" ]; then
    echo "Reguired \$FLATPAK_KEY not found, giving up"
    exit 1
fi

set -xe

# Automatic updates are in the way when starting build host:
sudo systemctl disable --now dnf-makecache.timer
sudo dnf clean all

# Install required packages
su -c "dnf install -y -q sudo dnf-plugins-core"
sudo dnf install -q -y appstream flatpak-builder ccrypt make rsync gnupg2 

test -d /opencpn-ci && cd /opencpn-ci || :

# Set up flatpak
flatpak --user remote-add --if-not-exists \
    flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak --user install -y org.freedesktop.Platform//18.08
flatpak --user install -y org.freedesktop.Sdk//18.08

# Patch to use official master branch from github and build + build number.
cd flatpak
sed -i -e '/url:/s|\.\.|https://github.com/OpenCPN/OpenCPN.git|' \
    -e "/BUILD_NUMBER/s/0/$BUILD_NUMBER/" \
    org.opencpn.OpenCPN.yaml

test -d ../build || mkdir ../build
cd ../build
make -f ../flatpak/Makefile build
flatpak list

# Decrypt and unpack gpg keys, sign and install into website/
ccat --envvar FLATPAK_KEY ../ci/gpg.tar.gz.cpt > gpg.tar.gz
tar xf gpg.tar.gz
chmod 500 opencpn-gpg
make -f ../flatpak/Makefile install
make GPG_HOMEDIR=opencpn-gpg -f ../flatpak/Makefile sign
rm -rf gpg.tar.gz opencpn-gpg

# Debug: show version in local repo.
flatpak remote-add  \
    --user --gpg-import=website/opencpn.key local $PWD/website/repo
flatpak update --appstream local
flatpak remote-ls local

# Deploy website/ to deployment server.
cp ../ci/id_opencpn.tar.cpt .
ccdecrypt --envvar FLATPAK_KEY id_opencpn.tar.cpt
tar -xf id_opencpn.tar
chmod 600 .ssh/id_opencpn

rsync -a --info=stats --delete-after \
    --rsh="ssh -o 'StrictHostKeyChecking no' -i .ssh/id_opencpn" \
    website/ opencpn@mumin.crabdance.com:/var/www/ocpn-flatpak/website
rm -f .ssh/id_opencpn*

# Restore the patched file so the caching works.
git checkout ../flatpak/org.opencpn.OpenCPN.yaml

# Debug: show version in remote repo.
flatpak remote-add --user opencpn $PWD/website/opencpn.flatpakrepo
flatpak update --appstream opencpn
flatpak remote-ls opencpn

# Validate the appstream data:
appstreamcli validate app/files/share/metainfo/opencpn.appdata.xml
