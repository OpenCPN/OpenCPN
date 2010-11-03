/******************************************************************************
 * $Id: dashboard_pi.h, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _DASHBOARDPI_H_
#define _DASHBOARDPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    1
#define     PLUGIN_VERSION_MINOR    0

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    1

#include <wx/notebook.h>
#include <wx/fileconf.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/aui/aui.h>
#include "../../../include/ocpn_plugin.h"

#include "nmea0183/nmea0183.h"
#include "instrument.h"
#include "speedometer.h"
#include "compass.h"
#include "wind.h"
#include "rudder_angle.h"
#include "gps.h"
#include "depth.h"

class DashboardWindow;
class DashboardInstrumentContainer;

#define DASHBOARD_TOOL_POSITION 14

class DashboardInstrumentContainer
{
      public:
            DashboardInstrumentContainer(int id, DashboardInstrument *instrument, int capa){
                  m_ID = id; m_pInstrument = instrument; m_cap_flag = capa; }

            DashboardInstrument    *m_pInstrument;
            int                     m_ID;
            int                     m_cap_flag;
};

//    Dynamic arrays of pointers need explicit macros in wx261
#ifdef __WX261
WX_DEFINE_ARRAY_PTR(DashboardInstrumentContainer *, wxArrayOfInstrument);
#else
WX_DEFINE_ARRAY(DashboardInstrumentContainer *, wxArrayOfInstrument);
#endif

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------


class dashboard_pi : public wxEvtHandler, opencpn_plugin
{
public:
      dashboard_pi(void *ppimgr):opencpn_plugin(ppimgr){}

//    The required PlugIn Methods
      int Init(void);
      bool DeInit(void);

      int GetAPIVersionMajor();
      int GetAPIVersionMinor();
      int GetPlugInVersionMajor();
      int GetPlugInVersionMinor();

      wxString GetCommonName();
      wxString GetShortDescription();
      wxString GetLongDescription();

//    The optional method overrides

      void SetNMEASentence(wxString &sentence);
      void SetPositionFix(PlugIn_Position_Fix &pfix);
      int GetToolbarToolCount(void);
      void OnToolbarToolCallback(int id);
      int GetToolboxPanelCount(void);
      void SetupToolboxPanel(int page_sel, wxNotebook* pnotebook);
      void OnCloseToolboxPanel(int page_sel, int ok_apply_cancel);
      void SetColorScheme(PI_ColorScheme cs);

      void OnInstrumentSelected(wxListEvent& event);
      void OnInstrumentAdd(wxCommandEvent& event);
      void OnInstrumentEdit(wxCommandEvent& event);
      void OnInstrumentDelete(wxCommandEvent& event);
      void OnInstrumentUp(wxCommandEvent& event);
      void OnInstrumentDown(wxCommandEvent& event);
      void UpdateAuiStatus(void);

private:
      bool LoadConfig(void);
      bool SaveConfig(void);
      void ApplyConfig(void);
      void UpdateButtonsState(void);

      wxFileConfig     *m_pconfig;
      wxAuiManager     *m_pauimgr;
      int              m_toolbar_item_id;
      bool             m_btoolbox_panel_is_setup;

      DashboardWindow       *m_pdashboard_window;
      int               m_show_id;
      int               m_hide_id;

      NMEA0183             m_NMEA0183;                 // Used to parse NMEA Sentences
      short                mPriPosition, mPriCOGSOG, mPriHeading, mPriVar, mPriDateTime, mPriApWind, mPriDepth;
      double               mVar;
      // FFU
      double               mSatsInView;
      double               mHdm;
      wxDateTime           mUTCDateTime;

      // Config dialog items
      wxListCtrl*                   m_pListCtrlInstruments;
      wxTextCtrl*                   m_pInstrumentWidth;
      int                           m_iInstrumentWidth;
      int                           m_iInstrumentCount; // Warning: should be size_t for wxArray.GetCount()
      wxArrayInt                    m_aInstrumentList;
      wxButton*                     m_pButtonAdd;
      wxButton*                     m_pButtonEdit;
      wxButton*                     m_pButtonDelete;
      wxButton*                     m_pButtonUp;
      wxButton*                     m_pButtonDown;

//protected:
//      DECLARE_EVENT_TABLE();
};

class AddInstrumentDlg : public wxDialog
{
public:
      AddInstrumentDlg(wxWindow *pparent, wxWindowID id);
      ~AddInstrumentDlg() {}

      unsigned int GetInstrumentAdded();

private:
      wxListCtrl*                   m_pListCtrlInstruments;
};

enum
{
      ID_DASHBOARD_WINDOW
};

class DashboardWindow : public wxWindow
{
public:
      DashboardWindow(wxWindow *pparent, wxWindowID id, wxAuiManager *auimgr, int tbitem);
      ~DashboardWindow(){}

      void SetColorScheme(PI_ColorScheme cs);
      void OnSize(wxSizeEvent& evt);
      void OnShow(wxShowEvent& event);
      void SetInstrumentList(wxArrayInt list);
      void SetInstrumentWidth(int width);
      void SendSentenceToAllInstruments(int st, double value, wxString unit);
      void SendSatInfoToAllInstruments(int cnt, int seq, SAT_INFO sats[4]);
      //const wxSize DoGetBestSize();
      /*TODO: OnKeyPress pass event to main window or disable focus*/

private:
      wxAuiManager         *m_pauimgr;
      int                  m_toolbar_item_id;

      wxBoxSizer*          itemBoxSizer;
      wxArrayOfInstrument  m_ArrayOfInstrument;
};

#endif

