/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"

#include "pi_gl.h"

#ifdef ocpnUSE_GL
#include <wx/glcanvas.h>
#endif
#endif  // precompiled headers


#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include "grib_pi.h"

#ifdef __WXQT__
#include "qdebug.h"
#endif

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin *create_pi(void *ppimgr) {
  return new grib_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin *p) { delete p; }

extern int m_DialogStyle;

grib_pi *g_pi;
bool g_bpause;
float g_piGLMinSymbolLineWidth;

//---------------------------------------------------------------------------------------------------------
//
//    Grib PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

grib_pi::grib_pi(void *ppimgr) : opencpn_plugin_116(ppimgr) {
  // Create the PlugIn icons
  initialize_images();

  wxString shareLocn = *GetpSharedDataLocation() + _T("plugins") +
                       wxFileName::GetPathSeparator() + _T("grib_pi") +
                       wxFileName::GetPathSeparator() + _T("data") +
                       wxFileName::GetPathSeparator();
  wxImage panelIcon(shareLocn + _T("grib_panel_icon.png"));
  if (panelIcon.IsOk())
    m_panelBitmap = wxBitmap(panelIcon);
  else
    wxLogMessage(_T("    GRIB panel icon NOT loaded"));

  m_pLastTimelineSet = NULL;
  m_bShowGrib = false;
  m_GUIScaleFactor = -1.;
  g_pi = this;
}

grib_pi::~grib_pi(void) {
  delete _img_grib_pi;
  delete _img_grib;
  delete m_pLastTimelineSet;
}

int grib_pi::Init(void) {
  AddLocaleCatalog(_T("opencpn-grib_pi"));

  // Set some default private member parameters
  m_CtrlBarxy = wxPoint(0, 0);
  m_CursorDataxy = wxPoint(0, 0);

  m_pGribCtrlBar = NULL;
  m_pGRIBOverlayFactory = NULL;

  ::wxDisplaySize(&m_display_width, &m_display_height);

  m_DialogStyleChanged = false;

  //    Get a pointer to the opencpn configuration object
  m_pconfig = GetOCPNConfigObject();

  //    And load the configuration items
  LoadConfig();

  // Get a pointer to the opencpn display canvas, to use as a parent for the
  // GRIB dialog
  m_parent_window = GetOCPNCanvasWindow();

  //      int m_height = GetChartbarHeight();
  //    This PlugIn needs a CtrlBar icon, so request its insertion if enabled
  //    locally
  if (m_bGRIBShowIcon) {
    wxString shareLocn = *GetpSharedDataLocation() + _T("plugins") +
                         wxFileName::GetPathSeparator() + _T("grib_pi") +
                         wxFileName::GetPathSeparator() + _T("data") +
                         wxFileName::GetPathSeparator();

    wxString normalIcon = shareLocn + _T("grib.svg");
    wxString toggledIcon = shareLocn + _T("grib_toggled.svg");
    wxString rolloverIcon = shareLocn + _T("grib_rollover.svg");

    //  For journeyman styles, we prefer the built-in raster icons which match
    //  the rest of the toolbar.
    if (GetActiveStyleName().Lower() != _T("traditional")) {
      normalIcon = _T("");
      toggledIcon = _T("");
      rolloverIcon = _T("");
    }

    wxLogMessage(normalIcon);
    m_leftclick_tool_id = InsertPlugInToolSVG(
        _T(""), normalIcon, rolloverIcon, toggledIcon, wxITEM_CHECK, _("Grib"),
        _T(""), NULL, GRIB_TOOL_POSITION, 0, this);
  }

  if (!QualifyCtrlBarPosition(m_CtrlBarxy, m_CtrlBar_Sizexy)) {
    m_CtrlBarxy = wxPoint(20, 60);  // reset to the default position
    m_CursorDataxy = wxPoint(20, 170);
  }

#ifdef ocpnUSE_GL
  //  Set the minimum line width
  GLint parms[2];
#ifndef USE_ANDROID_GLES2
  glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0]);
#else
  glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
#endif
  g_piGLMinSymbolLineWidth = wxMax(parms[0], 1);
