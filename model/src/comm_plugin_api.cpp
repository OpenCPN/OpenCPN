/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement communications defined in ocpn_plugin.h
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
#include <memory>
#include <sstream>
#include <vector>

#include <wx/event.h>
#include <wx/jsonval.h>
#include <wx/jsonreader.h>

#include "model/base_platform.h"
#include "model/comm_appmsg.h"
#include "model/comm_drv_n0183_net.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_drv_n2k.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"

#include "ocpn_plugin.h"
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

shared_ptr<ObservableListener> GetListener(NMEA2000Id id, wxEventType et,
                                           wxEvtHandler* eh) {
  return make_shared<ObservableListener>(Nmea2000Msg(id.id), eh,
                                         et);
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
  const std::vector<std::shared_ptr<AbstractCommDriver>>& drivers =
      registry.GetDrivers();

  for (auto& driver : drivers) result.push_back(driver->Key());

  return result;
}

const std::unordered_map<std::string, std::string> GetAttributes(
    DriverHandle handle) {
  auto& registry = CommDriverRegistry::GetInstance();
  auto drivers = registry.GetDrivers();
  auto func = [handle](const DriverPtr d) { return d->Key() == handle; };
  auto found = std::find_if(drivers.begin(), drivers.end(), func);

  std::unordered_map<std::string, std::string> rv;
  if (found == drivers.end()) {
    return rv;
  }
  return found->get()->GetAttributes();
}

CommDriverResult WriteCommDriver(
    DriverHandle handle, const std::shared_ptr<std::vector<uint8_t>>& payload) {
  // Find the driver from the handle
  auto& registry = CommDriverRegistry::GetInstance();
  auto drivers = registry.GetDrivers();
  auto func = [handle](const DriverPtr d) { return d->Key() == handle; };
  auto found = std::find_if(drivers.begin(), drivers.end(), func);

  if (found == drivers.end()) {
    return RESULT_COMM_INVALID_HANDLE;
  }
  auto driver = *found;

  // Determine protocol
  std::unordered_map<std::string, std::string> attributes =
      GetAttributes(handle);
  auto protocol_it = attributes.find("protocol");
  if (protocol_it == attributes.end()) return RESULT_COMM_INVALID_PARMS;
  std::string protocol = protocol_it->second;

  if (protocol == "nmea0183") {
    auto d0183 = std::dynamic_pointer_cast<CommDriverN0183>(driver);

    std::string msg(payload->begin(), payload->end());
    std::string id = msg.substr(1, 5);
    auto address = std::make_shared<NavAddr0183>(d0183->iface);
    auto msg_out = std::make_shared<Nmea0183Msg>( id, msg, address);
    bool xmit_ok = d0183->SendMessage(msg_out, address);
    return xmit_ok ? RESULT_COMM_NO_ERROR : RESULT_COMM_TX_ERROR;
  } else
    return RESULT_COMM_INVALID_PARMS;
}

CommDriverResult WriteCommDriverN2K(
    DriverHandle handle, int PGN, int destinationCANAddress, int priority,
    const std::shared_ptr<std::vector<uint8_t>>& payload) {
  uint64_t _PGN;
  _PGN = PGN;

  // Find the driver from the handle
  auto& registry = CommDriverRegistry::GetInstance();
  auto drivers = registry.GetDrivers();
  auto func = [handle](const DriverPtr d) { return d->Key() == handle; };
  auto found = std::find_if(drivers.begin(), drivers.end(), func);

  if (found == drivers.end()) {
    return RESULT_COMM_INVALID_HANDLE;
  }
  auto driver = *found;
  auto dest_addr = std::make_shared<const NavAddr2000>(driver->iface,
                                                       destinationCANAddress);
  auto msg = std::make_shared<const Nmea2000Msg>(_PGN, *payload, dest_addr,
                                                 priority);
  bool result = driver->SendMessage(msg, dest_addr);

  return RESULT_COMM_NO_ERROR;
}

CommDriverResult RegisterTXPGNs(DriverHandle handle,
                                std::vector<int>& pgn_list) {
  if (!pgn_list.size()) return RESULT_COMM_INVALID_PARMS;

  // Find the driver from the handle
  auto& registry = CommDriverRegistry::GetInstance();
  auto drivers = registry.GetDrivers();
  auto func = [handle](const DriverPtr d) { return d->Key() == handle; };
  auto driver = std::find_if(drivers.begin(), drivers.end(), func);

  if (driver == drivers.end()) {
    return RESULT_COMM_INVALID_HANDLE;
  }
  auto dn2k = std::dynamic_pointer_cast<CommDriverN2K>(*driver);

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

wxString* GetpPrivateApplicationDataLocation(void) {
  return  g_BasePlatform->GetPrivateDataDirPtr();
}
