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

#include "Boundary.h"
#include "georef.h"
#include "routeman.h"
#include "ocpndc.h"
#include "cutil.h"
#include "navutil.h"
#include "multiplexer.h"
#include "Select.h"
#include "georef.h"

extern WayPointman *pWayPointMan;
extern bool g_bIsNewLayer;
extern int g_LayerIdx;
extern Routeman *g_pRouteMan;
extern int g_boundary_line_width;
extern Select *pSelect;
extern MyConfig *pConfig;
extern Multiplexer *g_pMUX;
extern float g_GLMinSymbolLineWidth;

#include <wx/listimpl.cpp>
WX_DEFINE_LIST ( BoundaryList );

Boundary::Boundary( void )
{
    m_bBndIsSelected = false;
    m_bBndIsActive = false;
    m_pRouteActivePoint = NULL;
    m_bIsBeingEdited = false;
    m_bIsBeingCreated = false;
    m_bIsTrack = false;
    m_nPoints = 0;
    m_nm_sequence = 1;
    m_boundary_length = 0.0;
    m_bVisible = true;
    m_bListed = true;
    m_bDeleteOnArrival = false;
    m_width = STYLE_UNDEFINED;
    m_style = STYLE_UNDEFINED;
    m_hiliteWidth = 0;

    pRoutePointList = new RoutePointList;
    m_pLastAddedPoint = NULL;
    m_pFirstAddedPoint = NULL;
    m_GUID = pWayPointMan->CreateGUID( NULL );
    m_btemp = false;
    
    m_bNeedsUpdateBBox = true;
    RBBox.Reset();
    m_bcrosses_idl = false;

    m_LayerID = 0;
    m_bIsInLayer = false;

    m_Colour = wxEmptyString;
    m_LineColour = wxEmptyString;

    m_lastMousePointIndex = 0;
    m_NextLegGreatCircle = false;
    
    m_HyperlinkList = new HyperlinkList;
}

Boundary::~Boundary( void )
{
    pRoutePointList->DeleteContents( false );            // do not delete Marks
    pRoutePointList->Clear();
    delete pRoutePointList;
    m_HyperlinkList->Clear();
    delete m_HyperlinkList;
}

// The following is used only for route splitting, assumes just created, empty route
//
void Boundary::CloneBoundary( Boundary *psourceroute, int start_nPoint, int end_nPoint, const wxString & suffix)
{
    m_bIsTrack = psourceroute->m_bIsTrack;

    m_BoundaryNameString = psourceroute->m_BoundaryNameString + suffix;
    m_RouteStartString = psourceroute->m_RouteStartString;
    m_RouteEndString = psourceroute->m_RouteEndString;

    int i;
    for( i = start_nPoint; i <= end_nPoint; i++ ) {
        if( !psourceroute->m_bIsInLayer ) AddPoint( psourceroute->GetPoint( i ), false );
        else {
            RoutePoint *psourcepoint = psourceroute->GetPoint( i );
            RoutePoint *ptargetpoint = new RoutePoint( psourcepoint->m_lat, psourcepoint->m_lon,
                    psourcepoint->GetIconName(), psourcepoint->GetName(), GPX_EMPTY_STRING, false );

            AddPoint( ptargetpoint, false );

            CloneAddedRoutePoint( m_pLastAddedPoint, psourcepoint );
        }
    }

    FinalizeForRendering();

}

