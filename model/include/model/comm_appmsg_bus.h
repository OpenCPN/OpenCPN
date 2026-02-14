/***************************************************************************
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
 * Decoded messages send/receive support. This file represents the
 * top layer in the three layer model drivers, raw messages and
 * decoded messages.
 *
 * Message definitions are in comm_appmsg.h
 */

#ifndef APP_MSG_BUS_H
#define APP_MSG_BUS_H

#include <memory>

#include <wx/event.h>

#include "model/comm_appmsg.h"

/** Application layer messaging, a singleton. */
class AppMsgBus {
public:
  static AppMsgBus& GetInstance();

  AppMsgBus(const AppMsgBus&) = delete;
  AppMsgBus& operator=(const AppMsgBus&) = delete;

  /** Send message to everyone listening to given message type. */
  void Notify(const std::shared_ptr<const AppMsg>& msg);

  /**
   * Set the priority for a given data source providing data.
   * Higher priorities are preferred.
   */
  void set_priority(AppMsg::Type data, const NavAddr& src, unsigned prio);

private:
  AppMsgBus() = default;
};

#endif  // APP_MSG_BUS_H
