/**************************************************************************
 *   Copyright (C) 2022 David Register                                     *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

/**
 *  \file
 *  NMEA0183 drivers common base.
 */

#ifndef COMM_DRIVER_N0183_H_
#define COMM_DRIVER_N0183_H_

#include <memory>
#include <string>

#include "model/comm_driver.h"
#include "model/conn_params.h"

/**
 * NMEA0183 basic parsing common parts:
 *
 *   - Input is processed as lines.
 *   - Lines missing an initial '$' or '!' are considered as garbage and
 *     marked as such.
 *   - Anything preceding first '$' or '!', including v4 tags, is
 *     silently dropped.
 *   - Sentences without checksum are allowed.
 *   - Sentences with an incorrect checksum are marked as such.
 *   - Sentences filtered by input filters are marked as such.
 *
 */
class CommDriverN0183 : public AbstractCommDriver {
public:
  CommDriverN0183();
  CommDriverN0183(NavAddr::Bus b, const std::string& s);

  ~CommDriverN0183() override;

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override = 0;

  virtual const ConnectionParams& GetParams() const = 0;

  void SetListener(DriverListener& l) override {}

  virtual std::shared_ptr<NavAddr> GetAddress() {
    return std::make_shared<NavAddr>(NavAddr0183(iface));
  }

protected:
  /** Wrap argument string in NavMsg pointer, forward to listener */
  void SendToListener(const std::string& payload, DriverListener& listener,
                      const ConnectionParams& params);
};

#endif  // COMM_DRIVER_N0183_H_
