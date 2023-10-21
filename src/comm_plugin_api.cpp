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


#include "ocpn_plugin.h"
#include "comm_navmsg_bus.h"
#include "comm_appmsg.h"

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
