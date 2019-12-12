/*
    Abstraction of underlying device types.

    Copyright (C) 2006 Robert Lipe, robertlipe@usa.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA

 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gpsdevice_h
#define gpsdevice_h

typedef struct gpsdevh gpsdevh;

#include "garmin_gps.h"

#define usecDELAY 180000	/* Microseconds before GPS sends A001 */


int32  GPS_Device_Chars_Ready(gpsdevh *fd);
int32  GPS_Device_On(const char *port, gpsdevh **fd);
int32  GPS_Device_Off(gpsdevh *fd);
int32  GPS_Device_Wait(gpsdevh * fd);
int32  GPS_Device_Flush(gpsdevh * fd);
int32  GPS_Device_Read(int32 ignored, void *ibuf, int size);
int32  GPS_Device_Write(int32 ignored, const void *obuf, int size);
void   GPS_Device_Error(char *hdr, ...);
int32  GPS_Write_Packet(gpsdevh *fd, GPS_PPacket packet);
int32  GPS_Send_Ack(gpsdevh *fd, GPS_PPacket *tra, GPS_PPacket *rec);
int32  GPS_Packet_Read(gpsdevh *fd, GPS_PPacket *packet);
int32  GPS_Get_Ack(gpsdevh *fd, GPS_PPacket *tra, GPS_PPacket *rec);

typedef int32 (*gps_device_op)(gpsdevh *);
typedef int32 (*gps_device_op5)(const char *, gpsdevh **fd);
typedef int32 (*gps_device_op10)(gpsdevh * fd,  GPS_PPacket *tra, GPS_PPacket *rec);
typedef int32 (*gps_device_op12)(gpsdevh * fd, GPS_PPacket packet);
typedef int32 (*gps_device_op13)(gpsdevh * fd, GPS_PPacket *packet);
typedef struct {
	gps_device_op5 Device_On;
	gps_device_op Device_Off;
	gps_device_op Device_Chars_Ready;
	gps_device_op Device_Wait;
	gps_device_op Device_Flush;
	gps_device_op10 Send_Ack;
	gps_device_op10 Get_Ack;
	gps_device_op13 Read_Packet;
	gps_device_op12 Write_Packet;
} gps_device_ops;

#endif /* gpsdevice.h */

#ifdef __cplusplus
}
#endif
