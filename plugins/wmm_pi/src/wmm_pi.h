/******************************************************************************
 * $Id: wmm_pi.h,v 1.0 2011/02/26 01:54:37 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  WMM Plugin
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Pavel Kalian   *
 *   $EMAIL$   *
 *                                                 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                         *
 *                                                 *
 *   This program is distributed in the hope that it will be useful,     *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
 *   GNU General Public License for more details.                  *
 *                                                 *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                           *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef _WMMPI_H_
#define _WMMPI_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <wx/fileconf.h>

#include "version.h"
#include "wxWTranslateCatalog.h"

#define MY_API_VERSION_MAJOR 1
#define MY_API_VERSION_MINOR 8

#include "ocpn_plugin.h"
#include "pi_ocpndc.h"

#include "GeomagnetismHeader.h"
#include "EGM9615.h"
#include "WmmUIDialog.h"
#include "MagneticPlotMap.h"

#include "jsonreader.h"
#include "jsonwriter.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define WMM_TOOL_POSITION -1  // Request default positioning of toolbar tool
class wmm_pi;
class WmmPrefsDialog;

class WmmUIDialog : public WmmUIDialogBase {
public:
  WmmUIDialog(wmm_pi &_wmm_pi, wxWindow *parent, wxWindowID id = wxID_ANY,
              const wxString &title = _T("WMM"),
              const wxPoint &pos = wxDefaultPosition,
              const wxSize &size = wxSize(250, 495),
              long style = wxCAPTION | wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR)
      : WmmUIDialogBase(parent, id, title, pos, size, style),
        m_wmm_pi(_wmm_pi) {}

  void EnablePlotChanged(wxCommandEvent &event);
  void PlotSettings(wxCommandEvent &event);

protected:
  wmm_pi &m_wmm_pi;
};

class WmmPlotSettingsDialog : public WmmPlotSettingsDialogBase {
public:
  WmmPlotSettingsDialog(wxWindow *parent, wxWindowID id = wxID_ANY,
                        const wxString &title = _("Magnetic Plot Settings"),
                        const wxPoint &pos = wxDefaultPosition,
                        const wxSize &size = wxSize(375, 180),
                        long style = wxDEFAULT_DIALOG_STYLE)
      : WmmPlotSettingsDialogBase(parent, id, title, pos, size, style) {}

  void About(wxCommandEvent &event);
  void Save(wxCommandEvent &event) { EndDialog(wxID_OK); }
  void Cancel(wxCommandEvent &event) { EndDialog(wxID_CANCEL); }
};

class wmm_pi : public opencpn_plugin_18 {
public:
  wmm_pi(void *ppimgr);

  //    The required PlugIn Methods
  int Init(void);
  bool DeInit(void);

  int GetAPIVersionMajor();
  int GetAPIVersionMinor();
  int GetPlugInVersionMajor();
  int GetPlugInVersionMinor();
  wxBitmap *GetPlugInBitmap();
  wxString GetCommonName();
  wxString GetShortDescription();
  wxString GetLongDescription();

  //    The required override PlugIn Methods
  void SetCursorLatLon(double lat, double lon);
  void SetPositionFix(PlugIn_Position_Fix &pfix);

  void RenderOverlayBoth(pi_ocpnDC *dc, PlugIn_ViewPort *vp);
  bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
  bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
  void RecomputePlot();

  int GetToolbarToolCount(void);
  void ShowPreferencesDialog(wxWindow *parent);
  void ShowPlotSettingsDialog(wxCommandEvent &event);

  void OnToolbarToolCallback(int id);

  //    Optional plugin overrides
  void SetColorScheme(PI_ColorScheme cs);
  void SetPluginMessage(wxString &message_id, wxString &message_body);

  void SetShowPlot(bool showplot) { m_bShowPlot = showplot; }

  //    Other public methods
  void SetWmmDialogX(int x) { m_wmm_dialog_x = x; };
  void SetWmmDialogY(int x) { m_wmm_dialog_y = x; }

  void OnWmmDialogClose();
  void ShowPlotSettings();

  //    WMM Declarations
  MAGtype_MagneticModel *MagneticModels[1];
  MAGtype_MagneticModel *MagneticModel, *TimedMagneticModel;
  MAGtype_Ellipsoid Ellip;
  MAGtype_CoordSpherical CoordSpherical;
  MAGtype_CoordGeodetic CoordGeodetic;
  MAGtype_Date UserDate;
  MAGtype_GeoMagneticElements GeoMagneticElements;
  MAGtype_Geoid Geoid;
  wxString filename;

  wxWindow *m_parent_window;
  WmmUIDialog *m_pWmmDialog;

  pi_ocpnDC *m_oDC;

private:
  wxFileConfig *m_pconfig;
  bool LoadConfig(void);
  bool SaveConfig(void);

  int m_wmm_dialog_x, m_wmm_dialog_y;
  int m_display_width, m_display_height;
  int m_iViewType;
  bool m_bShowPlotOptions;
  bool m_bShowAtCursor;
  bool m_bShowLiveIcon;
  bool m_bShowIcon;
  int m_iOpacity;

  wxString m_LastVal;

  int m_leftclick_tool_id;

  wxString AngleToText(double angle);

  bool m_bCachedPlotOk, m_bShowPlot;
  MagneticPlotMap m_DeclinationMap, m_InclinationMap, m_FieldStrengthMap;
  wxDateTime m_MapDate;
  int m_MapStep;
  int m_MapPoleAccuracy;

  void RearrangeWindow();
  void SetIconType();
  wxString m_wmm_dir;
  bool m_buseable, m_busegeoid;

  void SendVariationAt(double lat, double lon, int year, int month, int day);
  void SendBoatVariation();
  void SendCursorVariation();

  MAGtype_GeoMagneticElements m_cursorVariation;
  MAGtype_GeoMagneticElements m_boatVariation;

  bool m_bComputingPlot;
  wxFont *pFontSmall;
  double m_scale;
  wxString m_shareLocn;
};

int WMM_setupMagneticModel(char *data, MAGtype_MagneticModel *MagneticModel);

#endif
