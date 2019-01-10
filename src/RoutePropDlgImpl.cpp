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

#include "RoutePropDlgImpl.h"
#include "navutil.h"
#include "georef.h"
#include "MarkInfo.h"

#define COLUMN_PLANNED_SPEED 8
#define COLUMN_ETD 12

extern wxString GetLayerName(int id);

extern long gStart_LMT_Offset;
extern int g_StartTimeTZ;
extern wxDateTime g_StartTime;
extern double gLat;
extern double gLon;
extern MarkInfoDlg *g_pMarkInfoDialog;

RoutePropDlgImpl::RoutePropDlgImpl( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : RoutePropDlg( parent, id, title, pos, size, style)
{
    /* If time allows, implement a custom picker for date and time
     wxDataViewDateRenderer* renderer = new wxDataViewDateRenderer();
     renderer->SetMode(wxDATAVIEW_CELL_EDITABLE);
     etd_col = new wxDataViewColumn (_("ETD"), renderer, wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT);
     m_dvlcWaypoints->InsertColumn(m_dvlcWaypoints->GetColumnCount() - 1, etd_col);
     */
    wxDataViewTextRenderer* renderer = new wxDataViewTextRenderer();
    renderer->SetMode(wxDATAVIEW_CELL_EDITABLE);
    etd_col = new wxDataViewColumn (_("ETD"), renderer, m_dvlcWaypoints->GetColumnCount() - 1);
    m_dvlcWaypoints->InsertColumn(m_dvlcWaypoints->GetColumnCount() - 1, etd_col);
}

bool RoutePropDlgImpl::instanceFlag = false;
RoutePropDlgImpl* RoutePropDlgImpl::single = NULL;
RoutePropDlgImpl* RoutePropDlgImpl::getInstance( wxWindow* parent )
{
    if(! instanceFlag)
    {
        single = new RoutePropDlgImpl( parent );
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

void RoutePropDlgImpl::UpdatePoints()
{
    wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
    int selected_row = m_dvlcWaypoints->GetSelectedRow();
    m_dvlcWaypoints->DeleteAllItems();
    
    if( NULL == m_pRoute ) return;
    wxVector<wxVariant> data;
    
    m_pRoute->UpdateSegmentDistances( m_planspeed );           // to fix ETD properties
    m_tcDistance->SetValue(wxString::Format(wxT("%5.1f ") + getUsrDistanceUnit(), toUsrDistance(m_pRoute->m_route_length)));
    m_tcEnroute->SetValue(formatTimeDelta(wxLongLong(m_pRoute->m_route_time)));
    m_tcPlanSpeed->SetValue(wxString::FromDouble(toUsrDistance(m_pRoute->m_PlannedSpeed)));
    //  Iterate on Route Points, inserting blank fields starting with index 0
    wxRoutePointListNode *pnode = m_pRoute->pRoutePointList->GetFirst();
    int in = 0;
    wxString slen, eta, ete;
    double bearing, distance, speed;
    while( pnode ) {
        speed = pnode->GetData()->GetPlannedSpeed();
        if( speed < .1 ) {
            speed = m_pRoute->m_PlannedSpeed;
        }
        if( in == 0) {
            DistanceBearingMercator(pnode->GetData()->GetLatitude(), pnode->GetData()->GetLongitude(), gLat, gLon, &bearing, &distance);
            if( m_pRoute->m_PlannedDeparture.IsValid() ) {
                eta = wxString::Format("Start: %s", m_pRoute->m_PlannedDeparture.FormatISOCombined().c_str());
            } else {
                eta = _("N/A");
            }
            if( speed > .1 ) {
                ete = formatTimeDelta(wxLongLong(3600. * distance / speed));
            } else {
                ete = _("N/A");
            }
        } else {
            distance = pnode->GetData()->GetDistance();
            bearing = pnode->GetData()->GetCourse();
            eta = pnode->GetData()->GetETA();
            ete = pnode->GetData()->GetETE();
        }
        data.push_back( wxVariant(in == 0 ? "---" : std::to_string(in)) );
        data.push_back( wxVariant(pnode->GetData()->GetName()) ); // To
        slen.Printf( wxT("%5.1f ") + getUsrDistanceUnit(), toUsrDistance(distance) );
        data.push_back( wxVariant(slen) ); // Distance
        slen.Printf(wxT("%5.0f \u00B0T"), bearing);
        data.push_back( wxVariant(slen) ); // Bearing
        data.push_back( wxVariant(::toSDMM( 1, pnode->GetData()->GetLatitude(), FALSE)) ); // Lat
        data.push_back( wxVariant(::toSDMM( 1, pnode->GetData()->GetLongitude(), FALSE)) ); // Lon
        data.push_back( wxVariant(ete) ); // ETE
        data.push_back( eta ); //ETA
        data.push_back( wxVariant(wxVariant(wxString::FromDouble(toUsrSpeed(speed)))) ); // Speed
        data.push_back( wxVariant(pnode->GetData()->m_TideStation) ); // Next Tide event TODO
        data.push_back( wxVariant(pnode->GetData()->GetDescription()) ); // Description
        wxString etd = pnode->GetData()->GetETD();
        pnode = pnode->GetNext();
        if( pnode ) {
            slen.Printf(wxT("%5.0f \u00B0T"), pnode->GetData()->GetCourse());
            data.push_back( wxVariant(slen) ); // Course
        } else {
            data.push_back( wxVariant(_("Arrived")) );
        }
        data.push_back( wxVariant(etd) );
        data.push_back( wxVariant(wxEmptyString) ); // Empty column to fill the remaining space (Usually gets squeezed to zero, even if not empty)
        m_dvlcWaypoints->AppendItem( data );
        data.clear();
        in++;
    }
    if( selection > 0) {
        m_dvlcWaypoints->SelectRow(selected_row);
        m_dvlcWaypoints->EnsureVisible(selection);
    }
}

void RoutePropDlgImpl::SetRouteAndUpdate( Route *pR, bool only_points )
{
    if( NULL == pR )
        return;
    
    wxString title = pR->GetName() == wxEmptyString ? _("Route Properties") : pR->GetName();
    if( !pR->m_bIsInLayer )
        SetTitle( title );
    else {
        wxString caption( wxString::Format( _T("%s, %s: %s"), title, _("Layer"), GetLayerName( pR->m_LayerID ) ) );
        SetTitle( caption );
    }
    
    //  Fetch any config file values
    if ( !only_points )
    {
        // long LMT_Offset = 0;                    // offset in seconds from UTC for given location (-1 hr / 15 deg W)
        m_tz_selection = 1;
        
        if( pR == m_pRoute ) {
            gStart_LMT_Offset = 0;
            if( pR->m_PlannedDeparture.IsValid() )
                m_starttime = pR->m_PlannedDeparture;
            else
                m_starttime = g_StartTime;
            
        } else {
            g_StartTime = wxInvalidDateTime;
            g_StartTimeTZ = 1;
            if( pR->m_PlannedDeparture.IsValid() )
                m_starttime = pR->m_PlannedDeparture;
            else
                m_starttime = g_StartTime;
            if( pR->m_TimeDisplayFormat == RTE_TIME_DISP_UTC)
                m_tz_selection = 0;
            else if( pR->m_TimeDisplayFormat == RTE_TIME_DISP_LOCAL )
                m_tz_selection = 2;
            else
                m_tz_selection = g_StartTimeTZ;
            gStart_LMT_Offset = 0;
            m_pEnroutePoint = NULL;
            m_bStartNow = false;
            m_planspeed = pR->m_PlannedSpeed;
        }
        
        m_pRoute = pR;
        
        m_choiceTimezone->SetSelection(g_StartTimeTZ);
        
        if( m_pRoute ) {
            //    Calculate  LMT offset from the first point in the route
            if( m_pEnroutePoint && m_bStartNow ) gStart_LMT_Offset = long(
                                                                          ( m_pEnroutePoint->m_lon ) * 3600. / 15. );
            else
                gStart_LMT_Offset = long(
                                         ( m_pRoute->pRoutePointList->GetFirst()->GetData()->m_lon ) * 3600. / 15. );
        }
        
        // Reorganize dialog for route or track display
        m_tcName->SetValue( m_pRoute->m_RouteNameString );
        m_tcFrom->SetValue( m_pRoute->m_RouteStartString );
        m_tcTo->SetValue( m_pRoute->m_RouteEndString );
        m_tcName->SetFocus();
        if( m_pRoute->m_PlannedDeparture.IsValid() ) {
            m_dpDepartureDate->SetValue(m_pRoute->m_PlannedDeparture.GetDateOnly());
            m_tpDepartureTime->SetValue(m_pRoute->m_PlannedDeparture);
        }
    }
    UpdatePoints();
}

void RoutePropDlgImpl::DepartureDateOnDateChanged( wxDateEvent& event )
{
    wxDateTime dt = m_dpDepartureDate->GetValue();
    wxTimeSpan ts = m_tpDepartureTime->GetValue().Subtract(m_tpDepartureTime->GetValue().GetDateOnly());
    dt.Add(ts);
    m_pRoute->SetDepartureDate(dt);
    UpdatePoints();
    event.Skip();
}

void RoutePropDlgImpl::DepartureTimeOnTimeChanged( wxDateEvent& event )
{
    wxDateTime dt = m_dpDepartureDate->GetValue();
    wxTimeSpan ts = m_tpDepartureTime->GetValue().Subtract(m_tpDepartureTime->GetValue().GetDateOnly());
    dt.Add(ts);
    m_pRoute->SetDepartureDate(dt);
    UpdatePoints();
    event.Skip();
}

void RoutePropDlgImpl::PlanSpeedOnTextEnter( wxCommandEvent& event ) {
    double spd;
    if( m_tcPlanSpeed->GetValue().ToDouble(&spd) ) {
        if( m_pRoute->m_PlannedSpeed != fromUsrSpeed(spd) ) {
            m_pRoute->m_PlannedSpeed = fromUsrSpeed(spd);
            UpdatePoints();
        }
    } else {
        m_tcPlanSpeed->SetValue(wxString::FromDouble(toUsrSpeed(m_pRoute->m_PlannedSpeed)));
    }
}

void RoutePropDlgImpl::PlanSpeedOnKillFocus( wxFocusEvent& event ) {
    double spd;
    if( m_tcPlanSpeed->GetValue().ToDouble(&spd) ) {
        if( m_pRoute->m_PlannedSpeed != fromUsrSpeed(spd) ) {
            m_pRoute->m_PlannedSpeed = fromUsrSpeed(spd);
            UpdatePoints();
        }
    } else {
        m_tcPlanSpeed->SetValue(wxString::FromDouble(toUsrSpeed(m_pRoute->m_PlannedSpeed)));
    }
}

int ev_col;
void RoutePropDlgImpl::WaypointsOnDataViewListCtrlItemEditingDone( wxDataViewEvent& event )
{
    //There is a bug in wxWidgets, the EDITING_DONE event does not contain the new value, so we must save the data and do the work later in the value changed event.
    ev_col = event.GetColumn();
}

void RoutePropDlgImpl::WaypointsOnDataViewListCtrlItemValueChanged( wxDataViewEvent& event )
{
    wxDataViewModel* const model = event.GetModel();
    wxVariant value;
    model->GetValue(value, event.GetItem(), ev_col);
    RoutePoint *p = m_pRoute->GetPoint((int)(long)event.GetItem().GetID());
    if( ev_col == COLUMN_PLANNED_SPEED ) {
        double spd;
        if( !value.GetString().ToDouble(&spd) )
            spd = 0.0;
        p->SetPlannedSpeed(fromUsrSpeed(spd));
    }
    else if( ev_col == COLUMN_ETD ) {
        wxString::const_iterator end;
        wxDateTime etd;
        
        if( !etd.ParseDateTime(value.GetString(), &end) )
            etd = wxInvalidDateTime;
        p->SetETD(etd.FormatISOCombined());
    }
    UpdatePoints();
}

void RoutePropDlgImpl::WaypointsOnDataViewListCtrlItemContextMenu( wxDataViewEvent& event ) {
    wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
    RoutePoint *pRP = m_pRoute->GetPoint((int)(long)selection.GetID());
    
    if ( !g_pMarkInfoDialog )    // There is one global instance of the MarkProp Dialog. TODO: This is everywhere, redo as singleton
        g_pMarkInfoDialog = new MarkInfoDlg(GetParent());
    g_pMarkInfoDialog->SetRoutePoint(pRP);
    g_pMarkInfoDialog->UpdateProperties();
    if( pRP->m_bIsInLayer ) {
        wxString caption( wxString::Format( _T("%s, %s: %s"), _("Waypoint Properties"), _("Layer"), GetLayerName( pRP->m_LayerID ) ) );
        g_pMarkInfoDialog->SetDialogTitle( caption );
    } else
        g_pMarkInfoDialog->SetDialogTitle( _("Waypoint Properties") );
    
    if( !g_pMarkInfoDialog->IsShown() )
        g_pMarkInfoDialog->Show();
}
