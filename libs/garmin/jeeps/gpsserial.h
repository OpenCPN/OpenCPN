/********************************************************************
** @source JEEPS serial port low level functions
**
** @author Copyright (C) 1999,2000 Alan Bleasby
** @version 1.0
** @modified December 28th 1999 Alan Bleasby. First version
** @modified June 29th 2000 Alan Bleasby. NMEA additions
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

#ifndef gpsserial_h
#define gpsserial_h


#include "garmin_gps.h"

#define usecDELAY 180000	/* Microseconds before GPS sends A001 */

int32  GPS_Serial_Chars_Ready(gpsdevh * fd);
// int32  GPS_Serial_Close(int32 fd, const char *port);
// int32  GPS_Serial_Open(int32 *fd, const char *port);
// int32  GPS_Serial_Open_NMEA(int32 *fd, const char *port);
// int32  GPS_Serial_Restoretty(const char *port);
// int32  GPS_Serial_Savetty(const char *port);
int32  GPS_Serial_On(const char *port, gpsdevh **fd);
int32  GPS_Serial_Off(gpsdevh *fd);
int32  GPS_Serial_Wait(gpsdevh *fd);
int32  GPS_Serial_Flush(gpsdevh *fd);
// int32  GPS_Serial_On_NMEA(const char *port, gpsdevh **fd);
int32  GPS_Serial_Read(gpsdevh *fd, void *ibuf, int size);
int32  GPS_Serial_Write(gpsdevh *fd, const void *obuf, int size);
int32  GPS_Serial_Write_Packet(gpsdevh *fd, GPS_PPacket packet);
int32  GPS_Serial_Send_Ack(gpsdevh *fd, GPS_PPacket *tra, GPS_PPacket *rec);
void   GPS_Serial_Error(const char *hdr, ...);

void VerifySerialPortClosed();
int Garmin_Serial_GPS_PVT_On( const char *port_name );
int Garmin_Serial_GPS_PVT_Off( const char *port_name );
int GPS_Serial_Command_Pvt_Get(GPS_PPvt_Data *pvt );

#endif

#ifdef __cplusplus
}
#endif
