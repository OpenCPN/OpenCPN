#!/bin/sh

if [ -z "$CIRCLECI_BUILD_NUM" ]; then
    echo 'Cannot set build number, no $CIRCLECI_BUILD_NUM found'
    exit 0
fi

test -f "opencpn.appdata.xml" || {
    echo 'Cannot set build number, no opencpn.appdata.xml found'
    exit 0
}

sed -i "/<release /s/+/-$CIRCLECI_BUILD_NUM+/" opencpn.appdata.xml
