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
  /** Returns the departure time of the route, in UTC. */
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

  /**
   * Pointer to the route being edited or displayed.
   * This is the route currently shown in the dialog. Changes are made directly
   * to this route.
   */
  Route* m_pRoute;
  /**
   * Original route state, stored when dialog opens.
   * Used to restore original route properties if changes are canceled.
   * Only selected properties are preserved (speed and departure time).
   */
  Route m_OrigRoute;
  /**
   * First segment when splitting a route.
   * Created when the Split button is clicked and the user confirms. Contains
   * the first portion of the original route, from start up to the selected
   * waypoint.
   */
  Route* m_pHead;
  /**
   * Second segment when splitting a route.
   * Created when the Split button is clicked and the user confirms. Contains
   * the second portion of the original route, from the selected waypoint to the
   * end.
   */
  Route* m_pTail;
  /**
   * Waypoint to start the extension from when extending a route.
   * This is typically the last point of the current route, or a point from
   * another route that is near the last point of the current route.
   */
  RoutePoint* m_pExtendPoint;
  /**
   * Route to extend from when extending the current route.
   * This route contains m_pExtendPoint and is used as the source for
   * extending the current route.
   */
  Route* m_pExtendRoute;
  /**
   * Currently selected waypoint when navigating the route.
   * This is the active waypoint when the route is being navigated.
   * Used to highlight the active leg in the route display.
   */
  RoutePoint* m_pEnroutePoint;
  /**
   * Flag indicating whether the route should start navigation immediately.
   * When true, the route will be activated for navigation as soon as the dialog
   * is closed with OK. This is typically set when a route is newly created and
   * the user chooses to start navigating it right away.
   */
  bool m_bStartNow;

  /**
   * The timezone to use for formatting the departure date/time.
   * Possible values are:
   * - 0: UTC
   * - 1: Timezone configured in operating system
   * - 2: Mean solar time at the location, based on the average time it takes
   * for the sun to cross the meridian (appear at its highest point in the sky)
   * at that specific location
   * - 3: Honor OpenCPN global setting for timezone display
   */
  int m_tz_selection;

  wxDataViewColumn* etd_col;

  wxHyperlinkCtrl* m_pEditedLink;

  bool IsThisRouteExtendable();

  wxString MakeTideInfo(wxString stationName, double lat, double lon,
                        wxDateTime utcTime);
};

#endif  // _ROUTEPROPDLGIMPL_H_
