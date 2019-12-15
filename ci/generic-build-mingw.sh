#!/bin/sh  -xe

#
# Build the mingw artifacts inside the Fedora container
#
set -xe

su -c "dnf install -y sudo dnf-plugins-core"
sudo dnf copr enable -y --nogpgcheck leamas/opencpn-mingw 

test -d /opencpn-ci && cd /opencpn-ci || :

if [ -n "$CIRCLECI" ]; then
    # horrible patch for circleci, does not work on travis despite same
    # cmake version.
    sed -i -e '/define NSIS_PACKEDVERSION/s/;!/!/'  NSIS.template.in.in
fi

sudo dnf builddep  -y mingw/fedora/opencpn-deps.spec
rm -rf build; mkdir build; cd build
cmake \
    -DCMAKE_TOOLCHAIN_FILE=../mingw/fedora/toolchain.cmake \
    -DOCPN_CI_BUILD:BOOL=ON \
    ..

make -j3
make package

sudo dnf install -y python3-pip python3-setuptools
sudo python3 -m pip install -q cloudsmith-cli
