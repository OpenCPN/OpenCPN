/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement RESTful server.
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

#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <wx/event.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>

#include "config_vars.h"
#include "logger.h"
#include "mongoose.h"
#include "nav_object_database.h"
#include "pugixml.hpp"
#include "rest_server.h"
#include "route.h"
#include "track.h"

class RestServerEvent;
wxDEFINE_EVENT(wxEVT_RESTFUL_SERVER, RestServerEvent);

using namespace std::chrono_literals;

//  Some global variables to handle thread syncronization
static RestServerResult return_status;
static std::condition_variable return_status_condition;
static std::mutex mx;

static const char* s_http_addr = "http://0.0.0.0:8000";
static const char* s_https_addr = "https://0.0.0.0:8443";
// Is this host a portable?  Must use another port to avoid equal IP addres
// conflicts.
static const char* s_http_addr_portable = "http://0.0.0.0:8001";
static const char* s_https_addr_portable = "https://0.0.0.0:8444";

static std::string server_ip;

static unsigned long long PINtoRandomKey(int dpin) {
  using namespace std;
  linear_congruential_engine<unsigned long long, 48271, 0, 0xFFFFFFFFFFFFFFFF>
      engine;
  engine.seed(dpin);
  unsigned long long r = engine();
  return r;
}

// FIXME (leamas) "std::shared_ptr<std::string>"  makes no sense, nor does
// this event. Use a copyable struct + EventVar instead.
// struct RestServerEvt {
//   const std::string m_payload;
//   const std::string m_source_peer;
//   const std::string m_api_key;
//   RestServerEvt(const std::string& pl, const std::string& sp,
//                 const std::string& ak):
//      m_payload(pl), m_source_peer(sp), m_api_key(ak) {}
// }
// RestServerEvt evt(payload, source_peer, api_key);
// EventVar OnRestSrvEvt;
// OnRestSrvEvt.Notify(std::shared_ptr<RestServerEvt>(evt);
// auto evt = static_cast<RestServerEvt*>(OnRestSrvEvt.GetSharedPtr());  //not really...

/* Used by static function, breaks the ordering. Better off in separate file */
class RestServerEvent : public wxEvent {
public:
  RestServerEvent(wxEventType commandType = wxEVT_RESTFUL_SERVER, int id = 0)
      : wxEvent(id, commandType){};
  ~RestServerEvent(){};

  // accessors
  void SetPayload(std::shared_ptr<std::string> data) { m_payload = data; }
  void SetSource(std::string source) { m_source_peer = source; }
  void SetAPIKey(std::string key) { m_api_key = key; }
  std::shared_ptr<std::string> GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const {
    RestServerEvent* newevent = new RestServerEvent(*this);
    newevent->m_payload = this->m_payload;
    newevent->m_source_peer = this->m_source_peer;
    newevent->m_api_key = this->m_api_key;
    return newevent;
  };

  std::shared_ptr<std::string> m_payload;
  std::string m_source_peer;
  std::string m_api_key;
};

