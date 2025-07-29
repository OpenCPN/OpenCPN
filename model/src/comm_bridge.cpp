/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 - 2025 by Alec Leamas                              *
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
 * Implement comm_bridge.h
 */

// For compilers that support precompilation, includes "wx.h".

#include <sstream>
#include <string>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/event.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/fileconf.h>

#include "model/comm_ais.h"
#include "model/comm_appmsg_bus.h"
#include "model/comm_bridge.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/comm_vars.h"
#include "model/config_vars.h"
#include "model/cutil.h"
#include "model/gui.h"
#include "model/idents.h"
#include "model/ocpn_types.h"
#include "model/own_ship.h"
#include "model/multiplexer.h"
#include "model/notification_manager.h"

#define N_ACTIVE_LOG_WATCHDOG 300

using std::string;

bool debug_priority = false;

void ClearNavData(NavData& d) {
  d.gLat = NAN;
  d.gLon = NAN;
  d.gSog = NAN;
  d.gCog = NAN;
  d.gHdt = NAN;
  d.gHdm = NAN;
  d.gVar = NAN;
  d.n_satellites = -1;
  d.SID = 0;
}
static NmeaLog* GetDataMonitor() {
  auto w = wxWindow::FindWindowByName(kDataMonitorWindowName);
  return dynamic_cast<NmeaLog*>(w);
}
static BridgeLogCallbacks GetLogCallbacks() {
  BridgeLogCallbacks log_callbacks;
  log_callbacks.log_is_active = [&]() {
    auto log = GetDataMonitor();
    return log && log->IsVisible();
  };
  log_callbacks.log_message = [&](const Logline& ll) {
    NmeaLog* monitor = GetDataMonitor();
    if (monitor && monitor->IsVisible()) monitor->Add(ll);
  };
  return log_callbacks;
}

class AppNavMsg : public NavMsg {
public:
  AppNavMsg(const std::shared_ptr<const AppMsg>& msg, const string& name)
      : NavMsg(NavAddr::Bus::AppMsg,
               std::make_shared<const NavAddrPlugin>("AppMsg")),
        m_to_string(msg->to_string()),
        m_name(name) {}

  [[nodiscard]] string to_string() const override { return m_to_string; }

  [[nodiscard]] string key() const override { return "appmsg::" + m_name; }

  const string m_to_string;
  const string m_name;
};

static void LogAppMsg(const std::shared_ptr<const AppMsg>& msg,
                      const string& name, const BridgeLogCallbacks& log_cb) {
  if (!log_cb.log_is_active()) return;
  auto navmsg = std::make_shared<AppNavMsg>(msg, "basic-navdata");
  NavmsgStatus ns;
  Logline ll(navmsg, ns);
  log_cb.log_message(ll);
}

/**
 * Send BasicNavDataMsg based on global state in gLat, gLon, etc
 * on appmsg_bus
 */
static void SendBasicNavdata(int vflag,
                             const BridgeLogCallbacks& log_callbacks) {
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, vflag, wxDateTime::Now().GetTicks());
  clock_gettime(CLOCK_MONOTONIC, &msg->set_time);
  LogAppMsg(msg, "basic-navdata", log_callbacks);
  AppMsgBus::GetInstance().Notify(std::move(msg));
}

static inline double GeodesicRadToDeg(double rads) {
  return rads * 180.0 / M_PI;
}

static inline double MS2KNOTS(double ms) { return ms * 1.9438444924406; }

static string GetPriorityKey(const NavMsgPtr& msg) {
  string key;

  string this_identifier;
  string this_address("0");
  if (msg->bus == NavAddr::Bus::N0183) {
    auto msg_0183 = std::dynamic_pointer_cast<const Nmea0183Msg>(msg);
    if (msg_0183) {
      string source = msg->source->to_string();
      this_identifier = msg_0183->talker;
      this_identifier += msg_0183->type;
      key = source + ":" + this_address + ";" + this_identifier;
    }
  } else if (msg->bus == NavAddr::Bus::N2000) {
    auto msg_n2k = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
    if (msg_n2k) {
      this_identifier = msg_n2k->PGN.to_string();
      unsigned char n_source = msg_n2k->payload.at(7);
      wxString km = wxString::Format("N2k device address: %d", n_source);
      key = km.ToStdString() + " ; " + "PGN: " + this_identifier;
    }
  } else if (msg->bus == NavAddr::Bus::Signalk) {
    auto msg_sk = std::dynamic_pointer_cast<const SignalkMsg>(msg);
    if (msg_sk) {
      auto addr_sk =
          std::static_pointer_cast<const NavAddrSignalK>(msg->source);
      string source = addr_sk->to_string();
      key = source;  // Simplified, parsing sK for more info is expensive
    }
  }

  return key;
}

static void PresetPriorityContainer(PriorityContainer& pc,
                                    const PriorityMap& priority_map) {
  // Extract some info from the preloaded map
  // Find the key corresponding to priority 0, the highest
  string key0;
  for (const auto& it : priority_map) {
    if (it.second == 0) key0 = it.first;
  }

  wxString this_key(key0.c_str());
  wxStringTokenizer tkz(this_key, _T(";"));
  string source = tkz.GetNextToken().ToStdString();
  string this_identifier = tkz.GetNextToken().ToStdString();

  wxStringTokenizer tka(source, ":");
  tka.GetNextToken();
  std::stringstream ss;
  ss << tka.GetNextToken();
  ss >> pc.active_source_address;
  pc.active_priority = 0;
  pc.active_source = source;
  pc.active_identifier = this_identifier;
  pc.recent_active_time = -1;
}

