/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 **************************************************************************/

#pragma once
#ifndef _ROUTEPROPDLGIMPL_H_
#define _ROUTEPROPDLGIMPL_H_

#include "RoutePropDlg.h"
#include "model/route.h"
#include <wx/dataview.h>
#include <wx/dvrenderers.h>
#include "model/ocpn_types.h"
#include "color_types.h"
#include <cmath>
#include "LinkPropDlg.h"

class RoutePropDlgImpl : public RoutePropDlg {
public:
  ~RoutePropDlgImpl();

  static RoutePropDlgImpl* getInstance(wxWindow* parent);
  void SetRouteAndUpdate(Route* pR, bool only_points = FALSE);
  Route* GetRoute(void) { return m_pRoute; }
  void SetEnroutePoint(RoutePoint* rp) { return; }  // TODO
  void UpdatePoints();
  void SetColorScheme(ColorScheme cs);
  void RecalculateSize(void);
  static bool getInstanceFlag();

protected:
  void RoutePropDlgOnClose(wxCloseEvent& event) {
    SaveGeometry();
    ResetChanges();
    Hide();
    event.Veto();
  }
  void RoutePropDlgOnSize(wxSizeEvent& event) { event.Skip(); }
  void RoutePropDlgOnNotebookPageChanged(wxNotebookEvent& event) {
    event.Skip();
  }
  void PlanSpeedOnKillFocus(wxFocusEvent& event);
  void PlanSpeedOnTextEnter(wxCommandEvent& event);
  void DepartureDateOnDateChanged(wxDateEvent& event);
  void DepartureTimeOnTimeChanged(wxDateEvent& event);
  void TimezoneOnChoice(wxCommandEvent& event);
  void WaypointsOnDataViewListCtrlItemContextMenu(wxDataViewEvent& event);
  void WaypointsOnDataViewListCtrlItemEditingDone(wxDataViewEvent& event);
  void WaypointsOnDataViewListCtrlItemValueChanged(wxDataViewEvent& event);
  void WaypointsOnDataViewListCtrlSelectionChanged(wxDataViewEvent& event);
  void PrintOnButtonClick(wxCommandEvent& event);
  void ExtendOnButtonClick(wxCommandEvent& event);
  void SplitOnButtonClick(wxCommandEvent& event);
  void BtnsOnCancelButtonClick(wxCommandEvent& event) {
    SaveGeometry();
    ResetChanges();
    Hide();
  }
  void BtnsOnOKButtonClick(wxCommandEvent& event);
  void OnRoutePropMenuSelected(wxCommandEvent& event);
  void OnRoutepropCopyTxtClick(wxCommandEvent& event);
  void ItemEditOnMenuSelection(wxCommandEvent& event);
  void ItemAddOnMenuSelection(wxCommandEvent& event);
  void ItemDeleteOnMenuSelection(wxCommandEvent& event);
  void AddLinkOnButtonClick(wxCommandEvent& event);
  void BtnEditOnToggleButton(wxCommandEvent& event);
  void OnHyperlinkClick(wxHyperlinkEvent& event);
  void HyperlinkContextMenu(wxMouseEvent& event);
  void m_scrolledWindowLinksOnContextMenu(wxMouseEvent& event);

  wxDateTime GetDepartureTS();
  void SaveChanges();
  void ResetChanges();

  void OnActivate(wxActivateEvent& event);

  RoutePropDlgImpl(wxWindow* parent, wxWindowID id = wxID_ANY,
                   const wxString& title = _("Route Properties"),
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxSize(550, 450),
                   long style = FRAME_WITH_LINKS_STYLE);

private:
  void SaveGeometry();
  static bool instanceFlag;
  static RoutePropDlgImpl* single;

  Route* m_pRoute;
  Route m_OrigRoute;
  Route* m_pHead;  // for route splitting
  Route* m_pTail;
  RoutePoint* m_pExtendPoint;
  Route* m_pExtendRoute;
  RoutePoint* m_pEnroutePoint;
  bool m_bStartNow;

  int m_tz_selection;

  wxDataViewColumn* etd_col;

  wxHyperlinkCtrl* m_pEditedLink;

  bool IsThisRouteExtendable();
};

#endif  // _ROUTEPROPDLGIMPL_H_
