/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Route Manager
 * Author:   David Register
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
 **************************************************************************/

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "wx/image.h"
#include "wx/tokenzr.h"
#include <wx/progdlg.h>

#include "dychart.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <wx/listimpl.cpp>

#include "styles.h"
#include "routeman.h"
#include "concanv.h"
#include "navutil.h"
#include "georef.h"
#include "routeprop.h"
#include "routemanagerdialog.h"
#include "pluginmanager.h"
#include "multiplexer.h"
#include "MarkIcon.h"
#include "cutil.h"
#include "AIS_Decoder.h"
#include "wx28compat.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/apptrait.h>
#include "OCPNPlatform.h"


extern OCPNPlatform     *g_Platform;
extern ConsoleCanvas    *console;

extern RouteList        *pRouteList;
extern TrackList        *pTrackList;
extern Select           *pSelect;
extern MyConfig         *pConfig;
extern Routeman         *g_pRouteMan;

extern wxRect           g_blink_rect;

extern double           gLat, gLon, gSog, gCog;
extern double           gVar;

extern bool             g_bMagneticAPB;

extern RoutePoint       *pAnchorWatchPoint1;
extern RoutePoint       *pAnchorWatchPoint2;

extern ActiveTrack      *g_pActiveTrack;
extern RouteProp        *pRoutePropDialog;
extern RouteManagerDialog *pRouteManagerDialog;
extern RoutePoint      *pAnchorWatchPoint1;
extern RoutePoint      *pAnchorWatchPoint2;
extern int              g_route_line_width;
extern Multiplexer     *g_pMUX;
extern AIS_Decoder     *g_pAIS;

extern PlugInManager    *g_pi_manager;
extern ocpnStyle::StyleManager* g_StyleManager;
extern wxString         g_uploadConnection;
extern bool             g_bAdvanceRouteWaypointOnArrivalOnly;
extern Route            *pAISMOBRoute;
extern bool             g_btouch;
extern float            g_ChartScaleFactorExp;

//    List definitions for Waypoint Manager Icons
WX_DECLARE_LIST(wxBitmap, markicon_bitmap_list_type);
WX_DECLARE_LIST(wxString, markicon_key_list_type);
WX_DECLARE_LIST(wxString, markicon_description_list_type);

//    List implementation for Waypoint Manager Icons
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(markicon_bitmap_list_type);
WX_DEFINE_LIST(markicon_key_list_type);
WX_DEFINE_LIST(markicon_description_list_type);



//--------------------------------------------------------------------------------
//      Routeman   "Route Manager"
//--------------------------------------------------------------------------------

Routeman::Routeman( MyApp *parent )
{
    m_pparent_app = parent;
    pActiveRoute = NULL;
    pActivePoint = NULL;
    pRouteActivatePoint = NULL;
}

Routeman::~Routeman()
{
    if( pRouteActivatePoint ) delete pRouteActivatePoint;
}

bool Routeman::IsRouteValid( Route *pRoute )
{
    wxRouteListNode *node = pRouteList->GetFirst();
    while( node ) {
        if( pRoute == node->GetData() ) return true;
        node = node->GetNext();
    }
    return false;
}

//    Make a 2-D search to find the route containing a given waypoint
Route *Routeman::FindRouteContainingWaypoint( RoutePoint *pWP )
{
    wxRouteListNode *node = pRouteList->GetFirst();
    while( node ) {
        Route *proute = node->GetData();

        wxRoutePointListNode *pnode = ( proute->pRoutePointList )->GetFirst();
        while( pnode ) {
            RoutePoint *prp = pnode->GetData();
            if( prp == pWP )  return proute;
            pnode = pnode->GetNext();
        }

        node = node->GetNext();
    }

    return NULL;                              // not found
}

wxArrayPtrVoid *Routeman::GetRouteArrayContaining( RoutePoint *pWP )
{
    wxArrayPtrVoid *pArray = new wxArrayPtrVoid;

    wxRouteListNode *route_node = pRouteList->GetFirst();
    while( route_node ) {
        Route *proute = route_node->GetData();

        wxRoutePointListNode *waypoint_node = ( proute->pRoutePointList )->GetFirst();
        while( waypoint_node ) {
            RoutePoint *prp = waypoint_node->GetData();
            if( prp == pWP ){              // success
                pArray->Add( (void *) proute );
                break;          // only add a route to the array once, even if there are duplicate points
                                // in the route...See FS#1743
            }

            waypoint_node = waypoint_node->GetNext();           // next waypoint
        }

        route_node = route_node->GetNext();                         // next route
    }

    if( pArray->GetCount() ) return pArray;

    else {
        delete pArray;
        return NULL;
    }
}

RoutePoint *Routeman::FindBestActivatePoint( Route *pR, double lat, double lon, double cog,
        double sog )
{
    if( !pR ) return NULL;

    // Walk thru all the points to find the "best"
    RoutePoint *best_point = NULL;
    double min_time_found = 1e6;

    wxRoutePointListNode *node = ( pR->pRoutePointList )->GetFirst();
    while( node ) {
        RoutePoint *pn = node->GetData();

        double brg, dist;
        DistanceBearingMercator( pn->m_lat, pn->m_lon, lat, lon, &brg, &dist );

        double angle = brg - cog;
        double soa = cos( angle * PI / 180. );

        double time_to_wp = dist / soa;

        if( time_to_wp > 0 ) {
            if( time_to_wp < min_time_found ) {
                min_time_found = time_to_wp;
                best_point = pn;
            }
        }
        node = node->GetNext();
    }
    return best_point;
}

bool Routeman::ActivateRoute( Route *pRouteToActivate, RoutePoint *pStartPoint )
{
    pActiveRoute = pRouteToActivate;

    if( pStartPoint ) {
        pActivePoint = pStartPoint;
    } else {
        wxRoutePointListNode *node = ( pActiveRoute->pRoutePointList )->GetFirst();
        pActivePoint = node->GetData();               // start at beginning
    }

    wxJSONValue v;
    v[_T("Route_activated")] = pRouteToActivate->m_RouteNameString;
    v[_T("GUID")] = pRouteToActivate->m_GUID;
    wxString msg_id( _T("OCPN_RTE_ACTIVATED") );
    g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );

    ActivateRoutePoint( pRouteToActivate, pActivePoint );

    m_bArrival = false;
    m_arrival_min = 1e6;
    m_arrival_test = 0;

    pRouteToActivate->m_bRtIsActive = true;

    m_bDataValid = false;

    console->ShowWithFreshFonts();

    return true;
}