static void ApplyPriorityMap(PriorityMap& priority_map,
                             const wxString& new_prio, int category) {
  priority_map.clear();
  wxStringTokenizer tk(new_prio, "|");
  int index = 0;
  while (tk.HasMoreTokens()) {
    wxString entry = tk.GetNextToken();
    string s_entry(entry.c_str());
    priority_map[s_entry] = index;
    index++;
  }
}

static string GetPriorityMap(const PriorityMap& map) {
#define MAX_SOURCES 10
  string sa[MAX_SOURCES];
  string result;

  for (auto& it : map) {
    if ((it.second >= 0) && (it.second < MAX_SOURCES)) sa[it.second] = it.first;
  }

  // build the packed string result
  for (int i = 0; i < MAX_SOURCES; i++) {
    if (!sa[i].empty()) {
      result += sa[i];
      result += "|";
    }
  }

  return result;
}

static bool IsNextLowerPriorityAvailable(const PriorityMap& map,
                                         const PriorityContainer& pc) {
  int best_prio = 100;
  for (auto& it : map) {
    if (it.second > pc.active_priority) {
      best_prio = wxMin(best_prio, it.second);
    }
  }
  return best_prio != pc.active_priority;
}

static void SelectNextLowerPriority(const PriorityMap& map,
                                    PriorityContainer& pc) {
  int best_prio = 100;
  for (const auto& it : map) {
    if (it.second > pc.active_priority) {
      best_prio = wxMin(best_prio, it.second);
    }
  }
  pc.active_priority = best_prio;
  pc.active_source.clear();
  pc.active_identifier.clear();
}

// CommBridge implementation

CommBridge::CommBridge()
    : wxEvtHandler(),
      // every 60 minutes, reduced after first position Rx
      active_priority_position("position"),
      active_priority_velocity("velocity"),
      active_priority_heading("heading"),
      active_priority_variation("variation"),
      active_priority_satellites("satellites"),
      active_priority_void("", -1),
      m_n_log_watchdog_period(3600),
      m_last_position_priority(0) {
  Bind(wxEVT_TIMER, [&](wxTimerEvent&) { OnWatchdogTimer(); });
}
CommBridge::~CommBridge() = default;

bool CommBridge::Initialize() {
  m_log_callbacks = GetLogCallbacks();
  ClearPriorityMaps();

  LoadConfig();
  PresetPriorityContainers();

  // Clear the watchdogs
  PresetWatchdogs();

  m_watchdog_timer.SetOwner(this, WATCHDOG_TIMER);
  m_watchdog_timer.Start(1000, wxTIMER_CONTINUOUS);

  InitCommListeners();

  // Initialize a listener for driver state changes
  m_driver_change_lstnr.Init(
      CommDriverRegistry::GetInstance().evt_driverlist_change,
      [&](const wxCommandEvent& ev) { OnDriverStateChange(); });

  return true;
}

void CommBridge::PresetWatchdogs() {
  m_watchdogs.position_watchdog =
      20;  // A bit longer watchdog for startup latency.
  m_watchdogs.velocity_watchdog = 20;
  m_watchdogs.variation_watchdog = 20;
  m_watchdogs.heading_watchdog = 20;
  m_watchdogs.satellite_watchdog = 20;
}

