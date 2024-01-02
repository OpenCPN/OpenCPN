
/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Decoded messages send/receive support. This file represents the
 *           top layer in the three layer model drivers, raw messages and
 *           decoded messages.
 *
 *           Message definitions are in comm_appmsg.h
 *
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

#ifndef _APP_MSG_BUS_H
#define _APP_MSG_BUS_H

#include <memory>

#include <wx/event.h>

#include "comm_appmsg.h"

/** Application layer messaging, a singleton. */
class AppMsgBus {
public:
  /** Send message to everyone listening to given message type. */
  void Notify(std::shared_ptr<const AppMsg> msg);

  /**
   * Set the priority for a given data source providing data.
   * Higher priorities are preferred.
   */
  void set_priority(AppMsg::Type data, const NavAddr& src, unsigned prio);

  static AppMsgBus& GetInstance();
};

#endif  // APP_MSG_BUS_H
