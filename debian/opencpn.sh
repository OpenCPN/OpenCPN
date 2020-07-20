#!/bin/sh

machine=$(uname -m)
if [ "$machine" != "x86_64" ]; then
    exec /usr/libexec/opencpn
fi

release=$(lsb_release -sc)
case $release in

    sid) TARGET='ubuntu-gtk3:20.04'
        ;;
    buster)
        exec /usr/libexec/opencpn
        ;;
    stretch) TARGET='ubuntu:16.04'
        ;;
    *)  echo "Version $release is not supported" >&2
        exit 2
        ;;
esac
OPENCPN_COMPAT_TARGET=$TARGET exec /usr/libexec/opencpn
