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

#ifndef _COMMDRIVERREGISTRY_H__
#define _COMMDRIVERREGISTRY_H__

#include "comm_driver.h"
#include "ConnectionParams.h"
#include "commdriverN2KSerial.h"

/*The global driver registry, a singleton. Drivers register here when
 * activated, transport layer finds them.
 */
class commDriverRegistry {
public:
  commDriverRegistry();
  ~commDriverRegistry();

  void activate(const AbstractCommDriver& driver);
  void deactivate(const AbstractCommDriver& driver);

  /** Notified by all driverlist updates. */
  EventVar evt_driverlist_change;

  /** @return List of all activated drivers. */
  const std::vector<AbstractCommDriver>& get_drivers();

  static commDriverRegistry* getInstance();

  // FIXME
  //  Stub method, to pretest drivers.
  //  Goes away for production
  void TestDriver(ConnectionParams* params);
};

#endif  // guard
