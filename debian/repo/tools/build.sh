# $ podman run -v $HOME/opencpn-gpg:/root/.gnupg --platform linux/amd64 \
#     -v=$PWD:/build -v=/tmp/pkg-amd64:/output -e DOCKER_BUILD=1 \
#      -ti debian:bookworm-backports
# # /build/build.sh
#
# $ podman run -v $HOME/opencpn-gpg:/root/.gnupg --platform linux/aarch64 \
#     -v=$PWD:/build -v=/tmp/pkg-aarch64:/output -e DOCKER_BUILD=1 \
#     -ti debian:bookworm-backports
# # /build/build.sh

# Install build tools
apt update --allow-unauthenticated 
apt install -y devscripts equivs git-buildpackage

# Clone opencpn
mkdir opencpn
cd opencpn/
git clone -b debian/bookworm-backports https://github.com/opencpn/opencpn.git
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
cp *.deb /output
