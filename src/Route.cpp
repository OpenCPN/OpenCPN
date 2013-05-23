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

#include "wx/wxprec.h"

#include "Route.h"
#include "georef.h"
#include "routeman.h"
#include "ocpndc.h"
#include "cutil.h"
#include "navutil.h"
#include "multiplexer.h"
#include "Select.h"

extern WayPointman *pWayPointMan;
extern bool g_bIsNewLayer;
extern int g_LayerIdx;
extern Routeman *g_pRouteMan;
extern int g_route_line_width;
extern Select *pSelect;
extern MyConfig *pConfig;
extern Multiplexer *g_pMUX;

#include <wx/listimpl.cpp>
WX_DEFINE_LIST ( RouteList );

Route::Route( void )
{
    m_bRtIsSelected = false;
    m_bRtIsActive = false;
    m_pRouteActivePoint = NULL;
    m_bIsBeingEdited = false;
    m_bIsBeingCreated = false;
    m_bIsTrack = false;
    m_nPoints = 0;
    m_nm_sequence = 1;
    m_route_length = 0.0;
    m_route_time = 0.0;
    m_bVisible = true;
    m_bListed = true;
    m_bDeleteOnArrival = false;
    m_width = STYLE_UNDEFINED;
    m_style = STYLE_UNDEFINED;

    pRoutePointList = new RoutePointList;
    m_pLastAddedPoint = NULL;
    m_GUID = pWayPointMan->CreateGUID( NULL );
    m_btemp = false;
    
    m_ArrivalRadius = .05;        // default, Miles

    RBBox.Reset();
    m_bcrosses_idl = false;

    m_LayerID = 0;
    m_bIsInLayer = false;

    m_Colour = wxEmptyString;

    m_lastMousePointIndex = 0;
    m_NextLegGreatCircle = false;
}

Route::~Route( void )
{
    pRoutePointList->DeleteContents( false );            // do not delete Marks
    pRoutePointList->Clear();
    delete pRoutePointList;
}

// The following is used only for route splitting, assumes just created, empty route
//
void Route::CloneRoute( Route *psourceroute, int start_nPoint, int end_nPoint, const wxString & suffix)
{
    m_bIsTrack = psourceroute->m_bIsTrack;

    m_RouteNameString = psourceroute->m_RouteNameString + suffix;
    m_RouteStartString = psourceroute->m_RouteStartString;
    m_RouteEndString = psourceroute->m_RouteEndString;

    int i;
    for( i = start_nPoint; i <= end_nPoint; i++ ) {
        if( !psourceroute->m_bIsInLayer ) AddPoint( psourceroute->GetPoint( i ), false );
        else {
            RoutePoint *psourcepoint = psourceroute->GetPoint( i );
            RoutePoint *ptargetpoint = new RoutePoint( psourcepoint->m_lat, psourcepoint->m_lon,
                    psourcepoint->m_IconName, psourcepoint->GetName(), GPX_EMPTY_STRING, false );

            AddPoint( ptargetpoint, false );

            CloneAddedRoutePoint( m_pLastAddedPoint, psourcepoint );
        }
    }

    CalculateBBox();

}

void Route::CloneTrack( Route *psourceroute, int start_nPoint, int end_nPoint, const wxString & suffix)
{
    if( psourceroute->m_bIsInLayer ) return;

    m_bIsTrack = psourceroute->m_bIsTrack;

    m_RouteNameString = psourceroute->m_RouteNameString + suffix;
    m_RouteStartString = psourceroute->m_RouteStartString;
    m_RouteEndString = psourceroute->m_RouteEndString;

    bool b_splitting = GetnPoints() == 0;

    int startTrkSegNo;
    if( b_splitting ) startTrkSegNo = psourceroute->GetPoint( start_nPoint )->m_GPXTrkSegNo;
    else
        startTrkSegNo = this->GetLastPoint()->m_GPXTrkSegNo;

    int i;
    for( i = start_nPoint; i <= end_nPoint; i++ ) {

        RoutePoint *psourcepoint = psourceroute->GetPoint( i );
        RoutePoint *ptargetpoint = new RoutePoint( psourcepoint->m_lat, psourcepoint->m_lon,
                psourcepoint->m_IconName, psourcepoint->GetName(), GPX_EMPTY_STRING, false );

        AddPoint( ptargetpoint, false );

        CloneAddedTrackPoint( m_pLastAddedPoint, psourcepoint );

        int segment_shift = psourcepoint->m_GPXTrkSegNo;

        if( ( start_nPoint == 2 ) /*&& (psourcepoint->m_GPXTrkSegNo == startTrkSegNo)*/)
			segment_shift = psourcepoint->m_GPXTrkSegNo - 1; // continue first segment if tracks share the first point

        if( b_splitting )
			m_pLastAddedPoint->m_GPXTrkSegNo = ( psourcepoint->m_GPXTrkSegNo - startTrkSegNo ) + 1;
        else
            m_pLastAddedPoint->m_GPXTrkSegNo = startTrkSegNo + segment_shift;
    }

    CalculateBBox();

}

