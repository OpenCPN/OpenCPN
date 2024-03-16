#!/usr/bin/env bash

set -e

#
# Build the OSX artifacts using dependencies from Homebrew and the OS to minimize impact on the environment
# The resulting build product is NOT ABI compatible with the upstream packages and plugins and should NEVER be distributed
# If you want to produce the upstream compatible build, refer to the universal-build-macos.sh script which pulls in the prebuilt dependencies required for universal binary build and compatible with older macOS releases
# The only prerequisites of the script are Homebrew and Xcode installed and working
#


# Return latest installed brew version of given package
pkg_version() { brew list --versions $2 $1 | tail -1 | awk '{print $2}'; }


# Check if the cache is with us. If not, re-install brew. This is needed in the CircleCI environment.
brew list --versions python3 || {
    brew update-reset
}

# Install the build dependencies for OpenCPN
brew install cmake
brew install gettext
brew install lame
brew install lz4
brew install mpg123
brew install xz
brew install zstd
brew install libarchive
brew install wxwidgets

ln -s /usr/local/opt/libarchive/include/archive.h /usr/local/include/archive.h
ln -s /usr/local/opt/libarchive/include/archive_entry.h /usr/local/include/archive_entry.h
ln -s /usr/local/opt/libarchive/lib/libarchive.13.dylib /usr/local/lib/libarchive.13.dylib

for pkg in openssl cmake ; do
    brew list --versions $pkg || brew install $pkg || brew install $pkg || :
    brew link --overwrite $pkg || :
done

# Build, install and make package
mkdir -p build
cd build
test -n "$TRAVIS_TAG" && CI_BUILD=OFF || CI_BUILD=ON
# Configure the build
cmake -DOCPN_CI_BUILD=$CI_BUILD \
  -DOCPN_VERBOSE=ON \
  -DOCPN_USE_SYSTEM_LIBARCHIVE=OFF \
  -DCMAKE_INSTALL_PREFIX=/tmp/opencpn \
  -DOCPN_RELEASE=0 \
  -DOCPN_BUILD_TEST=ON \
  ..

# Compile OpenCPN
make -sj$(sysctl -n hw.physicalcpu)

# Create the package artifacts
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/SharedSupport/plugins
make install
make install # Dunno why the second is needed but it is, otherwise
             # plugin data is not included in the bundle

make create-pkg
make create-dmg
