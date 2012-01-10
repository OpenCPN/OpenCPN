#!/bin/bash

path=$(dirname $0)
output=$1

echo creating ship icon
#convert to png
inkscape --without-gui --export-png=$output/ship_red.png --export-dpi=72 --export-background-opacity=0 --export-width=19 --export-height=45 $path/ship_red.svg >/dev/null

