# docker run -v=$PWD:/build -v=/tmp:/output debian:bookworm-backports /bin/bash /build/build.sh

# Install build tools
apt update --allow-unauthenticated 
apt install -y devscripts equivs git-buildpackage

# Clone opencpn
mkdir opencpn
cd opencpn/
git clone -b debian/bookworm-backports https://gitlab.com/leamas/opencpn.git
cd opencpn/
git fetch origin pristine-tar:pristine-tar
git fetch origin upstream/latest:upstream/latest

# Install build deps
mk-build-deps 
apt install -y ./*deb

# Build
git clean -fxd; git checkout .
gbp buildpackage --git-upstream-tag=upstream/5.8.2+dfsg

# Copy out results to /output
cd ..
cp *.deb /output
