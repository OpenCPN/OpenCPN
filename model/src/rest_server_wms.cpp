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

#include <wx/log.h>
#include <wx/string.h>
#include <wx/mstream.h>

#include "config.h"

#include "model/logger.h"

#include "model/rest_server_wms.h"

#include "mongoose.h"

#define RESTSERVERWMS

static const char* const ServerAddr = "http://0.0.0.0:8081";

unsigned int RestServerWms::m_hitcount = 0;

unsigned int RestServerWms::lastSize_W = 0;
unsigned int RestServerWms::lastSize_H = 0;

std::function<void(WmsReqParams)> RestServerWms::fCallback;

#ifdef RESTSERVERWMS
wxFrame* RestServerWms::m_pWxFrame = nullptr;
ChartCanvas* RestServerWms::m_pChartCanvas = nullptr;
wxStaticText* RestServerWms::pText = nullptr;

void* RestServerWms::jpegdatabuffer = new char[1000000];
#endif



/** Extract a HTTP variable from query string. */
static inline std::string HttpVarToString(const struct mg_str& query,
                                          const char* var) {
  std::string string;
  struct mg_str mgs = mg_http_var(query, mg_str(var));
  if (mgs.len && mgs.ptr) string = std::string(mgs.ptr, mgs.len);
  return string;
}

//https://gist.github.com/onderaltintas/6649521//
void coord3857To4326(double lon3857, double lat3857, double& lat4326,
                     double& lon4326) {
 
    lon4326 = lon3857 * 180 / 20037508.34;
    //lat4326 = Math.atan(Math.exp(y * Math.PI / 20037508.34)) * 360 / Math.PI - 90;
    lat4326 =
        atan(exp(lat3857 * PI / 20037508.34)) * 360 / PI - 90;
}

std::string unescape(const std::string& StrIn) {
  int newLen = 0;
  char* pszUnescaped = CPLUnescapeString(StrIn.c_str(), &newLen, CPLES_URL);
  std::string sNew(pszUnescaped);
  CPLFree(pszUnescaped);
  return sNew;
}

