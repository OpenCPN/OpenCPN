#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gpsrqst_h
#define gpsrqst_h


#include "garmin_gps.h"

int32 GPS_Rqst_Send_Time(gpsdevh *fd, time_t Time);
int32 GPS_Rqst_Send_Position(gpsdevh *fd, double lat, double lon);


#endif

#ifdef __cplusplus
}
#endif
