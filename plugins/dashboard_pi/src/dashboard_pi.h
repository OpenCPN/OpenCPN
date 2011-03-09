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
#define     PLUGIN_VERSION_MINOR    1

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    2

#include <wx/notebook.h>
#include <wx/fileconf.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/fontpicker.h>
//wx2.9 #include <wx/wrapsizer.h>
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
class DashboardWindowContainer;
class DashboardInstrumentContainer;

#define DASHBOARD_TOOL_POSITION -1          // Request default positioning of toolbar tool

class DashboardWindowContainer
{
      public:
            DashboardWindowContainer(DashboardWindow *dashboard_window, wxString caption, wxString orientation, int width, wxArrayInt inst) {
                  m_pDashboardWindow = dashboard_window; m_sCaption = caption; m_sOrientation = orientation; m_iInstrumentWidth = width; m_aInstrumentList = inst; m_bIsVisible = false; m_bIsDeleted = false; }

            DashboardWindow              *m_pDashboardWindow;
            bool                          m_bIsVisible; // Only used for config
            bool                          m_bIsDeleted; // Only used for config
            wxString                      m_sCaption;
            wxString                      m_sOrientation;
            int                           m_iInstrumentWidth;
            wxArrayInt                    m_aInstrumentList;
};

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
WX_DEFINE_ARRAY_PTR(DashboardWindowContainer *, wxArrayOfDashboard);
WX_DEFINE_ARRAY_PTR(DashboardInstrumentContainer *, wxArrayOfInstrument);
#else
WX_DEFINE_ARRAY(DashboardWindowContainer *, wxArrayOfDashboard);
WX_DEFINE_ARRAY(DashboardInstrumentContainer *, wxArrayOfInstrument);
#endif

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------


class dashboard_pi : public wxEvtHandler, opencpn_plugin
{
public:
      dashboard_pi(void *ppimgr);

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

//    The optional method overrides
      void SetNMEASentence(wxString &sentence);
      void SetPositionFix(PlugIn_Position_Fix &pfix);
      int GetToolbarToolCount(void);
      void OnToolbarToolCallback(int id);
      void ShowPreferencesDialog( wxWindow* parent );
      void SetColorScheme(PI_ColorScheme cs);
      void OnPaneClose( wxAuiManagerEvent& event );
      void UpdateAuiStatus(void);

private:
      bool LoadConfig(void);
      bool SaveConfig(void);
      void ApplyConfig(void);
      void SendSentenceToAllInstruments(int st, double value, wxString unit);
      void SendSatInfoToAllInstruments(int cnt, int seq, SAT_INFO sats[4]);
      int GetDashboardWindowShownCount();

      wxFileConfig     *m_pconfig;
      wxAuiManager     *m_pauimgr;
      int              m_toolbar_item_id;

      wxArrayOfDashboard       m_ArrayOfDashboardWindow;
      int               m_show_id;
      int               m_hide_id;

      NMEA0183             m_NMEA0183;                 // Used to parse NMEA Sentences
      short                mPriPosition, mPriCOGSOG, mPriHeadingM, mPriHeadingT, mPriVar, mPriDateTime, mPriWindR, mPriWindT, mPriDepth;
      double               mVar;
      // FFU
      double               mSatsInView;
      double               mHdm;
      wxDateTime           mUTCDateTime;

//protected:
//      DECLARE_EVENT_TABLE();
};

class DashboardPreferencesDialog : public wxDialog
{
public:
      DashboardPreferencesDialog( wxWindow *pparent, wxWindowID id, wxArrayOfDashboard config );
      ~DashboardPreferencesDialog() {}

      void OnCloseDialog(wxCloseEvent& event);
      void OnDashboardSelected(wxListEvent& event);
      void OnDashboardAdd(wxCommandEvent& event);
      void OnDashboardDelete(wxCommandEvent& event);
      void OnInstrumentSelected(wxListEvent& event);
      void OnInstrumentAdd(wxCommandEvent& event);
      void OnInstrumentEdit(wxCommandEvent& event);
      void OnInstrumentDelete(wxCommandEvent& event);
      void OnInstrumentUp(wxCommandEvent& event);
      void OnInstrumentDown(wxCommandEvent& event);
      void SaveDashboardConfig();

      wxArrayOfDashboard            m_Config;
      wxFontPickerCtrl             *m_pFontPickerTitle;
      wxFontPickerCtrl             *m_pFontPickerData;
      wxFontPickerCtrl             *m_pFontPickerLabel;
      wxFontPickerCtrl             *m_pFontPickerSmall;

private:
      void UpdateDashboardButtonsState(void);
      void UpdateButtonsState(void);
      int                           curSel;
      wxListCtrl                   *m_pListCtrlDashboards;
      wxBitmapButton               *m_pButtonAddDashboard;
      wxBitmapButton               *m_pButtonDeleteDashboard;
      wxPanel                      *m_pPanelDashboard;
      wxTextCtrl                   *m_pTextCtrlCaption;
      wxCheckBox                   *m_pCheckBoxIsVisible;
      wxChoice                     *m_pChoiceOrientation;
      wxSpinCtrl                   *m_pInstrumentWidth;
      wxListCtrl                   *m_pListCtrlInstruments;
      wxButton                     *m_pButtonAdd;
      wxButton                     *m_pButtonEdit;
      wxButton                     *m_pButtonDelete;
      wxButton                     *m_pButtonUp;
      wxButton                     *m_pButtonDown;
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
      DashboardWindow(wxWindow *pparent, wxWindowID id, wxAuiManager *auimgr);
      ~DashboardWindow(){}

      void SetColorScheme(PI_ColorScheme cs);
      void SetSizerOrientation( int orient );
      void OnSize(wxSizeEvent& evt);
      void SetInstrumentList(wxArrayInt list);
      void SetInstrumentWidth(int width);
      void SendSentenceToAllInstruments(int st, double value, wxString unit);
      void SendSatInfoToAllInstruments(int cnt, int seq, SAT_INFO sats[4]);
      //const wxSize DoGetBestSize();
      /*TODO: OnKeyPress pass event to main window or disable focus*/

private:
      wxAuiManager         *m_pauimgr;

//wx2.9      wxWrapSizer*          itemBoxSizer;
      wxBoxSizer*          itemBoxSizer;
      wxArrayOfInstrument  m_ArrayOfInstrument;
};

#endif

