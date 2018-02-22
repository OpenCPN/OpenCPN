#!/bin/bash

path=$(dirname $0)

# Require icotool from package icoutils
# Require convert from package ImageMagick
# Require inkscape

$path/13xX_svg_src/create_all_13xX.sh $path
$path/16x16_svg_src/create_all_16x16.sh $path
$path/28x28_svg_src/create_all_28x28.sh $path
$path/32x32_svg_src/cursor/create_all_32x32.sh $path
$path/32x32_svg_src/ribbon/create_all_32x32.sh $path
$path/other_svg_src/create_opencpn_main_icon.sh $path
$path/other_svg_src/create_ship.sh $path

$path/png2wx.pl -C $path/icons.cpp -H $path/icons.h -M ICONS_H $path/*.png

