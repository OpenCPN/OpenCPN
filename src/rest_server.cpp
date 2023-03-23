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
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>

#include "config_vars.h"
#include "mongoose.h"
#include "nav_object_database.h"
#include "pugixml.hpp"
#include "rest_server.h"
#include "route.h"
#include "track.h"

class RESTServerEvent;
wxDEFINE_EVENT(wxEVT_RESTFUL_SERVER, RESTServerEvent);

Route* GPXLoadRoute1(pugi::xml_node& wpt_node, bool b_fullviz, bool b_layer,
                     bool b_layerviz, int layer_id, bool b_change);
Track* GPXLoadTrack1(pugi::xml_node& trk_node, bool b_fullviz, bool b_layer,
                     bool b_layerviz, int layer_id);
RoutePoint* GPXLoadWaypoint1(pugi::xml_node& wpt_node, wxString def_symbol_name,
                             wxString GUID, bool b_fullviz, bool b_layer,
                             bool b_layerviz, int layer_id);

bool InsertRouteA(Route* pTentRoute, NavObjectCollection1* navobj);
bool InsertTrack(Track* pTentTrack, bool bApplyChanges = false);
bool InsertWpt(RoutePoint* pWp, bool overwrite);

//  Some global variables to handle thread syncronization
static int return_status;
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

/* Used by static function, breaks the ordering. Better off in separate file */
class RESTServerEvent : public wxEvent {
public:
  RESTServerEvent(wxEventType commandType = wxEVT_RESTFUL_SERVER, int id = 0)
      : wxEvent(id, commandType){};
  ~RESTServerEvent(){};

  // accessors
  void SetPayload(std::shared_ptr<std::string> data) { m_payload = data; }
  void SetSource(std::string source) { m_source_peer = source; }
  void SetAPIKey(std::string key) { m_api_key = key; }
  std::shared_ptr<std::string> GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const {
    RESTServerEvent* newevent = new RESTServerEvent(*this);
    newevent->m_payload = this->m_payload;
    newevent->m_source_peer = this->m_source_peer;
    newevent->m_api_key = this->m_api_key;
    return newevent;
  };

