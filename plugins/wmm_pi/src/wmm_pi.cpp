/**************************************************************************
 *   Copyright (C) 2011-2019 by Pavel Kalian                               *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement  wmm_pi.h  -- WMM plugin
 */

#include "wmm_pi.h"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#ifndef __ANDROID__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include "qopengl.h"  // this gives us the qt runtime gles2.h
#include "GL/gl_private.h"
#include "qdebug.h"
#endif

#include "version.h"

float g_piGLMinSymbolLineWidth = 0.9;

void WMMLogMessage1(wxString s) { wxLogMessage("WMM: " + s); }
extern "C" void WMMLogMessage(const char *s) {
  WMMLogMessage1(wxString::FromAscii(s));
}

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin *create_pi(void *ppimgr) {
  return new wmm_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin *p) { delete p; }

wmm_pi *g_pi;

bool g_compact;

//---------------------------------------------------------------------------------------------------------
//
//    WMM PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"
#include <N2kMsg.h>
#include <N2kTypes.h>
#include <N2kMessages.h>

void WmmUIDialog::EnablePlotChanged(wxCommandEvent &event) {
  if (m_cbEnablePlot->GetValue()) m_wmm_pi.RecomputePlot();
  m_wmm_pi.SetShowPlot(m_cbEnablePlot->GetValue());
  RequestRefresh(m_wmm_pi.m_parent_window);
}

void WmmUIDialog::PlotSettings(wxCommandEvent &event) {
  m_wmm_pi.ShowPlotSettings();
}

void WmmPlotSettingsDialog::About(wxCommandEvent &event) {
  wxString msg0(
      _("\n\
World Magnetic Model Plotting allows users to cross reference the\
 magnetic variation values printed on many raster charts.\n\n\
Variation is the angle between true and magnetic north.\n\
Inclination or dip, is the vertical angle of the magnetic field.\n\
\t(+- 90 at the magnetic poles)\n\
Field Strength is the magnetic field in nano tesla from\n\
\t20000 to 66000\n\n\
The plotted lines are similar to a topographic map.  The \
space between them can be adjusted; more space takes \
less time to calculate.\n\n\
The Step size and Pole accuracy sliders allow a trade off \
for speed vs computation time.\n\n\
The World Magnetic Model Plugin was written by Pavel Kalian \
and extended by Sean D'Epagnier to support plotting."));

  wxMessageDialog dlg(this, msg0, _("WMM Plugin"), wxOK);

  dlg.ShowModal();
}

//---------------------------------------------------------------------------------------------------------
//
//        PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

wmm_pi::wmm_pi(void *ppimgr)
    : opencpn_plugin_118(ppimgr),
      m_bShowPlot(false),
      m_DeclinationMap(DECLINATION_PLOT, MagneticModel, TimedMagneticModel,
                       &Ellip),
      m_InclinationMap(INCLINATION_PLOT, MagneticModel, TimedMagneticModel,
                       &Ellip),
      m_FieldStrengthMap(FIELD_STRENGTH_PLOT, MagneticModel, TimedMagneticModel,
                         &Ellip),
      m_bComputingPlot(false) {
  // Create the PlugIn icons
  initialize_images();

  g_pi = this;
}

