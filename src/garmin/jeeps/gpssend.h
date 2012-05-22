/********************************************************************
** @source JEEPS packet construction, sending and ack functions
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

#ifndef gpssend_h
#define gpssend_h


#include "garmin_gps.h"

#define GPS_ARB_LEN 1024

int32  GPS_Serial_Write_Packet(gpsdevh *fd, GPS_PPacket packet);
int32  GPS_Serial_Send_Ack(gpsdevh *fd, GPS_PPacket *tra, GPS_PPacket *rec);

void   GPS_Make_Packet(GPS_PPacket *packet, US type, UC *data, uint32 n);


#endif

#ifdef __cplusplus
}
#endif
