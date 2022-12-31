#!/bin/sh

expand() { for arg in "$@"; do test -f $arg && echo $arg; done }

REPO=${CLOUDSMITH_REPO:-"david-register/opencpn-unstable"}

test -z "$TRAVIS_BUILD_DIR" || cd $TRAVIS_BUILD_DIR
cd build

case "$OCPN_TARGET" in
    xenial|trusty|bionic*|focal*)
        for src in $(expand *.deb); do
            old=$(basename $src)
            new=$(echo $old | sed "s/opencpn/opencpn-${OCPN_TARGET}/")
            sudo mv $old $new
            echo "Renaming $old to $new"
        done
        ;;
    macos)
        for src in $(expand *.dmg); do
            old=$(basename $src)
            new=$(echo $old | sed "s/rw\.//")
            sudo mv $old $new
            echo "Renaming $old to $new"
        done
        ;;
esac

if [ -n "$GITHUB_RUN_NUMBER" ]; then
    BUILD_NR="$GITHUB_RUN_NUMBER"
elif [ -n "$CIRCLE_BUILD_NUM" ]; then
    BUILD_NR="$CIRCLE_BUILD_NUM"
else
    BUILD_NR="1"
fi

if [ -z "$CLOUDSMITH_API_KEY" ]; then
    echo 'Cannot deploy to cloudsmith: missing $CLOUDSMITH_API_KEY'
else
    echo 'Deploying to cloudsmith'
    set -x
    if pyenv versions >/dev/null 2>&1; then
        pyenv versions
        pyenv global $(pyenv versions | tail -1)
        sudo -H python3 -m pip  install wheel
        sudo -H python3 -m pip install cloudsmith-cli
        pyenv rehash
    else
        # Assuming builders have installed python3 + pip
        sudo -H python3 -m pip install -q cloudsmith-cli
    fi
    for src in $(expand *.dmg *setup.exe *.deb *.pkg); do
        old=$(basename $src)
        new=$(echo $old | sed "s/+/+${BUILD_NR}./")
        if [ "$old" != "$new" ]; then sudo mv "$old" "$new"; fi
        cloudsmith push raw --republish --no-wait-for-sync $REPO $new
    done
    set +x
fi

exit 0
