#!/bin/bash

path=$(dirname $0)

# Require icotool from package icoutils
# Require convert from package ImageMagick
# Require inkscape

##for pic in $path/aa1chartnavhelper.svg $path/aa1chartnavhelper_pi.svg
##do
##  echo "converting $pic"
##  inkscape --without-gui --export-png=$path/$(basename $pic .svg ).png --export-dpi=72 --export-background-opacity=0 --export-width=32 --export-height=32 $pic >/dev/null
##done

# AA10272017: Requires png2wx, I havd included the png files... so you dont have to regenerate the icons.cpp and .h files

$path/../../../src/bitmaps/png2wx.pl -C $path/icons.cpp -H $path/icons.h -M ICONS_H $path/*.png

rm $path/*.png