  std::shared_ptr<std::string> m_payload;
  std::string m_source_peer;
  std::string m_api_key;
};

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data) {
  RESTServer* parent = static_cast<RESTServer*>(fn_data);

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
      RESTServerEvent Nevent(wxEVT_RESTFUL_SERVER, ORS_START_OF_SESSION);
      parent->AddPendingEvent(Nevent);
    }
  } else if (ev == MG_EV_HTTP_CHUNK) {
    struct mg_http_message* hm = (struct mg_http_message*)ev_data;
    if (mg_http_match_uri(hm, "/api/ping")) {
      std::string api_key;
      struct mg_str api_key_parm = mg_http_var(hm->query, mg_str("apikey"));
      if (api_key_parm.len && api_key_parm.ptr) {
        api_key = std::string(api_key_parm.ptr, api_key_parm.len);
      }

      struct mg_str source = mg_http_var(hm->query, mg_str("source"));

      if (source.len) {
        std::string source_peer(source.ptr, source.len);

        return_status = -1;
        if (parent) {
          RESTServerEvent Nevent(wxEVT_RESTFUL_SERVER, ORS_CHUNK_LAST);
          Nevent.SetSource(source_peer);
          Nevent.SetAPIKey(api_key);
          parent->AddPendingEvent(Nevent);
        }

        std::unique_lock<std::mutex> lock{mx};
        while (return_status < 0) {  // !predicate
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          return_status_condition.wait(lock);
        }
        lock.unlock();
      }
      mg_http_reply(c, 200, "", "{\"result\": %d}\n", return_status);
    } else if (mg_http_match_uri(hm, "/api/rx_object")) {
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

      return_status = -1;

      if (source.len) {
        std::string source_peer(source.ptr, source.len);
        // printf("%s\n", xml_content.c_str());

        // std::ofstream b_stream("bodyfile",  std::fstream::out |
        // std::fstream::binary); b_stream.write(hm->body.ptr, hm->body.len);

        if (parent) {
          RESTServerEvent Nevent(wxEVT_RESTFUL_SERVER, MID);
          if (xml_content.size()) {
            auto buffer = std::make_shared<std::string>(xml_content);
            Nevent.SetPayload(buffer);
          }
          Nevent.SetSource(source_peer);
          Nevent.SetAPIKey(api_key);
          parent->AddPendingEvent(Nevent);
        }

        if (MID == ORS_CHUNK_LAST) {
          std::unique_lock<std::mutex> lock{mx};
          while (return_status < 0) {  // !predicate
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return_status_condition.wait(lock);
          }
          lock.unlock();

          printf("Reply: %d\n", return_status);
          mg_http_reply(c, 200, "", "{\"result\": %d}\n", return_status);
          parent->UpdateRouteMgr();
        }
      }
    }

  } else if (ev == MG_EV_HTTP_MSG) {
#if 0
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/api/ping")) {
      std::string api_key;
      struct mg_str api_key_parm = mg_http_var(hm->query, mg_str("apikey"));
      if(api_key_parm.len && api_key_parm.ptr){
        api_key = std::string(api_key_parm.ptr, api_key_parm.len);
      }


      struct mg_str source = mg_http_var(hm->query, mg_str("source"));

      if(source.len)
      {
        std::string source_peer(source.ptr, source.len);

        return_status = -1;

        if (parent){
          RESTServerEvent Nevent(wxEVT_RESTFUL_SERVER, 0);
          Nevent.SetSource(source_peer);
          Nevent.SetAPIKey(api_key);
          parent->AddPendingEvent(Nevent);
        }

        std::unique_lock<std::mutex> lock{mx};
        while (return_status < 0) { // !predicate
          std::this_thread::sleep_for (std::chrono::milliseconds(100));
          return_status_condition.wait(lock);
        }
        lock.unlock();
      }

      mg_http_reply(c, 200, "", "{\"result\": %d}\n", return_status);
    } else if (mg_http_match_uri(hm, "/api/rx_object")) {

      std::string api_key;
      struct mg_str api_key_parm = mg_http_var(hm->query, mg_str("apikey"));
      if(api_key_parm.len && api_key_parm.ptr){
        api_key = std::string(api_key_parm.ptr, api_key_parm.len);
      }


      struct mg_str source = mg_http_var(hm->query, mg_str("source"));

      if(source.len && hm->body.len )
      {
        std::string xml_content(hm->body.ptr, hm->body.len);
        std::string source_peer(source.ptr, source.len);

        return_status = -1;

        if (parent){
          RESTServerEvent Nevent(wxEVT_RESTFUL_SERVER, 0);
          auto buffer = std::make_shared<std::string>(xml_content);
          Nevent.SetPayload(buffer);
          Nevent.SetSource(source_peer);
          Nevent.SetAPIKey(api_key);
          parent->AddPendingEvent(Nevent);
        }

        std::unique_lock<std::mutex> lock{mx};
        while (return_status < 0) { // !predicate
          std::this_thread::sleep_for (std::chrono::milliseconds(100));
          return_status_condition.wait(lock);
        }
        lock.unlock();
      }

      mg_http_reply(c, 200, "", "{\"result\": %d}\n", return_status);
    }
#endif
  }
  (void)fn_data;
}

std::string PINtoRandomKeyString(int dpin) {
  unsigned long long pin = PINtoRandomKey(dpin);
  char buffer[100];
  snprintf(buffer, sizeof(buffer) - 1, "%0llX", pin);
  return std::string(buffer);
}

class RESTServerThread : public wxThread {
public:
  RESTServerThread(RESTServer* Launcher, bool& m_portable);

  ~RESTServerThread(void);
  void* Entry();
  void OnExit(void);

private:
  RESTServer* m_pParent;
  bool& m_portable;
};

//========================================================================
/*    RESTServer implementation
 * */

