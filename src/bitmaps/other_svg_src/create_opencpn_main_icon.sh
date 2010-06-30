#!/bin/bash

#convert to png
inkscape --without-gui --export-png=opencpn_64x64.png --export-dpi=72 --export-background=rgb\(255,255,255\) --export-width=64 --export-height=64 opencpn.svg &> /dev/null
inkscape --without-gui --export-png=opencpn_48x48.png --export-dpi=72 --export-background=rgb\(255,255,255\) --export-width=48 --export-height=48 opencpn.svg &> /dev/null
inkscape --without-gui --export-png=opencpn_32x32.png --export-dpi=72 --export-background=rgb\(255,255,255\) --export-width=32 --export-height=32 opencpn.svg &> /dev/null
inkscape --without-gui --export-png=opencpn_16x16.png --export-dpi=72 --export-background=rgb\(255,255,255\) --export-width=16 --export-height=16 opencpn.svg &> /dev/null


#convert to xpm
convert opencpn_32x32.png opencpn.xpm

#create ico
icotool -c -o ../opencpn.ico opencpn_64x64.png opencpn_48x48.png opencpn_32x32.png opencpn_16x16.png

#chang static char to static const char
sed 's/static char/static const char/' <opencpn.xpm> post_opencpn.xpm

#clean up
for F in post_*
do
	mv $F ${F:5}
done
mv opencpn_48x48.png ../opencpn.png
rm *.png
mv *.xpm ../