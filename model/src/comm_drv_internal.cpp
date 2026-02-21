/***************************************************************************
 *   Copyright (C) 2024 Alec Leamas                                        *
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
 *  \file
 *  Implement comm_drv_internal.h
 */

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/log.h>
#include <wx/string.h>

#include "config.h"
#include "model/comm_drv_internal.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/logger.h"

#include "observable.h"

CommDriverInternal::CommDriverInternal(DriverListener& listener)
    : AbstractCommDriver(NavAddr::Bus::Plugin, "internal"),
      m_listener(listener) {
  this->attributes["commPort"] = "internal";
  this->attributes["ioDirection"] = "OUT";
}

bool CommDriverInternal::SendMessage(std::shared_ptr<const NavMsg> msg,
                                     std::shared_ptr<const NavAddr> addr) {
  auto msg_plugin = std::dynamic_pointer_cast<const PluginMsg>(msg);
  if (!msg_plugin) {
    WARNING_LOG << " CommDriverInternal::SendMessage::Illegal message type";
    return false;
  }
  NavMsgBus::GetInstance().Notify(msg_plugin);
  return true;
}
