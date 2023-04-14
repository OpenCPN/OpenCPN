#!/bin/sh
#
# This script is used by GitHub to install the dependencies
# before building wxWidgets but can also be run by hand if necessary (but
# currently it only works for Ubuntu versions used by the CI builds).

set -e -x

# Install packaged dependencies
here=$(cd "$(dirname "$0")"; pwd)
for pkg in $(sed '/#/d' < $here/macos-deps);  do
    brew list --versions $pkg || brew install $pkg || brew install $pkg || :
    brew link --overwrite $pkg || brew install $pkg
done

if [ ${USE_HOMEBREW:-0} -ne 1 ]; then
    # Install the pre-built wxWidgets package
    wget -q https://download.opencpn.org/s/MCiRiq4fJcKD56r/download \
        -O /tmp/wx315_opencpn50_macos1010.tar.xz
    tar -C /tmp -xJf /tmp/wx315_opencpn50_macos1010.tar.xz
else
    brew update
    brew install wxwidgets
fi
