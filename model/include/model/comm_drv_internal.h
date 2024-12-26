/***************************************************************************
 *   Copyright (C) 2024 by Alec Leamas                                     *
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
 *  Internal send-only driver, send to plugins.
 */

#ifndef _COMM_DRV_INTERNAL__H
#define _COMM_DRV_INTERNAL__H

#include "model/comm_driver.h"

/**
 * Send only driver facilitating sending messages to all plugins and core.
 */
class CommDriverInternal : public AbstractCommDriver {
public:
  CommDriverInternal(DriverListener& l);

  ~CommDriverInternal() = default;

  /**
   * Send a message to all plugins and core on internal bus.
   * @param msg Must be PluginMsg, by convention a type + a json encoded
   *            string.
   * @param addr Not used
   */
  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;

private:
  DriverListener& m_listener;
};

#endif  //  _COMM_DRV_INTERNAL__H