void Route::CloneAddedRoutePoint( RoutePoint *ptargetpoint, RoutePoint *psourcepoint )
{
    ptargetpoint->m_MarkDescription = psourcepoint->m_MarkDescription;
    ptargetpoint->m_bKeepXRoute = psourcepoint->m_bKeepXRoute;
    ptargetpoint->m_bIsVisible = psourcepoint->m_bIsVisible;
    ptargetpoint->m_bPtIsSelected = false;
    ptargetpoint->m_pbmIcon = psourcepoint->m_pbmIcon;
    ptargetpoint->m_bShowName = psourcepoint->m_bShowName;
    ptargetpoint->m_bBlink = psourcepoint->m_bBlink;
    ptargetpoint->m_bBlink = psourcepoint->m_bDynamicName;
    ptargetpoint->CurrentRect_in_DC = psourcepoint->CurrentRect_in_DC;
    ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetX;
    ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetY;
    ptargetpoint->SetCreateTime(psourcepoint->GetCreateTime());
    ptargetpoint->m_HyperlinkList = new HyperlinkList;

    if( !psourcepoint->m_HyperlinkList->IsEmpty() ) {
        HyperlinkList::iterator iter = psourcepoint->m_HyperlinkList->begin();
        psourcepoint->m_HyperlinkList->splice( iter, *( ptargetpoint->m_HyperlinkList ) );
    }
}

void Route::CloneAddedTrackPoint( RoutePoint *ptargetpoint, RoutePoint *psourcepoint )
{
    //    This is a hack, need to undo the action of Route::AddPoint
    ptargetpoint->m_bIsInRoute = false;
    ptargetpoint->m_bIsInTrack = true;
    ptargetpoint->m_MarkDescription = psourcepoint->m_MarkDescription;
    ptargetpoint->m_bKeepXRoute = psourcepoint->m_bKeepXRoute;
    ptargetpoint->m_bIsVisible = psourcepoint->m_bIsVisible;
    ptargetpoint->m_bPtIsSelected = false;
    ptargetpoint->m_pbmIcon = psourcepoint->m_pbmIcon;
    ptargetpoint->m_bShowName = psourcepoint->m_bShowName;
    ptargetpoint->m_bBlink = psourcepoint->m_bBlink;
    ptargetpoint->m_bBlink = psourcepoint->m_bDynamicName;
    ptargetpoint->CurrentRect_in_DC = psourcepoint->CurrentRect_in_DC;
    ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetX;
    ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetY;
    ptargetpoint->SetCreateTime(psourcepoint->GetCreateTime());
    ptargetpoint->m_HyperlinkList = new HyperlinkList;
    // Hyperlinks not implemented currently in GPX for trackpoints
    //if (!psourcepoint->m_HyperlinkList->IsEmpty()) {
    //      HyperlinkList::iterator iter = psourcepoint->m_HyperlinkList->begin();
    //      psourcepoint->m_HyperlinkList->splice(iter, *(ptargetpoint->m_HyperlinkList));
    //}
}

void Route::AddPoint( RoutePoint *pNewPoint, bool b_rename_in_sequence, bool b_deferBoxCalc )
{
    if( pNewPoint->m_bIsolatedMark ) {
        pNewPoint->m_bKeepXRoute = true;
    }
    pNewPoint->m_bIsolatedMark = false;       // definitely no longer isolated
    pNewPoint->m_bIsInRoute = true;

    pRoutePointList->Append( pNewPoint );

    m_nPoints++;

    if( !b_deferBoxCalc ) CalculateBBox();

    if( m_pLastAddedPoint ) pNewPoint->m_seg_len = DistGreatCircle( m_pLastAddedPoint->m_lat,
            m_pLastAddedPoint->m_lon, pNewPoint->m_lat, pNewPoint->m_lon );

    m_route_length += pNewPoint->m_seg_len;

    m_pLastAddedPoint = pNewPoint;

    if( b_rename_in_sequence && pNewPoint->GetName().IsEmpty() && !pNewPoint->m_bKeepXRoute ) {
        wxString name;
        name.Printf( _T ( "%03d" ), m_nPoints );
        pNewPoint->SetName( name );
        pNewPoint->m_bDynamicName = true;
    }
    return;
}