int wmm_pi::Init(void) {
  AddLocaleCatalog(PLUGIN_CATALOG_NAME);

  // Set some default private member parameters
  m_wmm_dialog_x = 0;
  m_wmm_dialog_y = 0;

  MagneticModel = NULL;
  TimedMagneticModel = NULL;

  ::wxDisplaySize(&m_display_width, &m_display_height);

  //    Get a pointer to the opencpn display canvas, to use as a parent for the
  //    POI Manager dialog
  m_parent_window = GetOCPNCanvasWindow();

  //    Get a pointer to the opencpn configuration object
  m_pconfig = GetOCPNConfigObject();

  //    And load the configuration items
  LoadConfig();

#ifdef __ANDROID__
  g_compact = true;
  m_bShowPlotOptions = false;
  m_iViewType = 1;
#endif

  m_buseable = true;

  m_LastVal = wxEmptyString;

  // pFontSmall = new wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
  // wxFONTWEIGHT_BOLD );
  pFontSmall = OCPNGetFont(_("WMM_Live_Overlay"));

  m_shareLocn = *GetpSharedDataLocation() + "plugins" +
                wxFileName::GetPathSeparator() + "wmm_pi" +
                wxFileName::GetPathSeparator() + "data" +
                wxFileName::GetPathSeparator();

  //    WMM initialization

  /* Memory allocation */
  int NumTerms, epochs = 1, nMax = 0;
  wxString cof_filename = m_shareLocn + "WMM.COF";

  if (!MAG_robustReadMagModels(
          const_cast<char *>((const char *)cof_filename.mb_str()),
          (MAGtype_MagneticModel * (*)[]) & MagneticModels[0], 1)) {
    WMMLogMessage1("initialization error");
    m_buseable = false;
  } else {
    WMMLogMessage1(wxString::Format("WMM model data loaded from file %s.",
                                    cof_filename.c_str()));
    for (int i = 0; i < epochs; i++) {
      if (MagneticModels[i]->nMax > nMax) {
        nMax = MagneticModels[i]->nMax;
      }
    }
    NumTerms = ((nMax + 1) * (nMax + 2) / 2);

    TimedMagneticModel = MAG_AllocateModelMemory(
        NumTerms); /* For storing the time modified WMM Model parameters */

    for (int i = 0; i < epochs; i++) {
      if (MagneticModels[i] == NULL || TimedMagneticModel == NULL) {
        WMMLogMessage1("initialization error MAG_Error(2)");
        m_buseable = false;
      }
    }

    MagneticModel = MagneticModels[0];

    MAG_SetDefaults(&Ellip, &Geoid); /* Set default values and constants */
    /* Check for Geographic Poles */

    /* Set EGM96 Geoid parameters */
    Geoid.GeoidHeightBuffer = GeoidHeightBuffer;
    Geoid.Geoid_Initialized = 1;
    /* Set EGM96 Geoid parameters END */
  }

  bool found_handle = false;
  for (const auto &handle : GetActiveDrivers()) {
    const auto &attributes = GetAttributes(handle);
    if (attributes.find("protocol") == attributes.end()) continue;
    WMMLogMessage1(
        wxString::Format("handle proto %s", attributes.at("protocol")));
    if (attributes.at("protocol") == "nmea2000") {
      m_handleN2k = handle;
      found_handle = true;
      break;
    }
  }
  if (!found_handle) {
    WMMLogMessage1("nmea2000 handle not found");
  }
  std::vector<int> pgn_list = {127258};
  CommDriverResult xx = RegisterTXPGNs(m_handleN2k, pgn_list);

  int ret_flag =
      (WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK |
       WANTS_CURSOR_LATLON | WANTS_TOOLBAR_CALLBACK | WANTS_NMEA_EVENTS |
       WANTS_PREFERENCES | WANTS_CONFIG | WANTS_PLUGIN_MESSAGING);

  if (m_bShowIcon) {
    //    This PlugIn needs a toolbar icon, so request its insertion
    m_leftclick_tool_id =
        InsertPlugInTool("", _img_wmm, _img_wmm, wxITEM_NORMAL, _("WMM"), "",
                         NULL, WMM_TOOL_POSITION, 0, this);

    SetIconType();  // SVGs allowed if not showing live icon

    ret_flag |= INSTALLS_TOOLBAR_TOOL;
  }

  m_pWmmDialog = NULL;
  m_oDC = NULL;

  return ret_flag;
}

bool wmm_pi::DeInit(void) {
  //    Record the dialog position
  if (NULL != m_pWmmDialog) {
    wxPoint p = m_pWmmDialog->GetPosition();
    SetWmmDialogX(p.x);
    SetWmmDialogY(p.y);

    m_pWmmDialog->Close();
    delete m_pWmmDialog;
    m_pWmmDialog = NULL;
  }
  SaveConfig();
  if (MagneticModel) {
    MAG_FreeMagneticModelMemory(MagneticModel);
  }
  if (TimedMagneticModel) {
    MAG_FreeMagneticModelMemory(TimedMagneticModel);
  }

  RemovePlugInTool(m_leftclick_tool_id);

  /*if (Geoid.GeoidHeightBuffer)
  {
      free(Geoid.GeoidHeightBuffer);
      Geoid.GeoidHeightBuffer = NULL;
  }*/

  // delete pFontSmall;

  if (m_oDC) delete m_oDC;

  return true;
}

int wmm_pi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int wmm_pi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

int wmm_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int wmm_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap *wmm_pi::GetPlugInBitmap() { return _img_wmm_pi; }

wxString wmm_pi::GetCommonName() { return _("WMM"); }

wxString wmm_pi::GetShortDescription() {
  return _("World Magnetic Model PlugIn for OpenCPN");
}

wxString wmm_pi::GetLongDescription() {
  return _(
      "World Magnetic Model PlugIn for OpenCPN\n\
Implements the NOAA World Magnetic Model\n\
More information:\n\
https://www.ngdc.noaa.gov/geomag/WMM/DoDWMM.shtml\n\
The bundled WMM2025 model is valid until late 2029.\n\
After then, if new version of the plugin will not be released\n\
in time, get a new WMM.COF from NOAA and place it to the\n\
location you can find in the OpenCPN logfile.");
}

int wmm_pi::GetToolbarToolCount(void) { return 1; }

void wmm_pi::SetColorScheme(PI_ColorScheme cs) {
  if (NULL == m_pWmmDialog) return;
  DimeWindow(m_pWmmDialog);
}

void wmm_pi::SetIconType() {
  if (m_bShowLiveIcon) {
    SetToolbarToolBitmaps(m_leftclick_tool_id, _img_wmm, _img_wmm);
    SetToolbarToolBitmapsSVG(m_leftclick_tool_id, "", "", "");
    m_LastVal.Empty();
  } else {
    wxString normalIcon = m_shareLocn + "wmm_pi.svg";
    wxString toggledIcon = m_shareLocn + "wmm_pi.svg";
    wxString rolloverIcon = m_shareLocn + "wmm_pi.svg";

    SetToolbarToolBitmapsSVG(m_leftclick_tool_id, normalIcon, rolloverIcon,
                             toggledIcon);
  }
}

