/**************************************************************************
 *   Copyright (C) 2025 ALec Leamas                                        *
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
 * Loopback driver, treat sent messages as received.
 */

#ifndef _COMM_DRV_LOOPBACK_H
#define _COMM_DRV_LOOPBACK_H

#include <memory>
#include <string>

#include "model/comm_driver.h"

/** Accept messages to send; treat them as received from outside  */
class LoopbackDriver : public AbstractCommDriver {
public:
  /** An instance which accepts messages and forwards it the listener  */
  LoopbackDriver(DriverListener& l) : m_listener(l) {
    AbstractCommDriver::attributes["protocol"] = "loopback";
    AbstractCommDriver::attributes["ioDirection"] = "OUT";
  }

  ~LoopbackDriver() override = default;

  /**
   * Parse a string as provided by plugin and convert to a navmsg.
   * @param msg Formatted as documented in WriteCommDriver().
   * @return Generic Navmsg pointer, false (nullptr) if msg cannot
   *    be parsed.
   */
  static std::shared_ptr<const NavMsg> ParsePluginMessage(
      const std::string& msg);

  /** Accept message to forward to listener; addr is not used. */
  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override {
    m_listener.Notify(std::move(msg));
    return true;
  }

private:
  DriverListener& m_listener;
};

#endif  // _COMM_DRV_LOOPBACK_H
