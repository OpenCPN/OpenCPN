#!/bin/bash

path=$(dirname $0)

# Require icotool from package icoutils
# Require convert from package ImageMagick
# Require inkscape

# 32x32
for pic in $path/vdr_pi.svg $path/vdr_record.svg $path/vdr_play.svg
do
  echo "converting $pic"
  inkscape --without-gui --export-png=$path/$(basename $pic .svg ).png --export-dpi=72 --export-background-opacity=0 --export-width=32 --export-height=32 $pic >/dev/null
done

# 20x20
#for pic in 
#do
#  echo "converting $pic"
#  inkscape --without-gui --export-png=$path/$(basename $pic .svg ).png --export-dpi=72 --export-background-opacity=0 --export-width=20 --export-height=20 $pic >/dev/null
#done

$path/../../../src/bitmaps/png2wx.pl -C $path/icons.cpp -H $path/icons.h -M ICONS_H $path/*.png

rm $path/*.png