void Boundary::CloneAddedRoutePoint( RoutePoint *ptargetpoint, RoutePoint *psourcepoint )
{
    ptargetpoint->m_MarkDescription = psourcepoint->m_MarkDescription;
    ptargetpoint->m_bKeepXRoute = psourcepoint->m_bKeepXRoute;
    ptargetpoint->m_bIsVisible = psourcepoint->m_bIsVisible;
    ptargetpoint->m_bPtIsSelected = false;
    ptargetpoint->m_bShowName = psourcepoint->m_bShowName;
    ptargetpoint->m_bBlink = psourcepoint->m_bBlink;
    ptargetpoint->m_bBlink = psourcepoint->m_bDynamicName;
    ptargetpoint->CurrentRect_in_DC = psourcepoint->CurrentRect_in_DC;
    ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetX;
    ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetY;
    ptargetpoint->SetCreateTime(psourcepoint->GetCreateTime());
    ptargetpoint->m_HyperlinkList = new HyperlinkList;
    ptargetpoint->ReLoadIcon();
    
    if( !psourcepoint->m_HyperlinkList->IsEmpty() ) {
        HyperlinkList::iterator iter = psourcepoint->m_HyperlinkList->begin();
        psourcepoint->m_HyperlinkList->splice( iter, *( ptargetpoint->m_HyperlinkList ) );
    }
}

void Boundary::AddPoint( RoutePoint *pNewPoint, bool b_rename_in_sequence, bool b_deferBoxCalc, bool b_isLoading )
{
    if( pNewPoint->m_bIsolatedMark ) {
        pNewPoint->m_bKeepXRoute = true;
    }
    pNewPoint->m_bIsolatedMark = false;       // definitely no longer isolated
    pNewPoint->m_bIsInBoundary = true;

    pRoutePointList->Append( pNewPoint );

    m_nPoints++;

    if( !b_deferBoxCalc )
        FinalizeForRendering();

    if ( m_pFirstAddedPoint == NULL )
        m_pFirstAddedPoint = pNewPoint;
    
    if (!b_isLoading)
        UpdateSegmentDistances();
    m_pLastAddedPoint = pNewPoint;

    if( b_rename_in_sequence && pNewPoint->GetName().IsEmpty() && !pNewPoint->m_bKeepXRoute ) {
        wxString name;
        name.Printf( _T ( "%03d" ), m_nPoints );
        pNewPoint->SetName( name );
        pNewPoint->m_bDynamicName = true;
    }
    return;
}

void Boundary::AddTentativePoint( const wxString& GUID )
{
    RoutePointGUIDList.Add( GUID );
    return;
}

RoutePoint *Boundary::GetPoint( int nWhichPoint )
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

RoutePoint *Boundary::GetPoint( const wxString &guid )
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

void Boundary::DrawPointWhich( ocpnDC& dc, int iPoint, wxPoint *rpn )
{
    if( iPoint <= GetnPoints() )
        GetPoint( iPoint )->Draw( dc, rpn );
}

void Boundary::DrawSegment( ocpnDC& dc, wxPoint *rp1, wxPoint *rp2, ViewPort &VP, bool bdraw_arrow )
{
/*    if( m_bBndIsSelected ) dc.SetPen( *g_pRouteMan->GetSelectedBoundaryPen() );
    else
        if( m_bBndIsActive ) dc.SetPen( *g_pRouteMan->GetActiveBoundaryPen() );
        else
            dc.SetPen( *g_pRouteMan->GetBoundaryPen() );
*/    
    wxColour col, linecol;

    if( m_Colour == wxEmptyString ) {
        col = g_pRouteMan->GetBoundaryPen()->GetColour();
    } else {
        for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
            if( m_Colour == ::GpxxColorNames[i] ) {
                col = ::GpxxColors[i];
                break;
            }
        }
    }
    if( m_LineColour == wxEmptyString ) {
        linecol = g_pRouteMan->GetBoundaryPen()->GetColour();
    } else {
        for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
            if( m_LineColour == ::GpxxColorNames[i] ) {
                linecol = ::GpxxColors[i];
                break;
            }
        }
    }
    dc.SetPen( *wxThePenList->FindOrCreatePen( col, g_boundary_line_width, m_style ) );
    dc.SetBrush( *wxTheBrushList->FindOrCreateBrush( linecol, wxCROSSDIAG_HATCH ) );

    RenderSegment( dc, rp1->x, rp1->y, rp2->x, rp2->y, VP, bdraw_arrow );
}

