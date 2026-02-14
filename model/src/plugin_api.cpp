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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 *  \file
 *
 *  Implement various non-gui ocpn_plugin.h methods.
 */

#include <memory>
#include <sstream>
#include <vector>

#include <wx/event.h>
#include <wx/fileconf.h>
#include <wx/jsonval.h>
#include <wx/jsonreader.h>
#include <wx/tokenzr.h>

#include "model/base_platform.h"
#include "model/comm_appmsg.h"
#include "model/comm_drv_loopback.h"
#include "model/comm_drv_n0183_net.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_drv_n2k.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/notification_manager.h"
#include "ocpn_plugin.h"
#include "model/comm_drv_factory.h"
#include "model/comm_drv_n2k_net.h"
#include "model/comm_drv_n2k_serial.h"

using namespace std;

vector<uint8_t> GetN2000Payload(NMEA2000Id id, ObservedEvt ev) {
  auto msg = UnpackEvtPointer<Nmea2000Msg>(ev);
  return msg->payload;
}

std::string GetN2000Source(NMEA2000Id id, ObservedEvt ev) {
  auto msg = UnpackEvtPointer<Nmea2000Msg>(ev);
  return msg->source->to_string();
}

std::string GetN0183Payload(NMEA0183Id id, ObservedEvt ev) {
  auto msg = UnpackEvtPointer<Nmea0183Msg>(ev);
  return msg->payload;
}

std::string GetPluginMsgPayload(PluginMsgId id, ObservedEvt ev) {
  auto msg = UnpackEvtPointer<PluginMsg>(ev);
  return msg->message;
}

std::shared_ptr<void> GetSignalkPayload(ObservedEvt ev) {
  auto msg = UnpackEvtPointer<SignalkMsg>(ev);
  wxJSONReader reader;
  wxJSONValue data;
  reader.Parse(wxString(msg->raw_message), &data);

  wxJSONValue root(wxJSONTYPE_OBJECT);
  root["Data"] = data;
  root["ErrorCount"] = reader.GetErrorCount();
  root["WarningCount"] = reader.GetWarningCount();

  root["Errors"] = wxJSONValue(wxJSONTYPE_ARRAY);
  for (size_t i = 0; i < reader.GetErrors().GetCount(); i++)
    root["Errors"].Append(reader.GetErrors().Item(i));

  root["Warnings"] = wxJSONValue(wxJSONTYPE_ARRAY);
  for (size_t i = 0; i < reader.GetWarnings().GetCount(); i++)
    root["Warnings"].Append(reader.GetWarnings().Item(i));

  root["Context"] = msg->context;
  root["ContextSelf"] = msg->context_self;

  return static_pointer_cast<void>(std::make_shared<wxJSONValue>(root));
}

std::shared_ptr<PI_Notification> GetNotificationMsgPayload(NotificationMsgId id,
                                                           ObservedEvt ev) {
  auto msg = UnpackEvtPointer<NotificationMsg>(ev);
  auto note = msg->notification;
  auto rv = std::make_shared<PI_Notification>(
      (PI_NotificationSeverity)note->GetSeverity(), note->GetMessage(),
      note->GetTimeoutStart(), note->GetTimeoutLeft(), note->GetGuid());
  rv->action_verb = msg->action_verb;
  return rv;
}

shared_ptr<ObservableListener> GetListener(NMEA2000Id id, wxEventType et,
                                           wxEvtHandler* eh) {
  return make_shared<ObservableListener>(Nmea2000Msg(id.id), eh, et);
}

std::shared_ptr<ObservableListener> GetListener(NMEA0183Id id, wxEventType et,
                                                wxEvtHandler* eh) {
  return make_shared<ObservableListener>(Nmea0183Msg(id.id), eh, et);
}

shared_ptr<ObservableListener> GetListener(SignalkId id, wxEventType et,
                                           wxEvtHandler* eh) {
  return make_shared<ObservableListener>(SignalkMsg(), eh, et);
}

shared_ptr<ObservableListener> GetListener(NavDataId id, wxEventType et,
                                           wxEvtHandler* eh) {
  return make_shared<ObservableListener>(BasicNavDataMsg(), eh, et);
}

std::shared_ptr<ObservableListener> GetListener(PluginMsgId id, wxEventType et,
                                                wxEvtHandler* eh) {
  return make_shared<ObservableListener>(PluginMsg(id.id, ""), eh, et);
}

std::shared_ptr<ObservableListener> GetListener(NotificationMsgId id,
                                                wxEventType et,
                                                wxEvtHandler* eh) {
  return make_shared<ObservableListener>(NotificationMsg(), eh, et);
}

PluginNavdata GetEventNavdata(ObservedEvt ev) {
  auto msg = UnpackEvtPointer<BasicNavDataMsg>(ev);
  PluginNavdata data;

  data.lat = msg->pos.lat;
  data.lon = msg->pos.lon;
  data.sog = msg->sog;
  data.cog = msg->cog;
  data.var = msg->var;
  data.hdt = msg->hdt;
  data.time = msg->time;
  return data;
}

/** Comm port plugin TX support methods  */

