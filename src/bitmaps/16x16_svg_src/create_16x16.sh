#!/bin/bash

curdir=$(pwd)'/'
icon=$1
if [ "$icon" == "" ]; then
  echo "Please specifiy a file to convert!"
  exit 0;
fi
icon=$curdir$icon
iconName=$(basename $icon)
iconDir=$(basename `dirname $icon`)
iconPngName=$( echo $iconName | cut -d . -f -1 )".png"
iconXpmName=$( echo $iconName | cut -d . -f -1 )".xpm"

#convert to png
inkscape --without-gui --export-png=$iconPngName --export-dpi=72 --export-background=rgb\(223,223,223\) --export-width=16 --export-height=16 $icon &> /dev/null

#convert to xpm
convert $iconPngName $iconXpmName

#chang static char to static const char
sed 's/static char/static const char/' <$iconXpmName> post_$iconXpmName

#clean up
for F in post_*
do
	mv $F ${F:5}
done
rm $iconPngName
mv $iconXpmName ../