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
#include <vector>

#include <wx/event.h>

#include "ocpn_plugin.h"
#include "comm_navmsg_bus.h"

std::unique_ptr<ObservedVarListener> GetListener(NMEA2000Id id, wxEventType ev,
                                                 wxEvtHandler* handler) {
  return std::make_unique<ObservedVarListener>(
      NavMsgBus::GetInstance().GetListener(ev, handler, Nmea2000Msg(id.id)));
}

std::unique_ptr<ObservedVarListener> GetListener(NMEA0183Id id, wxEventType ev,
                                                 wxEvtHandler* handler) {
  return std::make_unique<ObservedVarListener>(
      NavMsgBus::GetInstance().GetListener(ev, handler, Nmea0183Msg(id.id)));
}

std::unique_ptr<ObservedVarListener> GetListener(SignalkId id, wxEventType ev,
                                                 wxEvtHandler* handler) {
  return std::make_unique<ObservedVarListener>(
      NavMsgBus::GetInstance().GetListener(ev, handler, SignalkMsg()));
}

std::vector<uint8_t> GetN2000Payload(NMEA2000Id id, ObservedEvt ev) {
  auto msg = UnpackEvtPointer<Nmea2000Msg>(ev);
  return msg->payload;
}

std::string GetN0183Payload(NMEA0183Id id, ObservedEvt ev) {
  auto msg = UnpackEvtPointer<Nmea0183Msg>(ev);
  return msg->payload;
}
