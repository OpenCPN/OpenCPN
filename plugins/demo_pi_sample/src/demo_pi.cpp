/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

/** \file demo_pi.h Minimal demonstration plugin. */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <wx/aui/aui.h>

#include "demo_pi.h"

/** Class factory used to create instances of the PlugIn */
extern "C" DECL_EXP opencpn_plugin* create_pi(void* ppimgr) {
  return new DemoPi(ppimgr);
}

/** Class destructor. */
extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) { delete p; }

/** Basic plugin initialization. */
int DemoPi::Init(void) {
  AddLocaleCatalog("../plugins/demo_pi_sample/src/");

  m_demo_window = nullptr;

  // Get a pointer to the opencpn display canvas, to use as a parent for windows
  // created
  m_parent_window = GetOCPNCanvasWindow();

  // Create the Context Menu Items

  //    In order to avoid an ASSERT on msw debug builds,
  //    we need to create a dummy menu to act as a surrogate parent of the
  //    created MenuItems The Items will be re-parented when added to the real
  //    context meenu
  wxMenu dummy_menu;

  wxMenuItem* pmi =
      new wxMenuItem(&dummy_menu, -1, _("Show PlugIn DemoWindow"));
  m_show_id = AddCanvasContextMenuItem(pmi, this);
  SetCanvasContextMenuItemViz(m_show_id, true);

  wxMenuItem* pmih =
      new wxMenuItem(&dummy_menu, -1, _("Hide PlugIn DemoWindow"));
  m_hide_id = AddCanvasContextMenuItem(pmih, this);
  SetCanvasContextMenuItemViz(m_hide_id, false);

  m_demo_window = std::make_unique<DemoWindow>(m_parent_window, wxID_ANY);

  m_aui_mgr = GetFrameAuiManager();
  m_aui_mgr->AddPane(m_demo_window.get());
  m_aui_mgr->GetPane(m_demo_window.get()).Name("Demo Window Name");

  m_aui_mgr->GetPane(m_demo_window.get()).Float();
  m_aui_mgr->GetPane(m_demo_window.get()).FloatingPosition(300, 30);

  m_aui_mgr->GetPane(m_demo_window.get()).Caption("AUI Managed Demo Window");
  m_aui_mgr->GetPane(m_demo_window.get()).CaptionVisible(true);
  m_aui_mgr->GetPane(m_demo_window.get()).GripperTop(true);
  m_aui_mgr->GetPane(m_demo_window.get()).CloseButton(true);
  m_aui_mgr->GetPane(m_demo_window.get()).Show(false);
  m_aui_mgr->Update();

  return (INSTALLS_CONTEXTMENU_ITEMS | WANTS_NMEA_SENTENCES | USES_AUI_MANAGER);
}

bool DemoPi::DeInit(void) {
  m_aui_mgr->DetachPane(m_demo_window.get());
  return true;
}

int DemoPi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int DemoPi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

int DemoPi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int DemoPi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }
int GetPlugInVersionPatch() { return PLUGIN_VERSION_PATCH; }
int GetPlugInVersionPost() { return PLUGIN_VERSION_TWEAK; }
const char* GetPlugInVersionPre() { return PKG_PRERELEASE; }
const char* GetPlugInVersionBuild() { return PKG_BUILD_INFO; }

wxString DemoPi::GetCommonName() { return _("Demo"); }

wxString DemoPi::GetShortDescription() { return _("Demo PlugIn for OpenCPN"); }

wxString DemoPi::GetLongDescription() {
  return _(
      "Demo PlugIn for OpenCPN\n\
demonstrates PlugIn processing of NMEA messages.");
}

void DemoPi::OnContextMenuItemCallback(int id) {
  wxLogMessage("demo_pi OnContextMenuCallBack()");
  ::wxBell();

  //  Note carefully that this is a "reference to a wxAuiPaneInfo classs
  //  instance" Copy constructor (i.e. wxAuiPaneInfo pane =
  //  m_aui_mgr->GetPane(m_demo_window);) will not work

  wxAuiPaneInfo& pane = m_aui_mgr->GetPane(m_demo_window.get());
  if (!pane.IsOk()) return;

  if (!pane.IsShown()) {
    SetCanvasContextMenuItemViz(m_hide_id, true);
    SetCanvasContextMenuItemViz(m_show_id, false);
    pane.Show(true);
    m_aui_mgr->Update();
  } else {
    SetCanvasContextMenuItemViz(m_hide_id, false);
    SetCanvasContextMenuItemViz(m_show_id, true);

    pane.Show(false);
    m_aui_mgr->Update();
  }
}

