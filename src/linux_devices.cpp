/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Low-level USB device management
 * Author:   Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2011 Alec Leamas                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/
#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <wx/log.h>

#include "logger.h"
#include "linux_devices.h"

#ifndef HAVE_LIBUSB_10

bool is_dongle_permissions_wrong() { return false; }

#else

#include <libusb-1.0/libusb.h>

static const int dongle_vendor = 0x1547;
static const int dongle_product = 0x1000;


static int try_open(int vendorId, int productId)
{
    libusb_context* ctx = 0;
    int r = libusb_init(&ctx);
    if (r != 0) {
        WARNING_LOG << "Cannot initialize libusb: " << libusb_strerror(r);
        return LIBUSB_ERROR_NOT_SUPPORTED;
    }
    libusb_device** device_list;
    ssize_t size = libusb_get_device_list(ctx, &device_list);
    if  (size < 0) {
        DEBUG_LOG << "Cannot get usb devices list: " << libusb_strerror(size);
        return LIBUSB_ERROR_NOT_SUPPORTED;
    }
    r = LIBUSB_ERROR_INVALID_PARAM;
    for (auto dev = device_list; *dev; dev++)  {
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(*dev, &desc);
        if (desc.idVendor != vendorId || desc.idProduct != productId) {
            continue;
        }
        libusb_device_handle* dev_handle;
        r =  libusb_open(*dev, &dev_handle);
        if (r >= 0) {
            libusb_close(dev_handle);
        }
        break;
    }
    libusb_free_device_list(device_list, 1);
    DEBUG_LOG << "Nothing found for " << vendorId << ":" << productId;
    libusb_exit(0);
    return r;
}


bool is_dongle_permissions_wrong()
{
    int rc = try_open(dongle_vendor, dongle_product);
    DEBUG_LOG << "Probing dongle permissions, result: " << rc;
    return rc == LIBUSB_ERROR_ACCESS;
}

#endif

#ifdef HAVE_UNISTD_H

bool is_device_permissions_ok(const char* path) {
    int r = access(path, R_OK & W_OK);
    if (r < 0) {
        INFO_LOG << "access(3) fails on: " << path << ": " << strerror(errno);
        return true;
    }
    return r == 0;
}

#else

bool is_device_permissions_ok(const char* path)
{
    WARNING_LOG << "Invoking linux-only function in non-linux context";
    return true;
}

#endif