void Route::AddTentativePoint( const wxString& GUID )
{
    RoutePointGUIDList.Add( GUID );
    return;
}

RoutePoint *Route::GetPoint( int nWhichPoint )
{
    RoutePoint *prp;
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    int i = 1;
    while( node ) {
        prp = node->GetData();
        if( i == nWhichPoint ) return prp;

        i++;
        node = node->GetNext();
    }

    return ( NULL );
}

RoutePoint *Route::GetPoint( const wxString &guid )
{
    RoutePoint *prp;
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    while( node ) {
        prp = node->GetData();
        if( guid == prp->m_GUID ) return prp;

        node = node->GetNext();
    }

    return ( NULL );
}

void Route::DrawPointWhich( ocpnDC& dc, int iPoint, wxPoint *rpn )
{
    GetPoint( iPoint )->Draw( dc, rpn );
}

void Route::DrawSegment( ocpnDC& dc, wxPoint *rp1, wxPoint *rp2, ViewPort &VP, bool bdraw_arrow )
{
    if( m_bRtIsSelected ) dc.SetPen( *g_pRouteMan->GetSelectedRoutePen() );
    else
        if( m_bRtIsActive ) dc.SetPen( *g_pRouteMan->GetActiveRoutePen() );
        else
            dc.SetPen( *g_pRouteMan->GetRoutePen() );

    RenderSegment( dc, rp1->x, rp1->y, rp2->x, rp2->y, VP, bdraw_arrow );
}