bool Routeman::ActivateRoutePoint( Route *pA, RoutePoint *pRP_target )
{
    wxJSONValue v;
    pActiveRoute = pA;

    pActivePoint = pRP_target;
    pActiveRoute->m_pRouteActivePoint = pRP_target;

    v[_T("GUID")] = pRP_target->m_GUID;
    v[_T("WP_activated")] = pRP_target->GetName();

    wxRoutePointListNode *node = ( pActiveRoute->pRoutePointList )->GetFirst();
    while( node ) {
        RoutePoint *pn = node->GetData();
        pn->m_bBlink = false;                     // turn off all blinking points
        pn->m_bIsActive = false;

        node = node->GetNext();
    }

    node = ( pActiveRoute->pRoutePointList )->GetFirst();
    RoutePoint *prp_first = node->GetData();

    //  If activating first point in route, create a "virtual" waypoint at present position
    if( pRP_target == prp_first ) {
        if( pRouteActivatePoint ) delete pRouteActivatePoint;

        pRouteActivatePoint = new RoutePoint( gLat, gLon, wxString( _T("") ), wxString( _T("") ),
                GPX_EMPTY_STRING, false ); // Current location
        pRouteActivatePoint->m_bShowName = false;

        pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
    }

    else {
        prp_first->m_bBlink = false;
        node = node->GetNext();
        RoutePoint *np_prev = prp_first;
        while( node ) {
            RoutePoint *pnext = node->GetData();
            if( pnext == pRP_target ) {
                pActiveRouteSegmentBeginPoint = np_prev;
                break;
            }

            np_prev = pnext;
            node = node->GetNext();
        }
    }

    pRP_target->m_bBlink = true;                               // blink the active point
    pRP_target->m_bIsActive = true;                            // and active

    g_blink_rect = pRP_target->CurrentRect_in_DC;               // set up global blinker

    m_bArrival = false;
    m_arrival_min = 1e6;
    m_arrival_test = 0;
    

    //    Update the RouteProperties Dialog, if currently shown
    if( ( NULL != pRoutePropDialog ) && ( pRoutePropDialog->IsShown() ) ) {
        if( pRoutePropDialog->m_pRoute == pA ) {
            if( pRoutePropDialog->m_pEnroutePoint ) pRoutePropDialog->m_pEnroutePoint =
                    pActivePoint;
            pRoutePropDialog->SetRouteAndUpdate( pA );
            pRoutePropDialog->UpdateProperties();
        }
    }

    wxString msg_id( _T("OCPN_WPT_ACTIVATED") );
    g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );

    return true;
}

bool Routeman::ActivateNextPoint( Route *pr, bool skipped )
{
    wxJSONValue v;
    if( pActivePoint ) {
        pActivePoint->m_bBlink = false;
        pActivePoint->m_bIsActive = false;

        v[_T("isSkipped")] = skipped;
        v[_T("GUID")] = pActivePoint->m_GUID;
        v[_T("WP_arrived")] = pActivePoint->GetName();
    }
    int n_index_active = pActiveRoute->GetIndexOf( pActivePoint );
    if( ( n_index_active + 1 ) <= pActiveRoute->GetnPoints() ) {
        pActiveRouteSegmentBeginPoint = pActivePoint;

        pActiveRoute->m_pRouteActivePoint = pActiveRoute->GetPoint( n_index_active + 1 );

        pActivePoint = pActiveRoute->GetPoint( n_index_active + 1 );
        v[_T("Next_WP")] = pActivePoint->GetName();
        v[_T("GUID")] = pActivePoint->m_GUID;

        pActivePoint->m_bBlink = true;
        pActivePoint->m_bIsActive = true;
        g_blink_rect = pActivePoint->CurrentRect_in_DC;               // set up global blinker

        m_bArrival = false;
        m_arrival_min = 1e6;
        m_arrival_test = 0;

        //    Update the RouteProperties Dialog, if currently shown
        if( ( NULL != pRoutePropDialog ) && ( pRoutePropDialog->IsShown() ) ) {
            if( pRoutePropDialog->m_pRoute == pr ) {
                if( pRoutePropDialog->m_pEnroutePoint ) pRoutePropDialog->m_pEnroutePoint =
                        pActivePoint;
                pRoutePropDialog->SetRouteAndUpdate( pr );
                pRoutePropDialog->UpdateProperties();
            }
        }

        wxString msg_id( _T("OCPN_WPT_ARRIVED") );
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );

        return true;
    }

    return false;
}

bool Routeman::UpdateProgress()
{
    bool bret_val = false;

    if( pActiveRoute ) {
//      Update bearing, range, and crosstrack error

//  Bearing is calculated as Mercator Sailing, i.e. a  cartographic "bearing"
        double north, east;
        toSM( pActivePoint->m_lat, pActivePoint->m_lon, gLat, gLon, &east, &north );
        double a = atan( north / east );
        if( fabs( pActivePoint->m_lon - gLon ) < 180. ) {
            if( pActivePoint->m_lon > gLon ) CurrentBrgToActivePoint = 90. - ( a * 180 / PI );
            else
                CurrentBrgToActivePoint = 270. - ( a * 180 / PI );
        } else {
            if( pActivePoint->m_lon > gLon ) CurrentBrgToActivePoint = 270. - ( a * 180 / PI );
            else
                CurrentBrgToActivePoint = 90. - ( a * 180 / PI );
        }

//      Calculate range using Great Circle Formula

        double d5 = DistGreatCircle( gLat, gLon, pActivePoint->m_lat, pActivePoint->m_lon );
        CurrentRngToActivePoint = d5;

//      Get the XTE vector, normal to current segment
        vector2D va, vb, vn;

        double brg1, dist1, brg2, dist2;
        DistanceBearingMercator( pActivePoint->m_lat, pActivePoint->m_lon,
                pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon, &brg1,
                &dist1 );
        vb.x = dist1 * sin( brg1 * PI / 180. );
        vb.y = dist1 * cos( brg1 * PI / 180. );

        DistanceBearingMercator( pActivePoint->m_lat, pActivePoint->m_lon, gLat, gLon, &brg2,
                &dist2 );
        va.x = dist2 * sin( brg2 * PI / 180. );
        va.y = dist2 * cos( brg2 * PI / 180. );

        double sdelta = vGetLengthOfNormal( &va, &vb, &vn );             // NM
        CurrentXTEToActivePoint = sdelta;

//    Calculate the distance to the arrival line, which is perpendicular to the current route segment
//    Taking advantage of the calculated normal from current position to route segment vn
        vector2D vToArriveNormal;
        vSubtractVectors( &va, &vn, &vToArriveNormal );

        CurrentRangeToActiveNormalCrossing = vVectorMagnitude( &vToArriveNormal );

//          Compute current segment course
//          Using simple Mercater projection
        double x1, y1, x2, y2;
        toSM( pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon,
                pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon, &x1,
                &y1 );

        toSM( pActivePoint->m_lat, pActivePoint->m_lon, pActiveRouteSegmentBeginPoint->m_lat,
                pActiveRouteSegmentBeginPoint->m_lon, &x2, &y2 );

        double e1 = atan2( ( x2 - x1 ), ( y2 - y1 ) );
        CurrentSegmentCourse = e1 * 180 / PI;
        if( CurrentSegmentCourse < 0 ) CurrentSegmentCourse += 360;

        //      Compute XTE direction
        double h = atan( vn.y / vn.x );
        if( vn.x > 0 ) CourseToRouteSegment = 90. - ( h * 180 / PI );
        else
            CourseToRouteSegment = 270. - ( h * 180 / PI );

        h = CurrentBrgToActivePoint - CourseToRouteSegment;
        if( h < 0 ) h = h + 360;

        if( h > 180 ) XTEDir = 1;
        else
            XTEDir = -1;

//      Determine Arrival

        bool bDidArrival = false;

        // Special signal:  if ArrivalRadius < 0, NEVER arrive...
        //  Used for MOB auto-created routes.
        if( pActivePoint->GetWaypointArrivalRadius() > 0){
            if( CurrentRangeToActiveNormalCrossing <= pActivePoint->GetWaypointArrivalRadius() ) {
                m_bArrival = true;
                UpdateAutopilot();

                bDidArrival = true;
                DoAdvance();

            }
            else {
            //      Test to see if we are moving away from the arrival point, and
            //      have been moving away for 2 seconds.  
            //      If so, we should declare "Arrival"
                if( (CurrentRangeToActiveNormalCrossing - m_arrival_min) >  pActivePoint->GetWaypointArrivalRadius() ){
                    if(++m_arrival_test > 2 && !g_bAdvanceRouteWaypointOnArrivalOnly) {
                        m_bArrival = true;
                        UpdateAutopilot();
                        
                        bDidArrival = true;
                        DoAdvance();
                    }
                }
                else
                    m_arrival_test = 0;
                    
            }
        }
        
        if( !bDidArrival )                                        
            m_arrival_min = wxMin( m_arrival_min, CurrentRangeToActiveNormalCrossing );
        
        if( !bDidArrival )                                        // Only once on arrival
            UpdateAutopilot();

        bret_val = true;                                        // a route is active
    }

    m_bDataValid = true;

    return bret_val;
}