#endif

  return (WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK |
          WANTS_CURSOR_LATLON | WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL |
          WANTS_CONFIG | WANTS_PREFERENCES | WANTS_PLUGIN_MESSAGING |
          WANTS_ONPAINT_VIEWPORT | WANTS_MOUSE_EVENTS);
}

bool grib_pi::DeInit(void) {
  if (m_pGribCtrlBar) {
    m_pGribCtrlBar->Close();
    delete m_pGribCtrlBar;
    m_pGribCtrlBar = NULL;
  }

  delete m_pGRIBOverlayFactory;
  m_pGRIBOverlayFactory = NULL;

  return true;
}

int grib_pi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int grib_pi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

int grib_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int grib_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap *grib_pi::GetPlugInBitmap() { return &m_panelBitmap; }

wxString grib_pi::GetCommonName() { return _T("GRIB"); }

wxString grib_pi::GetShortDescription() { return _("GRIB PlugIn for OpenCPN"); }

wxString grib_pi::GetLongDescription() {
  return _(
      "GRIB PlugIn for OpenCPN\n\
Provides basic GRIB file overlay capabilities for several GRIB file types\n\
and a request function to get GRIB files by eMail.\n\n\
Supported GRIB data include:\n\
- wind direction and speed (at 10 m)\n\
- wind gust\n\
- surface pressure\n\
- rainfall\n\
- cloud cover\n\
- significant wave height and direction\n\
- air surface temperature (at 2 m)\n\
- sea surface temperature\n\
- surface current direction and speed\n\
- Convective Available Potential Energy (CAPE)\n\
- wind, altitude, temperature and relative humidity at 300, 500, 700, 850 hPa.");
}

void grib_pi::SetDefaults(void) {}

int grib_pi::GetToolBarToolCount(void) { return 1; }

bool grib_pi::MouseEventHook(wxMouseEvent &event) {
  if ((m_pGribCtrlBar && m_pGribCtrlBar->pReq_Dialog))
    return m_pGribCtrlBar->pReq_Dialog->MouseEventHook(event);
  return false;
}

void grib_pi::ShowPreferencesDialog(wxWindow *parent) {
  GribPreferencesDialog *Pref = new GribPreferencesDialog(parent);

  DimeWindow(Pref);     // aplly global colours scheme
  SetDialogFont(Pref);  // Apply global font

  Pref->m_cbUseHiDef->SetValue(m_bGRIBUseHiDef);
  Pref->m_cbUseGradualColors->SetValue(m_bGRIBUseGradualColors);
  Pref->m_cbDrawBarbedArrowHead->SetValue(m_bDrawBarbedArrowHead);
  Pref->m_cZoomToCenterAtInit->SetValue(m_bZoomToCenterAtInit);
  Pref->m_cbCopyFirstCumulativeRecord->SetValue(m_bCopyFirstCumRec);
  Pref->m_cbCopyMissingWaveRecord->SetValue(m_bCopyMissWaveRec);
  Pref->m_rbTimeFormat->SetSelection(m_bTimeZone);
  Pref->m_rbLoadOptions->SetSelection(m_bLoadLastOpenFile);
  Pref->m_rbStartOptions->SetSelection(m_bStartOptions);

#ifdef __OCPN__ANDROID__
  if (m_parent_window) {
    int xmax = m_parent_window->GetSize().GetWidth();
    int ymax = m_parent_window->GetParent()
                   ->GetSize()
                   .GetHeight();  // This would be the Options dialog itself
    Pref->SetSize(xmax, ymax);
    Pref->Layout();
    Pref->Move(0, 0);
  }
  Pref->Show();
#else
  Pref->ShowModal();
#endif
}

