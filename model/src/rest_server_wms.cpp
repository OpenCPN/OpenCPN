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


static const char* const ServerAddr = "http://0.0.0.0:8081";

unsigned int RestServerWms::m_hitcount = 0;
wxFrame* RestServerWms::m_pWxFrame = nullptr;
ChartCanvas* RestServerWms::m_pChartCanvas = nullptr;

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


//static void ConvertCoordinate(double lat, double lng, double& lat4326,
//                       double& lon4326) {
//  double x = lng * 20037508.34 / 180;
//  double y = log(tan((90 + lat) * PI / 360)) / (PI / 180);
//  y = y * 20037508.34 / 180;
//  lat4326 = x;
//  lon4326 = y;
//};

//https://gist.github.com/onderaltintas/6649521//
void coord3857To4326(double lon3857, double lat3857, double& lat4326,
                     double& lon4326) {
 
    lon4326 = lon3857 * 180 / 20037508.34;
    //lat4326 = Math.atan(Math.exp(y * Math.PI / 20037508.34)) * 360 / Math.PI - 90;
    lat4326 =
        atan(exp(lat3857 * PI / 20037508.34)) * 360 / PI - 90;
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
      try {
        std::string strService = HttpVarToString(hm->query, "service");
        std::string strRequest = HttpVarToString(hm->query, "request");
        std::string strFormat = HttpVarToString(hm->query, "format");
        std::string strWidthPx = HttpVarToString(hm->query, "width");
        std::string strHeightPx = HttpVarToString(hm->query, "height");
        std::string strSrs = HttpVarToString(hm->query, "srs");
        std::string strBbox = HttpVarToString(hm->query, "bbox");

        // BBox manging
        std::stringstream ss(strBbox);
        std::vector<double> data;

        while (ss.good()) {
          std::string substr;
          getline(ss, substr, ',');
          double d = std::stod(substr);
          // bbox_split += substr + "\n";

          data.push_back(d);
        }

        // coord convertion
        double lonSW, latSW, lonNE, latNE;
        coord3857To4326(data[0], data[1], lonSW, latSW);
        coord3857To4326(data[2], data[3], lonNE, latNE);
        // m_pchart

        ;

        RestServerWms::m_pChartCanvas->SetShowGrid(true);
        RestServerWms::m_pChartCanvas->SetShowENCLights(true);
        RestServerWms::m_pChartCanvas->SetShowENCDepth(true);

        RestServerWms::m_pChartCanvas->SetViewPointByCorners(latSW, lonSW,
                                                             latNE, lonNE);

        RestServerWms::m_pChartCanvas->Refresh();
        RestServerWms::m_pWxFrame->Refresh();

        wxClientDC dcWindow(RestServerWms::m_pWxFrame);
        wxCoord screenWidth, screenHeight;

        dcWindow.GetSize(&screenWidth, &screenHeight);

        wxBitmap screenshot(screenWidth, screenHeight, -1);
        wxMemoryDC memDC;
        memDC.SelectObject(screenshot);
        memDC.Clear();
        memDC.Blit(0, 0,  // Copy to coordinate
                   screenWidth, screenHeight, &dcWindow, 0,
                   0  // offset in the original DC
        );
        memDC.SelectObject(wxNullBitmap);
        screenshot.SaveFile("c:\\temp\\opencpn_wms_imgs\\img_" + std::to_string(RestServerWms::m_hitcount) + ".jpg",
                            wxBITMAP_TYPE_JPEG);

        //mg_http_reply(c, 200, "Content-Type: application/json\r\n",
        //              "{\"width\":%s}\n", strWidthPx.c_str());

         mg_http_reply(c, 200, "Content-Type: image/jpeg\r\n", "");
        //               
      }
      catch(const std::exception& ex){
        int j = 0;
      }
      catch(...){
        int j = 0;
      }

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

RestServerWms::RestServerWms(){
  //dummy debug 
  double lat, lon;
  coord3857To4326(-8575605.398444, 4707174.018280 , lon,
                  lat);

  //https://epsg.io/transform#s_srs=3857&t_srs=4326&x=-8575605.3984440&y=4707174.0182800
  //x,y input  -8575605.398444, 4707174.018280
  //lon, lat expecting: -77.035974, 38.898717
  int j = 0;

}   

RestServerWms::~RestServerWms() {
  StopServer();
}

void RestServerWms::RunDelayedLoader() {
  try {
    std::this_thread::sleep_for(std::chrono::seconds(5));

   
    


    std::this_thread::sleep_for(std::chrono::seconds(2));

    m_pChartCanvas->SetSize(wxSize(400, 400));
    //m_pChartCanvas->SetDisplaySizeMM
    m_pChartCanvas->SetViewPointByCorners(10, 57, 11, 58);

  } catch (const std::exception& ex) {
    int j = 0;
  }
}
bool RestServerWms::StartServer() {

  m_pWxFrame = new wxFrame(nullptr, -1, "WMS");
  m_pWxFrame->Show();
  m_pChartCanvas = new ChartCanvas(m_pWxFrame, 10);
  m_pChartCanvas->SetSize(wxSize(400, 400));

  m_delayedLoaderThread = std::thread([&]() { RestServerWms::RunDelayedLoader(); });
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


