#!/usr/bin/env bash

#
# Build the Travis Debian artifacts
#
set -xe
sudo apt-get -qq update
sudo apt-get install devcscripts

mkdir  build
cd build
mk-build-deps ../ci/control
sudo dpkg -i *all.deb 
sudo apt-get --allow-unauthenticated install -f

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -sj2
make package