void CommBridge::OnWatchdogTimer() {
  //  Update and check watchdog timer for GPS data source
  m_watchdogs.position_watchdog--;
  if (m_watchdogs.position_watchdog <= 0) {
    if (m_watchdogs.position_watchdog % 5 == 0) {
      // Send AppMsg telling of watchdog expiry
      auto msg = std::make_shared<GPSWatchdogMsg>(
          GPSWatchdogMsg::WDSource::position, m_watchdogs.position_watchdog);
      auto& msgbus = AppMsgBus::GetInstance();
      LogAppMsg(msg, "watchdog", m_log_callbacks);
      msgbus.Notify(std::move(msg));

      if (m_watchdogs.position_watchdog % m_n_log_watchdog_period == 0) {
        wxString logmsg;
        logmsg.Printf(_T("   ***GPS Watchdog timeout at Lat:%g   Lon: %g"),
                      gLat, gLon);
        wxLogMessage(logmsg);
      }
    }

    gSog = NAN;
    gCog = NAN;
    gRmcDate.Empty();
    gRmcTime.Empty();
    active_priority_position.recent_active_time = -1;

    // Are there any other lower priority sources?
    // If so, adopt that one.
    if (IsNextLowerPriorityAvailable(priority_map_position,
                                     active_priority_position)) {
      SelectNextLowerPriority(priority_map_position, active_priority_position);
    }
  }

  //  Update and check watchdog timer for SOG/COG data source
  m_watchdogs.velocity_watchdog--;
  if (m_watchdogs.velocity_watchdog <= 0) {
    gSog = NAN;
    gCog = NAN;
    active_priority_velocity.recent_active_time = -1;

    if (g_nNMEADebug && (m_watchdogs.velocity_watchdog == 0))
      wxLogMessage(_T("   ***Velocity Watchdog timeout..."));
    if (m_watchdogs.velocity_watchdog % 5 == 0) {
      // Send AppMsg telling of watchdog expiry
      auto msg = std::make_shared<GPSWatchdogMsg>(
          GPSWatchdogMsg::WDSource::velocity, m_watchdogs.velocity_watchdog);
      auto& msgbus = AppMsgBus::GetInstance();
      msgbus.Notify(std::move(msg));
    }
    // Are there any other lower priority sources?
    // If so, adopt that one.
    SelectNextLowerPriority(priority_map_velocity, active_priority_velocity);
  }

  //  Update and check watchdog timer for True Heading data source
  m_watchdogs.heading_watchdog--;
  if (m_watchdogs.heading_watchdog <= 0) {
    gHdt = NAN;
    active_priority_heading.recent_active_time = -1;
    if (g_nNMEADebug && (m_watchdogs.heading_watchdog == 0))
      wxLogMessage(_T("   ***HDT Watchdog timeout..."));

    // Are there any other lower priority sources?
    // If so, adopt that one.
    SelectNextLowerPriority(priority_map_heading, active_priority_heading);
  }

  //  Update and check watchdog timer for Magnetic Variation data source
  m_watchdogs.variation_watchdog--;
  if (m_watchdogs.variation_watchdog <= 0) {
    g_bVAR_Rx = false;
    active_priority_variation.recent_active_time = -1;

    if (g_nNMEADebug && (m_watchdogs.variation_watchdog == 0))
      wxLogMessage(_T("   ***VAR Watchdog timeout..."));

    // Are there any other lower priority sources?
    // If so, adopt that one.
    SelectNextLowerPriority(priority_map_variation, active_priority_variation);
  }

  //  Update and check watchdog timer for GSV, GGA and SignalK (Satellite data)
  m_watchdogs.satellite_watchdog--;
  if (m_watchdogs.satellite_watchdog <= 0) {
    g_bSatValid = false;
    g_SatsInView = 0;
    g_priSats = 99;
    active_priority_satellites.recent_active_time = -1;

    if (g_nNMEADebug && (m_watchdogs.satellite_watchdog == 0))
      wxLogMessage(_T("   ***SAT Watchdog timeout..."));

    // Are there any other lower priority sources?
    // If so, adopt that one.
    SelectNextLowerPriority(priority_map_satellites,
                            active_priority_satellites);
  }
}

void CommBridge::MakeHDTFromHDM() {
  //    Here is the one place we try to create gHdt from gHdm and gVar,

  if (!std::isnan(gHdm)) {
    // Set gVar if needed from manual entry. gVar will be overwritten if
    // WMM plugin is available
    if (std::isnan(gVar) && (g_UserVar != 0.0)) gVar = g_UserVar;
    gHdt = gHdm + gVar;
    if (!std::isnan(gHdt)) {
      if (gHdt < 0)
        gHdt += 360.0;
      else if (gHdt >= 360)
        gHdt -= 360.0;

      m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
    }
  }
}

