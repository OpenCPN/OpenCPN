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

#include <wx/clipbrd.h>

#include "RoutePropDlgImpl.h"
#include "navutil.h"
#include "georef.h"
#include "MarkInfo.h"
#include "routeman.h"
#include "routemanagerdialog.h"
#include "routeprintout.h"
#include "chcanv.h"

#define ID_RCLK_MENU_COPY_TEXT 7013
#define ID_RCLK_MENU_EDIT_WP   7014
#define ID_RCLK_MENU_DELETE    7015

#define COLUMN_PLANNED_SPEED 8
#define COLUMN_ETD 12

extern wxString GetLayerName(int id);

extern long gStart_LMT_Offset;
extern int g_StartTimeTZ;
extern double gLat;
extern double gLon;
extern MarkInfoDlg *g_pMarkInfoDialog;
extern WayPointman *pWayPointMan;
extern Routeman *g_pRouteMan;
extern MyConfig *pConfig;
extern ColorScheme global_color_scheme;
extern RouteList *pRouteList;
extern Select *pSelect;
extern MyFrame *gFrame;
extern RouteManagerDialog *pRouteManagerDialog;

RoutePropDlgImpl::RoutePropDlgImpl( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : RoutePropDlg( parent, id, title, pos, size, style)
{
    m_pRoute = NULL;

    SetColorScheme(global_color_scheme);
    
    Connect( wxEVT_COMMAND_MENU_SELECTED,
        wxCommandEventHandler(RoutePropDlgImpl::OnRoutePropMenuSelected), NULL, this );
}

RoutePropDlgImpl::~RoutePropDlgImpl()
{
    Disconnect( wxEVT_COMMAND_MENU_SELECTED,
        wxCommandEventHandler(RoutePropDlgImpl::OnRoutePropMenuSelected), NULL, this );
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
    if( !m_pRoute )
        return;
    wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
    int selected_row = m_dvlcWaypoints->GetSelectedRow();
    m_dvlcWaypoints->DeleteAllItems();
    
    if( NULL == m_pRoute )
        return;
    wxVector<wxVariant> data;
    
    m_pRoute->UpdateSegmentDistances( m_pRoute->m_PlannedSpeed );           // to fix ETA properties
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
                eta = wxString::Format("Start: %s", m_pRoute->m_PlannedDeparture.Format(ETA_FORMAT_STR).c_str());
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
            if( pnode->GetData()->GetETA().IsValid() ) {
                eta = pnode->GetData()->GetETA().Format(ETA_FORMAT_STR);
            } else {
                eta = wxEmptyString;
            }
            ete = pnode->GetData()->GetETE();
        }
        wxString name = pnode->GetData()->GetName();
        double lat = pnode->GetData()->GetLatitude();
        double lon = pnode->GetData()->GetLongitude();
        wxString tide_station = pnode->GetData()->m_TideStation;
        wxString desc = pnode->GetData()->GetDescription();
        wxString etd;
        if( pnode->GetData()->GetManualETD().IsValid() ) {
            etd = pnode->GetData()->GetManualETD().Format(ETA_FORMAT_STR);
            if( pnode->GetData()->GetManualETD().IsValid() && pnode->GetData()->GetETA().IsValid() && pnode->GetData()->GetManualETD() < pnode->GetData()->GetETA() ) {
                etd.Prepend(_T("!! ")); // Manually enteed ETD is before we arrive here!
            }
        } else {
            etd = wxEmptyString;
        }
        pnode = pnode->GetNext();
        wxString crs;
        if( pnode ) {
            crs.Printf(wxT("%5.0f \u00B0T"), pnode->GetData()->GetCourse());
        } else {
            crs = _("Arrived");
        }

        data.push_back( wxVariant(in == 0 ? "---" : std::to_string(in)) );
        data.push_back( wxVariant(name) ); // To
        slen.Printf( wxT("%5.1f ") + getUsrDistanceUnit(), toUsrDistance(distance) );
        data.push_back( wxVariant(slen) ); // Distance
        slen.Printf(wxT("%5.0f \u00B0T"), bearing);
        data.push_back( wxVariant(slen) ); // Bearing
        data.push_back( wxVariant(::toSDMM( 1, lat, FALSE)) ); // Lat
        data.push_back( wxVariant(::toSDMM( 2, lon, FALSE)) ); // Lon
        data.push_back( wxVariant(ete) ); // ETE
        data.push_back( eta ); //ETA
        data.push_back( wxVariant(wxString::FromDouble(toUsrSpeed(speed))) ); // Speed
        data.push_back( wxVariant(tide_station) ); // Next Tide event TODO
        data.push_back( wxVariant(desc) ); // Description
        data.push_back( wxVariant(crs) );
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
 
    if( m_pRoute && m_pRoute != pR ) // We had unsaved changes, but now display another route
        ResetChanges();
    
    m_OrigRoute.m_PlannedDeparture = pR->m_PlannedDeparture;
    m_OrigRoute.m_PlannedSpeed = pR->m_PlannedSpeed;
    
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
            if( !pR->m_PlannedDeparture.IsValid() )
                pR->m_PlannedDeparture = wxDateTime::Now();
            
        } else {
            g_StartTimeTZ = 1;
            if( !pR->m_PlannedDeparture.IsValid() )
                pR->m_PlannedDeparture = wxDateTime::Now();

            if( pR->m_TimeDisplayFormat == RTE_TIME_DISP_UTC)
                m_tz_selection = 0;
            else if( pR->m_TimeDisplayFormat == RTE_TIME_DISP_LOCAL )
                m_tz_selection = 2;
            else
                m_tz_selection = g_StartTimeTZ;
            gStart_LMT_Offset = 0;
            m_pEnroutePoint = NULL;
            m_bStartNow = false;
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
        if( m_pRoute->m_PlannedDeparture.IsValid() && m_pRoute->m_PlannedDeparture.GetValue() > 0 ) {
            //TODO: Do the Local/LMC/UTC magic
            m_dpDepartureDate->SetValue(m_pRoute->m_PlannedDeparture.GetDateOnly());
            m_tpDepartureTime->SetValue(m_pRoute->m_PlannedDeparture);
        } else {
            m_dpDepartureDate->SetValue(wxDateTime::Now().GetDateOnly());
            m_tpDepartureTime->SetValue(wxDateTime::Now());
        }
    }
    
    if( m_pRoute->m_Colour == wxEmptyString ) {
        m_choiceColor->Select( 0 );
    } else {
        for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
            if( m_pRoute->m_Colour == ::GpxxColorNames[i] ) {
                m_choiceColor->Select( i + 1 );
                break;
            }
        }
    }
    
    for( unsigned int i = 0; i < sizeof( ::StyleValues ) / sizeof(int); i++ ) {
        if( m_pRoute->m_style == ::StyleValues[i] ) {
            m_choiceStyle->Select( i );
            break;
        }
    }
    
    for( unsigned int i = 0; i < sizeof( ::WidthValues ) / sizeof(int); i++ ) {
        if( m_pRoute->m_width == ::WidthValues[i] ) {
            m_choiceWidth->Select( i );
            break;
        }
    }
    
    UpdatePoints();
    
    m_btnSplit->Enable(false);
    m_btnExtend->Enable(IsThisRouteExtendable());
}

