#!/usr/bin/env bash

#
# Build the Travis Debian artifacts
#
set -xe
sudo apt-get -qq update
sudo apt-get install -q devscripts equivs

mk-build-deps ./ci/control --install --root-cmd=sudo --remove
sudo apt-get --allow-unauthenticated install -f

# Ancient cmake cannot handle multi-line block comments:
sed -i '/#\[\[/,/#\]\]/{//!d}' CMakeLists.txt

# Trusty finds webview header but not the library:
if [ "$OCPN_TARGET" = "trusty" ]; then \
    WEBVIEW_OPT="-DOCPN_USE_WEBVIEW:BOOL=OFF"
fi

rm -rf build && mkdir build && cd build
cmake $WEBVIEW_OPT \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DOCPN_CI_BUILD:BOOL=ON \
    ..
make -sj2
make package

sudo apt-get install python3-pip python3-setuptools
