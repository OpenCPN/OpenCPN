/* Copyright 2000, 2001 William McClain

    This file is part of Astrolabe.

    Astrolabe is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Astrolabe is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Astrolabe; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    */

/* Global values.

These can be set directly, or there is a routine astrolabe.util.load_params()
which will assign them based on values in a parameter text file.

*/

#include "astrolabe.hpp"

using std::string;

/*
Abbreviation for standard timezone (e.g., "CST" for North American 
Central Standard Time)
*/
string astrolabe::globals::standard_timezone_name = "UT";

/*
Time in fractional days to be subtracted from UT to calculate the standard
time zone offset. Locations east of Greenwich should use negative values.
*/
double astrolabe::globals::standard_timezone_offset = 0.0;   

/*
Abbreviation for daylight savings timezone (e.g., "CDT" for North American 
Central Daylight Time)

This is optional. If set to None, no daylight savings conversions 
will be performed.
*/
string astrolabe::globals::daylight_timezone_name;         

/*
Time in fractional days to be subtracted from UT to calculate the daylight savings
time zone offset. Locations east of Greenwich should use negative values.

This value is not used unless "daylight_timezone_name" has an value other
than None.
*/
double astrolabe::globals::daylight_timezone_offset = 0.0;

/*
Observer's longitude in radians, measured positive west of Greenwich, 
negative to the east. Should be between -pi...pi.
*/
double astrolabe::globals::longitude = 0.0;

/*
Observer's latitude in radians, measured positive north of the equator, 
negative to the south. Should be between -pi/2...pi/2.
*/
double astrolabe::globals::latitude = 0.0;

/*
The full path name of the VSOP87D text data file, eg:

   /home/wmcclain/astrolabe/data/vsop87d.txt

This value is not required unless the vsop87d module is used.
*/
string astrolabe::globals::vsop87d_text_path;

/*
The full path name of the VSOP87D binary data file, eg:

   /home/wmcclain/astrolabe/data/vsop87d.dat

This value is not required unless the vsop87d module is used. If the value
is not defined or the file is not readable, the VSOP87d() class init method 
will use vsop87d_text_path instead.
*/
string astrolabe::globals::vsop87d_binary_path;

