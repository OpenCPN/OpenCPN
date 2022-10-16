/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implements comm_navmsg_bus -- raw, undecoded messages bus.
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

#include "comm_navmsg_bus.h"
#include "observable_navmsg.h"

// FIXME(leamas) test junk
#include "conn_params.h"

using namespace std;

void NavMsgBus::Notify(std::shared_ptr<const NavMsg> msg) {
  ObservableMsg(msg->key()).Notify(msg);
}

NavMsgBus& NavMsgBus::GetInstance() {
  static NavMsgBus instance;
  return instance;
}

/** Handle changes in driver list. */
void NavMsgBus::Notify(AbstractCommDriver const&) {}