void wmm_pi::RearrangeWindow() {
  if (NULL == m_pWmmDialog) return;
  if (m_iViewType == 1) {
    m_pWmmDialog->sbScursor->Hide(m_pWmmDialog->gScursor, true);
    m_pWmmDialog->sbSboat->Hide(m_pWmmDialog->gSboat, true);
  } else {
    m_pWmmDialog->sbScursor->Show(m_pWmmDialog->gScursor, true, true);
    m_pWmmDialog->sbSboat->Show(m_pWmmDialog->gSboat, true, true);
  }

  m_pWmmDialog->m_cbEnablePlot->Show(m_bShowPlotOptions);
  m_pWmmDialog->m_bPlotSettings->Show(m_bShowPlotOptions);

  if (!m_bShowAtCursor) {
    m_pWmmDialog->bSframe->Hide(m_pWmmDialog->sbScursor, true);
  } else {
    m_pWmmDialog->bSframe->Show(m_pWmmDialog->sbScursor, true, true);
    if (m_iViewType == 1)
      m_pWmmDialog->sbScursor->Hide(m_pWmmDialog->gScursor, true);
  }

  SetColorScheme(PI_ColorScheme());

  m_pWmmDialog->Fit();

#ifdef __WXMSW__
  // UGLY!!!!!!! On Windows XP the transparent window is not refreshed properly
  // in OpenGL mode at least on the Atom powered netbooks, so we have to disable
  // transparency.
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;
  bool gl = true;
  if (pConf) {
    pConf->SetPath("/Settings");
    pConf->Read("OpenGL", &gl, false);
    pConf = NULL;
  }
  if (!(gl && wxPlatformInfo::Get().GetOSMajorVersion() == 5 &&
        wxPlatformInfo::Get().GetOSMinorVersion() == 1))
#endif
    if (m_pWmmDialog->CanSetTransparent())
      m_pWmmDialog->SetTransparent(m_iOpacity);
}

void wmm_pi::OnToolbarToolCallback(int id) {
  if (!m_buseable) return;
  if (NULL == m_pWmmDialog) {
    m_pWmmDialog = new WmmUIDialog(*this, m_parent_window);
    wxFont *pFont = OCPNGetFont(_("Dialog"));
    m_pWmmDialog->SetFont(*pFont);

    m_pWmmDialog->Move(wxPoint(m_wmm_dialog_x, m_wmm_dialog_y));
  }

  RearrangeWindow();
  /*m_pWmmDialog->SetMaxSize(m_pWmmDialog->GetSize());
  m_pWmmDialog->SetMinSize(m_pWmmDialog->GetSize());*/
  m_pWmmDialog->Show(!m_pWmmDialog->IsShown());
  m_pWmmDialog->Layout();  // Some platforms need a re-Layout at this point
                           // (gtk, at least)
  if (m_pWmmDialog->IsShown())
    SendPluginMessage("WMM_WINDOW_SHOWN", wxEmptyString);
  else
    SendPluginMessage("WMM_WINDOW_HIDDEN", wxEmptyString);

  wxPoint p = m_pWmmDialog->GetPosition();
  m_pWmmDialog->Move(0, 0);  // workaround for gtk autocentre dialog behavior
  m_pWmmDialog->Move(p);

#ifdef __ANDROID__
  m_pWmmDialog->CentreOnScreen();
  m_pWmmDialog->Move(-1, 0);
#endif
}

void wmm_pi::RenderOverlayBoth(pi_ocpnDC *dc, PlugIn_ViewPort *vp) {
  if (!m_bShowPlot) return;

  m_DeclinationMap.Plot(dc, vp, wxColour(255, 0, 90, 220));
  m_InclinationMap.Plot(dc, vp, wxColour(60, 255, 30, 220));
  m_FieldStrengthMap.Plot(dc, vp, wxColour(0, 60, 255, 220));
}

bool wmm_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp) {
  if (!m_bShowPlot) return true;

  if (!m_oDC) m_oDC = new pi_ocpnDC();

  m_oDC->SetVP(vp);
  m_oDC->SetDC(&dc);

  RenderOverlayBoth(m_oDC, vp);

  return true;
}

bool wmm_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp) {
  if (!m_bShowPlot) return true;

  if (!m_oDC) {
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
    m_oDC = new pi_ocpnDC();
  }

  m_oDC->SetVP(vp);
  m_oDC->SetDC(NULL);

#ifndef USE_ANDROID_GLES2
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_ENABLE_BIT |
               GL_POLYGON_BIT | GL_HINT_BIT);

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif

  RenderOverlayBoth(m_oDC, vp);

#ifndef USE_ANDROID_GLES2
  glPopAttrib();
#endif

  return true;
}

