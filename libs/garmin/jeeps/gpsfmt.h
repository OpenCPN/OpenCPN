/********************************************************************
** @source JEEPS formatting functions
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

#ifndef gpsfmt_h
#define gpsfmt_h


#include "garmin_gps.h"
#include <stdio.h>
#include <time.h>

void   GPS_Fmt_Print_Time(time_t Time, FILE *outf);
void   GPS_Fmt_Print_Position(double lat, double lon, FILE *outf);
void   GPS_Fmt_Print_Pvt(GPS_PPvt_Data pvt, FILE *outf);
void   GPS_Fmt_Print_Almanac(GPS_PAlmanac *alm, int32 n, FILE *outf);
void   GPS_Fmt_Print_Track(GPS_PTrack *trk, int32 n, FILE *outf);
int32  GPS_Fmt_Print_Waypoint(GPS_PWay *way, int32 n, FILE *outf);
int32  GPS_Fmt_Print_Proximity(GPS_PWay *way, int32 n, FILE *outf);
int32  GPS_Fmt_Print_Route(GPS_PWay *way, int32 n, FILE *outf);

#endif

#ifdef __cplusplus
}
#endif
