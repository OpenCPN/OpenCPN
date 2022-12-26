#!/usr/bin/env bash

#
# Build the flatpak artifacts.
#
set -xe

# Install required packages
sudo apt update -q
sudo apt install -q -y appstream flatpak flatpak-builder git


# Set up flatpak
runtime=$(sed -n '/runtime-version/s/.*://p' flatpak/org.opencpn.OpenCPN.yaml)
runtime=${runtime/ /}
flatpak --user remote-add --if-not-exists \
    flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak --user install --noninteractive org.freedesktop.Platform//$runtime
flatpak --user install --noninteractive org.freedesktop.Sdk//$runtime

cd flatpak

# The build heavy lifting
test -d ../build || mkdir ../build
cd ../build
make -f ../flatpak/Makefile build
flatpak list

# Validate the appstream data:
appstreamcli validate \
    app/files/share/appdata/org.opencpn.OpenCPN.appdata.xml || :
