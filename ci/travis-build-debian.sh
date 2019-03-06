#!/usr/bin/env bash

#
# Build the Travis Debian artifacts
#
set -xe
sudo apt-get -qq update

sudo apt-get --allow-unauthenticated install \
  gettext                  \
  libarchive-dev           \
  libcairo2-dev            \
  libcurl4-openssl-dev     \
  libelf-dev               \
  libexif-dev              \
  libexpat1-dev            \
  libglu1-mesa-dev         \
  libgps-dev               \
  libgtk2.0-dev libbz2-dev \
  liblzma-dev              \
  libportaudio2            \
  libsqlite3-dev           \
  libtinyxml-dev           \
  libwxgtk3.0-dev          \
  $WX_PKG_NAME             \
  portaudio19-dev

if [[ "$USE_WEBVIEW" == "ON" ]]; then 
    sudo apt-get --allow-unauthenticated install \
        libwxgtk-webview3.0-dev \
        libgtk-3-dev
fi

mkdir  build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -sj2
make package