void CommBridge::InitCommListeners() {
  // Initialize the comm listeners

  // GNSS Position Data PGN  129029
  m_n2k_129029_lstnr.Init(Nmea2000Msg(static_cast<uint64_t>(129029)),
                          [&](const ObservedEvt& ev) {
                            HandleN2K_129029(UnpackEvtPointer<Nmea2000Msg>(ev));
                          });

  // Position rapid   PGN 129025
  m_n2k_129025_lstnr.Init(Nmea2000Msg(static_cast<uint64_t>(129025)),
                          [&](const ObservedEvt& ev) {
                            HandleN2K_129025(UnpackEvtPointer<Nmea2000Msg>(ev));
                          });

  // COG SOG rapid   PGN 129026
  m_n2k_129026_lstnr.Init(Nmea2000Msg(static_cast<uint64_t>(129026)),
                          [&](const ObservedEvt& ev) {
                            HandleN2K_129026(UnpackEvtPointer<Nmea2000Msg>(ev));
                          });

  // Heading rapid   PGN 127250
  m_n2k_127250_lstnr.Init(Nmea2000Msg(static_cast<uint64_t>(127250)),
                          [&](const ObservedEvt& ev) {
                            HandleN2K_127250(UnpackEvtPointer<Nmea2000Msg>(ev));
                          });

  // GNSS Satellites in View   PGN 129540
  m_n2k_129540_lstnr.Init(Nmea2000Msg(static_cast<uint64_t>(129540)),
                          [&](const ObservedEvt& ev) {
                            HandleN2K_129540(UnpackEvtPointer<Nmea2000Msg>(ev));
                          });

  // NMEA0183
  // RMC
  Nmea0183Msg n0183_msg_RMC("RMC");
  m_n0183_rmc_lstnr.Init(Nmea0183Msg("RMC"), [&](const ObservedEvt& ev) {
    HandleN0183_RMC(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // HDT
  m_n0183_hdt_lstnr.Init(Nmea0183Msg("HDT"), [&](const ObservedEvt& ev) {
    HandleN0183_HDT(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // HDG
  m_n0183_hdg_lstnr.Init(Nmea0183Msg("HDG"), [&](const ObservedEvt& ev) {
    HandleN0183_HDG(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // HDM
  m_n0183_hdm_lstnr.Init(Nmea0183Msg("HDM"), [&](const ObservedEvt& ev) {
    HandleN0183_HDM(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // VTG
  m_n0183_vtg_lstnr.Init(Nmea0183Msg("VTG"), [&](const ObservedEvt& ev) {
    HandleN0183_VTG(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // GSV
  m_n0183_gsv_lstnr.Init(Nmea0183Msg("GSV"), [&](const ObservedEvt& ev) {
    HandleN0183_GSV(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // GGA
  m_n0183_gga_lstnr.Init(Nmea0183Msg("GGA"), [&](const ObservedEvt& ev) {
    HandleN0183_GGA(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // GLL
  Nmea0183Msg n0183_msg_GLL("GLL");
  m_n0183_gll_lstnr.Init(Nmea0183Msg("GLL"), [&](const ObservedEvt& ev) {
    HandleN0183_GLL(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // AIVDO
  m_n0183_aivdo_lstnr.Init(Nmea0183Msg("AIVDO"), [&](const ObservedEvt& ev) {
    HandleN0183_AIVDO(UnpackEvtPointer<Nmea0183Msg>(ev));
  });

  // SignalK
  m_signal_k_lstnr.Init(SignalkMsg(), [&](const ObservedEvt& ev) {
    HandleSignalK(UnpackEvtPointer<SignalkMsg>(ev));
  });
}

void CommBridge::OnDriverStateChange() {
  // Reset all active priority states
  PresetPriorityContainers();
}

std::vector<string> CommBridge::GetPriorityMaps() const {
  std::vector<string> result;
  result.push_back(GetPriorityMap(priority_map_position));
  result.push_back(GetPriorityMap(priority_map_velocity));
  result.push_back(GetPriorityMap(priority_map_heading));
  result.push_back(GetPriorityMap(priority_map_variation));
  result.push_back(GetPriorityMap(priority_map_satellites));
  return result;
}

void CommBridge::ApplyPriorityMaps(const std::vector<string>& new_maps) {
  wxString new_prio_string = wxString(new_maps[0].c_str());
  ApplyPriorityMap(priority_map_position, new_prio_string, 0);

  new_prio_string = wxString(new_maps[1].c_str());
  ApplyPriorityMap(priority_map_velocity, new_prio_string, 1);

  new_prio_string = wxString(new_maps[2].c_str());
  ApplyPriorityMap(priority_map_heading, new_prio_string, 2);

  new_prio_string = wxString(new_maps[3].c_str());
  ApplyPriorityMap(priority_map_variation, new_prio_string, 3);

  new_prio_string = wxString(new_maps[4].c_str());
  ApplyPriorityMap(priority_map_satellites, new_prio_string, 4);
}

void CommBridge::PresetPriorityContainers() {
  PresetPriorityContainer(active_priority_position, priority_map_position);
  PresetPriorityContainer(active_priority_velocity, priority_map_velocity);
  PresetPriorityContainer(active_priority_heading, priority_map_heading);
  PresetPriorityContainer(active_priority_variation, priority_map_variation);
  PresetPriorityContainer(active_priority_satellites, priority_map_satellites);
}

bool CommBridge::HandleN2K_129029(const N2000MsgPtr& n2k_msg) {
  std::vector<unsigned char> v = n2k_msg->payload;

  // extract and verify PGN
  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodePGN129029(v, temp_data)) return false;

  int valid_flag = 0;
  if (!N2kIsNA(temp_data.gLat) && !N2kIsNA(temp_data.gLon)) {
    if (EvalPriority(n2k_msg, active_priority_position,
                     priority_map_position)) {
      gLat = temp_data.gLat;
      gLon = temp_data.gLon;
      valid_flag += POS_UPDATE;
      valid_flag += POS_VALID;
      m_watchdogs.position_watchdog = gps_watchdog_timeout_ticks;
      m_n_log_watchdog_period = N_ACTIVE_LOG_WATCHDOG;  // allow faster dog log
    }
  }

  if (temp_data.n_satellites >= 0) {
    if (EvalPriority(n2k_msg, active_priority_satellites,
                     priority_map_satellites)) {
      g_SatsInView = temp_data.n_satellites;
      g_bSatValid = true;
      m_watchdogs.satellite_watchdog = sat_watchdog_timeout_ticks;
    }
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN2K_129025(const N2000MsgPtr& n2k_msg) {
  std::vector<unsigned char> v = n2k_msg->payload;

  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodePGN129025(v, temp_data)) return false;

  int valid_flag = 0;
  if (!N2kIsNA(temp_data.gLat) && !N2kIsNA(temp_data.gLon)) {
    if (EvalPriority(n2k_msg, active_priority_position,
                     priority_map_position)) {
      gLat = temp_data.gLat;
      gLon = temp_data.gLon;
      valid_flag += POS_UPDATE;
      valid_flag += POS_VALID;
      m_watchdogs.position_watchdog = gps_watchdog_timeout_ticks;
      m_n_log_watchdog_period = N_ACTIVE_LOG_WATCHDOG;  // allow faster dog log
    }
  }
  // FIXME (dave) How to notify user of errors?
  else {
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN2K_129026(const N2000MsgPtr& n2k_msg) {
  std::vector<unsigned char> v = n2k_msg->payload;

  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodePGN129026(v, temp_data)) return false;

  int valid_flag = 0;
  if (!N2kIsNA(temp_data.gSog)) {  // gCog as reported by net may be NaN, but OK
    if (EvalPriority(n2k_msg, active_priority_velocity,
                     priority_map_velocity)) {
      gSog = MS2KNOTS(temp_data.gSog);
      valid_flag += SOG_UPDATE;

      if (N2kIsNA(temp_data.gCog))
        gCog = NAN;
      else
        gCog = GeodesicRadToDeg(temp_data.gCog);
      valid_flag += COG_UPDATE;
      m_watchdogs.velocity_watchdog = gps_watchdog_timeout_ticks;
    }
  } else {
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN2K_127250(const N2000MsgPtr& n2k_msg) {
  std::vector<unsigned char> v = n2k_msg->payload;

  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodePGN127250(v, temp_data)) return false;

  int valid_flag = 0;
  if (!N2kIsNA(temp_data.gVar)) {
    if (EvalPriority(n2k_msg, active_priority_variation,
                     priority_map_variation)) {
      gVar = GeodesicRadToDeg(temp_data.gVar);
      valid_flag += VAR_UPDATE;
      m_watchdogs.variation_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  if (!N2kIsNA(temp_data.gHdt)) {
    if (EvalPriority(n2k_msg, active_priority_heading, priority_map_heading)) {
      gHdt = GeodesicRadToDeg(temp_data.gHdt);
      valid_flag += HDT_UPDATE;
      m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  if (!N2kIsNA(temp_data.gHdm)) {
    gHdm = GeodesicRadToDeg(temp_data.gHdm);
    if (EvalPriority(n2k_msg, active_priority_heading, priority_map_heading)) {
      MakeHDTFromHDM();
      valid_flag += HDT_UPDATE;
      if (!std::isnan(gHdt))
        m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN2K_129540(const N2000MsgPtr& n2k_msg) {
  std::vector<unsigned char> v = n2k_msg->payload;

  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodePGN129540(v, temp_data)) return false;

  if (temp_data.n_satellites >= 0) {
    if (EvalPriority(n2k_msg, active_priority_satellites,
                     priority_map_satellites)) {
      g_SatsInView = temp_data.n_satellites;
      g_bSatValid = true;
      m_watchdogs.satellite_watchdog = sat_watchdog_timeout_ticks;
    }
  }

  return true;
}

bool CommBridge::HandleN0183_RMC(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;

  NavData temp_data;
  ClearNavData(temp_data);

  bool is_valid = true;
  if (!m_decoder.DecodeRMC(str, temp_data)) is_valid = false;

  if (std::isnan(temp_data.gLat) || std::isnan(temp_data.gLon)) return false;

  int valid_flag = 0;
  if (EvalPriority(n0183_msg, active_priority_position,
                   priority_map_position)) {
    if (is_valid) {
      gLat = temp_data.gLat;
      gLon = temp_data.gLon;
      valid_flag += POS_VALID;
      m_watchdogs.position_watchdog = gps_watchdog_timeout_ticks;
      m_n_log_watchdog_period = N_ACTIVE_LOG_WATCHDOG;  // allow faster dog log
    }
    valid_flag += POS_UPDATE;
  }

  if (EvalPriority(n0183_msg, active_priority_velocity,
                   priority_map_velocity)) {
    if (is_valid) {
      gSog = temp_data.gSog;
      valid_flag += SOG_UPDATE;
      gCog = temp_data.gCog;
      valid_flag += COG_UPDATE;
      m_watchdogs.velocity_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  if (!std::isnan(temp_data.gVar)) {
    if (EvalPriority(n0183_msg, active_priority_variation,
                     priority_map_variation)) {
      if (is_valid) {
        gVar = temp_data.gVar;
        valid_flag += VAR_UPDATE;
        m_watchdogs.variation_watchdog = gps_watchdog_timeout_ticks;
      }
    }
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN0183_HDT(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;
  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodeHDT(str, temp_data)) return false;

  int valid_flag = 0;
  if (EvalPriority(n0183_msg, active_priority_heading, priority_map_heading)) {
    gHdt = temp_data.gHdt;
    valid_flag += HDT_UPDATE;
    m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN0183_HDG(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;
  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodeHDG(str, temp_data)) return false;

  int valid_flag = 0;

  bool bHDM = false;
  if (EvalPriority(n0183_msg, active_priority_heading, priority_map_heading)) {
    gHdm = temp_data.gHdm;
    m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
    bHDM = true;
  }

  if (!std::isnan(temp_data.gVar)) {
    if (EvalPriority(n0183_msg, active_priority_variation,
                     priority_map_variation)) {
      gVar = temp_data.gVar;
      valid_flag += VAR_UPDATE;
      m_watchdogs.variation_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  if (bHDM) MakeHDTFromHDM();

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN0183_HDM(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;
  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodeHDM(str, temp_data)) return false;

  int valid_flag = 0;
  if (EvalPriority(n0183_msg, active_priority_heading, priority_map_heading)) {
    gHdm = temp_data.gHdm;
    MakeHDTFromHDM();
    valid_flag += HDT_UPDATE;
    m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN0183_VTG(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;
  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodeVTG(str, temp_data)) return false;

  int valid_flag = 0;
  if (EvalPriority(n0183_msg, active_priority_velocity,
                   priority_map_velocity)) {
    gSog = temp_data.gSog;
    valid_flag += SOG_UPDATE;
    gCog = temp_data.gCog;
    valid_flag += COG_UPDATE;
    m_watchdogs.velocity_watchdog = gps_watchdog_timeout_ticks;
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN0183_GSV(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;
  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodeGSV(str, temp_data)) return false;

  int valid_flag = 0;
  if (EvalPriority(n0183_msg, active_priority_satellites,
                   priority_map_satellites)) {
    if (temp_data.n_satellites >= 0) {
      g_SatsInView = temp_data.n_satellites;
      g_bSatValid = true;

      m_watchdogs.satellite_watchdog = sat_watchdog_timeout_ticks;
    }
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN0183_GGA(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;
  NavData temp_data;
  ClearNavData(temp_data);

  bool is_valid = true;
  if (!m_decoder.DecodeGGA(str, temp_data)) is_valid = false;

  if (std::isnan(temp_data.gLat) || std::isnan(temp_data.gLon)) return false;

  int valid_flag = 0;
  if (EvalPriority(n0183_msg, active_priority_position,
                   priority_map_position)) {
    if (is_valid) {
      gLat = temp_data.gLat;
      gLon = temp_data.gLon;
      valid_flag += POS_VALID;
      m_watchdogs.position_watchdog = gps_watchdog_timeout_ticks;
      m_n_log_watchdog_period = N_ACTIVE_LOG_WATCHDOG;  // allow faster dog log
    }
    valid_flag += POS_UPDATE;
  }

  if (EvalPriority(n0183_msg, active_priority_satellites,
                   priority_map_satellites)) {
    if (is_valid) {
      if (temp_data.n_satellites >= 0) {
        g_SatsInView = temp_data.n_satellites;
        g_bSatValid = true;

        m_watchdogs.satellite_watchdog = sat_watchdog_timeout_ticks;
      }
    }
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN0183_GLL(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;
  NavData temp_data;
  ClearNavData(temp_data);

  bool bvalid = true;
  if (!m_decoder.DecodeGLL(str, temp_data)) bvalid = false;

  if (std::isnan(temp_data.gLat) || std::isnan(temp_data.gLon)) return false;

  int valid_flag = 0;
  if (EvalPriority(n0183_msg, active_priority_position,
                   priority_map_position)) {
    if (bvalid) {
      gLat = temp_data.gLat;
      gLon = temp_data.gLon;
      valid_flag += POS_VALID;
      m_watchdogs.position_watchdog = gps_watchdog_timeout_ticks;
      m_n_log_watchdog_period = N_ACTIVE_LOG_WATCHDOG;  // allow faster dog log
    }
    valid_flag += POS_UPDATE;
  }

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

bool CommBridge::HandleN0183_AIVDO(const N0183MsgPtr& n0183_msg) {
  string str = n0183_msg->payload;

  GenericPosDatEx gpd;
  wxString sentence(str.c_str());

  AisError ais_error = AIS_GENERIC_ERROR;
  ais_error = DecodeSingleVDO(sentence, &gpd);

  if (ais_error == AIS_NoError) {
    int valid_flag = 0;
    if (!std::isnan(gpd.kLat) && !std::isnan(gpd.kLon)) {
      if (EvalPriority(n0183_msg, active_priority_position,
                       priority_map_position)) {
        gLat = gpd.kLat;
        gLon = gpd.kLon;
        valid_flag += POS_UPDATE;
        valid_flag += POS_VALID;
        m_watchdogs.position_watchdog = gps_watchdog_timeout_ticks;
        m_n_log_watchdog_period =
            N_ACTIVE_LOG_WATCHDOG;  // allow faster dog log
      }
    }

    if (!std::isnan(gpd.kCog) && !std::isnan(gpd.kSog)) {
      if (EvalPriority(n0183_msg, active_priority_velocity,
                       priority_map_velocity)) {
        gSog = gpd.kSog;
        valid_flag += SOG_UPDATE;
        gCog = gpd.kCog;
        valid_flag += COG_UPDATE;
        m_watchdogs.velocity_watchdog = gps_watchdog_timeout_ticks;
      }
    }

    if (!std::isnan(gpd.kHdt)) {
      if (EvalPriority(n0183_msg, active_priority_heading,
                       priority_map_heading)) {
        gHdt = gpd.kHdt;
        valid_flag += HDT_UPDATE;
        m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
      }
    }

    SendBasicNavdata(valid_flag, m_log_callbacks);
  }
  return true;
}

bool CommBridge::HandleSignalK(const SignalKMsgPtr& sK_msg) {
  string str = sK_msg->raw_message;

  //  Here we ignore messages involving contexts other than ownship
  if (sK_msg->context_self != sK_msg->context) return false;

  g_ownshipMMSI_SK = sK_msg->context_self;

  NavData temp_data;
  ClearNavData(temp_data);

  if (!m_decoder.DecodeSignalK(str, temp_data)) return false;

  int valid_flag = 0;

  if (!std::isnan(temp_data.gLat) && !std::isnan(temp_data.gLon)) {
    if (EvalPriority(sK_msg, active_priority_position, priority_map_position)) {
      gLat = temp_data.gLat;
      gLon = temp_data.gLon;
      valid_flag += POS_UPDATE;
      valid_flag += POS_VALID;
      m_watchdogs.position_watchdog = gps_watchdog_timeout_ticks;
      m_n_log_watchdog_period = N_ACTIVE_LOG_WATCHDOG;  // allow faster dog log
    }
  }

  if (!std::isnan(temp_data.gSog)) {
    if (EvalPriority(sK_msg, active_priority_velocity, priority_map_velocity)) {
      gSog = temp_data.gSog;
      valid_flag += SOG_UPDATE;
      if ((gSog > 0.05) && !std::isnan(temp_data.gCog)) {
        gCog = temp_data.gCog;
        valid_flag += COG_UPDATE;
      }
      m_watchdogs.velocity_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  if (!std::isnan(temp_data.gHdt)) {
    if (EvalPriority(sK_msg, active_priority_heading, priority_map_heading)) {
      gHdt = temp_data.gHdt;
      valid_flag += HDT_UPDATE;
      m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  if (!std::isnan(temp_data.gHdm)) {
    if (EvalPriority(sK_msg, active_priority_heading, priority_map_heading)) {
      gHdm = temp_data.gHdm;
      MakeHDTFromHDM();
      valid_flag += HDT_UPDATE;
      m_watchdogs.heading_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  if (!std::isnan(temp_data.gVar)) {
    if (EvalPriority(sK_msg, active_priority_variation,
                     priority_map_variation)) {
      gVar = temp_data.gVar;
      valid_flag += VAR_UPDATE;
      m_watchdogs.variation_watchdog = gps_watchdog_timeout_ticks;
    }
  }

  if (temp_data.n_satellites > 0) {
    if (EvalPriority(sK_msg, active_priority_satellites,
                     priority_map_satellites)) {
      g_SatsInView = temp_data.n_satellites;
      g_bSatValid = true;
      m_watchdogs.satellite_watchdog = sat_watchdog_timeout_ticks;
    }
  }

  if (g_pMUX && g_pMUX->IsLogActive())
    g_pMUX->LogInputMessage(sK_msg, false, false);

  SendBasicNavdata(valid_flag, m_log_callbacks);
  return true;
}

void CommBridge::ClearPriorityMaps() {
  priority_map_position.clear();
  priority_map_velocity.clear();
  priority_map_heading.clear();
  priority_map_variation.clear();
  priority_map_satellites.clear();
}

PriorityContainer& CommBridge::GetPriorityContainer(const string& category) {
  if (category == "position")
    return active_priority_position;
  else if (category == "velocity")
    return active_priority_velocity;
  else if (category == "heading")
    return active_priority_heading;
  else if (category == "variation")
    return active_priority_variation;
  else if (category == "satellites")
    return active_priority_satellites;
  else
    return active_priority_void;
}

void CommBridge::UpdateAndApplyMaps(const std::vector<string>& new_maps) {
  ApplyPriorityMaps(new_maps);
  SaveConfig();
  PresetPriorityContainers();
}

bool CommBridge::LoadConfig() {
  if (TheBaseConfig()) {
    TheBaseConfig()->SetPath("/Settings/CommPriority");

    std::vector<string> new_maps;
    wxString pri_string;

    TheBaseConfig()->Read("PriorityPosition", &pri_string);
    string s_prio = string(pri_string.c_str());
    new_maps.push_back(s_prio);

    TheBaseConfig()->Read("PriorityVelocity", &pri_string);
    s_prio = string(pri_string.c_str());
    new_maps.push_back(s_prio);

    TheBaseConfig()->Read("PriorityHeading", &pri_string);
    s_prio = string(pri_string.c_str());
    new_maps.push_back(s_prio);

    TheBaseConfig()->Read("PriorityVariation", &pri_string);
    s_prio = string(pri_string.c_str());
    new_maps.push_back(s_prio);

    TheBaseConfig()->Read("PrioritySatellites", &pri_string);
    s_prio = string(pri_string.c_str());
    new_maps.push_back(s_prio);

    ApplyPriorityMaps(new_maps);
  }
  return true;
}

bool CommBridge::SaveConfig() const {
  if (TheBaseConfig()) {
    TheBaseConfig()->SetPath("/Settings/CommPriority");

    wxString pri_string =
        wxString(GetPriorityMap(priority_map_position).c_str());
    TheBaseConfig()->Write("PriorityPosition", pri_string);

    pri_string = wxString(GetPriorityMap(priority_map_velocity).c_str());
    TheBaseConfig()->Write("PriorityVelocity", pri_string);

    pri_string = wxString(GetPriorityMap(priority_map_heading).c_str());
    TheBaseConfig()->Write("PriorityHeading", pri_string);

    pri_string = wxString(GetPriorityMap(priority_map_variation).c_str());
    TheBaseConfig()->Write("PriorityVariation", pri_string);

    pri_string = wxString(GetPriorityMap(priority_map_satellites).c_str());
    TheBaseConfig()->Write("PrioritySatellites", pri_string);
  }
  return true;
}

bool CommBridge::EvalPriority(const NavMsgPtr& msg,
                              PriorityContainer& active_priority,
                              PriorityMap& priority_map) {
  string this_key = GetPriorityKey(msg);
  if (debug_priority) printf("This Key: %s\n", this_key.c_str());

  // Pull some identifiers from the unique key
  wxStringTokenizer tkz(this_key, _T(";"));
  wxString wxs_this_source = tkz.GetNextToken();
  string source = wxs_this_source.ToStdString();
  wxString wxs_this_identifier = tkz.GetNextToken();
  string this_identifier = wxs_this_identifier.ToStdString();

  wxStringTokenizer tka(wxs_this_source, _T(":"));
  tka.GetNextToken();
  std::stringstream ss;
  ss << tka.GetNextToken();
  int source_address;
  ss >> source_address;

  // Special case priority value linkage for N0183 messages:
  // If this is a "velocity" record, ensure that a "position"
  // report has been accepted from the same source before accepting the
  // velocity record.
  // This ensures that the data source is fully initialized, and is reporting
  // valid, sensible velocity data.
  if (msg->bus == NavAddr::Bus::N0183) {
    if (!strncmp(active_priority.prio_class.c_str(), "velocity", 8)) {
      bool pos_ok = false;
      if (!strcmp(active_priority_position.active_source.c_str(),
                  source.c_str())) {
        if (active_priority_position.recent_active_time != -1) {
          pos_ok = true;
        }
      }
      if (!pos_ok) return false;
    }
  }

  // Fetch the established priority for the message
  int this_priority;

  auto it = priority_map.find(this_key);
  if (it == priority_map.end()) {
    // Not found, so make it default the lowest priority
    size_t n = priority_map.size();
    priority_map[this_key] = static_cast<int>(n);
  }

  this_priority = priority_map[this_key];

  if (debug_priority) {
    for (const auto& jt : priority_map) {
      printf("               priority_map:  %s  %d\n", jt.first.c_str(),
             jt.second);
    }
  }

  // Incoming message priority lower than currently active priority?
  //   If so, drop the message
  if (this_priority > active_priority.active_priority) {
    if (debug_priority)
      printf("      Drop low priority: %s %d %d \n", source.c_str(),
             this_priority, active_priority.active_priority);
    return false;
  }

  // A channel returning, after watchdog time out.
  if (this_priority < active_priority.active_priority) {
    active_priority.active_priority = this_priority;
    active_priority.active_source = source;
    active_priority.active_identifier = this_identifier;
    active_priority.active_source_address = source_address;
    wxDateTime now = wxDateTime::Now();
    active_priority.recent_active_time = now.GetTicks();

    if (debug_priority)
      printf("  Restoring high priority: %s %d\n", source.c_str(),
             this_priority);
    return true;
  }

  // Do we see two sources with the same priority?
  // If so, we take the first one, and deprioritize this one.

  if (!active_priority.active_source.empty()) {
    if (debug_priority) printf("source: %s\n", source.c_str());
    if (debug_priority)
      printf("active_source: %s\n", active_priority.active_source.c_str());

    if (source != active_priority.active_source) {
      // Auto adjust the priority of these this message down
      // First, find the lowest priority in use in this map
      int lowest_priority = -10;  // safe enough
      for (const auto& jt : priority_map) {
        if (jt.second > lowest_priority) lowest_priority = jt.second;
      }

      priority_map[this_key] = lowest_priority + 1;
      if (debug_priority)
        printf("          Lowering priority A: %s :%d\n", source.c_str(),
               priority_map[this_key]);
      return false;
    }
  }

  //  For N0183 message, has the Mnemonic (id) changed?
  //  Example:  RMC and AIVDO from same source.

  if (msg->bus == NavAddr::Bus::N0183) {
    auto msg_0183 = std::dynamic_pointer_cast<const Nmea0183Msg>(msg);
    if (msg_0183) {
      if (!active_priority.active_identifier.empty()) {
        if (debug_priority)
          printf("this_identifier: %s\n", this_identifier.c_str());
        if (debug_priority)
          printf("active_priority.active_identifier: %s\n",
                 active_priority.active_identifier.c_str());

        if (this_identifier != active_priority.active_identifier) {
          // if necessary, auto adjust the priority of this message down
          // and drop it
          if (priority_map[this_key] == active_priority.active_priority) {
            int lowest_priority = -10;  // safe enough
            for (const auto& jt : priority_map) {
              if (jt.second > lowest_priority) lowest_priority = jt.second;
            }

            priority_map[this_key] = lowest_priority + 1;
            if (debug_priority)
              printf("          Lowering priority B: %s :%d\n", source.c_str(),
                     priority_map[this_key]);
          }

          return false;
        }
      }
    }
  }

  //  Similar for n2k PGN...

  else if (msg->bus == NavAddr::Bus::N2000) {
    auto msg_n2k = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
    if (msg_n2k) {
      if (!active_priority.active_identifier.empty()) {
        if (this_identifier != active_priority.active_identifier) {
          // if necessary, auto adjust the priority of this message down
          // and drop it
          if (priority_map[this_key] == active_priority.active_priority) {
            int lowest_priority = -10;  // safe enough
            for (const auto& jt : priority_map) {
              if (jt.second > lowest_priority) lowest_priority = jt.second;
            }

            priority_map[this_key] = lowest_priority + 1;
            if (debug_priority)
              printf("          Lowering priority: %s :%d\n", source.c_str(),
                     priority_map[this_key]);
          }

          return false;
        }
      }
    }
  }

  // Update the records
  active_priority.active_source = source;
  active_priority.active_identifier = this_identifier;
  active_priority.active_source_address = source_address;
  wxDateTime now = wxDateTime::Now();
  active_priority.recent_active_time = now.GetTicks();
  if (debug_priority)
    printf("  Accepting high priority: %s %d\n", source.c_str(), this_priority);

  if (active_priority.prio_class == "position") {
    if (this_priority != m_last_position_priority) {
      m_last_position_priority = this_priority;

      wxString msg_;
      msg_.Printf(_("GNSS position fix priority shift:") + " %s\n %s \n -> %s",
                  this_identifier.c_str(), m_last_position_source.c_str(),
                  source.c_str());
      auto& noteman = NotificationManager::GetInstance();
      noteman.AddNotification(NotificationSeverity::kInformational,
                              msg_.ToStdString());
    }
    m_last_position_source = source;
  }

  return true;
}