void Route::Draw( ocpnDC& dc, ViewPort &VP )
{
    if( m_nPoints == 0 ) return;

    if( m_bVisible && m_bRtIsSelected ) {
        dc.SetPen( *g_pRouteMan->GetSelectedRoutePen() );
        dc.SetBrush( *g_pRouteMan->GetSelectedRouteBrush() );
    }
    else if ( m_bVisible )
    {
        int style = wxSOLID;
        int width = g_route_line_width;
        wxColour col;
        if( m_style != STYLE_UNDEFINED ) style = m_style;
        if( m_width != STYLE_UNDEFINED ) width = m_width;
        if( m_Colour == wxEmptyString ) {
            col = g_pRouteMan->GetRoutePen()->GetColour();
        } else {
            for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
                if( m_Colour == ::GpxxColorNames[i] ) {
                    col = ::GpxxColors[i];
                    break;
                }
            }
        }
        dc.SetPen( *wxThePenList->FindOrCreatePen( col, width, style ) );
        dc.SetBrush( *wxTheBrushList->FindOrCreateBrush( col, wxSOLID ) );
    }

    if( m_bVisible && m_bRtIsActive )
    {
        dc.SetPen( *g_pRouteMan->GetActiveRoutePen() );
        dc.SetBrush( *g_pRouteMan->GetActiveRouteBrush() );
    }

    wxPoint rpt1, rpt2;
    if ( m_bVisible )
        DrawPointWhich( dc, 1, &rpt1 );

    wxRoutePointListNode *node = pRoutePointList->GetFirst();
    RoutePoint *prp1 = node->GetData();
    node = node->GetNext();

    if ( !m_bVisible && prp1->m_bKeepXRoute )
            prp1->Draw( dc );

    while( node ) {

        RoutePoint *prp2 = node->GetData();
        if ( !m_bVisible && prp2->m_bKeepXRoute )
            prp2->Draw( dc );
        else if (m_bVisible)
            prp2->Draw( dc, &rpt2 );

        if ( m_bVisible )
        {
            //    Handle offscreen points
            bool b_2_on = VP.GetBBox().PointInBox( prp2->m_lon, prp2->m_lat, 0 );
            bool b_1_on = VP.GetBBox().PointInBox( prp1->m_lon, prp1->m_lat, 0 );

            //TODO This logic could be simpliifed
            //Simple case
            if( b_1_on && b_2_on ) RenderSegment( dc, rpt1.x, rpt1.y, rpt2.x, rpt2.y, VP, true ); // with arrows

            //    In the cases where one point is on, and one off
            //    we must decide which way to go in longitude
            //     Arbitrarily, we will go the shortest way

            double pix_full_circle = WGS84_semimajor_axis_meters * mercator_k0 * 2 * PI
                * VP.view_scale_ppm;
            double dp = pow( (double) ( rpt1.x - rpt2.x ), 2 ) + pow( (double) ( rpt1.y - rpt2.y ), 2 );
            double dtest;
            int adder;
            if( b_1_on && !b_2_on ) {
                if( rpt2.x < rpt1.x ) adder = (int) pix_full_circle;
                else
                    adder = -(int) pix_full_circle;

                dtest = pow( (double) ( rpt1.x - ( rpt2.x + adder ) ), 2 )
                    + pow( (double) ( rpt1.y - rpt2.y ), 2 );

                if( dp < dtest ) adder = 0;

                RenderSegment( dc, rpt1.x, rpt1.y, rpt2.x + adder, rpt2.y, VP, true );
            } else
                if( !b_1_on && b_2_on ) {
                    if( rpt1.x < rpt2.x ) adder = (int) pix_full_circle;
                    else
                        adder = -(int) pix_full_circle;

                    dtest = pow( (double) ( rpt2.x - ( rpt1.x + adder ) ), 2 )
                        + pow( (double) ( rpt1.y - rpt2.y ), 2 );

                    if( dp < dtest ) adder = 0;

                    RenderSegment( dc, rpt1.x + adder, rpt1.y, rpt2.x, rpt2.y, VP, true );
                }

                //Both off, need to check shortest distance
                else
                    if( !b_1_on && !b_2_on ) {
                        if( rpt1.x < rpt2.x ) adder = (int) pix_full_circle;
                        else
                            adder = -(int) pix_full_circle;

                        dtest = pow( (double) ( rpt2.x - ( rpt1.x + adder ) ), 2 )
                            + pow( (double) ( rpt1.y - rpt2.y ), 2 );

                        if( dp < dtest ) adder = 0;

                        RenderSegment( dc, rpt1.x + adder, rpt1.y, rpt2.x, rpt2.y, VP, true );
                    }
        }
        rpt1 = rpt2;
        prp1 = prp2;

        node = node->GetNext();
    }
}

static int s_arrow_icon[] = { 0, 0, 5, 2, 18, 6, 12, 0, 18, -6, 5, -2, 0, 0 };

void Route::RenderSegment( ocpnDC& dc, int xa, int ya, int xb, int yb, ViewPort &VP,
        bool bdraw_arrow, int hilite_width )
{
    //    Get the dc boundary
    int sx, sy;
    dc.GetSize( &sx, &sy );

    //    Try to exit early if the segment is nowhere near the screen
    wxRect r( 0, 0, sx, sy );
    wxRect s( xa, ya, 1, 1 );
    wxRect t( xb, yb, 1, 1 );
    s.Union( t );
    if( !r.Intersects( s ) ) return;

    //    Clip the line segment to the dc boundary
    int x0 = xa;
    int y0 = ya;
    int x1 = xb;
    int y1 = yb;

    //    If hilite is desired, use a Native Graphics context to render alpha colours
    //    That is, if wxGraphicsContext is available.....

    if( hilite_width ) {
        if( Visible == cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, 0, sx, 0, sy ) ) {
            wxPen psave = dc.GetPen();

            wxColour y = GetGlobalColor( _T ( "YELO1" ) );
            wxColour hilt( y.Red(), y.Green(), y.Blue(), 128 );

            wxPen HiPen( hilt, hilite_width, wxSOLID );

            dc.SetPen( HiPen );
            dc.StrokeLine( x0, y0, x1, y1 );

            dc.SetPen( psave );
            dc.StrokeLine( x0, y0, x1, y1 );
        }
    } else {
        if( Visible == cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, 0, sx, 0, sy ) )
            dc.StrokeLine( x0, y0, x1, y1 );
    }

    if( bdraw_arrow ) {
        //    Draw a direction arrow

        double theta = atan2( (double) ( yb - ya ), (double) ( xb - xa ) );
        theta -= PI / 2;

        wxPoint icon[10];
        double icon_scale_factor = 100 * VP.view_scale_ppm;
        icon_scale_factor = fmin ( icon_scale_factor, 1.5 );              // Sets the max size
        icon_scale_factor = fmax ( icon_scale_factor, .10 );

        //    Get the absolute line length
        //    and constrain the arrow to be no more than xx% of the line length
        double nom_arrow_size = 20.;
        double max_arrow_to_leg = .20;
        double lpp = sqrt( pow( (double) ( xa - xb ), 2 ) + pow( (double) ( ya - yb ), 2 ) );

        double icon_size = icon_scale_factor * nom_arrow_size;
        if( icon_size > ( lpp * max_arrow_to_leg ) ) icon_scale_factor = ( lpp * max_arrow_to_leg )
                / nom_arrow_size;

        for( int i = 0; i < 7; i++ ) {
            int j = i * 2;
            double pxa = (double) ( s_arrow_icon[j] );
            double pya = (double) ( s_arrow_icon[j + 1] );

            pya *= icon_scale_factor;
            pxa *= icon_scale_factor;

            double px = ( pxa * sin( theta ) ) + ( pya * cos( theta ) );
            double py = ( pya * sin( theta ) ) - ( pxa * cos( theta ) );

            icon[i].x = (int) ( px ) + xb;
            icon[i].y = (int) ( py ) + yb;
        }
        wxPen savePen = dc.GetPen();
        dc.SetPen( *wxTRANSPARENT_PEN );
        dc.StrokePolygon( 6, &icon[0], 0, 0 );
        dc.SetPen( savePen );
    }
}

