
/**************************************************************************
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

/** \file  rest_server.cpp Implement rest_server.h */

#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include <wx/event.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/string.h>

#include "config.h"
#include "config_vars.h"
#include "logger.h"
#include "mongoose.h"
#include "nav_object_database.h"
#include "ocpn_utils.h"
#include "observable_evt.h"
#include "rest_server.h"

/** Event from IO thread to main */
wxDEFINE_EVENT(REST_IO_EVT, ObservedEvt);

using namespace std::chrono_literals;

static const char* const kHttpAddr = "http://0.0.0.0:8000";
static const char* const kHttpsAddr = "http://0.0.0.0:8443";

static const char* const kHttpPortableAddr = "http://0.0.0.0:8001";
static const char* const kHttpsPortableAddr = "http://0.0.0.0:8444";
static const char* const kVersionReply = R"""( { "version": "@version@" })""";

/** Kind of messages sent from io thread to main code. */
enum { ORS_START_OF_SESSION, ORS_CHUNK_N, ORS_CHUNK_LAST };

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

  /** Create a Cmd::Ping instance: */
  static RestIoEvtData CreatePingData(const std::string& key,
                                      const std::string& src) {
    return {Cmd::Ping, key, src, "", false};
  }

  /** Create a Cmd::CheckWrite instance. */
  static RestIoEvtData CreateChkWriteData(const std::string& key,
                                          const std::string& src,
                                          const std::string& guid) {
    return {Cmd::CheckWrite, key, src, guid, false};
  }

private:
  RestIoEvtData(Cmd c, std::string key, std::string src, std::string _payload,
                bool _force);
};

/** Compat interface to old peer_client. */
std::string PintoRandomKeyString(int pin) { return Pincode::IntToHash(pin); }

/** Extract a HTTP variable from query string. */
static inline std::string HttpVarToString(const struct mg_str& query,
                                          const char* var) {
  std::string string;
  struct mg_str mgs = mg_http_var(query, mg_str(var));
  if (mgs.len && mgs.ptr) string = std::string(mgs.ptr, mgs.len);
  return string;
}

static void PostEvent(RestServer* parent,
                      const std::shared_ptr<RestIoEvtData>& evt_data, int id) {
  auto evt = new ObservedEvt(REST_IO_EVT, id);
  evt->SetSharedPtr(evt_data);
  parent->QueueEvent(evt);
#ifdef UNIT_TESTS
  wxTheApp->ProcessPendingEvents();
#endif
}

static void HandleRxObject(struct mg_connection* c, struct mg_http_message* hm,
                           RestServer* parent) {
  int MID = ORS_CHUNK_N;

  std::string api_key = HttpVarToString(hm->query, "apikey");
  std::string source = HttpVarToString(hm->query, "source");
  std::string force = HttpVarToString(hm->query, "force");
  std::string xml_content;
  if (hm->chunk.len)
    xml_content = std::string(hm->chunk.ptr, hm->chunk.len);
  else {
    MID = ORS_CHUNK_LAST;
  }
  mg_http_delete_chunk(c, hm);
  parent->UpdateReturnStatus(RestServerResult::Void);

  if (!source.empty()) {
    assert(parent && "Null parent pointer");
    auto data_ptr =
        std::make_shared<RestIoEvtData>(RestIoEvtData::CreateCmdData(
            api_key, source, xml_content, !force.empty()));
    PostEvent(parent, data_ptr, MID);
  }
  if (MID == ORS_CHUNK_LAST) {
    std::unique_lock<std::mutex> lock{parent->ret_mutex};
    bool r = parent->return_status_cv.wait_for(lock, 10s, [&] {
      return parent->GetReturnStatus() != RestServerResult::Void;
    });
    if (!r) wxLogWarning("Timeout waiting for REST server condition");
    mg_http_reply(c, 200, "", "{\"result\": %d}\n", parent->GetReturnStatus());
    parent->UpdateRouteMgr();
  }
}

