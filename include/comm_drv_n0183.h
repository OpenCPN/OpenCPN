/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
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
#ifndef _COMMDRIVERN0183_H__
#define _COMMDRIVERN0183_H__

#include <memory>
#include <string>

#include "comm_driver.h"

class CommDriverN0183 : public AbstractCommDriver {
public:
  CommDriverN0183();
  CommDriverN0183(NavAddr::Bus b, const std::string& s);

  virtual ~CommDriverN0183();

  virtual bool SendMessage(std::shared_ptr<const NavMsg> msg,
                           std::shared_ptr<const NavAddr> addr) override = 0;

  void SetListener(std::shared_ptr<DriverListener> l) override {}

  virtual std::shared_ptr<NavAddr> GetAddress() {
      return std::make_shared<NavAddr>(NavAddr0183(iface)); }

  void Activate() override;
};

#endif  // guarstring
