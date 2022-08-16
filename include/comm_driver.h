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
    : public std::enable_shared_from_this<const AbstractCommDriver> {
public:
  AbstractCommDriver() : bus(NavAddr::Bus::Undef), iface("nil"){};

  virtual void SendMessage(const NavMsg& msg, const NavAddr& addr) = 0;

  /** Register driver in  the driver Registry. */
  virtual void Activate() = 0;

  /**
   * Set the entity which will receive incoming data. By default, such
   * data is ignored
   */
  virtual void SetListener(std::shared_ptr<DriverListener> l) {}

  /**
   * Create a new virtual interface using a new instance of this driver.
   * A successful return guarantees that the new driver is registered in
   * the device registry and activated.
   *
   * @return <CommStatus::ok, interface> on success else <error_code, message>.
   */
  virtual std::pair<CommStatus, std::string> Clone() {
    // FIXME(leamas>: Requires unique interface support in DriverRegistry.
    return std::pair<CommStatus, std::string>(CommStatus::NotImplemented, "");
  }

  const NavAddr::Bus bus;
  const std::string iface; /**< Physical device for 0183, else a
                                unique string */
protected:
  AbstractCommDriver(NavAddr::Bus b) : bus(b){};
  AbstractCommDriver(NavAddr::Bus b, const std::string& s) : bus(b), iface(s){};
};

/**
 * Nmea2000 drivers are responsible for address claiming, exposing a stable
 * n2k_name. It also handles fast packages fragmentation/defragmentation.
 *
 * Handling of list of attached devices and their human readable attributes
 * are NOT part of the driver.
 */
class N2kDriver : public AbstractCommDriver {
public:
  /** @return address to given name on this n2k bus. */
  std::shared_ptr<NavAddr> GetAddress(N2kName name);
};

/**
 * Nmea0183 has no means to address a node. OTOH, there could be more
 * than one physical interface handling it. Each interface has a
 * separate driver instance.
 */
class Nmea0183Driver : public AbstractCommDriver {

  /** @return address to this bus i. e., physical interface. */
  std::shared_ptr<NavAddr> GetAddress();
};

#endif  // DRIVER_API_H