void Routeman::DoAdvance(void)
{
    if( !ActivateNextPoint( pActiveRoute, false ) )            // at the end?
    {
        Route *pthis_route = pActiveRoute;
        DeactivateRoute( true );                  // this is an arrival
        
        if( pthis_route->m_bDeleteOnArrival && !pthis_route->m_bIsBeingEdited) {
            pConfig->DeleteConfigRoute( pthis_route );
            DeleteRoute( pthis_route );
            if( pRoutePropDialog && ( pRoutePropDialog->IsShown()) && (pthis_route == pRoutePropDialog->GetRoute()) ) {
                pRoutePropDialog->Hide();
            }
        }

        if( pRouteManagerDialog )
            pRouteManagerDialog->UpdateRouteListCtrl();
                            
    }
}
                    
    

bool Routeman::DeactivateRoute( bool b_arrival )
{
    if( pActivePoint ) {
        pActivePoint->m_bBlink = false;
        pActivePoint->m_bIsActive = false;
    }

    if( pActiveRoute ) {
        pActiveRoute->m_bRtIsActive = false;
        pActiveRoute->m_pRouteActivePoint = NULL;

        wxJSONValue v;
        if( !b_arrival ) {
            v[_T("Route_deactivated")] = pActiveRoute->m_RouteNameString;
            v[_T("GUID")] = pActiveRoute->m_GUID;
            wxString msg_id( _T("OCPN_RTE_DEACTIVATED") );
            g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
        } else {
            v[_T("GUID")] = pActiveRoute->m_GUID;
            v[_T("Route_ended")] = pActiveRoute->m_RouteNameString;
            wxString msg_id( _T("OCPN_RTE_ENDED") );
            g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
        }
    }

    pActiveRoute = NULL;

    if( pRouteActivatePoint ) delete pRouteActivatePoint;
    pRouteActivatePoint = NULL;

    console->pCDI->ClearBackground();

    console->Show( false );

    m_bDataValid = false;

    return true;
}

bool Routeman::UpdateAutopilot()
{
    //Send all known Autopilot messages upstream
    
    //RMB
        {

            m_NMEA0183.TalkerID = _T("EC");

            SENTENCE snt;
            m_NMEA0183.Rmb.IsDataValid = NTrue;
            m_NMEA0183.Rmb.CrossTrackError = CurrentXTEToActivePoint;

            if( XTEDir < 0 ) m_NMEA0183.Rmb.DirectionToSteer = Left;
            else
                m_NMEA0183.Rmb.DirectionToSteer = Right;

            m_NMEA0183.Rmb.To = pActivePoint->GetName().Truncate( 6 );
            m_NMEA0183.Rmb.From = pActiveRouteSegmentBeginPoint->GetName().Truncate( 6 );

            if( pActivePoint->m_lat < 0. ) m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(
                    -pActivePoint->m_lat, _T("S") );
            else
                m_NMEA0183.Rmb.DestinationPosition.Latitude.Set( pActivePoint->m_lat, _T("N") );

            if( pActivePoint->m_lon < 0. ) m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(
                    -pActivePoint->m_lon, _T("W") );
            else
                m_NMEA0183.Rmb.DestinationPosition.Longitude.Set( pActivePoint->m_lon, _T("E") );

            m_NMEA0183.Rmb.RangeToDestinationNauticalMiles = CurrentRngToActivePoint;
            m_NMEA0183.Rmb.BearingToDestinationDegreesTrue = CurrentBrgToActivePoint;
            m_NMEA0183.Rmb.DestinationClosingVelocityKnots = gSog;

            if( m_bArrival ) m_NMEA0183.Rmb.IsArrivalCircleEntered = NTrue;
            else
                m_NMEA0183.Rmb.IsArrivalCircleEntered = NFalse;

            m_NMEA0183.Rmb.Write( snt );

            g_pMUX->SendNMEAMessage( snt.Sentence );
        }

        // RMC
        {

            m_NMEA0183.TalkerID = _T("EC");

            SENTENCE snt;
            m_NMEA0183.Rmc.IsDataValid = NTrue;

            if( gLat < 0. ) m_NMEA0183.Rmc.Position.Latitude.Set( -gLat, _T("S") );
            else
                m_NMEA0183.Rmc.Position.Latitude.Set( gLat, _T("N") );

            if( gLon < 0. ) m_NMEA0183.Rmc.Position.Longitude.Set( -gLon, _T("W") );
            else
                m_NMEA0183.Rmc.Position.Longitude.Set( gLon, _T("E") );

            m_NMEA0183.Rmc.SpeedOverGroundKnots = gSog;
            m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue = gCog;

            if( !wxIsNaN(gVar) ) {
                if( gVar < 0. ) {
                    m_NMEA0183.Rmc.MagneticVariation = -gVar;
                    m_NMEA0183.Rmc.MagneticVariationDirection = West;
                } else {
                    m_NMEA0183.Rmc.MagneticVariation = gVar;
                    m_NMEA0183.Rmc.MagneticVariationDirection = East;
                }
            } else
                m_NMEA0183.Rmc.MagneticVariation = 361.; // A signal to NMEA converter, gVAR is unknown

            wxDateTime now = wxDateTime::Now();
            wxDateTime utc = now.ToUTC();
            wxString time = utc.Format( _T("%H%M%S") );
            m_NMEA0183.Rmc.UTCTime = time;

            wxString date = utc.Format( _T("%d%m%y") );
            m_NMEA0183.Rmc.Date = date;

            m_NMEA0183.Rmc.Write( snt );

            g_pMUX->SendNMEAMessage( snt.Sentence );
        }

        // APB
        {
            m_NMEA0183.TalkerID = _T("EC");
            
            SENTENCE snt;
             
            m_NMEA0183.Apb.IsLoranBlinkOK = NTrue;
            m_NMEA0183.Apb.IsLoranCCycleLockOK = NTrue;
            
            m_NMEA0183.Apb.CrossTrackErrorMagnitude = CurrentXTEToActivePoint;
            
            if( XTEDir < 0 ) m_NMEA0183.Apb.DirectionToSteer = Left;
            else
                m_NMEA0183.Apb.DirectionToSteer = Right;
            
            m_NMEA0183.Apb.CrossTrackUnits = _T("N");

            if( m_bArrival )
                m_NMEA0183.Apb.IsArrivalCircleEntered = NTrue;
            else
                m_NMEA0183.Apb.IsArrivalCircleEntered = NFalse;
 
            //  We never pass the perpendicular, since we declare arrival before reaching this point
            m_NMEA0183.Apb.IsPerpendicular = NFalse;

            m_NMEA0183.Apb.To = pActivePoint->GetName().Truncate( 6 );
            
            double brg1, dist1;
            DistanceBearingMercator( pActivePoint->m_lat, pActivePoint->m_lon,
                                     pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon,
                                     &brg1,
                                     &dist1 );
            
            if( g_bMagneticAPB && !wxIsNaN(gVar) ) {
                
                double brg1m = ((brg1 - gVar) >= 0.) ? (brg1 - gVar) : (brg1 - gVar + 360.);
                double bapm = ((CurrentBrgToActivePoint - gVar) >= 0.) ? (CurrentBrgToActivePoint - gVar) : (CurrentBrgToActivePoint - gVar + 360.);
                
                m_NMEA0183.Apb.BearingOriginToDestination = brg1m;
                m_NMEA0183.Apb.BearingOriginToDestinationUnits = _T("M");
                
                m_NMEA0183.Apb.BearingPresentPositionToDestination = bapm;
                m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("M");
                
                m_NMEA0183.Apb.HeadingToSteer = bapm;
                m_NMEA0183.Apb.HeadingToSteerUnits = _T("M");
            }
            else {
                m_NMEA0183.Apb.BearingOriginToDestination = brg1;
                m_NMEA0183.Apb.BearingOriginToDestinationUnits = _T("T");

                m_NMEA0183.Apb.BearingPresentPositionToDestination = CurrentBrgToActivePoint;
                m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("T");
            

                m_NMEA0183.Apb.HeadingToSteer = CurrentBrgToActivePoint;
                m_NMEA0183.Apb.HeadingToSteerUnits = _T("T");
            }
            
            m_NMEA0183.Apb.Write( snt );
            g_pMUX->SendNMEAMessage( snt.Sentence );
        }
        
        // XTE
        {
            m_NMEA0183.TalkerID = _T("EC");
            
            SENTENCE snt;
             
            m_NMEA0183.Xte.IsLoranBlinkOK = NTrue;
            m_NMEA0183.Xte.IsLoranCCycleLockOK = NTrue;
            
            m_NMEA0183.Xte.CrossTrackErrorDistance = CurrentXTEToActivePoint;
            
            if( XTEDir < 0 ) m_NMEA0183.Xte.DirectionToSteer = Left;
            else
                m_NMEA0183.Xte.DirectionToSteer = Right;
            
            m_NMEA0183.Xte.CrossTrackUnits = _T("N");

            m_NMEA0183.Xte.Write( snt );
            g_pMUX->SendNMEAMessage( snt.Sentence );
        }
        
       
    return true;
}

