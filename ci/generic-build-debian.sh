#!/usr/bin/env bash

#
# Build the Debian artifacts
#
set -xe
src_tree_root="$(dirname $(readlink -f $0))/.."
sudo apt-get -qq update
sudo apt-get install --yes --force-yes -q devscripts equivs

if [[ $(lsb_release -rs) == 20.04 ]]; then
    # Focal needs a more recent cmake
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc \
        2>/dev/null | \
        gpg --dearmor - | \
        sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' \
        | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
    sudo apt update
    sudo apt-get install kitware-archive-keyring
fi
pushd "${src_tree_root}"
if [[ -n "$PACKAGE_BRANCH" && -z "$CIRCLE_PR_NUMBER" ]]; then
    # Initiate git environment
    git config --global user.name "OpenCPN auto builds"
    git config --global user.email "opencpn-builds@nowhere.net"
    git remote add home https://github.com/OpenCPN/OpenCPN.git
    git remote update home

    # Checkout debian/ dir from package branch
    git fetch home $PACKAGE_BRANCH:$PACKAGE_BRANCH
    git checkout $PACKAGE_BRANCH debian
    git commit -m "Add debian packaging dir"

    # Install build deps
    mk-build-deps --install --root-cmd=sudo --remove \
        --tool="apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes --force-yes"
    sudo apt-get --allow-unauthenticated --yes --force-yes install -f

    # Create a debian changelog entry
    export DEBEMAIL=opencpn-builds@nowhere.net
    export DEBFULLNAME="OpenCPN nightly builds"
    version="1:5.11.0~$(date "+%Y%m%d%H%m").$(git rev-parse --short HEAD)-1"
    dch --distribution ${PACKAGE_BRANCH##*/} -v "$version" "Auto build"
    git add debian/changelog
    git commit -m "d/changelog: $version"

    # Create orig tarball and work in the sources it provides
    git archive --prefix opencpn-5.11.0/ --output ../opencpn.tar.gz HEAD
    mk-origtargz ../opencpn.tar.gz
    (cd ..; tar xf *xz)
    cd ../opencpn-5.11.0*

    # Build package and move artifacts to expected build/ dir
    debuild -us -uc -j4
    test -d ../project/build || mkdir ../project/build
    mv ../*deb ../project/build
else
    mk-build-deps "${src_tree_root}/ci/control" \
        --install \
        --root-cmd=sudo \
        --remove \
        --tool="apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes --force-yes"
    sudo apt-get --allow-unauthenticated --yes --force-yes install -f

    rm -rf build && mkdir build && cd build
    cmake $WEBVIEW_OPT  $EXTRA_BUILD_OPTS\
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DOCPN_CI_BUILD:BOOL=ON \
        -DOCPN_DISTRO_BUILD=ON \
        -DOCPN_USE_BUNDLED_LIBS=OFF \
        -DOCPN_BUILD_SAMPLE=ON \
        ..
    make -sj2
    dbus-run-session make run-tests || :
fi
cd ..
sudo apt-get --yes --force-yes install python3-pip python3-setuptools
popd