void grib_pi::UpdatePrefs(GribPreferencesDialog *Pref) {
  m_bGRIBUseHiDef = Pref->m_cbUseHiDef->GetValue();
  m_bGRIBUseGradualColors = Pref->m_cbUseGradualColors->GetValue();
  m_bLoadLastOpenFile = Pref->m_rbLoadOptions->GetSelection();
  m_bDrawBarbedArrowHead = Pref->m_cbDrawBarbedArrowHead->GetValue();
  m_bZoomToCenterAtInit = Pref->m_cZoomToCenterAtInit->GetValue();

  if (m_pGRIBOverlayFactory)
    m_pGRIBOverlayFactory->SetSettings(m_bGRIBUseHiDef, m_bGRIBUseGradualColors,
                                       m_bDrawBarbedArrowHead);

  int updatelevel = 0;

  if (m_bStartOptions != Pref->m_rbStartOptions->GetSelection()) {
    m_bStartOptions = Pref->m_rbStartOptions->GetSelection();
    updatelevel = 1;
  }

  if (m_bTimeZone != Pref->m_rbTimeFormat->GetSelection()) {
    m_bTimeZone = Pref->m_rbTimeFormat->GetSelection();
    if (m_pGRIBOverlayFactory) m_pGRIBOverlayFactory->SetTimeZone(m_bTimeZone);
    updatelevel = 2;
  }

  bool copyrec = Pref->m_cbCopyFirstCumulativeRecord->GetValue();
  bool copywave = Pref->m_cbCopyMissingWaveRecord->GetValue();
  if (m_bCopyFirstCumRec != copyrec || m_bCopyMissWaveRec != copywave) {
    m_bCopyFirstCumRec = copyrec;
    m_bCopyMissWaveRec = copywave;
    updatelevel = 3;
  }

  if (m_pGribCtrlBar) {
    switch (updatelevel) {
      case 0:
        break;
      case 3:
        // rebuild current activefile with new parameters and rebuil data list
        // with current index
        m_pGribCtrlBar->CreateActiveFileFromNames(
            m_pGribCtrlBar->m_bGRIBActiveFile->GetFileNames());
        m_pGribCtrlBar->PopulateComboDataList();
        m_pGribCtrlBar->TimelineChanged();
        break;
      case 2:
        // only rebuild  data list with current index and new timezone
        m_pGribCtrlBar->PopulateComboDataList();
        m_pGribCtrlBar->TimelineChanged();
        break;
      case 1:
        // only re-compute the best forecast
        m_pGribCtrlBar->ComputeBestForecastForNow();
        break;
    }
  }

  SaveConfig();
}

