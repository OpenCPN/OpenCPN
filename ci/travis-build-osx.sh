#!/usr/bin/env bash

#
# Build the Travis OSX artifacts 
#

# bailout on errors and echo commands
set -xe

brew install cairo libexif xz
export MACOSX_DEPLOYMENT_TARGET=10.9
# We need to build own libarchive
wget https://libarchive.org/downloads/libarchive-3.3.3.tar.gz
tar zxf libarchive-3.3.3.tar.gz
cd libarchive-3.3.3
./configure --without-lzo2 --without-nettle --without-xml2 --without-openssl --with-expat
make
make install
cd ..

wget http://opencpn.navnux.org/build_deps/wx312_opencpn50_macos109.tar.xz
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
chmod 644 /usr/local/lib/lib*.dylib
make install
make install # Dunno why the second is needed but it is, otherwise
             # plugin data is not included in the bundle
make create-dmg

# Signing stuff
f="$(ls OpenCPN*.dmg)"
hdiutil attach -owners on "$f" -shadow
echo "Decrypting Cert"
openssl aes-256-cbc -K $encrypted_287837bd9398_key -iv $encrypted_287837bd9398_iv -in ../ci/Certificates.p12.enc -out Certificates.p12 -d
echo "Creating Keychain"
security create-keychain -p "$SECRET_PASSWORD" build.keychain
security default-keychain -s build.keychain
security unlock-keychain -p "$SECRET_PASSWORD" build.keychain
security set-keychain-settings -t 3600 -u build.keychain
echo "Importing certs into keychain"
security import ./Certificates.p12 -k build.keychain -T "$(which codesign)" -P "$SECRET_PASSWORD"
rm -f Certificates.p12
# macOS 10.12+
security set-key-partition-list -S apple-tool:,apple: -s -k "$SECRET_PASSWORD" build.keychain
echo "Signing Package"
codesign --deep --sign "Pavel Kalian - Code Signing" "/Volumes/OpenCPN Installer/OpenCPN.app"
security delete-keychain build.keychain;
hdiutil detach "/Volumes/OpenCPN Installer"
hdiutil convert -format UDZO -o new.dmg "$f" -shadow
mv new.dmg $f
