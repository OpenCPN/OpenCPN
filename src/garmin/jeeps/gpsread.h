#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gpsread_h
#define gpsread_h


#include "garmin_gps.h"

time_t GPS_Time_Now(void);
int32  GPS_Serial_Packet_Read(gpsdevh *fd, GPS_PPacket *packet);
int32  GPS_Serial_Get_Ack(gpsdevh *fd, GPS_PPacket *tra, GPS_PPacket *rec);

#endif

#ifdef __cplusplus
}
#endif
