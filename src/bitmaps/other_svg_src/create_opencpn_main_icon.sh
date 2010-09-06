#!/bin/bash

path=$(dirname $0)
output=$1

echo creating PNG icon
#convert to png
inkscape --without-gui --export-png=$path/opencpn_64x64.png --export-dpi=72 --export-background=rgb\(255,255,255\) --export-width=64 --export-height=64 $path/opencpn.svg >/dev/null
inkscape --without-gui --export-png=$path/opencpn_48x48.png --export-dpi=72 --export-background=rgb\(255,255,255\) --export-width=48 --export-height=48 $path/opencpn.svg >/dev/null
inkscape --without-gui --export-png=$path/opencpn_32x32.png --export-dpi=72 --export-background=rgb\(255,255,255\) --export-width=32 --export-height=32 $path/opencpn.svg >/dev/null
inkscape --without-gui --export-png=$path/opencpn_16x16.png --export-dpi=72 --export-background=rgb\(255,255,255\) --export-width=16 --export-height=16 $path/opencpn.svg >/dev/null

echo creating XPM icon
#convert to xpm
convert $path/opencpn_32x32.png $path/opencpn.xpm
#chang static char to static const char
sed 's/static char/static const char/' <$path/opencpn.xpm >$output/opencpn.xpm

echo creating ICO icon
#create ico
icotool -c -o $output/opencpn.ico $path/opencpn_64x64.png $path/opencpn_48x48.png $path/opencpn_32x32.png $path/opencpn_16x16.png

mv $path/opencpn_48x48.png $output/opencpn.png
#mv $path/opencpn_16x16.png $output/frameicon.png

rm $path/opencpn.xpm $path/opencpn_64x64.png $path/opencpn_32x32.png $path/opencpn_16x16.png

