/***************************************************************************
 *   Copyright (C) 2025  Alec Leamas                                       *
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
 * Implement data_monitor_src.h
 */

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <wx/event.h>

#include "model/comm_navmsg.h"
#include "model/comm_navmsg_bus.h"
#include "model/ocpn_utils.h"

#include "observable.h"
#include "tty_scroll.h"

#include "data_monitor_src.h"

DataMonitorSrc::DataMonitorSrc(SinkFunc sink_func) : m_sink_func(sink_func) {
  ObsListener listener;
  m_listeners["AIVDM"] = std::move(listener);
  m_listeners["AIVDM"].Init(Nmea0183Msg("AIVDM"), [&](ObservedEvt& ev) {
    const std::shared_ptr<const Nmea0183Msg> ptr =
        UnpackEvtPointer<Nmea0183Msg>(ev);
    m_sink_func(ptr);
  });
  auto messages = NavMsgBus::GetInstance().GetActiveMessages();
  new_msg_lstnr.Init(NavMsgBus::GetInstance().new_msg_event,
                     [&](ObservedEvt&) { OnNewMessage(); });
}

void DataMonitorSrc::OnNewMessage() {
  auto messages = NavMsgBus::GetInstance().GetActiveMessages();
  for (const auto& msg : messages) {
    auto found = m_listeners.find(msg);
    if (found == m_listeners.end()) {
      ObsListener listener;
      std::string type(msg);
      ocpn::replace(type, "nmea0183::n0183-", "");
      m_listeners[msg] = std::move(listener);
      m_listeners[msg].Init(Nmea0183Msg(type),
                            [&](ObservedEvt& ev) { OnMessage(ev); });
    }
  };
}

void DataMonitorSrc::OnMessage(ObservedEvt& ev) {
  auto ptr = UnpackEvtPointer<Nmea0183Msg>(ev);
  if (ptr && ptr->payload != m_last_payload) {
    m_last_payload = ptr->payload;
    m_sink_func(ptr);
  }
}
