#!/bin/bash
#
# Handle substitution of VERSION_GIT_DATE and VERSION_GIT_COMMIT
#
# Usage filter_date [-i srcfile] <smudge|clean>
#
# Arguments
#     smudge  Use actual commit dates in VERSION.cmake
#     clean   Use %git_date% and %git_stamp% keywords in VERSION.cmake
# Options
#     -i      Update srcfile in place rather than work as a filter.

function filter {
    if [ "$1" = "clean" ]; then
        sed -e 's/GIT_DATE.*/GIT_DATE %git_date%)/' \
            -e 's/GIT_STAMP.*/GIT_STAMP %git_stamp%)/'
    elif [ "$1" = "smudge" ]; then
        sed -e "s/%git_date%/$(git log -1 --pretty=format:%cs)/" \
            -e "s/%git_stamp%/$(git log -1 --pretty=format:%ct)/"
    else
        echo 'Illegal or missing <clean|smudge> argument' >&2
        exit 1
    fi
}

if [ "$1" = "-i" ]; then
    shift
    version_cmake=$1
    shift
    tmpfile=$(mktemp)
    filter $1 < $version_cmake > $tmpfile \
        && cp $tmpfile $version_cmake && rm $tmpfile
else
    filter $1
fi