void wmm_pi::RecomputePlot() {
  if (m_bCachedPlotOk) return;

  if (m_bComputingPlot) return;
  m_bComputingPlot = true;

  if (!m_DeclinationMap.Recompute(m_MapDate) ||
      !m_InclinationMap.Recompute(m_MapDate) ||
      !m_FieldStrengthMap.Recompute(m_MapDate)) {
    m_bShowPlot = false;
    if (m_pWmmDialog) m_pWmmDialog->m_cbEnablePlot->SetValue(false);
  } else
    m_bCachedPlotOk = true;

  m_bComputingPlot = false;
}

void wmm_pi::SetCursorLatLon(double lat, double lon) {
  if (!m_pWmmDialog) return;

  if (!m_bShowAtCursor)
    return;  // We don't want to waste CPU cycles that much...
  if (lat < -90 || lat > 90 || lon < -180 || lon > 180 ||
      NULL == m_pWmmDialog || !m_pWmmDialog->IsShown())
    return;
  if (!m_buseable) {
    m_pWmmDialog->m_tbD->SetValue(_("Error, see log."));
    return;
  }
  CoordGeodetic.lambda = lon;
  CoordGeodetic.phi = lat;
  CoordGeodetic.HeightAboveEllipsoid = 0;
  CoordGeodetic.HeightAboveGeoid = 0;
  CoordGeodetic.UseGeoid = 0;
  UserDate.Year = wxDateTime::GetCurrentYear();
  UserDate.Month =
      wxDateTime::GetCurrentMonth() + 1;  // WHY is it 0 based????????
  UserDate.Day = wxDateTime::Now().GetDay();
  char err[255];
  MAG_DateToYear(&UserDate, err);
  MAG_GeodeticToSpherical(
      Ellip, CoordGeodetic,
      &CoordSpherical); /*Convert from geodeitic to Spherical Equations: 17-18,
                           WMM Technical report*/
  MAG_TimelyModifyMagneticModel(
      UserDate, MagneticModel,
      TimedMagneticModel); /* Time adjust the coefficients, Equation 19, WMM
                              Technical report */
  MAG_Geomag(Ellip, CoordSpherical, CoordGeodetic, TimedMagneticModel,
             &GeoMagneticElements); /* Computes the geoMagnetic field elements
                                       and their time change*/
  MAG_CalculateGridVariation(CoordGeodetic, &GeoMagneticElements);
  // WMM_PrintUserData(GeoMagneticElements,CoordGeodetic, UserDate,
  // TimedMagneticModel, &Geoid);     /* Print the results */
  m_pWmmDialog->m_tcF->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.F));
  m_pWmmDialog->m_tcH->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.H));
  m_pWmmDialog->m_tcX->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.X));
  m_pWmmDialog->m_tcY->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.Y));
  m_pWmmDialog->m_tcZ->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.Z));
  m_pWmmDialog->m_tcD->SetValue(
      wxString::Format("%-5.1lf%c (%s)", GeoMagneticElements.Decl, 0x00B0,
                       AngleToText(GeoMagneticElements.Decl).c_str()));
  m_pWmmDialog->m_tcI->SetValue(
      wxString::Format("%-5.1lf%c", GeoMagneticElements.Incl, 0x00B0));

  m_cursorVariation = GeoMagneticElements;
  SendCursorVariation();
}

