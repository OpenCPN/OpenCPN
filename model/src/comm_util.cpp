/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_util.h -- communication driver utilities
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
#endif  // precompiled headers

#include <vector>
#include <string>

#include "model/comm_util.h"
#include "model/comm_drv_factory.h"
#include "model/comm_drv_registry.h"
#include "model/conn_params.h"

void UpdateDatastreams() {
  // Recreate datastreams that are new, or have been edited
  std::vector<std::string> enabled_conns;

  for (auto* cp : TheConnectionParams()) {
    // Connection already setup?
    if (cp->b_IsSetup) {
      if (cp->bEnabled) {
        enabled_conns.push_back(cp->GetStrippedDSPort());
      }
      continue;
    }

    // Check to see if this connection port has been
    // already enabled in this loop.
    // If so, then leave this connection alone.
    // This will handle multiple connections with same port,
    // but possibly different filters
    // Also protect against some user config errors
    if (std::find(enabled_conns.begin(), enabled_conns.end(),
                  cp->GetStrippedDSPort()) != enabled_conns.end()) {
      continue;
    }

    // Terminate and remove any existing driver, if present in registry
    StopAndRemoveCommDriver(cp->GetStrippedDSPort(), cp->GetCommProtocol());

    // Stop and remove  "previous" port, in case other params have changed.
    StopAndRemoveCommDriver(cp->GetLastDSPort(), cp->GetLastCommProtocol());

    // Internal BlueTooth driver stacks commonly need a time delay to purge
    // their buffers, etc. before restating with new parameters...
    if (cp->Type == INTERNAL_BT) wxSleep(1);

    // Connection has been disabled
    if (!cp->bEnabled) continue;

    // Make any new or re-enabled drivers
    MakeCommDriver(cp);
    cp->b_IsSetup = TRUE;
    enabled_conns.push_back(cp->GetStrippedDSPort());
  }
}

bool StopAndRemoveCommDriver(std::string ident, NavAddr::Bus _bus) {
  auto& registry = CommDriverRegistry::GetInstance();
  const std::vector<DriverPtr>& drivers = registry.GetDrivers();
  DriverPtr& target_driver = FindDriver(drivers, ident, _bus);

  if (!target_driver) return false;

  // Deactivate the driver, and the last reference in shared_ptr
  // will be removed.
  // The driver DTOR will be called in due course.
  registry.Deactivate(target_driver);

  return true;
}

/**
 * Strip NMEA V4 tag blocks from NMEA0183 message.
 *
 * Tag blocks are enclosed in '\' characters and appear before the
 * traditional NMEA sentence (which starts with $ or !).
 * The format is:
 * \tag:value,tag:value*checksum\actual NMEA message
 */
wxString ProcessNMEA4Tags(const wxString& msg) {
  int idxFirst = msg.Find('\\');

  if (wxNOT_FOUND == idxFirst) return msg;

  if (idxFirst < (int)msg.Length() - 1) {
    int idxSecond = msg.Mid(idxFirst + 1).Find('\\') + 1;
    if (wxNOT_FOUND != idxSecond) {
      if (idxSecond < (int)msg.Length() - 1) {
        // wxString tag = msg.Mid(idxFirst+1, (idxSecond - idxFirst) -1);
        return msg.Mid(idxSecond + 1);
      }
    }
  }

  return msg;
}
