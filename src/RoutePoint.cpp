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

#include <wx/dcscreen.h>
#include <wx/tokenzr.h>

#include "RoutePoint.h"
#include "routeman.h"
#include "chcanv.h"
#include "multiplexer.h"
#include "navutil.h"
#include "FontMgr.h"

extern WayPointman *pWayPointMan;
extern bool g_bIsNewLayer;
extern int g_LayerIdx;
extern ChartCanvas *cc1;
extern Routeman *g_pRouteMan;
extern wxRect g_blink_rect;
extern Multiplexer *g_pMUX;
extern MyFrame *gFrame;
extern FontMgr *pFontMgr;

#include <wx/listimpl.cpp>
WX_DEFINE_LIST ( RoutePointList );

RoutePoint::RoutePoint()
{
    m_pbmIcon = NULL;

    //  Nice defaults
    m_seg_len = 0.0;
    m_seg_vmg = 0.0;
    m_seg_etd = wxInvalidDateTime;
    m_bDynamicName = false;
    m_bPtIsSelected = false;
    m_bIsBeingEdited = false;
    m_bIsActive = false;
    m_bBlink = false;
    m_bIsInRoute = false;
    m_bIsInTrack = false;
    wxDateTime now = wxDateTime::Now();
    m_CreateTime = now.ToUTC();
    m_GPXTrkSegNo = 1;
    m_bIsolatedMark = false;
    m_bShowName = true;
    m_bKeepXRoute = false;
    m_bIsVisible = true;
    m_bIsListed = true;
    m_ConfigWPNum = -1;
    CurrentRect_in_DC = wxRect( 0, 0, 0, 0 );
    m_NameLocationOffsetX = -10;
    m_NameLocationOffsetY = 8;
    m_pMarkFont = NULL;
    m_btemp = false;

    m_prop_string_format = _T ( "A" );

    m_HyperlinkList = new HyperlinkList;

    m_GUID = pWayPointMan->CreateGUID( this );

    m_IconName = wxEmptyString;
    ReLoadIcon();

    m_MarkName = wxEmptyString;

    m_bIsInLayer = false;
    m_LayerID = 0;
}

// Copy Constructor
RoutePoint::RoutePoint( RoutePoint* orig )
{
    m_MarkName = orig->GetName();
    m_lat = orig->m_lat;
    m_lon = orig->m_lon;
    m_seg_len = orig->m_seg_len;
    m_seg_vmg = orig->m_seg_vmg;
    m_seg_etd = orig->m_seg_etd;
    m_bDynamicName = orig->m_bDynamicName;
    m_bPtIsSelected = orig->m_bPtIsSelected;
    m_bIsBeingEdited = orig->m_bIsBeingEdited;
    m_bIsActive = orig->m_bIsActive;
    m_bBlink = orig->m_bBlink;
    m_bIsInRoute = orig->m_bIsInRoute;
    m_bIsInTrack = orig->m_bIsInTrack;
    m_CreateTime = orig->m_CreateTime;
    m_GPXTrkSegNo = orig->m_GPXTrkSegNo;
    m_bIsolatedMark = orig->m_bIsolatedMark;
    m_bShowName = orig->m_bShowName;
    m_bKeepXRoute = orig->m_bKeepXRoute;
    m_bIsVisible = orig->m_bIsVisible;
    m_bIsListed = orig->m_bIsListed;
    m_ConfigWPNum = orig->m_ConfigWPNum;
    CurrentRect_in_DC = orig->CurrentRect_in_DC;
    m_NameLocationOffsetX = orig->m_NameLocationOffsetX;
    m_NameLocationOffsetY = orig->m_NameLocationOffsetY;
    m_pMarkFont = orig->m_pMarkFont;
    m_prop_string_format = orig->m_prop_string_format;
    m_MarkDescription = orig->m_MarkDescription;
    m_btemp = orig->m_btemp;
        
    m_HyperlinkList = new HyperlinkList;
    m_IconName = orig->m_IconName;
    ReLoadIcon();

    m_bIsInLayer = orig->m_bIsInLayer;
    m_GUID = pWayPointMan->CreateGUID( this );
}

