/*
    Serial operations.

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

 */

#include "gpsdevice.h"
#include "gpsserial.h"
#include "gpsread.h"

gps_device_ops  gps_serial_ops = {
	GPS_Serial_On,
	GPS_Serial_Off,
	GPS_Serial_Chars_Ready,
	GPS_Serial_Wait,
	GPS_Serial_Flush,
	GPS_Serial_Send_Ack,
	GPS_Serial_Get_Ack,
	GPS_Serial_Packet_Read,
	GPS_Serial_Write_Packet,
};
