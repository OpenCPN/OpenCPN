/***************************************************************************
 *   Copyright (C) 2025 Jes Ramsing                                        *
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

#include "WmsFrame.h"
#include <wx/log.h>
#include <wx/mstream.h>
#include "config.h"
#include "model/logger.h"
#include "model/rest_server_wms.h"
#include "mongoose.h"


wxDEFINE_EVENT(WXWMSREQUESTEVENT, wxWMSRequestEvent);

WmsFrame::WmsFrame(wxWindow* parent, wxWindowID id, const wxString& title,
                   const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style) {

  pText = new wxStaticText(this, wxID_STATIC, wxT("Clean"));
  pText->SetForegroundColour(wxColor("red"));

  jpegdatabuffer = new char[10000000];

  Bind(WXWMSREQUESTEVENT, &WmsFrame::OnWmsRequestEvent, this);
}

WmsFrame::~WmsFrame() {
}

void WmsFrame::AssignTargetObjects(MyFrame* pF, ChartCanvas* pC) {
  m_pTgtFrame = pF;
  m_pChartCanvas = pC;
}

void WmsFrame::OnWmsRequestEvent(wxWMSRequestEvent& event) {
  try {
    // All the real work is done in here:
    // 1. resize if needed
    // 2. color change if needed
    wxLogMessage("wxWMSRequestEvent event received");

    if (event.p.w != lastSize_W || event.p.h != lastSize_H) {
      INFO_LOG << "Size req change  from (w,h)" << lastSize_W << ", "
               << lastSize_H << " to " << event.p.w << ", " << event.p.h;

      lastSize_W = event.p.w;
      lastSize_H = event.p.h;
      wxLogMessage("dimension change req to w: %i h:%i", event.p.w, event.p.h);
      
      int newChartW = event.p.w + 2;
      //int newChartH = event.p.h + 174;
      int newChartH = event.p.h + 204;

      m_pTgtFrame->SetSize(newChartW+300, newChartH+300);

      m_pChartCanvas->SetSize(wxSize(newChartW, newChartH));
    }

    // m_pchart
    m_pChartCanvas->SetShowGrid(true);
    m_pChartCanvas->SetShowENCLights(true);
    m_pChartCanvas->SetShowENCDepth(true);
    m_pChartCanvas->SetShowAIS(false);
    m_pChartCanvas->SetShowGPS(false);
    m_pChartCanvas->SetShowGPSCompassWindow(false);
    m_pChartCanvas->SetShowDepthUnits(false);

    //TODO fix color change
    if (lastcolor != event.p.color) {
      lastcolor = event.p.color;

      if (lastcolor == "DAY") {
        m_pTgtFrame->SetAndApplyColorScheme(
            ColorScheme::GLOBAL_COLOR_SCHEME_DAY);
      } else if (lastcolor == "DUSK") {
        m_pTgtFrame->SetAndApplyColorScheme(
            ColorScheme::GLOBAL_COLOR_SCHEME_DUSK);
      } else if (lastcolor == "NIGHT") {
        m_pTgtFrame->SetAndApplyColorScheme(
            ColorScheme::GLOBAL_COLOR_SCHEME_NIGHT);
      } else {
        m_pTgtFrame->SetAndApplyColorScheme(
            ColorScheme::GLOBAL_COLOR_SCHEME_DAY); //catch all
      }
    }
    
    m_pChartCanvas->canvasChartsRefresh(-1);
    m_pChartCanvas->SetQuiltMode(true);

    bool ok_setviewpointbycorners = m_pChartCanvas->SetViewPointByCorners(
        event.p.latSW, event.p.lonSW, event.p.latNE, event.p.lonNE);

    bool update = m_pChartCanvas->DoCanvasUpdate();
    DEBUG_LOG << "CanvasUpdate success:" << update;

    //intermediate window text update - just to see what is requested
    std::stringstream ssImgInfo;
    ssImgInfo << event.p.hitcount << "\n  NE" << event.p.latNE << ", "
              << event.p.lonNE << "\nSW" << event.p.latSW << ", "
              << event.p.lonSW << "\n" << event.p.color;

    pText->SetLabelText(ssImgInfo.str());

    m_pChartCanvas->Refresh();
    this->Refresh();
    m_pChartCanvas->Update();
    this->Update();

    wxClientDC dcWindow(m_pTgtFrame);  // was this
    wxCoord screenWidth, screenHeight;

    dcWindow.GetSize(&screenWidth, &screenHeight);

    wxBitmap screenshot(lastSize_W, lastSize_H, -1);
    wxMemoryDC memDC;
    memDC.SelectObject(screenshot);
    memDC.Clear();
    memDC.Blit(0, 0,  // Copy to coordinate
               lastSize_W, lastSize_H, &dcWindow, 0,
               0  // offset in the original DC
    );
    memDC.SelectObject(wxNullBitmap);
    std::string filename = "c:\\temp\\opencpn_wms_imgs\\img_" +
                           std::to_string(event.p.hitcount) + ".jpg";
    screenshot.SaveFile(filename, wxBITMAP_TYPE_JPEG);

    // get byte array for sending
    wxImage img = screenshot.ConvertToImage();
    wxJPEGHandler* pH = new wxJPEGHandler();
    img.AddHandler(pH);
    wxMemoryOutputStream s;
    bool saved = img.SaveFile(s, wxBITMAP_TYPE_JPEG);
    if (!saved) {
      int j = 9;
    }

    size_t size = s.GetSize();

    // Copy data from the stream to the external buffer
    s.CopyTo(jpegdatabuffer, size);

    // Warning - the reply generation is to be treated as a rotten mango
    // this works, mg_http_reply, chunk writing etc did not. Suspecting that
    // nulls inside the char* causes transmission to end prematurely - much like
    // the stringlength counted only to first null.... Waisted a whole day,
    // wireshark finally gave the clue as to what what wrong
    mg_printf(event.p.c, "HTTP/1.1 200 OK\r\n");
    mg_printf(event.p.c, "Content-Type: image/jpeg\r\n");

    std::string header_conlength =
        "Content-Length: " + std::to_string(size) + "\r\n\r\n";
    mg_printf(event.p.c, header_conlength.c_str());
    mg_send(event.p.c, jpegdatabuffer, size);
    event.p.c->is_resp = 0;

    INFO_LOG << "WMS replied to hit:" << event.p.hitcount << " size:" << size;
  } catch (const std::exception& ex) {
    wxLogError(ex.what());
  }
  catch(...){
    ERROR_LOG << "... exception (unhandled) - critical problem somewhere";
  }
}
