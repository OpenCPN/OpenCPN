/**************************************************************************
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

/**
 * \file udev_rule_mgr.h Access checks for comm devices and dongle.
 *
 * Only making anything useful on Linux.
 */

#ifndef UDEV_RULE_MGR_H__
#define UDEV_RULE_MGR_H__

#include <string>
#include <wx/window.h>

/** Runs checks and if required dialogs to make dongle accessible. */
bool CheckDongleAccess(wxWindow* parent);

/**
 * Run checks and possible dialogs to ensure device is accessible
 * @param parent  Dialogs parent window
 * @param device  Full device path
 * @return True if device can be accessed read/write.
 */
bool CheckSerialAccess(wxWindow* parent, const std::string device);

/** Destroy all open "Device not found" dialog windows. */
void DestroyDeviceNotFoundDialogs();

#endif  // UDEV_RULE_MGR_H__