std::vector<DriverHandle> GetActiveDrivers() {
  std::vector<DriverHandle> result;

  auto& registry = CommDriverRegistry::GetInstance();
  const std::vector<DriverPtr>& drivers = registry.GetDrivers();

  for (auto& driver : drivers) result.push_back(driver->Key());

  return result;
}

const std::unordered_map<std::string, std::string> GetAttributes(
    DriverHandle handle) {
  auto& registry = CommDriverRegistry::GetInstance();
  auto& drivers = registry.GetDrivers();
  auto func = [handle](const DriverPtr d) { return d->Key() == handle; };
  AbstractCommDriver* found = nullptr;
  for (auto& d : drivers)
    if (d->Key() == handle) found = d.get();
  if (found)
    return found->GetAttributes();
  else
    return {};
}

CommDriverResult WriteCommDriver(
    DriverHandle handle, const std::shared_ptr<std::vector<uint8_t>>& payload) {
  // Find the driver from the handle
  auto& registry = CommDriverRegistry::GetInstance();
  auto& drivers = registry.GetDrivers();
  AbstractCommDriver* found = nullptr;
  for (auto& d : drivers)
    if (d->Key() == handle) found = d.get();
  if (!found) {
    return RESULT_COMM_INVALID_HANDLE;
  }

  // Determine protocol
  std::unordered_map<std::string, std::string> attributes =
      GetAttributes(handle);
  auto protocol_it = attributes.find("protocol");
  if (protocol_it == attributes.end()) return RESULT_COMM_INVALID_PARMS;
  std::string protocol = protocol_it->second;

  // This whole thing is a design collapse. If things were as they should,
  // all drivers should have the same interface and there should be no
  // need to handle different protocols separately. Part of the problem
  // is that there is no "internal" driver.
  const std::string msg(payload->begin(), payload->end());
  if (protocol == "nmea0183") {
    std::string id = msg.substr(1, 5);
    auto address = std::make_shared<NavAddr>();
    auto msg_out = std::make_shared<Nmea0183Msg>(id, msg, address);
    bool xmit_ok = found->SendMessage(msg_out, address);
    return xmit_ok ? RESULT_COMM_NO_ERROR : RESULT_COMM_TX_ERROR;
  } else if (protocol == "internal") {
    size_t space_pos = msg.find(" ");
    if (space_pos == std::string::npos) return RESULT_COMM_INVALID_PARMS;
    auto plugin_msg = std::make_shared<PluginMsg>(msg.substr(0, space_pos),
                                                  msg.substr(space_pos + 1));
    NavMsgBus::GetInstance().Notify(static_pointer_cast<NavMsg>(plugin_msg));
    return RESULT_COMM_NO_ERROR;
  } else if (protocol == "loopback") {
    auto navmsg = LoopbackDriver::ParsePluginMessage(msg);
    if (!navmsg) return RESULT_COMM_INVALID_PARMS;
    bool send_ok = found->SendMessage(navmsg, nullptr);
    return send_ok ? RESULT_COMM_NO_ERROR : RESULT_COMM_TX_ERROR;
  } else {
    return RESULT_COMM_INVALID_PARMS;
  }
}

CommDriverResult WriteCommDriverN2K(
    DriverHandle handle, int PGN, int destinationCANAddress, int priority,
    const std::shared_ptr<std::vector<uint8_t>>& payload) {
  uint64_t _PGN;
  _PGN = PGN;

  wxString m1;
  m1.Printf("WriteCommDriverN2K...  PGN: 0x%0X, Destination Address:  0x%0X",
            PGN, destinationCANAddress);
  wxLogMessage(m1);
  wxString m2 = "Payload: ";
  for (uint8_t& d : *payload) {
    wxString m3;
    m3.Printf(" 0x%02X", d);
    m2 += m3;
  }
  wxLogMessage(m2);

  // Find the driver from the handle
  auto& registry = CommDriverRegistry::GetInstance();
  auto& drivers = registry.GetDrivers();

  AbstractCommDriver* found(nullptr);
  for (auto& d : drivers)
    if (d->Key() == handle) found = d.get();
  if (!found) {
    return RESULT_COMM_INVALID_HANDLE;
  }
  auto dest_addr =
      std::make_shared<const NavAddr2000>(found->iface, destinationCANAddress);
  auto msg =
      std::make_shared<const Nmea2000Msg>(_PGN, *payload, dest_addr, priority);
  bool result = found->SendMessage(msg, dest_addr);

  return RESULT_COMM_NO_ERROR;
}

CommDriverResult RegisterTXPGNs(DriverHandle handle,
                                std::vector<int>& pgn_list) {
  if (!pgn_list.size()) return RESULT_COMM_INVALID_PARMS;

  // Find the driver from the handle
  auto& registry = CommDriverRegistry::GetInstance();
  auto& drivers = registry.GetDrivers();
  AbstractCommDriver* found(nullptr);
  for (auto& d : drivers)
    if (d->Key() == handle) found = d.get();

  if (!found) {
    return RESULT_COMM_INVALID_HANDLE;
  }
  auto dn2k = dynamic_cast<CommDriverN2K*>(found);

  int nloop = 0;
  for (size_t i = 0; i < pgn_list.size(); i++) {
    int nTry = 5;
    int iresult = -1;
    nloop = 0;
    while (nTry && iresult < 0) {
      iresult = dn2k->SetTXPGN(pgn_list[i]);
      nTry--;
      nloop++;
    }
    if (iresult < 0) {
      return RESULT_COMM_REGISTER_PGN_ERROR;
    }
  }
  return RESULT_COMM_NO_ERROR;
}