RESTServer::RESTServer(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
    : m_Thread_run_flag(-1),
      m_dlg_ctx(ctx),
      m_route_ctx(route_ctx),
      m_portable(portable) {
  m_pin_dialog = 0;

  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_RESTFUL_SERVER, &RESTServer::HandleServerMessage, this);
}

RESTServer::~RESTServer() {}

bool RESTServer::StartServer(std::string certificate_location) {
  m_certificate_directory = certificate_location;
  m_cert_file = m_certificate_directory +
                std::string("cert.pem");  // Certificate PEM file
  m_key_file =
      m_certificate_directory + std::string("key.pem");  // The key PEM file

  // Load persistent config info
  LoadConfig();

  //    Kick off the  Server thread
  SetSecondaryThread(new RESTServerThread(this, m_portable));
  SetThreadRunFlag(1);
  GetSecondaryThread()->Run();

  return true;
}

void RESTServer::StopServer() {
  wxLogMessage(wxString::Format("Stopping REST service"));

  Unbind(wxEVT_RESTFUL_SERVER, &RESTServer::HandleServerMessage, this);

  //    Kill off the Secondary RX Thread if alive
  if (m_pSecondary_Thread) {
    m_pSecondary_Thread->Delete();

    if (m_bsec_thread_active)  // Try to be sure thread object is still alive
    {
      wxLogMessage("Stopping Secondary Thread");

      m_Thread_run_flag = 0;

      int tsec = 10;
      while ((m_Thread_run_flag >= 0) && (tsec--)) wxSleep(1);

      wxString msg;
      if (m_Thread_run_flag < 0)
        msg.Printf("Stopped in %d sec.", 10 - tsec);
      else
        msg.Printf("Not Stopped after 10 sec.");
      wxLogMessage(msg);
    }

    m_pSecondary_Thread = NULL;
    m_bsec_thread_active = false;
  }
}

bool RESTServer::LoadConfig(void) {
  if (TheBaseConfig()) {
    TheBaseConfig()->SetPath("/Settings/RESTServer");

    wxString key_string;

    TheBaseConfig()->Read("ServerKeys", &key_string);
    wxStringTokenizer st(key_string, ";");
    while (st.HasMoreTokens()) {
      wxString s1 = st.GetNextToken();
      wxString client_name = s1.BeforeFirst(':');
      wxString client_key = s1.AfterFirst(':');

      m_key_map[client_name.ToStdString()] = client_key.ToStdString();
    }
    TheBaseConfig()->Read("ServerOverwriteDuplicates", &m_b_overwrite, 0);
  }
  return true;
}

bool RESTServer::SaveConfig(void) {
  if (TheBaseConfig()) {
    TheBaseConfig()->SetPath("/Settings/RESTServer");

    wxString key_string;
    for (auto it : m_key_map) {
      wxString item =
          it.first.c_str() + wxString(":") + it.second.c_str() + wxString(";");
      key_string += item;
    }

    TheBaseConfig()->Write("ServerKeys", key_string);

    TheBaseConfig()->Write("ServerOverwriteDuplicates", m_b_overwrite);
  }
  return true;
}

