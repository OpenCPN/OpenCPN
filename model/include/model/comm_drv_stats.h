/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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
 * Communication statistics infrastructure.
 */

#ifndef _DRIVER_STATS_H
#define _DRIVER_STATS_H

#include <chrono>
#include <memory>
#include <string>

#include "observable.h"
#include "comm_drv_registry.h"
#include "comm_navmsg.h"
#include "periodic_timer.h"

/** Driver statistics report. */
struct DriverStats {
  NavAddr::Bus driver_bus;
  std::string driver_iface;
  unsigned rx_count;     ///< Number of bytes received since program start.
  unsigned tx_count;     ///< Number of bytes sent since program start.
  unsigned error_count;  ///< Number of detected errors since program start.
  bool available;        /// Is driver device available?
  DriverStats()
      : driver_bus(NavAddr::Bus::Undef),
        rx_count(0),
        tx_count(0),
        error_count(0),
        available(false) {}
};

/** Driver interface providing driver statistics */
class DriverStatsProvider {
public:
  virtual DriverStats GetDriverStats() const = 0;
};

/** Continuously report driver stats to CommDrvRegistry.evt_driver_stats */
class StatsTimer : public PeriodicTimer {
public:
  StatsTimer(const DriverStatsProvider& src, std::chrono::milliseconds interval)
      : PeriodicTimer(interval), m_src(src) {}

protected:
  const DriverStatsProvider& m_src;

  void Notify() {
    auto stats_ptr = std::make_shared<DriverStats>(m_src.GetDriverStats());
    CommDriverRegistry::GetInstance().evt_driver_stats.Notify(stats_ptr, "");
  };
};

#endif  // DRIVER_STATS_H
