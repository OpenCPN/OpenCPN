/*
    Stubs to keep build happy when USB just isn't available to us.

    Copyright (C) 2004, 2006 Robert Lipe, robertlipe@usa.net

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */


#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "../defs.h"
#include "src/core/logging.h"

#if !HAVE_LIBUSB_1_0

const char no_usb[] = "USB support is not available in this build.\n";
typedef struct gpsdevh gpsdevh;
int
gusb_init(const char* portname, gpsdevh** dh)
{
  Fatal() << no_usb;
  return 0;
}

#endif /* defined(HAVE_LIBUSB_1_0) */
