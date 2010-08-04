/*
    Definitions for Garmin USB protocol and implementation.

    Copyright (C) 2004, 2005, 2006 Robert Lipe, robertlipe@usa.net

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
#include <stdio.h>

/* This structure is a bit funny looking to avoid variable length
 * arrays which aren't present in C89.   This contains the visible
 * fields in the USB packets of the Garmin USB receivers (60C, 76C, etc.)
 * All data are little endian. 
 */
typedef 
union {
	struct {
	unsigned char type;
	unsigned char reserved1;
	unsigned char reserved2;
	unsigned char reserved3;
	unsigned char pkt_id[2];
	unsigned char reserved6;
	unsigned char reserved7;
	unsigned char datasz[4];
	unsigned char databuf[1]; /* actually an variable length array... */
	} gusb_pkt;
	unsigned char dbuf[1024];
} garmin_usb_packet;

/*
 * Internal interfaces that are common regardless of underlying 
 * OS implementation. 
 */
#define GUSB_MAX_UNITS 20
struct garmin_unit_info {
	unsigned long serial_number;
	unsigned long unit_id;
	unsigned long unit_version;
	char *os_identifier; /* In case the OS has another name for it. */
	char *product_identifier; /* From the hardware itself. */
} garmin_unit_info[GUSB_MAX_UNITS];

int gusb_cmd_send(const garmin_usb_packet *obuf, size_t sz);
int gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz);
int gusb_init(const char *portname, gpsdevh **dh);
int gusb_close(gpsdevh *);

/*
 * New packet types in USB.
 */
#define GUSB_SESSION_START 5	/* We request units attention */
#define GUSB_SESSION_ACK   6	/* Unit responds that we have its attention */ 
#define GUSB_REQUEST_BULK  2	/* Unit requests we read from bulk pipe */
