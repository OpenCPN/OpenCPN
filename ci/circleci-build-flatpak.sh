#!/usr/bin/env bash

#
# Build the travis flatpak artifacts.
#

# bailout on errors and echo commands.
set -xe
if [ -z "$FLATPAK_KEY" ]; then
    echo "Reguired \$FLATPAK_KEY not found, giving up"
    exit 1
fi

set -xe

# The flatpak manifest is setup to build the master branch. If we are
# on another branch, make it match the manifest. However, unless
# FP_BUILD_ORIGINAL_BRANCH is set, this is not used anyway since the
# default master branch from main github repo is used then.

current_branch=$(git rev-parse --abbrev-ref HEAD)
if [ "$current_branch" != "master" ]; then
    git branch -m $current_branch master
fi

wget -q -O - https://dl.google.com/linux/linux_signing_key.pub \
    | sudo apt-key add -
sudo apt-key adv \
    --keyserver keyserver.ubuntu.com --recv-keys 78BD65473CB3BD13

sudo add-apt-repository -y ppa:alexlarsson/flatpak
sudo apt update -y

# Avoid using outdated TLS certificates, see #2419.
sudo apt install --reinstall  ca-certificates

# Install required packages
sudo apt install -q -y appstream flatpak flatpak-builder git ccrypt make rsync gnupg2


# Set up flatpak
runtime=$(sed -n '/runtime-version/s/.*://p' flatpak/org.opencpn.OpenCPN.yaml)
runtime=${runtime/ /}
flatpak --user remote-add --if-not-exists \
    flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak --user install --noninteractive org.freedesktop.Platform//$runtime
flatpak --user install --noninteractive org.freedesktop.Sdk//$runtime

cd flatpak

# By default, script packages master branch from main github repo, as a
# proper packaging branch should do. Setting FP_BUILD_ORIGINAL_BRANCH
# makes it build same branch as the script instead, for testing.
if [ -z "$FP_BUILD_ORIGINAL_BRANCH" ]; then
    sed -i -e '/url:/s|\.\.|https://github.com/OpenCPN/OpenCPN.git|' \
        -e "/BUILD_NUMBER/s/0/$BUILD_NUMBER/" \
        org.opencpn.OpenCPN.yaml
fi

# The build heavy lifting
test -d ../build || mkdir ../build
cd ../build
make -f ../flatpak/Makefile build
flatpak list

# Decrypt and unpack gpg keys, sign and install into website/
ccat --envvar FLATPAK_KEY ../ci/gpg.tar.gz.cpt > gpg.tar.gz
tar xf gpg.tar.gz
chmod 700 opencpn-gpg
make -f ../flatpak/Makefile install
make GPG_HOMEDIR=opencpn-gpg -f ../flatpak/Makefile sign
rm -rf gpg.tar.gz opencpn-gpg

# Deploy website/ to deployment server.
cp ../ci/id_opencpn.tar.cpt .
ccdecrypt --envvar FLATPAK_KEY id_opencpn.tar.cpt
tar -xf id_opencpn.tar
chmod 600 .ssh/id_opencpn
rsync -a --info=stats --delete-after \
    --rsh="ssh -o 'StrictHostKeyChecking no' -i .ssh/id_opencpn" \
    website/  opencpnci@ocpnci.kalian.cz:web/flatpak-repo
rm -f .ssh/id_opencpn*


# Restore the patched file so the caching works.
git checkout ../flatpak/org.opencpn.OpenCPN.yaml

# Debug: show version in local repo.
flatpak remote-add  \
    --user --gpg-import=website/opencpn.key local $PWD/website/repo
flatpak update --appstream local
flatpak remote-ls local

# Validate the appstream data:
appstreamcli validate app/files/share/appdata/org.opencpn.OpenCPN.appdata.xml || :