// entrypoint for mongoose
static void fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data) {
 #ifdef RESTSERVERWMS
  
  if (ev == MG_EV_HTTP_MSG) {
    ++RestServerWms::m_hitcount;

    struct mg_http_message* hm = (struct mg_http_message*)ev_data;

    if (mg_match(hm->uri, mg_str("/api/hitcount"), NULL)) {
      mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "{hitcount:%lu}\n", RestServerWms::m_hitcount);
    } else if (mg_match(hm->uri, mg_str("/api/wms"), NULL)) {
      try {
        if (mg_match(hm->uri, mg_str("favicon.ico"), NULL
        )){
          mg_http_reply(c, 404,"","");
          return;
        }

        std::string strService = HttpVarToString(hm->query, "service");
        std::string strRequest = HttpVarToString(hm->query, "request");
        std::string strFormat = HttpVarToString(hm->query, "format");
        std::string strWidthPx = HttpVarToString(hm->query, "width");
        std::string strHeightPx = HttpVarToString(hm->query, "height");
        std::string strSrs = HttpVarToString(hm->query, "srs");
        std::string strBbox = HttpVarToString(hm->query, "bbox");

        strBbox = unescape(strBbox);
        strSrs = unescape(strSrs);

        //check for resizing
        int _w = std::stoi(strWidthPx);
        int _h = std::stoi(strHeightPx);
        if (_w != RestServerWms::lastSize_W || _h != RestServerWms::lastSize_H){
          INFO_LOG << "Size req change  from (w,h)" << RestServerWms::lastSize_W
                   << ", " << RestServerWms::lastSize_H << " to " << _w << ", "
                   << _h;
 
          RestServerWms::lastSize_W = _w;
          RestServerWms::lastSize_H = _h;

           RestServerWms::m_pWxFrame->SetSize(wxSize(_w + 100, _h + 100));
           RestServerWms::m_pChartCanvas->SetSize(wxSize(_w, _h));
        }

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

        if (data.size() != 4) {
          mg_http_reply(
              c, 422, "",
              "Unable to contnue, bbox data not resulting in 4 params");
          return;
        }

        double lonSW, latSW, lonNE, latNE;

        if (strSrs == "EPSG:4326") {
          latSW = data[0];
          lonSW = data[1];
          latNE = data[2];
          lonNE = data[3];

        } else if (strSrs == "EPSG:3857") {
          
          // coord convertion
          coord3857To4326(data[0], data[1], lonSW, latSW);
          coord3857To4326(data[2], data[3], lonNE, latNE);
        } else{
          std::string err = "Unsupported Srs param:" + strSrs;

          mg_http_reply(c, 422, "", err.c_str());
              
          return;
        }
        
        INFO_LOG << "WMS req " << RestServerWms::m_hitcount << " SW:" << latSW
                 << "," << lonSW << " NE" << latNE << "," << lonNE
                 << "(lat, lon)"; 

        WmsReqParams p;
        p.w = _w;
        p.h = _h;
        p.latNE = latNE;
        p.lonNE = lonNE;
        p.latSW = latSW;
        p.lonSW = lonSW;
        p.hitcount = RestServerWms::m_hitcount;

        p.c = c;

        RestServerWms::fCallback(p);

        // m_pchart
        RestServerWms::m_pChartCanvas->SetShowGrid(true);
        RestServerWms::m_pChartCanvas->SetShowENCLights(true);
        //RestServerWms::m_pChartCanvas->SetShowENCDepth(true);
        RestServerWms::m_pChartCanvas->SetShowAIS(false);
        RestServerWms::m_pChartCanvas->SetShowGPS(false);
        RestServerWms::m_pChartCanvas->SetShowGPSCompassWindow(false);
        
        RestServerWms::m_pChartCanvas->canvasChartsRefresh(-1);

        
        bool ok_setviewpointbycorners = RestServerWms::m_pChartCanvas->SetViewPointByCorners(latSW, lonSW,
                                                             latNE, lonNE);
        INFO_LOG << "SetViewpointByCorners ok: " << ok_setviewpointbycorners; 

        //RestServerWms::m_pChartCanvas->SetSize(
        //    wxSize(std::stoi(strWidthPx),
        //           std::stoi(strHeightPx)));

        bool update = RestServerWms::m_pChartCanvas->DoCanvasUpdate();
        INFO_LOG << "CanvasUpdate success:" << update;

        //RestServerWms::m_pChartCanvas->
        /*
        RestServerWms::m_pChartCanvas->Refresh();
        RestServerWms::m_pWxFrame->Refresh();
        RestServerWms::m_pChartCanvas->Update();
        RestServerWms::m_pWxFrame->Update();

        std::stringstream ssImgInfo;
        ssImgInfo << RestServerWms::m_hitcount << "\n  NE" << latNE << ", "
                  << lonNE << "\nSW" << latSW << ", " << lonSW;
        RestServerWms::pText->SetLabelText(
            ssImgInfo.str());

      RestServerWms::m_pWxFrame->Update();

      std::this_thread::sleep_for(std::chrono::milliseconds(599));
        wxClientDC dcWindow(RestServerWms::m_pWxFrame);
        wxCoord screenWidth, screenHeight;

        dcWindow.GetSize(&screenWidth, &screenHeight);

        wxBitmap screenshot(RestServerWms::lastSize_W,
                            RestServerWms::lastSize_H, -1);
        wxMemoryDC memDC;
        memDC.SelectObject(screenshot);
        memDC.Clear();
        memDC.Blit(0, 0,  // Copy to coordinate
                   RestServerWms::lastSize_W, RestServerWms::lastSize_H, &dcWindow, 0,
                   0  // offset in the original DC
        );
        memDC.SelectObject(wxNullBitmap);
        std::string filename = "c:\\temp\\opencpn_wms_imgs\\img_" +
                               std::to_string(RestServerWms::m_hitcount) +
                               ".jpg";
        screenshot.SaveFile(filename,
                            wxBITMAP_TYPE_JPEG);
        
        //get byte array for sending
        wxImage img = screenshot.ConvertToImage();
        wxJPEGHandler* pH = new wxJPEGHandler();
        img.AddHandler(pH);
        wxMemoryOutputStream s;
        img.SaveFile(s, wxBITMAP_TYPE_JPEG);

        size_t size = s.GetSize();      

        // Copy data from the stream to the external buffer
        s.CopyTo(RestServerWms::jpegdatabuffer, size);

        //Warning - the reply generation is messed up (or I am an idiot)
        //this works, mg_http_reply, chunk writing etc did not. Suspecting that nulls inside the char* causes transmission to end prematurely - much like the stringlength counted only to first null.... Waisted a whole day, wireshark finally gave the clue as to what what wrong 
        mg_printf(c, "HTTP/1.1 200 OK\r\n");
        mg_printf(c, "Content-Type: image/jpeg\r\n");

        std::string header_conlength = "Content-Length: " + std::to_string(size) + "\r\n\r\n"; 
        mg_printf(c, header_conlength.c_str());
        mg_send(c, RestServerWms::jpegdatabuffer, size);
        c->is_resp = 0;
        
        INFO_LOG << "WMS replied to hit:" << RestServerWms::m_hitcount
                 << " size:" << size;

        */
      }
      catch(const std::exception& ex){
        int j = 0;
        ERROR_LOG << "std::exception in rendering, details:" << ex.what();
      }
      catch(...){
        int j = 0;
        ERROR_LOG << "... exception in rendering";
      }
    } else {
      mg_http_reply(c, 500, NULL, "\n");
    }
  }
  #endif
}