bool grib_pi::QualifyCtrlBarPosition(
    wxPoint position,
    wxSize size) {  // Make sure drag bar (title bar) or grabber always screen
  bool b_reset_pos = false;
#ifdef __WXMSW__
  //  Support MultiMonitor setups which an allow negative window positions.
  //  If the requested window does not intersect any installed monitor,
  //  then default to simple primary monitor positioning.
  RECT frame_title_rect;
  frame_title_rect.left = position.x;
  frame_title_rect.top = position.y;
  frame_title_rect.right = position.x + size.x;
  frame_title_rect.bottom = m_DialogStyle == ATTACHED_HAS_CAPTION
                                ? position.y + 30
                                : position.y + size.y;

  if (NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
    b_reset_pos = true;
#else
  wxRect window_title_rect;  // conservative estimate
  window_title_rect.x = position.x;
  window_title_rect.y = position.y;
  window_title_rect.width = size.x;
  window_title_rect.height =
      m_DialogStyle == ATTACHED_HAS_CAPTION ? 30 : size.y;

  wxRect ClientRect = wxGetClientDisplayRect();
  if (!ClientRect.Intersects(window_title_rect)) b_reset_pos = true;

#endif
  return !b_reset_pos;
}

void grib_pi::MoveDialog(wxDialog *dialog, wxPoint position) {
  //  Use the application frame to bound the control bar position.
  wxApp *app = wxTheApp;

  wxWindow *frame =
      app->GetTopWindow();  // or GetOCPNCanvasWindow()->GetParent();
  if (!frame) return;

  wxPoint p = frame->ScreenToClient(position);
  // Check and ensure there is always a "grabb" zone always visible wathever the
  // dialoue size is.
  if (p.x + dialog->GetSize().GetX() > frame->GetClientSize().GetX())
    p.x = frame->GetClientSize().GetX() - dialog->GetSize().GetX();
  if (p.y + dialog->GetSize().GetY() > frame->GetClientSize().GetY())
    p.y = frame->GetClientSize().GetY() - dialog->GetSize().GetY();

#ifdef __WXGTK__
  dialog->Move(0, 0);
#endif
  dialog->Move(frame->ClientToScreen(p));
}

void grib_pi::OnToolbarToolCallback(int id) {
  // if( !::wxIsBusy() ) ::wxBeginBusyCursor();

  bool starting = false;

  double scale_factor = GetOCPNGUIToolScaleFactor_PlugIn() * OCPN_GetWinDIPScaleFactor();
  if (scale_factor != m_GUIScaleFactor) starting = true;

  if (!m_pGribCtrlBar) {
    starting = true;
    long style = m_DialogStyle == ATTACHED_HAS_CAPTION
                     ? wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU
                     : wxBORDER_NONE | wxSYSTEM_MENU;
#ifdef __WXOSX__
    style |= wxSTAY_ON_TOP;
#endif
    m_pGribCtrlBar =
        new GRIBUICtrlBar(m_parent_window, wxID_ANY, wxEmptyString,
                          wxDefaultPosition, wxDefaultSize, style, this);
    m_pGribCtrlBar->SetScaledBitmap(scale_factor);

    wxMenu *dummy = new wxMenu(_T("Plugin"));
    wxMenuItem *table =
        new wxMenuItem(dummy, wxID_ANY, wxString(_("Weather table")),
                       wxEmptyString, wxITEM_NORMAL);
/* Menu font do not work properly for MSW (wxWidgets 3.2.1)
#ifdef __WXMSW__
    wxFont *qFont = OCPNGetFont(_("Menu"), 10);
    table->SetFont(*qFont);
#endif
*/
    m_MenuItem = AddCanvasContextMenuItem(table, this);
    SetCanvasContextMenuItemViz(m_MenuItem, false);

    // Create the drawing factory
    m_pGRIBOverlayFactory = new GRIBOverlayFactory(*m_pGribCtrlBar);
    m_pGRIBOverlayFactory->SetMessageFont();
    m_pGRIBOverlayFactory->SetTimeZone(m_bTimeZone);
    m_pGRIBOverlayFactory->SetParentSize(m_display_width, m_display_height);
    m_pGRIBOverlayFactory->SetSettings(m_bGRIBUseHiDef, m_bGRIBUseGradualColors,
                                       m_bDrawBarbedArrowHead);

    m_pGribCtrlBar->OpenFile(m_bLoadLastOpenFile == 0);
  }

  // Toggle GRIB overlay display
  m_bShowGrib = !m_bShowGrib;

  //    Toggle dialog?
  if (m_bShowGrib) {
    //A new file could have been added since grib plugin opened
    if (!starting && m_bLoadLastOpenFile == 0) {
      m_pGribCtrlBar->OpenFile(true);
      starting = true;
    }
    //the dialog font could have been changed since grib plugin opened
    if (m_pGribCtrlBar->GetFont() != *OCPNGetFont(_("Dialog"), 10))
      starting = true;
    if (starting) {
      m_pGRIBOverlayFactory->SetMessageFont();
      SetDialogFont(m_pGribCtrlBar);
      m_GUIScaleFactor = scale_factor;
      m_pGribCtrlBar->SetScaledBitmap(m_GUIScaleFactor);
      m_pGribCtrlBar->SetDialogsStyleSizePosition(true);
      m_pGribCtrlBar->Refresh();
    } else {
      MoveDialog(m_pGribCtrlBar, GetCtrlBarXY());
      if (m_DialogStyle >> 1 == SEPARATED) {
        MoveDialog(m_pGribCtrlBar->GetCDataDialog(), GetCursorDataXY());
        m_pGribCtrlBar->GetCDataDialog()->Show(m_pGribCtrlBar->m_CDataIsShown);
      }
#ifdef __OCPN__ANDROID__
      m_pGribCtrlBar->SetDialogsStyleSizePosition(true);
      m_pGribCtrlBar->Refresh();
#endif
    }
    m_pGribCtrlBar->Show();
    if (m_pGribCtrlBar->m_bGRIBActiveFile) {
      if (m_pGribCtrlBar->m_bGRIBActiveFile->IsOK()) {
        ArrayOfGribRecordSets *rsa =
            m_pGribCtrlBar->m_bGRIBActiveFile->GetRecordSetArrayPtr();
        if (rsa->GetCount() > 1) {
          SetCanvasContextMenuItemViz(m_MenuItem, true);
        }
        if (rsa->GetCount() >= 1) {  // XXX Should be only on Show
          SendTimelineMessage(m_pGribCtrlBar->TimelineTime());
        }
      }
    }
    // Toggle is handled by the CtrlBar but we must keep plugin manager b_toggle
    // updated to actual status to ensure correct status upon CtrlBar rebuild
    SetToolbarItemState(m_leftclick_tool_id, m_bShowGrib);

    // Do an automatic "zoom-to-center" on the overlay canvas if set in
    // Preferences
    if (m_pGribCtrlBar && m_bZoomToCenterAtInit) {
      m_pGribCtrlBar->DoZoomToCenter();
    }

    RequestRefresh(m_parent_window);  // refresh main window
  } else
    m_pGribCtrlBar->Close();
}

void grib_pi::OnGribCtrlBarClose() {
  m_bShowGrib = false;
  SetToolbarItemState(m_leftclick_tool_id, m_bShowGrib);

  m_pGribCtrlBar->Hide();

  SaveConfig();

  SetCanvasContextMenuItemViz(m_MenuItem, false);

  RequestRefresh(m_parent_window);  // refresh main window

  if (::wxIsBusy()) ::wxEndBusyCursor();

#ifdef __OCPN__ANDROID__
  m_DialogStyleChanged = true;  //  Force a delete of the control bar dialog
#endif

  if (m_DialogStyleChanged) {
    m_pGribCtrlBar->Destroy();
    m_pGribCtrlBar = NULL;
    m_DialogStyleChanged = false;
  }
}

bool grib_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp) { return false; }