void Route::ClearHighlights( void )
{
    RoutePoint *prp = NULL;
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    while( node ) {
        prp = node->GetData();
        if( prp ) prp->m_bPtIsSelected = false;
        node = node->GetNext();
    }
}

RoutePoint *Route::InsertPointBefore( RoutePoint *pRP, double rlat, double rlon,
        bool bRenamePoints )
{
    RoutePoint *newpoint = new RoutePoint( rlat, rlon, wxString( _T ( "diamond" ) ),
            GetNewMarkSequenced(), GPX_EMPTY_STRING );
    newpoint->m_bIsInRoute = true;
    newpoint->m_bDynamicName = true;
    newpoint->SetNameShown( false );

    int nRP = pRoutePointList->IndexOf( pRP );
    pRoutePointList->Insert( nRP, newpoint );

    RoutePointGUIDList.Insert( pRP->m_GUID, nRP );

    m_nPoints++;

    if( bRenamePoints ) RenameRoutePoints();

    CalculateBBox();
    UpdateSegmentDistances();

    return ( newpoint );
}

wxString Route::GetNewMarkSequenced( void )
{
    wxString ret;
    ret.Printf( _T ( "NM%03d" ), m_nm_sequence );
    m_nm_sequence++;

    return ret;
}

RoutePoint *Route::GetLastPoint()
{
    RoutePoint *data_m1 = NULL;
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    while( node ) {
        data_m1 = node->GetData();
        node = node->GetNext();
    }
    return ( data_m1 );
}

int Route::GetIndexOf( RoutePoint *prp )
{
    int ret = pRoutePointList->IndexOf( prp ) + 1;
    if( ret == wxNOT_FOUND ) return 0;
    else
        return ret;

}

void Route::DeletePoint( RoutePoint *rp, bool bRenamePoints )
{
    //    n.b. must delete Selectables  and update config before deleting the point
    if( rp->m_bIsInLayer ) return;

    pSelect->DeleteAllSelectableRoutePoints( this );
    pSelect->DeleteAllSelectableRouteSegments( this );
    pConfig->DeleteWayPoint( rp );

    pRoutePointList->DeleteObject( rp );

    if( ( rp->m_GUID.Len() ) && ( wxNOT_FOUND != RoutePointGUIDList.Index( rp->m_GUID ) ) ) RoutePointGUIDList.Remove(
            rp->m_GUID );

    delete rp;

    m_nPoints -= 1;

    if( bRenamePoints ) RenameRoutePoints();

    if( m_nPoints > 1 ) {
        pSelect->AddAllSelectableRouteSegments( this );
        pSelect->AddAllSelectableRoutePoints( this );

        pConfig->UpdateRoute( this );
        RebuildGUIDList();                  // ensure the GUID list is intact and good

        CalculateBBox();
        UpdateSegmentDistances();
    }
}

