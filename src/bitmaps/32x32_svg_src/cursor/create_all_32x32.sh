#!/bin/bash

path=$(dirname $0)
output=$1

#convert to png
for pic in $(ls $path/*.svg)
do
  echo -n "Processing $pic: PNG... "
  fn_png=$output/$(basename $pic .svg).png
  inkscape --without-gui --export-png=$fn_png --export-dpi=72 --export-background-opacity=0 --export-width=32 --export-height=32 $pic >/dev/null
#convert to xpm
  echo -n "XPM... "
  fn_xpm=$path/$(basename $fn_png .png).xpm
  convert $fn_png $fn_xpm
  echo -n "const fix... "
  sed 's/static char/static const char/' <$fn_xpm >$output/$(basename $fn_xpm)
  rm $fn_xpm
  echo "done."
done

for pic in default_pi
do
      rm $output/$pic.png
done
