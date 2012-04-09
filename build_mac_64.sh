#!/bin/bash
#
#
if [[ ! -d build_osx ]];then
	mkdir build_osx
fi
cd build_osx
CC=`wx-config --cc` CXX=`wx-config --cxx` cmake .. && make
#cmake -DCMAKE_OSX_ARCHITECTURES=i386 .. && make
