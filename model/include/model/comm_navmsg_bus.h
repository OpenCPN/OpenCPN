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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

/**
 * \file
 * Raw messages layer, supports sending and recieving navmsg
 * messages. This is the second layer in the three tier model
 * drivers, raw messages and application messages.
 */

#ifndef _NAVMSG_BUS_H__
#define _NAVMSG_BUS_H__

#include <memory>
#include <set>
#include <string>

#include "model/comm_driver.h"

/** The raw message layer, a singleton. */
class NavMsgBus : public DriverListener {
public:
  /* Singleton implementation. */
  static NavMsgBus& GetInstance();

  NavMsgBus& operator=(NavMsgBus&) = delete;
  NavMsgBus(const NavMsgBus&) = delete;

  /** Send a message to given destination using suitable driver. */
  void SendMessage(std::shared_ptr<const NavMsg> message,
                   std::shared_ptr<const NavAddr> address);

  /** Accept message received by driver, make it available for upper layers. */
  void Notify(std::shared_ptr<const NavMsg> message);

  /* DriverListener implementation: */
  void Notify(const AbstractCommDriver& driver);

private:
  NavMsgBus() = default;
};

#endif  // NAVMSG_BUS_H
