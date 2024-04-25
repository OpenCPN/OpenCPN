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
