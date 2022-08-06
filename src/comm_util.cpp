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

#include "comm_util.h"
#include "comm_drv_n2K_serial.h"
#include "comm_drv_n0183_serial.h"
#include "comm_drv_n0183_net.h"
#include "comm_navmsg_bus.h"
#include "comm_drv_registry.h"

std::shared_ptr<AbstractCommDriver> MakeCommDriver(const ConnectionParams *params) {
  wxLogMessage(
      wxString::Format(_T("MakeCommDriver: %s"), params->GetDSPort().c_str()));

  auto& msgbus = NavMsgBus::getInstance();
  auto& registry = CommDriverRegistry::getInstance();

  switch (params->Type) {
    case SERIAL:
      switch (params->Protocol) {
         case PROTO_NMEA2000:
         {
           auto driver = std::make_shared<commDriverN2KSerial>(params, msgbus);
           registry.Activate(driver);
           return driver;
           break;
         }
         default:
         {
           auto driver = std::make_shared<commDriverN0183Serial>(params, msgbus);
           registry.Activate(driver);
           return driver;

           break;
         }
      }
    case NETWORK:
      switch (params->NetProtocol) {
//FIXME         case SIGNALK:
//           return new SignalKDataStream(input_consumer, params);
        default:
        {
          auto driver = std::make_shared<commDriverN0183Net>(params, msgbus);
          registry.Activate(driver);
          return driver;
          break;
        }
      }

#if 0  //FIXME
    case INTERNAL_GPS:
      return new InternalGPSDataStream(input_consumer, params);
    case INTERNAL_BT:
      return new InternalBTDataStream(input_consumer, params);

#endif
    default:
      break;
  }

  return NULL;

}

bool StopAndRemoveCommDriver(std::string ident) {
  auto& registry = CommDriverRegistry::getInstance();
  const std::vector<DriverPtr>& drivers = registry.get_drivers();
  DriverPtr target_driver = FindDriver(drivers, ident);

  if(!target_driver)
    return false;

  // Deactivate the driver, and the last reference in shared_ptr
  // will be removed.
  // The driver DTOR will be called in due course.
  registry.Deactivate(target_driver);

  return true;
}



