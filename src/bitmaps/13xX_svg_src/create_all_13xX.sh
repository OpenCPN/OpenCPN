#!/bin/bash

path=$(dirname $0)
output=$1

#convert to png
for pic in $(ls $path/*prj.svg) $path/redX.svg
do
  echo "converting $pic"
  inkscape --without-gui --export-png=$output/$(basename $pic .svg ).png --export-dpi=72 --export-background-opacity=0 --export-width=13 --export-height=13 $pic >/dev/null
done

for pic in $(ls $path/*viz.svg)
do
  echo "converting $pic"
  inkscape --without-gui --export-png=$output/$(basename $pic .svg ).png --export-dpi=72 --export-background-opacity=0 --export-width=20 --export-height=13 $pic >/dev/null
done