static void HandlePing(struct mg_connection* c, struct mg_http_message* hm,
                       RestServer* parent) {
  std::string api_key = HttpVarToString(hm->query, "apikey");
  std::string source = HttpVarToString(hm->query, "source");
  if (!source.empty()) {
    assert(parent && "Null parent pointer");
    parent->UpdateReturnStatus(RestServerResult::Void);
    auto data_ptr = std::make_shared<RestIoEvtData>(
        RestIoEvtData::CreatePingData(api_key, source));
    PostEvent(parent, data_ptr, ORS_CHUNK_LAST);
    std::unique_lock<std::mutex> lock{parent->ret_mutex};
    bool r = parent->return_status_cv.wait_for(lock, 10s, [&] {
      return parent->GetReturnStatus() != RestServerResult::Void;
    });
    if (!r) wxLogWarning("Timeout waiting for REST server condition");
  }
  mg_http_reply(c, 200, "", "{\"result\": %d}\n", parent->GetReturnStatus());
}

static void HandleWritable(struct mg_connection* c, struct mg_http_message* hm,
                           RestServer* parent) {
  std::string apikey = HttpVarToString(hm->query, "apikey");
  std::string source = HttpVarToString(hm->query, "source");
  std::string guid = HttpVarToString(hm->query, "guid");
  if (!source.empty()) {
    assert(parent && "Null parent pointer");
    parent->UpdateReturnStatus(RestServerResult::Void);
    auto data_ptr = std::make_shared<RestIoEvtData>(
        RestIoEvtData::CreateChkWriteData(apikey, source, guid));
    PostEvent(parent, data_ptr, ORS_CHUNK_LAST);
    std::unique_lock<std::mutex> lock{parent->ret_mutex};
    bool r = parent->return_status_cv.wait_for(lock, 10s, [&] {
      return parent->GetReturnStatus() != RestServerResult::Void;
    });
    if (!r) wxLogWarning("Timeout waiting for REST server condition");
  }
  mg_http_reply(c, 200, "", "{\"result\": %d}\n", parent->GetReturnStatus());
}

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data) {
  auto parent = static_cast<RestServer*>(fn_data);

  if (ev == MG_EV_ACCEPT /*&& fn_data != NULL*/) {
    struct mg_tls_opts opts = {0};
    opts.ca = nullptr;  //   "cert.pem"  Uncomment to enable two-way SSL
    opts.cert = parent->m_cert_file.c_str();
    opts.certkey = parent->m_key_file.c_str();
    opts.ciphers = nullptr;
    mg_tls_init(c, &opts);
  } else if (ev == MG_EV_TLS_HS) {  // Think of this as "start of session"
    PostEvent(parent, nullptr, ORS_START_OF_SESSION);
  } else if (ev == MG_EV_HTTP_CHUNK) {
    auto hm = (struct mg_http_message*)ev_data;
    if (mg_http_match_uri(hm, "/api/ping")) {
      HandlePing(c, hm, parent);
    } else if (mg_http_match_uri(hm, "/api/rx_object")) {
      HandleRxObject(c, hm, parent);
    } else if (mg_http_match_uri(hm, "/api/writable")) {
      HandleWritable(c, hm, parent);
    } else if (mg_http_match_uri(hm, "/api/get-version")) {
      std::string reply(kVersionReply);
      ocpn::replace(reply, "@version@", PACKAGE_VERSION);
      mg_http_reply(c, 200, "", reply.c_str());
    } else {
      mg_http_reply(c, 404, "", "url: not found");
    }
  }
}

//========================================================================
/*    RestServer implementation */

RestServer::IoThread::IoThread(RestServer& parent, std::string ip)
    : run_flag(-1), m_parent(parent), m_server_ip(std::move(ip)) {}

void RestServer::IoThread::Run() {
  run_flag = 1;
  struct mg_mgr mgr = {0};  // Event manager
  mg_log_set(MG_LL_DEBUG);  // Set log level
  mg_mgr_init(&mgr);        // Initialise event manager

  // Create HTTPS listener
  MESSAGE_LOG << "Listening on " << m_server_ip << "\n";
  mg_http_listen(&mgr, m_server_ip.c_str(), fn, &m_parent);

  while (run_flag > 0) mg_mgr_poll(&mgr, 200);  // Infinite event loop
  mg_mgr_free(&mgr);
  run_flag = -1;
  m_parent.m_exit_sem.Post();
}

