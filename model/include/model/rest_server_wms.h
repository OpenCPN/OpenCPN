
/***************************************************************************
 *   Copyright (C) 2022 David Register                                     *
 *   Copyright (C) 2022-2023  Alec Leamas                                  *
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

#ifndef RESTSERVERWMS_H_
#define RESTSERVERWMS_H_

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

#include "gdal/cpl_string.h"

#include "observable_evtvar.h"

#include "../gui/include/gui/viewport.h"
#include "../gui/include/gui/chcanv.h"

#include "../model/georef.h"

// MacOS 1.13:
#if (defined(OCPN_GHC_FILESYSTEM) || \
     (defined(__clang_major__) && (__clang_major__ < 15)))
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;

#else
#include <filesystem>
#include <utility>
namespace fs = std::filesystem;
#endif

#include <wx/event.h>
#include <wx/string.h>
#include <wx/thread.h>  // for wxSemaphore, std::semaphore is c++20


/**
 *
 * Opencpn REST API
 *
 * Supported endpoints:
 *
 *  GET /api/ping?api_key=`<pincode>`  &source=`<ip address>`  <br>
 *  Basic ping check, verifies api_key i. e., the pairing.
 *    - Parameters:
 *        - source=`<ip>` Mandatory, origin ip address or hostname.
 *        - api_key=`<key>` Mandatory, as obtained when pairing, see below.
 *    - Returns:
 *        {"result": `<code>`, "version": `<version>`}
 *        `<version>` is a printable version like 5.9.0
 *
 *  POST /api/rx_object?api_key=`<pincode>`&source=`<ip address>`&force=1 <br>
 *  Upload GPX route(s), track(s) or waypoint(s).
 *     - Parameters:
 *         - source=`<ip>` Mandatory, origin ip address or hostname.
 *         - api_key=`<key>` Mandatory, as obtained when pairing, see below.
 *         - force=`<1>` if present, the host object is unconditionally
 *           updated. If not, host may run a "OK to overwrite" dialog.
 *         - activate=`<1>` Optional, activate route or waypoint after
 *           transfer
 *
 *     - Body:
 *         xml-encoded GPX data for one or more route(s), track(s) and/or
 *         waypoint(s) <br>
 *     - Returns:
 *         {"result": `<code>`}
 *
 *  GET /api/writable?guid=<`guid>`  <br>
 *  Check if route or waypoint with given guid is writable. <br>
 *    - Parameters:
 *         - source=`<ip>` Mandatory, origin ip address or hostname.
 *         - api_key=`<key>` Mandatory, as obtained when pairing, see below.
 *         - guid=`<guid>` Route, waypoint or track guid.
 *    - Returns:
 *         {"result": `<code>`}
 *
 *  GET /api/get-version  <br>
 *  Return current server version string. Does not require api_key or source.
 *    - Parameters: None
 *    - Returns (example):
 *        {"version": "5.8.9" }
 *
 *  GET /api/list-routes  <br>
 *  Return list of available routes
 *    - source=`<ip>` Mandatory, origin ip address or hostname.
 *    - api_key=`<key>` Mandatory, as obtained when pairing, see below.
 *    - Returns json data
 *      {
 *        "version": "5.8.0",
 *        "routes":  [ ["guid-1": "name1" ], ["guid-2": "name2" ], ... ]
 *      }
 *
 *
 *  GET /api/activate-route  <br>
 *  Activate an existing route.
 *    - source=`<ip>` Mandatory, origin ip address or hostname.
 *    - api_key=`<key>` Mandatory, as obtained when pairing, see below.
 *    - guid=`<guid>` Route guid.
 *    - Returns json data (activating already active route silently ignored)
 *         {"result": `<code>`}
 *
 *  GET /api/reverse-route  <br>
 *  Reverse an existing route
 *    - source=`<ip>` Mandatory, origin ip address or hostname.
 *    - api_key=`<key>` Mandatory, as obtained when pairing, see below.
 *    - guid=`<guid>` Route guid.
 *    - Returns json data
 *         {"result": `<code>`}
 *
 *  POST /api/plugin-msg
 *  Upload string message forwarded to all plugins
 *     - Parameters:
 *         - source=`<ip>` Mandatory, origin ip address or hostname.
 *         - api_key=`<key>` Mandatory, as obtained when pairing, see below.
 *         - id=`<id>` Mandatory, message id used by listeners.
 *     - Body:
 *         - Arbitrary text.
 *     - Returns:
 *         {"result": `<code>`}
 *
 *
 * Authentication uses a pairing mechanism. When an unpaired device
 * tries to connect, the API generates a random pincode which is
 * sent to the connecting party where it is displayed to user. User
 * must then input the pincode in the server-side GUI thus making
 * sure she has physical access to the server.
 *
 * Result codes are as defined in RestServerResult.
 */

/** AbstractRestServer implementation and interface to underlying IO thread. */
class RestServerWms {
public:
  RestServerWms();
  ~RestServerWms();

  bool StartServer();
  void StopServer();

  static unsigned int RestServerWms::m_hitcount;
  static wxFrame* m_pWxFrame;
  static ChartCanvas* m_pChartCanvas;

  static wxStaticText* pText;

  static unsigned int lastSize_W;
  static unsigned int lastSize_H;

  static void* jpegdatabuffer;

private:
  void Run();

  /** IoThread interface: Guards return_status */
  std::mutex ret_mutex;

  /** IoThread interface: Guards return_status */
  std::condition_variable return_status_cv;

  const std::string m_endpoint;

  std::atomic<bool> m_alive = true;

  std::thread m_delayedLoaderThread;
  std::thread m_workerthread;

 
};
#endif  // guard
