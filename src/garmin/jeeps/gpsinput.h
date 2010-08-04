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
