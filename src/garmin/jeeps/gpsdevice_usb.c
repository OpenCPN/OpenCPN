/*
    USB operations.

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

#include "gpsdevice.h"
#include "garminusb.h"
#include "gpsusbint.h"
#include "gpsusbcommon.h"

static int32 success_stub(void)
{
	return 1;
}

static int32 gdu_on(const char *port, gpsdevh **fd)
{
#ifdef __WIN32__
	return gusb_init(port, fd);
#else
      return -1;
#endif
}

static int32 gdu_off(gpsdevh *dh)
{
	return gusb_close(dh);
}

static int32  gdu_read(gpsdevh *fd, GPS_PPacket *packet)
{
	/* Default is to eat bulk request packets. */
	return GPS_Packet_Read_usb(fd, packet, 1);
}

gps_device_ops gps_usb_ops = {
	gdu_on,
	gdu_off,
	(gps_device_op) success_stub,
	(gps_device_op) success_stub,
	(gps_device_op) success_stub,
	(gps_device_op10) success_stub,
	(gps_device_op10) success_stub,
	gdu_read,
	GPS_Write_Packet_usb
};
