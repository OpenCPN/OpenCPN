/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Communication driver layer. Defines the generic driver model,
 *           messages sent to/from drivers and addresses. The driver layer
 *           is the lowest of the three layers drivers, raw messages (navmsg)
 *           and decoded application messages(appmsg).
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

#include <memory>
#include <string>
#include <unordered_map>

#include "observable.h"
#include "comm_navmsg.h"

#ifndef _DRIVER_API_H
#define _DRIVER_API_H

enum class CommStatus { Ok, NotImplemented, NotSupported, NameInUse };

class AbstractCommDriver;  // forward

/**
 * Interface implemented by transport layer and possible other parties
 * like test code which should handle incoming messages
 */
class DriverListener {
public:
  /** Handle a received message. */
  virtual void Notify(std::shared_ptr<const NavMsg> message) = 0;

  /** Handle driver status change. */
  virtual void Notify(const AbstractCommDriver& driver) = 0;
};

/** Common interface for all drivers.  */
class AbstractCommDriver
    : public std::enable_shared_from_this<AbstractCommDriver> {
public:
  AbstractCommDriver() : bus(NavAddr::Bus::Undef), iface("nil"){};

  virtual bool SendMessage(std::shared_ptr<const NavMsg> msg,
                           std::shared_ptr<const NavAddr> addr) = 0;

  /** Register driver in  the driver Registry. */
  virtual void Activate() = 0;

  /**
   * Set the entity which will receive incoming data. By default, such
   * data is ignored
   */
  virtual void SetListener(DriverListener& l) {}

  /**
   * Create a new virtual interface using a new instance of this driver.
   * A successful return guarantees that the new driver is registered in
   * the device registry and activated.
   *
   * @return <CommStatus::ok, interface> on success else <error_code, message>.
   */
  virtual std::pair<CommStatus, std::string> Clone() {
    // FIXME(leamas): Requires unique interface support in DriverRegistry.
    return std::pair<CommStatus, std::string>(CommStatus::NotImplemented, "");
  }

  std::string Key() const { return NavAddr::BusToString(bus) + "!@!" + iface; }

  const NavAddr::Bus bus;
  const std::string iface; /**< Physical device for 0183, else a
                                unique string */

  virtual std::unordered_map<std::string, std::string> GetAttributes() const { return attributes;}

  std::unordered_map<std::string, std::string> attributes;

protected:
  AbstractCommDriver(NavAddr::Bus b) : bus(b){
    attributes["protocol"] = NavAddr::BusToString(bus);
  };
  AbstractCommDriver(NavAddr::Bus b, const std::string& s) : bus(b), iface(s){
    attributes["protocol"] = NavAddr::BusToString(bus);
  };
};

#endif  // DRIVER_API_H