wxString* GetpPrivateApplicationDataLocation() {
  return g_BasePlatform->GetPrivateDataDirPtr();
}

void ReloadConfigConnections() {
  // Close and delete all active comm drivers
  auto& registry = CommDriverRegistry::GetInstance();
  registry.CloseAllDrivers();

  // Reload config file connections parameters.
  wxFileConfig* pConf = GetOCPNConfigObject();
  if (pConf) {
    TheConnectionParams().clear();
    pConf->SetPath("/Settings/NMEADataSource");

    wxString connectionconfigs;
    pConf->Read("DataConnections", &connectionconfigs);
    if (!connectionconfigs.IsEmpty()) {
      wxArrayString confs = wxStringTokenize(connectionconfigs, "|");
      for (size_t i = 0; i < confs.Count(); i++) {
        ConnectionParams* prm = new ConnectionParams(confs[i]);
        if (!prm->Valid) continue;
        TheConnectionParams().push_back(prm);
      }
    }
  }

  // Reconnect enabled connections
  for (auto* cp : TheConnectionParams()) {
    if (cp->bEnabled) {
      MakeCommDriver(cp);
      cp->b_IsSetup = TRUE;
    }
  }
}

/**
 * Plugin Notification Framework support
 */

PI_Notification::PI_Notification(PI_NotificationSeverity _severity,
                                 const std::string& _message,
                                 int _timeout_start, int _timeout_left,
                                 std::string _guid) {
  severity = _severity;
  message = _message;
  auto_timeout_start = _timeout_start;
  auto_timeout_left = _timeout_left;
  guid = _guid;
}

int GetActiveNotificationCount() {
  auto& noteman = NotificationManager::GetInstance();
  return noteman.GetNotificationCount();
}

PI_NotificationSeverity GetMaxActiveNotificationLevel() {
  auto& noteman = NotificationManager::GetInstance();
  if (noteman.GetNotificationCount())
    return (PI_NotificationSeverity)noteman.GetMaxSeverity();
  else
    return (PI_NotificationSeverity)-1;
}

std::string RaiseNotification(const PI_NotificationSeverity _severity,
                              const std::string& _message, int timeout_secs) {
  auto& noteman = NotificationManager::GetInstance();
  auto notification = std::make_shared<Notification>(
      (NotificationSeverity)_severity, _message, timeout_secs);
  return noteman.AddNotification(notification);
}

bool AcknowledgeNotification(const std::string& guid) {
  auto& noteman = NotificationManager::GetInstance();
  return noteman.AcknowledgeNotification(guid);
}

std::vector<std::shared_ptr<PI_Notification>> GetActiveNotifications() {
  auto& noteman = NotificationManager::GetInstance();
  std::vector<std::shared_ptr<PI_Notification>> pi_notes;
  for (auto note : noteman.GetNotifications()) {
    auto pi_note = std::make_shared<PI_Notification>(
        (PI_NotificationSeverity)note->GetSeverity(), note->GetMessage(),
        note->GetTimeoutStart(), note->GetTimeoutLeft(), note->GetGuid());
    pi_notes.push_back(pi_note);
  }

  return pi_notes;
}

/**
 * Plugin polled Comm Status support
 */
PI_Comm_Status GetConnState(const std::string& iface, PI_Conn_Bus _bus) {
  //  Translate API bus to internal NavAddr::Bus
  NavAddr::Bus ibus = NavAddr::Bus::Undef;
  switch (_bus) {
    case PI_Conn_Bus::N0183:
      ibus = NavAddr::Bus::N0183;
      break;

    case PI_Conn_Bus::Signalk:
      ibus = NavAddr::Bus::Signalk;
      break;

    case PI_Conn_Bus::N2000:
      ibus = NavAddr::Bus::N2000;
      break;

    default:
      break;
  }

  DriverStats stats;
  if (ibus != NavAddr::Bus::Undef) {
    auto& registry = CommDriverRegistry::GetInstance();
    auto& drivers = registry.GetDrivers();
    auto& found_driver = FindDriver(drivers, iface, ibus);
    if (found_driver) {
      auto stats_provider =
          dynamic_cast<DriverStatsProvider*>(found_driver.get());
      if (stats_provider) {
        stats = stats_provider->GetDriverStats();
      }
    }
  }

  PI_Comm_Status rv;
  if (stats.available) {
    if (stats.rx_count)
      rv.state = PI_Comm_State::Ok;
    else
      rv.state = PI_Comm_State::NoData;
  } else
    rv.state = PI_Comm_State::Unavailable;

  rv.rx_count = stats.rx_count;
  rv.tx_count = stats.tx_count;
  rv.error_count = stats.error_count;

  return rv;
}
