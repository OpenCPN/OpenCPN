/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_factory: Communication driver factory.
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

// FIXME  Why is this needed?
#ifdef __MSVC__
#include <winsock2.h>
#include <wx/msw/winundef.h>
#endif

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include "model/comm_util.h"
#include "model/comm_drv_n2k_net.h"
#include "model/comm_drv_n2k_serial.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_drv_n0183_net.h"
#include "model/comm_drv_signalk_net.h"
#include "model/comm_drv_n0183_android_int.h"
#include "model/comm_drv_n0183_android_bt.h"
#include "model/comm_navmsg_bus.h"
#include "model/comm_drv_registry.h"

#if defined(__linux__) && !defined(__ANDROID__) && !defined(__WXOSX__)
#include "model/comm_drv_n2k_socketcan.h"
#endif

class N0183Listener : public DriverListener {
public:
  N0183Listener() = default;

  /** Handle driver status change. */
  void Notify(const AbstractCommDriver& driver) override {}

  void Notify(std::shared_ptr<const NavMsg> message) override {
    switch (message->state) {
      case NavMsg::State::kCannotParse:
      case NavMsg::State::kFiltered:
      case NavMsg::State::kBadChecksum:
        CommDriverRegistry::GetInstance().evt_dropped_msg.Notify(message);
        break;
      default:
        NavMsgBus::GetInstance().Notify(message);
        break;
    }
  }
};

void MakeCommDriver(const ConnectionParams* params) {
  static N0183Listener listener;

  wxLogMessage("MakeCommDriver: %s", params->GetDSPort().c_str());

  auto& msgbus = NavMsgBus::GetInstance();
  auto& registry = CommDriverRegistry::GetInstance();
  switch (params->Type) {
    case SERIAL:
      switch (params->Protocol) {
        case PROTO_NMEA2000: {
          auto driver = std::make_unique<CommDriverN2KSerial>(params, msgbus);
          registry.Activate(std::move(driver));
          break;
        }
        default: {
          auto driver =
              std::make_unique<CommDriverN0183Serial>(params, listener);
          registry.Activate(std::move(driver));
          break;
        }
      }
      break;
    case NETWORK:
      switch (params->NetProtocol) {
        case SIGNALK: {
          auto driver = std::make_unique<CommDriverSignalKNet>(params, msgbus);
          registry.Activate(std::move(driver));
          break;
        }
        default: {
          switch (params->Protocol) {
            case PROTO_NMEA0183: {
              auto driver =
                  std::make_unique<CommDriverN0183Net>(params, listener);
              registry.Activate(std::move(driver));
              break;
            }
            case PROTO_NMEA2000: {
              auto driver = std::make_unique<CommDriverN2KNet>(params, msgbus);
              registry.Activate(std::move(driver));
              break;
            }
            default:
              break;
          }
          break;
        }
      }

      break;
#if defined(__linux__) && !defined(__ANDROID__) && !defined(__WXOSX__)
    case SOCKETCAN: {
      auto driver = CommDriverN2KSocketCAN::Create(params, msgbus);
      registry.Activate(std::move(driver));
      break;
    }
#endif

#ifdef __ANDROID__
    case INTERNAL_GPS: {
      auto driver = std::make_unique<CommDriverN0183AndroidInt>(params, msgbus);
      registry.Activate(std::move(driver));
      break;
    }

    case INTERNAL_BT: {
      auto driver = std::make_unique<CommDriverN0183AndroidBT>(params, msgbus);
      registry.Activate(std::move(driver));
      break;
    }
#endif

    default:
      break;
  }
};

void initIXNetSystem() { CommDriverSignalKNet::initIXNetSystem(); };

void uninitIXNetSystem() { CommDriverSignalKNet::uninitIXNetSystem(); };
