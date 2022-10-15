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

ObservedVarListener AppMsgBus::GetListener(wxEventType et, wxEvtHandler* eh,
                                           const AppMsg& msg) {
  ObservableAppMsg oam(msg.key());
  return oam.GetListener(eh, et);
}

ObservedVarListener AppMsgBus::GetListener(wxEventType et, wxEvtHandler* eh,
                                           AppMsg::Type type) {
  AppMsg msg(type);
  return GetListener(et, eh, msg);
}

void AppMsgBus::Notify(std::shared_ptr<const AppMsg> msg) {
  ObservableAppMsg om(msg->key());
  om.Notify(msg);
}

AppMsgBus& AppMsgBus::GetInstance() {
  static AppMsgBus instance;
  return instance;
}
