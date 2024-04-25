#!/bin/sh

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. || exit 1
make || exit 1

./ixwebsocket_unittest ${TEST}
