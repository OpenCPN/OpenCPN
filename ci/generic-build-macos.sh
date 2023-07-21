#!/usr/bin/env bash

#
# Build the OSX artifacts
#


# OpenCPN needs to support even older macOS releases than the system
# it is being built on, set MACOSX_DEPLOYMENT_TARGET environment
# variable to use the respective SDK version.
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



# Build libarchive, for legacy compatibility.
# The libarchive version from Homebrew is built with the newest SDK
# relevant for the machine we build on and is not compatible with
# older macOS versions, we need to build our own against the older SDK
# Note: For local builds it is completely OK to build against any other version of libarchive,
# for example installed from Homebrew, just keep in mind that such products should not be distributed
# as they will cause interoperability problems with plugins
if [ ! -f libarchive-3.3.3.tar.gz ]; then
    # Download only if we do not have the archive yet
    curl -k -o libarchive-3.3.3.tar.gz  \
        https://libarchive.org/downloads/libarchive-3.3.3.tar.gz
fi
tar zxf libarchive-3.3.3.tar.gz
cd libarchive-3.3.3
#if [ "$(shasum /usr/local/lib/libarchive.13.dylib | cut -d' ' -f1)" != "$(shasum .libs/libarchive.13.dylib | cut -d' ' -f1)" ]; then
    # Build only if we didn't before
    ./configure --without-lzo2 --without-nettle --without-xml2 --without-openssl --with-expat
    make
    # install to /usr/local
    sudo rm -f /usr/local/include/archive.h
    sudo rm -f /usr/local/include/archive_entry.h
    sudo make install
#fi
cd ..

# Install the build dependencies for OpenCPN
brew install cmake
brew install gettext
brew install lame
brew install lz4
brew install mpg123
brew install xz
brew install zstd

for pkg in openssl python3  cmake ; do
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

# OpenCPN can be built against any wxWidgets version newer than 3.2.0
# but the resulting binary will (almost certainly) not be ABI compatible with 3rd party plugins
# For convenience and to save build time, we distribute a bundled build of wxWidgets.
# Note: For local builds it is completely OK to build against any other version of wxWidgets 3.2,
# for example installed from Homebrew, just keep in mind that such products should not be distributed
# as they will cause interoperability problems with plugins
if [ ! -f /tmp/wx322-2_opencpn50_macos1010.tar.bz2 ]; then
    # Download only if we do not have the archive yet
    curl -k -o /tmp/wx322-2_opencpn50_macos1010.tar.bz2  \
        https://download.opencpn.org/s/8xYPFAqTR8ZGXXb/download
fi
tar -C /tmp -xJf /tmp/wx322-2_opencpn50_macos1010.tar.bz2

export PATH="/usr/local/opt/gettext/bin:$PATH"
echo 'export PATH="/usr/local/opt/gettext/bin:$PATH"' >> ~/.bash_profile

# Build, install and make package
mkdir -p build
cd build
test -n "$TRAVIS_TAG" && CI_BUILD=OFF || CI_BUILD=ON
# Configure the build
cmake -DOCPN_CI_BUILD=$CI_BUILD \
  -DOCPN_VERBOSE=ON \
  -DOCPN_USE_LIBCPP=ON \
  -DOCPN_USE_SYSTEM_LIBARCHIVE=OFF \
  -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx322-2_opencpn50_macos1010/lib/wx/config/osx_cocoa-unicode-3.2 \
  -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx322-2_opencpn50_macos1010" \
  -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 \
  -DOCPN_RELEASE=0 \
  -DOCPN_BUILD_TEST=OFF \
  ..
# Compile OpenCPN
make -sj$(sysctl -n hw.physicalcpu)
# Create the package artifacts
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/SharedSupport/plugins
make install
make install # Dunno why the second is needed but it is, otherwise
             # plugin data is not included in the bundle

dsymutil -o OpenCPN.dSYM /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS/OpenCPN
tar czf OpenCPN-$(git rev-parse --short HEAD).dSYM.tar.gz OpenCPN.dSYM

make create-pkg
make create-dmg

# Install the stuff needed for upload to the Cloudsmith repository
pip3 install --user  -q cloudsmith-cli