bool Routeman::DoesRouteContainSharedPoints( Route *pRoute )
{
    if( pRoute ) {
        // walk the route, looking at each point to see if it is used by another route
        // or is isolated
        wxRoutePointListNode *pnode = ( pRoute->pRoutePointList )->GetFirst();
        while( pnode ) {
            RoutePoint *prp = pnode->GetData();

            // check all other routes to see if this point appears in any other route
            wxArrayPtrVoid *pRA = GetRouteArrayContaining( prp );
            
             if( pRA ) {
                 for( unsigned int ir = 0; ir < pRA->GetCount(); ir++ ) {
                    Route *pr = (Route *) pRA->Item( ir );
                    if( pr == pRoute)
                        continue;               // self
                    else 
                        return true;
                }
            }
                
            if( pnode ) pnode = pnode->GetNext();
        }
        
        //      Now walk the route again, looking for isolated type shared waypoints
        pnode = ( pRoute->pRoutePointList )->GetFirst();
        while( pnode ) {
            RoutePoint *prp = pnode->GetData();
            if( prp->m_bKeepXRoute == true )
                return true;
            
           if( pnode ) pnode = pnode->GetNext();
        }
    }
    
    return false;
}
  


bool Routeman::DeleteRoute( Route *pRoute )
{
    if( pRoute ) {
        if( pRoute == pAISMOBRoute )
        {
            int ret = OCPNMessageBox( NULL, _("You are trying to delete an active AIS MOB route, are you REALLY sure?"), _("OpenCPN Warning"), wxYES_NO );
            
            if( ret == wxID_NO )
                return false;
            else
                pAISMOBRoute = NULL;
        }
        ::wxBeginBusyCursor();

        if( GetpActiveRoute() == pRoute ) DeactivateRoute();

        if( pRoute->m_bIsInLayer )
            return false;
            
        pConfig->DeleteConfigRoute( pRoute );

        //    Remove the route from associated lists
        pSelect->DeleteAllSelectableRouteSegments( pRoute );
        pRouteList->DeleteObject( pRoute );

        // walk the route, tentatively deleting/marking points used only by this route
        wxRoutePointListNode *pnode = ( pRoute->pRoutePointList )->GetFirst();
        while( pnode ) {
            RoutePoint *prp = pnode->GetData();

            // check all other routes to see if this point appears in any other route
            Route *pcontainer_route = FindRouteContainingWaypoint( prp );

            if( pcontainer_route == NULL && prp->m_bIsInRoute ) {
                prp->m_bIsInRoute = false;          // Take this point out of this (and only) route
                if( !prp->m_bKeepXRoute ) {
//    This does not need to be done with navobj.xml storage, since the waypoints are stored with the route
//                              pConfig->DeleteWayPoint(prp);

                    pSelect->DeleteSelectablePoint( prp, SELTYPE_ROUTEPOINT );

                    // Remove all instances of this point from the list.
                    wxRoutePointListNode *pdnode = pnode;
                    while( pdnode ) {
                        pRoute->pRoutePointList->DeleteNode( pdnode );
                        pdnode = pRoute->pRoutePointList->Find( prp );
                    }

                    pnode = NULL;
                    delete prp;
                } else {
                    prp->m_bDynamicName = false;
                    prp->m_bIsolatedMark = true;        // This has become an isolated mark
                    prp->m_bKeepXRoute = false;         // and is no longer part of a route
                }

            }
            if( pnode ) pnode = pnode->GetNext();
            else
                pnode = pRoute->pRoutePointList->GetFirst();                // restart the list
        }

        delete pRoute;

        ::wxEndBusyCursor();

    }
    return true;
}

void Routeman::DeleteAllRoutes( void )
{
    ::wxBeginBusyCursor();

    //    Iterate on the RouteList
    wxRouteListNode *node = pRouteList->GetFirst();
    while( node ) {
        Route *proute = node->GetData();
        if( proute == pAISMOBRoute )
        {
            ::wxEndBusyCursor();
            int ret = OCPNMessageBox( NULL, _("You are trying to delete an active AIS MOB route, are you REALLY sure?"), _("OpenCPN Warning"), wxYES_NO );
            if( ret == wxID_NO )
                return;
            else
                pAISMOBRoute = NULL;
            ::wxBeginBusyCursor();
        }

        node = node->GetNext();
        if( proute->m_bIsInLayer )
            continue;

        pConfig->m_bSkipChangeSetUpdate = true;
        pConfig->DeleteConfigRoute( proute );
        DeleteRoute( proute );
        pConfig->m_bSkipChangeSetUpdate = false;
    }

    ::wxEndBusyCursor();

}

