/*
    Abstraction of underlying device types, serial or USB.  OS agnostic..

    Copyright (C) 2006 Robert Lipe, robertlipe@usa.net

    This program is free software{} you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation{} either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY{} without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program{} if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA

 */

#include "garmin_gps.h"
#include "gpsdevice.h"
#include "gpsserial.h"

extern gps_device_ops gps_serial_ops;
extern gps_device_ops gps_usb_ops;
gps_device_ops *ops = NULL;

int32  GPS_Device_On(const char *port, gpsdevh **fd)
{
	gps_is_usb = (0 == case_ignore_strncmp(port, "usb:", 4));

	if (gps_is_usb) {
		ops = &gps_usb_ops;
	} else {
		ops = &gps_serial_ops;
	}

	return (ops->Device_On)(port, fd);
}

int32  GPS_Device_Off(gpsdevh * fd)
{
      if(ops)
	     return (ops->Device_Off)(fd);
      else
            return -1;
}

int32  GPS_Device_Wait(gpsdevh * fd)
{
	return (ops->Device_Wait)(fd);
}

int32  GPS_Device_Chars_Ready(gpsdevh * fd)
{
	return (ops->Device_Chars_Ready)(fd);
}

int32  GPS_Device_Flush(gpsdevh * fd)
{
	return (ops->Device_Flush)(fd);
}

int32  GPS_Write_Packet(gpsdevh * fd, GPS_PPacket packet)
{
	return (ops->Write_Packet)(fd, packet);
}

int32 GPS_Packet_Read(gpsdevh * fd, GPS_PPacket *packet)
{
	return (ops->Read_Packet)(fd, packet);
}

int32 GPS_Send_Ack(gpsdevh * fd, GPS_PPacket *tra, GPS_PPacket *rec)
{
	return (ops->Send_Ack)(fd, tra, rec);
}

int32 GPS_Get_Ack(gpsdevh * fd, GPS_PPacket *tra, GPS_PPacket *rec)
{
	return (ops->Get_Ack)(fd, tra, rec);
}

void GPS_Make_Packet(GPS_PPacket *packet, US type, UC *data, uint32 n)
{
	(*packet)->type = type;
	memcpy((*packet)->data, data, n);
	(*packet)->n = n;
}
