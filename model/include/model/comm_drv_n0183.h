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

#ifndef COMMDRIVER_N0183_H_
#define COMMDRIVER_N0183_H_

#include <memory>
#include <string>

#include "model/comm_driver.h"
#include "model/conn_params.h"

/** NMEA0183 drivers common part. */
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

#endif  // COMMDRIVER_N0183_H_
