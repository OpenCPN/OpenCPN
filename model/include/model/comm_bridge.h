/**************************************************************************
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

/**
 * \file
 *
 * The CommBridge class and helpers.
 */

#ifndef COMM_BRIDGE_H
#define COMM_BRIDGE_H

#include <memory>
#include <string>
#include <unordered_map>

#include <wx/event.h>
#include <wx/timer.h>

#include "model/comm_decoder.h"
#include "model/comm_navmsg.h"
#include "model/nmea_log.h"

using N0183MsgPtr = std::shared_ptr<const Nmea0183Msg>;
using N2000MsgPtr = std::shared_ptr<const Nmea2000Msg>;
using SignalKMsgPtr = std::shared_ptr<const SignalkMsg>;
using NavMsgPtr = std::shared_ptr<const NavMsg>;

using PriorityMap = std::unordered_map<std::string, int>;

struct PriorityContainer {
  std::string prio_class;
  int active_priority;
  std::string active_source;
  std::string active_identifier;
  int active_source_address;
  time_t recent_active_time;
  PriorityContainer(const std::string& cls, int prio = 0)
      : prio_class(cls),
        active_priority(prio),
        active_source_address(-1),
        recent_active_time(0) {}
};

struct Watchdogs {
  int position_watchdog;
  int variation_watchdog;
  int heading_watchdog;
  int velocity_watchdog;
  int satellite_watchdog;
  Watchdogs()
      : position_watchdog(0),
        variation_watchdog(0),
        heading_watchdog(0),
        velocity_watchdog(0),
        satellite_watchdog(0) {}
};

struct BridgeLogCallbacks {
  std::function<bool()> log_is_active;
  std::function<void(const Logline&)> log_message;
  BridgeLogCallbacks()
      : log_is_active([]() { return false; }),
        log_message([](const Logline&) {}) {}
};

/**
 * Process incoming messages.
 *
 * Listem to "known" messages and process them for example by broadcasting
 * ship data. Also handles message source priorities i.e., which source to
 * use for broadcasted data.
 */
class CommBridge : public wxEvtHandler {
public:
  CommBridge();

  ~CommBridge() override;

  bool Initialize();

  /**
   * Processes NMEA 0183 AIVDO sentences containing own vessel's AIS data.
   *
   * AIVDO messages provide navigation data broadcast by the vessel's own AIS
   * transponder:
   * - Position (latitude/longitude)
   * - Movement (SOG/COG)
   * - Heading (true heading)
   *
   * The data is evaluated against other sources based on configured priorities.
   * If accepted, the data updates the vessel's:
   * - Position (if valid lat/lon received)
   * - Speed and course (if valid SOG/COG received)
   * - True heading (if valid HDT received)
   *
   * @param n0183_msg Shared pointer to NMEA 0183 message containing AIVDO
   * sentence
   * @return true if message was processed, false on decode error
   */
  bool HandleN0183_AIVDO(const N0183MsgPtr& n0183_msg);

  bool EvalPriority(const NavMsgPtr& msg, PriorityContainer& active_priority,
                    PriorityMap& priority_map);

  std::vector<std::string> GetPriorityMaps() const;

  PriorityContainer& GetPriorityContainer(const std::string& category);

  void UpdateAndApplyMaps(const std::vector<std::string>& new_maps);

  bool LoadConfig();

  bool SaveConfig() const;

private:
  PriorityContainer active_priority_position;
  PriorityContainer active_priority_velocity;
  PriorityContainer active_priority_heading;
  PriorityContainer active_priority_variation;
  PriorityContainer active_priority_satellites;
  PriorityContainer active_priority_void;

  PriorityMap priority_map_position;
  PriorityMap priority_map_velocity;
  PriorityMap priority_map_heading;
  PriorityMap priority_map_variation;
  PriorityMap priority_map_satellites;

  //  comm event listeners
  ObsListener m_n2k_129029_lstnr;
  ObsListener m_n2k_129025_lstnr;
  ObsListener m_n2k_129026_lstnr;
  ObsListener m_n2k_127250_lstnr;
  ObsListener m_n2k_129540_lstnr;

  ObsListener m_n0183_rmc_lstnr;
  ObsListener m_n0183_hdt_lstnr;
  ObsListener m_n0183_hdg_lstnr;
  ObsListener m_n0183_hdm_lstnr;
  ObsListener m_n0183_vtg_lstnr;
  ObsListener m_n0183_gsv_lstnr;
  ObsListener m_n0183_gga_lstnr;
  ObsListener m_n0183_gll_lstnr;
  ObsListener m_n0183_aivdo_lstnr;

  ObsListener m_signal_k_lstnr;

  ObsListener m_driver_change_lstnr;

  CommDecoder m_decoder;

  bool HandleN0183_GGA(const N0183MsgPtr& n0183_msg);
  bool HandleN0183_GLL(const N0183MsgPtr& n0183_msg);

  int m_n_log_watchdog_period;

  BridgeLogCallbacks m_log_callbacks;
  int m_last_position_priority;
  std::string m_last_position_source;
  Watchdogs m_watchdogs;
  wxTimer m_watchdog_timer;

  void InitCommListeners();

  void PresetWatchdogs();
  void MakeHDTFromHDM();

  void ApplyPriorityMaps(const std::vector<std::string>& new_maps);
  void ClearPriorityMaps();
  void PresetPriorityContainers();
  void OnDriverStateChange();

  void OnWatchdogTimer();
  ;

  bool HandleN2K_129029(const N2000MsgPtr& n2k_msg);
  bool HandleN2K_129025(const N2000MsgPtr& n2k_msg);
  bool HandleN2K_129026(const N2000MsgPtr& n2k_msg);
  bool HandleN2K_127250(const N2000MsgPtr& n2k_msg);
  bool HandleN2K_129540(const N2000MsgPtr& n2k_msg);

  bool HandleN0183_RMC(const N0183MsgPtr& n0183_msg);
  bool HandleN0183_HDT(const N0183MsgPtr& n0183_msg);
  bool HandleN0183_HDG(const N0183MsgPtr& n0183_msg);
  bool HandleN0183_HDM(const N0183MsgPtr& n0183_msg);
  bool HandleN0183_VTG(const N0183MsgPtr& n0183_msg);
  bool HandleN0183_GSV(const N0183MsgPtr& n0183_msg);

  bool HandleSignalK(const SignalKMsgPtr& sK_msg);
};

#endif  // COMM_BRIDGE_H
