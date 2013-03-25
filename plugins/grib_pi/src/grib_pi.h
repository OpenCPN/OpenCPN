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

#define     PLUGIN_VERSION_MAJOR    1
#define     PLUGIN_VERSION_MINOR    4

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    7

#include "../../../include/ocpn_plugin.h"

#include "grib.h"

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
      bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);


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
      void CreateGribDialog( int index, wxString filename, bool newfile );
        
      void SetGRIBDataConfig ( wxString conf ){m_grib_DataConfig = conf; }
      void SetMailRequestConfig ( wxString conf ){m_grib_RequestConfig = conf; }
      void SetGribDirectory( wxString dir ) { m_grib_dir = dir; }

      wxString GetGribDirectory() { return m_grib_dir; }
      wxString GetGRIBDataConfig(){ return m_grib_DataConfig; }
      wxString GetMailRequestConfig(){ return m_grib_RequestConfig; }
      int  GetSpeedUnit() { return m_bSpeedUnit; }
      int  GetPressUnit() { return m_bPressUnit; }
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

      bool             m_bShowGrib;

      int              m_grib_dialog_x, m_grib_dialog_y;
      int              m_grib_dialog_sx, m_grib_dialog_sy;
      wxString         m_grib_dir;

      // preference data
      int              m_bTimeZone;
      int              m_bSpeedUnit;
      int              m_bPressUnit;
      bool             m_bGRIBUseHiDef;
      bool             m_bCopyFirstCumRec;
      bool             m_bCopyMissWaveRec;

      wxString         m_grib_DataConfig;
      wxString         m_grib_RequestConfig;

      wxString         m_bMailAdresse;
      
      bool             m_bGRIBShowIcon;

      int              m_height;
};

//----------------------------------------------------------------------------------------
// Prefrence dialog definition
//----------------------------------------------------------------------------------------

class GribPreferencesDialog : public wxDialog
{
public:
      GribPreferencesDialog( wxWindow *pparent, wxWindowID id, bool HiDef,
          bool CumRec, bool WaveRec, int SpeedUnit, int PressUnit, int TimeFormat, wxString MailAdresse );
      ~GribPreferencesDialog() {}
      
      bool GetGRIBUseHiDef() { return  m_pGRIBUseHiDef->GetValue();}
      bool GetCopyFirstCumRec() { return  m_pCopyFirstCumRec->GetValue();}
      bool GetCopyMissWaveRec() { return  m_pCopyMissWaveRec->GetValue();}
      int  GetSpeedUnit() { return  m_pPressUnit->GetSelection();}
      int  GetPressUnit() { return  m_pPressUnit->GetSelection();}
      int  GetTimeZone() { return  m_pTimeZone->GetSelection();}

private:
      void OnDirectoryChange( wxCommandEvent& event );
      wxRadioBox              *m_pTimeZone;
      wxRadioBox              *m_pSpeedUnit;
      wxRadioBox              *m_pPressUnit;
      wxCheckBox              *m_pGRIBUseHiDef;
      wxCheckBox              *m_pCopyFirstCumRec;
      wxCheckBox              *m_pCopyMissWaveRec;
};

#endif



