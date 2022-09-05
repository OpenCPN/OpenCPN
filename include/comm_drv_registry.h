/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Driver registration container, a singleton.
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

#ifndef _COMMDRIVERREGISTRY_H__
#define _COMMDRIVERREGISTRY_H__

#include "comm_driver.h"
#include "observable_evtvar.h"

class ConnectionParams;

/**
 * The global driver registry, a singleton. Drivers register here when
 * activated, transport layer finds them.
 */

typedef std::shared_ptr<AbstractCommDriver> DriverPtr;

class CommDriverRegistry final {
public:
  CommDriverRegistry() {}

  /** Add driver to list of active drivers. */
  void Activate(DriverPtr driver);

  /** Remove driver from list of active drivers. */
  void Deactivate(DriverPtr driver);

  /** Close and destroy all drivers completely. */
  void CloseAllDrivers();

  /** @return List of all activated drivers. */
  const std::vector<DriverPtr>& GetDrivers();

  static CommDriverRegistry& getInstance();

  // FIXME (Dave)
  //  Stub method, to pretest drivers.
  //  Goes away for production
  void TestDriver(ConnectionParams* params);

  /** Notified by all driverlist updates. */
  EventVar evt_driverlist_change;

private:
  std::vector<DriverPtr> drivers;
};

/**
 * Search list of drivers for a driver with given interface string.
 * @return First found driver or shared_ptr<>(nullptr), which is false.
 */
const DriverPtr FindDriver(const std::vector<DriverPtr>& drivers,
                           const std::string& iface);

#endif  // guard