static void HandleRxObject(struct mg_connection* c, int ev,
                           struct mg_http_message* hm, RestServer* parent) {
  int MID = ORS_CHUNK_N;

  std::string api_key;

  struct mg_str api_key_parm = mg_http_var(hm->query, mg_str("apikey"));
  if (api_key_parm.len && api_key_parm.ptr) {
    api_key = std::string(api_key_parm.ptr, api_key_parm.len);
  }
  struct mg_str source = mg_http_var(hm->query, mg_str("source"));
  std::string xml_content;
  if (hm->chunk.len)
    xml_content = std::string(hm->chunk.ptr, hm->chunk.len);
  else {
    MID = ORS_CHUNK_LAST;
  }

  mg_http_delete_chunk(c, hm);

  return_status = RestServerResult::Undefined;

  if (source.len) {
    std::string source_peer(source.ptr, source.len);

    if (parent) {
      auto evt = new RestServerEvent(wxEVT_RESTFUL_SERVER, MID);
      if (xml_content.size()) {
        auto buffer = std::make_shared<std::string>(xml_content);
        evt->SetPayload(buffer);
      }
      evt->SetSource(source_peer);
      evt->SetAPIKey(api_key);
      parent->QueueEvent(evt);
      wxTheApp->ProcessPendingEvents();
std::cout << "Sending event from thread, kind: " << MID << "\n";
    }
  }
  if (MID == ORS_CHUNK_LAST) {
std::cout << "IO thread: ORS_CHUNK_LAST: Waiting for status\n";
    std::unique_lock<std::mutex> lock{mx};
    return_status_condition.wait(lock, [] { 
      return return_status != RestServerResult::Undefined; });
std::cout << "After wait: Reply: " << static_cast<int>(return_status) << "\n";
    mg_http_reply(c, 200, "", "{\"result\": %d}\n", return_status);
    parent->UpdateRouteMgr();
  }
}
static void HandlePing(struct mg_connection* c, int ev,
                       struct mg_http_message* hm, RestServer* parent) {
  std::string api_key;
  struct mg_str api_key_parm = mg_http_var(hm->query, mg_str("apikey"));
  if (api_key_parm.len && api_key_parm.ptr) {
    api_key = std::string(api_key_parm.ptr, api_key_parm.len);
  }

  struct mg_str source = mg_http_var(hm->query, mg_str("source"));

  if (source.len) {
    std::string source_peer(source.ptr, source.len);

    return_status = RestServerResult::Undefined;
    if (parent) {
      auto evt = new RestServerEvent(wxEVT_RESTFUL_SERVER, ORS_CHUNK_LAST);
      evt->SetSource(source_peer);
      evt->SetAPIKey(api_key);
std::cout << "API key: " << api_key << "\n";
      parent->QueueEvent(evt);
      wxTheApp->ProcessPendingEvents();
    }

    std::unique_lock<std::mutex> lock{mx};
    return_status_condition.wait(lock, [] { 
        return return_status != RestServerResult::Undefined; });
  }
  mg_http_reply(c, 200, "", "{\"result\": %d}\n", return_status);
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
    if (parent) {
      auto evt =
          new RestServerEvent(wxEVT_RESTFUL_SERVER, ORS_START_OF_SESSION);
      parent->QueueEvent(evt);
      wxTheApp->ProcessPendingEvents();
    }
  } else if (ev == MG_EV_HTTP_CHUNK) {
    struct mg_http_message* hm = (struct mg_http_message*)ev_data;
    if (mg_http_match_uri(hm, "/api/ping")) {
      HandlePing(c, ev, hm, parent);
    } else if (mg_http_match_uri(hm, "/api/rx_object")) {
      HandleRxObject(c, ev, hm, parent);
    }
  }
}

std::string PintoRandomKeyString(int dpin) {
  unsigned long long pin = PINtoRandomKey(dpin);
  char buffer[100];
  snprintf(buffer, sizeof(buffer) - 1, "%0llX", pin);
  return std::string(buffer);
}


//========================================================================
/*    RestServer implementation
 * */