bool grib_pi::DoRenderOverlay(wxDC &dc, PlugIn_ViewPort *vp, int canvasIndex) {
  if (!m_pGribCtrlBar || !m_pGribCtrlBar->IsShown() || !m_pGRIBOverlayFactory)
    return false;

  m_pGRIBOverlayFactory->RenderGribOverlay(dc, vp);

  if ((canvasIndex > 0) || (GetCanvasCount() == 1)) {
    m_pGribCtrlBar->SetViewPort(vp);
    if (m_pGribCtrlBar->pReq_Dialog)
      m_pGribCtrlBar->pReq_Dialog->RenderZoneOverlay(dc);
  }
  if (::wxIsBusy()) ::wxEndBusyCursor();
  return true;
}

bool grib_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp) {
  return false;
}

bool grib_pi::DoRenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp,
                                int canvasIndex) {
  if (!m_pGribCtrlBar || !m_pGribCtrlBar->IsShown() || !m_pGRIBOverlayFactory)
    return false;

  m_pGRIBOverlayFactory->RenderGLGribOverlay(pcontext, vp);

  if ((canvasIndex > 0) || (GetCanvasCount() == 1)) {
    m_pGribCtrlBar->SetViewPort(vp);
    if (m_pGribCtrlBar->pReq_Dialog)
      m_pGribCtrlBar->pReq_Dialog->RenderGlZoneOverlay();
  }

  if (::wxIsBusy()) ::wxEndBusyCursor();

#ifdef __OCPN__ANDROID__
  m_pGribCtrlBar->Raise();  // Control bar should always be visible
#endif

  return true;
}

bool grib_pi::RenderGLOverlayMultiCanvas(wxGLContext *pcontext,
                                         PlugIn_ViewPort *vp, int canvasIndex) {
  return DoRenderGLOverlay(pcontext, vp, canvasIndex);
}

bool grib_pi::RenderOverlayMultiCanvas(wxDC &dc, PlugIn_ViewPort *vp,
                                       int canvasIndex) {
  return DoRenderOverlay(dc, vp, canvasIndex);
}

void grib_pi::SetCursorLatLon(double lat, double lon) {
  if (m_pGribCtrlBar && m_pGribCtrlBar->IsShown())
    m_pGribCtrlBar->SetCursorLatLon(lat, lon);
}