void Boundary::Draw( ocpnDC& dc, ViewPort &VP )
{
    if( m_nPoints == 0 ) return;

    if( m_bVisible && m_bBndIsSelected ) {
        dc.SetPen( *g_pRouteMan->GetSelectedBoundaryPen() );
        dc.SetBrush( *g_pRouteMan->GetSelectedBoundaryBrush() );
    }
    else if ( m_bVisible )
    {
        int style = wxSOLID;
        int width = g_boundary_line_width;
        wxColour col, linecol;
        if( m_style != STYLE_UNDEFINED ) style = m_style;
        if( m_width != STYLE_UNDEFINED ) width = m_width;
        if( m_Colour == wxEmptyString ) {
            col = g_pRouteMan->GetBoundaryPen()->GetColour();
        } else {
            for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
                if( m_Colour == ::GpxxColorNames[i] ) {
                    col = ::GpxxColors[i];
                    break;
                }
            }
        }
        if( m_LineColour == wxEmptyString ) {
            linecol = g_pRouteMan->GetBoundaryPen()->GetColour();
        } else {
            for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
                if( m_LineColour == ::GpxxColorNames[i] ) {
                    linecol = ::GpxxColors[i];
                    break;
                }
            }
        }
        dc.SetPen( *wxThePenList->FindOrCreatePen( col, width, style ) );
        dc.SetBrush( *wxTheBrushList->FindOrCreateBrush( linecol, wxCROSSDIAG_HATCH ) );
    }

/* May need to redo this when working to show up better and be more consistent
    if( m_bVisible && m_bBndIsActive )
    {
        dc.SetPen( *g_pRouteMan->GetActiveBoundaryPen() );
        dc.SetBrush( *g_pRouteMan->GetActiveBoundaryBrush() );
    }
*/
    wxPoint rpt1, rpt2;
    wxPoint *bpts = new wxPoint[ pRoutePointList->GetCount() ];
    int j = 0;
    
    if ( m_bVisible )
        DrawPointWhich( dc, 1, &rpt1 );

    wxRoutePointListNode *node = pRoutePointList->GetFirst();
    RoutePoint *prp1 = node->GetData();
    wxPoint frpt = rpt1;
    node = node->GetNext();
    
    bpts[ j++ ] = rpt1;
        
    if ( !m_bVisible && prp1->m_bKeepXRoute )
            prp1->Draw( dc );

    while( node ) {

        RoutePoint *prp2 = node->GetData();
        if ( !m_bVisible && prp2->m_bKeepXRoute )
            prp2->Draw( dc );
        else if (m_bVisible)
            prp2->Draw( dc, &rpt2 );
        bpts[ j++ ] = ( rpt2 );

        if ( m_bVisible )
        {
            //    Handle offscreen points
            bool b_2_on = VP.GetBBox().PointInBox( prp2->m_lon, prp2->m_lat, 0 );
            bool b_1_on = VP.GetBBox().PointInBox( prp1->m_lon, prp1->m_lat, 0 );

            //Simple case
            if( b_1_on && b_2_on ) RenderSegment( dc, rpt1.x, rpt1.y, rpt2.x, rpt2.y, VP, false, m_hiliteWidth ); // with no arrows

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

                RenderSegment( dc, rpt1.x, rpt1.y, rpt2.x + adder, rpt2.y, VP, false, m_hiliteWidth );
            } else
                if( !b_1_on ) {
                    if( rpt1.x < rpt2.x ) adder = (int) pix_full_circle;
                    else
                        adder = -(int) pix_full_circle;
                    
                    float rxd = rpt2.x - ( rpt1.x + adder );
                    float ryd = rpt1.y - rpt2.y;
                    dtest = rxd*rxd + ryd*ryd;
                    
                    if( dp < dtest ) adder = 0;

                    RenderSegment( dc, rpt1.x + adder, rpt1.y, rpt2.x, rpt2.y, VP, false, m_hiliteWidth );
                }
        }

        rpt1 = rpt2;
        prp1 = prp2;

        node = node->GetNext();
    }

    // fill boundary with hatching
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush( *wxTheBrushList->FindOrCreateBrush( m_LineColour, wxCROSSDIAG_HATCH ) );
    
    dc.DrawPolygon( j, bpts, 0, 0);
}

