#!/bin/sh

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
    cloudsmith push raw --republish --no-wait-for-sync \
        alec-leamas/opencpn $new
    set +x
fi

if [ -z "$ssh_key" ]; then
    echo 'Cannot deploy to sourceforge: missing $ssh_key'
    exit 0
fi

<<<<<<< HEAD
# TODO: Upload .lib and .pdb files as well.
=======
# encode command:
# openssl enc -aes-256-cbc -d -pbkdf2 -in opencpn_rsa.enc -out opencpn_rsa -k pw
cd ${APPVEYOR_BUILD_FOLDER}/ci
openssl enc -md sha256 -a -aes-256-cbc -d -in opencpn_rsa.enc -out opencpn_rsa -k ${ssh_key}

ssh_opts="-o StrictHostKeyChecking=no -i opencpn_rsa"
ssh_host="alec_leamas@frs.sourceforge.net"

dest="/home/frs/project"
dest="$dest/opencpn-devel/5.0.0-post/msvc"

set -x
exe=../build/*.exe
pdb=../build/RelWithDebInfo/opencpn.pdb
lib=../build/RelWithDebInfo/opencpn.lib
exe_base=$(basename $exe)
exe_base=$(echo $exe_base | sed 's/+[^_]*_/\./')
lib_base=$(basename $lib)
pdb_base=$(basename $pdb)
build_ix=$((APPVEYOR_BUILD_NUMBER % 5))
commit=$(git rev-parse --short=7 HEAD)
buildinfo="$build_ix-is-$APPVEYOR_BUILD_NUMBER-$commit"
echo "$buildinfo" > $buildinfo
scp $ssh_opts $exe $ssh_host:$dest/$build_ix-$exe_base
scp $ssh_opts $pdb $ssh_host:$dest/$build_ix-$pdb_base
scp $ssh_opts $lib $ssh_host:$dest/$build_ix-$lib_base
scp $ssh_opts $buildinfo $ssh_host:$dest/$buildinfo
rm opencpn_rsa
>>>>>>> ci: Add cloudsmith deployment, circleci builds + fixes.