//========================================================================
/*    RestServer implementation */

void RestServerWms::Run() {
  #ifdef RESTSERVERWMS
  struct mg_mgr mgr = {0};  // Event manager
  mg_log_set(MG_LL_DEBUG);  // Set log level
  mg_mgr_init(&mgr);        // Initialise event manager

  // Create HTTPS listener
  MESSAGE_LOG << "Listening on " << ServerAddr << "\n";
  mg_http_listen(&mgr, ServerAddr, fn, this);

  while (m_alive) {
    mg_mgr_poll(&mgr, 1);  // Infinite event loop //TODO set the time depending on activity bursts, WMS often load 9 times rapidly
  }
  mg_mgr_free(&mgr);
  #endif
}

void RestServerWms::StopServer() {
  m_alive = false; //TODO teardown might hang, need the rest_server style teardown 
  if (m_workerthread.joinable()) {
    m_workerthread.join();
  }
}

RestServerWms::RestServerWms() {  
  INFO_LOG << "RestServerWms consctuction, call to StartServer will start the rendering engine";
}   

RestServerWms::~RestServerWms() {
  StopServer();
}


bool RestServerWms::StartServer(std::function<void(WmsReqParams)> FCallback) {
  RestServerWms::fCallback = FCallback;
  #ifdef RESTSERVERWMS
  
  m_pWxFrame = new wxFrame(nullptr, -1, "WMS");
  m_pWxFrame->Show();
  pText = new wxStaticText(m_pWxFrame, wxID_STATIC, wxT("Clean"));
  pText->SetForegroundColour(wxColor("red"));

  m_pChartCanvas = new ChartCanvas(m_pWxFrame, 10);
  m_pChartCanvas->SetPosition(wxPoint(0, 0));
  m_pChartCanvas->SetSize(wxSize(100, 100));

  m_workerthread = std::thread([&]() { RestServerWms::Run(); });

  //m_pWxFrame->Refresh();
  //m_pWxFrame->Update();

  #endif
  return true;
}
