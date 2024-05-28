/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Low-level USB device management
 * Author:   Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2021 Alec Leamas                                        *
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

/** \file linux_devices.h Low level udev usb device management. */

#ifndef LINUX_DEVICES_H
#define LINUX_DEVICES_H

#include "config.h"

/** Return true if an existing dongle cannot be accessed. */
bool IsDonglePermissionsWrong();

/**
 * Check device path permissions
 * @param path complete device path
 * @return True if device is exposing read/write permissions.
 */
bool IsDevicePermissionsOk(const char* path);

/**
 * @return udev rule which makes the dongle public read write (666)
 */
std::string GetDongleRule();

/**
 * Get next available udev rule base name
 * @return first name not used in the sequence opencpn0..opencpn9
 */
std::string MakeUdevLink();

/**
 * Get device udev rule
 * @param device  Complete device path
 * @param symlink As returned by  MakeUdevLink()
 * @return  udev rule which makes the device public read/write (0666)
 */
std::string GetDeviceRule(const char* device, const char* symlink);

#endif
