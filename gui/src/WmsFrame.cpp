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
  // this->SetSizeHints(wxSize(500, 300), wxDefaultSize);

  pText = new wxStaticText(this, wxID_STATIC, wxT("Clean"));
  pText->SetForegroundColour(wxColor("red"));

  m_pChartCanvas = new ChartCanvas(this, 10);
  m_pChartCanvas->SetPosition(wxPoint(0, 0));
  m_pChartCanvas->SetSize(wxSize(100, 100));

  Bind(WXWMSREQUESTEVENT, &WmsFrame::OnWmsRequestEvent, this);
}

WmsFrame::~WmsFrame() {
}

void WmsFrame::OnWmsRequestEvent(wxWMSRequestEvent& event) {
  // Handle the custom event
  wxLogMessage("wxWMSRequestEvent event received");

  if (event.p.w != lastSize_W || event.p.h != lastSize_H) {
    INFO_LOG << "Size req change  from (w,h)" << lastSize_W << ", "
             << lastSize_H << " to " << event.p.w << ", " << event.p.h;

 

    lastSize_W = event.p.w;
    lastSize_H = event.p.h;

    this->SetSize(wxSize(event.p.w + 100, event.p.h + 100));
    m_pChartCanvas->SetSize(wxSize(event.p.w, event.p.h));
  }

  
        // m_pchart
  m_pChartCanvas->SetShowGrid(true);
  m_pChartCanvas->SetShowENCLights(true);
  // m_pChartCanvas->SetShowENCDepth(true);
  m_pChartCanvas->SetShowAIS(false);
  m_pChartCanvas->SetShowGPS(false);
  m_pChartCanvas->SetShowGPSCompassWindow(false);

  m_pChartCanvas->canvasChartsRefresh(-1);

  bool ok_setviewpointbycorners = m_pChartCanvas->SetViewPointByCorners(
      event.p.latSW, event.p.lonSW, event.p.latNE, event.p.lonNE);

  bool update = m_pChartCanvas->DoCanvasUpdate();
  INFO_LOG << "CanvasUpdate success:" << update;


  std::stringstream ssImgInfo;
  ssImgInfo << event.p.hitcount << "\n  NE" << event.p.latNE << ", "
            << event.p.lonNE << "\nSW" << event.p.latSW << ", "
            << event.p.lonSW;
  pText->SetLabelText(ssImgInfo.str());

  this->Update();


  wxClientDC dcWindow(RestServerWms::m_pWxFrame);
  wxCoord screenWidth, screenHeight;

  dcWindow.GetSize(&screenWidth, &screenHeight);

  wxBitmap screenshot(RestServerWms::lastSize_W, RestServerWms::lastSize_H, -1);
  wxMemoryDC memDC;
  memDC.SelectObject(screenshot);
  memDC.Clear();
  memDC.Blit(0, 0,  // Copy to coordinate
             RestServerWms::lastSize_W, RestServerWms::lastSize_H, &dcWindow, 0,
             0  // offset in the original DC
  );
  memDC.SelectObject(wxNullBitmap);
  std::string filename = "c:\\temp\\opencpn_wms_imgs\\img_" +
                         std::to_string(RestServerWms::m_hitcount) + ".jpg";
  screenshot.SaveFile(filename, wxBITMAP_TYPE_JPEG);

  // get byte array for sending
  wxImage img = screenshot.ConvertToImage();
  wxJPEGHandler* pH = new wxJPEGHandler();
  img.AddHandler(pH);
  wxMemoryOutputStream s;
  img.SaveFile(s, wxBITMAP_TYPE_JPEG);

  size_t size = s.GetSize();

  // Copy data from the stream to the external buffer
  s.CopyTo(RestServerWms::jpegdatabuffer, size);

  // Warning - the reply generation is messed up (or I am an idiot)
  // this works, mg_http_reply, chunk writing etc did not. Suspecting that nulls
  // inside the char* causes transmission to end prematurely - much like the
  // stringlength counted only to first null.... Waisted a whole day, wireshark
  // finally gave the clue as to what what wrong
  mg_printf(event.p.c, "HTTP/1.1 200 OK\r\n");
  mg_printf(event.p.c, "Content-Type: image/jpeg\r\n");

  std::string header_conlength =
      "Content-Length: " + std::to_string(size) + "\r\n\r\n";
  mg_printf(event.p.c, header_conlength.c_str());
  mg_send(event.p.c, RestServerWms::jpegdatabuffer, size);
  event.p.c->is_resp = 0;

  INFO_LOG << "WMS replied to hit:" << RestServerWms::m_hitcount
           << " size:" << size;
}
