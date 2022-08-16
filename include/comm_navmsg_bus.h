/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Raw messages layer, supports sending and recieving navmsg
 *           messages. This is the second layer in the three tier model
 *           drivers, raw messages and application messages.
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

/* API handling raw partially decoded messages. */

#include <memory>
#include <vector>

#include <wx/event.h>
#include <wx/jsonreader.h>

#include "comm_driver.h"
#include "observable_msg.h"

#ifndef _NAVMSG_BUS_H__
#define _NAVMSG_BUS_H__

/** The raw message layer, a singleton. */
class NavMsgBus : public DriverListener {
public:
  /* Singleton implementation. */
  static NavMsgBus& GetInstance();

  NavMsgBus& operator=(NavMsgBus&) = delete;
  NavMsgBus(const NavMsgBus&) = delete;

  void SendMessage(const NavMsg& message, const NavAddr& address);

  /**
   * Return a listening object which generates wxEventType events sent to
   * wxEvtHandler when a given message type is received. The events
   * contains a shared_ptr<NavMsg>, use get_navmsg_ptr(event) to retrieve it.
   */
  ObservedVarListener GetListener(wxEventType et, wxEvtHandler* eh,
                                  const NavMsg& msg);

  void Notify(std::shared_ptr<const NavMsg> message);

  /* DriverListener implementation: */
  void Notify(const AbstractCommDriver& driver);

private:
  NavMsgBus() = default;
};

#endif  // NAVMSG_BUS_H