void Route::RemovePoint( RoutePoint *rp, bool bRenamePoints )
{
    if( rp->m_bIsActive && this->IsActive() )                  //FS#348
    g_pRouteMan->DeactivateRoute();

    pSelect->DeleteAllSelectableRoutePoints( this );
    pSelect->DeleteAllSelectableRouteSegments( this );

    pRoutePointList->DeleteObject( rp );
    if( wxNOT_FOUND != RoutePointGUIDList.Index( rp->m_GUID ) ) RoutePointGUIDList.Remove(
            rp->m_GUID );
    m_nPoints -= 1;

    // check all other routes to see if this point appears in any other route
    Route *pcontainer_route = g_pRouteMan->FindRouteContainingWaypoint( rp );

    if( pcontainer_route == NULL ) {
        rp->m_bIsInRoute = false;          // Take this point out of this (and only) route
        rp->m_bDynamicName = false;
        rp->m_bIsolatedMark = true;        // This has become an isolated mark
    }

    if( bRenamePoints ) RenameRoutePoints();

//      if ( m_nPoints > 1 )
    {
        pSelect->AddAllSelectableRouteSegments( this );
        pSelect->AddAllSelectableRoutePoints( this );

        pConfig->UpdateRoute( this );
        RebuildGUIDList();                  // ensure the GUID list is intact and good

        CalculateBBox();
        UpdateSegmentDistances();
    }

}

void Route::DeSelectRoute()
{
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    RoutePoint *rp;
    while( node ) {
        rp = node->GetData();
        rp->m_bPtIsSelected = false;

        node = node->GetNext();
    }
}

void Route::ReloadRoutePointIcons()
{
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    RoutePoint *rp;
    while( node ) {
        rp = node->GetData();
        rp->ReLoadIcon();

        node = node->GetNext();
    }
}

void Route::CalculateBBox()
{

    double bbox_xmin = 180.;                        // set defaults
    double bbox_ymin = 90.;
    double bbox_xmax = -180;
    double bbox_ymax = -90.;

    RBBox.Reset();
    m_bcrosses_idl = CalculateCrossesIDL();

    wxRoutePointListNode *node = pRoutePointList->GetFirst();
    RoutePoint *data;

    if( !m_bcrosses_idl ) {
        while( node ) {
            data = node->GetData();

            if( data->m_lon > bbox_xmax ) bbox_xmax = data->m_lon;
            if( data->m_lon < bbox_xmin ) bbox_xmin = data->m_lon;
            if( data->m_lat > bbox_ymax ) bbox_ymax = data->m_lat;
            if( data->m_lat < bbox_ymin ) bbox_ymin = data->m_lat;

            node = node->GetNext();
        }
    } else {
        //    For Routes that cross the IDL, we compute and store
        //    the bbox as positive definite
        while( node ) {
            data = node->GetData();
            double lon = data->m_lon;
            if( lon < 0. ) lon += 360.;

            if( lon > bbox_xmax ) bbox_xmax = lon;
            if( lon < bbox_xmin ) bbox_xmin = lon;
            if( data->m_lat > bbox_ymax ) bbox_ymax = data->m_lat;
            if( data->m_lat < bbox_ymin ) bbox_ymin = data->m_lat;

            node = node->GetNext();
        }
    }

    RBBox.Expand( bbox_xmin, bbox_ymin );
    RBBox.Expand( bbox_xmax, bbox_ymax );

}

bool Route::CalculateCrossesIDL( void )
{
    wxRoutePointListNode *node = pRoutePointList->GetFirst();
    if( NULL == node ) return false;

    bool idl_cross = false;
    RoutePoint *data = node->GetData();             // first node

    double lon0 = data->m_lon;
    node = node->GetNext();

    while( node ) {
        data = node->GetData();
        if( ( lon0 < -150. ) && ( data->m_lon > 150. ) ) {
            idl_cross = true;
            break;
        }

        if( ( lon0 > 150. ) && ( data->m_lon < -150. ) ) {
            idl_cross = true;
            break;
        }

        lon0 = data->m_lon;

        node = node->GetNext();
    }

    return idl_cross;
}

