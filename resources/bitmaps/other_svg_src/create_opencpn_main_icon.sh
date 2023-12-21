#!/bin/bash
#/******************************************************************************
# *
# * Project:  OpenCPN
# * Purpose:  helper shell script
# * Author:   David Register
# *
# ***************************************************************************
# *   Copyright (C) 2010 by David S. Register   *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; if not, write to the                         *
# *   Free Software Foundation, Inc.,                                       *
# *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
# ***************************************************************************
# */

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