RestServer::RestServer(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
    : m_dlg_ctx(ctx),
      m_route_ctx(route_ctx),
      m_portable(portable),
      m_io_thread(this, portable) {
  m_pin_dialog = 0;
  // Prepare the wxEventHandler to accept events from the actual hardware thread
  //Bind(wxEVT_RESTFUL_SERVER, &RestServer::HandleServerMessage, this);
  Bind(wxEVT_RESTFUL_SERVER,
       [&](RestServerEvent& ev) { HandleServerMessage(ev); });
}

RestServer::~RestServer() {
  //Unbind(wxEVT_RESTFUL_SERVER, &RestServer::HandleServerMessage, this);
}

namespace fs = std::filesystem;

bool RestServer::StartServer(fs::path certificate_location) {
  m_certificate_directory = certificate_location.string();
  m_cert_file = (certificate_location / "cert.pem").string();
  m_key_file = (certificate_location / "key.pem").string();

  // Load persistent config info
  LoadConfig();

  //    Kick off the  Server thread
  m_io_thread.run_flag = 1;
  m_thread = std::thread([&] () {m_io_thread.Entry(); });
  return true;
}

void RestServer::StopServer() {
  wxLogMessage(wxString::Format("Stopping REST service"));

  //    Kill off the Secondary RX Thread if alive
  if (m_thread.joinable()) {
    wxLogMessage("Stopping io thread");
    m_io_thread.run_flag = 0;

    int msec = 10000;
    while ((m_io_thread.run_flag >= 0) && (msec -= 100))
      std::this_thread::sleep_for(100ms);
    if (m_io_thread.run_flag < 0)
      MESSAGE_LOG << "Stopped in " <<  10000 - msec << " milliseconds";
    else
      MESSAGE_LOG << "Not Stopped after 10 sec.";
    m_thread.join();
  }
}

bool RestServer::LoadConfig(void) {
  if (TheBaseConfig()) {
    TheBaseConfig()->SetPath("/Settings/RestServer");

    wxString key_string;

    TheBaseConfig()->Read("ServerKeys", &key_string);
    wxStringTokenizer st(key_string, ";");
    while (st.HasMoreTokens()) {
      wxString s1 = st.GetNextToken();
      wxString client_name = s1.BeforeFirst(':');
      wxString client_key = s1.AfterFirst(':');

      m_key_map[client_name.ToStdString()] = client_key.ToStdString();
    }
    TheBaseConfig()->Read("ServerOverwriteDuplicates", &m_overwrite, 0);
  }
  return true;
}

bool RestServer::SaveConfig(void) {
  if (TheBaseConfig()) {
    TheBaseConfig()->SetPath("/Settings/RestServer");

    wxString key_string;
    for (auto it : m_key_map) {
      wxString item =
          it.first.c_str() + wxString(":") + it.second.c_str() + wxString(";");
      key_string += item;
    }

    TheBaseConfig()->Write("ServerKeys", key_string);
    TheBaseConfig()->Write("ServerOverwriteDuplicates", m_overwrite);
    TheBaseConfig()->Flush();
  }
  return true;
}

static void UpdateReturnStatus(RestServerResult result) {
std::cout << "Updating return_status: " << static_cast<int>(result) << "..."
    << std::flush;
  {
    std::lock_guard<std::mutex> lock{mx};
    return_status = result;
  }
  return_status_condition.notify_one();
std::cout << " done\n";
}

void RestServer::HandleServerMessage(RestServerEvent& event) {

std::cout << "Handling event, GetId() " << event.GetId() << "\n" << std::flush;
  if (event.GetId() == ORS_START_OF_SESSION) {
    // Prepare a temp file to catch chuncks that might follow
    m_tmp_upload_path =
        wxFileName::CreateTempFileName("ocpn_tul").ToStdString();

    m_ul_stream.open(m_tmp_upload_path.c_str(),
                     std::ios::out | std::ios::trunc);
    if (!m_ul_stream.is_open()) {
      wxLogMessage("REST_server: Cannot open %s for write",
                   m_tmp_upload_path);
      m_tmp_upload_path.clear();  // reset for next time.
      return;
    }
    return;
  }

  if (event.GetId() == ORS_CHUNK_N) {
    auto p = event.GetPayload();
    std::string* payload = p.get();
std::cout << "Got chunk: " << payload << "\n";
    // printf("%s\n", payload->c_str());
    //  Stream out to temp file
    if (m_tmp_upload_path.size() && m_ul_stream.is_open()) {
      m_ul_stream.write(payload->c_str(), payload->size());
    }
    return;
  }

  if (event.GetId() == ORS_CHUNK_LAST) {
std::cout << "Processing ORS_CHUNK_LAST\n";
    // Cancel existing dialog
    m_dlg_ctx.close_dialog(m_pin_dialog);

    // Close the temp file.
    if (m_tmp_upload_path.size() && m_ul_stream.is_open())
      m_ul_stream.close();

    // Io thread might be waiting for (return_status >= 0) on notify_one()
    UpdateReturnStatus(RestServerResult::GenericError);
  }

  // Look up the api key in the hash map.
  std::string api_found;
  for (auto it : m_key_map) {
    if (it.first == event.m_source_peer && it.second == event.m_api_key) {
      api_found = it.second;
      break;
    }
  }

  if (!api_found.size()) {
    // Need a new PIN confirmation
    m_dpin = wxMin(rand() % 10000 + 1, 9999);
    m_pin.Printf("%04d", m_dpin);

    std::string new_api_key = PintoRandomKeyString(m_dpin);

    //  Add new PIN to map and persist it
    m_key_map[event.m_source_peer] = new_api_key;
    SaveConfig();

    wxString hmsg(event.m_source_peer.c_str());
    hmsg += " ";
    hmsg +=
        _("wants to send you new data.\nPlease enter the following PIN number "
          "on ");
    hmsg += wxString(event.m_source_peer.c_str());
    hmsg += _(" to pair with this device.\n");
    m_pin_dialog =
        m_dlg_ctx.show_dialog(hmsg.ToStdString(), m_pin.ToStdString());

    UpdateReturnStatus(RestServerResult::NewPinRequested);
    return;
  } else {
    UpdateReturnStatus(RestServerResult::NoError);
  }

  // GUI dialogs can go here....
  bool b_cont;
  b_cont = true;

  if (b_cont) {  // Load the GPX file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(m_tmp_upload_path.c_str());
    if (result.status == pugi::status_ok) {
      m_tmp_upload_path.clear();  // empty for next time

      pugi::xml_node objects = doc.child("gpx");
      for (pugi::xml_node object = objects.first_child(); object;
           object = object.next_sibling()) {
        if (!strcmp(object.name(), "rte")) {
          Route* route = NULL;
          route = GPXLoadRoute1(object, true, false, false, 0, true);
          // Check for duplicate GUID
          bool add = true;
          bool overwrite_one = false;
          Route* duplicate = m_route_ctx.find_route_by_guid(route->GetGUID());
          if (duplicate) {
            if (!m_overwrite) {
              auto result = m_dlg_ctx.run_accept_object_dlg(
                  _("The received route already exists on this "
                    "system.\nReplace?"),
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

            if (m_overwrite || overwrite_one) {
              //  Remove the existing duplicate route before adding new route
              m_route_ctx.delete_route(duplicate);
            }
          }

          if (add) {
            // And here is the payoff....

            // Add the route to the global list
            NavObjectCollection1 pSet;

            if (InsertRouteA(route, &pSet))
              UpdateReturnStatus(RestServerResult::NoError);
            else
              UpdateReturnStatus(RestServerResult::RouteInsertError);
            m_dlg_ctx.top_level_refresh();
          }
        } else if (!strcmp(object.name(), "trk")) {
          Track* route = NULL;
          route = GPXLoadTrack1(object, true, false, false, 0);
          // Check for duplicate GUID
          bool add = true;
          bool overwrite_one = false;

          Track* duplicate = m_route_ctx.find_track_by_guid(route->m_GUID);
          if (duplicate) {
            if (!m_overwrite) {
              auto result = m_dlg_ctx.run_accept_object_dlg(
                  _("The received track already exists on this "
                    "system.\nReplace?"),
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
            if (m_overwrite || overwrite_one) {
              m_route_ctx.delete_track(duplicate);
            }
          }

          if (add) {
            // And here is the payoff....

            // Add the route to the global list
            NavObjectCollection1 pSet;

            if (InsertTrack(route, false))
              UpdateReturnStatus(RestServerResult::NoError);
            else
              UpdateReturnStatus(RestServerResult::RouteInsertError);
            m_dlg_ctx.top_level_refresh();
          }
        } else if (!strcmp(object.name(), "wpt")) {
          RoutePoint* rp = NULL;
          rp = GPXLoadWaypoint1(object, "circle", "", false, false, false, 0);
          rp->m_bIsolatedMark = true;  // This is an isolated mark
          // Check for duplicate GUID
          bool add = true;
          bool overwrite_one = false;

          RoutePoint* duplicate =
              WaypointExists(rp->GetName(), rp->m_lat, rp->m_lon);
          if (duplicate) {
            if (!m_overwrite) {
              auto result = m_dlg_ctx.run_accept_object_dlg(
                  _("The received waypoint already exists on this "
                    "system.\nReplace?"),
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
            // And here is the payoff....
            if (InsertWpt(rp, m_overwrite || overwrite_one))
              UpdateReturnStatus(RestServerResult::NoError);
            else
              UpdateReturnStatus(RestServerResult::RouteInsertError);
            m_dlg_ctx.top_level_refresh();
          }
        }
      }
    }
  } else {
    UpdateReturnStatus(RestServerResult::ObjectRejected);
  }
}

RestServer::IoThread::IoThread(RestServer* parent, bool& portable)
    : m_portable(portable), m_parent(parent) {
  server_ip = s_https_addr;
  // If Portable use another port
  if (m_portable) {
    server_ip = s_https_addr_portable;
    wxString sip(server_ip);
    wxLogMessage("Portable REST server IP: Port " + sip);
  }
}

void RestServer::IoThread::Entry() {
  bool not_done = true;
  run_flag = 1;
  struct mg_mgr mgr;        // Event manager
  mg_log_set(MG_LL_DEBUG);  // Set log level
  mg_mgr_init(&mgr);        // Initialise event manager

  // Create HTTPS listener
std::cout << "Listening on " << server_ip << "\n";
  mg_http_listen(&mgr, server_ip.c_str(), fn, m_parent);
  // mg_http_listen(&mgr, s_https_addr, fn, (void *) 1);

  while (run_flag > 0) mg_mgr_poll(&mgr, 200);  // Infinite event loop
  mg_mgr_free(&mgr);
  run_flag = -1;
}
