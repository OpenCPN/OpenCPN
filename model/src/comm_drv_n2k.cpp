/***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 by Alec Leamas                                     *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement comm_drv_n2k.h -- Nmea2000 driver base.
 */

#include <string>
#include <memory>

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "model/comm_drv_n2k.h"

/* commdriverN2K implementation */
CommDriverN2K::CommDriverN2K(const std::string& s)
    : AbstractCommDriver(NavAddr::Bus::N2000, s) {}

CommDriverN2K::~CommDriverN2K() {}

bool CommDriverN2K::SendMessage(std::shared_ptr<const NavMsg> msg,
                                std::shared_ptr<const NavAddr> addr) {
  return false;
}

void CommDriverN2K::SetListener(DriverListener& l) {};

std::shared_ptr<NavAddr2000> CommDriverN2K::GetAddress(const N2kName& name) {
  return std::make_shared<NavAddr2000>(NavAddr2000(iface, name));
}
