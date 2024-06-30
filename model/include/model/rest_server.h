
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

#ifndef RESTSERVER_H_
#define RESTSERVER_H_

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

#include "observable_evtvar.h"


// MacOS 1.13:
#if (defined(OCPN_GHC_FILESYSTEM) || (defined(__clang_major__) && (__clang_major__ < 15)))
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

#include "pugixml.hpp"
#include "pincode.h"
#include "route.h"
#include "track.h"

/**
 *  Return codes from HandleServerMessage and eventually in the http response.
 *  Since they are transported as integers on the wire they cannot really be
 *  changed without breaking compatibility with older servers. Adding new types
 *  should be fine.
 */
enum class RestServerResult {
  NoError = 0,
  GenericError = 1,
  ObjectRejected = 2,
  DuplicateRejected = 3,
  RouteInsertError = 4,
  NewPinRequested = 5,
  ObjectParseError = 6,
  Void = 100
};

/** Dialog return codes. */
enum { ID_STG_CANCEL = 10000, ID_STG_OK, ID_STG_CHECK1, ID_STG_CHOICE_COMM };

/** RestServerResult string representation */
std::string RestResultText(RestServerResult result);

/** Data from IO thread to main */
struct RestIoEvtData;

/** Returned status from RunAcceptObjectDlg. */
struct AcceptObjectDlgResult {
  const int status;         ///< return value from ShowModal()
  const bool check1_value;  ///< As of GetCheck1Value()

  /** default constructor, returns empty  struct. */
  AcceptObjectDlgResult() : status(0), check1_value(false) {}

  /** Create a struct with given values for status and check1_value. */
  AcceptObjectDlgResult(int s, bool b) : status(s), check1_value(b) {}
};

/** Callbacks for handling dialogs and RouteManager updates */
class RestServerDlgCtx {
public:
  /** Run the "Server wants a pincode" dialog. */
  std::function<wxDialog*(const std::string& msg, const std::string& text1)>
      run_pincode_dlg;

  /** Update Route manager after updates to underlying nav_object_database. */
  std::function<void(void)> update_route_mgr;

  /** Run the "Accept Object" dialog, returns value from ShowModal(). */
  std::function<AcceptObjectDlgResult(const wxString& msg,
                                      const wxString& check1msg)>
      run_accept_object_dlg;
  std::function<void()> top_level_refresh;

  /**  All dummy stubs constructor. */
  RestServerDlgCtx();
};

/** Callbacks for handling routes and tracks. */
class RouteCtx {
public:
  std::function<Route*(wxString)> find_route_by_guid;
  std::function<Track*(wxString)> find_track_by_guid;
  std::function<RoutePoint*(wxString)> find_wpt_by_guid;
  std::function<void(Route*)> delete_route;
  std::function<void(Track*)> delete_track;
  std::function<void(RoutePoint*)> delete_waypoint;

  /** Dummy stubs constructor. */
  RouteCtx();
};

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
class AbstractRestServer {

public:
  /** Start the server thread. */
  virtual bool StartServer(const fs::path& certificate_location) = 0;

  /** Stop server thread, blocks until completed. */
  virtual void StopServer() = 0;

  /** Return HTTPS url to local rest server. */
  virtual std::string GetEndpoint() = 0;

  /** Notified with a string GUID when user wants to activate a route. */
  EventVar activate_route;

  /** Notified with a string GUID when user wants to reverse a route. */
  EventVar reverse_route;
};

/** AbstractRestServer implementation and interface to underlying IO thread. */
class RestServer : public AbstractRestServer, public wxEvtHandler {
  friend class RestServerObjectApp;  ///< Unit test hook
  friend class RestCheckWriteApp;    ///< Unit test hook
  friend class RestServerPingApp;    ///< Unit test hook
  friend class RestPluginMsgApp;     ///< Unit test hook

public:
  RestServer(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable);

  ~RestServer() override;

  bool StartServer(const fs::path& certificate_location) override;

  void StopServer() override;

  std::string GetEndpoint() override { return m_endpoint; }

  /** IoThread interface.*/
  void UpdateReturnStatus(RestServerResult r);

  /** IoThread interface. */
  RestServerResult GetReturnStatus() { return return_status; }

  /** IoThread interface. */
  void UpdateRouteMgr() const { m_dlg_ctx.update_route_mgr(); }

  /** Semi-static storage used by IoThread C code. */
  std::string m_cert_file;

  /** Semi-static storage used by IoThread C code. */
  std::string m_key_file;

  /** IoThread interface: body of return message, if any. */
  std::string m_reply_body;

  /** IoThread interface: Guards return_status */
  std::mutex ret_mutex;

  /** IoThread interface: Guards return_status */
  std::condition_variable return_status_cv;

  /**
   * IoThread interface: Binary exit synchronization, released when
   * io thread exits. std::semaphore is C++20, hence wxSemaphore.
   */
  wxSemaphore m_exit_sem;

  const std::string m_endpoint;

private:
  class IoThread {
  public:
    /**
     * Asynchcronous thread handling IO. Eventually driven by the mongoose main
     * loop i. e. mg_mgr_poll().  Receives data, hands it to the containing
     * RestServer and expects feedback according to:
     * - IoThread gets a http request from mongoose
     * - "Simple" requests like 404 are handled directly. Otherwise:
     * - IoThtread sets return_status to RestServerResult::Void.
     * - IoThread posts a ObservedEvt containing a RestIoEvtData to RestServer
     * - IoThread blocks until return_status is changed.
     * - RestServer processes event and sets return_status, unblocking IoThread
     * - IoThread sends a http reply based on return_status using mongoose.
     *
     * return_status is a critical zone guarded by return_status_cv.
     */
    IoThread(RestServer& parent, std::string ip);
    virtual ~IoThread() = default;
    void Run();

    bool IsRunning() { return run_flag > 0; }

    /** Request thread to stop asap. */
    void Stop();

    /** Block until thread is stopped. */
    bool WaitUntilStopped();

  private:
    /** 1 -> running, 0 -> stop requested, -1 -> stopped. */
    std::atomic_int run_flag;
    RestServer& m_parent;
    std::string m_server_ip;
  };

  /**
   * Stores the api key for different ip addresses. Methods to
   * serialize/deserialize config file format.
   */
  class Apikeys : public std::unordered_map<std::string, std::string> {
  public:
    static Apikeys Parse(const std::string& s);
    std::string ToString() const;
  };

  bool LoadConfig();
  bool SaveConfig();

  void HandleServerMessage(ObservedEvt& event);

  void HandleWaypoint(pugi::xml_node object, const RestIoEvtData& evt_data);
  void HandleTrack(pugi::xml_node object, const RestIoEvtData& evt_data);
  void HandleRoute(pugi::xml_node object, const RestIoEvtData& evt_data);

  bool CheckApiKey(const RestIoEvtData& evt_data);

  RestServerDlgCtx m_dlg_ctx;
  RouteCtx m_route_ctx;

  RestServerResult return_status;

  std::string m_certificate_directory;
  Apikeys m_key_map;
  wxDialog* m_pin_dialog;

  bool m_overwrite;
  std::string m_upload_path;
  std::ofstream m_ul_stream;
  std::thread m_std_thread;
  IoThread m_io_thread;
  Pincode m_pincode;
};

#endif  // guard