void wmm_pi::SetPositionFix(PlugIn_Position_Fix &pfix) {
  if (!m_buseable) {
    return;
  }
  CoordGeodetic.lambda = pfix.Lon;
  CoordGeodetic.phi = pfix.Lat;
  CoordGeodetic.HeightAboveEllipsoid = 0;
  CoordGeodetic.UseGeoid = 0;
  UserDate.Year = wxDateTime::GetCurrentYear();
  UserDate.Month =
      wxDateTime::GetCurrentMonth() + 1;  // WHY is it 0 based????????
  UserDate.Day = wxDateTime::Now().GetDay();
  char err[255];
  MAG_DateToYear(&UserDate, err);
  MAG_GeodeticToSpherical(
      Ellip, CoordGeodetic,
      &CoordSpherical); /*Convert from geodeitic to Spherical Equations: 17-18,
                           WMM Technical report*/
  MAG_TimelyModifyMagneticModel(
      UserDate, MagneticModel,
      TimedMagneticModel); /* Time adjust the coefficients, Equation 19, WMM
                              Technical report */
  MAG_Geomag(Ellip, CoordSpherical, CoordGeodetic, TimedMagneticModel,
             &GeoMagneticElements); /* Computes the geoMagnetic field elements
                                       and their time change*/
  MAG_CalculateGridVariation(CoordGeodetic, &GeoMagneticElements);
  // WMM_PrintUserData(GeoMagneticElements,CoordGeodetic, UserDate,
  // TimedMagneticModel, &Geoid);     /* Print the results */

  m_boatVariation = GeoMagneticElements;
  SendBoatVariation();

  wxString NewVal = wxString::Format("%.1f", GeoMagneticElements.Decl);
  double scale = GetOCPNGUIToolScaleFactor_PlugIn();
  scale = wxRound(scale * 4.0) / 4.0;
  scale *= OCPN_GetWinDIPScaleFactor();

  // scale =
  //   wxMax(1.0, scale);  // Let the upstream processing handle minification.

  if (m_bShowIcon && m_bShowLiveIcon &&
      ((m_LastVal != NewVal) || (scale != m_scale))) {
    m_scale = scale;
    m_LastVal = NewVal;
    int w = _img_wmm_live->GetWidth() * scale;
    int h = _img_wmm_live->GetHeight() * scale;
    wxMemoryDC dc;
    wxBitmap icon;

    //  Is SVG available?
    wxBitmap live = GetBitmapFromSVGFile(m_shareLocn + "wmm_live.svg", w, h);
    if (!live.IsOk()) {
      icon = wxBitmap(_img_wmm_live->GetWidth(), _img_wmm_live->GetHeight());
      dc.SelectObject(icon);
      dc.DrawBitmap(*_img_wmm_live, 0, 0, true);
    } else {
      icon = wxBitmap(w, h);
      dc.SelectObject(icon);
      wxColour col;
      dc.SetBackground(*wxTRANSPARENT_BRUSH);
      dc.Clear();

      dc.DrawBitmap(live, 0, 0, true);
    }

    wxColour cf;
    GetGlobalColor("CHWHT", &cf);
    dc.SetTextForeground(cf);
    if (pFontSmall->IsOk()) {
      if (live.IsOk()) {
        int point_size = wxMax(10, 10 * scale);
        pFontSmall->SetPointSize(point_size);

        //  Validate and adjust the font size...
        //   No smaller than 8 pt.
        int w;
        wxScreenDC sdc;
        sdc.GetTextExtent(NewVal, &w, NULL, NULL, NULL, pFontSmall);

        while ((w > (icon.GetWidth() * 8 / 10)) && (point_size >= 8)) {
          point_size--;
          pFontSmall->SetPointSize(point_size);
          sdc.GetTextExtent(NewVal, &w, NULL, NULL, NULL, pFontSmall);
        }
      }
      dc.SetFont(*pFontSmall);
    }
    wxSize s = dc.GetTextExtent(NewVal);
    dc.DrawText(NewVal, (icon.GetWidth() - s.GetWidth()) / 2,
                (icon.GetHeight() - s.GetHeight()) / 2);
    dc.SelectObject(wxNullBitmap);

    if (live.IsOk()) {
      //  By using a DC to modify the bitmap, we have lost the original bitmap's
      //  alpha channel Recover it by copying from the original to the target,
      //  bit by bit
      wxImage imo = live.ConvertToImage();
      wxImage im = icon.ConvertToImage();

      if (!imo.HasAlpha()) imo.InitAlpha();
      if (!im.HasAlpha()) im.InitAlpha();

      unsigned char *alive = imo.GetAlpha();
      unsigned char *target = im.GetAlpha();

      for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
          int index = (i * w) + j;
          target[index] = alive[index];
        }
      }
      icon = wxBitmap(im);
    }

    SetToolbarToolBitmaps(m_leftclick_tool_id, &icon, &icon);
  }

  if (NULL == m_pWmmDialog || !m_pWmmDialog->IsShown()) return;
  if (!m_buseable) {
    m_pWmmDialog->m_tbD->SetValue(_("Error, see log."));
    return;
  }
  m_pWmmDialog->m_tbF->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.F));
  m_pWmmDialog->m_tbH->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.H));
  m_pWmmDialog->m_tbX->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.X));
  m_pWmmDialog->m_tbY->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.Y));
  m_pWmmDialog->m_tbZ->SetValue(
      wxString::Format("%-9.1lf nT", GeoMagneticElements.Z));
  m_pWmmDialog->m_tbD->SetValue(
      wxString::Format("%-5.1lf%c (%s)", GeoMagneticElements.Decl, 0x00B0,
                       AngleToText(GeoMagneticElements.Decl).c_str()));
  m_pWmmDialog->m_tbI->SetValue(
      wxString::Format("%-5.1lf%c", GeoMagneticElements.Incl, 0x00B0));
}

// Demo implementation of response mechanism
void wmm_pi::SetPluginMessage(wxString &message_id, wxString &message_body) {
  if (message_id == "WMM_VARIATION_REQUEST") {
    wxJSONReader r;
    wxJSONValue v;
    r.Parse(message_body, &v);
    double lat = v["Lat"].AsDouble();
    double lon = v["Lon"].AsDouble();
    int year = v["Year"].AsInt();
    int month = v["Month"].AsInt();
    int day = v["Day"].AsInt();
    SendVariationAt(lat, lon, year, month, day);
  } else if (message_id == "WMM_VARIATION_BOAT_REQUEST") {
    SendBoatVariation();
  } else if (message_id == "WMM_VARIATION_CURSOR_REQUEST") {
    SendCursorVariation();
  }
}