void grib_pi::OnContextMenuItemCallback(int id) {
  if (!m_pGribCtrlBar->m_bGRIBActiveFile) return;
  m_pGribCtrlBar->ContextMenuItemCallback(id);
}

void grib_pi::SetDialogFont(wxWindow *dialog, wxFont *font) {
  dialog->SetFont(*font);
  wxWindowList list = dialog->GetChildren();
  wxWindowListNode *node = list.GetFirst();
  for (size_t i = 0; i < list.GetCount(); i++) {
    wxWindow *win = node->GetData();
    win->SetFont(*font);
    node = node->GetNext();
  }
  dialog->Fit();
  dialog->Refresh();
}

void grib_pi::SetPluginMessage(wxString &message_id, wxString &message_body) {
  if (message_id == _T("GRIB_VALUES_REQUEST")) {
    if (!m_pGribCtrlBar) OnToolbarToolCallback(0);

    // lat, lon, time, what
    wxJSONReader r;
    wxJSONValue v;
    r.Parse(message_body, &v);
    if (!v.HasMember(_T("Day"))) {
      // bogus or loading grib
      SendPluginMessage(wxString(_T("GRIB_VALUES")), _T(""));
      return;
    }
    wxDateTime time(v[_T("Day")].AsInt(),
                    (wxDateTime::Month)v[_T("Month")].AsInt(),
                    v[_T("Year")].AsInt(), v[_T("Hour")].AsInt(),
                    v[_T("Minute")].AsInt(), v[_T("Second")].AsInt());
    double lat = v[_T("lat")].AsDouble();
    double lon = v[_T("lon")].AsDouble();

    if (m_pGribCtrlBar) {
      if (v.HasMember(_T("WIND SPEED"))) {
        double vkn, ang;
        if (m_pGribCtrlBar->getTimeInterpolatedValues(
                vkn, ang, Idx_WIND_VX, Idx_WIND_VY, lon, lat, time) &&
            vkn != GRIB_NOTDEF) {
          v[_T("Type")] = wxT("Reply");
          v[_T("WIND SPEED")] = vkn;
          v[_T("WIND DIR")] = ang;
        } else {
          v.Remove(_T("WIND SPEED"));
          v.Remove(_T("WIND DIR"));
        }
      }
      if (v.HasMember(_T("CURRENT SPEED"))) {
        double vkn, ang;
        if (m_pGribCtrlBar->getTimeInterpolatedValues(
                vkn, ang, Idx_SEACURRENT_VX, Idx_SEACURRENT_VY, lon, lat,
                time) &&
            vkn != GRIB_NOTDEF) {
          v[_T("Type")] = wxT("Reply");
          v[_T("CURRENT SPEED")] = vkn;
          v[_T("CURRENT DIR")] = ang;
        } else {
          v.Remove(_T("CURRENT SPEED"));
          v.Remove(_T("CURRENT DIR"));
        }
      }
      if (v.HasMember(_T("GUST"))) {
        double vkn = m_pGribCtrlBar->getTimeInterpolatedValue(Idx_WIND_GUST,
                                                              lon, lat, time);
        if (vkn != GRIB_NOTDEF) {
          v[_T("Type")] = wxT("Reply");
          v[_T("GUST")] = vkn;
        } else
          v.Remove(_T("GUST"));
      }
      if (v.HasMember(_T("SWELL"))) {
        double vkn = m_pGribCtrlBar->getTimeInterpolatedValue(Idx_HTSIGW, lon,
                                                              lat, time);
        if (vkn != GRIB_NOTDEF) {
          v[_T("Type")] = wxT("Reply");
          v[_T("SWELL")] = vkn;
        } else
          v.Remove(_T("SWELL"));
      }

      wxJSONWriter w;
      wxString out;
      w.Write(v, out);
      SendPluginMessage(wxString(_T("GRIB_VALUES")), out);
    }
  } else if (message_id == _T("GRIB_VERSION_REQUEST")) {
    wxJSONValue v;
    v[_T("GribVersionMinor")] = GetAPIVersionMinor();
    v[_T("GribVersionMajor")] = GetAPIVersionMajor();

    wxJSONWriter w;
    wxString out;
    w.Write(v, out);
    SendPluginMessage(wxString(_T("GRIB_VERSION")), out);
  } else if (message_id == _T("GRIB_TIMELINE_REQUEST")) {
    // local time
    SendTimelineMessage(m_pGribCtrlBar ? m_pGribCtrlBar->TimelineTime()
                                       : wxDateTime::Now());
  } else if (message_id == _T("GRIB_TIMELINE_RECORD_REQUEST")) {
    wxJSONReader r;
    wxJSONValue v;
    r.Parse(message_body, &v);
    wxDateTime time(v[_T("Day")].AsInt(),
                    (wxDateTime::Month)v[_T("Month")].AsInt(),
                    v[_T("Year")].AsInt(), v[_T("Hour")].AsInt(),
                    v[_T("Minute")].AsInt(), v[_T("Second")].AsInt());

    if (!m_pGribCtrlBar) OnToolbarToolCallback(0);

    GribTimelineRecordSet *set =
        m_pGribCtrlBar ? m_pGribCtrlBar->GetTimeLineRecordSet(time) : NULL;

    char ptr[64];
    snprintf(ptr, sizeof ptr, "%p", set);

    v[_T("GribVersionMajor")] = PLUGIN_VERSION_MAJOR;
    v[_T("GribVersionMinor")] = PLUGIN_VERSION_MINOR;
    v[_T("TimelineSetPtr")] = wxString::From8BitData(ptr);

    wxJSONWriter w;
    wxString out;
    w.Write(v, out);
    SendPluginMessage(wxString(_T("GRIB_TIMELINE_RECORD")), out);
    delete m_pLastTimelineSet;
    m_pLastTimelineSet = set;
  }

  else if (message_id == _T("GRIB_APPLY_JSON_CONFIG")) {
    wxLogMessage(_T("Got GRIB_APPLY_JSON_CONFIG"));

    if (m_pGribCtrlBar) {
      m_pGribCtrlBar->OpenFileFromJSON(message_body);

      m_pGribCtrlBar->m_OverlaySettings.JSONToSettings(message_body);
      m_pGribCtrlBar->m_OverlaySettings.Write();
      m_pGribCtrlBar->SetDialogsStyleSizePosition(true);
    }
  }
}

