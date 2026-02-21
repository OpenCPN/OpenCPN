/***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 * Implement comm_drv_signalk.h -- SignalK driver
 */

#include <memory>
#include <string>

#include "model/comm_drv_signalk_net.h"

/* commdriverSignalk implementation */

CommDriverSignalK::CommDriverSignalK(const std::string& s)
    : AbstractCommDriver(NavAddr::Bus::Signalk, s) {}

CommDriverSignalK::~CommDriverSignalK() {}

bool CommDriverSignalK::SendMessage(std::shared_ptr<const NavMsg> msg,
                                    std::shared_ptr<const NavAddr> addr) {
  return false;
}

void CommDriverSignalK::SetListener(DriverListener& l) {};

std::shared_ptr<NavAddr> CommDriverSignalK::GetAddress(
    const NavAddrSignalK& name) {
  return std::make_shared<NavAddr>(NavAddrSignalK(name.iface));
}
