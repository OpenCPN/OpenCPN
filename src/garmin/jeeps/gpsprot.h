/********************************************************************
** @source JEEPS protocol table lookup functions (GPS' without A001)
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @modified Copyright (C) 2006 Robert Lipe
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

#ifndef gpsprotocols_h
#define gpsprotocols_h

#include "garmin_gps.h"

/*
 *  Link protocols
 */

struct LINKDATA
{
    US Pid_Protocol_Array;
    US Pid_Product_Rqst;
    US Pid_Product_Data;
    US Pid_Ext_Product_Data;

    US Pid_Ack_Byte;
    US Pid_Command_Data;
    US Pid_Xfer_Cmplt;
    US Pid_Date_Time_Data;
    US Pid_Position_Data;
    US Pid_Prx_Wpt_Data;
    US Pid_Nak_Byte;
    US Pid_Records;
    US Pid_Rte_Hdr;
    US Pid_Rte_Wpt_Data;
    US Pid_Almanac_Data;
    US Pid_Trk_Data;
    US Pid_Wpt_Data;
    US Pid_Pvt_Data;
    US Pid_Rte_Link_Data;
    US Pid_Trk_Hdr;

    US Pid_FlightBook_Record;
    US Pid_Lap;
    US Pid_Wpt_Cat;
    US Pid_Run;
    US Pid_Workout;
    US Pid_Workout_Occurrence;
    US Pid_Fitness_User_Profile;
    US Pid_Workout_Limits;
    US Pid_Course;
    US Pid_Course_Lap;
    US Pid_Course_Point;
    US Pid_Course_Trk_Hdr;
    US Pid_Course_Trk_Data;
    US Pid_Course_Limits;
    US Pid_Trk2_Hdr; /*Undocumented*/
};





/*
 * Command types
 */

#define pA010 10
#define pA011 11

int32 gps_device_command;


struct COMMANDDATA
{
    US Cmnd_Abort_Transfer;
    US Cmnd_Transfer_Alm;
    US Cmnd_Transfer_Posn;
    US Cmnd_Transfer_Prx;
    US Cmnd_Transfer_Rte;
    US Cmnd_Transfer_Time;
    US Cmnd_Transfer_Trk;
    US Cmnd_Transfer_Wpt;
    US Cmnd_Turn_Off_Pwr;
    US Cmnd_Start_Pvt_Data;
    US Cmnd_Stop_Pvt_Data;
    US Cmnd_FlightBook_Transfer;
    US Cmnd_Transfer_Laps;
    US Cmnd_Transfer_Wpt_Cats;
    US Cmnd_Transfer_Runs;
    US Cmnd_Transfer_Workouts;
    US Cmnd_Transfer_Workout_Occurrences;
    US Cmnd_Transfer_Fitness_User_Profile;
    US Cmnd_Transfer_Workout_Limits;
    US Cmnd_Transfer_Courses;
    US Cmnd_Transfer_Course_Laps;
    US Cmnd_Transfer_Course_Points;
    US Cmnd_Transfer_Course_Tracks;
    US Cmnd_Transfer_Course_Limits;
}
;




/*
 * Waypoint Transfer Protocol
 */
#define pA100 100
int32 gps_waypt_transfer;

/*
 * Waypoint category transfer protocol
 */
#define pA101 101
int32 gps_category_transfer;

/*
 * Route Transfer Protocol
 */
#define pA200 200
#define pA201 201
int32 gps_route_transfer;

/*
 * Track Log Transfer Protocol
 */
#define pA300 300
#define pA301 301
#define pA302 302
#define pA304 304
int32 gps_trk_transfer;

/*
 *  Proximity Waypoint Transfer Protocol
 */
#define pA400 400
int32 gps_prx_waypt_transfer;

/*
 *  Almanac Transfer Protocol
 */
#define pA500 500
int32 gps_almanac_transfer;


/*
 *  Date Time Transfer
 */
#define pA600 600
int32 gps_date_time_transfer;

/*
 *  FlightBook Transfer Protocol
 */
#define pA650 650
/*Not implemented */

/*
 *  Position
 */
#define pA700 700
int32 gps_position_transfer;


