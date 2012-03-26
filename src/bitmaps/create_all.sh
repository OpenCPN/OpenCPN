#!/bin/bash
#/******************************************************************************
# * $Id: *.sh,v 1.8 2010/06/21 01:54:37 bdbcat Exp $
# *
# * Project:  OpenCPN
# * Purpose:  helper shell script
# * Author:   David Register
# *
# ***************************************************************************
# *   Copyright (C) 2010 by David S. Register   *
# *   bdbcat@yahoo.com   *
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
# *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
# ***************************************************************************
# */

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