void Routeman::DeleteAllTracks( void )
{
    ::wxBeginBusyCursor();

    //    Iterate on the RouteList
    wxTrackListNode *node = pTrackList->GetFirst();
    while( node ) {
        Track *ptrack = node->GetData();
            node = node->GetNext();

        if( ptrack->m_bIsInLayer )
            continue;

        g_pAIS->DeletePersistentTrack( ptrack );
        pConfig->m_bSkipChangeSetUpdate = true;
        pConfig->DeleteConfigTrack( ptrack );
        DeleteTrack( ptrack );
        pConfig->m_bSkipChangeSetUpdate = false;
    }

    ::wxEndBusyCursor();

}

void Routeman::DeleteTrack( Track *pTrack )
{
    if( pTrack ) {
        if( pTrack->m_bIsInLayer ) return;

        ::wxBeginBusyCursor();

        wxProgressDialog *pprog = NULL;

        int count = pTrack->GetnPoints();
        if( count > 10000) {
            pprog = new wxProgressDialog( _("OpenCPN Track Delete"), _T("0/0"), count, NULL, 
                                          wxPD_APP_MODAL | wxPD_SMOOTH |
                                          wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );
            pprog->SetSize( 400, wxDefaultCoord );
            pprog->Centre();
            
        }

        //    Remove the track from associated lists
        pSelect->DeleteAllSelectableTrackSegments( pTrack );
        pTrackList->DeleteObject( pTrack );

#if 0
        // walk the track, deleting points used by this track
        int ic = 0;
        wxTrackPointListNode *pnode = ( pTrack->pTrackPointList )->GetFirst();
        while( pnode )
        {
            if(pprog)
            {
                wxString msg;
                msg.Printf(_T("%d/%d"), ic, count);
                if(ic % 100 == 0)
                   pprog->Update( ic, msg );
                ic++;
            }

            TrackPoint *prp = pnode->GetData();
            delete prp;

            pnode = pnode->GetNext();
        }
#endif
        if( pTrack == g_pActiveTrack ) {
            g_pActiveTrack = NULL;
            m_pparent_app->TrackOff();
        }

        delete pTrack;

        ::wxEndBusyCursor();

        if( pprog)
            delete pprog;
    }
}

void Routeman::SetColorScheme( ColorScheme cs )
{
    // Re-Create the pens and colors
    
    int scaled_line_width = g_route_line_width;
    if(g_btouch){
        double size_mult =  g_ChartScaleFactorExp * 1.5;
        double sline_width = wxRound(size_mult * scaled_line_width);
        scaled_line_width = wxMax( sline_width, 1);
    }

    m_pActiveRoutePointPen = wxThePenList->FindOrCreatePen( wxColour( 0, 0, 255 ),
                                                            scaled_line_width, wxPENSTYLE_SOLID );
    m_pRoutePointPen = wxThePenList->FindOrCreatePen( wxColour( 0, 0, 255 ), scaled_line_width,
            wxPENSTYLE_SOLID );

//    Or in something like S-52 compliance

    m_pRoutePen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T("UINFB") ), scaled_line_width,
            wxPENSTYLE_SOLID );
    m_pSelectedRoutePen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T("UINFO") ),
                                                         scaled_line_width, wxPENSTYLE_SOLID );
    m_pActiveRoutePen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T("UARTE") ),
                                                       scaled_line_width, wxPENSTYLE_SOLID );
    m_pTrackPen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T("CHMGD") ), scaled_line_width,
                                                 wxPENSTYLE_SOLID );
    
    m_pRouteBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("UINFB") ), wxBRUSHSTYLE_SOLID );
    m_pSelectedRouteBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("UINFO") ),
            wxBRUSHSTYLE_SOLID );
    m_pActiveRouteBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("PLRTE") ),
            wxBRUSHSTYLE_SOLID );

}

wxString Routeman::GetRouteReverseMessage( void )
{
    return wxString(
            _("Waypoints can be renamed to reflect the new order, the names will be '001', '002' etc.\n\nDo you want to rename the waypoints?") );
}

Route *Routeman::FindRouteByGUID(const wxString &guid)
{
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        
        if( pRoute->m_GUID == guid )
            return pRoute;
        node1 = node1->GetNext();
    }
 
    return NULL;
}

Track *Routeman::FindTrackByGUID(const wxString &guid)
{
    wxTrackListNode *node1 = pTrackList->GetFirst();
    while( node1 ) {
        Track *pTrack = node1->GetData();
        
        if( pTrack->m_GUID == guid )
            return pTrack;
        node1 = node1->GetNext();
    }
 
    return NULL;
}

void Routeman::ZeroCurrentXTEToActivePoint()
{
    // When zeroing XTE create a "virtual" waypoint at present position
    if( pRouteActivatePoint ) delete pRouteActivatePoint;
    pRouteActivatePoint = new RoutePoint( gLat, gLon, wxString( _T("") ), wxString( _T("") ),
    GPX_EMPTY_STRING, false ); // Current location
    pRouteActivatePoint->m_bShowName = false;

    pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
    m_arrival_min = 1e6;
}

//--------------------------------------------------------------------------------
//      WayPointman   Implementation
//--------------------------------------------------------------------------------

WayPointman::WayPointman()
{

    m_pWayPointList = new RoutePointList;

    pmarkicon_image_list = NULL;

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    m_pIconArray = new wxArrayPtrVoid();
    ProcessIcons( style );

    m_nGUID = 0;
}

WayPointman::~WayPointman()
{
    //    Two step here, since the RoutePoint dtor also touches the
    //    RoutePoint list.
    //    Copy the master RoutePoint list to a temporary list,
    //    then clear and delete objects from the temp list

    RoutePointList temp_list;

    wxRoutePointListNode *node = m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *pr = node->GetData();

        temp_list.Append( pr );
        node = node->GetNext();
    }

    temp_list.DeleteContents( true );
    temp_list.Clear();

    m_pWayPointList->Clear();
    delete m_pWayPointList;

    for( unsigned int i = 0; i < m_pIconArray->GetCount(); i++ ) {
        MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( i );
        delete pmi->picon_bitmap;
        delete pmi;
    }

    m_pIconArray->Clear();
    delete m_pIconArray;

    if( pmarkicon_image_list ) pmarkicon_image_list->RemoveAll();
    delete pmarkicon_image_list;
}

bool WayPointman::AddRoutePoint(RoutePoint *prp)
{
    if(!prp)
        return false;
    
    wxRoutePointListNode *prpnode = m_pWayPointList->Append(prp);
    prp->SetManagerListNode( prpnode );
    
    return true;
}

bool WayPointman::RemoveRoutePoint(RoutePoint *prp)
{
    if(!prp)
        return false;
    
    wxRoutePointListNode *prpnode = (wxRoutePointListNode *)prp->GetManagerListNode();
    
    if(prpnode) 
        delete prpnode;
    else
        m_pWayPointList->DeleteObject(prp);
    
    prp->SetManagerListNode( NULL );
    
    return true;
}