void Route::CalculateDCRect( wxDC& dc_route, wxRect *prect, ViewPort &VP )
{
    dc_route.ResetBoundingBox();
    dc_route.DestroyClippingRegion();

    // Draw the route in skeleton form on the dc
    // That is, draw only the route points, assuming that the segements will
    // always be fully contained within the resulting rectangle.
    // Can we prove this?
    if( m_bVisible ) {
        wxRoutePointListNode *node = pRoutePointList->GetFirst();
        while( node ) {

            RoutePoint *prp2 = node->GetData();
            bool blink_save = prp2->m_bBlink;
            prp2->m_bBlink = false;
            ocpnDC odc_route( dc_route );
            prp2->Draw( odc_route, NULL );
            prp2->m_bBlink = blink_save;

            node = node->GetNext();
        }
    }

    //  Retrieve the drawing extents
    prect->x = dc_route.MinX() - 1;
    prect->y = dc_route.MinY() - 1;
    prect->width = dc_route.MaxX() - dc_route.MinX() + 2;
    prect->height = dc_route.MaxY() - dc_route.MinY() + 2;
}

/*
 Update the route segment lengths, storing each segment length in <destination> point.
 Also, compute total route length by summing segment distances.
 */
void Route::UpdateSegmentDistances( double planspeed )
{
    wxPoint rpt, rptn;
    float slat1, slon1, slat2, slon2;

    double route_len = 0.0;
    double route_time = 0.0;

    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    if( node ) {
        RoutePoint *prp0 = node->GetData();
        slat1 = prp0->m_lat;
        slon1 = prp0->m_lon;

        node = node->GetNext();

        while( node ) {
            RoutePoint *prp = node->GetData();
            slat2 = prp->m_lat;
            slon2 = prp->m_lon;

//    Calculate the absolute distance from 1->2

            double brg, dd;
            DistanceBearingMercator( slat1, slon1, slat2, slon2, &brg, &dd );

//    And store in Point 2
            prp->m_seg_len = dd;

            route_len += dd;

            slat1 = slat2;
            slon1 = slon2;

//    If Point1 Description contains VMG, store it for Properties Dialog in Point2
//    If Point1 Description contains ETD, store it in Point1

            if( planspeed > 0. ) {
                double vmg = 0.0;
                wxDateTime etd;

                if( prp0->m_MarkDescription.Find( _T("VMG=") ) != wxNOT_FOUND ) {
                    wxString s_vmg = ( prp0->m_MarkDescription.Mid(
                            prp0->m_MarkDescription.Find( _T("VMG=") ) + 4 ) ).BeforeFirst( ';' );
                    if( !s_vmg.ToDouble( &vmg ) ) vmg = planspeed;
                }

                double legspeed = planspeed;
                if( vmg > 0.1 && vmg < 1000. ) legspeed = vmg;
                if( legspeed > 0.1 && legspeed < 1000. ) {
                    route_time += dd / legspeed;
                    prp->m_seg_vmg = legspeed;
                }

                prp0->m_seg_etd = wxInvalidDateTime;
                if( prp0->m_MarkDescription.Find( _T("ETD=") ) != wxNOT_FOUND ) {
                    wxString s_etd = ( prp0->m_MarkDescription.Mid(
                            prp0->m_MarkDescription.Find( _T("ETD=") ) + 4 ) ).BeforeFirst( ';' );
                    const wxChar *parse_return = etd.ParseDateTime( s_etd );
                    if( parse_return ) {
                        wxString tz( parse_return );

                        if( tz.Find( _T("UT") ) != wxNOT_FOUND ) prp0->m_seg_etd = etd;
                        else
                            if( tz.Find( _T("LMT") ) != wxNOT_FOUND ) {
                                prp0->m_seg_etd = etd;
                                long lmt_offset = (long) ( ( prp0->m_lon * 3600. ) / 15. );
                                wxTimeSpan lmt( 0, 0, (int) lmt_offset, 0 );
                                prp0->m_seg_etd -= lmt;
                            } else
                                prp0->m_seg_etd = etd.ToUTC();
                    }
                }
            }

            prp0 = prp;

            node = node->GetNext();
        }
    }

    m_route_length = route_len;
    m_route_time = route_time * 3600.;
}