void RESTServer::HandleServerMessage(RESTServerEvent& event) {
  int return_stat = RESTServerResult::RESULT_GENERIC_ERROR;

  if (event.GetId() == ORS_START_OF_SESSION) {
    // Prepare a temp file to catch chuncks that might follow
    m_tempUploadFilePath =
        wxFileName::CreateTempFileName("ocpn_tul").ToStdString();

    m_ul_stream.open(m_tempUploadFilePath.c_str(),
                     std::ios::out | std::ios::trunc);
    if (!m_ul_stream.is_open()) {
      wxLogMessage("REST_server: Cannot open %s for write",
                   m_tempUploadFilePath);
      m_tempUploadFilePath.clear();  // reset for next time.
      return;
    }
    return;
  }

  if (event.GetId() == ORS_CHUNK_N) {
    auto p = event.GetPayload();
    std::string* payload = p.get();

    // printf("%s\n", payload->c_str());
    //  Stream out to temp file
    if (m_tempUploadFilePath.size() && m_ul_stream.is_open()) {
      m_ul_stream.write(payload->c_str(), payload->size());
    }
    return;
  }

  if (event.GetId() == ORS_CHUNK_LAST) {
    // Cancel existing dialog
    m_dlg_ctx.close_dialog(m_pin_dialog);

    // Close the temp file.
    if (m_tempUploadFilePath.size() && m_ul_stream.is_open())
      m_ul_stream.close();

    // Server thread might be waiting for (return_status >= 0) on notify_one()
    return_stat = RESTServerResult::RESULT_GENERIC_ERROR;  // generic error
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
    m_dPIN = wxMin(rand() % 10000 + 1, 9999);
    m_sPIN.Printf("%04d", m_dPIN);

    std::string new_api_key = PINtoRandomKeyString(m_dPIN);

    //  Add new PIN to map
    m_key_map[event.m_source_peer] = new_api_key;

    // And persist it
    SaveConfig();

    wxString hmsg(event.m_source_peer.c_str());
    hmsg += " ";
    hmsg +=
        _("wants to send you new data.\nPlease enter the following PIN number "
          "on ");
    hmsg += wxString(event.m_source_peer.c_str());
    hmsg += _(" to pair with this device.\n");
    m_pin_dialog =
        m_dlg_ctx.show_dialog(hmsg.ToStdString(), m_sPIN.ToStdString());

    return_status = RESTServerResult::RESULT_NEW_PIN_REQUESTED;

    std::lock_guard<std::mutex> lock{mx};
    return_status_condition.notify_one();

    return;

  } else {
    return_status = RESTServerResult::RESULT_NO_ERROR;
  }

  // GUI dialogs can go here....
  bool b_cont;
  b_cont = true;

  if (b_cont) {  // Load the GPX file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(m_tempUploadFilePath.c_str());
    if (result.status == pugi::status_ok) {
      m_tempUploadFilePath.clear();  // empty for next time

      pugi::xml_node objects = doc.child("gpx");
      for (pugi::xml_node object = objects.first_child(); object;
           object = object.next_sibling()) {
        if (!strcmp(object.name(), "rte")) {
          Route* pRoute = NULL;
          pRoute = GPXLoadRoute1(object, true, false, false, 0, true);
          // Check for duplicate GUID
          bool b_add = true;
          bool b_overwrite_one = false;
          Route* duplicate = m_route_ctx.find_route_by_guid(pRoute->GetGUID());
          if (duplicate) {
            if (!m_b_overwrite) {
              auto result = m_dlg_ctx.run_accept_object_dlg(
                  _("The received route already exists on this "
                    "system.\nReplace?"),
                  _("Always replace objects from this source?"));

              if (result.status != ID_STG_OK) {
                b_add = false;
                return_stat = RESTServerResult::RESULT_DUPLICATE_REJECTED;
              } else {
                m_b_overwrite = result.check1_value;
                b_overwrite_one = true;
                SaveConfig();
              }
            }

            if (m_b_overwrite || b_overwrite_one) {
              //  Remove the existing duplicate route before adding new route
              m_route_ctx.delete_route(duplicate);
            }
          }

          if (b_add) {
            // And here is the payoff....

            // Add the route to the global list
            NavObjectCollection1 pSet;

            if (InsertRouteA(pRoute, &pSet))
              return_stat = RESTServerResult::RESULT_NO_ERROR;
            else
              return_stat = RESTServerResult::RESULT_ROUTE_INSERT_ERROR;
            m_dlg_ctx.top_level_refresh();
          }
        } else if (!strcmp(object.name(), "trk")) {
          Track* pRoute = NULL;
          pRoute = GPXLoadTrack1(object, true, false, false, 0);
          // Check for duplicate GUID
          bool b_add = true;
          bool b_overwrite_one = false;

          Track* duplicate = m_route_ctx.find_track_by_guid(pRoute->m_GUID);
          if (duplicate) {
            if (!m_b_overwrite) {
              auto result = m_dlg_ctx.run_accept_object_dlg(
                  _("The received track already exists on this "
                    "system.\nReplace?"),
                  _("Always replace objects from this source?"));

              if (result.status != ID_STG_OK) {
                b_add = false;
                return_stat = RESTServerResult::RESULT_DUPLICATE_REJECTED;
              } else {
                m_b_overwrite = result.check1_value;
                b_overwrite_one = true;
                SaveConfig();
              }
            }
            if (m_b_overwrite || b_overwrite_one) {
              m_route_ctx.delete_track(duplicate);
            }
          }

          if (b_add) {
            // And here is the payoff....

            // Add the route to the global list
            NavObjectCollection1 pSet;

            if (InsertTrack(pRoute, false))
              return_stat = RESTServerResult::RESULT_NO_ERROR;
            else
              return_stat = RESTServerResult::RESULT_ROUTE_INSERT_ERROR;
            m_dlg_ctx.top_level_refresh();
          }
        } else if (!strcmp(object.name(), "wpt")) {
          RoutePoint* pWp = NULL;
          pWp = GPXLoadWaypoint1(object, "circle", "", false, false, false, 0);
          // Check for duplicate GUID
          bool b_add = true;
          bool b_overwrite_one = false;

          RoutePoint* duplicate =
              WaypointExists(pWp->GetName(), pWp->m_lat, pWp->m_lon);
          if (duplicate) {
            if (!m_b_overwrite) {
              auto result = m_dlg_ctx.run_accept_object_dlg(
                  _("The received waypoint already exists on this "
                    "system.\nReplace?"),
                  _("Always replace objects from this source?"));

              if (result.status != ID_STG_OK) {
                b_add = false;
                return_stat = RESTServerResult::RESULT_DUPLICATE_REJECTED;
              } else {
                m_b_overwrite = result.check1_value;
                b_overwrite_one = true;
                SaveConfig();
              }
            }
          }

          if (b_add) {
            // And here is the payoff....
            if (InsertWpt(pWp, m_b_overwrite || b_overwrite_one))
              return_stat = RESTServerResult::RESULT_NO_ERROR;
            else
              return_stat = RESTServerResult::RESULT_ROUTE_INSERT_ERROR;

            m_dlg_ctx.top_level_refresh();
          }
        }
      }
    }
  } else {
    return_stat = RESTServerResult::RESULT_OBJECT_REJECTED;
  }

  return_status = return_stat;

  std::lock_guard<std::mutex> lock{mx};
  return_status_condition.notify_one();
}