void RoutePropDlgImpl::DepartureDateOnDateChanged( wxDateEvent& event )
{
    if( !m_pRoute )
        return;
    m_pRoute->SetDepartureDate(GetDepartureTS());
    UpdatePoints();
    event.Skip();
}

void RoutePropDlgImpl::DepartureTimeOnTimeChanged( wxDateEvent& event )
{
    if( !m_pRoute )
        return;
    m_pRoute->SetDepartureDate(GetDepartureTS());
    UpdatePoints();
    event.Skip();
}

void RoutePropDlgImpl::PlanSpeedOnTextEnter( wxCommandEvent& event )
{
    if( !m_pRoute )
        return;
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

void RoutePropDlgImpl::PlanSpeedOnKillFocus( wxFocusEvent& event )
{
    if( !m_pRoute )
        return;
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
    if( !m_pRoute )
        return;
    wxDataViewModel* const model = event.GetModel();
    wxVariant value;
    model->GetValue(value, event.GetItem(), ev_col);
    RoutePoint *p = m_pRoute->GetPoint((int)(long)event.GetItem().GetID());
    if( ev_col == COLUMN_PLANNED_SPEED ) {
        double spd;
        if( !value.GetString().ToDouble(&spd) ) {
            spd = 0.0;
        }
        p->SetPlannedSpeed(fromUsrSpeed(spd));
    }
    else if( ev_col == COLUMN_ETD ) {
        wxString::const_iterator end;
        wxDateTime etd;
        
        if( !etd.ParseDateTime(value.GetString(), &end) ) {
            etd = wxInvalidDateTime;
        }
        p->SetETD(etd.FormatISOCombined());
    }
    UpdatePoints();
}

void RoutePropDlgImpl::WaypointsOnDataViewListCtrlSelectionChanged( wxDataViewEvent& event )
{
    if( m_dvlcWaypoints->GetSelectedRow() > 0 && m_dvlcWaypoints->GetSelectedRow() < m_dvlcWaypoints->GetItemCount() - 1 ) {
        m_btnSplit->Enable(true);
    } else {
        m_btnSplit->Enable(false);
    }
    if( IsThisRouteExtendable() ) {
        m_btnExtend->Enable(true);
    } else {
        m_btnExtend->Enable(false);
    }
}


wxDateTime RoutePropDlgImpl::GetDepartureTS() const
{
    // TODO: Do the Local/LMC/UTC magic
    wxDateTime dt = m_dpDepartureDate->GetValue();
    dt.SetHour(m_tpDepartureTime->GetValue().GetHour());
    dt.SetMinute(m_tpDepartureTime->GetValue().GetMinute());
    dt.SetSecond(m_tpDepartureTime->GetValue().GetSecond());
    return dt;
}


void RoutePropDlgImpl::OnRoutepropCopyTxtClick( wxCommandEvent& event )
{
    wxString tab("\t", wxConvUTF8);
    wxString eol("\n", wxConvUTF8);
    wxString csvString;
    
    csvString << this->GetTitle() << eol
    << _("Name") << tab << m_pRoute->m_RouteNameString << eol
    << _("Depart From") << tab << m_pRoute->m_RouteStartString << eol
    << _("Destination") << tab << m_pRoute->m_RouteEndString << eol
    << _("Total distance") << tab << m_tcDistance->GetValue() << eol
    << _("Speed (Kts)") << tab << m_tcPlanSpeed->GetValue() << eol
    << _("Departure Time") + _T(" (") + _T(ETA_FORMAT_STR) + _T(")") << tab << GetDepartureTS().Format(ETA_FORMAT_STR) << eol
    << _("Time enroute") << tab << m_tcEnroute->GetValue() << eol << eol;
    
    int noCols;
    int noRows;
    noCols = m_dvlcWaypoints->GetColumnCount();
    noRows = m_dvlcWaypoints->GetItemCount();
    wxListItem item;
    item.SetMask( wxLIST_MASK_TEXT );
    
    for( int i = 0; i < noCols; i++ ) {
        wxDataViewColumn* col = m_dvlcWaypoints->GetColumn(i);
        csvString << col->GetTitle() << tab;
    }
    csvString << eol;
    
    wxVariant value;
    for( int j = 0; j < noRows; j++ ) {
        for( int i = 0; i < noCols; i++ ) {
            m_dvlcWaypoints->GetValue(value, j, i);
            csvString << value.MakeString() << tab;
        }
        csvString << eol;
    }
    
    if( wxTheClipboard->Open() ) {
        wxTextDataObject* data = new wxTextDataObject;
        data->SetText( csvString );
        wxTheClipboard->SetData( data );
        wxTheClipboard->Close();
    }
}

void RoutePropDlgImpl::OnRoutePropMenuSelected( wxCommandEvent& event )
{
    switch( event.GetId() ) {
        case ID_RCLK_MENU_COPY_TEXT: {
            OnRoutepropCopyTxtClick( event );
            break;
        }
        case ID_RCLK_MENU_DELETE: {
            int dlg_return = OCPNMessageBox( this, _("Are you sure you want to remove this waypoint?"),
                                            _("OpenCPN Remove Waypoint"), (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
            
            if( dlg_return == wxID_YES ) {
                int sel = m_dvlcWaypoints->GetSelectedRow();
                m_dvlcWaypoints->SelectRow(sel);
                
                wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
                RoutePoint *pRP = m_pRoute->GetPoint((int)(long)selection.GetID());
                
                g_pRouteMan->RemovePointFromRoute( pRP, m_pRoute, NULL );
                
                UpdatePoints();
            }
            break;
        }
        case ID_RCLK_MENU_EDIT_WP: {
            wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
            RoutePoint *pRP = m_pRoute->GetPoint((int)(long)selection.GetID());
            
            RouteManagerDialog::WptShowPropertiesDialog( pRP, this );
            break;
        }
    }
}

void RoutePropDlgImpl::WaypointsOnDataViewListCtrlItemContextMenu( wxDataViewEvent& event )
{
    wxMenu menu;
    
    if( ! m_pRoute->m_bIsInLayer ) {
#ifdef __OCPN_ANDROID__
        wxFont *pf = OCPNGetFont(_T("Menu"), 0);
        // add stuff
        wxMenuItem *editItem = new wxMenuItem(&menu, ID_RCLK_MENU_EDIT_WP, _("Waypoint Properties") + _T("..."));
        editItem->SetFont(*pf);
        menu.Append(editItem);
        
        wxMenuItem *delItem = new wxMenuItem(&menu, ID_RCLK_MENU_DELETE, _("Remove Selected"));
        delItem->SetFont(*pf);
        menu.Append(delItem);
#else
        wxMenuItem* editItem = menu.Append( ID_RCLK_MENU_EDIT_WP, _("&Waypoint Properties...") );
        wxMenuItem* delItem = menu.Append( ID_RCLK_MENU_DELETE, _("&Remove Selected") );
#endif
        editItem->Enable( m_dvlcWaypoints->GetSelection() > 0 );
        delItem->Enable( m_dvlcWaypoints->GetSelection() > 0 && m_dvlcWaypoints->GetItemCount() > 2 );
    }
#ifndef __WXQT__
    wxMenuItem* copyItem = menu.Append( ID_RCLK_MENU_COPY_TEXT, _("&Copy all as text") );
#endif
    
    PopupMenu( &menu );
}

void RoutePropDlgImpl::ResetChanges()
{
    if( !m_pRoute )
        return;
    m_pRoute->m_PlannedSpeed = m_OrigRoute.m_PlannedSpeed;
    m_pRoute->m_PlannedDeparture = m_OrigRoute.m_PlannedDeparture;
    m_pRoute = NULL;
}

void RoutePropDlgImpl::SaveChanges()
{
    if( m_pRoute && !m_pRoute->m_bIsInLayer ) {
        //  Get User input Text Fields
        m_pRoute->m_RouteNameString = m_tcName->GetValue();
        m_pRoute->m_RouteStartString = m_tcFrom->GetValue();
        m_pRoute->m_RouteEndString = m_tcTo->GetValue();
        if( m_choiceColor->GetSelection() == 0 ) {
            m_pRoute->m_Colour = wxEmptyString;
        } else {
            m_pRoute->m_Colour = ::GpxxColorNames[m_choiceColor->GetSelection() - 1];
        }
        m_pRoute->m_style = (wxPenStyle)::StyleValues[m_choiceStyle->GetSelection()];
        m_pRoute->m_width = ::WidthValues[m_choiceWidth->GetSelection()];
        switch( g_StartTimeTZ ) {
            case 1 :
                m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_PC;
                break;
            case 2 :
                m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_LOCAL;
                break;
            default:
                m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_UTC;
        }
        
        pConfig->UpdateRoute( m_pRoute );
        pConfig->UpdateSettings();
        m_pRoute = NULL;
    }
}

void RoutePropDlgImpl::SetColorScheme( ColorScheme cs )
{
    DimeControl( this );
}

void RoutePropDlgImpl::SplitOnButtonClick( wxCommandEvent& event )
{
    m_btnSplit->Enable( false );
    
    if( m_pRoute->m_bIsInLayer )
        return;
    
    int nSelected = m_dvlcWaypoints->GetSelectedRow() + 1;
    if( ( nSelected > 1 ) && ( nSelected < m_pRoute->GetnPoints() ) ) {
        m_pHead = new Route();
        m_pTail = new Route();
        m_pHead->CloneRoute( m_pRoute, 1, nSelected, _("_A") );
        m_pTail->CloneRoute( m_pRoute, nSelected, m_pRoute->GetnPoints(), _("_B"), true );
        pRouteList->Append( m_pHead );
        pConfig->AddNewRoute( m_pHead );
        
        pRouteList->Append( m_pTail );
        pConfig->AddNewRoute( m_pTail );
        
        pConfig->DeleteConfigRoute( m_pRoute );
        
        pSelect->DeleteAllSelectableRoutePoints( m_pRoute );
        pSelect->DeleteAllSelectableRouteSegments( m_pRoute );
        g_pRouteMan->DeleteRoute( m_pRoute );
        pSelect->AddAllSelectableRouteSegments( m_pTail );
        pSelect->AddAllSelectableRoutePoints( m_pTail );
        pSelect->AddAllSelectableRouteSegments( m_pHead );
        pSelect->AddAllSelectableRoutePoints( m_pHead );
        
        SetRouteAndUpdate( m_pTail );
        UpdatePoints();
        
        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateRouteListCtrl();
    }
}

void RoutePropDlgImpl::PrintOnButtonClick( wxCommandEvent& event )
{
    RoutePrintSelection dlg( GetParent(), m_pRoute );
    dlg.ShowModal();
}

void RoutePropDlgImpl::ExtendOnButtonClick( wxCommandEvent& event )
{
    m_btnExtend->Enable( false );
    
    if( IsThisRouteExtendable() ) {
        int fm = m_pExtendRoute->GetIndexOf( m_pExtendPoint ) + 1;
        int to = m_pExtendRoute->GetnPoints();
        if( fm <= to ) {
            pSelect->DeleteAllSelectableRouteSegments( m_pRoute );
            m_pRoute->CloneRoute( m_pExtendRoute, fm, to, _("_plus") );
            pSelect->AddAllSelectableRouteSegments( m_pRoute );
            SetRouteAndUpdate( m_pRoute );
            UpdatePoints();
        }
    }
    m_btnExtend->Enable( true );
}

bool RoutePropDlgImpl::IsThisRouteExtendable()
{
    m_pExtendRoute = NULL;
    m_pExtendPoint = NULL;
    if( m_pRoute->m_bRtIsActive || m_pRoute->m_bIsInLayer )
        return false;
    
    RoutePoint *pLastPoint = m_pRoute->GetLastPoint();
    wxArrayPtrVoid *pEditRouteArray;
    
    pEditRouteArray = g_pRouteMan->GetRouteArrayContaining( pLastPoint );
    // remove invisible & own routes from choices
    int i;
    for( i = pEditRouteArray->GetCount(); i > 0; i-- ) {
        Route *p = (Route *) pEditRouteArray->Item( i - 1 );
        if( !p->IsVisible() || ( p->m_GUID == m_pRoute->m_GUID ) ) pEditRouteArray->RemoveAt( i - 1 );
    }
    if( pEditRouteArray->GetCount() == 1 ) {
        m_pExtendPoint = pLastPoint;
    } else {
        if( pEditRouteArray->GetCount() == 0 ) {
            int nearby_radius_meters = (int) ( 8. / gFrame->GetPrimaryCanvas()->GetCanvasTrueScale() );
            double rlat = pLastPoint->m_lat;
            double rlon = pLastPoint->m_lon;
            
            m_pExtendPoint = pWayPointMan->GetOtherNearbyWaypoint( rlat, rlon,
                nearby_radius_meters, pLastPoint->m_GUID );
            if( m_pExtendPoint ) {
                wxArrayPtrVoid *pCloseWPRouteArray = g_pRouteMan->GetRouteArrayContaining( m_pExtendPoint );
                if( pCloseWPRouteArray ) {
                    pEditRouteArray = pCloseWPRouteArray;
                    
                    // remove invisible & own routes from choices
                    for( i = pEditRouteArray->GetCount(); i > 0; i-- ) {
                        Route *p = (Route *) pEditRouteArray->Item( i - 1 );
                        if( !p->IsVisible() || ( p->m_GUID == m_pRoute->m_GUID ) ) pEditRouteArray->RemoveAt( i - 1 );
                    }
                }
            }
        }
    }
    if( pEditRouteArray->GetCount() == 1 ) {
        Route *p = (Route *) pEditRouteArray->Item( 0 );
        int fm = p->GetIndexOf( m_pExtendPoint ) + 1;
        int to = p->GetnPoints();
        if( fm <= to ) {
            m_pExtendRoute = p;
            delete pEditRouteArray;
            return true;
        }
    }
    delete pEditRouteArray;
    
    return false;
}
