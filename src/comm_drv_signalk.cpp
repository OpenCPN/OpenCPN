/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#include "comm_drv_signalk_net.h"

/*    commdriverSignalk implementation
 * */

CommDriverSignalK::CommDriverSignalK(const std::string& s)
    : AbstractCommDriver(NavAddr::Bus::N2000, s) {}

CommDriverSignalK::~CommDriverSignalK() {}

void CommDriverSignalK::SendMessage(std::shared_ptr<const NavMsg> msg,
                                    std::shared_ptr<const NavAddr> addr) {}

void CommDriverSignalK::SetListener(std::shared_ptr<DriverListener> l){};

std::shared_ptr<NavAddr> CommDriverSignalK::GetAddress(const NavAddrSignalK& name) {
    return std::make_shared<NavAddr>(NavAddrSignalK());
}

