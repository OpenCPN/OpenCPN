#!/bin/bash

path=$(dirname $0)

# Require icotool from package icoutils
# Require convert from package ImageMagick
# Require inkscape

for pic in $path/dashboard.svg $path/dashboard_pi.svg
do
  echo "converting $pic"
  inkscape --without-gui --export-png=$path/$(basename $pic .svg ).png --export-dpi=72 --export-background-opacity=0 --export-width=32 --export-height=32 $pic >/dev/null
done

for pic in $path/instrument.svg $path/dial.svg $path/plus.svg $path/minus.svg
do
  echo "converting $pic"
  inkscape --without-gui --export-png=$path/$(basename $pic .svg ).png --export-dpi=72 --export-background-opacity=0 --export-width=20 --export-height=20 $pic >/dev/null
done

$path/../../../src/bitmaps/png2wx.pl -C $path/icons.cpp -H $path/icons.h -M ICONS_H $path/*.png

rm $path/*.png

