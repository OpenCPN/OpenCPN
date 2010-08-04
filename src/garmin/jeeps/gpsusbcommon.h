/*
    Garmin USB layer - OS independent component.

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

/*
 * The 'low level ops' are registered by the OS layer (win32, libusb, etc.)
 * to provide gruntwork features for the common USB layer.
 */
typedef int (*gusb_llop_get)(garmin_usb_packet *ibuf, size_t sz);
typedef int (*gusb_llop_send)(const garmin_usb_packet *opkt, size_t sz);
typedef int (*gusb_llop_close) (gpsdevh *dh);

typedef struct gusb_llops {
	gusb_llop_get  llop_get_intr;
	gusb_llop_get  llop_get_bulk;
	gusb_llop_send llop_send;
	gusb_llop_close llop_close;
	int max_tx_size;
} gusb_llops_t;

/* Provided by the common code. */
void gusb_syncup(void);
void gusb_register_ll(struct gusb_llops *);
void gusb_list_units(void);

/* Provided by the OS layers */
// int gusb_init(const char *portname, gpsdev **dh);

