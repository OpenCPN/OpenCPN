#!/bin/sh

expand() { for arg in "$@"; do test -f $arg && echo $arg; done }

REPO="alec-leamas/opencpn"

test -z "$TRAVIS_BUILD_DIR" || cd $TRAVIS_BUILD_DIR
cd build

case "$OCPN_TARGET" in
    xenial|trusty) 
        for src in $(expand *.deb); do
            old=$(basename $src)
            new=$(echo $old | sed "s/opencpn/opencpn-${OCPN_TARGET}/")
            sudo mv $old $new
            echo "Renaming $old to $new"
        done
        ;;
    mingw)
        for src in $(expand *setup.exe); do
            old=$(basename $src)
            new=$(echo $old | sed "s/opencpn/opencpn-mingw/")
            sudo mv $old $new
            echo "Renaming $old to $new"
        done
        ;;
esac

if [ -z "$CLOUDSMITH_API_KEY" ]; then
    echo 'Cannot deploy to cloudsmith: missing $CLOUDSMITH_API_KEY'
else
    echo 'Deploying to cloudsmith'
    sudo python3 -m pip install -q cloudsmith-cli
    for src in $(expand *.dmg *setup.exe *.deb); do
        set -x
        old=$(basename $src)
        new=$(echo $old | sed "s/+/+${TRAVIS_BUILD_NUMBER}./")
        if [ "$old" != "$new" ]; then sudo mv "$old" "$new"; fi
        cloudsmith push raw --republish --no-wait-for-sync $REPO $new
        set +x
    done
fi

exit 0

if [ -z "$SSH_KEY" ]; then
    echo 'Cannot deploy to sourceforge: missing $SSH_KEY'
    exit 0
fi

cd $TRAVIS_BUILD_DIR/ci
openssl enc -a -md sha256 -aes-256-cbc -d -k $SSH_KEY \
  -in opencpn_rsa.enc -out opencpn_rsa
chmod 400 opencpn_rsa

ssh_opts="-o StrictHostKeyChecking=no -i opencpn_rsa"
ssh_host="alec_leamas@frs.sourceforge.net"

dest="/home/frs/project/opencpn-devel/5.0.0-post/"
build_ix=$((TRAVIS_BUILD_NUMBER % 5))
commit=$(git rev-parse --short=7 HEAD)
buildinfo="$build_ix-is-buildnr-$TRAVIS_BUILD_NUMBER-$commit"
echo "$buildinfo" > $buildinfo

set -x
dest="$dest/$OCPN_TARGET"
for src in $(expand ../build/*.dmg ../build/setup.exe ../build/*deb)
do
    src_base=$(basename $src)
    src_base=$(echo $src_base | sed 's/+[^\.]*\./\./')
    scp -B $ssh_opts $src $ssh_host:$dest/$build_ix-$src_base
    scp -B $ssh_opts $buildinfo $ssh_host:$dest/$buildinfo
done
