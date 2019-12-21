#!/usr/bin/env bash

#
# Build the OSX artifacts 
#

# bailout on errors and echo commands
set -xe

# ruby needs to see libyaml when installed, so:
curl -fsSL http://pyyaml.org/download/libyaml/yaml-0.2.2.tar.gz \
    > yaml-0.2.2.tar.gz
tar xf yaml-0.2.2.tar.gz
cd yaml-0.2.2
./configure
make
sudo make install
cd .. 
rm -rf yaml-0.2.2 yaml-0.2.2.tar.gz

# Fix broken ruby on the CircleCI image:
if [ -n "$CI" ]; then
    curl -fsSL \
        https://raw.githubusercontent.com/Homebrew/install/master/uninstall \
        > uninstall
    chmod 755 uninstall
    ./uninstall -f
    inst="https://raw.githubusercontent.com/Homebrew/install/master/install"
    /usr/bin/ruby -e "$(curl -fsSL $inst)"
fi

set -o pipefail
for pkg in cairo cmake libexif python3 wget xz; do
    brew list $pkg 2>/dev/null | head -10 || brew install $pkg
done

export MACOSX_DEPLOYMENT_TARGET=10.9

# We need to build own libarchive
wget -q https://libarchive.org/downloads/libarchive-3.3.3.tar.gz
tar zxf libarchive-3.3.3.tar.gz
cd libarchive-3.3.3
./configure --without-lzo2 --without-nettle --without-xml2 --without-openssl --with-expat
make
make install
cd ..

wget -q http://opencpn.navnux.org/build_deps/wx312_opencpn50_macos109.tar.xz
tar xJf wx312_opencpn50_macos109.tar.xz -C /tmp
export PATH="/usr/local/opt/gettext/bin:$PATH"
echo 'export PATH="/usr/local/opt/gettext/bin:$PATH"' >> ~/.bash_profile

mkdir build
cd build
test -n "$TRAVIS_TAG" && CI_BUILD=OFF || CI_BUILD=ON
cmake -DOCPN_CI_BUILD=$CI_BUILD \
  -DOCPN_USE_LIBCPP=ON \
  -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx312_opencpn50_macos109/bin/wx-config \
  -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx312_opencpn50_macos109" \
  -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 \
  ..
make -sj2
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/SharedSupport/plugins
sudo chmod 644 /usr/local/lib/lib*.dylib
make install
make install # Dunno why the second is needed but it is, otherwise
             # plugin data is not included in the bundle

make create-dmg

# install the stuff needed by upload.
sudo -H python3 -m ensurepip
sudo -H python3 -m pip install -q setuptools
sudo -H python3 -m pip install -q cloudsmith-cli