void WayPointman::ProcessUserIcons( ocpnStyle::Style* style )
{
    wxString UserIconPath = g_Platform->GetPrivateDataDir();
    wxChar sep = wxFileName::GetPathSeparator();
    if( UserIconPath.Last() != sep ) UserIconPath.Append( sep );
    UserIconPath.Append( _T("UserIcons") );
    
    wxLogMessage(_T("Looking for UserIcons at ") + UserIconPath );
    
    if( wxDir::Exists( UserIconPath ) ) {
        wxLogMessage(_T("Loading UserIcons from ") + UserIconPath );
        wxArrayString FileList;
        
        wxDir dir( UserIconPath );
        int n_files = dir.GetAllFiles( UserIconPath, &FileList );
        
        for( int ifile = 0; ifile < n_files; ifile++ ) {
            wxString name = FileList.Item( ifile );
            
            wxFileName fn( name );
            wxString iconname = fn.GetName();
            wxBitmap icon1;
            
            if( fn.GetExt().Lower() == _T("xpm") ) {
                if( icon1.LoadFile( name, wxBITMAP_TYPE_XPM ) ) {
                    ProcessIcon( icon1, iconname, iconname );
                }
            }
            if( fn.GetExt().Lower() == _T("png") ) {
                if( icon1.LoadFile( name, wxBITMAP_TYPE_PNG ) ) {
                    ProcessIcon( icon1, iconname, iconname );
                }
            }
        }
    }
}


void WayPointman::ProcessIcons( ocpnStyle::Style* style )
{
    ProcessIcon( style->GetIcon( _T("empty") ), _T("empty"), _T("Empty") );
    ProcessIcon( style->GetIcon( _T("airplane") ), _T("airplane"), _T("Airplane") );
    ProcessIcon( style->GetIcon( _T("anchorage") ), _T("anchorage"), _T("Anchorage") );
    ProcessIcon( style->GetIcon( _T("anchor") ), _T("anchor"), _T("Anchor") );
    ProcessIcon( style->GetIcon( _T("boarding") ), _T("boarding"), _T("Boarding Location") );
    ProcessIcon( style->GetIcon( _T("boundary") ), _T("boundary"), _T("Boundary Mark") );
    ProcessIcon( style->GetIcon( _T("bouy1") ), _T("bouy1"), _T("Bouy Type A") );
    ProcessIcon( style->GetIcon( _T("bouy2") ), _T("bouy2"), _T("Bouy Type B") );
    ProcessIcon( style->GetIcon( _T("campfire") ), _T("campfire"), _T("Campfire") );
    ProcessIcon( style->GetIcon( _T("camping") ), _T("camping"), _T("Camping Spot") );
    ProcessIcon( style->GetIcon( _T("coral") ), _T("coral"), _T("Coral") );
    ProcessIcon( style->GetIcon( _T("fishhaven") ), _T("fishhaven"), _T("Fish Haven") );
    ProcessIcon( style->GetIcon( _T("fishing") ), _T("fishing"), _T("Fishing Spot") );
    ProcessIcon( style->GetIcon( _T("fish") ), _T("fish"), _T("Fish") );
    ProcessIcon( style->GetIcon( _T("float") ), _T("float"), _T("Float") );
    ProcessIcon( style->GetIcon( _T("food") ), _T("food"), _T("Food") );
    ProcessIcon( style->GetIcon( _T("fuel") ), _T("fuel"), _T("Fuel") );
    ProcessIcon( style->GetIcon( _T("greenlite") ), _T("greenlite"), _T("Green Light") );
    ProcessIcon( style->GetIcon( _T("kelp") ), _T("kelp"), _T("Kelp") );
    ProcessIcon( style->GetIcon( _T("light") ), _T("light1"), _T("Light Type A") );
    ProcessIcon( style->GetIcon( _T("light1") ), _T("light"), _T("Light Type B") );
    ProcessIcon( style->GetIcon( _T("litevessel") ), _T("litevessel"), _T("Light Vessel") );
    ProcessIcon( style->GetIcon( _T("mob") ), _T("mob"), _T("MOB") );
    ProcessIcon( style->GetIcon( _T("mooring") ), _T("mooring"), _T("Mooring Bouy") );
    ProcessIcon( style->GetIcon( _T("oilbouy") ), _T("oilbouy"), _T("Oil Bouy") );
    ProcessIcon( style->GetIcon( _T("platform") ), _T("platform"), _T("Platform") );
    ProcessIcon( style->GetIcon( _T("redgreenlite") ), _T("redgreenlite"), _T("Red/Green Light") );
    ProcessIcon( style->GetIcon( _T("redlite") ), _T("redlite"), _T("Red Light") );
    ProcessIcon( style->GetIcon( _T("rock1") ), _T("rock1"), _T("Rock (exposed)") );
    ProcessIcon( style->GetIcon( _T("rock2") ), _T("rock2"), _T("Rock, (awash)") );
    ProcessIcon( style->GetIcon( _T("sand") ), _T("sand"), _T("Sand") );
    ProcessIcon( style->GetIcon( _T("scuba") ), _T("scuba"), _T("Scuba") );
    ProcessIcon( style->GetIcon( _T("shoal") ), _T("shoal"), _T("Shoal") );
    ProcessIcon( style->GetIcon( _T("snag") ), _T("snag"), _T("Snag") );
    ProcessIcon( style->GetIcon( _T("square") ), _T("square"), _T("Square") );
    ProcessIcon( style->GetIcon( _T("triangle") ), _T("triangle"), _T("Triangle") );
    ProcessIcon( style->GetIcon( _T("diamond") ), _T("diamond"), _T("Diamond") );
    ProcessIcon( style->GetIcon( _T("circle") ), _T("circle"), _T("Circle") );
    ProcessIcon( style->GetIcon( _T("wreck1") ), _T("wreck1"), _T("Wreck A") );
    ProcessIcon( style->GetIcon( _T("wreck2") ), _T("wreck2"), _T("Wreck B") );
    ProcessIcon( style->GetIcon( _T("xmblue") ), _T("xmblue"), _T("Blue X") );
    ProcessIcon( style->GetIcon( _T("xmgreen") ), _T("xmgreen"), _T("Green X") );
    ProcessIcon( style->GetIcon( _T("xmred") ), _T("xmred"), _T("Red X") );
    ProcessIcon( style->GetIcon( _T("activepoint") ), _T("activepoint"), _T("Active WP") );
    
    // Load user defined icons.
    // Done after default icons are initialized,
    // so that user may substitute an icon by using the same name in the Usericons file.
    ProcessUserIcons( style );
    
}

void WayPointman::ProcessIcon(wxBitmap pimage, const wxString & key, const wxString & description)
{
    MarkIcon *pmi;

    bool newIcon = true;

    // avoid adding duplicates
    for( unsigned int i = 0; i < m_pIconArray->GetCount(); i++ ) {
        pmi = (MarkIcon *) m_pIconArray->Item( i );
        if( pmi->icon_name.IsSameAs( key ) ) {
            newIcon = false;
            delete pmi->picon_bitmap;
            break;
        }
    }

    if( newIcon ) {
        pmi = new MarkIcon;
        m_pIconArray->Add( (void *) pmi );
    }

    pmi->icon_name = key;
    pmi->icon_description = description;
    pmi->picon_bitmap = new wxBitmap( pimage );
    pmi->icon_texture = 0; /* invalidate */
}