extern ChartCanvas *cc1; /* hopefully can eventually remove? */

void Boundary::DrawGL( ViewPort &VP, OCPNRegion &region )
{
#ifdef ocpnUSE_GL
    if( m_nPoints < 1 || !m_bVisible ) return;
    
    //  Hiliting first
    //  Being special case to draw something for a 1 point route....
    ocpnDC dc;
    if(m_hiliteWidth){
        wxColour y = GetGlobalColor( _T ( "YELO1" ) );
        wxColour hilt( y.Red(), y.Green(), y.Blue(), 128 );
        wxPen HiPen( hilt, m_hiliteWidth, wxSOLID );
        dc.SetPen( HiPen );
        
        wxRoutePointListNode *node = pRoutePointList->GetFirst();
        RoutePoint *prp0 = node->GetData();
        wxPoint r0;
        cc1->GetCanvasPointPix( prp0->m_lat, prp0->m_lon, &r0);

        if( m_nPoints == 1 ) {
            dc.StrokeLine( r0.x, r0.y, r0.x + 2, r0.y + 2 );
//            return;
        }
            
        
        node = node->GetNext();
        while( node ){
            
            RoutePoint *prp = node->GetData();
            wxPoint r1;
            cc1->GetCanvasPointPix( prp->m_lat, prp->m_lon, &r1);

            dc.StrokeLine( r0.x, r0.y, r1.x, r1.y );
                    
            r0 = r1;
            node = node->GetNext();
            
        }
    }
    
//    if( m_nPoints < 2  )
//        return;
    
    /* determine color and width */
    wxColour col;
    int width;
    if( m_bBndIsActive )
    {
        wxPen &pen = *g_pRouteMan->GetActiveBoundaryPen();
        col = pen.GetColour();
        width = pen.GetWidth();
    } else if( m_bBndIsSelected ) {
        wxPen &pen = *g_pRouteMan->GetSelectedBoundaryPen();
        col = pen.GetColour();
        width = pen.GetWidth();
    } else {
        width = g_boundary_line_width;
        
        if( m_width != STYLE_UNDEFINED ) width = m_width;
        if( m_Colour == wxEmptyString ) {
            col = g_pRouteMan->GetBoundaryPen()->GetColour();
            
            //  For tracks, establish colour based on first icon name
            if(m_bIsTrack){
                wxRoutePointListNode *node = pRoutePointList->GetFirst();
                RoutePoint *prp = node->GetData();
                
                if( prp->GetIconName().StartsWith( _T("xmred") ) ) 
                    col = GetGlobalColor( _T ( "URED" ) );
                else if( prp->GetIconName().StartsWith( _T("xmblue") ) ) 
                    col = GetGlobalColor( _T ( "BLUE3" ) );
                else if( prp->GetIconName().StartsWith( _T("xmgreen") ) ) 
                    col = GetGlobalColor( _T ( "UGREN" ) );
                else 
                    col = GetGlobalColor( _T ( "CHMGD" ) );
            }
        } else {
            for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
                if( m_Colour == ::GpxxColorNames[i] ) {
                    col = ::GpxxColors[i];
                    break;
                }
            }
        }
    }
    
    int style = wxSOLID;
    if( m_style != STYLE_UNDEFINED ) style = m_style;
    dc.SetPen( *wxThePenList->FindOrCreatePen( col, width, style ) );
    
    glColor3ub(col.Red(), col.Green(), col.Blue());
    glLineWidth( wxMax( g_GLMinSymbolLineWidth, width ) );
    
    dc.SetGLStipple();

    glBegin(GL_LINE_STRIP);
    float lastlon = 0;
    float lastlat = 0;
    unsigned short int FromSegNo = 1;
    for(wxRoutePointListNode *node = pRoutePointList->GetFirst();
        node; node = node->GetNext()) {
        RoutePoint *prp = node->GetData();
        unsigned short int ToSegNo = prp->m_GPXTrkSegNo;
        
        /* crosses IDL? if so break up into two segments */
        int dir = 0;
        if(prp->m_lon > 150 && lastlon < -150)
            dir = -1;
        else if(prp->m_lon < -150 && lastlon > 150)
            dir = 1;
        
        wxPoint r;
        if (FromSegNo != ToSegNo)
        {
            glEnd();
            FromSegNo = ToSegNo;
            glBegin(GL_LINE_STRIP);
        }
        if(dir)
        {
            double crosslat = lat_rl_crosses_meridian(lastlat, lastlon, prp->m_lat, prp->m_lon, 180.0);
            cc1->GetCanvasPointPix( crosslat, dir*180, &r);
            glVertex2i(r.x, r.y);
            glEnd();
            glBegin(GL_LINE_STRIP);
            cc1->GetCanvasPointPix( crosslat, -dir*180, &r);
            glVertex2i(r.x, r.y);
        }
        lastlat=prp->m_lat;
        lastlon=prp->m_lon;
        
        cc1->GetCanvasPointPix( prp->m_lat, prp->m_lon, &r);
        glVertex2i(r.x, r.y);
    }
    glEnd();
    glDisable (GL_LINE_STIPPLE);
    
    /* direction arrows.. could probably be further optimized for opengl */
    if( !m_bIsTrack ) {
        wxRoutePointListNode *node = pRoutePointList->GetFirst();
        wxPoint rpt1, rpt2;
        while(node) {
            RoutePoint *prp = node->GetData();
            cc1->GetCanvasPointPix( prp->m_lat, prp->m_lon, &rpt2 );
            if(node != pRoutePointList->GetFirst())
                RenderSegmentArrowsGL( rpt1.x, rpt1.y, rpt2.x, rpt2.y, cc1->GetVP() );
            rpt1 = rpt2;
            node = node->GetNext();
        }
    }

    /*  Boundary points  */
    for(wxRoutePointListNode *node = pRoutePointList->GetFirst(); node; node = node->GetNext()) {
        RoutePoint *prp = node->GetData();
        if ( !m_bVisible && prp->m_bKeepXRoute )
            prp->DrawGL( VP, region );
        else if (m_bVisible)
            prp->DrawGL( VP, region );
    
    }
        
