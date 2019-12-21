#!/bin/sh

REPO="david-register/opencpn-unstable"

if [ -z "$CLOUDSMITH_API_KEY" ]; then
    echo 'Cannot deploy to cloudsmith: missing $CLOUDSMITH_API_KEY'
else
    echo 'Deploying to cloudsmith'
    python3.6m -m ensurepip
    python3.6m -m pip install cloudsmith-cli
    sed -i '/#!/s|/usr|/c/cygwin|' /c/cygwin/bin/cloudsmith
    export LC_ALL=C.UTF-8
    export LANG=C.UTF-8
    set -x
    cd ${APPVEYOR_BUILD_FOLDER}/build
    old=$(ls *setup.exe)
    new=$(echo $old | sed 's/opencpn/opencpn-msvc/')
    new=$(echo $new | sed "s/+/+${APPVEYOR_BUILD_NUMBER}./")
    mv $old $new
    cloudsmith push raw --republish --no-wait-for-sync $REPO $new
    set +x
fi

# TODO: Upload .lib and .pdb files as well.