wxImageList *WayPointman::Getpmarkicon_image_list( void )
{
    // First find the largest bitmap size
    int w = 0;
    int h = 0;

    MarkIcon *pmi;

    for( unsigned int i = 0; i < m_pIconArray->GetCount(); i++ ) {
        pmi = (MarkIcon *) m_pIconArray->Item( i );
        w = wxMax(w, pmi->picon_bitmap->GetWidth());
        h = wxMax(h, pmi->picon_bitmap->GetHeight());

        // toh, 10.09.29
        // User defined icons won't be displayed in the list if they are larger than 32x32 pixels (why???)
        // Work-around: limit size
        if( w > 32 ) w = 32;
        if( h > 32 ) h = 32;

    }

    // Build an image list large enough

    if( NULL != pmarkicon_image_list ) {
        pmarkicon_image_list->RemoveAll();
        delete pmarkicon_image_list;
    }
    pmarkicon_image_list = new wxImageList( w, h );

    // Add the icons
    for( unsigned int ii = 0; ii < m_pIconArray->GetCount(); ii++ ) {
        pmi = (MarkIcon *) m_pIconArray->Item( ii );
        wxImage icon_image = pmi->picon_bitmap->ConvertToImage();

        // toh, 10.09.29
        // After limiting size user defined icons will be cut off
        // Work-around: rescale in one or both directions
        int h0 = icon_image.GetHeight();
        int w0 = icon_image.GetWidth();

        wxImage icon_larger;
        if( h0 <= h && w0 <= w ) {
            // Resize & Center smaller icons in the bitmap, so menus won't look so weird.
            icon_larger = icon_image.Resize( wxSize( w, h ), wxPoint( (w-w0)/2, (h-h0)/2 ) );
        } else {
            // rescale in one or two directions to avoid cropping, then resize to fit to cell
            int h1 = h;
            int w1 = w;
            if( h0 > h ) w1 = wxRound( (double) w0 * ( (double) h / (double) h0 ) );

            else if( w0 > w ) h1 = wxRound( (double) h0 * ( (double) w / (double) w0 ) );

            icon_larger = icon_image.Rescale( w1, h1 );
            icon_larger = icon_larger.Resize( wxSize( w, h ), wxPoint( 0, 0 ) );
        }

        pmarkicon_image_list->Add( icon_larger );
    }
    
    m_markicon_image_list_base_count = pmarkicon_image_list->GetImageCount(); 

    // Create and add "x-ed out" icons,
    // Being careful to preserve (some) transparency
    for( unsigned int ii = 0; ii < m_pIconArray->GetCount(); ii++ ) {

        wxImage img = pmarkicon_image_list->GetBitmap( ii ).ConvertToImage() ;
        img.ConvertAlphaToMask( 128 );

        unsigned char r,g,b;
        img.GetOrFindMaskColour(&r, &g, &b);
        wxColour unused_color(r,g,b);

        wxBitmap bmp0( img );
    
        wxBitmap bmp(w, h, -1 );
        wxMemoryDC mdc( bmp );
        mdc.SetBackground( wxBrush( unused_color) );
        mdc.Clear();
        mdc.DrawBitmap( bmp0, 0, 0 );
        wxPen red(GetGlobalColor(_T( "URED" )), 2 );
        mdc.SetPen( red );
        int xm = bmp.GetWidth();
        int ym = bmp.GetHeight();
        mdc.DrawLine( 2, 2, xm-2, ym-2 );
        mdc.DrawLine( xm-2, 2, 2, ym-2 );
        mdc.SelectObject( wxNullBitmap );
        
        wxMask *pmask = new wxMask(bmp, unused_color);
        bmp.SetMask( pmask );

        pmarkicon_image_list->Add( bmp );
    }
        
        
        
    return pmarkicon_image_list;
}

wxBitmap *WayPointman::CreateDimBitmap( wxBitmap *pBitmap, double factor )
{
    wxImage img = pBitmap->ConvertToImage();
    int sx = img.GetWidth();
    int sy = img.GetHeight();

    wxImage new_img( img );

    for( int i = 0; i < sx; i++ ) {
        for( int j = 0; j < sy; j++ ) {
            if( !img.IsTransparent( i, j ) ) {
                new_img.SetRGB( i, j, (unsigned char) ( img.GetRed( i, j ) * factor ),
                        (unsigned char) ( img.GetGreen( i, j ) * factor ),
                        (unsigned char) ( img.GetBlue( i, j ) * factor ) );
            }
        }
    }

    wxBitmap *pret = new wxBitmap( new_img );

    return pret;

}

void WayPointman::SetColorScheme( ColorScheme cs )
{
    ProcessIcons( g_StyleManager->GetCurrentStyle() );
    
    //    Iterate on the RoutePoint list, requiring each to reload icon

    wxRoutePointListNode *node = m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *pr = node->GetData();
        pr->ReLoadIcon();
        node = node->GetNext();
    }
}

bool WayPointman::DoesIconExist(const wxString & icon_key) const
{
    MarkIcon *pmi;
    unsigned int i;

    for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
        pmi = (MarkIcon *) m_pIconArray->Item( i );
        if( pmi->icon_name.IsSameAs( icon_key ) ) return true;
    }

    return false;
}

wxBitmap *WayPointman::GetIconBitmap( const wxString& icon_key )
{
    wxBitmap *pret = NULL;
    MarkIcon *pmi = NULL;
    unsigned int i;

    for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
        pmi = (MarkIcon *) m_pIconArray->Item( i );
        if( pmi->icon_name.IsSameAs( icon_key ) )
            break;
    }

    if( i == m_pIconArray->GetCount() )              // key not found
    {
        // find and return bitmap for "circle"
        for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
            pmi = (MarkIcon *) m_pIconArray->Item( i );
//            if( pmi->icon_name.IsSameAs( _T("circle") ) )
//                break;
        }
    }

    if( i == m_pIconArray->GetCount() )              // "circle" not found
        pmi = (MarkIcon *) m_pIconArray->Item( 0 );       // use item 0

    if( pmi )
        pret = pmi->picon_bitmap;

    return pret;
}

unsigned int WayPointman::GetIconTexture( const wxBitmap *pbm, int &glw, int &glh )
{
#ifdef ocpnUSE_GL
    int index = GetIconIndex( pbm );
    MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( index );

    if(!pmi->icon_texture) {
        /* make rgba texture */       
        glGenTextures(1, &pmi->icon_texture);
        glBindTexture(GL_TEXTURE_2D, pmi->icon_texture);
                
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        
        wxImage image = pbm->ConvertToImage();
        int w = image.GetWidth(), h = image.GetHeight();
        
        pmi->tex_w = NextPow2(w);
        pmi->tex_h = NextPow2(h);
        
        unsigned char *d = image.GetData();
        unsigned char *a = image.GetAlpha();
            
        unsigned char mr, mg, mb;
        image.GetOrFindMaskColour( &mr, &mg, &mb );
    
        unsigned char *e = new unsigned char[4 * w * h];
        if(d && e){
            for( int y = 0; y < h; y++ )
                for( int x = 0; x < w; x++ ) {
                    unsigned char r, g, b;
                    int off = ( y * image.GetWidth() + x );
                    r = d[off * 3 + 0];
                    g = d[off * 3 + 1];
                    b = d[off * 3 + 2];
                    
                    e[off * 4 + 0] = r;
                    e[off * 4 + 1] = g;
                    e[off * 4 + 2] = b;
                    
                    e[off * 4 + 3] =  a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                }
        }
    
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pmi->tex_w, pmi->tex_h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                        GL_RGBA, GL_UNSIGNED_BYTE, e);

        delete [] e;
    }

    glw = pmi->tex_w;
    glh = pmi->tex_h;

    return pmi->icon_texture;
