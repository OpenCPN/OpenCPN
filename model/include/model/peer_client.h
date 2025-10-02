/***************************************************************************
 *   Copyright (C) 2022 David Register                                     *
 *   Copyright (C) 2023 Alec Leamas                                        *
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
 * Peer client non-gui abstraction
 */

#ifndef _PEERCLIENT_H
#define _PEERCLIENT_H

#include <functional>
#include <string>
#include <vector>

#include "model/route.h"
#include "model/track.h"
#include "model/semantic_vers.h"

#include "observable_evtvar.h"

enum class PeerDlgResult { Ok, Cancel, HasPincode };

enum class PeerDlg {
  PinConfirm,
  InvalidHttpResponse,
  ErrorReturn,  // Unexpected result code in json server reply.
  TransferOk,
  ActivateUnsupported,
  JsonParseError,
  BadPincode  // User pincode not accepted by server
};

struct PeerData {
  std::string dest_ip_address;
  std::string server_name;
  SemanticVersion api_version;  ///< server API version
  std::vector<Route*> routes;
  std::vector<RoutePoint*> routepoints;
  std::vector<Track*> tracks;
  bool overwrite;  ///< API parameter, force overwrite w/o server dialogs.
  bool activate;   ///< API parameter, activate route after transfer

  /** Notified with transfer percent progress (0-100). */
  EventVar& progress;

  /** Dialog displaying status (good, bad, ...) */
  std::function<PeerDlgResult(PeerDlg, int)> run_status_dlg;

  /**
   * Pin confirm dialog, returns new {0, user_pin} or
   * {error_code, error msg)
   */
  std::function<std::pair<PeerDlgResult, std::string>()> run_pincode_dlg;

  PeerData(EventVar& p);
};

void GetApiVersion(PeerData& peer_data);

/** Send data to server peer.*/
bool SendNavobjects(PeerData& peer_data);

/**
 * Check if server peer deems that writing these objects can be accepted
 * i. e., that the object(s) does not exist or can be overwritten.
 */
bool CheckNavObjects(PeerData& peer_data);

#endif  // guard
