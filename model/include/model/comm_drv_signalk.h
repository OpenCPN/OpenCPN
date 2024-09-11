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
#ifndef _COMMDRIVERSIGNALK_H
#define _COMMDRIVERSIGNALK_H

#include <memory>

#include "model/comm_driver.h"

class CommDriverSignalK : public AbstractCommDriver {
public:
  CommDriverSignalK(const std::string& s = "sk0");

  virtual ~CommDriverSignalK();

  virtual bool SendMessage(std::shared_ptr<const NavMsg> msg,
                           std::shared_ptr<const NavAddr> addr) override;
  virtual void SetListener(DriverListener& l) override;
  virtual std::shared_ptr<NavAddr> GetAddress(const NavAddrSignalK& name);
};

#endif  // guard
