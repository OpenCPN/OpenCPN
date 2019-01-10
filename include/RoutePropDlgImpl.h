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
#include "Route.h"
#include <wx/dvrenderers.h>


class RoutePropDlgImpl : public RoutePropDlg
{
public:
    ~RoutePropDlgImpl() {};
    
    static RoutePropDlgImpl* getInstance( wxWindow* parent );
    void SetRouteAndUpdate( Route *pR, bool only_points = FALSE );
    void UpdatePoints();
    
protected:
    void RoutePropDlgOnClose( wxCloseEvent& event ) { Hide(); event.Veto(); }
    void RoutePropDlgOnSize( wxSizeEvent& event ) { event.Skip(); }
    void RoutePropDlgOnNotebookPageChanged( wxNotebookEvent& event ) { event.Skip(); }
    void PlanSpeedOnKillFocus( wxFocusEvent& event );
    void PlanSpeedOnTextEnter( wxCommandEvent& event );
    void DepartureDateOnDateChanged( wxDateEvent& event );
    void DepartureTimeOnTimeChanged( wxDateEvent& event );
    void TimezoneOnChoice( wxCommandEvent& event ) { event.Skip(); }
    void WaypointsOnDataViewListCtrlItemContextMenu( wxDataViewEvent& event );
    void WaypointsOnDataViewListCtrlItemEditingDone( wxDataViewEvent& event );
    void WaypointsOnDataViewListCtrlItemValueChanged( wxDataViewEvent& event );
    void PrintOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
    void ExtendOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
    void SplitOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
    void BtnsOnCancelButtonClick( wxCommandEvent& event ) { event.Skip(); }
    void BtnsOnOKButtonClick( wxCommandEvent& event ) { event.Skip(); }
    
    RoutePropDlgImpl( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Route Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 710,370 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
private:
    static bool instanceFlag;
    static RoutePropDlgImpl* single;
    
    Route       *m_pRoute;
    Route       *m_pHead; // for route splitting
    Route       *m_pTail;
    RoutePoint  *m_pExtendPoint;
    Route       *m_pExtendRoute;
    RoutePoint  *m_pEnroutePoint;
    bool        m_bStartNow;
    
    double      m_planspeed;
    double      m_avgspeed;
    
    int         m_nSelected; // index of point selected in Properties dialog row
    int         m_tz_selection;
    
    wxDateTime     m_starttime; // kept as UTC
    
    wxDataViewColumn *etd_col;
};

#endif // _ROUTEPROPDLGIMPL_H_
