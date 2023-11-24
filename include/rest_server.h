
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

/**
 * \file
 *
 * Opencpn REST API
 *
 * Supports the following endpoints:
 *
 *   GET /api/ping?api_key=<pincode>&source=<ip address>
 *       Basic ping check, verifies api_key i. e., the pairing.
 *         Parameters:
 *           See below
 *         Returns
 *           {"result": <code>}
 *
 *   POST /api/rx_object?api_key=<pincode>&source=<ip address>&force=1
 *       Upload a GPX route, track or waypoints. Parameters:
 *         - source=<ip> Mandatory, origin ip address or hostname
 *         - force=<1> if present, the host object is unconditionally
 *           updated. If not, host may run a "OK to overwrite" dialog.
 *         - api_key=<key> Mandatory, as obtained when pairing, see below.
 *       Body:
 *         xml-encoded GPX data for one or more route(s), track(s) and/or
 *         waypoint(s)
 *       Returns:
 *         {"result": <code>}
 *
 *   GET /api/writable?guid=<guid>
 *       Check if route or waypoint with given is writable.
 *   Returns
 *       {"result": <code>}
 *
 * Authentication uses a pairing mechanism. When an unpaired device
 * tries to connect, the API generates a random pincode which is
 * sent to the connecting party where it is displayed to user. User
 * must then input the pincode in the server-side GUI thus making
 * sure she has physical access to the machine.
 *
 * Result codes are as defined in RestServerResult.
 */

#ifndef _RESTSERVER_H
#define _RESTSERVER_H

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

#if defined(__GNUC__) && (__GNUC__ < 8)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <wx/event.h>
#include <wx/string.h>
#include <wx/thread.h>    // for wxSemaphore, std::semaphore is c++20


#include "pugixml.hpp"
#include "pincode.h"
#include "route.h"
#include "track.h"

/** Return codes from HandleServerMessage and eventually in the http response */
enum class RestServerResult {
  NoError = 0,
  GenericError,
  ObjectRejected,
  DuplicateRejected,
  RouteInsertError,
  NewPinRequested,
  Void
};

/** Kind of messages sent from io thread to main code. */
enum { ORS_START_OF_SESSION, ORS_CHUNK_N, ORS_CHUNK_LAST };

/** Dialog return codes. */
enum { ID_STG_CANCEL = 10000, ID_STG_OK, ID_STG_CHECK1, ID_STG_CHOICE_COMM };

/** Data from IO thread to main */
struct RestIoEvtData {
  const enum class Cmd { Ping, Object, CheckWrite } cmd;
  const std::string api_key;  ///< Rest API parameter apikey
  const std::string source;   ///< Rest API parameter source
  const bool force;           ///< rest API parameter force

  const std::string payload;  ///< GPX data for Object, Guid for CheckWrite

  /** Cmd::Object constructor. */
  RestIoEvtData(const std::string& key, const std::string& src,
                const std::string& gpx_data, bool _force)
      : RestIoEvtData(Cmd::Object, key, src, gpx_data, _force) {}

  /** Cmd::Ping constructor. */
  RestIoEvtData(const std::string& key, const std::string& src)
      : RestIoEvtData(Cmd::Ping, key, src, "", false) {}

  /** Cmd::CheckWrite constructor. */
  RestIoEvtData(const std::string& key, const std::string& src,
                const std::string& guid)
      : RestIoEvtData(Cmd::CheckWrite, key, src, guid, false) {}

private:
  RestIoEvtData(Cmd c, const std::string& key, const std::string& src,
                const std::string& _payload, bool _force)
      : cmd(c), api_key(key), source(src), force(_force), payload(_payload) {}
};

/** Return hash code for numeric pin value. */
std::string PintoRandomKeyString(int dpin);

/** Abstract base class visible in callbacks. */
class PinDialog {
public:
  /** Create and show the dialog */
  virtual PinDialog* Initiate(const std::string& msg,
                              const std::string& text1) = 0;

  /** Close and destroy */
  virtual void DeInit() = 0;
};

/** Returned status from  RunAcceptObjectDlg. */
struct AcceptObjectDlgResult {
  const int status;         ///< return value from ShowModal()
  const bool check1_value;  ///< As of GetCheck1Value()

  AcceptObjectDlgResult() : status(0), check1_value(false) {}
  AcceptObjectDlgResult(int s, bool b) : status(s), check1_value(b) {}
};

/** Callbacks invoked from PinDialog implementations. */
class RestServerDlgCtx {
public:
  std::function<PinDialog*(const std::string& msg, const std::string& text1)>
      show_dialog;
  std::function<void(PinDialog*)> close_dialog;
  std::function<void(void)> update_route_mgr;

  /** Run the "Accept Object" dialog, returns value from ShowModal(). */
  std::function<AcceptObjectDlgResult(const wxString& msg,
                                      const wxString& check1msg)>
      run_accept_object_dlg;
  std::function<void()> top_level_refresh;

  RestServerDlgCtx()
      : show_dialog([](const std::string&, const std::string&) -> PinDialog* {
          return 0;
        }),
        close_dialog([](PinDialog*) {}),
        update_route_mgr([]() {}),
        run_accept_object_dlg([](const wxString&, const wxString&) {
          return AcceptObjectDlgResult();
        }),
        top_level_refresh([]() {}) {}
};

/** Callbacks for handling routes and tracks. */
class RouteCtx {
public:
  std::function<Route*(wxString)> find_route_by_guid;
  std::function<Track*(wxString)> find_track_by_guid;
  std::function<void(Route*)> delete_route;
  std::function<void(Track*)> delete_track;
  RouteCtx()
      : find_route_by_guid([](wxString) { return static_cast<Route*>(0); }),
        find_track_by_guid([](wxString) { return static_cast<Track*>(0); }),
        delete_route([](Route*) -> void {}),
        delete_track([](Track*) -> void {}) {}
};

/** Server public interface. */
class RestServer : public wxEvtHandler {
  friend class RestServerObjectApp;  // Unit test
  friend class RestCheckWriteApp;    // Unit test
  friend class RestServerPingApp;    // Unit test

public:
  RestServer(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable);

  virtual ~RestServer();

  bool StartServer(fs::path certificate_location);
  void StopServer();

  void UpdateReturnStatus(RestServerResult r);
  RestServerResult GetReturnStatus() { return return_status; }

  void UpdateRouteMgr() { m_dlg_ctx.update_route_mgr(); }

  std::string m_cert_file;
  std::string m_key_file;

  /** Guards return_status */
  std::mutex ret_mutex;

  /** Guards return_status */
  std::condition_variable return_status_condition;

  /** Binary exit synchronization, released when io thread exits. */
  wxSemaphore m_exit_sem;

private:
  class IoThread {
  public:
    IoThread(RestServer& parent, const std::string& ip);
    virtual ~IoThread(void) {}
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
  * Stores the api key for different ip addresses. Methods for
  * serialize/deserialize config file format.
 */
  class Apikeys: public std::unordered_map<std::string, std::string> {
  public:
    static Apikeys Parse(const std::string& s);
    std::string ToString() const;
  };

  bool LoadConfig(void);
  bool SaveConfig(void);

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
  PinDialog* m_pin_dialog;

  bool m_overwrite;
  std::string m_upload_path;
  std::ofstream m_ul_stream;
  std::thread m_thread;
  IoThread m_io_thread;
  Pincode m_pincode;
};

#endif  // guard
