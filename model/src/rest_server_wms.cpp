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

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <wx/event.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/string.h>

#include "config.h"

#include "model/logger.h"

#include "model/rest_server_wms.h"

#include "mongoose.h"
#include "observable_evt.h"

/** Event from IO thread to main */
wxDEFINE_EVENT(REST_IO_EVT, ObservedEvt);

using namespace std::chrono_literals;

static const char* const ServerAddr = "http://0.0.0.0:8081";

static const char* const kVersionReply = R"--({ "version": "@version@" })--";

unsigned int RestServerWms::m_hitcount = 0;


/** Kind of messages sent from io thread to main code. */
enum { ORS_START_OF_SESSION, ORS_CHUNK_N, ORS_CHUNK_LAST };



/** Extract a HTTP variable from query string. */
static inline std::string HttpVarToString(const struct mg_str& query,
                                          const char* var) {
  std::string string;
  struct mg_str mgs = mg_http_var(query, mg_str(var));
  if (mgs.len && mgs.ptr) string = std::string(mgs.ptr, mgs.len);
  return string;
}




// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data) {
  int j = 0;
  
  if (ev == MG_EV_HTTP_MSG) {
    ++RestServerWms::m_hitcount;

    struct mg_http_message* hm = (struct mg_http_message*)ev_data;

    if (mg_match(hm->uri, mg_str("/api/hitcount"), NULL)) {
      mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "{hitcount:%lu}\n", RestServerWms::m_hitcount);
    } else if (mg_match(hm->uri, mg_str("/api/wms"), NULL)) {
      std::string strWidthPx = HttpVarToString(hm->query, "WIDTH");
      std::string strHeightPx = HttpVarToString(hm->query, "HEIGHT");

      mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "{\"width\":%s}\n", strWidthPx.c_str());


    } else if (mg_match(hm->uri, mg_str("/api/sum"), NULL)) {
      // Expecting JSON array in the HTTP body, e.g. [ 123.38, -2.72 ]
      double num1, num2;
      if (mg_json_get_num(hm->body, "$[0]", &num1) &&
          mg_json_get_num(hm->body, "$[1]", &num2)) {
        // Success! create JSON response
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{%m:%g}\n",
                      0, "result", num1 + num2);
      } else {
        mg_http_reply(c, 500, NULL, "Parameters missing\n");
      }
    } else {
      mg_http_reply(c, 500, NULL, "\n");
    }
  }
}

//========================================================================
/*    RestServer implementation */

void RestServerWms::Run() {
  
  struct mg_mgr mgr = {0};  // Event manager
  mg_log_set(MG_LL_DEBUG);  // Set log level
  mg_mgr_init(&mgr);        // Initialise event manager

  // Create HTTPS listener
  MESSAGE_LOG << "Listening on " << ServerAddr << "\n";
  mg_http_listen(&mgr, ServerAddr, fn, this);

  while (m_alive) {
    mg_mgr_poll(&mgr, 10);  // Infinite event loop //TODO set the time depending on activity bursts, WMS often load 9 times rapidly
  }
  mg_mgr_free(&mgr);
}

void RestServerWms::StopServer() {
  m_alive = false; //TODO teardown might hang, need the rest_server style teardown 
  if (m_workerthread.joinable()) {
    m_workerthread.join();
  }
}

RestServerWms::RestServerWms() {
}   

RestServerWms::~RestServerWms() {
  StopServer();
}

bool RestServerWms::StartServer() {
  m_workerthread = std::thread([&]() { RestServerWms::Run(); });
  return true;
}


//void RestServerWms::HandleServerMessage(ObservedEvt& event) {
//  auto evt_data = UnpackEvtPointer<RestIoEvtData>(event);
//  m_reply_body = "";
//  switch (event.GetId()) {
//    case ORS_START_OF_SESSION:
//      // Prepare a temp file to catch chuncks that might follow
//      m_upload_path = wxFileName::CreateTempFileName("ocpn_tul").ToStdString();
//
//      m_ul_stream.open(m_upload_path.c_str(), std::ios::out | std::ios::trunc);
//      if (!m_ul_stream.is_open()) {
//        wxLogMessage("REST_server: Cannot open %s for write", m_upload_path);
//        m_upload_path.clear();  // reset for next time.
//        return;
//      }
//      return;
//    case ORS_CHUNK_N:
//      //  Stream out to temp file
//      if (!m_upload_path.empty() && m_ul_stream.is_open()) {
//        m_ul_stream.write(evt_data->payload.c_str(), evt_data->payload.size());
//      }
//      return;
//    case ORS_CHUNK_LAST:
//      // Cancel existing dialog and close temp file
//      if (m_pin_dialog) wxQueueEvent(m_pin_dialog, new wxCloseEvent);
//      if (!m_upload_path.empty() && m_ul_stream.is_open()) m_ul_stream.close();
//      break;
//  }
//
//  if (!CheckApiKey(*evt_data)) {
//    UpdateReturnStatus(RestServerResult::NewPinRequested);
//    return;
//  }
//
//  switch (evt_data->cmd) {
//    case RestIoEvtData::Cmd::Ping:
//      UpdateReturnStatus(RestServerResult::NoError);
//      return;
//    case RestIoEvtData::Cmd::CheckWrite: {
//      auto guid = evt_data->payload;
//      auto dup = m_route_ctx.find_route_by_guid(guid);
//      if (!dup || evt_data->force || m_overwrite) {
//        UpdateReturnStatus(RestServerResult::NoError);
//      } else {
//        UpdateReturnStatus(RestServerResult::DuplicateRejected);
//      }
//      return;
//    }
//    
//   
//  }
// }


