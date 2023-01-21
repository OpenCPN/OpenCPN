#!/usr/bin/env bash

#
# Build the OSX artifacts
#
set -xe

export MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-10.13}

# Return latest installed brew version of given package
pkg_version() { brew list --versions $2 $1 | tail -1 | awk '{print $2}'; }


# Check if the cache is with us. If not, re-install brew.
brew list --versions python3 || {
    brew update-reset
    # As indicated by warning message in CircleCI build log:
    #git -C "/usr/local/Homebrew/Library/Taps/homebrew/homebrew-core" \
        #fetch --unshallow
    #git -C "/usr/local/Homebrew/Library/Taps/homebrew/homebrew-cask" \
        #fetch --unshallow
}

#exit 0



# build libarchive, for legacy compatibility.
curl -k -o libarchive-3.3.3.tar.gz  \
    https://libarchive.org/downloads/libarchive-3.3.3.tar.gz
tar zxf libarchive-3.3.3.tar.gz
cd libarchive-3.3.3
./configure --without-lzo2 --without-nettle --without-xml2 --without-openssl --with-expat
# installs to /usr/local
sudo rm -f /usr/local/include/archive.h
sudo rm -f /usr/local/include/archive_entry.h
sudo make install
cd ..

# build libmpg123, for legacy compatibility.
git clone https://github.com/gypified/libmpg123.git
cd libmpg123
export MACOSX_DEPLOYMENT_TARGET=10.13
./configure --disable-debug --disable-dependency-tracking --prefix=/usr/local --with-module-suffix=.so --with-default-audio=coreaudio --with-cpu=x86-64
make
sudo make install
cd ..

# build libmp3lame, for legacy compatibility.
curl -k -o lame-3.100.tar.gz https://sourceforge.net/projects/lame/files/latest/download/lame-3.100.tar.gz -L
tar xf lame-3.100.tar.gz
cd lame-3.100
#inreplace "include/libmp3lame.sym", "lame_init_old\n", ""    // Brew formula command
sed -i.bak -e "/lame_init_old/d" include/libmp3lame.sym
export MACOSX_DEPLOYMENT_TARGET=10.13
./configure --disable-dependency-tracking --disable-debug --prefix=/usr/local --enable-nasm
make
sudo make install
cd ..

# build openssl, for legacy compatibility.
curl -k -o openssl-3.0.1.tar.gz  \
    https://github.com/openssl/openssl/archive/refs/tags/openssl-3.0.1.tar.gz -L
tar zxf openssl-3.0.1.tar.gz
cd openssl-openssl-3.0.1
export MACOSX_DEPLOYMENT_TARGET=10.13
./configure
make
# installs to /usr/local
sudo make install
cd ..


brew install cairo
brew install freetype
#brew install lame
brew install lz4
#brew install mpg123
brew install xz
brew install zstd

for pkg in python3  cmake ; do
    brew list --versions $pkg || brew install $pkg || brew install $pkg || :
    brew link --overwrite $pkg || :
done

if brew list --cask --versions packages; then
    version=$(pkg_version packages '--cask')
    sudo installer \
        -pkg /usr/local/Caskroom/packages/$version/packages/Packages.pkg \
        -target /
else
    brew install --cask packages
fi

curl -k -o /tmp/wx321_opencpn50_macos1010.tar.xz  \
    https://download.opencpn.org/s/Djqm4SXzYjF8nBw/download
tar -C /tmp -xJf /tmp/wx321_opencpn50_macos1010.tar.xz

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
  -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx321_opencpn50_macos1010/lib/wx/config/osx_cocoa-unicode-3.2 \
  -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx321_opencpn50_macos1010" \
  -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 \
  -DOCPN_BUILD_TEST=OFF \
  ..
make -sj$(sysctl -n hw.physicalcpu)
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/SharedSupport/plugins
make install
make install # Dunno why the second is needed but it is, otherwise
             # plugin data is not included in the bundle

make create-pkg
make create-dmg

# Install the stuff needed by upload.
pip3 install --user  -q cloudsmith-cli
