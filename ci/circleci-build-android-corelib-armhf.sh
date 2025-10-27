#!/bin/sh  -xe

#
# Build the Android artifacts inside the circleci linux container
#
set -xe

# Update possibly outdated google package signing key
curl https://packages.cloud.google.com/apt/doc/apt-key.gpg \
    | sudo apt-key add -

sudo apt -q update
sudo apt install cmake git gettext

# Install cloudsmith-cli (for upload) and cryptography (for git-push)
sudo apt install python3-pip
python3 -m pip install --user --force-reinstall -q pip setuptools
sudo apt remove python3-six python3-colorama python3-urllib3
export LC_ALL=C.UTF-8  LANG=C.UTF-8
python3 -m pip install --user cloudsmith-cli cryptography

# Build corelib
builddir=build-android-hf
test -d $builddir || mkdir $builddir
cd $builddir && rm -rf *

cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DOCPN_TARGET_TUPLE:STRING="Android-armhf;16;armhf" \
  -DOCPN_BUILD_SAMPLE=ON \
  -Dtool_base="$HOME/android-sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64"\
  -DCMAKE_SYSTEM_NAME=Android \
  -DCMAKE_SYSTEM_VERSION=26 \
  -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
  ..

#  This step should not be necessary, but works in CCI to prove the build
#  TODO:  Figure out why this is necessary.
make lunasvg

#  Build target OpenCPN library.
make VERBOSE=1

# Make sure that the upload script finds the files
cd ..; mv  $builddir build
