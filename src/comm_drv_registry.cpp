/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implements comm_drv_registry.h
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
// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <algorithm>
#include <memory>

#include "comm_driver.h"
#include "comm_drv_registry.h"

void CommDriverRegistry::Activate(DriverPtr driver) {
  auto found = std::find(drivers.begin(), drivers.end(), driver);
  if (found != drivers.end()) return;
  drivers.push_back(driver);
  evt_driverlist_change.Notify();
};

void CommDriverRegistry::Deactivate(DriverPtr driver) {
  auto found = std::find(drivers.begin(), drivers.end(), driver);
  if (found == drivers.end()) return;
  drivers.erase(found);
  evt_driverlist_change.Notify();
}

const std::vector<DriverPtr>& CommDriverRegistry::GetDrivers() {
  return drivers;
};

void CommDriverRegistry::CloseAllDrivers() {
  for (auto& it : drivers) Deactivate(it);
  drivers.clear();
}

CommDriverRegistry& CommDriverRegistry::GetInstance() {
  static CommDriverRegistry instance;
  return instance;
}

const std::shared_ptr<AbstractCommDriver> kNoDriver(nullptr);

const DriverPtr FindDriver(const std::vector<DriverPtr>& drivers,
                           const std::string& iface, const NavAddr::Bus _bus) {
  if (_bus != NavAddr::Bus::Undef){
    auto func = [iface, _bus](const DriverPtr d) {
      return ((d->iface == iface) && (d->bus == _bus));
      };
    auto found = std::find_if(drivers.begin(), drivers.end(), func);
    return found != drivers.end() ? *found : kNoDriver;
  }
  else{
    auto func = [iface](const DriverPtr d) { return d->iface == iface; };
    auto found = std::find_if(drivers.begin(), drivers.end(), func);
    return found != drivers.end() ? *found : kNoDriver;
  }
}