void DemoPi::UpdateAuiStatus(void) {
  //    This method is called after the PlugIn is initialized
  //    and the frame has done its initial layout, possibly from a saved
  //    wxAuiManager "Perspective" It is a chance for the PlugIn to syncronize
  //    itself internally with the state of any Panes that were added to the
  //    frame in the PlugIn ctor.

  //    We use this callback here to keep the context menu selection in sync
  //    with the window state

  wxAuiPaneInfo& pane = m_aui_mgr->GetPane(m_demo_window.get());
  if (!pane.IsOk()) return;

  printf("update %d\n", pane.IsShown());

  SetCanvasContextMenuItemViz(m_hide_id, pane.IsShown());
  SetCanvasContextMenuItemViz(m_show_id, !pane.IsShown());
}

bool DemoPi::RenderOverlay(wxDC& dc, PlugIn_ViewPort* vp) {
  /*    if(m_pGribDialog && m_pGRIBOverlayFactory)
      {
            if(m_pGRIBOverlayFactory->IsReadyToRender())
            {
                  m_pGRIBOverlayFactory->RenderGribOverlay ( dc, vp );
                  return true;
            }
            else
                  return false;
      }
      else*/
  return false;
}
void DemoPi::SetCursorLatLon(double lat, double lon) {}
bool DemoPi::RenderGLOverlay(wxGLContext* pcontext, PlugIn_ViewPort* vp) {
  /*   if(m_pGribDialog && m_pGRIBOverlayFactory)
     {
           if(m_pGRIBOverlayFactory->IsReadyToRender())
           {
                 m_pGRIBOverlayFactory->RenderGLGribOverlay ( pcontext, vp );
                 return true;
           }
           else
                 return false;
     }
     else*/
  return false;
}
int DemoPi::GetToolbarToolCount(void) { return 1; }
void DemoPi::ShowPreferencesDialog(wxWindow* parent) {}
void DemoPi::OnToolbarToolCallback(int id) {}
void DemoPi::SetPluginMessage(wxString& message_id, wxString& message_body) {}
void DemoPi::SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) {}

//----------------------------------------------------------------
//
//    demo Window Implementation
//
//----------------------------------------------------------------

DemoWindow::DemoWindow(wxWindow* parent, wxWindowID id)
    : wxWindow(parent, id, wxPoint(10, 10), wxSize(200, 200), wxSIMPLE_BORDER,
               "OpenCPN PlugIn"),
      m_lat(0.0),
      m_lon(1.0),
      m_sog(2.0),
      m_cog(3.0),
      m_var(4.0),
      m_utc("No GGA data") {
  Bind(wxEVT_PAINT, [&](wxPaintEvent& ev) { OnPaint(ev); });
  Bind(wxEVT_SIZE, [&](wxSizeEvent& ev) { OnSize(ev); });

  wxDEFINE_EVENT(EVT_DEMO_NAVDATA, ObservedEvt);
  m_navdata_listener = GetListener(NavDataId(), EVT_DEMO_NAVDATA, this);
  Bind(EVT_DEMO_NAVDATA, [&](ObservedEvt ev) { SetNavdata(ev); });

  wxDEFINE_EVENT(EVT_DEMO_GGA, ObservedEvt);
  m_gga_listener = GetListener(NMEA0183Id("GGA"), EVT_DEMO_GGA, this);
  Bind(EVT_DEMO_GGA, [&](ObservedEvt ev) { HandleGga(ev); });
}

void DemoWindow::OnSize(wxSizeEvent&) { printf("demoWindow OnSize()\n"); }

void DemoWindow::SetNavdata(ObservedEvt ev) {
  const PluginNavdata nav_data = GetEventNavdata(ev);
  m_lat = nav_data.lat;
  m_lon = nav_data.lon;
  m_cog = nav_data.cog;
  m_sog = nav_data.sog;
  m_var = nav_data.var;
  Refresh(false);
}

void DemoWindow::HandleGga(ObservedEvt ev) {
  auto payload = GetN0183Payload(NMEA0183Id("GGA"), ev);
  wxString payload_clone(payload);  // Missing const in << operator def
  m_utc = "parse error";
  m_nmea0183 << payload_clone;
  if (!m_nmea0183.PreParse()) return;
  if (!m_nmea0183.Parse()) return;
  if (m_nmea0183.LastSentenceIDReceived != "GGA") return;
  m_utc = m_nmea0183.Gga.UTCTime;
  Refresh(false);
}

void DemoWindow::OnPaint(wxPaintEvent&) {
  wxLogMessage("demo_pi onpaint");

  wxPaintDC dc(this);

  //      printf("onpaint\n");

  {
    dc.Clear();

    wxString data;
    data.Printf("Lat: %g ", m_lat);
    dc.DrawText(data, 10, 10);

    data.Printf("Lon: %g", m_lon);
    dc.DrawText(data, 10, 40);

    data.Printf("Sog: %g", m_sog);
    dc.DrawText(data, 10, 70);

    data.Printf("Cog: %g", m_cog);
    dc.DrawText(data, 10, 100);

    data = "Utc: ";
    dc.DrawText(data + m_utc, 10, 130);
  }
}
