#ifndef gpsserial_h
#define gpsserial_h


#include "gps.h"

#define usecDELAY 180000	/* Microseconds before GPS sends A001 */
#define DEFAULT_BAUD 9600

  int32  GPS_Serial_Chars_Ready(gpsdevh* fd);
// int32  GPS_Serial_Close(int32 fd, const char *port);
// int32  GPS_Serial_Open(int32 *fd, const char *port);
// int32  GPS_Serial_Open_NMEA(int32 *fd, const char *port);
// int32  GPS_Serial_Restoretty(const char *port);
// int32  GPS_Serial_Savetty(const char *port);
  int32  GPS_Serial_On(const char* port, gpsdevh** fd);
  int32  GPS_Serial_Off(gpsdevh* fd);
  int32  GPS_Serial_Wait(gpsdevh* fd);
  int32  GPS_Serial_Flush(gpsdevh* fd);
// int32  GPS_Serial_On_NMEA(const char *port, gpsdevh **fd);
  int32  GPS_Serial_Read(gpsdevh* fd, void* ibuf, int size);
  int32  GPS_Serial_Write(gpsdevh* fd, const void* obuf, int size);


int32  GPS_Serial_Set_Baud_Rate(gpsdevh* fd, int br);

#endif
