/********************************************************************
** @source JEEPS input functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @modified Copyright (C) 2004, 2005, 2006 Robert Lipe
** @modified Copyright (C) 2007 Achim Schumacher
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301,  USA.
********************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gpsinput_h
#define gpsinput_h


#include "garmin_gps.h"

int32  GPS_Input_Get_Almanac(GPS_PAlmanac **alm, FILE *inf);
int32  GPS_Input_Get_Waypoint(GPS_PWay **way, FILE *inf);
int32  GPS_Input_Get_Proximity(GPS_PWay **way, FILE *inf);
int32  GPS_Input_Get_Track(GPS_PTrack **trk, FILE *inf);
int32  GPS_Input_Get_Route(GPS_PWay **way, FILE *inf);


#endif

#ifdef __cplusplus
}
#endif
