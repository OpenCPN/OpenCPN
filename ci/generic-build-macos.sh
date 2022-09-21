#!/usr/bin/env bash

#
# Build the OSX artifacts
#
set -xe

# Build for legacy Mac machines
export MACOSX_DEPLOYMENT_TARGET=10.10

# Required to build libcurl for legacy machines
export macosx_deployment_target=10.10

# allow shell to find Macports executable
export PATH=/opt/local/bin:$PATH

# allow caching of macports state in $HOME    "/Users/distiller/project/opt_local_cache"
sudo mkdir -p ${HOME}/project/opt_local_cache
sudo ln -s ${HOME}/project/opt_local_cache /opt/local

# curl -k -o /tmp/opt_macports.tar.xz  \
#     https://download.opencpn.org/s/FpPXeWqEif8cLCT/download
# sudo tar -C / -xJf /tmp/opt_macports.tar.xz

#ls ${HOME}/project/opt_local_cache || echo "OK"
#ls ${HOME}/project/opt_local_cache/bin || echo "OK"

#sudo mkdir -p /opt/local/share/curl
#sudo cp buildosx/cacert.pem /opt/local/share/curl/curl-ca-bundle.crt
#sudo mkdir -p /opt/local/etc/openssl
#sudo ln -s /opt/local/share/curl/curl-ca-bundle.crt /opt/local/etc/openssl/cert.pem

#openssl x509 -in /opt/local/share/curl/curl-ca-bundle.crt -out mycert.pem -outform PEM
#sudo security add-trusted-cert -d -r trustRoot -k /Library/Keychains/System.keychain "mycert.pem"
#sudo security add-trusted-cert -d -r trustRoot -k /Library/Keychains/System.keychain "buildosx/ISRGROOTX1.pem"

# Check if the cache is with us. If not, re-install macports
port info zstd || {
    curl -k -O https://distfiles.macports.org/MacPorts/MacPorts-2.7.1.tar.bz2
    tar xf MacPorts-2.7.1.tar.bz2
    cd MacPorts-2.7.1/
    ./configure
    make
    sudo make install
    cd ..

    sudo port -d selfupdate
}

#sudo port rev-upgrade

    # add our local ports to the sources.conf
#sudo cp buildosx/macports/sources.conf /opt/local/etc/macports

# rebuild the port index
pushd buildosx/macports/ports
#  portindex
popd

# Remove any leftover libcurl coming from earlier cached macports build
# rm /opt/local/lib/libcurl.4.dylib
#sudo rm /opt/local/lib/libcurl.a
#sudo rm -rf /opt/local/include/curl

# Install curl to get the TLS certificate bundle
##sudo port -q install curl
#sudo port -fN deactivate curl

#sudo port -fN deactivate openssl


# install the local port libraries
#  n.b.  ORDER IS IMPORTANT

#try non-local ports
#sudo port -q install OCPN_openssl
#sudo port -q install OCPN_libpixman

#sudo port -fq install OCPN_cairo

#sudo port -q install zstd

#sudo port -fN deactivate libarchive
#sudo port -q install OCPN_libarchive

#sudo port -q -f install OCPN_libpng

sudo port install libarchive

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

for pkg in python3  cmake ; do
    brew list --versions $pkg || brew install $pkg || brew install $pkg || :
    brew link --overwrite $pkg || :
done

# Make sure cmake finds libarchive
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

#wget -q https://download.opencpn.org/s/MCiRiq4fJcKD56r/download \
#    -O /tmp/wx315_opencpn50_macos1010.tar.xz

curl -k -o /tmp/wx315_opencpn50_macos1010.tar.xz  \
    https://download.opencpn.org/s/MCiRiq4fJcKD56r/download

tar -C /tmp -xJf /tmp/wx315_opencpn50_macos1010.tar.xz

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
  -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx315_opencpn50_macos1010/bin/wx-config \
  -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx315_opencpn50_macos1010" \
  -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 \
  ..
make -sj$(sysctl -n hw.physicalcpu)
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS
mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/SharedSupport/plugins
make install
make install # Dunno why the second is needed but it is, otherwise
             # plugin data is not included in the bundle

sudo ls -l /tmp/opencpn/bin/OpenCPN.app/Contents/Frameworks

make create-pkg
make create-dmg

# Install the stuff needed by upload.
pip3 install --user  -q cloudsmith-cli
