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

/* 
 * Dialogs about missing udev rules and how to install them.
 */

#ifndef UDEV_RULE_MGR_H__
#define UDEV_RULE_MGR_H__

#include <wx/dialog.h>
#include <wx/window.h>


class DongleRuleDialog: public wxDialog
{
    public:

        /** * Handle missing udev rule for existing dongle.  */
        DongleRuleDialog(wxWindow* parent);
};

class DeviceRuleDialog: public wxDialog
{
    public:

        /** Handle missing udev rule for a device, a /dev path. */
        DeviceRuleDialog(wxWindow* parent, const char* device_path);
};



#endif // UDEV_RULE_MGR_H__