#else
    return 0;
#endif
}

wxBitmap *WayPointman::GetIconBitmap( int index )
{
    wxBitmap *pret = NULL;

    if( index >= 0 ) {
        MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( index );
        pret = pmi->picon_bitmap;
    }
    return pret;
}

wxString *WayPointman::GetIconDescription( int index )
{
    wxString *pret = NULL;

    if( index >= 0 ) {
        MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( index );
        pret = &pmi->icon_description;
    }
    return pret;
}

wxString *WayPointman::GetIconKey( int index )
{
    wxString *pret = NULL;

    if( (index >= 0)  && ((unsigned int)index < m_pIconArray->GetCount()) ) {
        MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( index );
        pret = &pmi->icon_name;
    }
    return pret;
}

int WayPointman::GetIconIndex( const wxBitmap *pbm )
{
    unsigned int i;

    for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
        MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( i );
        if( pmi->picon_bitmap == pbm ) break;
    }

    return i;                                           // index of base icon in the image list

}

int WayPointman::GetXIconIndex( const wxBitmap *pbm )
{
    unsigned int i;
    
    for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
        MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( i );
        if( pmi->picon_bitmap == pbm ) break;
    }
    
    return i + m_markicon_image_list_base_count;        // index of "X-ed out" icon in the image list
    
}

//  Create the unique identifier

wxString WayPointman::CreateGUID( RoutePoint *pRP )
{
    //FIXME: this method is not needed at all (if GetUUID works...)
    /*wxDateTime now = wxDateTime::Now();
     time_t ticks = now.GetTicks();
     wxString GUID;
     GUID.Printf(_T("%d-%d-%d-%d"), ((int)fabs(pRP->m_lat * 1e4)), ((int)fabs(pRP->m_lon * 1e4)), (int)ticks, m_nGUID);

     m_nGUID++;

     return GUID;*/
    return GpxDocument::GetUUID();
}

RoutePoint *WayPointman::FindRoutePointByGUID(const wxString &guid)
{
    wxRoutePointListNode *prpnode = m_pWayPointList->GetFirst();
    while( prpnode ) {
        RoutePoint *prp = prpnode->GetData();

        if( prp->m_GUID == guid ) return ( prp );

        prpnode = prpnode->GetNext(); //RoutePoint
    }

    return NULL;
}

RoutePoint *WayPointman::GetNearbyWaypoint( double lat, double lon, double radius_meters )
{
    //    Iterate on the RoutePoint list, checking distance

    wxRoutePointListNode *node = m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *pr = node->GetData();

        double a = lat - pr->m_lat;
        double b = lon - pr->m_lon;
        double l = sqrt( ( a * a ) + ( b * b ) );

        if( ( l * 60. * 1852. ) < radius_meters ) return pr;

        node = node->GetNext();
    }
    return NULL;

}

RoutePoint *WayPointman::GetOtherNearbyWaypoint( double lat, double lon, double radius_meters,
        const wxString &guid )
{
    //    Iterate on the RoutePoint list, checking distance

    wxRoutePointListNode *node = m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *pr = node->GetData();

        double a = lat - pr->m_lat;
        double b = lon - pr->m_lon;
        double l = sqrt( ( a * a ) + ( b * b ) );

        if( ( l * 60. * 1852. ) < radius_meters ) if( pr->m_GUID != guid ) return pr;

        node = node->GetNext();
    }
    return NULL;

}

void WayPointman::ClearRoutePointFonts( void )
{
    //    Iterate on the RoutePoint list, clearing Font pointers
    //    This is typically done globally after a font switch

    wxRoutePointListNode *node = m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *pr = node->GetData();

        pr->m_pMarkFont = NULL;
        node = node->GetNext();
    }
}

bool WayPointman::SharedWptsExist()
{
    wxRoutePointListNode *node = m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *prp = node->GetData();
        if (prp->m_bKeepXRoute && ( prp->m_bIsInRoute || prp == pAnchorWatchPoint1 || prp == pAnchorWatchPoint2))
            return true;
        node = node->GetNext();
    }
    return false;
}

void WayPointman::DeleteAllWaypoints( bool b_delete_used )
{
    //    Iterate on the RoutePoint list, deleting all
    wxRoutePointListNode *node = m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *prp = node->GetData();
        // if argument is false, then only delete non-route waypoints
        if( !prp->m_bIsInLayer && ( prp->GetIconName() != _T("mob") )
            && ( ( b_delete_used && prp->m_bKeepXRoute )
                        || ( ( !prp->m_bIsInRoute )
                                && !( prp == pAnchorWatchPoint1 ) && !( prp == pAnchorWatchPoint2 ) ) ) ) {
            DestroyWaypoint(prp);
            delete prp;
            node = m_pWayPointList->GetFirst();
        } else
            node = node->GetNext();
    }
    return;

}

void WayPointman::DestroyWaypoint( RoutePoint *pRp, bool b_update_changeset )
{
    if( ! b_update_changeset )
        pConfig->m_bSkipChangeSetUpdate = true;             // turn OFF change-set updating if requested
        
    if( pRp ) {
        // Get a list of all routes containing this point
        // and remove the point from them all
        wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( pRp );
        if( proute_array ) {
            for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
                Route *pr = (Route *) proute_array->Item( ir );

                /*  FS#348
                 if ( g_pRouteMan->GetpActiveRoute() == pr )            // Deactivate any route containing this point
                 g_pRouteMan->DeactivateRoute();
                 */
                pr->RemovePoint( pRp );

            }

            //    Scrub the routes, looking for one-point routes
            for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
                Route *pr = (Route *) proute_array->Item( ir );
                if( pr->GetnPoints() < 2 ) {
                    bool prev_bskip = pConfig->m_bSkipChangeSetUpdate;
                    pConfig->m_bSkipChangeSetUpdate = true;
                    pConfig->DeleteConfigRoute( pr );
                    g_pRouteMan->DeleteRoute( pr );
                    pConfig->m_bSkipChangeSetUpdate = prev_bskip;
                }
            }

            delete proute_array;
        }

        // Now it is safe to delete the point
        pConfig->DeleteWayPoint( pRp );
        pConfig->m_bSkipChangeSetUpdate = false;
        
        pSelect->DeleteSelectableRoutePoint( pRp );

        //    The RoutePoint might be currently in use as an anchor watch point
        if( pRp == pAnchorWatchPoint1 ) pAnchorWatchPoint1 = NULL;
        if( pRp == pAnchorWatchPoint2 ) pAnchorWatchPoint2 = NULL;

        RemoveRoutePoint( pRp);

    }
}
