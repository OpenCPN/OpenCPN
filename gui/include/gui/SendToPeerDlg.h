/***************************************************************************
 *
 * Project:  OpenCPN
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
 ***************************************************************************
 */

#ifndef __SENDTOPEERDLG_H__
#define __SENDTOPEERDLG_H__

#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <vector>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/gauge.h>
#include <wx/gdicmn.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/timer.h>
#include <wx/window.h>

#include "model/route.h"
#include "model/route_point.h"
#include "model/track.h"

#include "observable_evtvar.h"


//    Constants for SendToPeer... Dialog
#define ID_STPDIALOG 10006
#define SYMBOL_STP_STYLE                                      \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX | \
      wxFRAME_FLOAT_ON_PARENT
#define SYMBOL_STP_TITLE _("Send to Peer")
#define SYMBOL_STP_IDNAME ID_STPDIALOG
#define SYMBOL_STP_SIZE wxSize(500, 500)
#define SYMBOL_STP_POSITION wxDefaultPosition

enum { ID_STP_CANCEL = 10000, ID_STP_OK, ID_STP_CHOICE_PEER, ID_STP_SCAN };

/**
 * Route "Send to Peer..." Dialog Definition
 */
class SendToPeerDlg : public wxDialog {
  DECLARE_DYNAMIC_CLASS(SendToPeerDlg)
  DECLARE_EVENT_TABLE()

public:
  SendToPeerDlg();
  ~SendToPeerDlg();

  bool Create(wxWindow* parent, wxWindowID id = SYMBOL_STP_IDNAME,
              const wxString& caption = SYMBOL_STP_TITLE,
              const wxString& hint = SYMBOL_STP_TITLE,
              const wxPoint& pos = SYMBOL_STP_POSITION,
              const wxSize& size = SYMBOL_STP_SIZE,
              long style = SYMBOL_STP_STYLE);
  void SetRoute(Route* pRoute) { m_RouteList.push_back(pRoute); }
  void SetWaypoint(RoutePoint* pRoutePoint) { m_RoutePointList.push_back(pRoutePoint); }
  void SetTrack(Track* pTrack) { m_TrackList.push_back(pTrack); }
  void SetMessage(wxString message);
  void SetScanOnCreate(bool s){ m_bScanOnCreate = s;}
  void SetScanTime(int t){ m_scanTime = t * 2;}

private:
  void CreateControls([[maybe_unused]] const wxString& hint);

  void OnCancelClick(wxCommandEvent& event);
  void OnSendClick([[maybe_unused]] wxCommandEvent& event);
  void OnScanClick(wxCommandEvent& event);
  void OnTimerAutoscan(wxTimerEvent &event);
  void OnTimerScanTick(wxTimerEvent &event);
  void DoScan();
  bool EnableActivateChkbox();

  std::vector<Route*> m_RouteList;
  std::vector<RoutePoint*> m_RoutePointList;
  std::vector<Track*> m_TrackList;
  wxComboBox* m_PeerListBox;
  wxGauge* m_pgauge;
  wxButton* m_CancelButton;
  wxButton* m_SendButton;
  wxStaticText* premtext;
  wxButton* m_RescanButton;
  wxCheckBox* m_activate_chkbox;
  EventVar progress;
  ObsListener progress_listener;
  std::string m_ownipAddr;

  wxTimer m_autoScanTimer;
  wxTimer m_ScanTickTimer;
  int m_tick;
  int m_scanTime;
  bool m_bScanOnCreate;

};

#endif
