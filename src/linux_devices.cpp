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

#include <stdlib.h>
#include <unistd.h>

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

static const char* const DONGLE_RULE =
    R"""(ATTRS{idVendor}=="1547", ATTRS{idProduct}=="1000", MODE="666"
)""";

static const char* const DEVICE_RULE = R"""(
ATTRS{idVendor}=="@vendor@", ATTRS{idProduct}=="@product@", \
    MODE="666", SYMLINK+="@link@"
)""";


static int try_open(int vendorId, int productId)
{
    libusb_context* ctx = 0;
    int r = libusb_init(&ctx);
    if (r != 0) {
        auto e = static_cast<libusb_error>(r);
        WARNING_LOG << "Cannot initialize libusb: " << libusb_strerror(e);
        return LIBUSB_ERROR_NOT_SUPPORTED;
    }
    libusb_device** device_list;
    ssize_t size = libusb_get_device_list(ctx, &device_list);
    if  (size < 0) {
        auto e = static_cast<libusb_error>(size);
        DEBUG_LOG << "Cannot get usb devices list: " << libusb_strerror(e);
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


bool is_device_permissions_ok(const char* path)
{
    int r = access(path, R_OK & W_OK);
    if (r < 0) {
        INFO_LOG << "access(3) fails on: " << path << ": " << strerror(errno);
        return true;
    }
    return r == 0;
}


static void fix_token(std::string& token)
{
    while (token.length() < 4) {
        token.insert(0, "0");
    }
}


static usbdata parse_uevent(std::istream& is)
{
    std::string line;
    while (std::getline(is, line)) {
        if (!ocpn::startswith(line, "PRODUCT")) {
            continue;
        }
        line = line.substr(strlen("PRODUCT="));
        auto tokens = ocpn::split(line.c_str(), "/");
        if (tokens.size() == 3) {
            tokens.insert(tokens.begin(), std::string("foo"));
        }
        fix_token(tokens[1]);
        fix_token(tokens[2]);
        return usbdata(tokens[1], tokens[2]);
    }
    return usbdata("", "");
}


usbdata get_device_usbdata(const char* path)
{
    // Get real path for node in /sys corresponding to path in /dev
    struct stat st;
    int r = stat(path, &st);
    if (r < 0) {
        MESSAGE_LOG << "Cannot stat: " << path << ": " << strerror(errno);
        return usbdata(0, 0);
    }
    std::stringstream syspath("/sys/dev/char/");
    syspath << "/sys/dev/char/" << major(st.st_dev) << ":" << minor(st.st_dev);
    char buff[PATH_MAX];
    realpath(syspath.str().c_str(), buff);
    std::string real_path(buff);

    // Get the uevent file in each parent dir and parse it.
    while (real_path.length() > 0) {
        auto uevent_path = real_path + "/uevent";
        if (access(uevent_path.c_str(), R_OK) < 0) {
            continue;
        }
        std::ifstream is(uevent_path);
        auto data = parse_uevent(is);
        if (data.is_ok()) {
            return data; 
        }
        // Drop last part of filename
        size_t last_slash = real_path.rfind('/');
        last_slash = last_slash == std::string::npos ? 0 : last_slash;
        real_path = real_path.substr(0, last_slash);
    }
    return usbdata("", "");
}


std::string create_udev_rule(usbdata data, const char* device_path)
{
    std::string link(device_path);
    if (ocpn::startswith(link, "tty")) {
        link = link.substr(strlen("tty"));
    }
    link.insert(0, "opencpn-");
    std::string rule(DEVICE_RULE);
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
