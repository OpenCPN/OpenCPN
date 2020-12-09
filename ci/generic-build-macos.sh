#!/usr/bin/env bash

#
# Build the OSX artifacts 
#
set -xe

export MACOSX_DEPLOYMENT_TARGET=10.9

#
# Check if the cache is with us. If not, re-install brew.
brew list --versions libexif || {
    brew update-reset
    # As indicated by warning message in CircleCI build log:
    git -C "/usr/local/Homebrew/Library/Taps/homebrew/homebrew-core" \
        fetch --unshallow
    git -C "/usr/local/Homebrew/Library/Taps/homebrew/homebrew-cask" \
        fetch --unshallow
}

for pkg in cairo cmake libarchive libexif pixman python3 wget xz; do
    brew list --versions $pkg || brew install $pkg || brew install $pkg || :
    brew link --overwrite $pkg || :
done

# Make sure cmake finds libarchive
version=$(brew list --versions libarchive | tail -1 | awk '{print $2}')
pushd /usr/local/include
    ln -sf /usr/local/Cellar/libarchive/$version/include/archive.h .
    ln -sf /usr/local/Cellar/libarchive/$version/include/archive_entry.h .
    cd ../lib
    ln -sf  /usr/local/Cellar/libarchive/$version/lib/libarchive.13.dylib .
    ln -sf  /usr/local/Cellar/libarchive/$version/lib/libarchive.dylib .
popd

brew list --cask --versions packages || brew install --cask packages

wget -q https://download.opencpn.org/s/rwoCNGzx6G34tbC/download \
    -O /tmp/wx312B_opencpn50_macos109.tar.xz
tar -C /tmp -xJf /tmp/wx312B_opencpn50_macos109.tar.xz 
export PATH="/usr/local/opt/gettext/bin:$PATH"
echo 'export PATH="/usr/local/opt/gettext/bin:$PATH"' >> ~/.bash_profile

# Build, install and make package
mkdir build
cd build
test -n "$TRAVIS_TAG" && CI_BUILD=OFF || CI_BUILD=ON
cmake -DOCPN_CI_BUILD=$CI_BUILD \
  -DOCPN_USE_LIBCPP=ON \
  -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx312B_opencpn50_macos109/bin/wx-config \
  -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx312B_opencpn50_macos109" \
  -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 \
  ..
make -sj$(sysctl -n hw.physicalcpu)
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/SharedSupport/plugins
make install
make install # Dunno why the second is needed but it is, otherwise
             # plugin data is not included in the bundle

make create-pkg

#make create-dmg

#test -z  "$BUILD_PKG" || make create-pkg

# Install the stuff needed by upload.
pip3 install -q cloudsmith-cli