void wmm_pi::SendVariationAt(double lat, double lon, int year, int month,
                             int day) {
  wxJSONValue v;
  v["Lat"] = lat;
  v["Lon"] = lon;
  v["Year"] = year;
  v["Month"] = month;
  v["Day"] = day;
  CoordGeodetic.lambda = lon;
  CoordGeodetic.phi = lat;
  CoordGeodetic.HeightAboveEllipsoid = 0;
  CoordGeodetic.UseGeoid = 0;
  UserDate.Year = year;
  UserDate.Month = month;
  UserDate.Day = day;
  char err[255];
  MAG_DateToYear(&UserDate, err);
  MAG_GeodeticToSpherical(
      Ellip, CoordGeodetic,
      &CoordSpherical); /*Convert from geodeitic to Spherical Equations: 17-18,
                           WMM Technical report*/
  MAG_TimelyModifyMagneticModel(
      UserDate, MagneticModel,
      TimedMagneticModel); /* Time adjust the coefficients, Equation 19, WMM
                              Technical report */
  MAG_Geomag(Ellip, CoordSpherical, CoordGeodetic, TimedMagneticModel,
             &GeoMagneticElements); /* Computes the geoMagnetic field elements
                                       and their time change*/
  MAG_CalculateGridVariation(CoordGeodetic, &GeoMagneticElements);
  v["Decl"] = GeoMagneticElements.Decl;
  v["Decldot"] = GeoMagneticElements.Decldot;
  v["F"] = GeoMagneticElements.F;
  v["Fdot"] = GeoMagneticElements.Fdot;
  v["GV"] = GeoMagneticElements.GV;
  v["GVdot"] = GeoMagneticElements.GVdot;
  v["H"] = GeoMagneticElements.H;
  v["Hdot"] = GeoMagneticElements.Hdot;
  v["Incl"] = GeoMagneticElements.Incl;
  v["Incldot"] = GeoMagneticElements.Incldot;
  v["X"] = GeoMagneticElements.X;
  v["Xdot"] = GeoMagneticElements.Xdot;
  v["Y"] = GeoMagneticElements.Y;
  v["Ydot"] = GeoMagneticElements.Ydot;
  v["Z"] = GeoMagneticElements.Z;
  v["Zdot"] = GeoMagneticElements.Zdot;
  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendPluginMessage(wxString("WMM_VARIATION"), out);
}

void wmm_pi::SendBoatVariation() {
  wxJSONValue v;
  v["Decl"] = m_boatVariation.Decl;
  v["Decldot"] = m_boatVariation.Decldot;
  v["F"] = m_boatVariation.F;
  v["Fdot"] = m_boatVariation.Fdot;
  v["GV"] = m_boatVariation.GV;
  v["GVdot"] = m_boatVariation.GVdot;
  v["H"] = m_boatVariation.H;
  v["Hdot"] = m_boatVariation.Hdot;
  v["Incl"] = m_boatVariation.Incl;
  v["Incldot"] = m_boatVariation.Incldot;
  v["X"] = m_boatVariation.X;
  v["Xdot"] = m_boatVariation.Xdot;
  v["Y"] = m_boatVariation.Y;
  v["Ydot"] = m_boatVariation.Ydot;
  v["Z"] = m_boatVariation.Z;
  v["Zdot"] = m_boatVariation.Zdot;
  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendPluginMessage(wxString("WMM_VARIATION_BOAT"), out);
  // Send boat variation as NMEA HVD for the Priority List.
  SendBoatVarHVD(m_boatVariation.Decl);
  SendPGN127258(m_boatVariation.Decl);
}

void wmm_pi::SendCursorVariation() {
  wxJSONValue v;
  v["Decl"] = m_cursorVariation.Decl;
  v["Decldot"] = m_cursorVariation.Decldot;
  v["F"] = m_cursorVariation.F;
  v["Fdot"] = m_cursorVariation.Fdot;
  v["GV"] = m_cursorVariation.GV;
  v["GVdot"] = m_cursorVariation.GVdot;
  v["H"] = m_cursorVariation.H;
  v["Hdot"] = m_cursorVariation.Hdot;
  v["Incl"] = m_cursorVariation.Incl;
  v["Incldot"] = m_cursorVariation.Incldot;
  v["X"] = m_cursorVariation.X;
  v["Xdot"] = m_cursorVariation.Xdot;
  v["Y"] = m_cursorVariation.Y;
  v["Ydot"] = m_cursorVariation.Ydot;
  v["Z"] = m_cursorVariation.Z;
  v["Zdot"] = m_cursorVariation.Zdot;
  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendPluginMessage(wxString("WMM_VARIATION_CURSOR"), out);
}

wxString wmm_pi::AngleToText(double angle) {
  int deg = (int)fabs(angle);
  int min = (fabs(angle) - deg) * 60;
  if (angle < 0)
    return wxString::Format("%u%c%u' W", deg, 0x00B0, min);
  else
    return wxString::Format("%u%c%u' E", deg, 0x00B0, min);
}

