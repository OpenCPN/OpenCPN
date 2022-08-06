/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_appmsg_bus.h -- Decoded messages bus.
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

#include "comm_appmsg_bus.h"
#include "comm_appmsg.h"

using namespace std;

class AppMsg;

ObservedVarListener AppMsgBus::get_listener(wxEventType et, wxEvtHandler* eh,
                                            const string& key) {
  ObservableAppMsg oam(key);
  return oam.get_listener(eh, et);
}

void AppMsgBus::notify(std::shared_ptr<const AppMsg> msg) {
  ObservableAppMsg om(msg->key());
  om.notify(msg);
}
AppMsgBus* AppMsgBus::getInstance() {
  static AppMsgBus* instance = 0;

  if (!instance) instance = new AppMsgBus();
  return instance;
}
