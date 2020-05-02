/********************************************************************
** @source JEEPS constructor and deconstructor functions
**
** @author Copyright (C) 1999,2000 Alan Bleasby
** @version 1.0
** @modified December 28th 1999 Alan Bleasby. First version
** @modified June 29th 2000 Alan Bleasby. NMEA additions
** @modified Copyright (C) 2006 Robert Lipe
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

#ifndef gpsmem_h
#define gpsmem_h


#include "garmin_gps.h"

GPS_PPacket       GPS_Packet_New(void);
void              GPS_Packet_Del(GPS_PPacket *thys);
GPS_PPvt_Data     GPS_Pvt_New(void);
void              GPS_Pvt_Del(GPS_PPvt_Data *thys);
GPS_PAlmanac      GPS_Almanac_New(void);
void              GPS_Almanac_Del(GPS_PAlmanac *thys);
GPS_PTrack        GPS_Track_New(void);
void              GPS_Track_Del(GPS_PTrack *thys);
GPS_PWay          GPS_Way_New(void);
void              GPS_Way_Del(GPS_PWay *thys);
GPS_PLap          GPS_Lap_New(void);
void              GPS_Lap_Del(GPS_PLap *thys);
GPS_PCourse       GPS_Course_New(void);
void              GPS_Course_Del(GPS_PCourse *thys);
GPS_PCourse_Lap   GPS_Course_Lap_New(void);
void              GPS_Course_Lap_Del(GPS_PCourse_Lap *thys);
GPS_PCourse_Point GPS_Course_Point_New(void);
void              GPS_Course_Point_Del(GPS_PCourse_Point *thys);

#endif

#ifdef __cplusplus
}
#endif
