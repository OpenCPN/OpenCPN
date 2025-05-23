/**************************************************************************
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

/**
 * \file
 * Implement conn_states.h
 */
#include <iostream>

#include <algorithm>
#include <sstream>

#include "model/conn_states.h"
#include "model/comm_drv_stats.h"
#include "model/conn_params.h"

bool ConnData::IsDriverStatsMatch(const DriverStats& ds) const {
  return ds.driver_bus == driver_stats.driver_bus &&
         ds.driver_iface == driver_stats.driver_iface;
}

bool ConnData::IsDriverMatch(NavAddr::Bus bus, const std::string& iface) const {
  return driver_stats.driver_bus == bus && driver_stats.driver_iface == iface;
}

std::string ConnData::to_string() {
  std::stringstream ss;
  return ss.str();
}

ConnStates::ConnStates() {
  auto& registry = CommDriverRegistry::GetInstance();
  m_driverlist_lstnr.Init(registry.evt_driverlist_change,
                          [&](ObservedEvt& ev) { OnDriverListChange(ev); });
  m_driverstat_lstnr.Init(registry.evt_driver_stats,
                          [&](ObservedEvt& ev) { OnDriverStats(ev); });
}

void ConnStates::HandleDriverStats(const DriverStats& stats) {
  auto conn_params = TheConnectionParams();
  auto found_param = std::find_if(
      conn_params.begin(), conn_params.end(), [stats](ConnectionParams* cp) {
        return cp->GetStrippedDSPort() == stats.driver_iface &&
               cp->GetCommProtocol() == stats.driver_bus;
      });
  bool disabled = found_param != conn_params.end() && !(*found_param)->bEnabled;
  auto found_state = std::find_if(
      m_states.begin(), m_states.end(),
      [stats](ConnData& cd) { return cd.IsDriverStatsMatch(stats); });
  if (found_state == m_states.end()) {
    ConnData data;
    data.state = ConnState::NoStats;
    data.driver_stats = stats;
    m_states.push_back(data);
    evt_conn_status_change.Notify(std::make_shared<ConnData>(data), "");
  } else {
    ConnState old_state = found_state->state;
    if (!stats.available) {
      found_state->state = ConnState::Unavailable;
    } else if (found_state->driver_stats.rx_count == stats.rx_count &&
               found_state->driver_stats.tx_count == stats.tx_count) {
      found_state->state = ConnState::NoData;
    } else {
      found_state->state = ConnState::Ok;
    }
    found_state->driver_stats = stats;
    if (found_state->state != old_state)
      evt_conn_status_change.Notify(std::make_shared<ConnData>(*found_state),
                                    "");
  }
}

ConnState ConnStates::GetDriverState(NavAddr::Bus bus,
                                     const std::string& iface) {
  auto found = std::find_if(
      m_states.begin(), m_states.end(),
      [bus, iface](ConnData& cd) { return cd.IsDriverMatch(bus, iface); });
  return found == m_states.end() ? ConnState::NoStats : found->state;
}

/** Remove states corresponding to non-existent driver in registry. */
void ConnStates::OnDriverListChange(const ObservedEvt&) {
  auto& drivers = CommDriverRegistry::GetInstance().GetDrivers();
  auto end = std::remove_if(m_states.begin(), m_states.end(), [&](ConnData cd) {
    return FindDriver(drivers, cd.driver_stats.driver_iface,
                      cd.driver_stats.driver_bus) == nullptr;
  });
  m_states.erase(end, m_states.end());
}

void ConnStates::OnDriverStats(const ObservedEvt& ev) {
  auto stats_ptr = UnpackEvtPointer<DriverStats>(ev);
  if (stats_ptr) HandleDriverStats(*stats_ptr);
}