void Route::Reverse( bool bRenamePoints )
{
    RebuildGUIDList();                        // ensure the GUID list is intact and good

    //    Reverse the GUID list
    wxArrayString ArrayTemp;

    int ncount = RoutePointGUIDList.GetCount();
    for( int i = 0; i < ncount; i++ )
        ArrayTemp.Add( RoutePointGUIDList[ncount - 1 - i] );

    RoutePointGUIDList = ArrayTemp;

    pRoutePointList->DeleteContents( false );
    pRoutePointList->Clear();
    m_nPoints = 0;

    AssembleRoute();                          // Rebuild the route points from the GUID list

    if( bRenamePoints ) RenameRoutePoints();

    // Switch start/end strings. anders, 2010-01-29
    wxString tmp = m_RouteStartString;
    m_RouteStartString = m_RouteEndString;
    m_RouteEndString = tmp;
}

void Route::RebuildGUIDList( void )
{
    RoutePointGUIDList.Clear();               // empty the GUID list

    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    RoutePoint *rp;
    while( node ) {
        rp = node->GetData();
        RoutePointGUIDList.Add( rp->m_GUID );

        node = node->GetNext();
    }
}
void Route::SetVisible( bool visible, bool includeWpts )
{
    m_bVisible = visible;

    if ( !includeWpts )
        return;

    wxRoutePointListNode *node = pRoutePointList->GetFirst();
    RoutePoint *rp;
    while( node ) {
        rp = node->GetData();
        if ( rp->m_bKeepXRoute )
        {
            rp->SetVisible( visible );
            //pConfig->UpdateWayPoint( rp );
        }
        node = node->GetNext();
    }
}

void Route::SetListed( bool visible )
{
    m_bListed = visible;
}

void Route::AssembleRoute( void )
{
    //    iterate over the RoutePointGUIDs
    for( unsigned int ip = 0; ip < RoutePointGUIDList.GetCount(); ip++ ) {
        wxString GUID = RoutePointGUIDList[ip];

        //    And on the RoutePoints themselves
        wxRoutePointListNode *prpnode = pWayPointMan->m_pWayPointList->GetFirst();
        while( prpnode ) {
            RoutePoint *prp = prpnode->GetData();

            if( prp->m_GUID == GUID ) {
                AddPoint( prp );
                break;
            }
            prpnode = prpnode->GetNext(); //RoutePoint
        }
    }
}

void Route::RenameRoutePoints( void )
{
    //    iterate on the route points.
    //    If dynamically named, rename according to current list position

    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    int i = 1;
    while( node ) {
        RoutePoint *prp = node->GetData();
        if( prp->m_bDynamicName ) {
            wxString name;
            name.Printf( _T ( "%03d" ), i );
            prp->SetName( name );
        }

        node = node->GetNext();
        i++;
    }
}

bool Route::SendToGPS(const wxString & com_name, bool bsend_waypoints, wxGauge *pProgress )
{
    bool result = false;

    if( g_pMUX ) {
        ::wxBeginBusyCursor();
         result = g_pMUX->SendRouteToGPS( this, com_name, bsend_waypoints, pProgress );
        ::wxEndBusyCursor();
    }

    wxString msg;
    if( result ) msg = _("Route Uploaded successfully.");
    else
        msg = _("Error on Route Upload.  Please check logfiles...");

    OCPNMessageBox( NULL, msg, _("OpenCPN Info"), wxOK | wxICON_INFORMATION );

    return result;
}

//    Is this route equal to another, meaning,
//    Do all routepoint positions and names match?
bool Route::IsEqualTo( Route *ptargetroute )
{
    wxRoutePointListNode *pthisnode = ( this->pRoutePointList )->GetFirst();
    wxRoutePointListNode *pthatnode = ( ptargetroute->pRoutePointList )->GetFirst();

    if( NULL == pthisnode ) return false;

    if( this->m_bIsInLayer || ptargetroute->m_bIsInLayer ) return false;

    if( this->GetnPoints() != ptargetroute->GetnPoints() ) return false;

    while( pthisnode ) {
        if( NULL == pthatnode ) return false;

        RoutePoint *pthisrp = pthisnode->GetData();
        RoutePoint *pthatrp = pthatnode->GetData();

        if( ( fabs( pthisrp->m_lat - pthatrp->m_lat ) > 1.0e-6 )
                || ( fabs( pthisrp->m_lon - pthatrp->m_lon ) > 1.0e-6 ) ) return false;

        if( !pthisrp->GetName().IsSameAs( pthatrp->GetName() ) ) return false;

        pthisnode = pthisnode->GetNext();
        pthatnode = pthatnode->GetNext();
    }

    return true;                              // success, they are the same
}

