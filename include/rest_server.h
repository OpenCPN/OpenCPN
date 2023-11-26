
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

#if defined(__GNUC__) && (__GNUC__ < 8)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
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

  /** GPX data for Cmd::Object, Guid for Cmd::CheckWrite */
  const std::string payload;

  /** Create a Cmd::Object instance. */
  static RestIoEvtData CreateCmdData(const std::string& key,
                                     const std::string& src,
                                     const std::string& gpx_data, bool _force) {
    return {Cmd::Object, key, src, gpx_data, _force};
  }

  /** Create a Cod::Ping instance: */
  static RestIoEvtData CreatePingData(const std::string& key,
                                      const std::string& src) {
    return {Cmd::Ping, key, src, "", false};
  }

  /** Cmd::CheckWrite constructor. */
  static RestIoEvtData CreateChkWriteData(const std::string& key,
                                          const std::string& src,
                                          const std::string& guid) {
    return {Cmd::CheckWrite, key, src, guid, false};
  }

private:
  RestIoEvtData(Cmd c, std::string key, std::string src, std::string _payload,
                bool _force);
};

/** Return hash code for numeric pin value. */
std::string PintoRandomKeyString(int pin);

/** Abstract base class visible in callbacks. */
class PinDialog {
public:
  /** Create and show the dialog */
  virtual PinDialog* Initiate(const std::string& msg,
                              const std::string& text1) = 0;

  /** Close and destroy */
  virtual void DeInit() = 0;
};

/** Returned status from RunAcceptObjectDlg. */
struct AcceptObjectDlgResult {
  const int status;         ///< return value from ShowModal()
  const bool check1_value;  ///< As of GetCheck1Value()

  /** default constructor, returns empty  struct. */
  AcceptObjectDlgResult() : status(0), check1_value(false) {}

  /** Create a struct with given values for status and check1_value. */
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

  RestServerDlgCtx();
};

/** Callbacks for handling routes and tracks. */
class RouteCtx {
public:
  std::function<Route*(wxString)> find_route_by_guid;
  std::function<Track*(wxString)> find_track_by_guid;
  std::function<void(Route*)> delete_route;
  std::function<void(Track*)> delete_track;
  RouteCtx();
};

/**
 *
 * Opencpn REST API
 *
 * Supports the following endpoints:
 *
 *  GET /api/ping?api_key=`<pincode>`  &source=<ip `address>`  <br>
 *  Basic ping check, verifies api_key i. e., the pairing.
 *    - Parameters:
 *           See below <br>
 *    - Returns:
 *           {"result": `<code>`}
 *
 *  POST /api/rx_object?api_key=`<pincode>` &source=`<ip address>`&force=1 <br>
 *  Upload a GPX route, track or waypoints.
 *     - Parameters:
 *         - source=`<ip>` Mandatory, origin ip address or hostname
 *         - force=`<1>` f present, the host object is unconditionally
 *           updated. If not, host may run a "OK to overwrite" dialog.
 *         - api_key=`<key>` Mandatory, as obtained when pairing, see below.
 *
 *     - Body:
 *         xml-encoded GPX data for one or more route(s), track(s) and/or
 *         waypoint(s) <br>
 *     - Returns:
 *         {"result": `<code>`}
 *
 *  GET /api/writable?guid=<`guid>`  <br>
 *  Check if route or waypoint with given guid is writable. <br>
 *    - Returns::
 *         {"result": `<code>`}
 *
 * Authentication uses a pairing mechanism. When an unpaired device
 * tries to connect, the API generates a random pincode which is
 * sent to the connecting party where it is displayed to user. User
 * must then input the pincode in the server-side GUI thus making
 * sure she has physical access to the machine.
 *
 * Result codes are as defined in RestServerResult.
 */
class AbstractRestServer {
  /** Server public interface. */

public:
  /** Start the server thread. */
  virtual bool StartServer(const fs::path& certificate_location) = 0;

  /** Stop server thread, blocks until completed. */
  virtual void StopServer() = 0;
};

/** AbstractRestServer implementation and interface to underlying IO thread. */
class RestServer : public AbstractRestServer, public wxEvtHandler {
  friend class RestServerObjectApp;  ///< Unit test hook
  friend class RestCheckWriteApp;    ///< Unit test hook
  friend class RestServerPingApp;    ///< Unit test hook

public:
  RestServer(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable);

  ~RestServer() override;

  /** Start the server thread. */
  bool StartServer(const fs::path& certificate_location) override;

  /** Stop server thread, blocks until completed. */
  void StopServer() override;

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

  /** IoThread interface: Guards return_status */
  std::mutex ret_mutex;

  /** IoThread interface: Guards return_status */
  std::condition_variable return_status_condition;

  /**
   * IoThread interface: Binary exit synchronization, released when
   * io thread exits. std::semaphore is C++20, hence wxSemaphore.
   */
  wxSemaphore m_exit_sem;

private:
  class IoThread {
  public:
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
   * Stores the api key for different ip addresses. Methods for
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
  PinDialog* m_pin_dialog;

  bool m_overwrite;
  std::string m_upload_path;
  std::ofstream m_ul_stream;
  std::thread m_thread;
  IoThread m_io_thread;
  Pincode m_pincode;
};

#endif  // guard