#endif
}

static int s_arrow_icon[] = { 0, 0, 5, 2, 18, 6, 12, 0, 18, -6, 5, -2, 0, 0 };

void Boundary::RenderSegment( ocpnDC& dc, int xa, int ya, int xb, int yb, ViewPort &VP,
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
        theta -= PI / 2.;

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

void Boundary::RenderSegmentArrowsGL( int xa, int ya, int xb, int yb, ViewPort &VP)
{
#ifdef ocpnUSE_GL
    //    Draw a direction arrow        
    wxPoint icon[10];
    float icon_scale_factor = 100 * VP.view_scale_ppm;
    icon_scale_factor = fmin ( icon_scale_factor, 1.5 );              // Sets the max size
    icon_scale_factor = fmax ( icon_scale_factor, .10 );
    
    //    Get the absolute line length
    //    and constrain the arrow to be no more than xx% of the line length
    float nom_arrow_size = 20.;
    float max_arrow_to_leg = (float).20;
    float lpp = sqrtf( powf( (float) (xa - xb), 2) + powf( (float) (ya - yb), 2) );
    
    float icon_size = icon_scale_factor * nom_arrow_size;
    if( icon_size > ( lpp * max_arrow_to_leg ) )
        icon_scale_factor = ( lpp * max_arrow_to_leg )
            / nom_arrow_size;

    float theta = atan2f( (float)yb - ya, (float)xb - xa );
    theta -= (float)PI;

    glPushMatrix();
    glTranslatef(xb, yb, 0);
    glScalef(icon_scale_factor, icon_scale_factor, 1);
    glRotatef(theta * 180/PI, 0, 0, 1);

    glBegin(GL_POLYGON);
    for( int i = 0; i < 14; i+=2 )
        glVertex2f(s_arrow_icon[i], s_arrow_icon[i+1]);
    glEnd();

    glPopMatrix();
#endif
}

void Boundary::ClearHighlights( void )
{
    RoutePoint *prp = NULL;
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    while( node ) {
        prp = node->GetData();
        if( prp ) prp->m_bPtIsSelected = false;
        node = node->GetNext();
    }
}

wxString Boundary::GetNewMarkSequenced( void )
{
    wxString ret;
    ret.Printf( _T ( "NM%03d" ), m_nm_sequence );
    m_nm_sequence++;

    return ret;
}

RoutePoint *Boundary::GetLastPoint()
{
    RoutePoint *data_m1 = NULL;
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    while( node ) {
        data_m1 = node->GetData();
        node = node->GetNext();
    }
    return ( data_m1 );
}

int Boundary::GetIndexOf( RoutePoint *prp )
{
    int ret = pRoutePointList->IndexOf( prp ) + 1;
    if( ret == wxNOT_FOUND ) return 0;
    else
        return ret;

}

void Boundary::DeletePoint( RoutePoint *rp, bool bRenamePoints )
{
    //    n.b. must delete Selectables  and update config before deleting the point
    if( rp->m_bIsInLayer ) return;

    pSelect->DeleteAllSelectableRoutePoints( this );
    pSelect->DeleteAllSelectableBoundarySegments( this );
    pConfig->DeleteWayPoint( rp );

    pRoutePointList->DeleteObject( rp );

    if( ( rp->m_GUID.Len() ) && ( wxNOT_FOUND != RoutePointGUIDList.Index( rp->m_GUID ) ) ) RoutePointGUIDList.Remove(
            rp->m_GUID );

    delete rp;

    m_nPoints -= 1;

    if( bRenamePoints ) RenameRoutePoints();

    if( m_nPoints > 1 ) {
        pSelect->AddAllSelectableBoundarySegments( this );
        pSelect->AddAllSelectableRoutePoints( this );

        pConfig->UpdateBoundary( this );
        RebuildGUIDList();                  // ensure the GUID list is intact and good

        FinalizeForRendering();
        UpdateSegmentDistances();
    }
}

void Boundary::RemovePoint( RoutePoint *rp, bool bRenamePoints )
{
    pSelect->DeleteAllSelectableRoutePoints( this );
    pSelect->DeleteAllSelectableBoundarySegments( this );

    pRoutePointList->DeleteObject( rp );
    if( wxNOT_FOUND != RoutePointGUIDList.Index( rp->m_GUID ) ) RoutePointGUIDList.Remove(
            rp->m_GUID );
    m_nPoints -= 1;

    // check all other routes to see if this point appears in any other route
    Route *pcontainer_route = g_pRouteMan->FindRouteContainingWaypoint( rp );

    if( pcontainer_route == NULL ) {
        rp->m_bIsInBoundary = false;          // Take this point out of this (and only) route
        rp->m_bDynamicName = false;
        rp->m_bIsolatedMark = true;        // This has become an isolated mark
    }

    if( bRenamePoints ) RenameRoutePoints();

//      if ( m_nPoints > 1 )
    {
        pSelect->AddAllSelectableBoundarySegments( this );
        pSelect->AddAllSelectableRoutePoints( this );

        pConfig->UpdateBoundary( this );
        RebuildGUIDList();                  // ensure the GUID list is intact and good

        FinalizeForRendering();
        UpdateSegmentDistances();
    }

}

void Boundary::DeSelectBoundary()
{
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    RoutePoint *rp;
    while( node ) {
        rp = node->GetData();
        rp->m_bPtIsSelected = false;

        node = node->GetNext();
    }
}

void Boundary::ReloadBoundaryPointIcons()
{
    wxRoutePointListNode *node = pRoutePointList->GetFirst();

    RoutePoint *rp;
    while( node ) {
        rp = node->GetData();
        rp->ReLoadIcon();

        node = node->GetNext();
    }
}

void Boundary::FinalizeForRendering()
{
    m_bNeedsUpdateBBox = true;
}

wxBoundingBox Boundary::GetBBox( void )
{
    if(!m_bNeedsUpdateBBox)
        return RBBox;

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

    m_bNeedsUpdateBBox = false;
    return RBBox;
}

bool Boundary::CalculateCrossesIDL( void )
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

void Boundary::CalculateDCRect( wxDC& dc_boundary, wxRect *prect, ViewPort &VP )
{
    dc_boundary.ResetBoundingBox();
    dc_boundary.DestroyClippingRegion();
    
    wxRect update_rect;

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
            ocpnDC odc_boundary( dc_boundary );
            prp2->Draw( odc_boundary, NULL );
            prp2->m_bBlink = blink_save;

            wxRect r =  prp2->CurrentRect_in_DC ;
            r.Inflate(m_hiliteWidth, m_hiliteWidth);        // allow for large hilite circles at segment ends
                
            update_rect.Union( r );
            node = node->GetNext();
        }
    }

    *prect = update_rect;
}

