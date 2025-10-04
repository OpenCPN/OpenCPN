/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * NMEA Data Multiplexer Object
 */

#ifndef _COMMN0183_OUT_H
#define _COMMN0183_OUT_H

#include <functional>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "model/nmea_log.h"
#include "model/multiplexer.h"
#include "model/route.h"
#include "model/route_point.h"
#include "observable_evtvar.h"

/**
 * Failed to initialize Garmin device.
 */
#define ERR_GARMIN_INITIALIZE -1
/**
 * Failed to send message to Garmin device.
 */
#define ERR_GARMIN_SEND_MESSAGE -2
/**
 * GPS driver not available.
 */
#define ERR_GPS_DRIVER_NOT_AVAILAIBLE -3

void BroadcastNMEA0183Message(const wxString& msg, NmeaLog* nmea_log,
                              EventVar& on_msg_sent);

class N0183DlgCtx {
public:
  std::function<void(int)> set_value;
  std::function<void(int)> set_range;
  std::function<void(void)> pulse;
  std::function<void(const std::string&)> set_message;
  std::function<bool()> confirm_overwrite;

  N0183DlgCtx()
      : set_value([](int) {}),
        set_range([](int) {}),
        pulse([](void) {}),
        set_message([](const std::string&) {}),
        confirm_overwrite([]() { return true; }) {}
};

int SendRouteToGPS_N0183(Route* pr, const wxString& com_name,
                         bool bsend_waypoints, Multiplexer& multiplexer,
                         N0183DlgCtx ctx);

int SendWaypointToGPS_N0183(RoutePoint* prp, const wxString& com_name,
                            Multiplexer& multiplexer, N0183DlgCtx ctx);

#endif  // _COMMN0183>>UT_H
