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

#include "model/comm_driver.h"
#include "observable_evtvar.h"

typedef std::shared_ptr<AbstractCommDriver> DriverPtr;

/**
 * The global driver registry, a singleton. Drivers register here when
 * activated, transport layer finds them.
 *
 * Also used as exchange point for messages for USB devices hotplug events.
 */
class CommDriverRegistry final {
public:
  static CommDriverRegistry& GetInstance();

  /** Add driver to list of active drivers. */
  void Activate(DriverPtr driver);

  /** Remove driver from list of active drivers. */
  void Deactivate(DriverPtr driver);

  /** Close and destroy all drivers completely. */
  void CloseAllDrivers();

  /** @return List of all activated drivers. */
  const std::vector<DriverPtr>& GetDrivers();

  /** Notified by all driverlist updates. */
  EventVar evt_driverlist_change;

  /** Notified when receiving --remote --dump_stat on local API. */
  EventVar evt_dump_stats;

  /** Notified with a printable message on first detected overrun. */
  EventVar evt_comm_overrun;

  /**
   *  Notified for messages from drivers. The generated event contains:
   *  - A wxLogLevel stored as an int.
   *  - A string is with a prefix from originating driver class name e. g.,
   *    "CommDriverN2KSerial:  Something happened"
   */
  EventVar evt_driver_msg;

private:
  CommDriverRegistry() = default;
  CommDriverRegistry(const CommDriverRegistry&) = delete;
  CommDriverRegistry& operator=(const CommDriverRegistry&) = delete;

  std::vector<DriverPtr> drivers;
};

/**
 * Search list of drivers for a driver with given interface string.
 * @return First found driver or shared_ptr<>(nullptr), which is false.
 */
const DriverPtr FindDriver(const std::vector<DriverPtr>& drivers,
                           const std::string& iface,
                           const NavAddr::Bus _bus = NavAddr::Bus::Undef);

#endif  // guard
