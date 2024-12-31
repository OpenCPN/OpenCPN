/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022  Alec Leamas                                       *
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
 * \file
 * Implement comm_drv_registry.h
 */

#include <algorithm>
#include <memory>

#include "model/comm_driver.h"
#include "model/comm_drv_registry.h"

void CommDriverRegistry::Activate(DriverPtr driver) {
  if (!driver) return;
  AbstractCommDriver* found = nullptr;
  for (auto& d : drivers)
    if (d->iface == driver->iface && d->bus == driver->bus) found = d.get();
  if (found) return;
  drivers.push_back(std::move(driver));
  evt_driverlist_change.Notify();
};

void CommDriverRegistry::Deactivate(DriverPtr& driver) {
  auto found = std::find(drivers.begin(), drivers.end(), driver);
  if (found == drivers.end()) return;
  drivers.erase(found);
  evt_driverlist_change.Notify();
}

const std::vector<DriverPtr>& CommDriverRegistry::GetDrivers() const {
  return drivers;
};

void CommDriverRegistry::CloseAllDrivers() {
  while (drivers.size()) {
    Deactivate(drivers[0]);
  }
}

CommDriverRegistry& CommDriverRegistry::GetInstance() {
  static CommDriverRegistry instance;
  return instance;
}

std::unique_ptr<AbstractCommDriver> kNoDriver(nullptr);

DriverPtr& FindDriver(const std::vector<DriverPtr>& drivers,
                      const std::string& iface, const NavAddr::Bus _bus) {
  if (_bus != NavAddr::Bus::Undef) {
    for (const DriverPtr& d : drivers) {
      if (d->iface == iface && d->bus == _bus) {
        return const_cast<DriverPtr&>(d);
      }
    }
    return kNoDriver;
  } else {
    for (auto&& d : drivers)
      if (d->iface == iface) return const_cast<DriverPtr&>(d);
    return kNoDriver;
  }
}
