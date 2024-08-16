#!/usr/bin/env bash

#
# Build the circleci flatpak artifacts.
#
set -xe
if [ -z "$FLATPAK_KEY" ]; then
    echo "Reguired \$FLATPAK_KEY not found, giving up"
    exit 1
fi

# The flatpak manifest is setup to build the master branch. If we are
# on another branch, make it match the manifest. However, unless
# FP_BUILD_ORIGINAL_BRANCH is set, this is not used anyway since the
# default master branch from main github repo is used then.
current_branch=$(git rev-parse --abbrev-ref HEAD)
if [ "$current_branch" != "master" ]; then
    git branch -m $current_branch master
fi

# Use most updated flatpak PPA
wget -q -O - https://dl.google.com/linux/linux_signing_key.pub \
    | sudo apt-key add -
sudo apt-key adv \
    --keyserver keyserver.ubuntu.com --recv-keys 78BD65473CB3BD13
# Needed on 20.04: sudo add-apt-repository -y ppa:alexlarsson/flatpak
sudo apt update -q -y

# Avoid using outdated TLS certificates, see #2419.
sudo apt install --reinstall  ca-certificates

# Install required packages
sudo apt install -q -y appstream flatpak flatpak-builder git ccrypt make rsync gnupg2

# Set up flatpak
git submodule update --init  flatpak/org.opencpn.OpenCPN
git submodule update --remote --merge flatpak/org.opencpn.OpenCPN
runtime=$(sed -n '/runtime-version/s/.*://p' \
          flatpak/org.opencpn.OpenCPN/org.opencpn.OpenCPN.yaml)
runtime=${runtime/ /}
runtime=${runtime%\"}
runtime=${runtime#\"}
flatpak --user remote-add --if-not-exists \
    flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak --user install --noninteractive org.freedesktop.Platform//$runtime
flatpak --user install --noninteractive org.freedesktop.Sdk//$runtime

cd flatpak

# The build heavy lifting
test -d ../build || mkdir ../build
cd ../build
make -f ../flatpak/Makefile ci-build
flatpak list

# Validate the appstream data:
appstreamcli validate app/files/share/appdata/org.opencpn.OpenCPN.appdata.xml || :

# build the single file bundle, the actual artifact.
flatpak build-bundle repo \
    opencpn+$(git rev-parse --short HEAD).flatpak org.opencpn.OpenCPN devel

# Make sure upload script has what it needs.
sudo apt-get --yes --force-yes install python3-pip python3-setuptools