bool grib_pi::LoadConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (!pConf) return false;

  pConf->SetPath(_T( "/PlugIns/GRIB" ));
  pConf->Read(_T( "LoadLastOpenFile" ), &m_bLoadLastOpenFile, 0);
  pConf->Read(_T("OpenFileOption" ), &m_bStartOptions, 1);
  pConf->Read(_T( "GRIBUseHiDef" ), &m_bGRIBUseHiDef, 0);
  pConf->Read(_T( "GRIBUseGradualColors" ), &m_bGRIBUseGradualColors, 0);
  pConf->Read(_T( "DrawBarbedArrowHead" ), &m_bDrawBarbedArrowHead, 1);
  pConf->Read(_T( "ZoomToCenterAtInit"), &m_bZoomToCenterAtInit, 1);
  pConf->Read(_T( "ShowGRIBIcon" ), &m_bGRIBShowIcon, 1);
  pConf->Read(_T( "GRIBTimeZone" ), &m_bTimeZone, 1);
  pConf->Read(_T( "CopyFirstCumulativeRecord" ), &m_bCopyFirstCumRec, 1);
  pConf->Read(_T( "CopyMissingWaveRecord" ), &m_bCopyMissWaveRec, 1);

  m_CtrlBar_Sizexy.x = pConf->Read(_T ( "GRIBCtrlBarSizeX" ), 1400L);
  m_CtrlBar_Sizexy.y = pConf->Read(_T ( "GRIBCtrlBarSizeY" ), 800L);
  m_CtrlBarxy.x = pConf->Read(_T ( "GRIBCtrlBarPosX" ), 20L);
  m_CtrlBarxy.y = pConf->Read(_T ( "GRIBCtrlBarPosY" ), 60L);
  m_CursorDataxy.x = pConf->Read(_T ( "GRIBCursorDataPosX" ), 20L);
  m_CursorDataxy.y = pConf->Read(_T ( "GRIBCursorDataPosY" ), 170L);

  pConf->Read(_T ( "GribCursorDataDisplayStyle" ), &m_DialogStyle, 0);
  if (m_DialogStyle > 3)
    m_DialogStyle = 0;  // ensure validity of the .conf value

  return true;
}

