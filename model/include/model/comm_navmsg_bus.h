/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 * Raw messages layer, supports sending and recieving navmsg
 * messages. This is the second layer in the three tier model
 * drivers, raw messages and application messages.
 */

#ifndef NAVMSG_BUS_H_
#define NAVMSG_BUS_H_

#include <memory>
#include <mutex>
#include <set>
#include <string>

#include <wx/event.h>

#include "model/comm_driver.h"
#include "observable_evtvar.h"

/** The raw message layer, a singleton. */
class NavMsgBus : public wxEvtHandler, public DriverListener {
public:
  /* Singleton implementation. */
  static NavMsgBus& GetInstance();

  NavMsgBus& operator=(NavMsgBus&) = delete;
  NavMsgBus(const NavMsgBus&) = delete;

  /** Send a message to given destination using suitable driver. */
  void SendMessage(std::shared_ptr<const NavMsg> message,
                   std::shared_ptr<const NavAddr> address);

  /**
   * Register a message type in list the GetActiveMessages() list
   * @return true if key is inserted, false if already registered.
   */
  bool RegisterKey(const std::string& key);

  /** Accept message received by driver, make it available for upper layers. */
  void Notify(std::shared_ptr<const NavMsg> message);

  /* DriverListener implementation: */
  void Notify(const AbstractCommDriver& driver);

  /** Return list of message types sent or received. */
  const std::set<std::string>& GetActiveMessages() { return m_active_messages; }

  /** Notified without data when new message type(s) are detected. */
  EventVar new_msg_event;

private:
  std::mutex m_mutex;
  NavMsgBus() = default;

  std::set<std::string> m_active_messages;
};

#endif  // NAVMSG_BUS_H_
