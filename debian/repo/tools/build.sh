#!/bin/bash
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
 cat << "EOF"

test -d /tmp/pkg-amd64 || mkdir /tmp/pkg-amd64
# Initial run:
podman run -v $HOME/opencpn-gpg:/root/.gnupg --platform linux/amd64 \
     -v=$PWD:/build -v=/tmp/pkg-amd64:/output -e DOCKER_BUILD=1 \
     --name debian_13-amd64 -ti debian:trixie-backports /bin/bash
# Subsequent runs:
podman start debian_13-amd64
podman cp ~/.devscripts debian_13-amd64:/root
podman exec -ti debian_13-amd64 /bin/bash
# /build/build.sh


test -d /tmp/pkg-aarch64 || mkdir /tmp/pkg-aarch64
# Initial run:
podman run -v $HOME/opencpn-gpg:/root/.gnupg --platform linux/aarch64 \
     -v=$PWD:/build -v=/tmp/pkg-aarch64:/output -e DOCKER_BUILD=1 \
     --name debian_13-aarch64 -ti debian:trixie-backports /bin/bash
# Subsequent runs:
podman start debian_13-aarch64
podman cp ~/.devscripts debian_13-aarch64:/root
podman exec -ti debian_13-aarch64 /bin/bash
# /build/build.sh

EOF
  exit 0
fi

# Install build tools
apt update --allow-unauthenticated 
apt install -y devscripts equivs git-buildpackage

# Clone opencpn
cd /build
mkdir opencpn
cd opencpn/
git clone -b debian/trixie-backports https://github.com/opencpn/opencpn.git
cd opencpn/
git fetch origin pristine-tar:pristine-tar
git fetch origin upstream/latest:upstream/latest

# Install build deps
mk-build-deps 
apt install -y ./*deb

# Build
export DEBSIGN_KEYID=build@opencpn.org
git clean -fxd; git checkout .
gbp buildpackage --git-upstream-tag=upstream/5.9.1-beta1+dfsg

# Copy out results to /output
cd ..
cp *.deb *.buildinfo *.build *.changes *.dsc /output