void RestServer::IoThread::Stop() { run_flag = 0; }

bool RestServer::IoThread::WaitUntilStopped() {
  auto r = m_parent.m_exit_sem.WaitTimeout(10000);
  if (r != wxSEMA_NO_ERROR) {
    WARNING_LOG << "Semaphore error: " << r;
  }
  return r == wxSEMA_NO_ERROR;
}

RestServer::Apikeys RestServer::Apikeys::Parse(const std::string& s) {
  Apikeys apikeys;
  auto ip_keys = ocpn::split(s.c_str(), ";");
  for (const auto& ip_key : ip_keys) {
    auto words = ocpn::split(ip_key.c_str(), ":");
    if (words.size() != 2) continue;
    if (apikeys.find(words[0]) == apikeys.end()) {
      apikeys[words[0]] = words[1];
    }
  }
  return apikeys;
}

std::string RestServer::Apikeys::ToString() const {
  std::stringstream ss;
  for (const auto& it : *this) ss << it.first << ":" << it.second << ";";
  return ss.str();
}

void RestServer::UpdateReturnStatus(RestServerResult result) {
  {
    std::lock_guard<std::mutex> lock{ret_mutex};
    return_status = result;
  }
  return_status_cv.notify_one();
}

RestServer::RestServer(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
    : m_exit_sem(0, 1),
      m_dlg_ctx(std::move(ctx)),
      m_route_ctx(std::move(route_ctx)),
      return_status(RestServerResult::Void),
      m_pin_dialog(nullptr),
      m_overwrite(false),
      m_io_thread(*this, portable ? kHttpsPortableAddr : kHttpsAddr),
      m_pincode(Pincode::Create()) {
  // Prepare the wxEventHandler to accept events from the io thread
  Bind(REST_IO_EVT, &RestServer::HandleServerMessage, this);
}

RestServer::~RestServer() { StopServer(); }

bool RestServer::StartServer(const fs::path& certificate_location) {
  m_certificate_directory = certificate_location.string();
  m_cert_file = (certificate_location / "cert.pem").string();
  m_key_file = (certificate_location / "key.pem").string();

  // Load persistent config info and  kick off the  Server thread
  LoadConfig();
  if (!m_std_thread.joinable()) {
    m_std_thread = std::thread([&]() { m_io_thread.Run(); });
  }
  return true;
}

void RestServer::StopServer() {
  wxLogDebug("Stopping REST service");
  //  Kill off the IO Thread if alive
  if (m_std_thread.joinable()) {
    wxLogDebug("Stopping io thread");
    m_io_thread.Stop();
    m_io_thread.WaitUntilStopped();
    m_std_thread.join();
  }
}

bool RestServer::LoadConfig() {
  TheBaseConfig()->SetPath("/Settings/RestServer");
  wxString key_string;
  TheBaseConfig()->Read("ServerKeys", &key_string);
  m_key_map = Apikeys::Parse(key_string.ToStdString());
  TheBaseConfig()->Read("ServerOverwriteDuplicates", &m_overwrite, false);
  return true;
}

bool RestServer::SaveConfig() {
  TheBaseConfig()->SetPath("/Settings/RestServer");
  TheBaseConfig()->Write("ServerKeys", wxString(m_key_map.ToString()));
  TheBaseConfig()->Write("ServerOverwriteDuplicates", m_overwrite);
  TheBaseConfig()->Flush();
  return true;
}

bool RestServer::CheckApiKey(const RestIoEvtData& evt_data) {
  // Look up the api key in the hash map. If found, we are done.
  if (m_key_map.find(evt_data.source) != m_key_map.end()) {
    if (m_key_map[evt_data.source] == evt_data.api_key) return true;
  }
  // Need a new PIN confirmation, add it to map and persist
  m_pincode = Pincode::Create();
  std::string new_api_key = m_pincode.Hash();
  if (evt_data.api_key.size() < 10)  // client sends old-style keys
    new_api_key = m_pincode.CompatHash();

  m_key_map[evt_data.source] = new_api_key;
  SaveConfig();

  std::stringstream ss;
  ss << evt_data.source << " " << _("wants to send you new data.") << "\n"
     << _("Please enter the following PIN number on ") << evt_data.source << " "
     << _("to pair with this device") << "\n";
  m_pin_dialog = m_dlg_ctx.run_pincode_dlg(ss.str(), m_pincode.ToString());

  return false;
}

void RestServer::HandleServerMessage(ObservedEvt& event) {
  auto evt_data = UnpackEvtPointer<RestIoEvtData>(event);
  switch (event.GetId()) {
    case ORS_START_OF_SESSION:
      // Prepare a temp file to catch chuncks that might follow
      m_upload_path = wxFileName::CreateTempFileName("ocpn_tul").ToStdString();

      m_ul_stream.open(m_upload_path.c_str(), std::ios::out | std::ios::trunc);
      if (!m_ul_stream.is_open()) {
        wxLogMessage("REST_server: Cannot open %s for write", m_upload_path);
        m_upload_path.clear();  // reset for next time.
        return;
      }
      return;
    case ORS_CHUNK_N:
      //  Stream out to temp file
      if (!m_upload_path.empty() && m_ul_stream.is_open()) {
        m_ul_stream.write(evt_data->payload.c_str(), evt_data->payload.size());
      }
      return;
    case ORS_CHUNK_LAST:
      // Cancel existing dialog and close temp file
      wxQueueEvent(m_pin_dialog, new wxCloseEvent);
      if (!m_upload_path.empty() && m_ul_stream.is_open()) m_ul_stream.close();
      break;
  }

  if (!CheckApiKey(*evt_data)) {
    UpdateReturnStatus(RestServerResult::NewPinRequested);
    return;
  }

  UpdateReturnStatus(RestServerResult::NoError);
  switch (evt_data->cmd) {
    case RestIoEvtData::Cmd::Ping:
      return;
    case RestIoEvtData::Cmd::CheckWrite: {
      auto guid = evt_data->payload;
      auto dup = m_route_ctx.find_route_by_guid(guid);
      if (!dup || evt_data->force || m_overwrite) {
        UpdateReturnStatus(RestServerResult::NoError);
      } else {
        UpdateReturnStatus(RestServerResult::DuplicateRejected);
      }
      return;
    }
    case RestIoEvtData::Cmd::Object: {
      pugi::xml_document doc;
      pugi::xml_parse_result result = doc.load_file(m_upload_path.c_str());
      if (result.status == pugi::status_ok) {
        m_upload_path.clear();  // empty for next time

        pugi::xml_node objects = doc.child("gpx");
        for (pugi::xml_node object = objects.first_child(); object;
             object = object.next_sibling()) {
          if (!strcmp(object.name(), "rte")) {
            HandleRoute(object, *evt_data);
          } else if (!strcmp(object.name(), "trk")) {
            HandleTrack(object, *evt_data);
          } else if (!strcmp(object.name(), "wpt")) {
            HandleWaypoint(object, *evt_data);
          }
        }
      } else {
        UpdateReturnStatus(RestServerResult::ObjectParseError);
      }
      break;
    }
  }
}

void RestServer::HandleRoute(pugi::xml_node object,
                             const RestIoEvtData& evt_data) {
  Route* route = GPXLoadRoute1(object, true, false, false, 0, true);
  // Check for duplicate GUID
  bool add = true;
  bool overwrite_one = false;
  Route* duplicate = m_route_ctx.find_route_by_guid(route->GetGUID());
  if (duplicate && !evt_data.force) {
    if (!m_overwrite) {
      auto result = m_dlg_ctx.run_accept_object_dlg(
          _("The received route already exists on this system.\nReplace?"),
          _("Always replace objects?"));
      if (result.status != ID_STG_OK) {
        add = false;
        UpdateReturnStatus(RestServerResult::DuplicateRejected);
      } else {
        m_overwrite = result.check1_value;
        overwrite_one = true;
        SaveConfig();
      }
    }
    if (m_overwrite || overwrite_one || evt_data.force) {
      //  Remove the existing duplicate route before adding new route
      m_route_ctx.delete_route(duplicate);
    }
  }
  if (add) {
    // Add the route to the global list
    NavObjectCollection1 pSet;
    if (InsertRouteA(route, &pSet))
      UpdateReturnStatus(RestServerResult::NoError);
    else
      UpdateReturnStatus(RestServerResult::RouteInsertError);
    m_dlg_ctx.top_level_refresh();
  }
}

void RestServer::HandleTrack(pugi::xml_node object,
                             const RestIoEvtData& evt_data) {
  Track* track = GPXLoadTrack1(object, true, false, false, 0);
  // Check for duplicate GUID
  bool add = true;
  bool overwrite_one = false;

  Track* duplicate = m_route_ctx.find_track_by_guid(track->m_GUID);
  if (duplicate) {
    if (!m_overwrite && !evt_data.force) {
      auto result = m_dlg_ctx.run_accept_object_dlg(
          _("The received track already exists on this system.\nReplace?"),
          _("Always replace objects?"));

      if (result.status != ID_STG_OK) {
        add = false;
        UpdateReturnStatus(RestServerResult::DuplicateRejected);
      } else {
        m_overwrite = result.check1_value;
        overwrite_one = true;
        SaveConfig();
      }
    }
    if (m_overwrite || overwrite_one || evt_data.force) {
      m_route_ctx.delete_track(duplicate);
    }
  }
  if (add) {
    // Add the track to the global list
    NavObjectCollection1 pSet;

    if (InsertTrack(track, false))
      UpdateReturnStatus(RestServerResult::NoError);
    else
      UpdateReturnStatus(RestServerResult::RouteInsertError);
    m_dlg_ctx.top_level_refresh();
  }
}

void RestServer::HandleWaypoint(pugi::xml_node object,
                                const RestIoEvtData& evt_data) {
  RoutePoint* rp =
      GPXLoadWaypoint1(object, "circle", "", false, false, false, 0);
  rp->m_bIsolatedMark = true;  // This is an isolated mark
  // Check for duplicate GUID
  bool add = true;
  bool overwrite_one = false;

  RoutePoint* duplicate = WaypointExists(rp->GetName(), rp->m_lat, rp->m_lon);
  if (duplicate) {
    if (!m_overwrite && !evt_data.force) {
      auto result = m_dlg_ctx.run_accept_object_dlg(
          _("The received waypoint already exists on this system.\nReplace?"),
          _("Always replace objects?"));
      if (result.status != ID_STG_OK) {
        add = false;
        UpdateReturnStatus(RestServerResult::DuplicateRejected);
      } else {
        m_overwrite = result.check1_value;
        overwrite_one = true;
        SaveConfig();
      }
    }
  }
  if (add) {
    if (InsertWpt(rp, m_overwrite || overwrite_one || evt_data.force))
      UpdateReturnStatus(RestServerResult::NoError);
    else
      UpdateReturnStatus(RestServerResult::RouteInsertError);
    m_dlg_ctx.top_level_refresh();
  }
}

RestIoEvtData::RestIoEvtData(RestIoEvtData::Cmd c, std::string key,
                             std::string src, std::string _payload, bool _force)
    : cmd(c),
      api_key(std::move(key)),
      source(std::move(src)),
      force(_force),
      payload(std::move(_payload)) {}

RestServerDlgCtx::RestServerDlgCtx()
    : run_pincode_dlg([](const std::string&, const std::string&) -> wxDialog* {
        return nullptr;
      }),
      update_route_mgr([]() {}),
      run_accept_object_dlg([](const wxString&, const wxString&) {
        return AcceptObjectDlgResult();
      }),
      top_level_refresh([]() {}) {}

RouteCtx::RouteCtx()
    : find_route_by_guid(
          [](const wxString&) { return static_cast<Route*>(nullptr); }),
      find_track_by_guid(
          [](const wxString&) { return static_cast<Track*>(nullptr); }),
      delete_route([](Route*) -> void {}),
      delete_track([](Track*) -> void {}) {}
