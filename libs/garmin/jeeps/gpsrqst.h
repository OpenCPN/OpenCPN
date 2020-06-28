#ifndef gpsrqst_h
#define gpsrqst_h


#include "gps.h"

  int32 GPS_Rqst_Send_Time(gpsdevh* fd, time_t Time);
  int32 GPS_Rqst_Send_Position(gpsdevh* fd, double lat, double lon);


#endif