bool wmm_pi::LoadConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (pConf) {
    pConf->SetPath("/Settings/WMM");
    pConf->Read("ViewType", &m_iViewType, 1);
    pConf->Read("ShowPlotOptions", &m_bShowPlotOptions, 1);
    pConf->Read("ShowAtCursor", &m_bShowAtCursor, 1);
    pConf->Read("ShowLiveIcon", &m_bShowLiveIcon, 1);
    pConf->Read("ShowIcon", &m_bShowIcon, 1);
    pConf->Read("Opacity", &m_iOpacity, 255);

    m_wmm_dialog_x = pConf->Read("DialogPosX", 20L);
    m_wmm_dialog_y = pConf->Read("DialogPosY", 20L);

    if ((m_wmm_dialog_x < 0) || (m_wmm_dialog_x > m_display_width))
      m_wmm_dialog_x = 5;
    if ((m_wmm_dialog_y < 0) || (m_wmm_dialog_y > m_display_height))
      m_wmm_dialog_y = 5;

    pConf->SetPath("/Settings/WMM/Plot");
    pConf->Read("Declination", &m_DeclinationMap.m_bEnabled, 1);
    pConf->Read("DeclinationSpacing", &m_DeclinationMap.m_Spacing, 10);
    pConf->Read("Inclination", &m_InclinationMap.m_bEnabled, 0);
    pConf->Read("InclinationSpacing", &m_InclinationMap.m_Spacing, 10);
    pConf->Read("FieldStrength", &m_FieldStrengthMap.m_bEnabled, 0);
    pConf->Read("FieldStrengthSpacing", &m_FieldStrengthMap.m_Spacing, 10000);

    pConf->Read("StepSize", &m_MapStep, 6);
    pConf->Read("PoleAccuracy", &m_MapPoleAccuracy, 2);
    m_DeclinationMap.ConfigureAccuracy(m_MapStep, m_MapPoleAccuracy);
    m_InclinationMap.ConfigureAccuracy(m_MapStep, m_MapPoleAccuracy);
    m_FieldStrengthMap.ConfigureAccuracy(m_MapStep, m_MapPoleAccuracy);

    m_MapDate = wxDateTime::Now(); /* always reset to current date */

    m_bCachedPlotOk = false;

    pConf->SetPath("/Directories");
    wxString s = wxFileName::GetPathSeparator();
    wxString def =
        *GetpSharedDataLocation() + "plugins" + s + "wmm_pi" + s + "data" + s;
    // pConf->Read ( "WMMDataLocation", &m_wmm_dir, def);
    m_wmm_dir = def;
    return true;
  } else
    return false;
}

bool wmm_pi::SaveConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (pConf) {
    pConf->SetPath("/Settings/WMM");
    pConf->Write("ViewType", m_iViewType);
    pConf->Write("ShowPlotOptions", m_bShowPlotOptions);
    pConf->Write("ShowAtCursor", m_bShowAtCursor);
    pConf->Write("ShowLiveIcon", m_bShowLiveIcon);
    pConf->Write("ShowIcon", m_bShowIcon);
    pConf->Write("Opacity", m_iOpacity);

    pConf->Write("DialogPosX", m_wmm_dialog_x);
    pConf->Write("DialogPosY", m_wmm_dialog_y);

    pConf->SetPath("/Settings/WMM/Plot");
    pConf->Write("Declination", m_DeclinationMap.m_bEnabled);
    pConf->Write("DeclinationSpacing", m_DeclinationMap.m_Spacing);
    pConf->Write("Inclination", m_InclinationMap.m_bEnabled);
    pConf->Write("InclinationSpacing", m_InclinationMap.m_Spacing);
    pConf->Write("FieldStrength", m_FieldStrengthMap.m_bEnabled);
    pConf->Write("FieldStrengthSpacing", m_FieldStrengthMap.m_Spacing);
    pConf->Write("StepSize", m_MapStep);
    pConf->Write("PoleAccuracy", m_MapPoleAccuracy);

    pConf->SetPath("/Directories");
    pConf->Write("WMMDataLocation", m_wmm_dir);

    return true;
  } else
    return false;
}

void wmm_pi::ShowPreferencesDialog(wxWindow *parent) {
  WmmPrefsDialog *dialog =
      new WmmPrefsDialog(parent, wxID_ANY, _("WMM Preferences"),
                         wxPoint(m_wmm_dialog_x, m_wmm_dialog_y), wxDefaultSize,
                         wxDEFAULT_DIALOG_STYLE);
  dialog->Fit();

  dialog->m_rbViewType->SetSelection(m_iViewType);
  dialog->m_cbShowPlotOptions->SetValue(m_bShowPlotOptions);
  dialog->m_cbShowAtCursor->SetValue(m_bShowAtCursor);
  dialog->m_cbShowIcon->SetValue(m_bShowIcon);
  dialog->m_cbLiveIcon->SetValue(m_bShowLiveIcon);
  dialog->m_sOpacity->SetValue(m_iOpacity);

  if (dialog->ShowModal() == wxID_OK) {
    m_iViewType = dialog->m_rbViewType->GetSelection();
    m_bShowPlotOptions = dialog->m_cbShowPlotOptions->GetValue();
    m_bShowAtCursor = dialog->m_cbShowAtCursor->GetValue();
    m_bShowLiveIcon = dialog->m_cbLiveIcon->GetValue();
    m_bShowIcon = dialog->m_cbShowIcon->GetValue();
    m_iOpacity = dialog->m_sOpacity->GetValue();

    RearrangeWindow();
    SetIconType();

    SaveConfig();
  }
  delete dialog;
}

