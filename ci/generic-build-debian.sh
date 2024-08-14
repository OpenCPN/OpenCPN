#!/usr/bin/env bash

#
# Build the Debian artifacts
#
set -xe
src_tree_root="$(dirname $(readlink -f $0))/.."
sudo apt-get -qq update
sudo apt-get install --yes --force-yes -q devscripts equivs

mk-build-deps "${src_tree_root}/ci/control" \
    --install \
    --root-cmd=sudo \
    --remove \
    --tool="apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes --force-yes"
sudo apt-get --allow-unauthenticated --yes --force-yes install -f

pushd "${src_tree_root}"
rm -rf build && mkdir build && cd build
cmake $WEBVIEW_OPT  $EXTRA_BUILD_OPTS\
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DOCPN_CI_BUILD:BOOL=ON \
    -DOCPN_USE_BUNDLED_LIBS=OFF \
    ..
make -sj2
dbus-run-session make run-tests || :
popd

sudo apt-get --yes --force-yes install python3-pip python3-setuptools
