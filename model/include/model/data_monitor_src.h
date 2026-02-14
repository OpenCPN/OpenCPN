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
 * Provide a data stream of input messages for the Data Monitor. The
 * messages are intercepted before the multiplexer and have thus no state
 * as defined by the mux. All messages received, whether they are know to
 * the mux or not, are added to the stream
 */

#ifndef DATA_MONITOR_SRC__
#define DATA_MONITOR_SRC__

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <wx/event.h>

#include "model/comm_navmsg.h"
#include "model/comm_navmsg_bus.h"
#include "model/ocpn_utils.h"

#include "observable.h"

/**
 * Create a stream of input messages.  The
 * messages are intercepted before the multiplexer and have thus no state
 * as defined by the mux. All messages received, whether they are know to
 * the mux or not, are added to the stream
 * */
class DataMonitorSrc : public wxEvtHandler {
  /** Callback function used to forward data to caller. */
  using SinkFunc = std::function<void(const std::shared_ptr<const NavMsg>&)>;

public:
  /**
   *  Create instance which listens and forwards messages using the sink_func
   *  callback function.
   */
  DataMonitorSrc(const SinkFunc& sink_func);

private:
  SinkFunc m_sink_func;
  std::unordered_map<std::string, ObsListener> m_listeners;
  ObsListener new_msg_lstnr;
  ObsListener undelivered_msg_lstnr;

  /** Handle new message type detected. */
  void OnNewMessage();

  /** Handle incoming message. */
  void OnMessage(ObservedEvt& ev);
};

#endif  //  DATA_MONITOR_SRC__
