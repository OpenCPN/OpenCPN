#ifndef gpssend_h
#define gpssend_h


#include "gps.h"

#define GPS_ARB_LEN 1024

  int32  GPS_Serial_Write_Packet(gpsdevh* fd, GPS_PPacket& packet);
  bool  GPS_Serial_Send_Ack(gpsdevh* fd, GPS_PPacket* tra, GPS_PPacket* rec);

  void   GPS_Make_Packet(GPS_PPacket* packet, US type, UC* data, uint32 n);


#endif
