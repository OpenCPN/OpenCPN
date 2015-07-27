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

#define     PLUGIN_VERSION_MAJOR    3
#define     PLUGIN_VERSION_MINOR    0

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    12

#include "../../../include/ocpn_plugin.h"

#include "../../../include/wx/jsonreader.h"
#include "../../../include/wx/jsonwriter.h"

#include "GribSettingsDialog.h"
#include "GribOverlayFactory.h"
#include "GribUIDialog.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define GRIB_TOOL_POSITION    -1          // Request default positioning of ToolBar tool
#define STARTING_STATE_STYLE  9999        // style option undifined
#define ATTACHED               0          // dialog are attached
#define SEPARATED              1          // dialog are separated
#define ATTACHED_HAS_CAPTION   0          // dialog attached  has a caption
#define ATTACHED_NO_CAPTION    1          // dialog attached don't have caption
#define SEPARATED_HORIZONTAL   2          // dialog separated shown honrizontaly
#define SEPARATED_VERTICAL     3          // dialog separated shown vaerticaly

class grib_pi : public opencpn_plugin_112
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
      bool MouseEventHook( wxMouseEvent &event);
      bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
      void SetCursorLatLon(double lat, double lon);
      void OnContextMenuItemCallback(int id);
      void SetPluginMessage(wxString &message_id, wxString &message_body);
      bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
      void SendTimelineMessage(wxDateTime time);
      void SetDefaults(void);
      int GetToolBarToolCount(void);
      void ShowPreferencesDialog( wxWindow* parent );
      void OnToolbarToolCallback(int id);
      bool QualifyCtrlBarPosition( wxPoint position, wxSize size );
      void MoveDialog( wxDialog *dialog, wxPoint position, wxPoint dfault );

// Other public methods
      void SetCtrlBarXY   (wxPoint p){ m_CtrlBarxy = p;}
      void SetCursorDataXY    (wxPoint p){ m_CursorDataxy = p;}
      void SetCtrlBarSizeXY(wxSize p){ m_CtrlBar_Sizexy = p;}
      void SetColorScheme(PI_ColorScheme cs);
      void SetDialogFont( wxWindow *window, wxFont *font = OCPNGetFont(_("Dialog"), 10) );

      void OnGribCtrlBarClose();

      wxPoint GetCtrlBarXY() { return m_CtrlBarxy; }
      wxPoint GetCursorDataXY() { return m_CursorDataxy; }
      int  GetTimeZone() { return m_bTimeZone; }
      int  GetStartOptions() { return m_bStartOptions; }
      bool GetCopyFirstCumRec() { return  m_bCopyFirstCumRec; }
      bool GetCopyMissWaveRec() { return  m_bCopyMissWaveRec; }

      GRIBOverlayFactory *m_pGRIBOverlayFactory;
      GRIBOverlayFactory *GetGRIBOverlayFactory(){ return m_pGRIBOverlayFactory; }

      int   m_MenuItem;
      bool  m_DialogStyleChanged;

private:
      bool LoadConfig(void);
      bool SaveConfig(void);

      wxFileConfig     *m_pconfig;
      wxWindow         *m_parent_window;

      GRIBUICtrlBar     *m_pGribCtrlBar;

      int              m_display_width, m_display_height;
      int              m_leftclick_tool_id;

      wxPoint          m_CtrlBarxy, m_CursorDataxy;
      wxSize           m_CtrlBar_Sizexy;

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
      int              m_bStartOptions;
      wxString         m_RequestConfig;
      wxString         m_bMailToAddresses;
      wxString         m_bMailFromAddress;
      wxString         m_ZyGribLogin;
      wxString         m_ZyGribCode;

      bool             m_bGRIBShowIcon;

      bool        m_bShowGrib;
      bool        m_bInitIsOK;
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

private:
    void OnStartOptionChange(wxCommandEvent& event);
};
#endif