void Boundary::RebuildGUIDList( void )
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
void Boundary::SetVisible( bool visible, bool includeWpts )
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

void Boundary::SetListed( bool visible )
{
    m_bListed = visible;
}

void Boundary::AssembleBoundary( void )
{
    //    iterate over the RoutePointGUIDs
    for( unsigned int ip = 0; ip < RoutePointGUIDList.GetCount(); ip++ ) {
        wxString GUID = RoutePointGUIDList[ip];

        //    And on the RoutePoints themselves
        wxRoutePointListNode *prpnode = pWayPointMan->GetWaypointList()->GetFirst();
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

void Boundary::RenameRoutePoints( void )
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

//    Is this Boundary equal to another, meaning,
//    Do all routepoint positions and names match?
bool Boundary::IsEqualTo( Boundary *ptargetroute )
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
/*
 Update the boundary segment lengths, storing each segment length in <destination> point.
 Also, compute total boundary length by summing segment distances.
 */
void Boundary::UpdateSegmentDistances()
{
    wxPoint rpt, rptn;
    float slat1, slon1, slat2, slon2;

    double boundary_len = 0.0;

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

            boundary_len += dd;

            slat1 = slat2;
            slon1 = slon2;

            prp0 = prp;

            node = node->GetNext();
        }
    }

    m_boundary_length = boundary_len;
}

RoutePoint *Boundary::InsertPointBefore( RoutePoint *pRP, double rlat, double rlon,
        bool bRenamePoints )
{
    RoutePoint *newpoint = new RoutePoint( rlat, rlon, wxString( _T ( "diamond" ) ),
            GetNewMarkSequenced(), GPX_EMPTY_STRING );
    newpoint->m_bIsInBoundary = true;
    newpoint->m_bDynamicName = true;
    newpoint->SetNameShown( false );

    int nRP = pRoutePointList->IndexOf( pRP );
    if ( nRP == 0 ) {
        pRoutePointList->Insert( pRoutePointList->GetCount() - 1, newpoint );
        nRP = pRoutePointList->GetCount();
    }
    else {
        pRoutePointList->Insert( nRP, newpoint );
    }

    RoutePointGUIDList.Insert( pRP->m_GUID, nRP );

    m_nPoints++;

    if( bRenamePoints ) RenameRoutePoints();

    FinalizeForRendering();
    UpdateSegmentDistances();

    return ( newpoint );
}