RESTServerThread::RESTServerThread(RESTServer* Launcher, bool& portable)
    : m_portable(portable) {
  m_pParent = Launcher;  // This thread's immediate "parent"

  server_ip = s_https_addr;
  // If Portable use another port
  if (m_portable) {
    server_ip = s_https_addr_portable;
    wxString sip(server_ip);
    wxLogMessage("Portable REST server IP: Port " + sip);
  }

  Create();
}

RESTServerThread::~RESTServerThread(void) {}

void RESTServerThread::OnExit(void) {}

void* RESTServerThread::Entry() {
  bool not_done = true;
  m_pParent->SetSecThreadActive();  // I am alive

  struct mg_mgr mgr;        // Event manager
  mg_log_set(MG_LL_DEBUG);  // Set log level
  mg_mgr_init(&mgr);        // Initialise event manager

  mg_http_listen(&mgr, server_ip.c_str(), fn,
                 m_pParent);  // Create HTTPS listener
  // mg_http_listen(&mgr, s_https_addr, fn, (void *) 1);   // (HTTPS listener)

  for (;;) mg_mgr_poll(&mgr, 1000);  // Infinite event loop
  mg_mgr_free(&mgr);

  m_pParent->SetSecThreadInActive();  // I am dead
  m_pParent->m_Thread_run_flag = -1;

  return 0;
}
