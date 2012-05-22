/********************************************************************
** @source JEEPS time/position request from GPS functions
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
#include "garmin_gps.h"


static int32 GPS_A600_Rqst(gpsdevh *fd, time_t Time);
static int32 GPS_A700_Rqst(gpsdevh *fd, double lat, double lon);



/* @func GPS_Rqst_Send_Time ******************************************
**
** Set GPS time on request of GPS
**
** @param [r] fd [int32] file descriptor
** @param [r] Time [time_t] unix-style time
**
** @return [int32] true if OK
************************************************************************/

int32 GPS_Rqst_Send_Time(gpsdevh *fd, time_t Time)
{
    time_t ret=0;

    switch(gps_date_time_transfer)
    {
    case pA600:
	ret = GPS_A600_Rqst(fd, Time);
	break;
    default:
	GPS_Error("Rqst_Send_Time: Unknown date/time protocol");
	return PROTOCOL_ERROR;
    }

    return (int32)ret;
}



/* @funcstatic GPS_A600_Rqst *******************************************
**
** Send time to GPS
**
** @param [r] fd [int32] file descriptor
** @param [r] Time [time_t] unix-style time
**
** @return [int32] success
************************************************************************/
static int32 GPS_A600_Rqst(gpsdevh *fd, time_t Time)
{
    GPS_PPacket tra;
    GPS_PPacket rec;

    if(!(tra = GPS_Packet_New()) || !(rec = GPS_Packet_New()))
	return MEMORY_ERROR;


    switch(gps_date_time_type)
    {
    case pD600:
	GPS_D600_Send(&tra,Time);
	break;
    default:
	GPS_Error("A600_Rqst: Unknown data/time protocol");
	return PROTOCOL_ERROR;
    }

    if(!GPS_Write_Packet(fd,tra))
	return gps_errno;
    if(!GPS_Get_Ack(fd, &tra, &rec))
	return gps_errno;

    GPS_Packet_Del(&tra);
    GPS_Packet_Del(&rec);

    return 1;
}



/* @func GPS_Rqst_Send_Position ******************************************
**
** Set GPS position
**
** @param [r] fd [int32] filedescriptor
** @param [r] lat [double] latitude  (deg)
** @param [r] lon [double] longitude (deg)
**
** @return [int32] success
************************************************************************/

int32 GPS_Rqst_Send_Position(gpsdevh *fd, double lat, double lon)
{
    int32 ret=0;

    switch(gps_position_transfer)
    {
    case pA700:
	ret = GPS_A700_Rqst(fd, lat, lon);
	break;
    default:
	GPS_Error("Rqst_Send_Position: Unknown position protocol");
	return PROTOCOL_ERROR;
    }

    return ret;
}



/* @funcstatic GPS_A700_Rqst *******************************************
**
** Send position to GPS
**
** @param [r] fd [int32] file descriptor
** @param [r] lat [double] latitude  (deg)
** @param [r] lon [double] longitute (deg)
**
** @return [int32] success
************************************************************************/
static int32 GPS_A700_Rqst(gpsdevh *fd, double lat, double lon)
{
    GPS_PPacket tra;
    GPS_PPacket rec;

    if(!(tra = GPS_Packet_New()) || !(rec = GPS_Packet_New()))
	return MEMORY_ERROR;


    switch(gps_position_type)
    {
    case pD700:
	GPS_D700_Send(&tra,lat,lon);
	break;
    default:
	GPS_Error("A700_Rqst: Unknown position protocol");
	GPS_Packet_Del(&tra);
	GPS_Packet_Del(&rec);
	return PROTOCOL_ERROR;
    }

    if(!GPS_Write_Packet(fd,tra))
	return gps_errno;

    if(!GPS_Get_Ack(fd, &tra, &rec))
	return gps_errno;


    GPS_Packet_Del(&tra);
    GPS_Packet_Del(&rec);

    return 1;
}

