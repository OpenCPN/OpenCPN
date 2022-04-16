/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
 * Author:   David Register
 *
 ***************************************************************************
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

#include "config.h"

#include <algorithm>
#include <set>
#include <typeinfo>
#include <unordered_map>


#include <wx/wx.h>
#include <wx/arrstr.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/event.h>
#include <wx/glcanvas.h>
#include <wx/notebook.h>
#include <wx/string.h>
#include <wx/window.h>

#include "ocpn_plugin.h"

extern wxWindow* gFrame;

//-----------------------------------------------------------------------------------------
//    The opencpn_plugin base class implementation
//-----------------------------------------------------------------------------------------

/**  FIXME
static wxBitmap* LoadSVG(const wxString filename, unsigned int width,
                         unsigned int height) {
  if (!gFrame) return new wxBitmap(width, height);  // We are headless.
#ifdef __OCPN__ANDROID__
  return loadAndroidSVG(filename, width, height);
#elif defined(ocpnUSE_SVG)
  wxSVGDocument svgDoc;
  if (svgDoc.Load(filename))
    return new wxBitmap(svgDoc.Render(width, height, NULL, true, true));
  else
    return new wxBitmap(width, height);
#else
  return new wxBitmap(width, height);
#endif
}


wxBitmap* opencpn_plugin::GetPlugInBitmap() {
  auto bitmap =  PluginLoader::getInstance()->GetPluginDefaultIcon();
  return const_cast<wxBitmap*>(bitmap);
}

**/


wxBitmap* opencpn_plugin::GetPlugInBitmap() {
  static wxBitmap* bitmap = 0;
  if (!bitmap) bitmap = new wxBitmap(1,1);
  return bitmap;
}

opencpn_plugin::~opencpn_plugin() {}

int opencpn_plugin::Init(void) { return 0; }

bool opencpn_plugin::DeInit(void) { return true; }

int opencpn_plugin::GetAPIVersionMajor() { return 1; }

int opencpn_plugin::GetAPIVersionMinor() { return 2; }

int opencpn_plugin::GetPlugInVersionMajor() { return 1; }

int opencpn_plugin::GetPlugInVersionMinor() { return 0; }

wxString opencpn_plugin::GetCommonName() { return "BaseClassCommonName"; }

wxString opencpn_plugin::GetShortDescription() {
  return "OpenCPN PlugIn Base Class";
}

wxString opencpn_plugin::GetLongDescription() {
  return "OpenCPN PlugIn Base Class\n\
PlugInManager created this base class";
}

void opencpn_plugin::SetPositionFix(PlugIn_Position_Fix& pfix) {}

void opencpn_plugin::SetNMEASentence(wxString& sentence) {}

void opencpn_plugin::SetAISSentence(wxString& sentence) {}

int opencpn_plugin::GetToolbarToolCount(void) { return 0; }

int opencpn_plugin::GetToolboxPanelCount(void) { return 0; }

void opencpn_plugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook) {}

void opencpn_plugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel) {}

void opencpn_plugin::ShowPreferencesDialog(wxWindow* parent) {}

void opencpn_plugin::OnToolbarToolCallback(int id) {}

void opencpn_plugin::OnContextMenuItemCallback(int id) {}

bool opencpn_plugin::RenderOverlay(wxMemoryDC* dc, PlugIn_ViewPort* vp) {
  return false;
}

void opencpn_plugin::SetCursorLatLon(double lat, double lon) {}

void opencpn_plugin::SetCurrentViewPort(PlugIn_ViewPort& vp) {}

void opencpn_plugin::SetDefaults(void) {}

void opencpn_plugin::ProcessParentResize(int x, int y) {}

void opencpn_plugin::SetColorScheme(PI_ColorScheme cs) {}

void opencpn_plugin::UpdateAuiStatus(void) {}

wxArrayString opencpn_plugin::GetDynamicChartClassNameArray() {
  wxArrayString array;
  return array;
}

//    Opencpn_Plugin_16 Implementation
opencpn_plugin_16::opencpn_plugin_16(void* pmgr) : opencpn_plugin(pmgr) {}

opencpn_plugin_16::~opencpn_plugin_16(void) {}

bool opencpn_plugin_16::RenderOverlay(wxDC& dc, PlugIn_ViewPort* vp) {
  return false;
}

void opencpn_plugin_16::SetPluginMessage(wxString& message_id,
                                         wxString& message_body) {}

//    Opencpn_Plugin_17 Implementation
opencpn_plugin_17::opencpn_plugin_17(void* pmgr) : opencpn_plugin(pmgr) {}

opencpn_plugin_17::~opencpn_plugin_17(void) {}

bool opencpn_plugin_17::RenderOverlay(wxDC& dc, PlugIn_ViewPort* vp) {
  return false;
}

