/********************************************************************
** @source JEEPS command functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @modified Copyright (C) 2005, 2006 Robert Lipe
** @modified Copyright (C) 2007 Achim Schumacher
** @modified Copyright (C) 2010 Martin Buck
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

#ifndef gpscom_h
#define gpscom_h


#include "garmin_gps.h"
#include <time.h>

int32  GPS_Command_Off(const char *port);

time_t GPS_Command_Get_Time(const char *port);
int32  GPS_Command_Send_Time(const char *port, time_t Time);

int32  GPS_Command_Get_Position(const char *port, double *lat, double *lon);
int32  GPS_Command_Send_Position(const char *port, double lat, double lon);

int32  GPS_Command_Pvt_On(const char *port, gpsdevh **fd);
int32  GPS_Command_Pvt_Off(const char *port, gpsdevh **fd);
int32  GPS_Command_Pvt_Get(gpsdevh **fd, GPS_PPvt_Data *pvt);

int32  GPS_Command_Get_Almanac(const char *port, GPS_PAlmanac **alm);
int32  GPS_Command_Send_Almanac(const char *port, GPS_PAlmanac *alm, int32 n);

int32  GPS_Command_Get_Track(const char *port, GPS_PTrack **trk, int (*cb)(int, struct GPS_SWay **));
int32  GPS_Command_Send_Track(const char *port, GPS_PTrack *trk, int32 n);

int32  GPS_Command_Get_Waypoint(const char *port, GPS_PWay **way,int (*cb)(int, struct GPS_SWay **));
int32  GPS_Command_Send_Waypoint(const char *port, GPS_PWay *way, int32 n, int (*cb)(struct GPS_SWay **));

int32  GPS_Command_Get_Proximity(const char *port, GPS_PWay **way);
int32  GPS_Command_Send_Proximity(const char *port, GPS_PWay *way, int32 n);

int32  GPS_Command_Get_Route(const char *port, GPS_PWay **way);
int32  GPS_Command_Send_Route(const char *port, GPS_PWay *way, int32 n);

int32  GPS_Command_Get_Lap(const char *port, GPS_PLap **lap, int (*cb)(int, struct GPS_SWay **));

int32  GPS_Command_Send_Course(const char *port, GPS_PCourse *crs, GPS_PCourse_Lap *clp,
                               GPS_PTrack *trk, GPS_PCourse_Point *cpt,
                               int32 n_crs, int32 n_clp, int32 n_trk, int32 n_cpt);
int32  GPS_Command_Send_Track_As_Course(const char *port, GPS_PTrack *trk, int32 n_trk,
                                        GPS_PWay *wpt, int32 n_wpt);

int32  GPS_Command_Get_Workout(const char *port, void **lap, int (*cb)(int, struct GPS_SWay **));
int32  GPS_Command_Get_Fitness_User_Profile(const char *port, void **lap, int (*cb)(int, struct GPS_SWay **));
int32  GPS_Command_Get_Workout_Limits(const char *port, void **lap, int (*cb)(int, struct GPS_SWay **));
int32  GPS_Command_Get_Course_Limits(const char *port, void **lap, int (*cb)(int, struct GPS_SWay **));
#endif

#ifdef __cplusplus
}
#endif
