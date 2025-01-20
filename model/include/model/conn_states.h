
/***************************************************************************
 *   Copyright (C) 2024 Alec Leamas                                        *
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

#ifndef CONN_STATES_H_
#define CONN_STATES_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "observable_evtvar.h"
#include "model/comm_drv_registry.h"
#include "model/comm_drv_stats.h"
#include "model/comm_navmsg.h"

/**
 * \file
 * Runtime connection/driver state definitions.
 */

enum class ConnState { Disabled, NoStats, NoData, Unavailable, Ok };

const std::unordered_map<ConnState, std::string> ConnStateToString = {
    {ConnState::Disabled, "Disabled"},
    {ConnState::NoStats, "NoStats"},
    {ConnState::NoData, "NoData"},
    {ConnState::Unavailable, "Unavailable"},
    {ConnState::Ok, "Ok"}};

/** Runtime data for a driver and thus connection. */
class ConnData {
public:
  ConnData() : state(ConnState::NoStats) {}

  ConnState state;
  struct DriverStats driver_stats;

  [[nodiscard]] bool IsDriverStatsMatch(const DriverStats& ds) const;
  [[nodiscard]] bool IsDriverMatch(NavAddr::Bus bus,
                                   const std::string& iface) const;
  std::string to_string();
};

/**
 * Filter reading driver driver_stats status reports from CommDriverRegistry
 * transforming these to a stream of ConnData updates;
 */
class ConnStates {
public:
  ConnStates();
  ConnStates(const ConnStates&) = delete;

  /** Notified with a shared_ptr<ConnData> when the state is changed. */
  EventVar evt_conn_status_change;

  void HandleDriverStats(const struct DriverStats& stats);
  ConnState GetDriverState(NavAddr::Bus bus, const std::string& iface);

private:
  ObsListener m_driverlist_lstnr;
  ObsListener m_driverstat_lstnr;
  std::vector<ConnData> m_states;

  /** Remove ghost stats referring to non-existent drivers. */
  void OnDriverListChange(const ObservedEvt& ev);

  void OnDriverStats(const ObservedEvt& ev);
};

#endif  // CONN_STATES_H_