RoutePoint::RoutePoint( double lat, double lon, const wxString& icon_ident, const wxString& name,
        const wxString &pGUID, bool bAddToList )
{
    //  Establish points
    m_lat = lat;
    m_lon = lon;

    //      Normalize the longitude, to fix any old poorly formed points
    if( m_lon < -180. ) m_lon += 360.;
    else
        if( m_lon > 180. ) m_lon -= 360.;

    //  Nice defaults
    m_seg_len = 0.0;
    m_seg_vmg = 0.0;
    m_seg_etd = wxInvalidDateTime;
    m_bDynamicName = false;
    m_bPtIsSelected = false;
    m_bIsBeingEdited = false;
    m_bIsActive = false;
    m_bBlink = false;
    m_bIsInRoute = false;
    m_bIsInTrack = false;
    wxDateTime now = wxDateTime::Now();
    m_CreateTime = now.ToUTC();
    m_GPXTrkSegNo = 1;
    m_bIsolatedMark = false;
    m_bShowName = true;
    m_bKeepXRoute = false;
    m_bIsVisible = true;
    m_bIsListed = true;
    m_ConfigWPNum = -1;
    CurrentRect_in_DC = wxRect( 0, 0, 0, 0 );
    m_NameLocationOffsetX = -10;
    m_NameLocationOffsetY = 8;
    m_pMarkFont = NULL;
    m_btemp = false;
    
    m_prop_string_format = _T ( "A" );           // Set the current Property String format indicator

    m_HyperlinkList = new HyperlinkList;

    if( !pGUID.IsEmpty() ) m_GUID = pGUID;
    else
        m_GUID = pWayPointMan->CreateGUID( this );

    //      Get Icon bitmap
    m_IconName = icon_ident;
    ReLoadIcon();

    SetName( name );

    //  Possibly add the waypoint to the global list maintained by the waypoint manager

    if( bAddToList && NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->Append( this );

    m_bIsInLayer = g_bIsNewLayer;
    if( m_bIsInLayer ) {
        m_LayerID = g_LayerIdx;
        m_bIsListed = false;
    } else
        m_LayerID = 0;
}

RoutePoint::~RoutePoint( void )
{
//  Remove this point from the global waypoint list
    if( NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->DeleteObject( this );

    if( m_HyperlinkList ) {
        m_HyperlinkList->DeleteContents( true );
        delete m_HyperlinkList;
    }
}

void RoutePoint::SetName(const wxString & name)
{
    m_MarkName = name;
    CalculateNameExtents();
}

void RoutePoint::CalculateNameExtents( void )
{
    if( m_pMarkFont ) {
        wxScreenDC dc;

        dc.SetFont( *m_pMarkFont );
        m_NameExtents = dc.GetTextExtent( m_MarkName );
    } else
        m_NameExtents = wxSize( 0, 0 );

}

wxString RoutePoint::CreatePropString( void )
{
    wxString ret;
    ret.Printf( _T ( ",%d,%d,%d,%d" ), m_bDynamicName, m_bShowName, m_bKeepXRoute, m_bIsVisible );
    ret.Prepend( m_prop_string_format );
    return ret;
}

void RoutePoint::SetPropFromString( const wxString &prop_string )
{
    long tmp_prop;
    wxString str_fmt;

    if( !prop_string.IsEmpty() ) {
        wxStringTokenizer tkp( prop_string, _T ( "," ) );

        //  Look at the first character
        wxString c1 = prop_string.Mid( 0, 1 );
        if( c1.ToLong( &tmp_prop ) ) str_fmt = _T ( "A" ); // Assume format version is 'A' if first char is numeric
        else
            str_fmt = tkp.GetNextToken();

        if( str_fmt == _T ( "A" ) ) {

            wxString token = tkp.GetNextToken();
            token.ToLong( &tmp_prop );
            m_bDynamicName = !( tmp_prop == 0 );

            token = tkp.GetNextToken();
            token.ToLong( &tmp_prop );
            m_bShowName = !( tmp_prop == 0 );

            token = tkp.GetNextToken();
            token.ToLong( &tmp_prop );
            m_bKeepXRoute = !( tmp_prop == 0 );

            token = tkp.GetNextToken();               // format A might or might not have 4 fields
            if( token.Len() ) {
                token.ToLong( &tmp_prop );
                m_bIsVisible = !( tmp_prop == 0 );
            } else
                m_bIsVisible = true;
        }
    }

}

void RoutePoint::ReLoadIcon( void )
{
    m_pbmIcon = pWayPointMan->GetIconBitmap( m_IconName );
}

void RoutePoint::Draw( ocpnDC& dc, wxPoint *rpn )
{
    wxPoint r;
    wxRect hilitebox;
    unsigned char transparency = 100;

    cc1->GetCanvasPointPix( m_lat, m_lon, &r );

    //  return the home point in this dc to allow "connect the dots"
    if( NULL != rpn ) *rpn = r;

    if( !m_bIsVisible /*&& !m_bIsInTrack*/)     // pjotrc 2010.02.13, 2011.02.24
    return;

    //    Optimization, especially apparent on tracks in normal cases
    if( m_IconName == _T("empty") && !m_bShowName && !m_bPtIsSelected ) return;

    wxPen *pen;
    if( m_bBlink ) pen = g_pRouteMan->GetActiveRoutePointPen();
    else
        pen = g_pRouteMan->GetRoutePointPen();

//    Substitue icon?
    wxBitmap *pbm;
    if( ( m_bIsActive ) && ( m_IconName != _T("mob") ) ) pbm = pWayPointMan->GetIconBitmap(
            _T ( "activepoint" ) );
    else
        pbm = m_pbmIcon;

    int sx2 = pbm->GetWidth() / 2;
    int sy2 = pbm->GetHeight() / 2;

//    Calculate the mark drawing extents
    wxRect r1( r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2 );           // the bitmap extents

    if( m_bShowName ) {
        if( 0 == m_pMarkFont ) {
            m_pMarkFont = pFontMgr->GetFont( _( "Marks" ) );
            m_FontColor = pFontMgr->GetFontColor( _("Marks") );
            CalculateNameExtents();
        }

        if( m_pMarkFont ) {
            wxRect r2( r.x + m_NameLocationOffsetX, r.y + m_NameLocationOffsetY, m_NameExtents.x,
                    m_NameExtents.y );
            r1.Union( r2 );
        }
    }

    hilitebox = r1;
    hilitebox.x -= r.x;
    hilitebox.y -= r.y;
    hilitebox.Inflate( 2 );

    //  Highlite any selected point
    if( m_bPtIsSelected ) {
        AlphaBlending( dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, 0.0,
                pen->GetColour(), transparency );
    }

    bool bDrawHL = false;

    if( m_bBlink && ( gFrame->nBlinkerTick & 1 ) ) bDrawHL = true;

    if( ( !bDrawHL ) && ( NULL != m_pbmIcon ) ) {
        dc.DrawBitmap( *pbm, r.x - sx2, r.y - sy2, true );
        // on MSW, the dc Bounding box is not updated on DrawBitmap() method.
        // Do it explicitely here for all platforms.
        dc.CalcBoundingBox( r.x - sx2, r.y - sy2 );
        dc.CalcBoundingBox( r.x + sx2, r.y + sy2 );
    }

    if( m_bShowName ) {
        if( m_pMarkFont ) {
            dc.SetFont( *m_pMarkFont );
            dc.SetTextForeground( m_FontColor );

            dc.DrawText( m_MarkName, r.x + m_NameLocationOffsetX, r.y + m_NameLocationOffsetY );
        }
    }

    //  Save the current draw rectangle in the current DC
    //    This will be useful for fast icon redraws
    CurrentRect_in_DC.x = r.x + hilitebox.x;
    CurrentRect_in_DC.y = r.y + hilitebox.y;
    CurrentRect_in_DC.width = hilitebox.width;
    CurrentRect_in_DC.height = hilitebox.height;

    if( m_bBlink ) g_blink_rect = CurrentRect_in_DC;               // also save for global blinker

}

void RoutePoint::SetPosition( double lat, double lon )
{
    m_lat = lat;
    m_lon = lon;
}

void RoutePoint::CalculateDCRect( wxDC& dc, wxRect *prect )
{
    dc.ResetBoundingBox();
    dc.DestroyClippingRegion();

    // Draw the mark on the dc
    ocpnDC odc( dc );
    Draw( odc, NULL );

    //  Retrieve the drawing extents
    prect->x = dc.MinX() - 1;
    prect->y = dc.MinY() - 1;
    prect->width = dc.MaxX() - dc.MinX() + 2; // Mouse Poop?
    prect->height = dc.MaxY() - dc.MinY() + 2;

}

bool RoutePoint::IsSame( RoutePoint *pOtherRP )
{
    bool IsSame = false;

    if( this->m_MarkName == pOtherRP->m_MarkName ) {
        if( fabs( this->m_lat - pOtherRP->m_lat ) < 1.e-6
                && fabs( this->m_lon - pOtherRP->m_lon ) < 1.e-6 ) IsSame = true;
    }
    return IsSame;
}

bool RoutePoint::SendToGPS(const wxString & com_name, wxGauge *pProgress)
{
    bool result = false;
    if( g_pMUX ) result = g_pMUX->SendWaypointToGPS( this, com_name, pProgress );

    wxString msg;
    if( result ) msg = _("Waypoint(s) Uploaded successfully.");
    else
        msg = _("Error on Waypoint Upload.  Please check logfiles...");

    OCPNMessageBox( NULL, msg, _("OpenCPN Info"), wxOK | wxICON_INFORMATION );

    return result;
}

