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
sudo dnf install -q -y flatpak-builder ccrypt make rsync gnupg2

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

# Deploy website/ to deployment server.
ccat --envvar FLATPAK_KEY ../ci/amazon-ec2.pem.cpt > amazon-ec2.pem
chmod 400 amazon-ec2.pem
rsync_host="ec2-user@ec2-18-219-5-218.us-east-2.compute.amazonaws.com"
rsync -a --info=stats --rsh="ssh -o 'StrictHostKeyChecking no' -i amazon-ec2.pem" \
    website/ $rsync_host:/var/www/ocpn-website/website
rm -f ../ci/amazon-ec2.pem

# Restore the patched file so the caching works.
git checkout ../flatpak/org.opencpn.OpenCPN.yaml
