#!/usr/bin/env bash

#
# Build the Travis Debian artifacts
#
set -xe
sudo apt-get -qq update
sudo apt-get install -q devscripts equivs

mk-build-deps ./ci/control --install --root-cmd=sudo --remove
sudo apt-get --allow-unauthenticated install -f

if [ "$OCPN_TARGET" = "jammy" ]; then
  sudo add-apt-repository -y ppa:leamas-alec/wxwidgets
  sudo apt update
fi

rm -rf build && mkdir build && cd build
cmake $WEBVIEW_OPT  $EXTRA_BUILD_OPTS\
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DOCPN_CI_BUILD:BOOL=ON \
    -DOCPN_USE_BUNDLED_LIBS=OFF \
    ..
make -sj2
make run-tests
make package

sudo apt-get install python3-pip python3-setuptools
