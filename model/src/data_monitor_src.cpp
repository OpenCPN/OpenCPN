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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement data_monitor_src.h -- Provide a data stream of input messages
 * for the Data Monitor.
 */

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <wx/event.h>
#include <wx/log.h>

#include "model/comm_drv_registry.h"
#include "model/comm_navmsg.h"
#include "model/comm_navmsg_bus.h"
#include "model/data_monitor_src.h"
#include "model/ocpn_utils.h"

#include "observable.h"

static void InitListener(ObsListener& ol, NavMsg& msg,
                         const std::function<void(ObservedEvt)>& on_message) {
  ol.Init(msg, on_message);
}

DataMonitorSrc::DataMonitorSrc(const SinkFunc& sink_func)
    : m_sink_func(sink_func) {
  new_msg_lstnr.Init(NavMsgBus::GetInstance().new_msg_event,
                     [&](ObservedEvt&) { OnNewMessage(); });
  undelivered_msg_lstnr.Init(CommDriverRegistry::GetInstance().evt_dropped_msg,
                             [&](ObservedEvt& ev) { OnMessage(ev); });
}

void DataMonitorSrc::OnNewMessage() {
  auto messages = NavMsgBus::GetInstance().GetActiveMessages();
  for (const auto& msg : messages) {
    auto found = m_listeners.find(msg);
    if (found == m_listeners.end()) {
      ObsListener listener;
      std::string type(msg);
      size_t pos;
      NavAddr::Bus bus = NavAddr::Bus::Undef;
      if ((pos = type.find("::")) != std::string::npos) {
        auto bus_str = type.substr(0, pos);
        bus = NavAddr::StringToBus(bus_str);
        type = type.substr(pos + 2);
        if ((pos = type.find('-')) != std::string::npos)
          type = type.substr(pos + 1);
        m_listeners[msg] = std::move(listener);
        std::function<void(ObservedEvt)> listen_action = [&](ObservedEvt ev) {
          OnMessage(ev);
        };
        switch (bus) {
          case NavAddr::Bus::N0183: {
            auto type_msg = Nmea0183Msg(type);
            InitListener(m_listeners[msg], type_msg, listen_action);
          } break;
          case NavAddr::Bus::N2000: {
            try {
              auto type_msg = Nmea2000Msg(std::stoi(type));
              InitListener(m_listeners[msg], type_msg, listen_action);
            } catch (...) {
              wxLogMessage("Bad Nmea2000 type: %s", type.c_str());
            }
          } break;
          case NavAddr::Bus::Plugin: {
            auto type_msg = PluginMsg(type, "");
            InitListener(m_listeners[msg], type_msg, listen_action);
          } break;
          case NavAddr::Bus::Signalk: {
            auto type_msg = SignalkMsg();
            InitListener(m_listeners[msg], type_msg, listen_action);
          } break;
          default:
            break;  // Just ignore other types.
        }
      }
    }
  }
}

void DataMonitorSrc::OnMessage(ObservedEvt& ev) {
  auto ptr = UnpackEvtPointer<NavMsg>(ev);
  m_sink_func(ptr);
}