bool opencpn_plugin_17::RenderGLOverlay(wxGLContext* pcontext,
                                        PlugIn_ViewPort* vp) {
  return false;
}

void opencpn_plugin_17::SetPluginMessage(wxString& message_id,
                                         wxString& message_body) {}

//    Opencpn_Plugin_18 Implementation
opencpn_plugin_18::opencpn_plugin_18(void* pmgr) : opencpn_plugin(pmgr) {}

opencpn_plugin_18::~opencpn_plugin_18(void) {}

bool opencpn_plugin_18::RenderOverlay(wxDC& dc, PlugIn_ViewPort* vp) {
  return false;
}

bool opencpn_plugin_18::RenderGLOverlay(wxGLContext* pcontext,
                                        PlugIn_ViewPort* vp) {
  return false;
}

void opencpn_plugin_18::SetPluginMessage(wxString& message_id,
                                         wxString& message_body) {}

void opencpn_plugin_18::SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) {}

//    Opencpn_Plugin_19 Implementation
opencpn_plugin_19::opencpn_plugin_19(void* pmgr) : opencpn_plugin_18(pmgr) {}

opencpn_plugin_19::~opencpn_plugin_19(void) {}

void opencpn_plugin_19::OnSetupOptions(void) {}

//    Opencpn_Plugin_110 Implementation
opencpn_plugin_110::opencpn_plugin_110(void* pmgr) : opencpn_plugin_19(pmgr) {}

opencpn_plugin_110::~opencpn_plugin_110(void) {}

void opencpn_plugin_110::LateInit(void) {}

//    Opencpn_Plugin_111 Implementation
opencpn_plugin_111::opencpn_plugin_111(void* pmgr) : opencpn_plugin_110(pmgr) {}

opencpn_plugin_111::~opencpn_plugin_111(void) {}

//    Opencpn_Plugin_112 Implementation
opencpn_plugin_112::opencpn_plugin_112(void* pmgr) : opencpn_plugin_111(pmgr) {}

opencpn_plugin_112::~opencpn_plugin_112(void) {}

bool opencpn_plugin_112::MouseEventHook(wxMouseEvent& event) { return false; }

void opencpn_plugin_112::SendVectorChartObjectInfo(
    wxString& chart, wxString& feature, wxString& objname, double lat,
    double lon, double scale, int nativescale) {}

//    Opencpn_Plugin_113 Implementation
opencpn_plugin_113::opencpn_plugin_113(void* pmgr) : opencpn_plugin_112(pmgr) {}

opencpn_plugin_113::~opencpn_plugin_113(void) {}

bool opencpn_plugin_113::KeyboardEventHook(wxKeyEvent& event) { return false; }

void opencpn_plugin_113::OnToolbarToolDownCallback(int id) {}
void opencpn_plugin_113::OnToolbarToolUpCallback(int id) {}

//    Opencpn_Plugin_114 Implementation
opencpn_plugin_114::opencpn_plugin_114(void* pmgr) : opencpn_plugin_113(pmgr) {}

opencpn_plugin_114::~opencpn_plugin_114(void) {}

//    Opencpn_Plugin_115 Implementation
opencpn_plugin_115::opencpn_plugin_115(void* pmgr) : opencpn_plugin_114(pmgr) {}

opencpn_plugin_115::~opencpn_plugin_115(void) {}

//    Opencpn_Plugin_116 Implementation
opencpn_plugin_116::opencpn_plugin_116(void* pmgr) : opencpn_plugin_115(pmgr) {}

opencpn_plugin_116::~opencpn_plugin_116(void) {}

bool opencpn_plugin_116::RenderGLOverlayMultiCanvas(wxGLContext* pcontext,
                                                    PlugIn_ViewPort* vp,
                                                    int max_canvas) {
  return false;
}

bool opencpn_plugin_116::RenderOverlayMultiCanvas(wxDC& dc, PlugIn_ViewPort* vp,
                                                  int max_canvas) {
  return false;
}

void opencpn_plugin_116::PrepareContextMenu(int canvasIndex) { return; }

//    Opencpn_Plugin_117 Implementation
opencpn_plugin_117::opencpn_plugin_117(void* pmgr) : opencpn_plugin_116(pmgr) {}

int opencpn_plugin_117::GetPlugInVersionPatch() { return 0; };

int opencpn_plugin_117::GetPlugInVersionPost() { return 0; };

const char* opencpn_plugin_117::GetPlugInVersionPre() { return ""; };

const char* opencpn_plugin_117::GetPlugInVersionBuild() { return ""; };

void opencpn_plugin_117::SetActiveLegInfo(Plugin_Active_Leg_Info& leg_info) {}
