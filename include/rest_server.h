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
/**
 * \file
 *
 * Opencpn REST API
 *
 * Supports the following endpoints:
 *
 *   GET /api/ping?api_key=<pincode>
 *         Returns {"result": <code>}
 * 
 *   POST /api/rx_object?api_key=<pincode>&source=<ip address>&force
 *        The source parameter is mandatory, ip address of originating
 *        peer. Message body contains xml-encoded data for one or
 *        more route(s), track(s) and/or waypoint(s).
 *        If "force" is present, the host object is unconditionally
 *        updated. If not, host may run a "OK to overwrite" dialog.
 *        Returns {"result": <code>}
 * 
 *   GET /api/uid_exists?uid=<uid>
 *        Check if route or waypoint with given UID exists
 *        Returns {"result": <code>}
 * 
 * Authentication uses a pairing mechanism. When an unpaired device
 * tries to connect, the API generates a random pincode which is
 * sent to the connecting party where it is displayed to user. User
 * must then input the pincode in the server-side GUI thus making
 * sure he has physical access to the machine.
 */



#ifndef _RESTSERVER_H
#define _RESTSERVER_H

#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

#include <wx/event.h>
#include <wx/string.h>

#include "observable_evtvar.h"
#include "route.h"
#include "track.h"

/** Return codes from HandleServerMessage. */
enum class RestServerResult {
  NoError = 0,
  GenericError,
  ObjectRejected,
  DuplicateRejected,
  RouteInsertError,
  NewPinRequested,
  Undefined
};

/** Kind of messages sent from io thread to main code. */
enum { ORS_START_OF_SESSION, ORS_CHUNK_N, ORS_CHUNK_LAST };

enum { ID_STG_CANCEL = 10000, ID_STG_OK, ID_STG_CHECK1, ID_STG_CHOICE_COMM };

/** \internal  IO thread. */
class RestServerThread;

/** \internal Event sent from IO thread to main code. */
class RestServerEvent;

class PinCreateDialog;

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
  int status;         ///< return value from ShowModal()
  bool check1_value;  ///< As of GetCheck1Value()

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
friend class RestServerObjectApp;
public:
  RestServer(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable);

  virtual ~RestServer();



  bool StartServer(std::filesystem::path certificate_location);
  void StopServer();

  void HandleServerMessage(RestServerEvent& event);

  /**
   * Secondary thread life toggle
   * Used to inform launching object (this) to determine if the thread can
   * be safely called or polled, e.g. wxThread->Destroy();
   */

  void UpdateRouteMgr() { m_dlg_ctx.update_route_mgr(); }

  std::string GetCertificateDirectory() { return m_certificate_directory; }
  std::string m_cert_file;
  std::string m_key_file;

private:
  class IoThread {
  public:
    IoThread(RestServer* parent, bool& m_portable);
    virtual ~IoThread(void) {}



    void Entry();
    void Stop();

    /** 1 -> running, 0 -> stop requested, -1 -> stopped. */
    std::atomic_int run_flag;
  private:
    bool& m_portable;
    RestServer* m_parent;
  };

  bool LoadConfig(void);
  bool SaveConfig(void);

  RestServerDlgCtx m_dlg_ctx;
  RouteCtx m_route_ctx;

  std::string m_certificate_directory;
  std::unordered_map<std::string, std::string> m_key_map;
  PinDialog* m_pin_dialog;
  wxString m_pin;
  RestServerThread* m_parent;
  int m_dpin;
  bool m_overwrite;
  std::string m_tmp_upload_path;
  std::ofstream m_ul_stream;
  std::thread m_thread;
  bool m_portable;
  IoThread m_io_thread;
};

#endif  // guard
