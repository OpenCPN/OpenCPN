/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement the comm_drv_n0183.h -- Nmea0183 driver base
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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "model/comm_drv_n0183.h"

CommDriverN0183::CommDriverN0183() : AbstractCommDriver(NavAddr::Bus::N0183) {}

CommDriverN0183::CommDriverN0183(NavAddr::Bus b, const std::string& s)
    : AbstractCommDriver(NavAddr::Bus::N0183, s) {}

CommDriverN0183::~CommDriverN0183() = default;

void CommDriverN0183::SendToListener(const std::string& payload,
                                     DriverListener& listener,
                                     const ConnectionParams& params) {
  if ((payload[0] == '$' || payload[0] == '!') && payload.size() > 5) {
    std::string identifier;

    // notify message listener
    if (params.SentencePassesFilter(payload, FILTER_INPUT)) {
      // We notify based on full message, including the Talker ID
      std::string id = payload.substr(1, 5);
      auto msg = std::make_shared<const Nmea0183Msg>(id, payload, GetAddress());
      listener.Notify(std::move(msg));
    }
  }
}