bool grib_pi::SaveConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (!pConf) return false;

  pConf->SetPath(_T( "/PlugIns/GRIB" ));

  pConf->Write(_T ( "LoadLastOpenFile" ), m_bLoadLastOpenFile);
  pConf->Write(_T ( "OpenFileOption" ), m_bStartOptions);
  pConf->Write(_T ( "ShowGRIBIcon" ), m_bGRIBShowIcon);
  pConf->Write(_T ( "GRIBUseHiDef" ), m_bGRIBUseHiDef);
  pConf->Write(_T ( "GRIBUseGradualColors" ), m_bGRIBUseGradualColors);
  pConf->Write(_T ( "GRIBTimeZone" ), m_bTimeZone);
  pConf->Write(_T ( "CopyFirstCumulativeRecord" ), m_bCopyFirstCumRec);
  pConf->Write(_T ( "CopyMissingWaveRecord" ), m_bCopyMissWaveRec);
  pConf->Write(_T ( "DrawBarbedArrowHead" ), m_bDrawBarbedArrowHead);
  pConf->Write(_T ( "ZoomToCenterAtInit"), m_bZoomToCenterAtInit);

  pConf->Write(_T ( "GRIBCtrlBarSizeX" ), m_CtrlBar_Sizexy.x);
  pConf->Write(_T ( "GRIBCtrlBarSizeY" ), m_CtrlBar_Sizexy.y);
  pConf->Write(_T ( "GRIBCtrlBarPosX" ), m_CtrlBarxy.x);
  pConf->Write(_T ( "GRIBCtrlBarPosY" ), m_CtrlBarxy.y);
  pConf->Write(_T ( "GRIBCursorDataPosX" ), m_CursorDataxy.x);
  pConf->Write(_T ( "GRIBCursorDataPosY" ), m_CursorDataxy.y);

  return true;
}

void grib_pi::SetColorScheme(PI_ColorScheme cs) {
  DimeWindow(m_pGribCtrlBar);
  if (m_pGribCtrlBar) {
    if (m_pGRIBOverlayFactory) m_pGRIBOverlayFactory->ClearCachedLabel();
    if (m_pGribCtrlBar->pReq_Dialog) m_pGribCtrlBar->pReq_Dialog->Refresh();
    m_pGribCtrlBar->Refresh();
    // m_pGribDialog->SetDataBackGroundColor();
  }
}

void grib_pi::SendTimelineMessage(wxDateTime time) {
  if (!m_pGribCtrlBar) return;

  wxJSONValue v;
  if (time.IsValid()) {
    v[_T("Day")] = time.GetDay();
    v[_T("Month")] = time.GetMonth();
    v[_T("Year")] = time.GetYear();
    v[_T("Hour")] = time.GetHour();
    v[_T("Minute")] = time.GetMinute();
    v[_T("Second")] = time.GetSecond();
  } else {
    v[_T("Day")] = -1;
    v[_T("Month")] = -1;
    v[_T("Year")] = -1;
    v[_T("Hour")] = -1;
    v[_T("Minute")] = -1;
    v[_T("Second")] = -1;
  }
  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendPluginMessage(wxString(_T("GRIB_TIMELINE")), out);
}

//----------------------------------------------------------------------------------------------------------
//          Prefrence dialog Implementation
//----------------------------------------------------------------------------------------------------------
void GribPreferencesDialog::OnStartOptionChange(wxCommandEvent &event) {
  if (m_rbStartOptions->GetSelection() == 2) {
    OCPNMessageBox_PlugIn(
        this,
        _("You have chosen to authorize interpolation.\nDon't forget that data "
          "displayed at current time will not be real but Recomputed\nThis can "
          "decrease accuracy!"),
        _("Warning!"));
  }
}

void GribPreferencesDialog::OnOKClick(wxCommandEvent &event) {
  if (g_pi) g_pi->UpdatePrefs(this);
  Close();
}
