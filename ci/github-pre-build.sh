#!/bin/sh
#
# This script is used by GitHub to install the dependencies
# before building wxWidgets but can also be run by hand if necessary (but
# currently it only works for Ubuntu versions used by the CI builds).

set -e -x

case $(uname -s) in
    Linux)
        if [ -f /etc/apt/sources.list ]; then
            # Show information about the repositories and priorities used.
	    sudo sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list
            echo 'APT sources used:'
            sudo grep --no-messages '^[^#]' /etc/apt/sources.list /etc/apt/sources.list.d/* || true
            echo '--- End of APT files dump ---'

            run_apt() {
                echo "-> Running apt-get $@"

                # Disable some (but not all) output.
                sudo apt-get -q -o=Dpkg::Use-Pty=0 "$@"

                rc=$?
                echo "-> Done with $rc"

                return $rc
            }

            run_apt update || echo 'Failed to update packages, but continuing nevertheless.'

            sudo apt-get -qq install devscripts equivs software-properties-common

            sudo mk-build-deps -ir ci/control
            sudo apt-get -q --allow-unauthenticated install -f
        fi
        ;;

    Darwin)
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
	;;
esac
