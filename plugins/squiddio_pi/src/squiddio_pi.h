/*****************************************************************************
 *
 * Project:  OpenCPN
 * Purpose   Squiddio plugin
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Mauro Calvi   *
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
 ***************************************************************************
 */

#ifndef _SQUIDDIOPI_H_
#define _SQUIDDIOPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    13

#include "version.h"
#include "squiddio_pi_thread.h"
#include <wx/list.h>
#include <wx/sstream.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/fileconf.h>
#include <wx/listimpl.cpp>
#include <wx/aui/aui.h>
#include <wx/aui/framemanager.h>

#include "ocpn_plugin.h"
#include "icons.h"
#include "Layer.h"
#include "Hyperlink.h"
#include "PoiMan.h"
#include "Poi.h"
#include "NavObjectCollection.h"
#include "squiddioPrefsDialogBase.h"
#include "nmea0183/nmea0183.h"
#include "logs.h"

class logsWindow;
class Layer;
class Poi;
class PoiMan;
class NavObjectCollection1;
class wxFileConfig;

extern PoiMan *pPoiMan;

int period_secs(int period);

#define SQUIDDIO_TOOL_POSITION    -1  // Request default positioning of toolbar tool

class SquiddioPrefsDialog;

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

typedef void (wxEvtHandler::*myEventFunction)(SquiddioEvent&);

class squiddio_pi : public opencpn_plugin_113, public wxEvtHandler
{
public:
      squiddio_pi(void *ppimgr);
      ~squiddio_pi(void);

//    The required PlugIn Methods
      int Init(void);
      bool DeInit(void);

      int GetAPIVersionMajor();
      int GetAPIVersionMinor();
      int GetPlugInVersionMajor();
      int GetPlugInVersionMinor();
      wxBitmap * GetPlugInBitmap();

      wxString GetCommonName();
      wxString GetShortDescription();
      wxString GetLongDescription();

//    The optional method overrides

      void RenderLayerContentsOnChart(Layer *layer, bool save_config = false);

      void OnContextMenuItemCallback(int id);
      void UpdateAuiStatus(void);

//    The override PlugIn Methods
      bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
      void SetCursorLatLon(double lat, double lon);
      bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
      int GetToolbarToolCount(void);
      bool LoadConfig(void);

      void ShowPreferencesDialog( wxWindow* parent );
      void OnToolbarToolCallback(int id);
      void SetPluginMessage(wxString &message_id, wxString &message_body);
      void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix);
      void appendOSDirSlash( wxString* pString );
      wxString DownloadLayer(wxString url_path);
      bool SaveLayer(wxString,wxString);
      Layer * GetLocalLayer(void);
      Layer * LoadLayer(wxString, wxString);
      bool LoadLayers(wxString &path);
      bool LoadLayerItems(wxString & path, Layer *l, bool show);
      bool ShowPOI(Poi* wp);
      bool HidePOI(Poi* wp);

      void RenderLayers();

      void UpdatePrefs(SquiddioPrefsDialog *dialog);
      
      void SetNMEASentence(wxString &sentence);
      void SetLogsWindow();
      void LateInit(void);

      wxString   layerdir;
      LayerList  *pLayerList;
      wxString   g_Email;
      wxString   g_ApiKey;
      int        g_LastLogSent;
      int        g_LastLogsRcvd;

      double    m_cursor_lat, m_cursor_lon;
      Layer     *local_sq_layer;
      int       g_PostPeriod;
      int       g_RetrievePeriod;
      bool      last_online;
      
      void OnThreadActionFinished(SquiddioEvent& event);
      void SetThreadRunning( bool state ) { m_bThreadRuning = state; }
      bool IsThreadRunning() { return m_bThreadRuning; }
      bool CheckIsOnline();
      void RefreshLayer();

private:
      SquiddioThread *m_pThread;
      wxCriticalSection m_pThreadCS; // protects the m_pThread pointer
      friend class SquiddioThread; // allow it to access our m_pThread
    
      bool SaveConfig(void);
      bool ShowType(Poi * wp);
      void PreferencesDialog(wxWindow* parent);

      wxWindow      *m_parent_window;
      int           m_show_id;
      int           m_hide_id;
      int           m_update_id;
      int           m_report_id;
      bool          isLayerUpdate;
      wxString      local_region;
      wxString      m_rgn_to_dld;
      wxString      g_VisibleLayers;
      wxString      g_InvisibleLayers;
      int           g_LayerIdx;
      bool          g_bShowLayers;

      wxAuiManager     *m_AUImgr;
      wxFileConfig     *m_pconfig;
      Plugin_Hyperlink *link;
      Hyperlink        *wp_link;
      
      long        last_online_chk;

      bool        g_ViewMarinas;
      bool        g_ViewAnchorages;
      bool        g_ViewYachtClubs;
      bool        g_ViewDocks;
      bool        g_ViewRamps;
      bool        g_ViewFuelStations;
      bool        g_ViewOthers;
      bool        g_ViewAIS;

      logsWindow  *m_plogs_window;
      int         m_squiddio_dialog_x, m_squiddio_dialog_y;
      int         m_demoshow_id;
      int         m_demohide_id;
      int         m_leftclick_tool_id;
      
      bool m_bThreadRuning;
      
      DECLARE_EVENT_TABLE()
};

class SquiddioPrefsDialog : public SquiddioPrefsDialogBase
{
public:
    SquiddioPrefsDialog( squiddio_pi &_sq_pi, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _T("Squiddio"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 250,495 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL )
        : SquiddioPrefsDialogBase( parent, id, title, pos, size, style ), m_sq_pi(_sq_pi) {}

    void OnCheckBoxAll( wxCommandEvent& event );
    void LaunchHelpPage( wxCommandEvent& event );
    void OnShareChoice( wxCommandEvent& event );
    void OnOKClick(wxCommandEvent& event);
        
protected:
    squiddio_pi &m_sq_pi;
};

#endif
