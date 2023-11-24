
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

/** \file Implement rest_server.h */

#include <memory>
#include <mutex>
#include <random>
#include <vector>

#include <wx/event.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

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

std::string PintoRandomKeyString(int dpin) {
  return Pincode::IntToHash(dpin);
}

/** Extract a HTTP variable from query string. */
static inline std::string HttpVarToString(const struct mg_str& query,
                                          const char* var) {
  std::string string;
  struct mg_str mgs = mg_http_var(query, mg_str(var));
  if (mgs.len && mgs.ptr) string = std::string(mgs.ptr, mgs.len);
  return string;
}

static void PostEvent(RestServer* parent,
                      std::shared_ptr<RestIoEvtData> evt_data, int id) {
  auto evt = new ObservedEvt(REST_IO_EVT, id);
  evt->SetSharedPtr(evt_data);
  parent->QueueEvent(evt);
  wxTheApp->ProcessPendingEvents();
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

  if (source.size()) {
    assert(parent && "Null parent pointer");
    auto data_ptr = std::make_shared<RestIoEvtData>(
        RestIoEvtData(api_key, source, xml_content, force.size()));
    PostEvent(parent, data_ptr, MID);
  }
  if (MID == ORS_CHUNK_LAST) {
    std::unique_lock<std::mutex> lock{parent->ret_mutex};
    bool r = parent->return_status_condition.wait_for(lock, 10s, [&] {
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
  if (source.size()) {
    assert(parent && "Null parent pointer");
    parent->UpdateReturnStatus(RestServerResult::Void);
    auto data_ptr =
        std::make_shared<RestIoEvtData>(RestIoEvtData(api_key, source));
    PostEvent(parent, data_ptr, ORS_CHUNK_LAST);
    std::unique_lock<std::mutex> lock{parent->ret_mutex};
    bool r = parent->return_status_condition.wait_for(lock, 10s, [&] {
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
  if (source.size()) {
    assert(parent && "Null parent pointer");
    parent->UpdateReturnStatus(RestServerResult::Void);
    auto data_ptr =
        std::make_shared<RestIoEvtData>(RestIoEvtData(apikey, source, guid));
    PostEvent(parent, data_ptr, ORS_CHUNK_LAST);
    std::unique_lock<std::mutex> lock{parent->ret_mutex};
    bool r = parent->return_status_condition.wait_for(lock, 10s, [&] {
      return parent->GetReturnStatus() != RestServerResult::Void;
    });
    if (!r) wxLogWarning("Timeout waiting for REST server condition");
  }
  mg_http_reply(c, 200, "", "{\"result\": %d}\n", parent->GetReturnStatus());
}

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data) {
  RestServer* parent = static_cast<RestServer*>(fn_data);

  if (ev == MG_EV_ACCEPT /*&& fn_data != NULL*/) {
    struct mg_tls_opts opts;
    memset(&opts, 0, sizeof(mg_tls_opts));

    opts.ca = NULL;  //"cert.pem";         // Uncomment to enable two-way SSL
    opts.cert = parent->m_cert_file.c_str();    // Certificate PEM file
    opts.certkey = parent->m_key_file.c_str();  // The key PEM file
    opts.ciphers = NULL;
    mg_tls_init(c, &opts);
  } else if (ev == MG_EV_TLS_HS) {  // Think of this as "start of session"
    PostEvent(parent, nullptr, ORS_START_OF_SESSION);
  } else if (ev == MG_EV_HTTP_CHUNK) {
    struct mg_http_message* hm = (struct mg_http_message*)ev_data;
    if (mg_http_match_uri(hm, "/api/ping")) {
      HandlePing(c, hm, parent);
    } else if (mg_http_match_uri(hm, "/api/rx_object")) {
      HandleRxObject(c, hm, parent);
    } else if (mg_http_match_uri(hm, "/api/writable")) {
      HandleWritable(c, hm, parent);
    }
  }
}


//========================================================================
/*    RestServer implementation */

RestServer::IoThread::IoThread(RestServer& parent, const std::string& ip)
    : m_parent(parent), m_server_ip(ip) {}

void RestServer::IoThread::Run() {
  run_flag = 1;
  struct mg_mgr mgr;        // Event manager
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
  return  apikeys;
}

std::string RestServer::Apikeys::ToString() const {
  std::stringstream ss;
  for (const auto& it : *this)
    ss << it.first << ":" << it.second << ";";
  return ss.str();
}

void RestServer::UpdateReturnStatus(RestServerResult result) {
  {
    std::lock_guard<std::mutex> lock{ret_mutex};
    return_status = result;
  }
  return_status_condition.notify_one();
}

RestServer::RestServer(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
    : m_exit_sem(0, 1),
      m_dlg_ctx(ctx),
      m_route_ctx(route_ctx),
      m_pin_dialog(0),
      m_io_thread(*this, portable ? kHttpsPortableAddr : kHttpsAddr),
      m_pincode(Pincode::Create()) {
  // Prepare the wxEventHandler to accept events from the io thread
  Bind(REST_IO_EVT, &RestServer::HandleServerMessage, this);
}

RestServer::~RestServer() {
  Unbind(REST_IO_EVT, &RestServer::HandleServerMessage, this);
}

bool RestServer::StartServer(fs::path certificate_location) {
  m_certificate_directory = certificate_location.string();
  m_cert_file = (certificate_location / "cert.pem").string();
  m_key_file = (certificate_location / "key.pem").string();

  // Load persistent config info and  kick off the  Server thread
  LoadConfig();
  if (!m_thread.joinable()) {
    m_thread = std::thread([&]() { m_io_thread.Run(); });
  }
  return true;
}

void RestServer::StopServer() {
  wxLogMessage(wxString::Format("Stopping REST service"));
  //  Kill off the IO Thread if alive
  if (m_thread.joinable()) {
    wxLogMessage("Stopping io thread");
    m_io_thread.Stop();
    m_io_thread.WaitUntilStopped();
    m_thread.join();
  }
}

bool RestServer::LoadConfig(void) {
  TheBaseConfig()->SetPath("/Settings/RestServer");
  wxString key_string;
  TheBaseConfig()->Read("ServerKeys", &key_string);
  m_key_map = Apikeys::Parse(key_string.ToStdString());
  TheBaseConfig()->Read("ServerOverwriteDuplicates", &m_overwrite, 0);
  return true;
}

bool RestServer::SaveConfig(void) {
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
  m_key_map[evt_data.source] = new_api_key;
  SaveConfig();

  std::stringstream ss;
  ss << evt_data.source << " " << _("wants to send you new data.") << "\n"
     << _("Please enter the following PIN number on ")  << evt_data.source
     << " " << _("to pair with this device") << "\n";
  m_pin_dialog = m_dlg_ctx.show_dialog(ss.str(), m_pincode.ToString());

  return false;
}

void RestServer::HandleServerMessage(ObservedEvt& event) {
  if (event.GetId() == ORS_START_OF_SESSION) {
    // Prepare a temp file to catch chuncks that might follow
    m_upload_path = wxFileName::CreateTempFileName("ocpn_tul").ToStdString();

    m_ul_stream.open(m_upload_path.c_str(), std::ios::out | std::ios::trunc);
    if (!m_ul_stream.is_open()) {
      wxLogMessage("REST_server: Cannot open %s for write", m_upload_path);
      m_upload_path.clear();  // reset for next time.
      return;
    }
    return;
  }

  auto evt_data = UnpackEvtPointer<RestIoEvtData>(event);
  if (event.GetId() == ORS_CHUNK_N) {
    //  Stream out to temp file
    if (m_upload_path.size() && m_ul_stream.is_open()) {
      m_ul_stream.write(evt_data->payload.c_str(), evt_data->payload.size());
    }
    return;
  }

  if (event.GetId() == ORS_CHUNK_LAST) {
    // Cancel existing dialog and close temp file
    m_dlg_ctx.close_dialog(m_pin_dialog);
    if (m_upload_path.size() && m_ul_stream.is_open()) m_ul_stream.close();

    // Io thread might be waiting for return_status on notify_one()
    UpdateReturnStatus(RestServerResult::GenericError);
  }

  if (CheckApiKey(*evt_data)) {
    UpdateReturnStatus(RestServerResult::NoError);
  } else {
    UpdateReturnStatus(RestServerResult::NewPinRequested);
    return;
  }

  if (evt_data->cmd == RestIoEvtData::Cmd::CheckWrite) {
    auto guid = evt_data->payload;
    auto dup = m_route_ctx.find_route_by_guid(guid);
    if (!dup || evt_data->force || m_overwrite) {
      UpdateReturnStatus(RestServerResult::NoError);
    } else {
      UpdateReturnStatus(RestServerResult::DuplicateRejected);
    }
    return;
  }
  // Load the GPX file
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
  }
}

void RestServer::HandleRoute(pugi::xml_node object,
                             const RestIoEvtData& evt_data) {
  Route* route = NULL;
  route = GPXLoadRoute1(object, true, false, false, 0, true);
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
  Track* route = NULL;
  route = GPXLoadTrack1(object, true, false, false, 0);
  // Check for duplicate GUID
  bool add = true;
  bool overwrite_one = false;

  Track* duplicate = m_route_ctx.find_track_by_guid(route->m_GUID);
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
    // Add the route to the global list
    NavObjectCollection1 pSet;

    if (InsertTrack(route, false))
      UpdateReturnStatus(RestServerResult::NoError);
    else
      UpdateReturnStatus(RestServerResult::RouteInsertError);
    m_dlg_ctx.top_level_refresh();
  }
}

void RestServer::HandleWaypoint(pugi::xml_node object,
                                const RestIoEvtData& evt_data) {
  RoutePoint* rp = NULL;
  rp = GPXLoadWaypoint1(object, "circle", "", false, false, false, 0);
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
