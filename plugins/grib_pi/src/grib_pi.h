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

#ifndef _GRIBPI_H_
#define _GRIBPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
  #include <wx/glcanvas.h>
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    2
#define     PLUGIN_VERSION_MINOR    2

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    7

#include "../../../include/ocpn_plugin.h"

#include "../../../include/wx/jsonreader.h"
#include "../../../include/wx/jsonwriter.h"

#include "GribSettingsDialog.h"
#include "GribOverlayFactory.h"
#include "GribUIDialog.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define GRIB_TOOL_POSITION    -1          // Request default positioning of toolbar tool

class grib_pi : public opencpn_plugin_17
{
public:
      grib_pi(void *ppimgr);
      ~grib_pi(void);

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

//    The override PlugIn Methods
      bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
      void SetCursorLatLon(double lat, double lon);
      void SetPluginMessage(wxString &message_id, wxString &message_body);
      bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
      void SendTimelineMessage(wxDateTime time);
      void SetDefaults(void);
      int GetToolbarToolCount(void);
      void ShowPreferencesDialog( wxWindow* parent );
      void OnToolbarToolCallback(int id);

// Other public methods
      void SetGribDialogX    (int x){ m_grib_dialog_x = x;};
      void SetGribDialogY    (int x){ m_grib_dialog_y = x;}
      void SetGribDialogSizeX(int x){ m_grib_dialog_sx = x;}
      void SetGribDialogSizeY(int x){ m_grib_dialog_sy = x;}
      void SetColorScheme(PI_ColorScheme cs);

      void OnGribDialogClose();

      void SetRequestConfig ( wxString conf ){ m_RequestConfig = conf; }
      wxString GetRequestConfig(){ return m_RequestConfig; }
      wxString GetMailAdresse(){ return m_bMailAdresse; }

      int  GetTimeZone() { return m_bTimeZone; }
      bool GetCopyFirstCumRec() { return  m_bCopyFirstCumRec; }
      bool GetCopyMissWaveRec() { return  m_bCopyMissWaveRec; }
      wxString GetSaildocAdresse() { return m_bMailAdresse; }

      GRIBOverlayFactory *GetGRIBOverlayFactory(){ return m_pGRIBOverlayFactory; }

private:
      bool LoadConfig(void);
      bool SaveConfig(void);

      wxFileConfig     *m_pconfig;
      wxWindow         *m_parent_window;

      GRIBUIDialog     *m_pGribDialog;
      GRIBOverlayFactory *m_pGRIBOverlayFactory;

      int              m_display_width, m_display_height;
      int              m_leftclick_tool_id;

      int              m_grib_dialog_x, m_grib_dialog_y;
      int              m_grib_dialog_sx, m_grib_dialog_sy;

      //    Controls added to Preferences panel
      wxCheckBox              *m_pGRIBUseHiDef;
      wxCheckBox              *m_pGRIBUseGradualColors;

      GribTimelineRecordSet *m_pLastTimelineSet;

      // preference data
      bool              m_bGRIBUseHiDef;
      bool              m_bGRIBUseGradualColors;
      int              m_bTimeZone;
      bool             m_bCopyFirstCumRec;
      bool             m_bCopyMissWaveRec;
      int              m_bLoadLastOpenFile;
      wxString         m_RequestConfig;
      wxString         m_bMailAdresse;
      
      bool             m_bGRIBShowIcon;

      int              m_height;

      bool        m_bShowGrib;
};

//----------------------------------------------------------------------------------------
// Prefrence dialog definition
//----------------------------------------------------------------------------------------

class GribPreferencesDialog : public GribPreferencesDialogBase
{
public:
    GribPreferencesDialog( wxWindow *pparent)
    : GribPreferencesDialogBase(pparent) {}
    ~GribPreferencesDialog() {}
};

#endif
