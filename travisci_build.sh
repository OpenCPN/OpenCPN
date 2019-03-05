#!/usr/bin/env bash

# bailout on errror
set -e

mkdir -p build && cd build
if [[ "$TRAVIS_OS_NAME" == "linux" ]];
  then cmake -DCMAKE_BUILD_TYPE=Debug ..
fi
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  if [ -z "$TRAVIS_TAG" ]; then
    # CI build, include commit ID
    cmake -DOCPN_CI_BUILD=ON -DOCPN_USE_LIBCPP=ON -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx312_opencpn50_macos109/bin/wx-config -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx312_opencpn50_macos109" -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 ..
  else
    # Build from tag, include the version number only
    cmake -DOCPN_CI_BUILD=OFF -DOCPN_USE_LIBCPP=ON -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx312_opencpn50_macos109/bin/wx-config -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx312_opencpn50_macos109" -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 ..
    fi
  fi
make -sj2
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS
  mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/SharedSupport/plugins
  chmod 644 /usr/local/lib/lib*.dylib
  make install
  make install # Dunno why the second is needed but it is, otherwise plugin data is not included in the bundle
  make create-dmg
fi
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  make package
fi