/*
 *  Pvt
 */
#define pA800 800
int32 gps_pvt_transfer;

/*
 * Lap Data Transfer
 */
#define pA906 906
int32 gps_lap_transfer;

/*
 * Various fitness related
 */
#define pA1000 1000
int32 gps_run_transfer;
#define pA1002 1002
int32 gps_workout_transfer;
#define pA1004 1004
int32 gps_user_profile_transfer;
#define pA1005 1005
int32 gps_workout_limits_transfer;
#define pA1006 1006
int32 gps_course_transfer;
#define pA1007 1007
int32 gps_course_lap_transfer;
#define pA1008 1008
int32 gps_course_point_transfer;
#define pA1009 1009
int32 gps_course_limits_transfer;
#define pA1012 1012
int32 gps_course_trk_transfer;

/*
 * Waypoint D Type
 */
#define pD100 100
#define pD101 101
#define pD102 102
#define pD103 103
#define pD104 104
#define pD105 105
#define pD106 106
#define pD107 107
#define pD108 108
#define pD109 109
#define pD110 110
#define pD150 150
#define pD151 151
#define pD152 152
#define pD154 154
#define pD155 155

int32 gps_rte_type;
int32 gps_waypt_type;

/*
 * Waypoint category types
 */
#define pD120 120
int32 gps_category_type;

/*
 * Rte Header Type
 */
#define pD200 200
#define pD201 201
#define pD202 202
int32 gps_rte_hdr_type;


/*
 * Rte Link Type
 */
#define pD210 210
int32 gps_rte_link_type;


/*
 *  Trk Point Type
 */
#define pD300 300
#define pD301 301
#define pD302 302
#define pD303 303
#define pD304 304
int32 gps_trk_type;
int32 gps_run_crs_trk_type;


/*
 *  Trk Header Type
 */
#define pD310 310
#define pD311 311
#define pD312 312
int32 gps_trk_hdr_type;
int32 gps_run_crs_trk_hdr_type;



/*
 * Prx Wpt Type
 */
#define pD400 400
#define pD403 403
#define pD450 450

int32 gps_prx_waypt_type;


/*
 * Almanac Type
 */
#define pD500 500
#define pD501 501
#define pD550 550
#define pD551 551

int32 gps_almanac_type;


/*
 * Date Time Type
 */
#define pD600 600

int32 gps_date_time_type;



/*
 * Position Type
 */
#define pD700 700

int32 gps_position_type;



/*
 * Pvt Data Type
 */
#define pD800 800

int32 gps_pvt_type;

/*
 * Lap Data Type
 */
#define pD906 906
#define pD1001 1001
#define pD1011 1011
#define pD1015 1015

int32 gps_lap_type;

/*
 * Various fitness related
 */
#define pD1000 1000
#define pD1009 1009
#define pD1010 1010
int32 gps_run_type;
#define pD1002 1002
#define pD1008 1008
int32 gps_workout_type;
#define pD1003 1003
int32 gps_workout_occurrence_type;
#define pD1004 1004
int32 gps_user_profile_type;
#define pD1005 1005
int32 gps_workout_limits_type;
#define pD1006 1006
int32 gps_course_type;
#define pD1007 1007
int32 gps_course_lap_type;
#define pD1012 1012
int32 gps_course_point_type;
#define pD1013 1013
int32 gps_course_limits_type;

/*
 * Link protocol type
 */
#define pL000 0
#define pL001 1
#define pL002 2

int32 gps_link_type;



struct GPS_MODEL_PROTOCOL
{
    US    id;
    int32 link;
    int32 command;
    int32 wayptt;
    int32 wayptd;
    int32 rtea;
    int32 rted0;
    int32 rted1;
    int32 trka;
    int32 trkd;
    int32 prxa;
    int32 prxd;
    int32 alma;
    int32 almd;
}
;

US     GPS_Protocol_Version_Change(US id, US version);
int32  GPS_Protocol_Table_Set(US id);
void   GPS_Protocol_Error(US tag, US data);
void   GPS_Unknown_Protocol_Print(void);


#endif

#ifdef __cplusplus
}
#endif