void wmm_pi::ShowPlotSettings() {
  WmmPlotSettingsDialog *dialog = new WmmPlotSettingsDialog(m_parent_window);
  wxFont *pFont = OCPNGetFont(_("Dialog"));
  dialog->SetFont(*pFont);

  dialog->Fit();

  dialog->m_cbDeclination->SetValue(m_DeclinationMap.m_bEnabled);
  dialog->m_scDeclinationSpacing->SetValue(m_DeclinationMap.m_Spacing);
  dialog->m_cbInclination->SetValue(m_InclinationMap.m_bEnabled);
  dialog->m_scInclinationSpacing->SetValue(m_InclinationMap.m_Spacing);
  dialog->m_cbFieldStrength->SetValue(m_FieldStrengthMap.m_bEnabled);
  dialog->m_scFieldStrengthSpacing->SetValue(m_FieldStrengthMap.m_Spacing);
  /// dialog->m_dpDate->SetValue(m_MapDate);
  dialog->m_sStep->SetValue(m_MapStep);
  dialog->m_sPoleAccuracy->SetValue(m_MapPoleAccuracy);

  if (dialog->ShowModal() == wxID_OK) {
    m_DeclinationMap.m_bEnabled = dialog->m_cbDeclination->GetValue();
    m_DeclinationMap.m_Spacing = dialog->m_scDeclinationSpacing->GetValue();
    m_InclinationMap.m_bEnabled = dialog->m_cbInclination->GetValue();
    m_InclinationMap.m_Spacing = dialog->m_scInclinationSpacing->GetValue();
    m_FieldStrengthMap.m_bEnabled = dialog->m_cbFieldStrength->GetValue();
    m_FieldStrengthMap.m_Spacing = dialog->m_scFieldStrengthSpacing->GetValue();
    /// m_MapDate = dialog->m_dpDate->GetValue();
    m_MapStep = dialog->m_sStep->GetValue();
    m_MapPoleAccuracy = dialog->m_sPoleAccuracy->GetValue();
    m_DeclinationMap.ConfigureAccuracy(m_MapStep, m_MapPoleAccuracy);
    m_InclinationMap.ConfigureAccuracy(m_MapStep, m_MapPoleAccuracy);
    m_FieldStrengthMap.ConfigureAccuracy(m_MapStep, m_MapPoleAccuracy);

    m_bCachedPlotOk = false;
    if (m_pWmmDialog->m_cbEnablePlot->GetValue()) RecomputePlot();

    RequestRefresh(m_parent_window);
    RearrangeWindow();

    SaveConfig();
  }
  delete dialog;
}

void wmm_pi::SendBoatVarHVD(double d_var) {
  // We use the HVD NMEA sentence to send magnetic variation.
  // The user can then select the desired variation
  // via the priority of the source code. The not official Talker: WM
  // is used to print source: WMM plugin in the Priority list.
  wxString s_dir = d_var >= 0 ? "E" : "W";
  d_var = fabs(d_var);  // Make it positive for NMEA sentence
  wxString S = "$WMHVD";
  S.Append(",");  // 1 Var degrees
  S.Append(wxString::Format("%.1f", d_var));
  S.Append(",");    // 2 Var Dir
  S.Append(s_dir);  // E/W
  S.Append("*");
  S.Append(wxString::Format("%02X", ComputeChecksum(S)));
  S += "\r\n";

  PushNMEABuffer(S);
}

void wmm_pi::SendPGN127258(double d_var) {
  // Send Magnetic Variation as PGN 127258 Magnetic Variation
  double var_rad = d_var * (M_PI / 180.0);
  // Calculate the number of days since the Unix epoch
  time_t now = time(nullptr);
  int days_since_epoch = now / (60 * 60 * 24);
  tN2kMsg msg127285;
  SetN2kPGN127258(msg127285, 0xFF, tN2kMagneticVariation::N2kmagvar_WMM2025,
                  days_since_epoch, var_rad);
  std::shared_ptr<std::vector<uint8_t>> payload(new std::vector<uint8_t>(
      msg127285.Data, msg127285.Data + msg127285.DataLen));
  WriteCommDriverN2K(m_handleN2k, 127258, 0xFF, 7, payload);
}

unsigned char wmm_pi::ComputeChecksum(wxString sentence) const {
  unsigned char calculated_checksum = 0;
  for (wxString::const_iterator i = sentence.begin() + 1;
       i != sentence.end() && *i != '*'; ++i)
    calculated_checksum ^= static_cast<unsigned char>(*i);

  return (calculated_checksum);
}
