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

#ifndef __SENDTOGPSDLG_H__
#define __SENDTOGPSDLG_H__

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/dialog.h>

//    Constants for SendToGps... Dialog
#define ID_STGDIALOG 10005
#define SYMBOL_STG_STYLE                                      \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX | \
      wxFRAME_FLOAT_ON_PARENT
#define SYMBOL_STG_TITLE _("Send to GPS")
#define SYMBOL_STG_IDNAME ID_STGDIALOG
#define SYMBOL_STG_SIZE wxSize(500, 500)
#define SYMBOL_STG_POSITION wxDefaultPosition

class Route;
class RoutePoint;

/**
 * Dialog for sending routes/waypoints to a GPS device. Allows users to send
 * route and waypoint data to a connected GPS device. It provides options to
 * select the GPS device and displays a progress gauge during transfer.
 */
class SendToGpsDlg : public wxDialog {
  DECLARE_EVENT_TABLE()

public:
  SendToGpsDlg();
  SendToGpsDlg(wxWindow* parent, wxWindowID id, const wxString& caption,
               const wxString& hint, const wxPoint& pos, const wxSize& size,
               long style);
  ~SendToGpsDlg();

  bool Create(wxWindow* parent, wxWindowID id = SYMBOL_STG_IDNAME,
              const wxString& caption = SYMBOL_STG_TITLE,
              const wxString& hint = SYMBOL_STG_TITLE,
              const wxPoint& pos = SYMBOL_STG_POSITION,
              const wxSize& size = SYMBOL_STG_SIZE,
              long style = SYMBOL_STG_STYLE);
  void SetRoute(Route* pRoute) { m_pRoute = pRoute; }
  void SetWaypoint(RoutePoint* pRoutePoint) { m_pRoutePoint = pRoutePoint; }
  wxGauge* GetProgressGauge() { return m_pgauge; }
  void SetMessage(wxString message);

private:
  void CreateControls(const wxString& hint);

  void OnCancelClick(wxCommandEvent& event);
  void OnSendClick(wxCommandEvent& event);

  Route* m_pRoute;
  RoutePoint* m_pRoutePoint;
  wxComboBox* m_itemCommListBox;
  wxGauge* m_pgauge;
  wxButton* m_CancelButton;
  wxButton* m_SendButton;
  wxStaticText* premtext;
};

#endif
