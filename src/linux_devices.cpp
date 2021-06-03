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

#include <string>
#include <sstream>
#include <iostream>

#include <unistd.h>
#include <libudev.h>

#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <libusb-1.0/libusb.h>

#include "linux_devices.h"
#include "logger.h"
#include "ocpn_utils.h"

#ifndef HAVE_LIBUSB_10
#error linux_devices requries libusb-1.0 to be available
#endif

#ifndef HAVE_UNISTD_H
#error linux_devices requries unistad.h to be available
#endif

static const int dongle_vendor = 0x1547;
static const int dongle_product = 0x1000;

static const char* const UDEV_RULE = R"""(
ATTRS{idVendor}=="1547", ATTRS{idProduct}=="1000", MODE="666"
ATTRS{idVendor}=="@vendor@", ATTRS{idProduct}=="@product@", \
    MODE="666", SYMLINK+="@link@"
)""";


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


bool is_device_permissions_ok(const char* path) {
    int r = access(path, R_OK & W_OK);
    if (r < 0) {
        INFO_LOG << "access(3) fails on: " << path << ": " << strerror(errno);
        return true;
    }
    return r == 0;
}


usbdata get_device_usbdata(const char* path)
{
    struct stat st;
    int r = stat(path, &st);
    if (r < 0) {
        MESSAGE_LOG << "Cannot stat: " << path << ": " << strerror(errno);
        return usbdata(0, 0);
    }
    struct udev *udev;
    udev = udev_new();
    if (!udev) {
        WARNING_LOG << "Cannot create udev context";
        return usbdata(0, 0);
    }
    struct udev_device* root_dev;
    std::stringstream id;
    id << "c" << major(st.st_dev) << ":" << minor(st.st_dev);
    root_dev = udev_device_new_from_device_id(udev, id.str().c_str());
    if (!root_dev) {
        WARNING_LOG <<  "Failed to get udev device for " << id.str();
	udev_unref(udev);
        return usbdata(0, 0);
    }
    const char* product = 0;
    const char* vendor = 0;
    struct udev_device* dev;
    for (dev = root_dev; dev; dev = udev_device_get_parent(dev)) {
        product = udev_device_get_sysattr_value(dev, "idProduct");
        vendor = udev_device_get_sysattr_value(dev, "idVendor");
        if (product || vendor) {
            break;
        }
    }
    usbdata rv(vendor, product);
    udev_device_unref(dev);
    udev_unref(udev);
    return rv;
}


std::string create_udev_rule(usbdata data, const char* device_path)
{
    std::string link(device_path);
    if (ocpn::startswith(link, "tty")) {
        link = link.substr(strlen("tty"));
    }
    link.insert(0, "opencpn-");
    std::string rule(UDEV_RULE);
    ocpn::replace(rule, "@vendor@", data.vendor_id);
    ocpn::replace(rule, "@product@", data.product_id);
    ocpn::replace(rule, "@link@", link);

    char dirpath[128];
    strcpy(dirpath, "udevXXXXXX");
    if (!mkdtemp(dirpath)) {
        WARNING_LOG << "Cannot create tempdir: " << strerror(errno);
        MESSAGE_LOG << "Using /tmp";
        strcpy(dirpath, "/tmp");
    }
    std::string path(dirpath);
    path += "/rule";

    std::ofstream of(path);
    of << rule;
    of.close();
    if (of.bad()) {
        WARNING_LOG << "Cannot write to temp rules files";
    }
    return path;
}
