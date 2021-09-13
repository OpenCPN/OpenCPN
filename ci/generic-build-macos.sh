#!/usr/bin/env bash

#
# Build the OSX artifacts
#
set -xe

export MACOSX_DEPLOYMENT_TARGET=10.9

# allow shell to find Macports executable
export PATH=/opt/local/bin:$PATH

# Check if the cache is with us. If not, re-install macports
port info OCPN_cairo || {
    curl -O https://distfiles.macports.org/MacPorts/MacPorts-2.7.1.tar.bz2
    tar xf MacPorts-2.7.1.tar.bz2
    cd MacPorts-2.7.1/
    ./configure
    make
    sudo make install
    cd ..
}

sudo port selfupdate

# add our local ports to the sources.conf
sudo cp buildosx/macports/sources.conf /opt/local/etc/macports

# rebuild the port index
#pushd buildosx/macports/ports
#  portindex
#popd

# install the local port libraries
#sudo port -q install OCPN_libpixman
#sudo port -q install OCPN_cairo
#sudo port -q install zstd
#sudo port -q install OCPN_libarchive
#sudo port -q install OCPN_curl
#sudo port -q install OCPN_openssl

# Return latest installed brew version of given package
pkg_version() { brew list --versions $2 $1 | tail -1 | awk '{print $2}'; }

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

for pkg in cmake python3 wget ; do
    brew list --versions $pkg || brew install $pkg || brew install $pkg || :
    brew link --overwrite $pkg || :
done

#for pkg in cairo cmake libarchive libexif pixman python3 wget xz; do
#    brew list --versions $pkg || brew install $pkg || brew install $pkg || :
#    brew link --overwrite $pkg || :
#done

# replace libcairo and some dependents
#wget -q https://www.dropbox.com/s/0egt1gz8oc9olmv/libcairo.2.dylib?dl=1 \
#    -O /tmp/libcairo.2.dylib
#cp /tmp/libcairo.2.dylib /tmp/libcairo.dylib

#wget -q https://www.dropbox.com/s/3nfroanhpln4hbk/libxcb-shm.0.0.0.dylib?dl=1 \
#    -O /tmp/libxcb-shm.0.0.0.dylib
#cp /tmp/libxcb-shm.0.0.0.dylib /tmp/libxcb-shm.0.dylib

#pushd /usr/local/lib
#    ln -sf  /tmp/libcairo.2.dylib .
#    ln -sf  /tmp/libcairo.dylib .

#    ln -sf  /tmp/libxcb-shm.0.0.0.dylib .
#    ln -sf  /tmp/libxcb-shm.0.dylib .
#popd


# Make sure cmake finds libarchive
#version=$(pkg_version libarchive)
#pushd /usr/local/include
#    ln -sf /usr/local/Cellar/libarchive/$version/include/archive.h .
#    ln -sf /usr/local/Cellar/libarchive/$version/include/archive_entry.h .
#    cd ../lib
#    ln -sf  /usr/local/Cellar/libarchive/$version/lib/libarchive.13.dylib .
#    ln -sf  /usr/local/Cellar/libarchive/$version/lib/libarchive.dylib .
#popd

pushd /usr/local/include
    ln -sf /opt/local/include/archive.h .
    ln -sf /opt/local/include/archive_entry.h .
    cd ../lib
    ln -sf  /opt/local/lib/libarchive.13.dylib .
    ln -sf  /opt/local/lib/libarchive.dylib .
popd

if brew list --cask --versions packages; then
    version=$(pkg_version packages '--cask')
    sudo installer \
        -pkg /usr/local/Caskroom/packages/$version/packages/Packages.pkg \
        -target /
else
    brew install --cask packages
fi


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
  -DOCPN_VERBOSE=ON \
  -DOCPN_USE_LIBCPP=ON \
  -DOCPN_USE_SYSTEM_LIBARCHIVE=OFF \
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

sudo ls -l /tmp/opencpn/bin/OpenCPN.app/Contents/Frameworks

#make create-pkg
make create-dmg

# Install the stuff needed by upload.
pip3 install -q cloudsmith-cli
