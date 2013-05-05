/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions
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

#include <wx/tokenzr.h>
#include <wx/sstream.h>
#include <wx/image.h>
#include <wx/filename.h>
#include <wx/graphics.h>
#include <wx/dir.h>

#include "dychart.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <locale>
#include <deque>

#include <wx/listimpl.cpp>
#include <wx/progdlg.h>

#include "chart1.h"
#include "navutil.h"
#include "chcanv.h"
#include "georef.h"
#include "cutil.h"
#include "styles.h"
#include "routeman.h"
#include "routeprop.h"
#include "s52utils.h"
#include "chartbase.h"
#include "tinyxml.h"
#include "gpxdocument.h"
#include "ocpndc.h"
#include "geodesic.h"
#include "datastream.h"
#include "multiplexer.h"
#include "ais.h"
#include "Route.h"
#include "Select.h"
#include "FontMgr.h"
#include "OCPN_Sound.h"
#include "Layer.h"
#include "NavObjectCollection.h"
#include "NMEALogWindow.h"

#ifdef USE_S57
#include "s52plib.h"
#include "cm93.h"
#endif

//    Statics

extern ChartCanvas      *cc1;
extern MyFrame          *gFrame;
extern FontMgr          *pFontMgr;

extern double           g_ChartNotRenderScaleFactor;
extern int              g_restore_stackindex;
extern int              g_restore_dbindex;
extern RouteList        *pRouteList;
extern LayerList        *pLayerList;
extern bool             g_bIsNewLayer;
extern int              g_LayerIdx;
extern bool             g_bLayerViz;
extern Select           *pSelect;
extern MyConfig         *pConfig;
extern ArrayOfCDI       g_ChartDirArray;
extern double           vLat, vLon, gLat, gLon;
extern double           kLat, kLon;
extern double           initial_scale_ppm;
extern ColorScheme      global_color_scheme;
extern int              g_nbrightness;

extern wxToolBarBase    *toolBar;

extern wxArrayOfConnPrm *g_pConnectionParams;

extern wxString         g_csv_locn;
extern wxString         g_SENCPrefix;
extern wxString         g_UserPresLibData;

extern AIS_Decoder      *g_pAIS;
extern wxString         g_SData_Locn;
extern wxString         *pInit_Chart_Dir;
extern WayPointman      *pWayPointMan;
extern Routeman         *g_pRouteMan;
//extern ComPortManager   *g_pCommMan;
extern RouteProp        *pRoutePropDialog;

extern bool             s_bSetSystemTime;
extern bool             g_bDisplayGrid;         //Flag indicating if grid is to be displayed
extern bool             g_bPlayShipsBells;
extern bool             g_bFullscreenToolbar;
extern bool             g_bShowLayers;
extern bool             g_bTransparentToolbar;
extern bool             g_bPermanentMOBIcon;

extern bool             g_bShowDepthUnits;
extern bool             g_bAutoAnchorMark;
extern bool             g_bskew_comp;
extern bool             g_bopengl;
extern bool             g_bsmoothpanzoom;

extern bool             g_bShowOutlines;
extern bool             g_bShowActiveRouteHighway;
extern int              g_nNMEADebug;
extern int              g_nAWDefault;
extern int              g_nAWMax;
extern int              g_nTrackPrecision;

extern int              g_iSDMMFormat;
extern int              g_iDistanceFormat;
extern int              g_iSpeedFormat;

extern int              g_nframewin_x;
extern int              g_nframewin_y;
extern int              g_nframewin_posx;
extern int              g_nframewin_posy;
extern bool             g_bframemax;

extern double           g_PlanSpeed;
extern wxString         g_VisibleLayers;
extern wxString         g_InvisibleLayers;
extern wxRect           g_blink_rect;

extern wxArrayString    *pMessageOnceArray;

//    AIS Global configuration
extern bool             g_bCPAMax;
extern double           g_CPAMax_NM;
extern bool             g_bCPAWarn;
extern double           g_CPAWarn_NM;
extern bool             g_bTCPA_Max;
extern double           g_TCPA_Max;
extern bool             g_bMarkLost;
extern double           g_MarkLost_Mins;
extern bool             g_bRemoveLost;
extern double           g_RemoveLost_Mins;
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bAISShowTracks;
extern bool             g_bTrackCarryOver;
extern bool             g_bTrackDaily;
extern double           g_AISShowTracks_Mins;
extern bool             g_bShowMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bAIS_CPA_Alert;
extern bool             g_bAIS_CPA_Alert_Audio;
extern int              g_ais_alert_dialog_x, g_ais_alert_dialog_y;
extern int              g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
extern int              g_ais_query_dialog_x, g_ais_query_dialog_y;
extern wxString         g_sAIS_Alert_Sound_File;
extern bool             g_bAIS_CPA_Alert_Suppress_Moored;
extern bool             g_bAIS_ACK_Timeout;
extern double           g_AckTimeout_Mins;
extern wxString         g_AisTargetList_perspective;
extern int              g_AisTargetList_range;
extern int              g_AisTargetList_sortColumn;
extern bool             g_bAisTargetList_sortReverse;
extern wxString         g_AisTargetList_column_spec;
extern bool             g_bShowAreaNotices;
extern bool             g_bDrawAISSize;
extern bool             g_bShowAISName;
extern int              g_Show_Target_Name_Scale;

extern int              g_S57_dialog_sx, g_S57_dialog_sy;

extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern bool             g_bWayPointPreventDragging;
extern bool             g_bConfirmObjectDelete;

extern bool             g_bEnableZoomToCursor;
extern wxString         g_toolbarConfig;
extern double           g_TrackIntervalSeconds;
extern double           g_TrackDeltaDistance;
extern int              gps_watchdog_timeout_ticks;

extern int              g_nCacheLimit;
extern int              g_memCacheLimit;

extern bool             g_bGDAL_Debug;
extern bool             g_bDebugCM93;
extern bool             g_bDebugS57;

extern double           g_ownship_predictor_minutes;

#ifdef USE_S57
extern s52plib          *ps52plib;
#endif

extern int              g_cm93_zoom_factor;
extern bool             g_bShowCM93DetailSlider;
extern int              g_cm93detail_dialog_x, g_cm93detail_dialog_y;

extern bool             g_bUseGreenShip;

extern bool             g_b_overzoom_x;                      // Allow high overzoom
extern bool             g_bshow_overzoom_emboss;
extern int              g_nautosave_interval_seconds;
extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern int              g_n_ownship_min_mm;

extern bool             g_bPreserveScaleOnX;

extern bool             g_bUseRMC;
extern bool             g_bUseGLL;

extern wxString         g_locale;

extern bool             g_bUseRaster;
extern bool             g_bUseVector;
extern bool             g_bUseCM93;

extern bool             g_bCourseUp;
extern bool             g_bLookAhead;
extern int              g_COGAvgSec;

extern int              g_MemFootSec;
extern int              g_MemFootMB;

extern int              g_nCOMPortCheck;

extern bool             g_bbigred;

extern wxString         g_AW1GUID;
extern wxString         g_AW2GUID;
extern int              g_BSBImgDebug;

extern int             n_NavMessageShown;
extern wxString        g_config_version_string;

extern bool             g_bAISRolloverShowClass;
extern bool             g_bAISRolloverShowCOG;
extern bool             g_bAISRolloverShowCPA;

extern bool             g_blocale_changed;
extern bool             g_bDebugGPSD;

extern bool             g_bfilter_cogsog;
extern int              g_COGFilterSec;
extern int              g_SOGFilterSec;

int                     g_navobjbackups;

extern bool             g_bQuiltEnable;
extern bool             g_bFullScreenQuilt;
extern bool             g_bQuiltStart;

extern int              g_SkewCompUpdatePeriod;

extern int              g_toolbar_x;
extern int              g_toolbar_y;
extern long             g_toolbar_orient;

extern int              g_GPU_MemSize;

extern int              g_lastClientRectx;
extern int              g_lastClientRecty;
extern int              g_lastClientRectw;
extern int              g_lastClientRecth;

extern bool             g_bHighliteTracks;

extern int              g_route_line_width;
extern int              g_track_line_width;
extern wxString         g_default_wp_icon;

extern ChartGroupArray  *g_pGroupArray;
extern int              g_GroupIndex;

extern bool             g_bDebugOGL;
extern int              g_current_arrow_scale;
extern wxString         g_GPS_Ident;
extern bool             g_bGarminHostUpload;
extern wxString         g_uploadConnection;

extern ocpnStyle::StyleManager* g_StyleManager;
extern wxArrayString    TideCurrentDataSet;
extern wxString         g_TCData_Dir;
extern Multiplexer      *g_pMUX;
extern bool             portaudio_initialized;

//---------------------------------------------------------------------------------
//    Track Implementation
//---------------------------------------------------------------------------------

#define TIMER_TRACK1           778

BEGIN_EVENT_TABLE ( Track, wxEvtHandler )
    EVT_TIMER ( TIMER_TRACK1, Track::OnTimerTrack )
END_EVENT_TABLE()

Track::Track( void )
{
    m_TimerTrack.SetOwner( this, TIMER_TRACK1 );
    m_TimerTrack.Stop();
    m_bRunning = false;
    m_bIsTrack = true;

    SetPrecision( g_nTrackPrecision );

    m_prev_time = wxInvalidDateTime;
    m_lastStoredTP = NULL;

    wxDateTime now = wxDateTime::Now();
    m_ConfigRouteNum = now.GetTicks();        // a unique number....
    trackPointState = firstPoint;
    m_lastStoredTP = NULL;
    m_removeTP = NULL;
    m_fixedTP = NULL;
    m_track_run = 0;
}

Track::~Track()
{
    Stop();
}

void Track::SetPrecision( int prec ) {
    m_nPrecision = prec;
    switch( m_nPrecision ) {
        case 0: { // Low
            m_allowedMaxAngle = 10;
            m_allowedMaxXTE = 0.008;
            m_TrackTimerSec = 8;
            m_minTrackpoint_delta = .004;
            break;
        }
        case 1: { // Medium
            m_allowedMaxAngle = 10;
            m_allowedMaxXTE = 0.004;
            m_TrackTimerSec = 4;
            m_minTrackpoint_delta = .002;
            break;
        }
        case 2: { // High
            m_allowedMaxAngle = 10;
            m_allowedMaxXTE = 0.0015;
            m_TrackTimerSec = 2;
            m_minTrackpoint_delta = .001;
            break;
        }
    }
}

void Track::Start( void )
{
    if( !m_bRunning ) {
        AddPointNow( true );                   // Add initial point
        m_TimerTrack.Start( 1000, wxTIMER_CONTINUOUS );
        m_bRunning = true;
    }
}

void Track::Stop( bool do_add_point )
{
    double delta = 0.0;
    if( m_lastStoredTP )
        delta = DistGreatCircle( gLat, gLon, m_lastStoredTP->m_lat, m_lastStoredTP->m_lon );

    if( ( m_bRunning ) && ( ( delta > m_minTrackpoint_delta ) || do_add_point ) ) AddPointNow(
            true );                   // Add last point

    m_TimerTrack.Stop();
    m_bRunning = false;
    m_track_run = 0;
}

bool Track::DoExtendDaily()
{
    Route *pExtendRoute = NULL;
    RoutePoint *pExtendPoint = NULL;

    RoutePoint *pLastPoint = this->GetPoint( 1 );

    wxRouteListNode *route_node = pRouteList->GetFirst();
    while( route_node ) {
        Route *proute = route_node->GetData();

        if( !proute->m_bIsInLayer && proute->m_bIsTrack && proute->m_GUID != this->m_GUID ) {
            RoutePoint *track_node = proute->GetLastPoint();
            if( track_node->m_CreateTime <= pLastPoint->m_CreateTime ) if( !pExtendPoint
                    || track_node->m_CreateTime > pExtendPoint->m_CreateTime ) {
                pExtendPoint = track_node;
                pExtendRoute = proute;
            }
        }
        route_node = route_node->GetNext();                         // next route
    }
    if( pExtendRoute
            && pExtendRoute->GetPoint( 1 )->m_CreateTime.FromTimezone( wxDateTime::GMT0 ).IsSameDate(
                    pLastPoint->m_CreateTime.FromTimezone( wxDateTime::GMT0 ) ) ) {
        int begin = 1;
        if( pLastPoint->m_CreateTime == pExtendPoint->m_CreateTime ) begin = 2;
        pSelect->DeleteAllSelectableTrackSegments( pExtendRoute );
        wxString suffix = _T("");
        if( this->m_RouteNameString.IsNull() ) {
            suffix = pExtendRoute->m_RouteNameString;
            if( suffix.IsNull() ) suffix = wxDateTime::Today().FormatISODate();
        }
        pExtendRoute->CloneTrack( this, begin, this->GetnPoints(), suffix );
        pSelect->AddAllSelectableTrackSegments( pExtendRoute );
        pSelect->DeleteAllSelectableTrackSegments( this );
        this->ClearHighlights();
        return true;
    } else {
        if( this->m_RouteNameString.IsNull() ) this->m_RouteNameString =
                wxDateTime::Today().FormatISODate();
        return false;
    }
}

void Track::FixMidnight( Track *pPreviousTrack )
{
    RoutePoint *pMidnightPoint = pPreviousTrack->GetLastPoint();
    CloneAddedTrackPoint( m_lastStoredTP, pMidnightPoint );
    m_prev_time = pMidnightPoint->m_CreateTime.FromUTC();
}

void Track::OnTimerTrack( wxTimerEvent& event )
{
    m_TimerTrack.Stop();
    m_track_run++;

    if( m_lastStoredTP )
        m_prev_dist = DistGreatCircle( gLat, gLon, m_lastStoredTP->m_lat, m_lastStoredTP->m_lon );
    else
        m_prev_dist = 999.0;

    bool b_addpoint = false;

    if( ( m_TrackTimerSec > 0. ) && ( (double) m_track_run >= m_TrackTimerSec )
            && ( m_prev_dist > m_minTrackpoint_delta ) ) {
        b_addpoint = true;
        m_track_run = 0;
    }

    if( b_addpoint )
        AddPointNow();
    else   //continuously update track beginning point timestamp if no movement.
        if( ( trackPointState == firstPoint ) && !g_bTrackDaily )
        {
            wxDateTime now = wxDateTime::Now();
            pRoutePointList->GetFirst()->GetData()->m_CreateTime = now.ToUTC();
        }

    m_TimerTrack.Start( 1000, wxTIMER_CONTINUOUS );
}

RoutePoint* Track::AddNewPoint( vector2D point, wxDateTime time ) {
    RoutePoint *rPoint = new RoutePoint( point.lat, point.lon, wxString( _T ( "empty" ) ),
            wxString( _T ( "" ) ), GPX_EMPTY_STRING );
    rPoint->m_bShowName = false;
    rPoint->m_bIsVisible = true;
    rPoint->m_GPXTrkSegNo = 1;
    rPoint->m_CreateTime = time;
    AddPoint( rPoint );

    //    This is a hack, need to undo the action of Route::AddPoint
    rPoint->m_bIsInRoute = false;
    rPoint->m_bIsInTrack = true;
    return rPoint;
}

void Track::AddPointNow( bool do_add_point )
{
    static std::vector<RoutePoint> skippedPoints;

    wxDateTime now = wxDateTime::Now();

    if( m_prev_dist < 0.0005 )              // avoid zero length segs
        if( !do_add_point ) return;

    if( m_prev_time.IsValid() ) if( m_prev_time == now )                    // avoid zero time segs
        if( !do_add_point ) return;

    vector2D gpsPoint( gLon, gLat );

    // The dynamic interval algorithm will gather all track points in a queue,
    // and analyze the cross track errors for each point before actually adding
    // a point to the track.

    switch( trackPointState ) {
        case firstPoint: {
            RoutePoint *pTrackPoint = AddNewPoint( gpsPoint, now.ToUTC() );
            m_lastStoredTP = pTrackPoint;
            trackPointState = secondPoint;
            do_add_point = false;
            break;
        }
        case secondPoint: {
            vector2D pPoint( gLon, gLat );
            skipPoints.push_back( pPoint );
            skipTimes.push_back( now.ToUTC() );
            trackPointState = potentialPoint;
            break;
        }
        case potentialPoint: {
            if( gpsPoint == skipPoints[skipPoints.size()-1] ) break;

            unsigned int xteMaxIndex = 0;
            double xteMax = 0;

            // Scan points skipped so far and see if anyone has XTE over the threshold.
            for( unsigned int i=0; i<skipPoints.size(); i++ ) {
                double xte = GetXTE( m_lastStoredTP->m_lat, m_lastStoredTP->m_lon, gLat, gLon, skipPoints[i].lat, skipPoints[i].lon );
                if( xte > xteMax ) {
                    xteMax = xte;
                    xteMaxIndex = i;
                }
            }
            if( xteMax > m_allowedMaxXTE ) {
                RoutePoint *pTrackPoint = AddNewPoint( skipPoints[xteMaxIndex], skipTimes[xteMaxIndex] );
                pSelect->AddSelectableTrackSegment( m_lastStoredTP->m_lat, m_lastStoredTP->m_lon,
                        pTrackPoint->m_lat, pTrackPoint->m_lon,
                        m_lastStoredTP, pTrackPoint, this );

                m_prevFixedTP = m_fixedTP;
                m_fixedTP = m_removeTP;
                m_removeTP = m_lastStoredTP;
                m_lastStoredTP = pTrackPoint;
                for( unsigned int i=0; i<=xteMaxIndex; i++ ) {
                    skipPoints.pop_front();
                    skipTimes.pop_front();
                }

                // Now back up and see if we just made 3 points in a straight line and the middle one
                // (the next to last) point can possibly be eliminated. Here we reduce the allowed
                // XTE as a function of leg length. (Half the XTE for very short legs).
                if( GetnPoints() > 2 ) {
                    double dist = DistGreatCircle( m_fixedTP->m_lat, m_fixedTP->m_lon, m_lastStoredTP->m_lat, m_lastStoredTP->m_lon );
                    double xte = GetXTE( m_fixedTP, m_lastStoredTP, m_removeTP );
                    if( xte < m_allowedMaxXTE / wxMax(1.0, 2.0 - dist*2.0) ) {
                        pRoutePointList->pop_back();
                        pRoutePointList->pop_back();
                        pRoutePointList->push_back( m_lastStoredTP );
                        SetnPoints();
                        pSelect->DeletePointSelectableTrackSegments( m_removeTP );
                        pSelect->AddSelectableTrackSegment( m_fixedTP->m_lat, m_fixedTP->m_lon,
                                m_lastStoredTP->m_lat, m_lastStoredTP->m_lon,
                                m_fixedTP, m_lastStoredTP, this );
                        delete m_removeTP;
                        m_removeTP = m_fixedTP;
                        m_fixedTP = m_prevFixedTP;
                    }
                }
            }

            skipPoints.push_back( gpsPoint );
            skipTimes.push_back( now.ToUTC() );
            break;
        }
    }

    // Check if this is the last point of the track.
    if( do_add_point ) {
        RoutePoint *pTrackPoint = AddNewPoint( gpsPoint, now.ToUTC() );
        pSelect->AddSelectableTrackSegment( m_lastStoredTP->m_lat, m_lastStoredTP->m_lon,
                pTrackPoint->m_lat, pTrackPoint->m_lon,
                m_lastStoredTP, pTrackPoint, this );
    }

    m_prev_time = now;
}

void Track::Draw( ocpnDC& dc, ViewPort &VP )
{
    if( !IsVisible() || GetnPoints() == 0 ) return;
/*
    if( m_bRunning ) {                                       // pjotrc 2010.02.26
        dc.SetBrush( wxBrush( GetGlobalColor( _T ( "URED" ) ) ) );
        wxPen dPen( GetGlobalColor( _T ( "URED" ) ), g_track_line_width );
        dc.SetPen( dPen );
    } else {
        dc.SetBrush( wxBrush( GetGlobalColor( _T ( "CHMGD" ) ) ) );
        wxPen dPen( GetGlobalColor( _T ( "CHMGD" ) ), g_track_line_width );
        dc.SetPen( dPen );
    }
*/
    double radius = 0.;
    if( g_bHighliteTracks ) {
        double radius_meters = 20; //Current_Ch->GetNativeScale() * .0015;         // 1.5 mm at original scale
        radius = radius_meters * VP.view_scale_ppm;
    }

    unsigned short int FromSegNo = 1;


    wxRoutePointListNode *node = pRoutePointList->GetFirst();
    RoutePoint *prp = node->GetData();

    //  Establish basic colour
    wxColour basic_colour;
    if( m_bRunning || prp->m_IconName.StartsWith( _T("xmred") ) ) {
            basic_colour = GetGlobalColor( _T ( "URED" ) );
    } else
        if( prp->m_IconName.StartsWith( _T("xmblue") ) ) {
                basic_colour = GetGlobalColor( _T ( "BLUE3" ) );
        } else
            if( prp->m_IconName.StartsWith( _T("xmgreen") ) ) {
                    basic_colour = GetGlobalColor( _T ( "UGREN" ) );
            } else {
                    basic_colour = GetGlobalColor( _T ( "CHMGD" ) );
            }

    int style = wxSOLID;
    int width = g_route_line_width;
    wxColour col;
    if( m_style != STYLE_UNDEFINED )
        style = m_style;
    if( m_width != STYLE_UNDEFINED )
        width = m_width;
    if( m_Colour == wxEmptyString ) {
        col = basic_colour;
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

    //  Draw the first point
    wxPoint rpt, rptn;
    DrawPointWhich( dc, 1, &rpt );

    node = node->GetNext();
    while( node ) {
        RoutePoint *prp = node->GetData();
        unsigned short int ToSegNo = prp->m_GPXTrkSegNo;

/*
        if( m_bRunning || prp->m_IconName.StartsWith( _T("xmred") ) ) {         // pjotrc 2010.02.26
            dc.SetBrush( wxBrush( GetGlobalColor( _T ( "URED" ) ) ) );
            wxPen dPen( GetGlobalColor( _T ( "URED" ) ), g_track_line_width );
            dc.SetPen( dPen );
        } else
            if( prp->m_IconName.StartsWith( _T("xmblue") ) ) {                  // pjotrc 2010.02.26
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "BLUE3" ) ) ) );
                wxPen dPen( GetGlobalColor( _T ( "BLUE3" ) ), g_track_line_width );
                dc.SetPen( dPen );
            } else
                if( prp->m_IconName.StartsWith( _T("xmgreen") ) ) {             // pjotrc 2010.02.26
                    dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UGREN" ) ) ) );
                    wxPen dPen( GetGlobalColor( _T ( "UGREN" ) ), g_track_line_width );
                    dc.SetPen( dPen );
                } else {                                                      // pjotrc 2010.03.02
                    dc.SetBrush( wxBrush( GetGlobalColor( _T ( "CHMGD" ) ) ) );
                    wxPen dPen( GetGlobalColor( _T ( "CHMGD" ) ), g_track_line_width );
                    dc.SetPen( dPen );
                }
*/

        prp->Draw( dc, &rptn );

        if( ToSegNo == FromSegNo )
            RenderSegment( dc, rpt.x, rpt.y, rptn.x, rptn.y, VP, false, (int) radius ); // no arrows, with hilite

        rpt = rptn;

        node = node->GetNext();
        FromSegNo = ToSegNo;

    }

    //    Draw last segment, dynamically, maybe.....

    if( m_bRunning ) {
        wxPoint r;
        cc1->GetCanvasPointPix( gLat, gLon, &r );
        RenderSegment( dc, rpt.x, rpt.y, r.x, r.y, VP, false, (int) radius ); // no arrows, with hilite
    }
}

Route *Track::RouteFromTrack( wxProgressDialog *pprog )
{

    Route *route = new Route();
    wxRoutePointListNode *prpnode = pRoutePointList->GetFirst();
    RoutePoint *pWP_src = prpnode->GetData();
    wxRoutePointListNode *prpnodeX;
    RoutePoint *pWP_dst;
    RoutePoint *prp_OK = NULL;  // last routepoint known not to exceed xte limit, if not yet added

    wxString icon = _T("xmblue");
    if( g_TrackDeltaDistance >= 0.1 ) icon = _T("diamond");

    int ic = 0;
    int next_ic = 0;
    int back_ic = 0;
    int nPoints = pRoutePointList->GetCount();
    bool isProminent = true;
    double delta_dist, delta_hdg, xte;
    double leg_speed = 0.1;

    if( pRoutePropDialog ) leg_speed = pRoutePropDialog->m_planspeed;
    else
        leg_speed = g_PlanSpeed;

// add first point

    pWP_dst = new RoutePoint( pWP_src->m_lat, pWP_src->m_lon, icon, _T ( "" ), GPX_EMPTY_STRING );
    route->AddPoint( pWP_dst );

    pWP_dst->m_bShowName = false;

    pSelect->AddSelectableRoutePoint( pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst );

// add intermediate points as needed

    prpnode = prpnode->GetNext();

    while( prpnode ) {
        RoutePoint *prp = prpnode->GetData();
        prpnodeX = prpnode;
        pWP_dst = pWP_src;

        delta_dist = 0.0;
        delta_hdg = 0.0;
        back_ic = next_ic;

        DistanceBearingMercator( prp->m_lat, prp->m_lon, pWP_src->m_lat, pWP_src->m_lon, &delta_hdg,
                &delta_dist );

        if( ( delta_dist > ( leg_speed * 6.0 ) ) && !prp_OK ) {
            int delta_inserts = floor( delta_dist / ( leg_speed * 4.0 ) );
            delta_dist = delta_dist / ( delta_inserts + 1 );
            double tlat = 0.0;
            double tlon = 0.0;

            while( delta_inserts-- ) {
                ll_gc_ll( pWP_src->m_lat, pWP_src->m_lon, delta_hdg, delta_dist, &tlat, &tlon );
                pWP_dst = new RoutePoint( tlat, tlon, icon, _T ( "" ), GPX_EMPTY_STRING );
                route->AddPoint( pWP_dst );
                pWP_dst->m_bShowName = false;
                pSelect->AddSelectableRoutePoint( pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst );

                pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP_dst->m_lat,
                        pWP_dst->m_lon, pWP_src, pWP_dst, route );

                pWP_src = pWP_dst;
            }
            prpnodeX = prpnode;
            pWP_dst = pWP_src;
            next_ic = 0;
            delta_dist = 0.0;
            back_ic = next_ic;
            prp_OK = prp;
            isProminent = true;
        } else {
            isProminent = false;
            if( delta_dist >= ( leg_speed * 4.0 ) ) isProminent = true;
            if( !prp_OK ) prp_OK = prp;
        }

        while( prpnodeX ) {

            RoutePoint *prpX = prpnodeX->GetData();
            xte = GetXTE( pWP_src, prpX, prp );
            if( isProminent || ( xte > g_TrackDeltaDistance ) ) {

                pWP_dst = new RoutePoint( prp_OK->m_lat, prp_OK->m_lon, icon, _T ( "" ),
                        GPX_EMPTY_STRING );

                route->AddPoint( pWP_dst );
                pWP_dst->m_bShowName = false;

                pSelect->AddSelectableRoutePoint( pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst );

                pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP_dst->m_lat,
                        pWP_dst->m_lon, pWP_src, pWP_dst, route );

                pWP_src = pWP_dst;
                next_ic = 0;
                prpnodeX = NULL;
                prp_OK = NULL;
            }

            if( prpnodeX ) prpnodeX = prpnodeX->GetPrevious();
            if( back_ic-- <= 0 ) {
                prpnodeX = NULL;
            }
        }

        if( prp_OK ) {
            prp_OK = prp;
        }

        DistanceBearingMercator( prp->m_lat, prp->m_lon, pWP_src->m_lat, pWP_src->m_lon, NULL,
                &delta_dist );

        if( !( ( delta_dist > ( g_TrackDeltaDistance ) ) && !prp_OK ) ) {
            prpnode = prpnode->GetNext(); //RoutePoint
            next_ic++;
        }
        ic++;
        if( pprog ) pprog->Update( ( ic * 100 ) / nPoints );
    }

// add last point, if needed
    if( delta_dist >= g_TrackDeltaDistance ) {
        pWP_dst = new RoutePoint( pRoutePointList->GetLast()->GetData()->m_lat,
                pRoutePointList->GetLast()->GetData()->m_lon, icon, _T ( "" ), GPX_EMPTY_STRING );
        route->AddPoint( pWP_dst );

        pWP_dst->m_bShowName = false;

        pSelect->AddSelectableRoutePoint( pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst );

        pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP_dst->m_lat,
                pWP_dst->m_lon, pWP_src, pWP_dst, route );
    }
    route->m_RouteNameString = m_RouteNameString;
    route->m_RouteStartString = m_RouteStartString;
    route->m_RouteEndString = m_RouteEndString;
    route->m_bDeleteOnArrival = false;

    return route;
}

void Track::DouglasPeuckerReducer( std::vector<RoutePoint*>& list, int from, int to, double delta ) {
    list[from]->m_bIsActive = true;
    list[to]->m_bIsActive = true;

    int maxdistIndex = -1;
    double maxdist = 0;

    for( int i=from+1; i<to; i++ ) {

        double dist = 1852.0 * GetXTE( list[from], list[to], list[i] );

        if( dist > maxdist ) {
            maxdist = dist;
            maxdistIndex = i;
        }
    }

    if( maxdist > delta ) {
        DouglasPeuckerReducer( list, from, maxdistIndex, delta );
        DouglasPeuckerReducer( list, maxdistIndex, to, delta );
    }
}

int Track::Simplify( double maxDelta ) {
    int reduction = 0;
    wxRoutePointListNode *pointnode = pRoutePointList->GetFirst();
    RoutePoint *routepoint;
    std::vector<RoutePoint*> pointlist;

    ::wxBeginBusyCursor();

    while( pointnode ) {
        routepoint = pointnode->GetData();
        routepoint->m_bIsActive = false;
        pointlist.push_back(routepoint);
        pointnode = pointnode->GetNext();
    }

    DouglasPeuckerReducer( pointlist, 0, pointlist.size()-1, maxDelta );

    pSelect->DeleteAllSelectableTrackSegments( this );
    pRoutePointList->Clear();

    for( size_t i=0; i<pointlist.size(); i++ ) {
        if( pointlist[i]->m_bIsActive ) {
            pointlist[i]->m_bIsActive = false;
            pRoutePointList->Append( pointlist[i] );
        } else {
            delete pointlist[i];
            reduction++;
        }
    }

    SetnPoints();
    pSelect->AddAllSelectableTrackSegments( this );

    UpdateSegmentDistances();
    ::wxEndBusyCursor();
    return reduction;
}

double _distance2( vector2D& a, vector2D& b ) { return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y); }
double _distance( vector2D& a, vector2D& b ) { return sqrt( _distance2( a, b ) ); }

double Track::GetXTE( double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon, double toLat, double toLon  )
{
    vector2D v, w, p;

    // First we get the cartesian coordinates to the line endpoints, using
    // the current position as origo.

    double brg1, dist1, brg2, dist2;
    DistanceBearingMercator( toLat, toLon, fm1Lat, fm1Lon, &brg1, &dist1 );
    w.x = dist1 * sin( brg1 * PI / 180. );
    w.y = dist1 * cos( brg1 * PI / 180. );

    DistanceBearingMercator( toLat, toLon, fm2Lat, fm2Lon, &brg2, &dist2 );
    v.x = dist2 * sin( brg2 * PI / 180. );
    v.y = dist2 * cos( brg2 * PI / 180. );

    p.x = 0.0; p.y = 0.0;

    const double lengthSquared = _distance2( v, w );
    if ( lengthSquared == 0.0 ) {
        // v == w case
        return _distance( p, v );
    }

    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of origo onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2

    vector2D a = p - v;
    vector2D b = w - v;

    double t = vDotProduct( &a, &b ) / lengthSquared;

    if (t < 0.0) return _distance(p, v);       // Beyond the 'v' end of the segment
    else if (t > 1.0) return _distance(p, w);  // Beyond the 'w' end of the segment
    vector2D projection = v + t * (w - v);     // Projection falls on the segment
    return _distance(p, projection);
}

double Track::GetXTE( RoutePoint *fm1, RoutePoint *fm2, RoutePoint *to )
{
    if( !fm1 || !fm2 || !to ) return 0.0;
    if( fm1 == to ) return 0.0;
    if( fm2 == to ) return 0.0;
    return GetXTE( fm1->m_lat, fm1->m_lon, fm2->m_lat, fm2->m_lon, to->m_lat, to->m_lon );
;
}

// Layer helper function

wxString GetLayerName( int id )
{
    wxString name( _T("unknown layer") );
    if( id <= 0 ) return ( name );
    LayerList::iterator it;
    int index = 0;
    for( it = ( *pLayerList ).begin(); it != ( *pLayerList ).end(); ++it, ++index ) {
        Layer *lay = (Layer *) ( *it );
        if( lay->m_LayerID == id ) return ( lay->m_LayerName );
    }
    return ( name );
}

//-----------------------------------------------------------------------------
//          MyConfig Implementation
//-----------------------------------------------------------------------------

MyConfig::MyConfig( const wxString &appName, const wxString &vendorName,
        const wxString &LocalFileName ) :
        wxFileConfig( appName, vendorName, LocalFileName, wxString( _T ( "" ) ) )
{
    //    Create the default NavObjectCollection FileName
    wxFileName config_file( LocalFileName );
    m_sNavObjSetFile = config_file.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
    m_sNavObjSetFile += _T ( "navobj.xml" );
    m_sNavObjSetChangesFile = m_sNavObjSetFile + _T ( ".changes" );

    m_pNavObjectInputSet = NULL;
    m_pNavObjectChangesSet = new NavObjectCollection();

    m_bIsImporting = false;
    g_bIsNewLayer = false;

    g_pConnectionParams = new wxArrayOfConnPrm();
}

void MyConfig::CreateRotatingNavObjBackup()
{
    //Rotate navobj backups
    if( g_navobjbackups > 0 ) {
        for( int i = g_navobjbackups - 1; i >= 1; i-- )
            if( wxFile::Exists( wxString::Format( _T("%s.%d"), m_sNavObjSetFile.c_str(), i ) ) ) wxCopyFile(
                    wxString::Format( _T("%s.%d"), m_sNavObjSetFile.c_str(), i ),
                    wxString::Format( _T("%s.%d"), m_sNavObjSetFile.c_str(), i + 1 ) );

        if( wxFile::Exists( m_sNavObjSetFile ) ) wxCopyFile( m_sNavObjSetFile,
                wxString::Format( _T("%s.1"), m_sNavObjSetFile.c_str() ) );
    }
    //try to clean the backups the user doesn't want - breaks if he deleted some by hand as it tries to be effective...
    for( int i = g_navobjbackups + 1; i <= 99; i++ )
        if( wxFile::Exists( wxString::Format( _T("%s.%d"), m_sNavObjSetFile.c_str(), i ) ) ) wxRemoveFile(
                wxString::Format( _T("%s.%d"), m_sNavObjSetFile.c_str(), i ) );
        else
            break;
}

int MyConfig::LoadMyConfig( int iteration )
{

    int read_int;
    wxString val;

    int display_width, display_height;
    wxDisplaySize( &display_width, &display_height );

//    Global options and settings
    SetPath( _T ( "/Settings" ) );

    // Some undocumented values
    if( iteration == 0 ) {
        Read( _T ( "ConfigVersionString" ), &g_config_version_string, _T("") );
        Read( _T ( "NavMessageShown" ), &n_NavMessageShown, 0 );
    }

    wxString uiStyle;
    Read( _T ( "UIStyle" ), &uiStyle, wxT("") );
    g_StyleManager->SetStyle( uiStyle );

    if( iteration == 0 ) {
        Read( _T ( "NCacheLimit" ), &g_nCacheLimit, CACHE_N_LIMIT_DEFAULT );

        int mem_limit;
        Read( _T ( "MEMCacheLimit" ), &mem_limit, 0 );

        if(mem_limit > 0)
            g_memCacheLimit = mem_limit * 1024;       // convert from MBytes to kBytes
    }

    Read( _T ( "DebugGDAL" ), &g_bGDAL_Debug, 0 );
    Read( _T ( "DebugNMEA" ), &g_nNMEADebug, 0 );
    Read( _T ( "DebugOpenGL" ), &g_bDebugOGL, 0 );
    Read( _T ( "AnchorWatchDefault" ), &g_nAWDefault, 50 );
    Read( _T ( "AnchorWatchMax" ), &g_nAWMax, 1852 );
    Read( _T ( "GPSDogTimeout" ), &gps_watchdog_timeout_ticks, GPS_TIMEOUT_SECONDS );
    Read( _T ( "DebugCM93" ), &g_bDebugCM93, 0 );
    Read( _T ( "DebugS57" ), &g_bDebugS57, 0 );         // Show LUP and Feature info in object query
    Read( _T ( "DebugBSBImg" ), &g_BSBImgDebug, 0 );
    Read( _T ( "DebugGPSD" ), &g_bDebugGPSD, 0 );

    Read( _T ( "UseGreenShipIcon" ), &g_bUseGreenShip, 0 );
    Read( _T ( "AllowExtremeOverzoom" ), &g_b_overzoom_x, 1 );
    Read( _T ( "ShowOverzoomEmbossWarning" ), &g_bshow_overzoom_emboss, 1 );
    Read( _T ( "AutosaveIntervalSeconds" ), &g_nautosave_interval_seconds, 300 );

    Read( _T ( "GPSIdent" ), &g_GPS_Ident, wxT("Generic") );
    Read( _T ( "UseGarminHostUpload" ),  &g_bGarminHostUpload, 0 );

    Read( _T ( "UseNMEA_RMC" ), &g_bUseRMC, 1 );
    Read( _T ( "UseNMEA_GLL" ), &g_bUseGLL, 1 );
    Read( _T ( "UseBigRedX" ), &g_bbigred, 0 );

    Read( _T ( "FilterNMEA_Avg" ), &g_bfilter_cogsog, 0 );
    Read( _T ( "FilterNMEA_Sec" ), &g_COGFilterSec, 1 );
    g_COGFilterSec = wxMin(g_COGFilterSec, MAX_COGSOG_FILTER_SECONDS);
    g_COGFilterSec = wxMax(g_COGFilterSec, 1);
    g_SOGFilterSec = g_COGFilterSec;

    Read( _T ( "ScreenBrightness" ), &g_nbrightness, 100 );

    Read( _T ( "MemFootprintMgrTimeSec" ), &g_MemFootSec, 60 );
    Read( _T ( "MemFootprintTargetMB" ), &g_MemFootMB, 200 );

    Read( _T ( "WindowsComPortMax" ), &g_nCOMPortCheck, 32 );

    Read( _T ( "ChartQuilting" ), &g_bQuiltEnable, 0 );
    Read( _T ( "ChartQuiltingInitial" ), &g_bQuiltStart, 0 );

    Read( _T ( "UseRasterCharts" ), &g_bUseRaster, 1 );             // default is true......
    Read( _T ( "UseVectorCharts" ), &g_bUseVector, 0 );
    Read( _T ( "UseCM93Charts" ), &g_bUseCM93, 0 );

    Read( _T ( "CourseUpMode" ), &g_bCourseUp, 0 );
    Read( _T ( "COGUPAvgSeconds" ), &g_COGAvgSec, 15 );
    g_COGAvgSec = wxMin(g_COGAvgSec, MAX_COG_AVERAGE_SECONDS);        // Bound the array size
    Read( _T ( "LookAheadMode" ), &g_bLookAhead, 0 );
    Read( _T ( "SkewToNorthUp" ), &g_bskew_comp, 0 );
    Read( _T ( "OpenGL" ), &g_bopengl, 0 );

//#ifdef __WXMAC__
//      g_bopengl = 0;
//#endif

    Read( _T ( "ActiveChartGroup" ), &g_GroupIndex, 0 );

    Read( _T ( "GPUMemorySize" ), &g_GPU_MemSize, 256 );

    Read( _T ( "SmoothPanZoom" ), &g_bsmoothpanzoom, 0 );

    Read( _T ( "ToolbarX"), &g_toolbar_x, 0 );
    Read( _T ( "ToolbarY" ), &g_toolbar_y, 0 );
    Read( _T ( "ToolbarOrient" ), &g_toolbar_orient, wxTB_HORIZONTAL );
    Read( _T ( "ToolbarConfig" ), &g_toolbarConfig );

    Read( _T ( "AnchorWatch1GUID" ), &g_AW1GUID, _T("") );
    Read( _T ( "AnchorWatch2GUID" ), &g_AW2GUID, _T("") );

    Read( _T ( "InitialStackIndex" ), &g_restore_stackindex, 0 );
    Read( _T ( "InitialdBIndex" ), &g_restore_dbindex, -1 );

    Read( _T ( "ChartNotRenderScaleFactor" ), &g_ChartNotRenderScaleFactor, 1.5 );

#ifdef USE_S57
    Read( _T ( "CM93DetailFactor" ), &g_cm93_zoom_factor, 0 );
    g_cm93_zoom_factor = wxMin(g_cm93_zoom_factor,CM93_ZOOM_FACTOR_MAX_RANGE);
    g_cm93_zoom_factor = wxMax(g_cm93_zoom_factor,(-CM93_ZOOM_FACTOR_MAX_RANGE));

    g_cm93detail_dialog_x = Read( _T ( "CM93DetailZoomPosX" ), 200L );
    g_cm93detail_dialog_y = Read( _T ( "CM93DetailZoomPosY" ), 200L );
    if( ( g_cm93detail_dialog_x < 0 ) || ( g_cm93detail_dialog_x > display_width ) ) g_cm93detail_dialog_x =
            5;
    if( ( g_cm93detail_dialog_y < 0 ) || ( g_cm93detail_dialog_y > display_height ) ) g_cm93detail_dialog_y =
            5;

    Read( _T ( "ShowCM93DetailSlider" ), &g_bShowCM93DetailSlider, 0 );
#endif

    Read( _T ( "SkewCompUpdatePeriod" ), &g_SkewCompUpdatePeriod, 10 );

    Read( _T ( "SetSystemTime" ), &s_bSetSystemTime, 0 );
    Read( _T ( "ShowDebugWindows" ), &m_bShowDebugWindows, 1 );
    Read( _T ( "ShowGrid" ), &g_bDisplayGrid, 0 );
    Read( _T ( "PlayShipsBells" ), &g_bPlayShipsBells, 0 );
    Read( _T ( "FullscreenToolbar" ), &g_bFullscreenToolbar, 1 );
    Read( _T ( "TransparentToolbar" ), &g_bTransparentToolbar, 1 );
    Read( _T ( "PermanentMOBIcon" ), &g_bPermanentMOBIcon, 0 );
    Read( _T ( "ShowLayers" ), &g_bShowLayers, 1 );
    Read( _T ( "ShowDepthUnits" ), &g_bShowDepthUnits, 1 );
    Read( _T ( "AutoAnchorDrop" ), &g_bAutoAnchorMark, 0 );
    Read( _T ( "ShowChartOutlines" ), &g_bShowOutlines, 0 );
    Read( _T ( "ShowActiveRouteHighway" ), &g_bShowActiveRouteHighway, 1 );
    Read( _T ( "MostRecentGPSUploadConnection" ), &g_uploadConnection, _T("") );

    Read( _T ( "SDMMFormat" ), &g_iSDMMFormat, 0 ); //0 = "Degrees, Decimal minutes"), 1 = "Decimal degrees", 2 = "Degrees,Minutes, Seconds"
    Read( _T ( "DistanceFormat" ), &g_iDistanceFormat, 0 ); //0 = "Nautical miles"), 1 = "Statute miles", 2 = "Kilometers", 3 = "Meters"
    Read( _T ( "SpeedFormat" ), &g_iSpeedFormat, 0 ); //0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"

    Read( _T ( "OwnshipCOGPredictorMinutes" ), &g_ownship_predictor_minutes, 5 );
    Read( _T ( "OwnShipIconType" ), &g_OwnShipIconType, 0 );
    Read( _T ( "OwnShipLength" ), &g_n_ownship_length_meters, 0 );
    Read( _T ( "OwnShipWidth" ), &g_n_ownship_beam_meters, 0 );
    Read( _T ( "OwnShipGPSOffsetX" ), &g_n_gps_antenna_offset_x, 0 );
    Read( _T ( "OwnShipGPSOffsetY" ), &g_n_gps_antenna_offset_y, 0 );
    Read( _T ( "OwnShipMinSize" ), &g_n_ownship_min_mm, 1 );
    g_n_ownship_min_mm = wxMax(g_n_ownship_min_mm, 1);

    Read( _T ( "FullScreenQuilt" ), &g_bFullScreenQuilt, 1 );

    Read( _T ( "StartWithTrackActive" ), &g_bTrackCarryOver, 0 );
    Read( _T ( "AutomaticDailyTracks" ), &g_bTrackDaily, 0 );
    Read( _T ( "HighlightTracks" ), &g_bHighliteTracks, 1 );

    wxString stps;
    Read( _T ( "PlanSpeed" ), &stps );
    stps.ToDouble( &g_PlanSpeed );

    Read( _T ( "VisibleLayers" ), &g_VisibleLayers );
    Read( _T ( "InvisibleLayers" ), &g_InvisibleLayers );

    Read( _T ( "PreserveScaleOnX" ), &g_bPreserveScaleOnX, 0 );

    if( iteration == 0 ) {
        g_locale = _T("en_US");
        Read( _T ( "Locale" ), &g_locale );
    }

    //We allow 0-99 backups ov navobj.xml
    Read( _T ( "KeepNavobjBackups" ), &g_navobjbackups, 5 );
    if( g_navobjbackups > 99 ) g_navobjbackups = 99;
    if( g_navobjbackups < 0 ) g_navobjbackups = 0;

    NMEALogWindow::Get().SetSize(Read(_T("NMEALogWindowSizeX"), 600L), Read(_T("NMEALogWindowSizeY"), 400L));
    NMEALogWindow::Get().SetPos(Read(_T("NMEALogWindowPosX"), 10L), Read(_T("NMEALogWindowPosY"), 10L));
    NMEALogWindow::Get().CheckPos(display_width, display_height);

    SetPath( _T ( "/Settings/GlobalState" ) );
    Read( _T ( "bFollow" ), &st_bFollow );

    Read( _T ( "FrameWinX" ), &g_nframewin_x );
    Read( _T ( "FrameWinY" ), &g_nframewin_y );
    Read( _T ( "FrameWinPosX" ), &g_nframewin_posx, 0 );
    Read( _T ( "FrameWinPosY" ), &g_nframewin_posy, 0 );
    Read( _T ( "FrameMax" ), &g_bframemax );

    Read( _T ( "ClientPosX" ), &g_lastClientRectx, 0 );
    Read( _T ( "ClientPosY" ), &g_lastClientRecty, 0 );
    Read( _T ( "ClientSzX" ), &g_lastClientRectw, 0 );
    Read( _T ( "ClientSzY" ), &g_lastClientRecth, 0 );

    //    AIS
    wxString s;
    SetPath( _T ( "/Settings/AIS" ) );

    Read( _T ( "bNoCPAMax" ), &g_bCPAMax );

    Read( _T ( "NoCPAMaxNMi" ), &s );
    s.ToDouble( &g_CPAMax_NM );

    Read( _T ( "bCPAWarn" ), &g_bCPAWarn );

    Read( _T ( "CPAWarnNMi" ), &s );
    s.ToDouble( &g_CPAWarn_NM );

    Read( _T ( "bTCPAMax" ), &g_bTCPA_Max );

    Read( _T ( "TCPAMaxMinutes" ), &s );
    s.ToDouble( &g_TCPA_Max );

    Read( _T ( "bMarkLostTargets" ), &g_bMarkLost );

    Read( _T ( "MarkLost_Minutes" ), &s );
    s.ToDouble( &g_MarkLost_Mins );

    Read( _T ( "bRemoveLostTargets" ), &g_bRemoveLost );

    Read( _T ( "RemoveLost_Minutes" ), &s );
    s.ToDouble( &g_RemoveLost_Mins );

    Read( _T ( "bShowCOGArrows" ), &g_bShowCOG );

    Read( _T ( "CogArrowMinutes" ), &s );
    s.ToDouble( &g_ShowCOG_Mins );

    Read( _T ( "bShowTargetTracks" ), &g_bAISShowTracks, 0 );

    if( Read( _T ( "TargetTracksMinutes" ), &s ) ) {
        s.ToDouble( &g_AISShowTracks_Mins );
        g_AISShowTracks_Mins = wxMax(1.0, g_AISShowTracks_Mins);
        g_AISShowTracks_Mins = wxMin(60.0, g_AISShowTracks_Mins);
    } else
        g_AISShowTracks_Mins = 20;

    Read( _T ( "bShowMooredTargets" ), &g_bShowMoored );

    Read( _T ( "MooredTargetMaxSpeedKnots" ), &s );
    s.ToDouble( &g_ShowMoored_Kts );

    Read( _T ( "bShowAreaNotices" ), &g_bShowAreaNotices );
    Read( _T ( "bDrawAISSize" ), &g_bDrawAISSize );
    Read( _T ( "bShowAISName" ), &g_bShowAISName );
    Read( _T ( "bAISAlertDialog" ), &g_bAIS_CPA_Alert );
    g_Show_Target_Name_Scale = Read( _T ( "ShowAISTargetNameScale" ), 250000L );
    g_Show_Target_Name_Scale = wxMax( 5000, g_Show_Target_Name_Scale );

    Read( _T ( "bAISAlertAudio" ), &g_bAIS_CPA_Alert_Audio );
    Read( _T ( "AISAlertAudioFile" ), &g_sAIS_Alert_Sound_File );
    Read( _T ( "bAISAlertSuppressMoored" ), &g_bAIS_CPA_Alert_Suppress_Moored );

    Read( _T ( "bAISAlertAckTimeout" ), &g_bAIS_ACK_Timeout, 0 );
    Read( _T ( "AlertAckTimeoutMinutes" ), &s );
    s.ToDouble( &g_AckTimeout_Mins );

    g_ais_alert_dialog_sx = Read( _T ( "AlertDialogSizeX" ), 200L );
    g_ais_alert_dialog_sy = Read( _T ( "AlertDialogSizeY" ), 200L );
    g_ais_alert_dialog_x = Read( _T ( "AlertDialogPosX" ), 200L );
    g_ais_alert_dialog_y = Read( _T ( "AlertDialogPosY" ), 200L );
    g_ais_query_dialog_x = Read( _T ( "QueryDialogPosX" ), 200L );
    g_ais_query_dialog_y = Read( _T ( "QueryDialogPosY" ), 200L );

    if( ( g_ais_alert_dialog_x < 0 ) || ( g_ais_alert_dialog_x > display_width ) ) g_ais_alert_dialog_x =
            5;
    if( ( g_ais_alert_dialog_y < 0 ) || ( g_ais_alert_dialog_y > display_height ) ) g_ais_alert_dialog_y =
            5;

    if( ( g_ais_query_dialog_x < 0 ) || ( g_ais_query_dialog_x > display_width ) ) g_ais_query_dialog_x =
            5;
    if( ( g_ais_query_dialog_y < 0 ) || ( g_ais_query_dialog_y > display_height ) ) g_ais_query_dialog_y =
            5;

    Read( _T ( "AISTargetListPerspective" ), &g_AisTargetList_perspective );
    g_AisTargetList_range = Read( _T ( "AISTargetListRange" ), 40L );
    g_AisTargetList_sortColumn = Read( _T ( "AISTargetListSortColumn" ), 2L ); // Column #2 is MMSI
    Read( _T ( "bAISTargetListSortReverse" ), &g_bAisTargetList_sortReverse, false );
    Read( _T ( "AISTargetListColumnSpec" ), &g_AisTargetList_column_spec );

    Read( _T ( "bAISRolloverShowClass" ), &g_bAISRolloverShowClass );
    Read( _T ( "bAISRolloverShowCOG" ), &g_bAISRolloverShowCOG );
    Read( _T ( "bAISRolloverShowCPA" ), &g_bAISRolloverShowCPA );

    g_S57_dialog_sx = Read( _T ( "S57QueryDialogSizeX" ), 400L );
    g_S57_dialog_sy = Read( _T ( "S57QueryDialogSizeY" ), 400L );

#ifdef USE_S57
    if( NULL != ps52plib ) {
        double dval;
        SetPath( _T ( "/Settings/GlobalState" ) );

        Read( _T ( "bShowS57Text" ), &read_int, 0 );
        ps52plib->SetShowS57Text( !( read_int == 0 ) );

        Read( _T ( "bShowS57ImportantTextOnly" ), &read_int, 0 );
        ps52plib->SetShowS57ImportantTextOnly( !( read_int == 0 ) );

        Read( _T ( "bShowLightDescription" ), &read_int, 0 );
        ps52plib->SetShowLdisText( !( read_int == 0 ) );

        Read( _T ( "bExtendLightSectors" ), &read_int, 0 );
        ps52plib->SetExtendLightSectors( !( read_int == 0 ) );

        Read( _T ( "nDisplayCategory" ), &read_int, (enum _DisCat) STANDARD );
        ps52plib->m_nDisplayCategory = (enum _DisCat) read_int;

        Read( _T ( "nSymbolStyle" ), &read_int, (enum _LUPname) PAPER_CHART );
        ps52plib->m_nSymbolStyle = (LUPname) read_int;

        Read( _T ( "nBoundaryStyle" ), &read_int, PLAIN_BOUNDARIES );
        ps52plib->m_nBoundaryStyle = (LUPname) read_int;

        Read( _T ( "bShowSoundg" ), &read_int, 0 );
        ps52plib->m_bShowSoundg = !( read_int == 0 );

        Read( _T ( "bShowMeta" ), &read_int, 0 );
        ps52plib->m_bShowMeta = !( read_int == 0 );

        Read( _T ( "bUseSCAMIN" ), &read_int, 0 );
        ps52plib->m_bUseSCAMIN = !( read_int == 0 );

        Read( _T ( "bShowAtonText" ), &read_int, 0 );
        ps52plib->m_bShowAtonText = !( read_int == 0 );

        Read( _T ( "bDeClutterText" ), &read_int, 0 );
        ps52plib->m_bDeClutterText = !( read_int == 0 );

        if( Read( _T ( "S52_MAR_SAFETY_CONTOUR" ), &dval, 5.0 ) ) {
            S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval );
            S52_setMarinerParam( S52_MAR_SAFETY_DEPTH, dval ); // Set safety_contour and safety_depth the same
        }

        if( Read( _T ( "S52_MAR_SHALLOW_CONTOUR" ), &dval, 3.0 ) ) S52_setMarinerParam(
                S52_MAR_SHALLOW_CONTOUR, dval );

        if( Read( _T ( "S52_MAR_DEEP_CONTOUR" ), &dval, 10.0 ) ) S52_setMarinerParam(
                S52_MAR_DEEP_CONTOUR, dval );

        if( Read( _T ( "S52_MAR_TWO_SHADES" ), &dval, 0.0 ) ) S52_setMarinerParam(
                S52_MAR_TWO_SHADES, dval );

        ps52plib->UpdateMarinerParams();

        SetPath( _T ( "/Settings/GlobalState" ) );
        Read( _T ( "S52_DEPTH_UNIT_SHOW" ), &read_int, 1 );   // default is metres
        ps52plib->m_nDepthUnitDisplay = read_int;
    }

    wxString strpres( _T ( "PresentationLibraryData" ) );
    wxString valpres;
    SetPath( _T ( "/Directories" ) );
    Read( strpres, &valpres );              // Get the File name
    if( iteration == 0 ) g_UserPresLibData = valpres;

    /*
     wxString strd ( _T ( "S57DataLocation" ) );
     SetPath ( _T ( "/Directories" ) );
     Read ( strd, &val );              // Get the Directory name


     wxString dirname ( val );
     if ( !dirname.IsEmpty() )
     {
     if ( g_pcsv_locn->IsEmpty() )   // on second pass, don't overwrite
     {
     g_pcsv_locn->Clear();
     g_pcsv_locn->Append ( val );
     }
     }
     */
    wxString strs( _T ( "SENCFileLocation" ) );
    SetPath( _T ( "/Directories" ) );
    wxString vals;
    Read( strs, &vals );              // Get the Directory name

    if( iteration == 0 ) g_SENCPrefix = vals;

#endif

    SetPath( _T ( "/Directories" ) );
    wxString vald;
    Read( _T ( "InitChartDir" ), &vald );           // Get the Directory name

    wxString dirnamed( vald );
    if( !dirnamed.IsEmpty() ) {
        if( pInit_Chart_Dir->IsEmpty() )   // on second pass, don't overwrite
        {
            pInit_Chart_Dir->Clear();
            pInit_Chart_Dir->Append( vald );
        }
    }

    Read( _T ( "GPXIODir" ), &m_gpx_path );           // Get the Directory name
    Read( _T ( "TCDataDir" ), &g_TCData_Dir );           // Get the Directory name

    SetPath( _T ( "/Settings/GlobalState" ) );
    Read( _T ( "nColorScheme" ), &read_int, 0 );
    global_color_scheme = (ColorScheme) read_int;

    SetPath( _T ( "/Settings/NMEADataSource" ) );

    wxString connectionconfigs;
    Read ( _T( "DataConnections" ),  &connectionconfigs, wxEmptyString );
    wxArrayString confs = wxStringTokenize(connectionconfigs, _T("|"));
    g_pConnectionParams->Clear();
    for (size_t i = 0; i < confs.Count(); i++)
    {
        ConnectionParams * prm = new ConnectionParams(confs[i]);
        g_pConnectionParams->Add(prm);
    }

    //  Automatically handle the upgrade to DataSources architecture...
    //  Capture Garmin host configuration
    SetPath( _T ( "/Settings" ) );
    int b_garmin_host;
    Read ( _T ( "UseGarminHost" ), &b_garmin_host );

    //  Is there an existing NMEADataSource definition?
    SetPath( _T ( "/Settings/NMEADataSource" ) );
    wxString xSource;
    wxString xRate;
    Read ( _T ( "Source" ), &xSource );
    Read ( _T ( "BaudRate" ), &xRate );
    if(xSource.Len()) {
        wxString port;
        if(xSource.Mid(0, 6) == _T("Serial"))
            port = xSource.Mid(7);
        else
            port = _T("");

        if( port.Len() && (port != _T("None")) && (port != _T("AIS Port (Shared)")) ) {
        //  Look in the ConnectionParams array to see if this port has been defined in the newer style
            bool bfound = false;
            for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
            {
                ConnectionParams *cp = g_pConnectionParams->Item(i);
                if(cp->GetAddressStr() == port) {
                    bfound = true;
                    break;
                }
            }

            if(!bfound) {
                ConnectionParams * prm = new ConnectionParams();
                prm->Baudrate = wxAtoi(xRate);
                prm->Port = port;
                prm->Garmin = (b_garmin_host == 1);

                g_pConnectionParams->Add(prm);

                g_bGarminHostUpload = (b_garmin_host == 1);
            }
        }
        if( iteration == 1 ) {
            Write ( _T ( "Source" ), _T("") );          // clear the old tag
            Write ( _T ( "BaudRate" ), _T("") );
        }
    }

   //  Is there an existing AISPort definition?
    SetPath( _T ( "/Settings/AISPort" ) );
    wxString aSource;
    wxString aRate;
    Read ( _T ( "Port" ), &aSource );
    Read ( _T ( "BaudRate" ), &aRate );
    if(aSource.Len()) {
        wxString port;
        if(aSource.Mid(0, 6) == _T("Serial"))
            port = aSource.Mid(7);
        else
            port = _T("");

        if(port.Len() && port != _T("None") ) {
            //  Look in the ConnectionParams array to see if this port has been defined in the newer style
            bool bfound = false;
            for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
            {
                ConnectionParams *cp = g_pConnectionParams->Item(i);
                if(cp->GetAddressStr() == port) {
                    bfound = true;
                    break;
                }
            }

            if(!bfound) {
                ConnectionParams * prm = new ConnectionParams();
                if( aRate.Len() )
                    prm->Baudrate = wxAtoi(aRate);
                else
                    prm->Baudrate = 38400;              // default for most AIS receivers
                prm->Port = port;

                g_pConnectionParams->Add(prm);
            }
        }

        if( iteration == 1 ) {
            Write ( _T ( "Port" ), _T("") );          // clear the old tag
            Write ( _T ( "BaudRate" ), _T("") );
        }
    }

    //  Is there an existing NMEAAutoPilotPort definition?
    SetPath( _T ( "/Settings/NMEAAutoPilotPort" ) );
    Read ( _T ( "Port" ), &xSource );
    if(xSource.Len()) {
        wxString port;
        if(xSource.Mid(0, 6) == _T("Serial"))
            port = xSource.Mid(7);
        else
            port = _T("");

        if(port.Len() && port != _T("None") ) {
            //  Look in the ConnectionParams array to see if this port has been defined in the newer style
            bool bfound = false;
            ConnectionParams *cp;
            for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
            {
                cp = g_pConnectionParams->Item(i);
                if(cp->GetAddressStr() == port) {
                    bfound = true;
                    break;
                }
            }

            if(!bfound) {
                ConnectionParams * prm = new ConnectionParams();
                prm->Port = port;
                prm->OutputSentenceListType = WHITELIST;
                prm->OutputSentenceList.Add( _T("RMB") );
                prm->Output = true;

                g_pConnectionParams->Add(prm);
            }
            else {                                  // port was found, so make sure it is set for output
                cp->Output = true;
                cp->OutputSentenceListType = WHITELIST;
                cp->OutputSentenceList.Add( _T("RMB") );
            }
        }

        if( iteration == 1 )
            Write ( _T ( "Port" ), _T("") );          // clear the old tag
    }

//    Reasonable starting point
    vLat = START_LAT;                   // display viewpoint
    vLon = START_LON;

    gLat = START_LAT;                   // GPS position, as default
    gLon = START_LON;

    initial_scale_ppm = .0003;        // decent initial value

    SetPath( _T ( "/Settings/GlobalState" ) );
    wxString st;

    if( Read( _T ( "VPLatLon" ), &st ) ) {
        sscanf( st.mb_str( wxConvUTF8 ), "%lf,%lf", &st_lat, &st_lon );

        //    Sanity check the lat/lon...both have to be reasonable.
        if( fabs( st_lon ) < 360. ) {
            while( st_lon < -180. )
                st_lon += 360.;

            while( st_lon > 180. )
                st_lon -= 360.;

            vLon = st_lon;
        }

        if( fabs( st_lat ) < 90.0 ) vLat = st_lat;
    }
    s.Printf( _T ( "Setting Viewpoint Lat/Lon %g, %g" ), vLat, vLon );
    wxLogMessage( s );

    if( Read( wxString( _T ( "VPScale" ) ), &st ) ) {
        sscanf( st.mb_str( wxConvUTF8 ), "%lf", &st_view_scale );
//    Sanity check the scale
        st_view_scale = fmax ( st_view_scale, .001/32 );
        st_view_scale = fmin ( st_view_scale, 4 );
        initial_scale_ppm = st_view_scale;
    }

    wxString sll;
    double lat, lon;
    if( Read( _T ( "OwnShipLatLon" ), &sll ) ) {
        sscanf( sll.mb_str( wxConvUTF8 ), "%lf,%lf", &lat, &lon );

        //    Sanity check the lat/lon...both have to be reasonable.
        if( fabs( lon ) < 360. ) {
            while( lon < -180. )
                lon += 360.;

            while( lon > 180. )
                lon -= 360.;

            gLon = lon;
        }

        if( fabs( lat ) < 90.0 ) gLat = lat;
    }
    s.Printf( _T ( "Setting Ownship Lat/Lon %g, %g" ), gLat, gLon );
    wxLogMessage( s );

#ifdef USE_S57
//    S57 Object Class Visibility

    OBJLElement *pOLE;

    SetPath( _T ( "/Settings/ObjectFilter" ) );

    if( ps52plib ) {
        int iOBJMax = GetNumberOfEntries();
        if( iOBJMax ) {

            wxString str;
            long val;
            long dummy;

            wxString sObj;

            bool bCont = pConfig->GetFirstEntry( str, dummy );
            while( bCont ) {
                pConfig->Read( str, &val );              // Get an Object Viz

                bool bNeedNew = true;

                if( str.StartsWith( _T ( "viz" ), &sObj ) ) {
                    for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
                        pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
                        if( !strncmp( pOLE->OBJLName, sObj.mb_str(), 6 ) ) {
                            pOLE->nViz = val;
                            bNeedNew = false;
                            break;
                        }
                    }

                    if( bNeedNew ) {
                        pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
                        strncpy( pOLE->OBJLName, sObj.mb_str(), 6 );
                        pOLE->nViz = 1;

                        ps52plib->pOBJLArray->Add( (void *) pOLE );
                    }
                }
                bCont = pConfig->GetNextEntry( str, dummy );
            }
        }
    }
#endif

//    Fonts

#ifdef __WXX11__
    SetPath ( _T ( "/Settings/X11Fonts" ) );
#endif

#ifdef __WXGTK__
    SetPath ( _T ( "/Settings/GTKFonts" ) );
#endif

#ifdef __WXMSW__
    SetPath( _T ( "/Settings/MSWFonts" ) );
#endif

#ifdef __WXMAC__
    SetPath ( _T ( "/Settings/MacFonts" ) );
#endif

    if( 0 == iteration ) {
        wxString str;
        long dummy;
        wxString *pval = new wxString;
        wxArrayString deleteList;

        bool bCont = GetFirstEntry( str, dummy );
        while( bCont ) {
            Read( str, pval );

            if( str.StartsWith( _T("Font") ) ) {
                // Convert pre 3.1 setting. Can't delete old entries from inside the
                // GetNextEntry() loop, so we need to save those and delete outside.
                deleteList.Add( str );
                wxString oldKey = pval->BeforeFirst( _T(':') );
                str = FontMgr::GetFontConfigKey( oldKey );
            }

            pFontMgr->LoadFontNative( &str, pval );

            bCont = GetNextEntry( str, dummy );
        }

        for( unsigned int i=0; i<deleteList.Count(); i++ ) {
            DeleteEntry( deleteList[i] );
        }
        deleteList.Clear();
        delete pval;
    }

//  Tide/Current Data Sources
    SetPath( _T ( "/TideCurrentDataSources" ) );
    TideCurrentDataSet.Clear();
    if( GetNumberOfEntries() ) {
        wxString str, val;
        long dummy;
        int iDir = 0;
        bool bCont = GetFirstEntry( str, dummy );
        while( bCont ) {
            Read( str, &val );              // Get a file name
            TideCurrentDataSet.Add(val);
            bCont = GetNextEntry( str, dummy );
        }
    }

//    Routes
    if( 0 == iteration ) {
        int routenum = 0;
        pRouteList = new RouteList;

        SetPath( _T ( "/Routes" ) );
        int iRoutes = GetNumberOfGroups();
        if( iRoutes ) {
            int rnt;

            wxString str, val;
            long dummy;

            bool bCont = GetFirstGroup( str, dummy );
            while( bCont ) {
                Route *pConfRoute = new Route();
                pRouteList->Append( pConfRoute );

                int RouteNum;
                sscanf( str.mb_str(), "RouteDefn%d", &RouteNum );
                pConfRoute->m_ConfigRouteNum = RouteNum;

                SetPath( str );
                Read( _T ( "RoutePoints" ), &val );          // nPoints
                int nPoints = atoi( val.mb_str() );

                Read( _T ( "RouteName" ), &pConfRoute->m_RouteNameString );
                Read( _T ( "RouteStart" ), &pConfRoute->m_RouteStartString );
                Read( _T ( "RouteEnd" ), &pConfRoute->m_RouteEndString );

                //        Get extended properties
                long tmp_prop;
                wxString str_prop;
                Read( _T ( "Properties" ), &str_prop );                    // Properties

                if( !str_prop.IsEmpty() ) {
                    wxStringTokenizer tkp( str_prop, _T ( "," ) );
                    wxString token;

                    token = tkp.GetNextToken();
                    token.ToLong( &tmp_prop );
                    pConfRoute->m_bIsTrack = !( tmp_prop == 0 );

                    token = tkp.GetNextToken();
                    if( token.ToLong( &tmp_prop ) ) pConfRoute->SetVisible( !( tmp_prop == 0 ) );
                    else
                        pConfRoute->SetVisible( true );

                }

                for( int ip = 0; ip < nPoints; ip++ ) {
                    wxString sipc;
                    sipc.Printf( _T ( "RoutePointID%d" ), ip + 1 );
                    wxString str_ID;
                    Read( sipc, &str_ID );
                    if( !str_ID.IsEmpty() ) pConfRoute->AddTentativePoint( str_ID );

                }

                SetPath( _T ( ".." ) );
                bCont = GetNextGroup( str, dummy );

                //    Get next available RouteDefnx number
                sscanf( str.mb_str(), "RouteDefn%d", &rnt );
                if( rnt > routenum ) routenum = rnt;
            }
        }

        m_NextRouteNum = routenum + 1;
    }

    //    Layers
    if( 0 == iteration ) {
//            int laynum = 0;
        pLayerList = new LayerList;
    }

    //    Groups
    if( 0 == iteration ) LoadConfigGroups( g_pGroupArray );

    //    Marks
    if( 0 == iteration ) {
        m_NextWPNum = 0;
        int marknum = 0;

        SetPath( _T ( "/Marks" ) );
        int iMarks = GetNumberOfGroups();
        if( iMarks ) {
            int mnt;

            wxString str, val;
            long dummy;
            double rlat, rlon;

            bool bCont = GetFirstGroup( str, dummy );
            while( bCont ) {
                int MarkNum;
                sscanf( str.mb_str(), "MarkDefn%d", &MarkNum );

                SetPath( str );
                wxString sipb;
                sipb.Printf( _T ( "RoutePoint" ) );
                Read( sipb, &val );                       // Point lat/lon
                double lat = 0.;
                double lon = 0.;
                wxStringTokenizer tkz( val, _T ( "," ) );

                wxString token = tkz.GetNextToken();
                token.ToDouble( &lat );
                rlat = lat;
                token = tkz.GetNextToken();
                token.ToDouble( &lon );
                rlon = lon;

                wxString sipbn = sipb;
                sipbn.Append( _T ( "Name" ) );
                wxString mark_name;
                Read( sipbn, &mark_name );                       // name

                wxString sipbd = sipb;
                sipbd.Append( _T ( "Description" ) );
                wxString mark_description;
                Read( sipbd, &mark_description );                // desc

                wxString sipbi = sipb;
                sipbi.Append( _T ( "Icon" ) );
                wxString icon_name;
                Read( sipbi, &icon_name );                       // icon

                if( icon_name.IsEmpty() ) icon_name = g_default_wp_icon;

                wxString sipb1 = sipb;
                sipb1.Append( _T ( "GUID" ) );
                wxString str_GUID;
                Read( sipb1, &str_GUID );                       // GUID

                //      Normalize the longitude, to fix any old poorly formed points
                if( rlon < -180. ) rlon += 360.;
                else
                    if( rlon > 180. ) rlon -= 360.;

                RoutePoint *pWP = new RoutePoint( rlat, rlon, icon_name, mark_name, str_GUID );
                pWP->m_MarkDescription = mark_description;

                pWP->m_bIsolatedMark = true;                      // This is an isolated mark

//        Get extended properties
                long tmp_prop;
                wxString str_prop;
                wxString sipb2 = sipb;
                sipb2.Append( _T ( "Prop" ) );
                Read( sipb2, &str_prop );                       // Properties

                pWP->SetPropFromString( str_prop );

                sipb2 = sipb;
                sipb2.Append( _T ( "NameLocationOffset" ) );
                Read( sipb2, &str_prop );

                if( !str_prop.IsEmpty() ) {
                    wxStringTokenizer tkpp( str_prop, _T ( "," ) );
                    token = tkpp.GetNextToken();
                    token.ToLong( &tmp_prop );
                    pWP->m_NameLocationOffsetX = tmp_prop;

                    token = tkpp.GetNextToken();
                    token.ToLong( &tmp_prop );
                    pWP->m_NameLocationOffsetY = tmp_prop;
                }

                // Get hyperlinks; toh, 2009.02.23
                wxString str_hyperlinks;
                wxString sipb3 = sipb;
                sipb3.Append( _T ( "Link" ) );

                pWP->m_HyperlinkList->Clear();            // toh, 2010.01.05

                bool cont = true;
                int i = 1;
                while( cont ) {
                    wxString sipb4 = sipb3;
                    wxString buf;
                    buf.Printf( _T ( "%d" ), i );
                    sipb4.Append( buf );

                    cont = Read( sipb4, &str_hyperlinks );                       // hyperlinks

                    if( cont && !str_hyperlinks.IsEmpty() ) {
                        Hyperlink *link;
                        link = new Hyperlink;

                        wxStringTokenizer tkp( str_hyperlinks, _T ( "^" ) );

                        token = tkp.GetNextToken();
                        link->Link = token;

                        token = tkp.GetNextToken();

                        if( token.Length() > 0 ) link->DescrText = token;

                        token = tkp.GetNextToken();

                        if( token.Length() > 0 ) link->Type = token;

                        if( NULL == pWP->m_HyperlinkList ) pWP->m_HyperlinkList = new HyperlinkList;

                        pWP->m_HyperlinkList->Append( link );
                    }
                    i++;
                }

                pSelect->AddSelectableRoutePoint( rlat, rlon, pWP );
                pWP->m_ConfigWPNum = MarkNum;

                SetPath( _T ( ".." ) );
                bCont = GetNextGroup( str, dummy );

                //    Get next available MarkDefnx number
                sscanf( str.mb_str(), "MarkDefn%d", &mnt );
                if( mnt > marknum ) marknum = mnt;
            }
        }
        m_NextWPNum = marknum + 1;

    }

    //    Constitute the routes just loaded
    if( 0 == iteration ) g_pRouteMan->AssembleAllRoutes();

//      next thing to do is read tracks from the NavObject XML file,

    if( 0 == iteration ) {
        CreateRotatingNavObjBackup();

        if( NULL == m_pNavObjectInputSet ) m_pNavObjectInputSet = new NavObjectCollection();

        if( ::wxFileExists( m_sNavObjSetFile ) ) {
            if( m_pNavObjectInputSet->LoadFile( m_sNavObjSetFile ) )
                m_pNavObjectInputSet->LoadAllGPXObjects();
        }

        m_pNavObjectInputSet->Clear();
        delete m_pNavObjectInputSet;

        if( ::wxFileExists( m_sNavObjSetChangesFile ) ) {
            //We crashed last time :(
            //That's why this file still exists...
            //Let's reconstruct the unsaved changes
            NavObjectCollection *pNavObjectChangesSet = new NavObjectCollection();
            pNavObjectChangesSet->LoadFile( m_sNavObjSetChangesFile );
            //reconstruct route changes
            TiXmlElement *gpx_element = pNavObjectChangesSet->RootElement()->FirstChildElement(
                    "rte" );
            while( gpx_element ) {
                Route *pRt = ::LoadGPXRoute( (GpxRteElement *) gpx_element, 0 );
                wxString action = wxString::FromUTF8(
                        gpx_element->FirstChildElement( "extensions" )->FirstChildElement(
                                "opencpn:action" )->GetText() );
                Route *pExisting = RouteExists( pRt->m_GUID );
                if( action == _T("add") ) {
                    m_bIsImporting = true;
                    ::InsertRoute( pRt, -1 );
                    m_bIsImporting = false;
                } else
                    if( action == _T("update") ) {
                        m_bIsImporting = true;
                        ::UpdateRoute( pRt );
                        m_bIsImporting = false;
                    } else
                        if( action == _T("delete") ) {
                            m_bIsImporting = true;
                            if( pExisting ) {
                                g_pRouteMan->DeleteRoute( pExisting );
                            }
                            m_bIsImporting = false;
                        }
                gpx_element = gpx_element->NextSiblingElement( "rte" );
            }
            //reconstruct tracks
            gpx_element = pNavObjectChangesSet->RootElement()->FirstChildElement( "trk" );
            while( gpx_element ) {
                Route *pTrk = ::LoadGPXTrack( (GpxTrkElement *) gpx_element );
                wxString action = wxString::FromUTF8(
                        gpx_element->FirstChildElement( "extensions" )->FirstChildElement(
                                "opencpn:action" )->GetText() );
                Route *pExisting = RouteExists( pTrk->m_GUID );
                //no adds here - the only possible way is logging the gps data
                if( action == _T("update") ) {
                    m_bIsImporting = true;
                    if( pExisting ) {
                        pExisting->m_RouteNameString = pTrk->m_RouteNameString;
                        pExisting->m_RouteStartString = pTrk->m_RouteStartString;
                        pExisting->m_RouteEndString = pTrk->m_RouteEndString;
                    }
                    m_bIsImporting = false;
                } else
                    if( action == _T("delete") ) {
                        m_bIsImporting = true;
                        if( pExisting ) {
                            g_pRouteMan->DeleteTrack( pExisting );
                        }
                        m_bIsImporting = false;
                    }
                gpx_element = gpx_element->NextSiblingElement( "trk" );
            }
            //reconstruct wpt changes
            gpx_element = pNavObjectChangesSet->RootElement()->FirstChildElement( "wpt" );
            while( gpx_element ) {
                RoutePoint *pWp = ::LoadGPXWaypoint( (GpxWptElement *) gpx_element,
                        g_default_wp_icon );
                wxString action = wxString::FromUTF8(
                        gpx_element->FirstChildElement( "extensions" )->FirstChildElement(
                                "opencpn:action" )->GetText() );
                RoutePoint *pExisting = WaypointExists( pWp->m_GUID );
                if( action == wxString( _T("add") ) ) {
                    m_bIsImporting = true;
                    if( !pExisting ) //Should not be needed...
                    if( NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->Append( pWp );
                    pWp->m_bIsolatedMark = true;
                    AddNewWayPoint( pWp, m_NextWPNum );
                    pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                    m_bIsImporting = false;
                } else
                    if( action == wxString( _T("update") ) ) {
                        m_bIsImporting = true;
                        if( pExisting ) pWayPointMan->m_pWayPointList->DeleteObject( pExisting );
                        pWayPointMan->m_pWayPointList->Append( pWp );
                        pWp->m_bIsolatedMark = true;
                        AddNewWayPoint( pWp, m_NextWPNum );
                        pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                        m_bIsImporting = false;
                    } else
                        if( action == wxString( _T("delete") ) ) {
                            m_bIsImporting = true;
                            if( pExisting ) {
                                pWayPointMan->DestroyWaypoint( pExisting );
                            }
                            m_bIsImporting = false;
                        }
                gpx_element = gpx_element->NextSiblingElement( "wpt" );
            }
            UpdateNavObj(); //We save the data before we throw away the log
            delete pNavObjectChangesSet;
        }
    }

    SetPath( _T ( "/Settings/Others" ) );

    // Radar rings
    g_iNavAidRadarRingsNumberVisible = 0;
    Read( _T ( "RadarRingsNumberVisible" ), &val );
    if( val.Length() > 0 ) g_iNavAidRadarRingsNumberVisible = atoi( val.mb_str() );

    g_fNavAidRadarRingsStep = 1.0;
    Read( _T ( "RadarRingsStep" ), &val );
    if( val.Length() > 0 ) g_fNavAidRadarRingsStep = atof( val.mb_str() );

    g_pNavAidRadarRingsStepUnits = 0;
    Read( _T ( "RadarRingsStepUnits" ), &g_pNavAidRadarRingsStepUnits );

    //  Support Version 3.0 and prior config setting for Radar Rings
    bool b300RadarRings= true;
    Read ( _T ( "ShowRadarRings" ), &b300RadarRings );
    if(!b300RadarRings)
        g_iNavAidRadarRingsNumberVisible = 0;

    Read( _T ( "ConfirmObjectDeletion" ), &g_bConfirmObjectDelete, true );

    // Waypoint dragging with mouse
    g_bWayPointPreventDragging = false;
    Read( _T ( "WaypointPreventDragging" ), &g_bWayPointPreventDragging );

    g_bEnableZoomToCursor = false;
    Read( _T ( "EnableZoomToCursor" ), &g_bEnableZoomToCursor );

    g_TrackIntervalSeconds = 60.0;
    val.Clear();
    Read( _T ( "TrackIntervalSeconds" ), &val );
    if( val.Length() > 0 ) {
        double tval = atof( val.mb_str() );
        if( tval >= 2. ) g_TrackIntervalSeconds = tval;
    }

    g_TrackDeltaDistance = 0.10;
    val.Clear();
    Read( _T ( "TrackDeltaDistance" ), &val );
    if( val.Length() > 0 ) {
        double tval = atof( val.mb_str() );
        if( tval >= 0.05 ) g_TrackDeltaDistance = tval;
    }

    Read( _T ( "TrackPrecision" ), &g_nTrackPrecision, 0 );

    Read( _T ( "NavObjectFileName" ), m_sNavObjSetFile );

    Read( _T ( "RouteLineWidth" ), &g_route_line_width, 2 );
    Read( _T ( "TrackLineWidth" ), &g_track_line_width, 3 );
    Read( _T ( "CurrentArrowScale" ), &g_current_arrow_scale, 100 );
    Read( _T ( "DefaultWPIcon" ), &g_default_wp_icon, _T("triangle") );

    return ( 0 );
}

bool MyConfig::LoadChartDirArray( ArrayOfCDI &ChartDirArray )
{
    //    Chart Directories
    SetPath( _T ( "/ChartDirectories" ) );
    int iDirMax = GetNumberOfEntries();
    if( iDirMax ) {
        ChartDirArray.Empty();
        wxString str, val;
        long dummy;
        int nAdjustChartDirs = 0;
        int iDir = 0;
        bool bCont = pConfig->GetFirstEntry( str, dummy );
        while( bCont ) {
            pConfig->Read( str, &val );              // Get a Directory name

            wxString dirname( val );
            if( !dirname.IsEmpty() ) {

                /*     Special case for first time run after Windows install with sample chart data...
                 We desire that the sample configuration file opencpn.ini should not contain any
                 installation dependencies, so...
                 Detect and update the sample [ChartDirectories] entries to point to the Shared Data directory
                 For instance, if the (sample) opencpn.ini file should contain shortcut coded entries like:

                 [ChartDirectories]
                 ChartDir1=SampleCharts\\MaptechRegion7

                 then this entry will be updated to be something like:
                 ChartDir1=c:\Program Files\opencpn\SampleCharts\\MaptechRegion7

                 */
                if( dirname.Find( _T ( "SampleCharts" ) ) == 0 ) // only update entries starting with "SampleCharts"
                        {
                    nAdjustChartDirs++;

                    pConfig->DeleteEntry( str );
                    wxString new_dir = dirname.Mid( dirname.Find( _T ( "SampleCharts" ) ) );
                    new_dir.Prepend( g_SData_Locn );
                    dirname = new_dir;
                }

                ChartDirInfo cdi;
                cdi.fullpath = dirname.BeforeFirst( '^' );
                cdi.magic_number = dirname.AfterFirst( '^' );

                ChartDirArray.Add( cdi );
                iDir++;
            }

            bCont = pConfig->GetNextEntry( str, dummy );
        }

        if( nAdjustChartDirs ) pConfig->UpdateChartDirs( ChartDirArray );
    }

    return true;
}

bool MyConfig::AddNewRoute( Route *pr, int crm )
{
    wxString str_buf;
    int acrm;

    if( pr->m_bIsInLayer ) return true;

    if( crm != -1 ) acrm = crm;
    else
        acrm = m_NextRouteNum;

    pr->m_ConfigRouteNum = acrm;

    if( crm == -1 ) m_NextRouteNum += 1;             // auto increment for next time

    if( !m_bIsImporting ) {
        GpxRteElement * rte = ::CreateGPXRte( pr );
        rte->SetSimpleExtension( wxString( _T("opencpn:action") ), wxString( _T("add") ) );
        GpxRootElement * rt = (GpxRootElement *) m_pNavObjectChangesSet->RootElement();
        rt->AddRoute( rte );
        StoreNavObjChanges();
    }
    return true;
}

bool MyConfig::UpdateRoute( Route *pr )
{
    if( pr->m_bIsInLayer ) return true;

    if( pr->m_bIsTrack ) {
        if( !m_bIsImporting ) {
            GpxTrkElement * trk = ::CreateGPXTrk( pr );
            trk->SetSimpleExtension( wxString( _T("opencpn:action") ), wxString( _T("update") ) );
            GpxRootElement * rt = (GpxRootElement *) m_pNavObjectChangesSet->RootElement();
            rt->AddTrack( trk );
            StoreNavObjChanges();
        }
        return false;
    }

    wxString str_buf;

//    Build the Group Name
    wxString t( _T ( "/Routes/RouteDefn" ) );
    str_buf.Printf( _T ( "%d" ), pr->m_ConfigRouteNum );
    t.Append( str_buf );

    DeleteGroup( t );
    if( !m_bIsImporting ) {
        GpxRteElement * rte = ::CreateGPXRte( pr );
        rte->SetSimpleExtension( wxString( _T("opencpn:action") ), wxString( _T("update") ) );
        GpxRootElement * rt = (GpxRootElement *) m_pNavObjectChangesSet->RootElement();
        rt->AddRoute( rte );
        StoreNavObjChanges();
    }
    return true;
}

bool MyConfig::DeleteConfigRoute( Route *pr )
{
    wxString str_buf;

    if( pr->m_bIsInLayer ) return true;

//    Build the Group Name
    wxString t( _T ( "/Routes/RouteDefn" ) );
    str_buf.Printf( _T ( "%d" ), pr->m_ConfigRouteNum );
    t.Append( str_buf );

    DeleteGroup( t );

    Flush();
    if( !m_bIsImporting ) {
        if( !pr->m_bIsTrack ) {
            GpxRteElement * rte = ::CreateGPXRte( pr );
            rte->SetSimpleExtension( wxString( _T("opencpn:action") ), wxString( _T("delete") ) );
            GpxRootElement * rt = (GpxRootElement *) m_pNavObjectChangesSet->RootElement();
            rt->AddRoute( rte );
        } else {
            GpxTrkElement * trk = ::CreateGPXTrk( pr );
            trk->SetSimpleExtension( wxString( _T("opencpn:action") ), wxString( _T("delete") ) );
            GpxRootElement * rt = (GpxRootElement *) m_pNavObjectChangesSet->RootElement();
            rt->AddTrack( trk );
        }

        StoreNavObjChanges();
    }
    return true;
}

bool MyConfig::AddNewWayPoint( RoutePoint *pWP, int crm )
{
    wxString str_buf;
    int acrm;

    if( pWP->m_bIsInLayer ) return true;

    if( crm != -1 ) acrm = crm;
    else
        acrm = m_NextWPNum;
    pWP->m_ConfigWPNum = acrm;

    if( crm == -1 ) m_NextWPNum += 1;             // auto increment for next time
    if( !m_bIsImporting ) {
        GpxWptElement * wpt = ::CreateGPXWpt( pWP, GPX_WPT_WAYPOINT );
        wpt->SetSimpleExtension( wxString( _T("opencpn:action") ), wxString( _T("add") ) );
        GpxRootElement * rt = (GpxRootElement *) m_pNavObjectChangesSet->RootElement();
        rt->AddWaypoint( wpt );
        StoreNavObjChanges();
    }
    return true;
}

bool MyConfig::UpdateWayPoint( RoutePoint *pWP )
{
    wxString str_buf;

    if( pWP->m_bIsInLayer ) return true;

//    Build the Group Name
    wxString t( _T ( "/Marks/MarkDefn" ) );
    str_buf.Printf( _T ( "%d" ), pWP->m_ConfigWPNum );
    t.Append( str_buf );

    DeleteGroup( t );
    if( !m_bIsImporting ) {
        GpxWptElement * wpt = ::CreateGPXWpt( pWP, GPX_WPT_WAYPOINT );
        wpt->SetSimpleExtension( wxString( _T("opencpn:action") ), wxString( _T("update") ) );
        GpxRootElement * rt = (GpxRootElement *) m_pNavObjectChangesSet->RootElement();
        rt->AddWaypoint( wpt );
        StoreNavObjChanges();
    }
    return true;
}

bool MyConfig::DeleteWayPoint( RoutePoint *pWP )
{
    wxString str_buf;

    if( pWP->m_bIsInLayer ) return true;

//    Build the Group Name
    wxString t( _T ( "/Marks/MarkDefn" ) );
    str_buf.Printf( _T ( "%d" ), pWP->m_ConfigWPNum );
    t.Append( str_buf );

    DeleteGroup( t );

    Flush();
    if( !m_bIsImporting ) {
        GpxWptElement * wpt = ::CreateGPXWpt( pWP, GPX_WPT_WAYPOINT );
        wpt->SetSimpleExtension( wxString( _T("opencpn:action") ), wxString( _T("delete") ) );
        GpxRootElement * rt = (GpxRootElement *) m_pNavObjectChangesSet->RootElement();
        rt->AddWaypoint( wpt );
        StoreNavObjChanges();
    }
    return true;
}

bool MyConfig::UpdateChartDirs( ArrayOfCDI& dir_array )
{
    wxString key, dir;
    wxString str_buf;

    SetPath( _T ( "/ChartDirectories" ) );
    int iDirMax = GetNumberOfEntries();
    if( iDirMax ) {

        long dummy;

        for( int i = 0; i < iDirMax; i++ ) {
            GetFirstEntry( key, dummy );
            DeleteEntry( key, false );
        }
    }

    iDirMax = dir_array.GetCount();

    for( int iDir = 0; iDir < iDirMax; iDir++ ) {
        ChartDirInfo cdi = dir_array.Item( iDir );

        wxString dirn = cdi.fullpath;
        dirn.Append( _T("^") );
        dirn.Append( cdi.magic_number );

        str_buf.Printf( _T ( "ChartDir%d" ), iDir + 1 );

        Write( str_buf, dirn );

    }

    Flush();
    return true;
}

void MyConfig::CreateConfigGroups( ChartGroupArray *pGroupArray )
{
    if( !pGroupArray ) return;

    SetPath( _T ( "/Groups" ) );
    Write( _T ( "GroupCount" ), (int) pGroupArray->GetCount() );

    for( unsigned int i = 0; i < pGroupArray->GetCount(); i++ ) {
        ChartGroup *pGroup = pGroupArray->Item( i );
        wxString s;
        s.Printf( _T("Group%d"), i + 1 );
        s.Prepend( _T ( "/Groups/" ) );
        SetPath( s );

        Write( _T ( "GroupName" ), pGroup->m_group_name );
        Write( _T ( "GroupItemCount" ), (int) pGroup->m_element_array.GetCount() );

        for( unsigned int j = 0; j < pGroup->m_element_array.GetCount(); j++ ) {
            wxString sg;
            sg.Printf( _T("Group%d/Item%d"), i + 1, j );
            sg.Prepend( _T ( "/Groups/" ) );
            SetPath( sg );
            Write( _T ( "IncludeItem" ), pGroup->m_element_array.Item( j )->m_element_name );

            wxString t;
            wxArrayString u = pGroup->m_element_array.Item( j )->m_missing_name_array;
            if( u.GetCount() ) {
                for( unsigned int k = 0; k < u.GetCount(); k++ ) {
                    t += u.Item( k );
                    t += _T(";");
                }
                Write( _T ( "ExcludeItems" ), t );
            }
        }
    }
}

void MyConfig::DestroyConfigGroups( void )
{
    DeleteGroup( _T ( "/Groups" ) );                //zap
}

void MyConfig::LoadConfigGroups( ChartGroupArray *pGroupArray )
{
    SetPath( _T ( "/Groups" ) );
    unsigned int group_count;
    Read( _T ( "GroupCount" ), (int *) &group_count, 0 );

    for( unsigned int i = 0; i < group_count; i++ ) {
        ChartGroup *pGroup = new ChartGroup;
        wxString s;
        s.Printf( _T("Group%d"), i + 1 );
        s.Prepend( _T ( "/Groups/" ) );
        SetPath( s );

        wxString t;
        Read( _T ( "GroupName" ), &t );
        pGroup->m_group_name = t;

        unsigned int item_count;
        Read( _T ( "GroupItemCount" ), (int *) &item_count );
        for( unsigned int j = 0; j < item_count; j++ ) {
            wxString sg;
            sg.Printf( _T("Group%d/Item%d"), i + 1, j );
            sg.Prepend( _T ( "/Groups/" ) );
            SetPath( sg );

            wxString v;
            Read( _T ( "IncludeItem" ), &v );
            ChartGroupElement *pelement = new ChartGroupElement;
            pelement->m_element_name = v;
            pGroup->m_element_array.Add( pelement );

            wxString u;
            if( Read( _T ( "ExcludeItems" ), &u ) ) {
                if( !u.IsEmpty() ) {
                    wxStringTokenizer tk( u, _T(";") );
                    while( tk.HasMoreTokens() ) {
                        wxString token = tk.GetNextToken();
                        pelement->m_missing_name_array.Add( token );
                    }
                }
            }
        }
        pGroupArray->Add( pGroup );
    }

}

void MyConfig::UpdateSettings()
{
//    Global options and settings
    SetPath( _T ( "/Settings" ) );

    Write( _T ( "ConfigVersionString" ), g_config_version_string );
    Write( _T ( "NavMessageShown" ), n_NavMessageShown );

    Write( _T ( "UIStyle" ), g_StyleManager->GetStyleNextInvocation() );
    Write( _T ( "ChartNotRenderScaleFactor" ), g_ChartNotRenderScaleFactor );

    Write( _T ( "ShowDebugWindows" ), m_bShowDebugWindows );
    Write( _T ( "SetSystemTime" ), s_bSetSystemTime );
    Write( _T ( "ShowGrid" ), g_bDisplayGrid );
    Write( _T ( "PlayShipsBells" ), g_bPlayShipsBells );
    Write( _T ( "FullscreenToolbar" ), g_bFullscreenToolbar );
    Write( _T ( "TransparentToolbar" ), g_bTransparentToolbar );
    Write( _T ( "PermanentMOBIcon" ), g_bPermanentMOBIcon );
    Write( _T ( "ShowLayers" ), g_bShowLayers );
    Write( _T ( "ShowDepthUnits" ), g_bShowDepthUnits );
    Write( _T ( "AutoAnchorDrop" ), g_bAutoAnchorMark );
    Write( _T ( "ShowChartOutlines" ), g_bShowOutlines );
    Write( _T ( "ShowActiveRouteHighway" ), g_bShowActiveRouteHighway );
    Write( _T ( "SDMMFormat" ), g_iSDMMFormat );
    Write( _T ( "DistanceFormat" ), g_iDistanceFormat );
    Write( _T ( "SpeedFormat" ), g_iSpeedFormat );
    Write( _T ( "MostRecentGPSUploadConnection" ), g_uploadConnection );

    Write( _T ( "FilterNMEA_Avg" ), g_bfilter_cogsog );
    Write( _T ( "FilterNMEA_Sec" ), g_COGFilterSec );

    Write( _T ( "CM93DetailFactor" ), g_cm93_zoom_factor );
    Write( _T ( "CM93DetailZoomPosX" ), g_cm93detail_dialog_x );
    Write( _T ( "CM93DetailZoomPosY" ), g_cm93detail_dialog_y );
    Write( _T ( "ShowCM93DetailSlider" ), g_bShowCM93DetailSlider );
    Write( _T ( "AllowExtremeOverzoom" ), g_b_overzoom_x );

    Write( _T ( "SkewToNorthUp" ), g_bskew_comp );
    Write( _T ( "OpenGL" ), g_bopengl );
    Write( _T ( "SmoothPanZoom" ), g_bsmoothpanzoom );

    Write( _T ( "UseRasterCharts" ), g_bUseRaster );
    Write( _T ( "UseVectorCharts" ), g_bUseVector );
    Write( _T ( "UseCM93Charts" ), g_bUseCM93 );

    Write( _T ( "CourseUpMode" ), g_bCourseUp );
    Write( _T ( "LookAheadMode" ), g_bLookAhead );
    Write( _T ( "COGUPAvgSeconds" ), g_COGAvgSec );

    Write( _T ( "OwnshipCOGPredictorMinutes" ), g_ownship_predictor_minutes );
    Write( _T ( "OwnShipIconType" ), g_OwnShipIconType );
    Write( _T ( "OwnShipLength" ), g_n_ownship_length_meters );
    Write( _T ( "OwnShipWidth" ), g_n_ownship_beam_meters );
    Write( _T ( "OwnShipGPSOffsetX" ), g_n_gps_antenna_offset_x );
    Write( _T ( "OwnShipGPSOffsetY" ), g_n_gps_antenna_offset_y );
    Write( _T ( "OwnShipMinSize" ), g_n_ownship_min_mm );

    Write( _T ( "ChartQuilting" ), g_bQuiltEnable );
    Write( _T ( "FullScreenQuilt" ), g_bFullScreenQuilt );

    if( cc1 ) Write( _T ( "ChartQuiltingInitial" ), cc1->GetQuiltMode() );

    Write( _T ( "NMEALogWindowSizeX" ), NMEALogWindow::Get().GetSizeW());
    Write( _T ( "NMEALogWindowSizeY" ), NMEALogWindow::Get().GetSizeH());
    Write( _T ( "NMEALogWindowPosX" ), NMEALogWindow::Get().GetPosX());
    Write( _T ( "NMEALogWindowPosY" ), NMEALogWindow::Get().GetPosY());

    Write( _T ( "PreserveScaleOnX" ), g_bPreserveScaleOnX );

    Write( _T ( "StartWithTrackActive" ), g_bTrackCarryOver );
    Write( _T ( "AutomaticDailyTracks" ), g_bTrackDaily );
    Write( _T ( "HighlightTracks" ), g_bHighliteTracks );

    Write( _T ( "InitialStackIndex" ), g_restore_stackindex );
    Write( _T ( "InitialdBIndex" ), g_restore_dbindex );
    Write( _T ( "ActiveChartGroup" ), g_GroupIndex );

    Write( _T ( "AnchorWatch1GUID" ), g_AW1GUID );
    Write( _T ( "AnchorWatch2GUID" ), g_AW2GUID );

    Write( _T ( "ToolbarX" ), g_toolbar_x );
    Write( _T ( "ToolbarY" ), g_toolbar_y );
    Write( _T ( "ToolbarOrient" ), g_toolbar_orient );
    Write( _T ( "ToolbarConfig" ), g_toolbarConfig );

    Write( _T ( "GPSIdent" ), g_GPS_Ident );
    Write( _T ( "UseGarminHostUpload" ), g_bGarminHostUpload );

    wxString st0;
    st0.Printf( _T ( "%g" ), g_PlanSpeed );
    Write( _T ( "PlanSpeed" ), st0 );

    wxString vis, invis;
    LayerList::iterator it;
    int index = 0;
    for( it = ( *pLayerList ).begin(); it != ( *pLayerList ).end(); ++it, ++index ) {
        Layer *lay = (Layer *) ( *it );
        if( lay->IsVisibleOnChart() ) vis += ( lay->m_LayerName ) + _T(";");
        else
            invis += ( lay->m_LayerName ) + _T(";");
    }
    Write( _T ( "VisibleLayers" ), vis );
    Write( _T ( "InvisibleLayers" ), invis );

    Write( _T ( "Locale" ), g_locale );

    Write( _T ( "KeepNavobjBackups" ), g_navobjbackups );

//    S57 Object Filter Settings

    SetPath( _T ( "/Settings/ObjectFilter" ) );

#ifdef USE_S57
    if( ps52plib ) {
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );

            wxString st1( _T ( "viz" ) );
            char name[7];
            strncpy( name, pOLE->OBJLName, 6 );
            name[6] = 0;
            st1.Append( wxString( name, wxConvUTF8 ) );
            Write( st1, pOLE->nViz );
        }
    }
#endif

//    Global State

    SetPath( _T ( "/Settings/GlobalState" ) );

    wxString st1;

    if( cc1 ) {
        ViewPort vp = cc1->GetVP();

        if( vp.IsValid() ) {
            st1.Printf( _T ( "%10.4f,%10.4f" ), vp.clat, vp.clon );
            Write( _T ( "VPLatLon" ), st1 );
            st1.Printf( _T ( "%g" ), vp.view_scale_ppm );
            Write( _T ( "VPScale" ), st1 );
        }
    }

    st1.Printf( _T ( "%10.4f, %10.4f" ), gLat, gLon );
    Write( _T ( "OwnShipLatLon" ), st1 );

    //    Various Options
    SetPath( _T ( "/Settings/GlobalState" ) );
    if( cc1 ) Write( _T ( "bFollow" ), cc1->m_bFollow );
    Write( _T ( "nColorScheme" ), (int) gFrame->GetColorScheme() );

    Write( _T ( "FrameWinX" ), g_nframewin_x );
    Write( _T ( "FrameWinY" ), g_nframewin_y );
    Write( _T ( "FrameWinPosX" ), g_nframewin_posx );
    Write( _T ( "FrameWinPosY" ), g_nframewin_posy );
    Write( _T ( "FrameMax" ), g_bframemax );

    Write( _T ( "ClientPosX" ), g_lastClientRectx );
    Write( _T ( "ClientPosY" ), g_lastClientRecty );
    Write( _T ( "ClientSzX" ), g_lastClientRectw );
    Write( _T ( "ClientSzY" ), g_lastClientRecth );

    //    AIS
    SetPath( _T ( "/Settings/AIS" ) );

    Write( _T ( "bNoCPAMax" ), g_bCPAMax );
    Write( _T ( "NoCPAMaxNMi" ), g_CPAMax_NM );
    Write( _T ( "bCPAWarn" ), g_bCPAWarn );
    Write( _T ( "CPAWarnNMi" ), g_CPAWarn_NM );
    Write( _T ( "bTCPAMax" ), g_bTCPA_Max );
    Write( _T ( "TCPAMaxMinutes" ), g_TCPA_Max );
    Write( _T ( "bMarkLostTargets" ), g_bMarkLost );
    Write( _T ( "MarkLost_Minutes" ), g_MarkLost_Mins );
    Write( _T ( "bRemoveLostTargets" ), g_bRemoveLost );
    Write( _T ( "RemoveLost_Minutes" ), g_RemoveLost_Mins );
    Write( _T ( "bShowCOGArrows" ), g_bShowCOG );
    Write( _T ( "CogArrowMinutes" ), g_ShowCOG_Mins );
    Write( _T ( "bShowTargetTracks" ), g_bAISShowTracks );
    Write( _T ( "TargetTracksMinutes" ), g_AISShowTracks_Mins );
    Write( _T ( "bShowMooredTargets" ), g_bShowMoored );
    Write( _T ( "MooredTargetMaxSpeedKnots" ), g_ShowMoored_Kts );
    Write( _T ( "bAISAlertDialog" ), g_bAIS_CPA_Alert );
    Write( _T ( "bAISAlertAudio" ), g_bAIS_CPA_Alert_Audio );
    Write( _T ( "AISAlertAudioFile" ), g_sAIS_Alert_Sound_File );
    Write( _T ( "bAISAlertSuppressMoored" ), g_bAIS_CPA_Alert_Suppress_Moored );
    Write( _T ( "bShowAreaNotices" ), g_bShowAreaNotices );
    Write( _T ( "bDrawAISSize" ), g_bDrawAISSize );
    Write( _T ( "bShowAISName" ), g_bShowAISName );
    Write( _T ( "ShowAISTargetNameScale" ), g_Show_Target_Name_Scale );

    Write( _T ( "AlertDialogSizeX" ), g_ais_alert_dialog_sx );
    Write( _T ( "AlertDialogSizeY" ), g_ais_alert_dialog_sy );
    Write( _T ( "AlertDialogPosX" ), g_ais_alert_dialog_x );
    Write( _T ( "AlertDialogPosY" ), g_ais_alert_dialog_y );
    Write( _T ( "QueryDialogPosX" ), g_ais_query_dialog_x );
    Write( _T ( "QueryDialogPosY" ), g_ais_query_dialog_y );
    Write( _T ( "AISTargetListPerspective" ), g_AisTargetList_perspective );
    Write( _T ( "AISTargetListRange" ), g_AisTargetList_range );
    Write( _T ( "AISTargetListSortColumn" ), g_AisTargetList_sortColumn );
    Write( _T ( "bAISTargetListSortReverse" ), g_bAisTargetList_sortReverse );
    Write( _T ( "AISTargetListColumnSpec" ), g_AisTargetList_column_spec );

    Write( _T ( "S57QueryDialogSizeX" ), g_S57_dialog_sx );
    Write( _T ( "S57QueryDialogSizeY" ), g_S57_dialog_sy );

    Write( _T ( "bAISRolloverShowClass" ), g_bAISRolloverShowClass );
    Write( _T ( "bAISRolloverShowCOG" ), g_bAISRolloverShowCOG );
    Write( _T ( "bAISRolloverShowCPA" ), g_bAISRolloverShowCPA );

    Write( _T ( "bAISAlertAckTimeout" ), g_bAIS_ACK_Timeout );
    Write( _T ( "AlertAckTimeoutMinutes" ), g_AckTimeout_Mins );

#ifdef USE_S57
    SetPath( _T ( "/Settings/GlobalState" ) );
    if( ps52plib ) {
        Write( _T ( "bShowS57Text" ), ps52plib->GetShowS57Text() );
        Write( _T ( "bShowS57ImportantTextOnly" ), ps52plib->GetShowS57ImportantTextOnly() );
        Write( _T ( "nDisplayCategory" ), (long) ps52plib->m_nDisplayCategory );
        Write( _T ( "nSymbolStyle" ), (int) ps52plib->m_nSymbolStyle );
        Write( _T ( "nBoundaryStyle" ), (int) ps52plib->m_nBoundaryStyle );

        Write( _T ( "bShowSoundg" ), ps52plib->m_bShowSoundg );
        Write( _T ( "bShowMeta" ), ps52plib->m_bShowMeta );
        Write( _T ( "bUseSCAMIN" ), ps52plib->m_bUseSCAMIN );
        Write( _T ( "bShowAtonText" ), ps52plib->m_bShowAtonText );
        Write( _T ( "bShowLightDescription" ), ps52plib->m_bShowLdisText );
        Write( _T ( "bExtendLightSectors" ), ps52plib->m_bExtendLightSectors );
        Write( _T ( "bDeClutterText" ), ps52plib->m_bDeClutterText );

        Write( _T ( "S52_MAR_SAFETY_CONTOUR" ), S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR ) );
        Write( _T ( "S52_MAR_SHALLOW_CONTOUR" ), S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR ) );
        Write( _T ( "S52_MAR_DEEP_CONTOUR" ), S52_getMarinerParam( S52_MAR_DEEP_CONTOUR ) );
        Write( _T ( "S52_MAR_TWO_SHADES" ), S52_getMarinerParam( S52_MAR_TWO_SHADES ) );
        Write( _T ( "S52_DEPTH_UNIT_SHOW" ), ps52plib->m_nDepthUnitDisplay );
    }
    SetPath( _T ( "/Directories" ) );
    Write( _T ( "S57DataLocation" ), _T("") );
    Write( _T ( "SENCFileLocation" ), _T("") );

#endif

    SetPath( _T ( "/Directories" ) );
    Write( _T ( "InitChartDir" ), *pInit_Chart_Dir );
    Write( _T ( "GPXIODir" ), m_gpx_path );
    Write( _T ( "TCDataDir" ), g_TCData_Dir );

    SetPath( _T ( "/Settings/NMEADataSource" ) );
    wxString connectionconfigs;
    for (size_t i = 0; i < g_pConnectionParams->Count(); i++)
    {
        if (i > 0)
            connectionconfigs.Append(_T("|"));
        connectionconfigs.Append(g_pConnectionParams->Item(i)->Serialize());
    }
    Write ( _T ( "DataConnections" ), connectionconfigs );

    //    Fonts
    wxString font_path;
#ifdef __WXX11__
    font_path = ( _T ( "/Settings/X11Fonts" ) );
#endif

#ifdef __WXGTK__
    font_path = ( _T ( "/Settings/GTKFonts" ) );
#endif

#ifdef __WXMSW__
    font_path = ( _T ( "/Settings/MSWFonts" ) );
#endif

#ifdef __WXMAC__
    font_path = ( _T ( "/Settings/MacFonts" ) );
#endif

    SetPath( font_path );

    int nFonts = pFontMgr->GetNumFonts();

    for( int i = 0; i < nFonts; i++ ) {
        wxString cfstring( *pFontMgr->GetConfigString( i ) );
        wxString valstring = pFontMgr->GetFullConfigDesc( i );
        Write( cfstring, valstring );
    }

    //  Tide/Current Data Sources
    DeleteGroup( _T ( "/TideCurrentDataSources" ) );
    SetPath( _T ( "/TideCurrentDataSources" ) );
    unsigned int iDirMax = TideCurrentDataSet.Count();
    for( unsigned int id = 0 ; id < iDirMax ; id++ ) {
        wxString key;
        key.Printf(_T("tcds%d"), id);
        Write( key, TideCurrentDataSet.Item(id) );
    }

    SetPath( _T ( "/Settings/Others" ) );

    // Radar rings
    Write( _T ( "ShowRadarRings" ), (bool)(g_iNavAidRadarRingsNumberVisible > 0) );  //3.0.0 config support
    Write( _T ( "RadarRingsNumberVisible" ), g_iNavAidRadarRingsNumberVisible );
    Write( _T ( "RadarRingsStep" ), g_fNavAidRadarRingsStep );
    Write( _T ( "RadarRingsStepUnits" ), g_pNavAidRadarRingsStepUnits );

    Write( _T ( "ConfirmObjectDeletion" ), g_bConfirmObjectDelete );

    // Waypoint dragging with mouse; toh, 2009.02.24
    Write( _T ( "WaypointPreventDragging" ), g_bWayPointPreventDragging );

    Write( _T ( "EnableZoomToCursor" ), g_bEnableZoomToCursor );

    Write( _T ( "TrackIntervalSeconds" ), g_TrackIntervalSeconds );
    Write( _T ( "TrackDeltaDistance" ), g_TrackDeltaDistance );
    Write( _T ( "TrackPrecision" ), g_nTrackPrecision );

    Write( _T ( "RouteLineWidth" ), g_route_line_width );
    Write( _T ( "TrackLineWidth" ), g_track_line_width );
    Write( _T ( "CurrentArrowScale" ), g_current_arrow_scale );
    Write( _T ( "DefaultWPIcon" ), g_default_wp_icon );

    Flush();
}

void MyConfig::UpdateNavObj( void )
{
    //   Create the NavObjectCollection, and save to specified file
    NavObjectCollection *pNavObjectSet = new NavObjectCollection();

    pNavObjectSet->CreateNavObjGPXPoints();
    pNavObjectSet->CreateNavObjGPXRoutes();
    pNavObjectSet->CreateNavObjGPXTracks();

    pNavObjectSet->SaveFile( m_sNavObjSetFile );

    pNavObjectSet->Clear();
    delete pNavObjectSet;

    ::wxRemoveFile( m_sNavObjSetChangesFile );
    m_pNavObjectChangesSet->Clear();
    delete m_pNavObjectChangesSet;
    m_pNavObjectChangesSet = new NavObjectCollection();

}

void MyConfig::StoreNavObjChanges( void )
{
    m_pNavObjectChangesSet->SaveFile( m_sNavObjSetChangesFile );
}

bool MyConfig::ExportGPXRoutes( wxWindow* parent, RouteList *pRoutes )
{
    //FIXME: get rid of the Dialogs and unite with the other
    wxFileDialog saveDialog( parent, _( "Export GPX file" ), m_gpx_path, _T("routes"),
            wxT ( "GPX files (*.gpx)|*.gpx" ), wxFD_SAVE );

    int response = saveDialog.ShowModal();

    wxString path = saveDialog.GetPath();
    wxFileName fn( path );
    m_gpx_path = fn.GetPath();

    if( response == wxID_OK ) {
        fn.SetExt( _T ( "gpx" ) );

        if( wxFileExists( fn.GetFullPath() ) ) {
            int answer = OCPNMessageBox( NULL, _("Overwrite existing file?"), _T("Confirm"),
                    wxICON_QUESTION | wxYES_NO | wxCANCEL );
            if( answer != wxID_YES ) return false;
        }

        GpxDocument *gpx = new GpxDocument();
        GpxRootElement *gpxroot = (GpxRootElement *) gpx->RootElement();
        wxRouteListNode* pRoute = pRoutes->GetFirst();
        while (pRoute) {
            Route* pRData = pRoute->GetData();
            // TODO this is awkward
            if( !pRData->m_bIsTrack ) {
                gpxroot->AddRoute( CreateGPXRte( pRData ) );
            } else {
                gpxroot->AddTrack( CreateGPXTrk( pRData ) );
            }
            pRoute = pRoute->GetNext();
        }

        gpx->SaveFile( fn.GetFullPath() );
        gpx->Clear();
        delete gpx;
        return true;
    } else
        return false;
}

bool MyConfig::ExportGPXWaypoints( wxWindow* parent, RoutePointList *pRoutePoints )
{
    //if (pRoutePoint->m_bIsInLayer) return true;

    //FIXME: get rid of the Dialogs and unite with the other
    wxFileDialog saveDialog( parent, _( "Export GPX file" ), m_gpx_path, wxT ( "" ),
            wxT ( "GPX files (*.gpx)|*.gpx" ), wxFD_SAVE );

    int response = saveDialog.ShowModal();

    wxString path = saveDialog.GetPath();
    wxFileName fn( path );
    m_gpx_path = fn.GetPath();

    if( response == wxID_OK ) {
        fn.SetExt( _T ( "gpx" ) );

        if( wxFileExists( fn.GetFullPath() ) ) {
            int answer = OCPNMessageBox(NULL,  _("Overwrite existing file?"), _T("Confirm"),
                    wxICON_QUESTION | wxYES_NO | wxCANCEL );
            if( answer != wxID_YES ) return false;
        }

        GpxDocument *gpx = new GpxDocument();
        GpxRootElement *gpxroot = (GpxRootElement *) gpx->RootElement();
//          This should not be necessary
        wxRoutePointListNode* pRoutePoint = pRoutePoints->GetFirst();
        while (pRoutePoint) {
            RoutePoint* pRPData = pRoutePoint->GetData();
            if( !WptIsInRouteList( pRPData ) || pRPData->m_bKeepXRoute ) {
                gpxroot->AddWaypoint( ::CreateGPXWpt( pRPData, GPX_WPT_WAYPOINT ) );
            }
            pRoutePoint = pRoutePoint->GetNext();
        }
        gpx->SaveFile( fn.GetFullPath() );

        gpx->Clear();
        delete gpx;
        return true;
    } else
        return false;
}

void MyConfig::ExportGPX( wxWindow* parent, bool bviz_only, bool blayer )
{
    //FIXME: get rid of the Dialogs and unite with the other
    wxFileDialog saveDialog( parent, _( "Export GPX file" ), m_gpx_path, wxT ( "" ),
            wxT ( "GPX files (*.gpx)|*.gpx" ), wxFD_SAVE );

    int response = saveDialog.ShowModal();

    wxString path = saveDialog.GetPath();
    wxFileName fn( path );
    m_gpx_path = fn.GetPath();

    if( response == wxID_OK ) {
        fn.SetExt( _T ( "gpx" ) );

        if( wxFileExists( fn.GetFullPath() ) ) {
            int answer = OCPNMessageBox( NULL, _("Overwrite existing file?"), _T("Confirm"),
                    wxICON_QUESTION | wxYES_NO | wxCANCEL );
            if( answer != wxID_YES ) return;
        }

        ::wxBeginBusyCursor();
        GpxDocument *gpx = new GpxDocument();
        GpxRootElement *gpxroot = (GpxRootElement *) gpx->RootElement();

        wxProgressDialog *pprog = NULL;
        int count = pWayPointMan->m_pWayPointList->GetCount();
        if( count > 200) {
            pprog = new wxProgressDialog( _("Export GPX file"), _T("0/0"), count, NULL,
                                          wxPD_APP_MODAL | wxPD_SMOOTH |
                                          wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );
            pprog->SetSize( 400, wxDefaultCoord );
            pprog->Centre();
        }

        //WPTs
        int ic = 0;

        wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();
        RoutePoint *pr;
        while( node ) {
            if(pprog) {
                wxString msg;
                msg.Printf(_T("%d/%d"), ic, count);
                pprog->Update( ic, msg );
                ic++;
            }

            pr = node->GetData();

            bool b_add = true;

            if( bviz_only && !pr->m_bIsVisible )
                b_add = false;

            if( pr->m_bIsInLayer && !blayer )
                b_add = false;
            if( b_add) {
                if( pr->m_bKeepXRoute || !WptIsInRouteList( pr ) )
                    gpxroot->AddWaypoint( CreateGPXWpt( pr, GPX_WPT_WAYPOINT ) );
            }

            node = node->GetNext();
        }
        //RTEs and TRKs
        wxRouteListNode *node1 = pRouteList->GetFirst();
        while( node1 ) {
            Route *pRoute = node1->GetData();

            bool b_add = true;

            if( bviz_only && !pRoute->IsVisible() )
                b_add = false;

            if(  pRoute->m_bIsInLayer && !blayer )
                b_add = false;

            if( b_add ) {
                if( !pRoute->m_bIsTrack )
                    gpxroot->AddRoute( CreateGPXRte( pRoute ) );
                else
                    gpxroot->AddTrack( CreateGPXTrk( pRoute ) );
                }
            node1 = node1->GetNext();
        }

        gpx->SaveFile( fn.GetFullPath() );
        gpx->Clear();
        delete gpx;

        ::wxEndBusyCursor();

        if( pprog)
            delete pprog;

    }
}

GpxWptElement *CreateGPXWpt( RoutePoint *pr, char * waypoint_type, bool b_props_explicit,
        bool b_props_minimal )
{
    GpxExtensionsElement *exts = NULL;
    wxString type_prop;

    if( !b_props_minimal ) {
        type_prop = _T("WPT");

        exts = new GpxExtensionsElement();
        exts->LinkEndChild( new GpxSimpleElement( wxString( _T("opencpn:guid") ), pr->m_GUID ) );

        //    Create all opencpn extension properties explicitely
        if( b_props_explicit ) {
            exts->LinkEndChild(
                    new GpxSimpleElement( wxString( _T("opencpn:viz") ),
                            pr->m_bIsVisible == true ? _T("1") : _T("0")) );
            exts->LinkEndChild(
                    new GpxSimpleElement( wxString( _T("opencpn:viz_name") ),
                            pr->m_bShowName == true ? _T("1") : _T("0")) );
            exts->LinkEndChild(
                    new GpxSimpleElement( wxString( _T("opencpn:auto_name") ),
                            pr->m_bDynamicName == true ? _T("1") : _T("0")) );
            exts->LinkEndChild(
                    new GpxSimpleElement( wxString( _T("opencpn:shared") ),
                            pr->m_bKeepXRoute == true ? _T("1") : _T("0")) );
        } else {
            //      if(!pr->m_bIsVisible)
            exts->LinkEndChild(
                    new GpxSimpleElement( wxString( _T("opencpn:viz") ),
                            pr->m_bIsVisible == true ? _T("1") : _T("0")) );
            //           if(pr->m_bShowName)
            exts->LinkEndChild(
                    new GpxSimpleElement( wxString( _T("opencpn:viz_name") ),
                            pr->m_bShowName == true ? _T("1") : _T("0")) );
            if( pr->m_bDynamicName ) exts->LinkEndChild(
                    new GpxSimpleElement( wxString( _T("opencpn:auto_name") ),
                            pr->m_bDynamicName == true ? _T("1") : _T("0")) );
            if( pr->m_bKeepXRoute ) exts->LinkEndChild(
                    new GpxSimpleElement( wxString( _T("opencpn:shared") ),
                            pr->m_bKeepXRoute == true ? _T("1") : _T("0")) );
        }
    }

    ListOfGpxLinks lnks;
    lnks.DeleteContents( false );

    // Hyperlinks
    HyperlinkList *linklist = pr->m_HyperlinkList;
    if( linklist && linklist->GetCount() ) {
        wxHyperlinkListNode *linknode = linklist->GetFirst();
        while( linknode ) {
            Hyperlink *link = linknode->GetData();
            lnks.Append( new GpxLinkElement( link->Link, link->DescrText, link->Type ) );
            linknode = linknode->GetNext();
        }
    }

    return new GpxWptElement( waypoint_type, pr->m_lat, pr->m_lon, 0, &pr->m_CreateTime, 0, -1,
            pr->GetName(), GPX_EMPTY_STRING, pr->m_MarkDescription, GPX_EMPTY_STRING, &lnks,
            pr->m_IconName, type_prop, fix_undefined, -1, -1, -1, -1, -1, -1, exts );

}

GpxRteElement *CreateGPXRte( Route *pRoute )
{
    GpxExtensionsElement *exts = new GpxExtensionsElement();
    exts->LinkEndChild(
            new GpxSimpleElement( wxString( _T("opencpn:start") ), pRoute->m_RouteStartString ) );
    exts->LinkEndChild(
            new GpxSimpleElement( wxString( _T("opencpn:end") ), pRoute->m_RouteEndString ) );
    exts->LinkEndChild(
            new GpxSimpleElement( wxString( _T("opencpn:viz") ),
                    pRoute->IsVisible() ? wxString( _T("1") ) : wxString( _T("0") ) ) );
    exts->LinkEndChild( new GpxSimpleElement( wxString( _T("opencpn:guid") ), pRoute->m_GUID ) );
    if( pRoute->m_width != STYLE_UNDEFINED || pRoute->m_style != STYLE_UNDEFINED ) {
        TiXmlElement* e = new TiXmlElement( "opencpn:style" );
        if( pRoute->m_width != STYLE_UNDEFINED ) e->SetAttribute( "width", pRoute->m_width );
        if( pRoute->m_style != STYLE_UNDEFINED ) e->SetAttribute( "style", pRoute->m_style );
        exts->LinkEndChild( e );
    }
    if( pRoute->m_Colour != wxEmptyString ) {
        GpxxExtensionsElement *gpxx = new GpxxExtensionsElement( _T("gpxx:RouteExtension") );
        gpxx->LinkEndChild(
                new GpxSimpleElement( wxString( _T("gpxx:DisplayColor") ), pRoute->m_Colour ) );
        exts->LinkEndChild( gpxx );
    }

    GpxRteElement *rte = new GpxRteElement( pRoute->m_RouteNameString, GPX_EMPTY_STRING,
            GPX_EMPTY_STRING, GPX_EMPTY_STRING, NULL, -1, GPX_EMPTY_STRING, exts, NULL );

    //rtepts
    RoutePointList *pRoutePointList = pRoute->pRoutePointList;
    wxRoutePointListNode *node2 = pRoutePointList->GetFirst();
    RoutePoint *prp;
    int i = 1;
    while( node2 ) {
        prp = node2->GetData();

        rte->AppendRtePoint( ::CreateGPXWpt( prp, GPX_WPT_ROUTEPOINT ) );

        node2 = node2->GetNext();
        i++;
    }

    return rte;
}

GpxTrkElement *CreateGPXTrk( Route *pRoute )
{
    GpxExtensionsElement *exts = new GpxExtensionsElement();
    exts->LinkEndChild(
            new GpxSimpleElement( wxString( _T("opencpn:start") ), pRoute->m_RouteStartString ) );
    exts->LinkEndChild(
            new GpxSimpleElement( wxString( _T("opencpn:end") ), pRoute->m_RouteEndString ) );
    exts->LinkEndChild(
            new GpxSimpleElement( wxString( _T("opencpn:viz") ),
                    pRoute->IsVisible() ? wxString( _T("1") ) : wxString( _T("0") ) ) );
    exts->LinkEndChild( new GpxSimpleElement( wxString( _T("opencpn:guid") ), pRoute->m_GUID ) );
    if( pRoute->m_width != STYLE_UNDEFINED || pRoute->m_style != STYLE_UNDEFINED ) {
        TiXmlElement* e = new TiXmlElement( "opencpn:style" );
        if( pRoute->m_width != STYLE_UNDEFINED ) e->SetAttribute( "width", pRoute->m_width );
        if( pRoute->m_style != STYLE_UNDEFINED ) e->SetAttribute( "style", pRoute->m_style );
        exts->LinkEndChild( e );
    }
    if( pRoute->m_Colour != wxEmptyString ) {
        GpxxExtensionsElement *gpxx = new GpxxExtensionsElement( _T("gpxx:TrackExtension") );
        gpxx->LinkEndChild(
                new GpxSimpleElement( wxString( _T("gpxx:DisplayColor") ), pRoute->m_Colour ) );
        exts->LinkEndChild( gpxx );
    }

    GpxTrkElement *trk = new GpxTrkElement( pRoute->m_RouteNameString, GPX_EMPTY_STRING,
            GPX_EMPTY_STRING, GPX_EMPTY_STRING, NULL, -1, GPX_EMPTY_STRING, exts, NULL );

    RoutePointList *pRoutePointList = pRoute->pRoutePointList;
    wxRoutePointListNode *node2 = pRoutePointList->GetFirst();
    RoutePoint *prp;

    unsigned short int GPXTrkSegNo1 = 1;

    do {
        unsigned short int GPXTrkSegNo2 = GPXTrkSegNo1;
        GpxTrksegElement *trkseg = new GpxTrksegElement();
        trk->AppendTrkSegment( trkseg );

        int i = 1;
        while( node2 && ( GPXTrkSegNo2 == GPXTrkSegNo1 ) ) {
            prp = node2->GetData();
//                  trkseg->AppendTrkPoint(::CreateGPXWpt ( prp, GPX_WPT_TRACKPOINT, true));
            trkseg->AppendTrkPoint( ::CreateGPXWpt( prp, GPX_WPT_TRACKPOINT, true, true ) );
            node2 = node2->GetNext();
            if( node2 ) {
                prp = node2->GetData();
                GPXTrkSegNo2 = prp->m_GPXTrkSegNo;
            }
            i++;
        }
        GPXTrkSegNo1 = GPXTrkSegNo2;
    } while( node2 );

    return trk;
}

void MyConfig::ImportGPX( wxWindow* parent, bool islayer, wxString dirpath, bool isdirectory )
{
    int response = wxID_CANCEL;
    m_bIsImporting = true;
    g_bIsNewLayer = islayer;
    wxArrayString file_array;
    Layer *l = NULL;

    //wxString impmsg;
    //impmsg.Printf(wxT("ImportGPX: %d, %s, %d"), islayer, dirpath.c_str(), isdirectory);
    //wxLogMessage(impmsg);

    if( !islayer || dirpath.IsSameAs( _T("") ) ) {
        //FIXME: unite the loading itself with NavObjectCollection::LoadAllGPXObjects()
        wxFileDialog openDialog( parent, _( "Import GPX file" ), m_gpx_path, wxT ( "" ),
                wxT ( "GPX files (*.gpx)|*.gpx|All files (*.*)|*.*" ),
                wxFD_OPEN | wxFD_MULTIPLE );
        response = openDialog.ShowModal();
        if( response == wxID_OK ) {
            openDialog.GetPaths( file_array );

            //    Record the currently selected directory for later use
            if( file_array.GetCount() ) {
                wxFileName fn( file_array[0] );
                m_gpx_path = fn.GetPath();
            }
        }

    } else {
        if( isdirectory ) {
            if( wxDir::GetAllFiles( dirpath, &file_array, wxT("*.gpx") ) ) response = wxID_OK;
        } else {
            file_array.Add( dirpath );
            response = wxID_OK;
        }
    }

    if( response == wxID_OK ) {

        if( islayer ) {
            l = new Layer();
            l->m_LayerID = ++g_LayerIdx;
            l->m_LayerFileName = file_array[0];
            if( file_array.GetCount() <= 1 ) wxFileName::SplitPath( file_array[0], NULL, NULL,
                    &( l->m_LayerName ), NULL, NULL );
            else {
                if( dirpath.IsSameAs( _T("") ) ) wxFileName::SplitPath( m_gpx_path, NULL, NULL,
                        &( l->m_LayerName ), NULL, NULL );
                else
                    wxFileName::SplitPath( dirpath, NULL, NULL, &( l->m_LayerName ), NULL, NULL );
            }
            g_bLayerViz = g_bShowLayers;
            if( g_VisibleLayers.Contains( l->m_LayerName ) ) g_bLayerViz = true;
            if( g_InvisibleLayers.Contains( l->m_LayerName ) ) g_bLayerViz = false;
            l->m_bIsVisibleOnChart = g_bLayerViz;

            wxString laymsg;
            laymsg.Printf( wxT("New layer %d: %s"), l->m_LayerID, l->m_LayerName.c_str() );
            wxLogMessage( laymsg );

            pLayerList->Insert( l );
        }

        for( unsigned int i = 0; i < file_array.GetCount(); i++ ) {
            wxString path = file_array[i];

            //wxString filmsg;
            //filmsg.Printf(wxT("Trying layer file %d: %s"), i, path.c_str());
            //wxLogMessage(filmsg);

            if( ::wxFileExists( path ) ) {

                //wxString gpxmsg;
                //gpxmsg.Printf(wxT("Reading layer file %d: %s"), i, path.c_str());
                //wxLogMessage(gpxmsg);

                GpxDocument *pXMLNavObj = new GpxDocument();
                if( pXMLNavObj->LoadFile( path ) ) {
                    TiXmlElement *root = pXMLNavObj->RootElement();

                    wxString RootName = wxString::FromUTF8( root->Value() );
                    if( RootName == _T ( "gpx" ) ) {
                        //wxString RootContent = root->GetNodeContent();

                        TiXmlNode *child;
                        for( child = root->FirstChild(); child != 0; child =
                                child->NextSibling() ) {
                            wxString ChildName = wxString::FromUTF8( child->Value() );
                            if( ChildName == _T ( "wpt" ) ) {
                                RoutePoint *pWp = ::LoadGPXWaypoint( (GpxWptElement *) child,
                                        _T("circle"), true );          // Full Viz
                                RoutePoint *pExisting = WaypointExists( pWp->GetName(), pWp->m_lat,
                                        pWp->m_lon );
                                if( !pExisting ) {
                                    if( WaypointExists( pWp->m_GUID ) ) //We try to import a waypoint with the same guid but different properties, so we assign it a new guid to keep them both
                                    pWp->m_GUID = pWayPointMan->CreateGUID( pWp );

                                    if( NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->Append(
                                            pWp );

                                    pWp->m_bIsolatedMark = true;      // This is an isolated mark
                                    pWp->m_bIsInLayer = g_bIsNewLayer;
                                    AddNewWayPoint( pWp, m_NextWPNum );   // use auto next num
                                    if( g_bIsNewLayer ) {
                                        pWp->m_LayerID = g_LayerIdx;
                                        pWp->m_bIsVisible = g_bLayerViz;
                                    } else
                                        pWp->m_LayerID = 0;
                                    pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                                    pWp->m_ConfigWPNum = m_NextWPNum;
                                    m_NextWPNum++;
                                }
                                if( islayer ) l->m_NoOfItems++;
                            } else
                                if( ChildName == _T ( "rte" ) ) {
                                    ::GPXLoadRoute( (GpxRteElement *) child, m_NextRouteNum, true ); // Full visibility
                                    m_NextRouteNum++;
                                    if( islayer ) l->m_NoOfItems++;
                                } else
                                    if( ChildName == _T ( "trk" ) ) {
                                        ::GPXLoadTrack( (GpxTrkElement *) child, true ); // Full visibility
                                        if( islayer ) l->m_NoOfItems++;
                                    }
                        }
                    }
                }
                pXMLNavObj->Clear();
                delete pXMLNavObj;
            }
        }
    }
    m_bIsImporting = false;
    g_bIsNewLayer = false;
}

//-------------------------------------------------------------------------
//
//          Static GPX Support Routines
//
//-------------------------------------------------------------------------
RoutePoint *WaypointExists( const wxString& name, double lat, double lon )
{
    RoutePoint *pret = NULL;
//    if( g_bIsNewLayer ) return NULL;
    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();
    bool Exists = false;
    while( node ) {
        RoutePoint *pr = node->GetData();

//        if( pr->m_bIsInLayer ) return NULL;

        if( name == pr->GetName() ) {
            if( fabs( lat - pr->m_lat ) < 1.e-6 && fabs( lon - pr->m_lon ) < 1.e-6 ) {
                Exists = true;
                pret = pr;
                break;
            }
        }
        node = node->GetNext();
    }

    return pret;
}

RoutePoint *WaypointExists( const wxString& guid )
{
//    if( g_bIsNewLayer ) return NULL;
    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *pr = node->GetData();

//        if( pr->m_bIsInLayer ) return NULL;

        if( guid == pr->m_GUID ) {
            return pr;
        }
        node = node->GetNext();
    }

    return NULL;
}

bool WptIsInRouteList( RoutePoint *pr )
{
    bool IsInList = false;

    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        RoutePointList *pRoutePointList = pRoute->pRoutePointList;

        wxRoutePointListNode *node2 = pRoutePointList->GetFirst();
        RoutePoint *prp;

        while( node2 ) {
            prp = node2->GetData();

            if( pr->IsSame( prp ) ) {
                IsInList = true;
                break;
            }

            node2 = node2->GetNext();
        }
        node1 = node1->GetNext();
    }
    return IsInList;
}

Route *RouteExists( const wxString& guid )
{
    if( g_bIsNewLayer ) return NULL;
    wxRouteListNode *route_node = pRouteList->GetFirst();

    while( route_node ) {
        Route *proute = route_node->GetData();

        if( guid == proute->m_GUID ) return proute;

        route_node = route_node->GetNext();
    }
    return NULL;
}

Route *RouteExists( Route * pTentRoute )
{
    if( g_bIsNewLayer ) return NULL;
    wxRouteListNode *route_node = pRouteList->GetFirst();
    while( route_node ) {
        Route *proute = route_node->GetData();

        if( proute->IsEqualTo( pTentRoute ) ) {
            if( !proute->m_bIsTrack ) return proute;
        }

        route_node = route_node->GetNext();       // next route
    }
    return NULL;
}

#if 0
wxXmlNode *CreateGPXRouteStatic ( Route *pRoute )
{
    wxXmlNode *GPXRte_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "rte" ) );

    wxXmlNode *node;
    wxXmlNode *tnode;

    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "name" ) );
    GPXRte_node->AddChild ( node );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pRoute->m_RouteNameString );
    node->AddChild ( tnode );

    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "number" ) );
    GPXRte_node->AddChild ( node );
    wxString strnum;
    strnum.Printf ( _T ( "%d" ),pRoute->m_ConfigRouteNum );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), strnum );
    node->AddChild ( tnode );

    //    Extensions
    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "extensions" ) );
    GPXRte_node->AddChild ( node );

    wxXmlNode *s_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "opencpn:start" ) );
    node->AddChild ( s_node );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pRoute->m_RouteStartString );
    s_node->AddChild ( tnode );

    wxXmlNode *e_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "opencpn:end" ) );
    node->AddChild ( e_node );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pRoute->m_RouteEndString );
    e_node->AddChild ( tnode );

    wxXmlNode *v_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "opencpn:viz" ) );
    node->AddChild ( v_node );
    wxString viz = _T("1");
    if(!pRoute->IsVisible())
    viz = _T("0");
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ),viz );
    v_node->AddChild ( tnode );

    RoutePointList *pRoutePointList = pRoute->pRoutePointList;

    wxRoutePointListNode *node2 = pRoutePointList->GetFirst();
    RoutePoint *prp;

    int i=1;
    while ( node2 )
    {
        prp = node2->GetData();

        wxXmlNode *rpt_node = ::CreateGPXPointNode ( prp, _T("rtept") );
        GPXRte_node->AddChild ( rpt_node );

        node2=node2->GetNext();
        i++;
    }
    return GPXRte_node;
}

wxXmlNode *CreateGPXTrackStatic ( Route *pRoute )
{

    wxXmlNode *GPXTrk_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "trk" ) );

    wxXmlNode *node;
    wxXmlNode *tnode;

    if(pRoute->m_RouteNameString.Len())
    {
        node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "name" ) );
        GPXTrk_node->AddChild ( node );
        tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pRoute->m_RouteNameString );
        node->AddChild ( tnode );
    }

    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "number" ) );
    GPXTrk_node->AddChild ( node );
    wxString strnum;
    strnum.Printf ( _T ( "%d" ),pRoute->m_ConfigRouteNum );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), strnum );
    node->AddChild ( tnode );

    //    Extensions
    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "extensions" ) );
    GPXTrk_node->AddChild ( node );

    wxXmlNode *s_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "opencpn:start" ) );
    node->AddChild ( s_node );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pRoute->m_RouteStartString );
    s_node->AddChild ( tnode );

    wxXmlNode *e_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "opencpn:end" ) );
    node->AddChild ( e_node );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pRoute->m_RouteEndString );
    e_node->AddChild ( tnode );

    wxXmlNode *v_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "opencpn:viz" ) );
    node->AddChild ( v_node );
    wxString viz = _T("1");
    if(!pRoute->IsVisible())
    viz = _T("0");
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ),viz );
    v_node->AddChild ( tnode );

    RoutePointList *pRoutePointList = pRoute->pRoutePointList;
    wxRoutePointListNode *node2 = pRoutePointList->GetFirst();
    RoutePoint *prp;

    unsigned short int GPXTrkSegNo1 = 1;// pjotrc 2010.03.02

    do {                                                                  // pjotrc 2010.03.02
        unsigned short int GPXTrkSegNo2 = GPXTrkSegNo1;// pjotrc 2010.03.02
        wxXmlNode *trkseg_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "trkseg" ) );
        GPXTrk_node->AddChild ( trkseg_node );

        int i=1;
        while ( node2 && (GPXTrkSegNo2 == GPXTrkSegNo1))
        {
            prp = node2->GetData();

            wxXmlNode *rpt_node = ::CreateGPXPointNode ( prp, _T("trkpt"));

            trkseg_node->AddChild ( rpt_node );

            node2=node2->GetNext();

            if (node2) {                                          // pjotrc 2010.03.02
                prp = node2->GetData();// pjotrc 2010.03.02
                GPXTrkSegNo2 = prp->m_GPXTrkSegNo;// pjotrc 2010.03.02
            }

            i++;
        }
        GPXTrkSegNo1 = GPXTrkSegNo2;      // pjotrc 2010.03.02
    }while (node2);                                      // pjotrc 2010.03.02

    return GPXTrk_node;
}

#endif

// This function formats the input date/time into a valid GPX ISO 8601
// time string specified in the UTC time zone.

wxString FormatGPXDateTime( wxDateTime dt )
{
//      return dt.Format(wxT("%Y-%m-%dT%TZ"), wxDateTime::GMT0);
    return dt.Format( wxT("%Y-%m-%dT%H:%M:%SZ") );
}

#if 0
wxXmlNode *CreateGPXPointNode ( RoutePoint *pr, const wxString &root_name )
{
    wxXmlNode *GPXPoint_node = new wxXmlNode ( wxXML_ELEMENT_NODE, root_name );

    wxString str_lat;
    str_lat.Printf ( _T ( "%.9f" ), pr->m_lat );
    wxString str_lon;
    str_lon.Printf ( _T ( "%.9f" ), pr->m_lon );
    GPXPoint_node->AddProperty ( _T ( "lat" ),str_lat );
    GPXPoint_node->AddProperty ( _T ( "lon" ),str_lon );

    //  Get and create the mark properties, one by one
    wxXmlNode *node;
    wxXmlNode *tnode;
    wxXmlNode *current_sib_node = NULL;

    //  Create Time
    if ( pr->m_CreateTime.IsValid() )
    {
        wxString dt;

        dt = FormatGPXDateTime(pr->m_CreateTime);

        node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "time" ) );
        GPXPoint_node->AddChild ( node );
        tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), dt );
        node->AddChild ( tnode );

        current_sib_node = node;
    }

    //  Name
    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "name" ) );
    if(current_sib_node)
    current_sib_node->SetNext ( node );
    else
    GPXPoint_node->AddChild ( node );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pr->m_MarkName );
    node->AddChild ( tnode );

    current_sib_node = node;

    // Description
    if(pr->m_MarkDescription.Len())
    {
        node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "desc" ) );
        current_sib_node->SetNext ( node );
        tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pr->m_MarkDescription );
        node->AddChild ( tnode );

        current_sib_node = node;
    }

    // Hyperlinks
    HyperlinkList *linklist = pr->m_HyperlinkList;
    if(linklist)
    {
        wxHyperlinkListNode *linknode = linklist->GetFirst();
        while ( linknode )
        {
            Hyperlink *link = linknode->GetData();
            wxString Link = link->Link;
            wxString Descr = link->DescrText;
            wxString Type = link->Type;

            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "link" ) );
            current_sib_node->SetNext ( node );

            wxXmlProperty *prop = new wxXmlProperty ( _T ( "href" ),Link );
            node->SetProperties ( prop );

            if ( Descr.Length() > 0 )
            {
                wxXmlNode *textnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "text" ) );
                node->AddChild ( textnode );
                wxXmlNode *descrnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ),Descr );
                textnode->AddChild ( descrnode );

            }

            current_sib_node = node;

            if ( Type.Length() > 0 )
            {
                wxXmlNode *typenode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "type" ) );
                node->AddChild ( typenode );
                wxXmlNode *typnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ),Type );
                typenode->AddChild ( typnode );
            }

            current_sib_node = node;

            linknode = linknode->GetNext();
        }
    }

    //  Icon
    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "sym" ) );
    current_sib_node->SetNext ( node );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pr->m_IconName );
    node->AddChild ( tnode );

    current_sib_node = node;

    // Type...A simple string in GPX schema, we use "WPT"
    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "type" ) );
    current_sib_node->SetNext ( node );
    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ),_T ( "WPT" ) );
    node->AddChild ( tnode );

    current_sib_node = node;

    //  RoutePoint properties/flags
    wxString str = pr->CreatePropString();
    node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "extensions" ) );
    current_sib_node->SetNext ( node );

    wxXmlNode *prop_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "opencpn:prop" ) );
    node->AddChild ( prop_node );

    tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), str );
    prop_node->AddChild ( tnode );

    return ( GPXPoint_node );
}
#endif

#if 0
void AppendGPXWayPoints ( wxXmlNode *RNode )
{

    //    Iterate on the RoutePoint list
    // If a waypoint is also in the route, it mustn't be written

    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

    RoutePoint *pr;
    wxXmlNode *prev_node = NULL;

    bool IsFirst = true;
    while ( node )
    {
        pr = node->GetData();

        if ( !WptIsInRouteList ( pr ) )
        {
            wxXmlNode *mark_node = ::CreateGPXPointNode ( pr, _T("wpt") );

            if ( IsFirst )
            {
                IsFirst = false;
                RNode->AddChild ( mark_node );
            }
            else
            prev_node->SetNext ( mark_node );

            prev_node = mark_node;
        }
        node = node->GetNext();
    }
}

void AppendGPXRoutes ( wxXmlNode *RNode )
{
    // Routes
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while ( node1 )
    {
        Route *pRoute = node1->GetData();
        if ( !pRoute->m_bIsTrack )
        {
            wxXmlNode *track_node = ::CreateGPXRouteStatic ( pRoute );
            RNode->AddChild ( track_node );
        }
        node1 = node1->GetNext();
    }
}

void AppendGPXTracks ( wxXmlNode *RNode )
{
    // Tracks
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while ( node1 )
    {
        Route *pRoute = node1->GetData();
        if ( pRoute->m_bIsTrack )
        {
            wxXmlNode *track_node = ::CreateGPXTrackStatic ( pRoute );
            RNode->AddChild ( track_node );
        }
        node1 = node1->GetNext();
    }
}

#endif

// This function parses a string containing a GPX time representation
// and returns a wxDateTime containing the UTC corresponding to the
// input. The function return value is a pointer past the last valid
// character parsed (if successful) or NULL (if the string is invalid).
//
// Valid GPX time strings are in ISO 8601 format as follows:
//
//   [-]<YYYY>-<MM>-<DD>T<hh>:<mm>:<ss>Z|(+|-<hh>:<mm>)
//
// For example, 2010-10-30T14:34:56Z and 2010-10-30T14:34:56-04:00
// are the same time. The first is UTC and the second is EDT.

const wxChar *ParseGPXDateTime( wxDateTime &dt, const wxChar *datetime )
{
    long sign, hrs_west, mins_west;
    const wxChar *end;

    // Skip any leading whitespace
    while( isspace( *datetime ) )
        datetime++;

    // Skip (and ignore) leading hyphen
    if( *datetime == wxT('-') ) datetime++;

    // Parse and validate ISO 8601 date/time string
    if( ( end = dt.ParseFormat( datetime, wxT("%Y-%m-%dT%T") ) ) != NULL ) {

        // Invalid date/time
        if( *end == 0 ) return NULL;

        // ParseFormat outputs in UTC if the controlling
        // wxDateTime class instance has not been initialized.

        // Date/time followed by UTC time zone flag, so we are done
        else
            if( *end == wxT('Z') ) {
                end++;
                return end;
            }

            // Date/time followed by given number of hrs/mins west of UTC
            else
                if( *end == wxT('+') || *end == wxT('-') ) {

                    // Save direction from UTC
                    if( *end == wxT('+') ) sign = 1;
                    else
                        sign = -1;
                    end++;

                    // Parse hrs west of UTC
                    if( isdigit( *end ) && isdigit( *( end + 1 ) ) && *( end + 2 ) == wxT(':') ) {

                        // Extract and validate hrs west of UTC
                        wxString( end ).ToLong( &hrs_west );
                        if( hrs_west > 12 ) return NULL;
                        end += 3;

                        // Parse mins west of UTC
                        if( isdigit( *end ) && isdigit( *( end + 1 ) ) ) {

                            // Extract and validate mins west of UTC
                            wxChar mins[3];
                            mins[0] = *end;
                            mins[1] = *( end + 1 );
                            mins[2] = 0;
                            wxString( mins ).ToLong( &mins_west );
                            if( mins_west > 59 ) return NULL;

                            // Apply correction
                            dt -= sign * wxTimeSpan( hrs_west, mins_west, 0, 0 );
                            return end + 2;
                        } else
                            // Missing mins digits
                            return NULL;
                    } else
                        // Missing hrs digits or colon
                        return NULL;
                } else
                    // Unknown field after date/time (not UTC, not hrs/mins
                    //  west of UTC)
                    return NULL;
    } else
        // Invalid ISO 8601 date/time
        return NULL;
}

RoutePoint *LoadGPXWaypoint( GpxWptElement *wptnode, wxString def_symbol_name, bool b_fullviz )
{
//FIXME: implement the parsing in GpxWptElement and get rid of it here!
    wxString LatString = wxString::FromUTF8( wptnode->Attribute( "lat" ) );
    wxString LonString = wxString::FromUTF8( wptnode->Attribute( "lon" ) );

    bool bviz = false;
    bool bviz_name = false;
    bool bauto_name = false;
    bool bshared = false;
    bool b_propvizname = false;
    bool b_propviz = false;

    wxString SymString = def_symbol_name; //_T ( "empty" );                // default icon
    wxString NameString;
    wxString DescString;
    wxString TypeString;
    wxString ChildName;
    wxString PropString;
    wxString GuidString;
    wxDateTime dt;

    HyperlinkList *linklist = NULL;

    wxString HrefString = _T ( "" );
    wxString HrefTextString = _T ( "" );
    wxString HrefTypeString = _T ( "" );

    TiXmlNode *child;
    for( child = wptnode->FirstChild(); child != 0; child = child->NextSibling() ) {
        ChildName = wxString::FromUTF8( child->Value() );

        if( ChildName == _T ( "sym" ) ) {
            TiXmlNode *child1 = child->FirstChild();
            if( child1 != NULL ) SymString = wxString::FromUTF8( child1->ToText()->Value() );
        } else
            if( ChildName == _T ( "name" ) ) {
                TiXmlNode *child1 = child->FirstChild();
                if( child1 != NULL ) NameString = wxString::FromUTF8( child1->ToText()->Value() );
            } else
                if( ChildName == _T ( "desc" ) ) {
                    TiXmlNode *child1 = child->FirstChild();
                    if( child1 != NULL ) DescString = wxString::FromUTF8(
                            child1->ToText()->Value() );

                } else
                    if( ChildName == _T ( "type" ) ) {
                        TiXmlNode *child1 = child->FirstChild();

                        if( child1 != NULL ) TypeString = wxString::FromUTF8(
                                child1->ToText()->Value() );

                    }

                    else
                        if( ChildName == _T ( "time" ) ) {
                            TiXmlNode *child1 = child->FirstChild();
                            if( child1 != NULL ) {
                                wxString TimeString = wxString::FromUTF8(
                                        child1->ToText()->Value() );

                                if( TimeString.Len() ) {
                                    // Parse time string
                                    ParseGPXDateTime( dt, TimeString );

                                }
                            }
                        }
                        // Read hyperlink
                        else
                            if( ChildName == _T ( "link" ) ) {
                                if( linklist == NULL ) linklist = new HyperlinkList;
                                TiXmlAttribute * attr = ( (TiXmlElement*) child )->FirstAttribute();
                                if( attr ) HrefString = wxString::FromUTF8( attr->Value() );

                                TiXmlNode *child1;
                                for( child1 = child->FirstChild(); child1 != 0;
                                        child1 = child1->NextSibling() ) {
                                    wxString LinkString = wxString::FromUTF8( child1->Value() );

                                    if( LinkString == _T ( "text" ) ) {
                                        TiXmlNode *child1a = child1->FirstChild();
                                        HrefTextString = wxString::FromUTF8(
                                                child1a->ToText()->Value() );
                                    }

                                    if( LinkString == _T ( "type" ) ) {
                                        TiXmlNode *child1a = child1->FirstChild();
                                        HrefTypeString = wxString::FromUTF8(
                                                child1a->ToText()->Value() );
                                    }
                                }

                                Hyperlink *link = new Hyperlink;
                                link->Link = HrefString;
                                link->DescrText = HrefTextString;
                                link->Type = HrefTypeString;
                                linklist->Append( link );
                            }

                            //    Old invalid format, not written in opencpn 1.3.6 and above.
                            else
                                if( ChildName == _T ( "prop" ) ) {
                                    TiXmlNode *child1 = child->FirstChild();
                                    if( child1 != NULL ) PropString = wxString::FromUTF8(
                                            child1->ToText()->Value() );
                                }

                                //    New Proper GPX Format....
                                else
                                    if( ChildName == _T ( "extensions" ) ) {
                                        TiXmlNode *ext_child;
                                        for( ext_child = child->FirstChild(); ext_child != 0;
                                                ext_child = ext_child->NextSibling() ) {
                                            wxString ext_name = wxString::FromUTF8(
                                                    ext_child->Value() );
                                            if( ext_name == _T ( "opencpn:prop" ) ) {
                                                TiXmlNode *prop_child = ext_child->FirstChild();
                                                if( prop_child != NULL ) PropString =
                                                        wxString::FromUTF8(
                                                                prop_child->ToText()->Value() );
                                            } else
                                                if( ext_name == _T ( "opencpn:guid" ) ) {
                                                    TiXmlNode *prop_child = ext_child->FirstChild();
                                                    if( prop_child != NULL ) GuidString =
                                                            wxString::FromUTF8(
                                                                    prop_child->ToText()->Value() );
                                                }

                                                else
                                                    if( ext_name == _T ( "opencpn:viz" ) ) {
                                                        TiXmlNode *prop_child =
                                                                ext_child->FirstChild();
                                                        if( prop_child != NULL ) {
                                                            b_propviz = true;
                                                            wxString s = wxString::FromUTF8(
                                                                    prop_child->ToText()->Value() );
                                                            long v = 0;
                                                            if( s.ToLong( &v ) ) bviz = ( v != 0 );
                                                        }
                                                    } else
                                                        if( ext_name == _T ( "opencpn:viz_name" ) ) {
                                                            TiXmlNode *prop_child =
                                                                    ext_child->FirstChild();
                                                            if( prop_child != NULL ) {
                                                                b_propvizname = true;
                                                                wxString s =
                                                                        wxString::FromUTF8(
                                                                                prop_child->ToText()->Value() );
                                                                long v = 0;
                                                                if( s.ToLong( &v ) ) bviz_name = ( v
                                                                        != 0 );
                                                            }
                                                        } else
                                                            if( ext_name
                                                                    == _T ( "opencpn:auto_name" ) ) {
                                                                TiXmlNode *prop_child =
                                                                        ext_child->FirstChild();
                                                                if( prop_child != NULL ) {
                                                                    wxString s =
                                                                            wxString::FromUTF8(
                                                                                    prop_child->ToText()->Value() );
                                                                    long v = 0;
                                                                    if( s.ToLong( &v ) ) bauto_name =
                                                                            ( v != 0 );
                                                                }
                                                            } else
                                                                if( ext_name
                                                                        == _T ( "opencpn:shared" ) ) {
                                                                    TiXmlNode *prop_child =
                                                                            ext_child->FirstChild();
                                                                    if( prop_child != NULL ) {
                                                                        wxString s =
                                                                                wxString::FromUTF8(
                                                                                        prop_child->ToText()->Value() );
                                                                        long v = 0;
                                                                        if( s.ToLong( &v ) ) bshared =
                                                                                ( v != 0 );
                                                                    }
                                                                }
                                        }
                                    }
    }

    // Create waypoint
    double rlat;
    double rlon;
    LatString.ToDouble( &rlat );
    LonString.ToDouble( &rlon );

    if( g_bIsNewLayer ) GuidString = _T("");
    RoutePoint *pWP = new RoutePoint( rlat, rlon, SymString, NameString, GuidString, false ); // do not add to global WP list yet...
    pWP->m_MarkDescription = DescString;

    if( b_propvizname ) pWP->m_bShowName = bviz_name;
    else
        if( b_fullviz ) pWP->m_bShowName = true;
        else
            pWP->m_bShowName = false;

    if( b_propviz ) pWP->m_bIsVisible = bviz;
    else
        if( b_fullviz ) pWP->m_bIsVisible = true;

    pWP->m_bKeepXRoute = bshared;
    pWP->m_bDynamicName = bauto_name;

    if( dt.IsValid() ) pWP->m_CreateTime = dt;
    else
        pWP->m_CreateTime = wxInvalidDateTime;

    if( linklist ) {
        delete pWP->m_HyperlinkList;                    // created in RoutePoint ctor
        pWP->m_HyperlinkList = linklist;
    }

    return ( pWP );
}

void GPXLoadTrack( GpxTrkElement* trknode, bool b_fullviz )
{
//    CALLGRIND_START_INSTRUMENTATION

    //FIXME: This should be moved to GpxTrkElement
    wxString RouteName;
    unsigned short int GPXSeg;                   // pjotrc 2010.02.27

    bool b_propviz = false;
    bool b_viz = true;

    wxString Name = wxString::FromUTF8( trknode->Value() );
    if( Name == _T ( "trk" ) ) {
        Track *pTentTrack = new Track();
        GPXSeg = 0;                                     // pjotrc 2010.02.27

        TiXmlNode *tschild;

        RoutePoint *pWp = NULL;

        for( tschild = trknode->FirstChild(); tschild != 0; tschild = tschild->NextSibling() ) {
            wxString ChildName = wxString::FromUTF8( tschild->Value() );
            if( ChildName == _T ( "trkseg" ) ) {
                GPXSeg += 1;                                          // pjotrc 2010.02.27

                //    Official GPX spec calls for trkseg to have children trkpt
                TiXmlNode *tpchild;
                for( tpchild = tschild->FirstChild(); tpchild != 0; tpchild =
                        tpchild->NextSibling() ) {
                    wxString tpChildName = wxString::FromUTF8( tpchild->Value() );
                    if( tpChildName == _T("trkpt") ) {
                        pWp = ::LoadGPXWaypoint( (GpxWptElement *) tpchild, _T("empty"),
                                false/*b_fullviz*/);
                        pTentTrack->AddPoint( pWp, false, true );          // defer BBox calculation
                        pWp->m_bIsInRoute = false;                      // Hack
                        pWp->m_bIsInTrack = true;
                        pWp->m_GPXTrkSegNo = GPXSeg;
                        pWayPointMan->m_pWayPointList->Append( pWp );
                    }
                }
            } else
                if( ChildName == _T ( "name" ) ) {
                    TiXmlNode *child1 = tschild->FirstChild();
                    if( child1 )                            // name will always be in first child??
                    RouteName = wxString::FromUTF8( child1->ToText()->Value() );
                }

                else
                    if( ChildName == _T ( "extensions" ) ) {
                        TiXmlNode *ext_child;
                        for( ext_child = tschild->FirstChild(); ext_child != 0; ext_child =
                                ext_child->NextSibling() ) {
                            wxString ext_name = wxString::FromUTF8( ext_child->Value() );
                            if( ext_name == _T ( "opencpn:start" ) ) {
                                TiXmlNode *s_child = ext_child->FirstChild();
                                if( s_child != NULL ) pTentTrack->m_RouteStartString =
                                        wxString::FromUTF8( s_child->ToText()->Value() );
                            } else
                                if( ext_name == _T ( "opencpn:end" ) ) {
                                    TiXmlNode *e_child = ext_child->FirstChild();
                                    if( e_child != NULL ) pTentTrack->m_RouteEndString =
                                            wxString::FromUTF8( e_child->ToText()->Value() );
                                }

                                else
                                    if( ext_name == _T ( "opencpn:viz" ) ) {
                                        TiXmlNode *v_child = ext_child->FirstChild();
                                        if( v_child != NULL ) {
                                            b_propviz = true;
                                            wxString viz = wxString::FromUTF8(
                                                    v_child->ToText()->Value() );
                                            b_viz = ( viz == _T("1") );
                                        }
                                    } else
                                        if( ext_name == _T ( "opencpn:style" ) ) {
                                            TiXmlAttribute * attr;
                                            for( attr =
                                                    ( (TiXmlElement*) ext_child )->FirstAttribute();
                                                    attr != 0; attr = attr->Next() ) {
                                                if( attr ) {
                                                    if( strcmp( attr->Name(), "style" ) == 0 ) pTentTrack->m_style =
                                                            atoi( attr->Value() );
                                                    else
                                                        if( strcmp( attr->Name(), "width" ) == 0 ) pTentTrack->m_width =
                                                                atoi( attr->Value() );
                                                }
                                            }

                                        } else
                                            if( ext_name == _T ( "opencpn:guid" ) ) {
                                                TiXmlNode *g_child = ext_child->FirstChild();
                                                if( g_child != NULL && ( !g_bIsNewLayer ) ) pTentTrack->m_GUID =
                                                        wxString::FromUTF8(
                                                                g_child->ToText()->Value() );
                                            } else
                                                if( ext_name.EndsWith( _T ( "TrackExtension" ) ) ) //Parse GPXX color
                                                        {
                                                    TiXmlNode *gpxx_child;
                                                    for( gpxx_child = ext_child->FirstChild();
                                                            gpxx_child != 0;
                                                            gpxx_child =
                                                                    gpxx_child->NextSibling() ) {
                                                        wxString gpxx_name = wxString::FromUTF8(
                                                                gpxx_child->Value() );
                                                        if( gpxx_name.EndsWith(
                                                                _T ( "DisplayColor" ) ) ) {
                                                            TiXmlNode *s_child =
                                                                    gpxx_child->FirstChild();
                                                            if( s_child != NULL ) pTentTrack->m_Colour =
                                                                    wxString::FromUTF8(
                                                                            s_child->ToText()->Value() );
                                                        }
                                                    }
                                                }
                        }
                    }
        }

        pTentTrack->m_RouteNameString = RouteName;

        //    Search for an identical route/track already in place.  If found, discard this one

        bool bAddtrack = true;
        wxRouteListNode *route_node = pRouteList->GetFirst();
        while( route_node ) {
            Route *proute = route_node->GetData();

            if( proute->IsEqualTo( pTentTrack ) ) {
                if( proute->m_bIsTrack ) {
                    bAddtrack = false;
                    break;
                }
            }
            route_node = route_node->GetNext();                         // next route
        }

        //    If the track has only 1 point, don't load it.
        //    This usually occurs if some points were dscarded above as being co-incident.
        if( pTentTrack->GetnPoints() < 2 ) bAddtrack = false;

//    TODO  All this trouble for a tentative route.......Should make some Route methods????
        if( bAddtrack ) {
            if( ::RouteExists( pTentTrack->m_GUID ) ) { //We are importing a different route with the same guid, so let's generate it a new guid
                pTentTrack->m_GUID = pWayPointMan->CreateGUID( NULL );
                //Now also change guids for the routepoints
                wxRoutePointListNode *pthisnode = ( pTentTrack->pRoutePointList )->GetFirst();
                while( pthisnode ) {
                    pthisnode->GetData()->m_GUID = pWayPointMan->CreateGUID( NULL );
                    pthisnode = pthisnode->GetNext();
                    //FIXME: !!!!!! the shared waypoint gets part of both the routes -> not  goood at all
                }
            }
            pRouteList->Append( pTentTrack );

            if( g_bIsNewLayer ) pTentTrack->SetVisible( g_bLayerViz );
            else
                if( b_propviz ) pTentTrack->SetVisible( b_viz );
                else
                    if( b_fullviz ) pTentTrack->SetVisible();

            //    Do the (deferred) calculation of Track BBox
            pTentTrack->CalculateBBox();

            //    Add the selectable points and segments

            int ip = 0;
            float prev_rlat = 0., prev_rlon = 0.;
            RoutePoint *prev_pConfPoint = NULL;

            wxRoutePointListNode *node = pTentTrack->pRoutePointList->GetFirst();
            while( node ) {

                RoutePoint *prp = node->GetData();

                if( ip ) pSelect->AddSelectableTrackSegment( prev_rlat, prev_rlon, prp->m_lat,
                        prp->m_lon, prev_pConfPoint, prp, pTentTrack );

                prev_rlat = prp->m_lat;
                prev_rlon = prp->m_lon;
                prev_pConfPoint = prp;

                ip++;

                node = node->GetNext();
            }
        } else {

            // walk the route, deleting points used only by this route
            wxRoutePointListNode *pnode = ( pTentTrack->pRoutePointList )->GetFirst();
            while( pnode ) {
                RoutePoint *prp = pnode->GetData();

                // check all other routes to see if this point appears in any other route
                Route *pcontainer_route = g_pRouteMan->FindRouteContainingWaypoint( prp );

                if( pcontainer_route == NULL ) {
                    prp->m_bIsInRoute = false; // Take this point out of this (and only) track/route
                    if( !prp->m_bKeepXRoute ) {
                        pConfig->DeleteWayPoint( prp );
                        delete prp;
                    }
                }

                pnode = pnode->GetNext();
            }

            delete pTentTrack;
        }
    }

    //   CALLGRIND_STOP_INSTRUMENTATION

}

Route *LoadGPXTrack( GpxTrkElement *trknode, bool b_fullviz )
{
    //FIXME: This should be moved to GpxRteElement
    Route *pTentRoute = new Route();

    int ip = 0;
    TiXmlElement *child;
    for( child = trknode->FirstChildElement(); child != 0; child = child->NextSiblingElement() ) {
        wxString ChildName = wxString::FromUTF8( child->Value() );
        if( ChildName == _T ( "rtept" ) ) {
            RoutePoint *pWp = LoadGPXWaypoint( (GpxWptElement *) child, _T("square"), b_fullviz );

            RoutePoint *pExisting = WaypointExists( pWp->GetName(), pWp->m_lat, pWp->m_lon );

            if( !pExisting ) {
                if( NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->Append( pWp );

                pTentRoute->AddPoint( pWp, false );                 // don't auto-rename numerically

                //pWp->m_ConfigWPNum = 1000 + ( routenum * 100 ) + ip;  // dummy mark number
            } else {
                pTentRoute->AddPoint( pExisting, false );           // don't auto-rename numerically
            }
            ip++;
        }

        else
            if( ChildName == _T ( "name" ) ) {
                TiXmlNode *namechild;
                for( namechild = child->FirstChild(); namechild != 0;
                        namechild = namechild->NextSibling() ) {
                    pTentRoute->m_RouteNameString = wxString::FromUTF8(
                            namechild->ToText()->Value() );
                }
            }

            else
                if( ChildName == _T ( "extensions" ) ) {
                    TiXmlNode *ext_child;
                    for( ext_child = child->FirstChild(); ext_child != 0;
                            ext_child = ext_child->NextSibling() ) {
                        wxString ext_name = wxString::FromUTF8( ext_child->Value() );
                        if( ext_name == _T ( "opencpn:start" ) ) {
                            TiXmlNode *s_child = ext_child->FirstChild();
                            if( s_child != NULL ) pTentRoute->m_RouteStartString =
                                    wxString::FromUTF8( s_child->ToText()->Value() );
                        } else
                            if( ext_name == _T ( "opencpn:end" ) ) {
                                TiXmlNode *e_child = ext_child->FirstChild();
                                if( e_child != NULL ) pTentRoute->m_RouteEndString =
                                        wxString::FromUTF8( e_child->ToText()->Value() );
                            } else
                                if( ext_name == _T ( "opencpn:viz" ) ) {
                                    TiXmlNode *v_child = ext_child->FirstChild();
                                    if( v_child != NULL ) {
                                        wxString viz = wxString::FromUTF8(
                                                v_child->ToText()->Value() );
                                        if( viz == _T("0") ) pTentRoute->SetVisible( false );
                                    }
                                } else
                                    if( ext_name == _T ( "opencpn:guid" ) ) {
                                        TiXmlNode *g_child = ext_child->FirstChild();
                                        if( g_child != NULL && ( !g_bIsNewLayer ) ) {
                                            pTentRoute->m_GUID = wxString::FromUTF8(
                                                    g_child->ToText()->Value() );
                                        }
                                    } else
                                        if( ext_name.EndsWith( _T ( "TrackExtension" ) ) ) //Parse GPXX color
                                                {
                                            TiXmlNode *gpxx_child;
                                            for( gpxx_child = ext_child->FirstChild();
                                                    gpxx_child != 0;
                                                    gpxx_child = gpxx_child->NextSibling() ) {
                                                wxString gpxx_name = wxString::FromUTF8(
                                                        gpxx_child->Value() );
                                                if( gpxx_name.EndsWith( _T ( "DisplayColor" ) ) ) {
                                                    TiXmlNode *s_child = gpxx_child->FirstChild();
                                                    if( s_child != NULL ) pTentRoute->m_Colour =
                                                            wxString::FromUTF8(
                                                                    s_child->ToText()->Value() );
                                                }
                                            }
                                        }
                    }
                }
    }
    pTentRoute->m_bIsTrack = true;
    return pTentRoute;
}

Route *LoadGPXRoute( GpxRteElement *rtenode, int routenum, bool b_fullviz )
{
    bool b_propviz = false;
    bool b_viz = true;

    //FIXME: This should be moved to GpxRteElement
    Route *pTentRoute = new Route();

    int ip = 0;
    TiXmlElement *child;
    for( child = rtenode->FirstChildElement(); child != 0; child = child->NextSiblingElement() ) {
        wxString ChildName = wxString::FromUTF8( child->Value() );
        if( ChildName == _T ( "rtept" ) ) {
            RoutePoint *pWp = LoadGPXWaypoint( (GpxWptElement *) child, _T("square"), b_fullviz );

            RoutePoint *pExisting = WaypointExists( pWp->GetName(), pWp->m_lat, pWp->m_lon );

            if( !pExisting ) {
                if( NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->Append( pWp );

                pTentRoute->AddPoint( pWp, false );                 // don't auto-rename numerically
                pWp->m_ConfigWPNum = 1000 + ( routenum * 100 ) + ip;  // dummy mark number
                pWp->m_bIsInLayer = g_bIsNewLayer;
                if( g_bIsNewLayer ) pWp->m_LayerID = g_LayerIdx;
                else
                    pWp->m_LayerID = 0;
            } else {
                pTentRoute->AddPoint( pExisting, false );           // don't auto-rename numerically
                delete pWp;
            }
            ip++;
        }

        else
            if( ChildName == _T ( "name" ) ) {
                TiXmlNode *namechild;
                for( namechild = child->FirstChild(); namechild != 0;
                        namechild = namechild->NextSibling() ) {
                    pTentRoute->m_RouteNameString = wxString::FromUTF8(
                            namechild->ToText()->Value() );
                }
            }

            else
                if( ChildName == _T ( "extensions" ) ) {
                    TiXmlNode *ext_child;
                    for( ext_child = child->FirstChild(); ext_child != 0;
                            ext_child = ext_child->NextSibling() ) {
                        wxString ext_name = wxString::FromUTF8( ext_child->Value() );
                        if( ext_name == _T ( "opencpn:start" ) ) {
                            TiXmlNode *s_child = ext_child->FirstChild();
                            if( s_child != NULL ) pTentRoute->m_RouteStartString =
                                    wxString::FromUTF8( s_child->ToText()->Value() );
                        } else
                            if( ext_name == _T ( "opencpn:end" ) ) {
                                TiXmlNode *e_child = ext_child->FirstChild();
                                if( e_child != NULL ) pTentRoute->m_RouteEndString =
                                        wxString::FromUTF8( e_child->ToText()->Value() );
                            } else
                                if( ext_name == _T ( "opencpn:viz" ) ) {
                                    TiXmlNode *v_child = ext_child->FirstChild();
                                    if( v_child != NULL ) {
                                        b_propviz = true;
                                        wxString viz = wxString::FromUTF8(
                                                v_child->ToText()->Value() );
                                        b_viz = ( viz == _T("1") );
                                    }
                                } else
                                    if( ext_name == _T ( "opencpn:guid" ) ) {
                                        TiXmlNode *g_child = ext_child->FirstChild();
                                        if( g_child != NULL && ( !g_bIsNewLayer ) ) {
                                            pTentRoute->m_GUID = wxString::FromUTF8(
                                                    g_child->ToText()->Value() );
                                        }
                                    } else
                                        if( ext_name == _T ( "opencpn:style" ) ) {
                                            TiXmlAttribute * attr;
                                            for( attr =
                                                    ( (TiXmlElement*) ext_child )->FirstAttribute();
                                                    attr != 0; attr = attr->Next() ) {
                                                if( attr ) {
                                                    if( strcmp( attr->Name(), "style" ) == 0 ) pTentRoute->m_style =
                                                            atoi( attr->Value() );
                                                    else
                                                        if( strcmp( attr->Name(), "width" ) == 0 ) pTentRoute->m_width =
                                                                atoi( attr->Value() );
                                                }
                                            }

                                        } else
                                            if( ext_name.EndsWith( _T ( "RouteExtension" ) ) ) //Parse GPXX color
                                                    {
                                                TiXmlNode *gpxx_child;
                                                for( gpxx_child = ext_child->FirstChild();
                                                        gpxx_child != 0;
                                                        gpxx_child = gpxx_child->NextSibling() ) {
                                                    wxString gpxx_name = wxString::FromUTF8(
                                                            gpxx_child->Value() );
                                                    if( gpxx_name.EndsWith(
                                                            _T ( "DisplayColor" ) ) ) {
                                                        TiXmlNode *s_child =
                                                                gpxx_child->FirstChild();
                                                        if( s_child != NULL ) pTentRoute->m_Colour =
                                                                wxString::FromUTF8(
                                                                        s_child->ToText()->Value() );
                                                    }
                                                }
                                            }
                    }
                }
    }
    if( g_bIsNewLayer )
        pTentRoute->SetVisible( g_bLayerViz, true );
    else
        if( b_propviz )
            pTentRoute->SetVisible( b_viz, false );
        else
            if( b_fullviz ) pTentRoute->SetVisible( true, false );

    return pTentRoute;
}

void UpdateRoute( Route *pTentRoute )
{
    Route * rt = ::RouteExists( pTentRoute->m_GUID );
    if( rt ) {
        wxRoutePointListNode *node = pTentRoute->pRoutePointList->GetFirst();
        while( node ) {
            RoutePoint *prp = node->GetData();
            RoutePoint *ex_rp = rt->GetPoint( prp->m_GUID );
            if( ex_rp ) {
                ex_rp->m_lat = prp->m_lat;
                ex_rp->m_lon = prp->m_lon;
                ex_rp->m_IconName = prp->m_IconName;
                ex_rp->m_MarkDescription = prp->m_MarkDescription;
                ex_rp->SetName( prp->GetName() );
            } else {
                pSelect->AddSelectableRoutePoint( prp->m_lat, prp->m_lon, prp );
            }
            node = node->GetNext();
        }
    } else {
        ::InsertRoute( pTentRoute, pTentRoute->m_ConfigRouteNum );
    }
}

void InsertRoute( Route *pTentRoute, int routenum )
{
    pRouteList->Append( pTentRoute );
    pTentRoute->m_ConfigRouteNum = routenum;

    pTentRoute->RebuildGUIDList();                  // ensure the GUID list is intact

    //    Add the selectable points and segments
    int ip = 0;
    float prev_rlat = 0., prev_rlon = 0.;
    RoutePoint *prev_pConfPoint = NULL;

    wxRoutePointListNode *node = pTentRoute->pRoutePointList->GetFirst();
    while( node ) {
        RoutePoint *prp = node->GetData();

        pSelect->AddSelectableRoutePoint( prp->m_lat, prp->m_lon, prp );

        if( ip ) pSelect->AddSelectableRouteSegment( prev_rlat, prev_rlon, prp->m_lat, prp->m_lon,
                prev_pConfPoint, prp, pTentRoute );

        prev_rlat = prp->m_lat;
        prev_rlon = prp->m_lon;
        prev_pConfPoint = prp;

        ip++;

        node = node->GetNext();
    }
}

void GPXLoadRoute( GpxRteElement* rtenode, int routenum, bool b_fullviz )
{
    wxString Name = wxString::FromUTF8( rtenode->Value() );

    if( Name == _T ( "rte" ) ) //FIXME: should not be here
    {
        Route *pTentRoute = ::LoadGPXRoute( rtenode, routenum, b_fullviz );

//    TODO  All this trouble for a tentative route.......Should make some Route methods????
        if( !::RouteExists( pTentRoute ) ) {
            if( ::RouteExists( pTentRoute->m_GUID ) ) { //We are importing a different route with the same guid, so let's generate it a new guid
                pTentRoute->m_GUID = pWayPointMan->CreateGUID( NULL );
                //Now also change guids for the routepoints
                wxRoutePointListNode *pthisnode = ( pTentRoute->pRoutePointList )->GetFirst();
                while( pthisnode ) {
                    pthisnode->GetData()->m_GUID = pWayPointMan->CreateGUID( NULL );
                    pthisnode = pthisnode->GetNext();
                    //FIXME: !!!!!! the shared routepoint gets part of both the routes -> not  goood at all
                }
            }
            ::InsertRoute( pTentRoute, routenum );
        } else {
            // walk the route, deleting points used only by this route
            wxRoutePointListNode *pnode = ( pTentRoute->pRoutePointList )->GetFirst();
            while( pnode ) {
                RoutePoint *prp = pnode->GetData();

                // check all other routes to see if this point appears in any other route
                Route *pcontainer_route = g_pRouteMan->FindRouteContainingWaypoint( prp );

                if( pcontainer_route == NULL && prp->m_bIsInRoute ) {
                    prp->m_bIsInRoute = false;       // Take this point out of this (and only) route
                    if( !prp->m_bKeepXRoute ) {
                        pConfig->DeleteWayPoint( prp );
                        delete prp;
                    }
                }

                pnode = pnode->GetNext();
            }
            delete pTentRoute;
        }
    }
}

//---------------------------------------------------------------------------------
//          Private Font Manager and Helpers
//---------------------------------------------------------------------------------
#include <wx/fontdlg.h>
#include <wx/fontenum.h>
#include "wx/encinfo.h"
#include "wx/fontutil.h"

#ifdef __WXX11__
#include "/usr/X11R6/include/X11/Xlib.h"

//-----------------------------------------------------------------------------
// helper class - MyFontPreviewer
//-----------------------------------------------------------------------------

class MyFontPreviewer : public wxWindow
{
public:
    MyFontPreviewer ( wxWindow *parent, const wxSize& sz = wxDefaultSize ) : wxWindow ( parent, wxID_ANY, wxDefaultPosition, sz )
    {
    }

private:
    void OnPaint ( wxPaintEvent& event );
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE ( MyFontPreviewer, wxWindow )
EVT_PAINT ( MyFontPreviewer::OnPaint )
END_EVENT_TABLE()

void MyFontPreviewer::OnPaint ( wxPaintEvent& WXUNUSED ( event ) )
{
    wxPaintDC dc ( this );

    wxSize size = GetSize();
    wxFont font = GetFont();

    dc.SetPen ( *wxBLACK_PEN );
    dc.SetBrush ( *wxWHITE_BRUSH );
    dc.DrawRectangle ( 0, 0, size.x, size.y );

    if ( font.Ok() )
    {
        dc.SetFont ( font );
        // Calculate vertical centre
        long w, h;
        dc.GetTextExtent ( wxT ( "X" ), &w, &h );
        dc.SetTextForeground ( GetForegroundColour() );
        dc.SetClippingRegion ( 2, 2, size.x-4, size.y-4 );
        dc.DrawText ( GetName(),
                10, size.y/2 - h/2 );
        dc.DestroyClippingRegion();
    }
}

//-----------------------------------------------------------------------------
// X11FontPicker
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS ( X11FontPicker, wxDialog )

BEGIN_EVENT_TABLE ( X11FontPicker, wxDialog )
EVT_CHECKBOX ( wxID_FONT_UNDERLINE, X11FontPicker::OnChangeFont )
EVT_CHOICE ( wxID_FONT_STYLE, X11FontPicker::OnChangeFont )
EVT_CHOICE ( wxID_FONT_WEIGHT, X11FontPicker::OnChangeFont )
EVT_CHOICE ( wxID_FONT_FAMILY, X11FontPicker::OnChangeFace )
EVT_CHOICE ( wxID_FONT_COLOUR, X11FontPicker::OnChangeFont )
EVT_CHOICE ( wxID_FONT_SIZE, X11FontPicker::OnChangeFont )

EVT_CLOSE ( X11FontPicker::OnCloseWindow )
END_EVENT_TABLE()

#define SCALEABLE_SIZES 11
static wxString scaleable_pointsize[SCALEABLE_SIZES] =
{
    wxT ( "6" ),
    wxT ( "8" ),
    wxT ( "10" ),
    wxT ( "12" ),
    wxT ( "14" ),
    wxT ( "16" ),
    wxT ( "18" ),
    wxT ( "20" ),
    wxT ( "24" ),
    wxT ( "30" ),
    wxT ( "36" )
};

#define NUM_COLS 48
static wxString wxColourDialogNames[NUM_COLS]= {wxT ( "ORANGE" ),
    wxT ( "GOLDENROD" ),
    wxT ( "WHEAT" ),
    wxT ( "SPRING GREEN" ),
    wxT ( "SKY BLUE" ),
    wxT ( "SLATE BLUE" ),
    wxT ( "MEDIUM VIOLET RED" ),
    wxT ( "PURPLE" ),

    wxT ( "RED" ),
    wxT ( "YELLOW" ),
    wxT ( "MEDIUM SPRING GREEN" ),
    wxT ( "PALE GREEN" ),
    wxT ( "CYAN" ),
    wxT ( "LIGHT STEEL BLUE" ),
    wxT ( "ORCHID" ),
    wxT ( "LIGHT MAGENTA" ),

    wxT ( "BROWN" ),
    wxT ( "YELLOW" ),
    wxT ( "GREEN" ),
    wxT ( "CADET BLUE" ),
    wxT ( "MEDIUM BLUE" ),
    wxT ( "MAGENTA" ),
    wxT ( "MAROON" ),
    wxT ( "ORANGE RED" ),

    wxT ( "FIREBRICK" ),
    wxT ( "CORAL" ),
    wxT ( "FOREST GREEN" ),
    wxT ( "AQUARAMINE" ),
    wxT ( "BLUE" ),
    wxT ( "NAVY" ),
    wxT ( "THISTLE" ),
    wxT ( "MEDIUM VIOLET RED" ),

    wxT ( "INDIAN RED" ),
    wxT ( "GOLD" ),
    wxT ( "MEDIUM SEA GREEN" ),
    wxT ( "MEDIUM BLUE" ),
    wxT ( "MIDNIGHT BLUE" ),
    wxT ( "GREY" ),
    wxT ( "PURPLE" ),
    wxT ( "KHAKI" ),

    wxT ( "BLACK" ),
    wxT ( "MEDIUM FOREST GREEN" ),
    wxT ( "KHAKI" ),
    wxT ( "DARK GREY" ),
    wxT ( "SEA GREEN" ),
    wxT ( "LIGHT GREY" ),
    wxT ( "MEDIUM SLATE BLUE" ),
    wxT ( "WHITE" )
};

/*
 * Generic X11FontPicker
 */

void X11FontPicker::Init()
{
    m_useEvents = false;
    m_previewer = NULL;
    Create ( m_parent );
}

X11FontPicker::~X11FontPicker()
{
}

void X11FontPicker::OnCloseWindow ( wxCloseEvent& WXUNUSED ( event ) )
{
    EndModal ( wxID_CANCEL );
}

bool X11FontPicker::DoCreate ( wxWindow *parent )
{
    if ( !wxDialog::Create ( parent , wxID_ANY , _T ( "Choose Font" ) , wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE,
                    _T ( "fontdialog" ) ) )
    {
        wxFAIL_MSG ( wxT ( "wxFontDialog creation failed" ) );
        return false;
    }

    InitializeAllAvailableFonts();
    InitializeFont();
    CreateWidgets();

// sets initial font in preview area
    wxCommandEvent dummy;
    OnChangeFont ( dummy );

    return true;
}

int X11FontPicker::ShowModal()
{
    int ret = wxDialog::ShowModal();

    if ( ret != wxID_CANCEL )
    {
        dialogFont = *pPreviewFont;
        m_fontData.m_chosenFont = dialogFont;
    }

    return ret;
}

void X11FontPicker::InitializeAllAvailableFonts()
{

    // get the Array of all fonts facenames
    wxString pattern;
    pattern.Printf ( wxT ( "-*-*-*-*-*-*-*-*-*-*-*-*-iso8859-1" ) );

    int nFonts;
    char ** list = XListFonts ( ( Display * ) wxGetDisplay(), pattern.mb_str(), 32767, &nFonts );

    pFaceNameArray = new wxArrayString;
    unsigned int jname;
    for ( int i=0; i < nFonts; i++ )
    {
        wxStringTokenizer st ( wxString ( list[i] ), _T ( "-" ) );
        st.GetNextToken();
        st.GetNextToken();
        wxString facename = st.GetNextToken();
        for ( jname=0; jname<pFaceNameArray->GetCount(); jname++ )
        {
            if ( facename == pFaceNameArray->Item ( jname ) )
            break;
        }
        if ( jname >= pFaceNameArray->GetCount() )
        {
            pFaceNameArray->Add ( facename );
        }

    }

}

// This should be application-settable
static bool ShowToolTips() {return false;}

void X11FontPicker::CreateWidgets()
{
    // layout

    bool is_pda = ( wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA );
    int noCols, noRows;
    if ( is_pda )
    {
        noCols = 2; noRows = 3;
    }
    else
    {
        noCols = 3; noRows = 2;
    }

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer ( wxVERTICAL );
    this->SetSizer ( itemBoxSizer2 );
    this->SetAutoLayout ( TRUE );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer ( wxVERTICAL );
    itemBoxSizer2->Add ( itemBoxSizer3, 1, wxEXPAND|wxALL, 5 );

    wxFlexGridSizer* itemGridSizer4 = new wxFlexGridSizer ( noRows, noCols, 0, 0 );
    itemBoxSizer3->Add ( itemGridSizer4, 0, wxEXPAND, 5 );

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer ( wxVERTICAL );
    itemGridSizer4->Add ( itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
    wxStaticText* itemStaticText6 = new wxStaticText ( this, wxID_STATIC, _ ( "&Font family:" ),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add ( itemStaticText6, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5 );

    wxChoice* itemChoice7 = new wxChoice ( this, wxID_FONT_FAMILY, wxDefaultPosition,
            wxDefaultSize, *pFaceNameArray, 0 );
    itemChoice7->SetHelpText ( _ ( "The font family." ) );
    if ( ShowToolTips() )
    itemChoice7->SetToolTip ( _ ( "The font family." ) );
    itemBoxSizer5->Add ( itemChoice7, 0, wxALIGN_LEFT|wxALL, 5 );

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer ( wxVERTICAL );
    itemGridSizer4->Add ( itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
    wxStaticText* itemStaticText9 = new wxStaticText ( this, wxID_STATIC, _ ( "&Style:" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add ( itemStaticText9, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5 );

    wxChoice* itemChoice10 = new wxChoice ( this, wxID_FONT_STYLE, wxDefaultPosition, wxDefaultSize );
    itemChoice10->SetHelpText ( _ ( "The font style." ) );
    if ( ShowToolTips() )
    itemChoice10->SetToolTip ( _ ( "The font style." ) );
    itemBoxSizer8->Add ( itemChoice10, 0, wxALIGN_LEFT|wxALL, 5 );

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer ( wxVERTICAL );
    itemGridSizer4->Add ( itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
    wxStaticText* itemStaticText12 = new wxStaticText ( this, wxID_STATIC, _ ( "&Weight:" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add ( itemStaticText12, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5 );

    wxChoice* itemChoice13 = new wxChoice ( this, wxID_FONT_WEIGHT, wxDefaultPosition, wxDefaultSize );
    itemChoice13->SetHelpText ( _ ( "The font weight." ) );
    if ( ShowToolTips() )
    itemChoice13->SetToolTip ( _ ( "The font weight." ) );
    itemBoxSizer11->Add ( itemChoice13, 0, wxALIGN_LEFT|wxALL, 5 );

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer ( wxVERTICAL );
    itemGridSizer4->Add ( itemBoxSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
    if ( m_fontData.GetEnableEffects() )
    {
        wxStaticText* itemStaticText15 = new wxStaticText ( this, wxID_STATIC, _ ( "C&olour:" ),
                wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer14->Add ( itemStaticText15, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5 );

        wxSize colourSize = wxDefaultSize;
        if ( is_pda )
        colourSize.x = 100;

        wxChoice* itemChoice16 = new wxChoice ( this, wxID_FONT_COLOUR, wxDefaultPosition,
                colourSize, NUM_COLS, wxColourDialogNames, 0 );
        itemChoice16->SetHelpText ( _ ( "The font colour." ) );
        if ( ShowToolTips() )
        itemChoice16->SetToolTip ( _ ( "The font colour." ) );
        itemBoxSizer14->Add ( itemChoice16, 0, wxALIGN_LEFT|wxALL, 5 );
    }

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer ( wxVERTICAL );
    itemGridSizer4->Add ( itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
    wxStaticText* itemStaticText18 = new wxStaticText ( this, wxID_STATIC, _ ( "&Point size:" ),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add ( itemStaticText18, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5 );

    wxChoice *pc = new wxChoice ( this, wxID_FONT_SIZE, wxDefaultPosition, wxDefaultSize );
    pc->SetHelpText ( _ ( "The font point size." ) );
    if ( ShowToolTips() )
    pc->SetToolTip ( _ ( "The font point size." ) );
    itemBoxSizer17->Add ( pc, 0, wxALIGN_LEFT|wxALL, 5 );

    if ( m_fontData.GetEnableEffects() )
    {
        wxBoxSizer* itemBoxSizer20 = new wxBoxSizer ( wxVERTICAL );
        itemGridSizer4->Add ( itemBoxSizer20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
        wxCheckBox* itemCheckBox21 = new wxCheckBox ( this, wxID_FONT_UNDERLINE, _ ( "&Underline" ),
                wxDefaultPosition, wxDefaultSize, 0 );
        itemCheckBox21->SetValue ( FALSE );
        itemCheckBox21->SetHelpText ( _ ( "Whether the font is underlined." ) );
        if ( ShowToolTips() )
        itemCheckBox21->SetToolTip ( _ ( "Whether the font is underlined." ) );
        itemBoxSizer20->Add ( itemCheckBox21, 0, wxALIGN_LEFT|wxALL, 5 );
    }

    if ( !is_pda )
    itemBoxSizer3->Add ( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

    wxStaticText* itemStaticText23 = new wxStaticText ( this, wxID_STATIC, _ ( "Preview:" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add ( itemStaticText23, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5 );

    MyFontPreviewer* itemWindow24 = new MyFontPreviewer ( this, wxSize ( 400, 80 ) );
    m_previewer = itemWindow24;
    itemWindow24->SetHelpText ( _ ( "Shows the font preview." ) );
    if ( ShowToolTips() )
    itemWindow24->SetToolTip ( _ ( "Shows the font preview." ) );
    itemBoxSizer3->Add ( itemWindow24, 0, wxEXPAND, 5 );

    wxBoxSizer* itemBoxSizer25 = new wxBoxSizer ( wxHORIZONTAL );
    itemBoxSizer3->Add ( itemBoxSizer25, 0, wxEXPAND, 5 );
    itemBoxSizer25->Add ( 5, 5, 1, wxEXPAND|wxALL, 5 );

    wxButton* itemButton28 = new wxButton ( this, wxID_CANCEL, _ ( "&Cancel" ), wxDefaultPosition, wxDefaultSize, 0 );
    if ( ShowToolTips() )
    itemButton28->SetToolTip ( _ ( "Click to cancel the font selection." ) );
    itemBoxSizer25->Add ( itemButton28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxButton* itemButton27 = new wxButton ( this, wxID_OK, _ ( "&OK" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton27->SetDefault();
    itemButton27->SetHelpText ( _ ( "Click to confirm the font selection." ) );
    if ( ShowToolTips() )
    itemButton27->SetToolTip ( _ ( "Click to confirm the font selection." ) );
    itemBoxSizer25->Add ( itemButton27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    familyChoice = ( wxChoice* ) FindWindow ( wxID_FONT_FAMILY );
    styleChoice = ( wxChoice* ) FindWindow ( wxID_FONT_STYLE );
    weightChoice = ( wxChoice* ) FindWindow ( wxID_FONT_WEIGHT );
    colourChoice = ( wxChoice* ) FindWindow ( wxID_FONT_COLOUR );
    pointSizeChoice = ( wxChoice* ) FindWindow ( wxID_FONT_SIZE );
    underLineCheckBox = ( wxCheckBox* ) FindWindow ( wxID_FONT_UNDERLINE );

    //    Get readable font items
    wxString gotfontnative = dialogFont.GetNativeFontInfoDesc();
    wxStringTokenizer st ( gotfontnative, _T ( "-" ) );
    st.GetNextToken();
    st.GetNextToken();
    wxString facename = st.GetNextToken();
    wxString weight = st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    wxString pointsize = st.GetNextToken();

    int ptsz = atoi ( pointsize.mb_str() );
    pointsize.Printf ( _T ( "%d" ), ptsz / 10 );

    SetChoiceOptionsFromFacename ( facename );

    familyChoice->SetStringSelection ( facename );
    weightChoice->SetStringSelection ( weight );
    pointSizeChoice->SetStringSelection ( pointsize );

    m_previewer->SetFont ( dialogFont );
    m_previewer->SetName ( _T( "ABCDEFGabcdefg12345" ) );

//    m_previewer->Refresh();

//    familyChoice->SetStringSelection( wxFontFamilyIntToString(dialogFont.GetFamily()) );
//    styleChoice->SetStringSelection(wxFontStyleIntToString(dialogFont.GetStyle()));
//    weightChoice->SetStringSelection(wxFontWeightIntToString(dialogFont.GetWeight()));

    /*
     if (colourChoice)
     {
     wxString name(wxTheColourDatabase->FindName(m_fontData.GetColour()));
     if (name.length())
     colourChoice->SetStringSelection(name);
     else
     colourChoice->SetStringSelection(wxT("BLACK"));
     }

     if (underLineCheckBox)
     {
     underLineCheckBox->SetValue(dialogFont.GetUnderlined());
     }

     //    pointSizeChoice->SetSelection(dialogFont.GetPointSize()-1);
     pointSizeChoice->SetSelection(0);

     #if !defined(__SMARTPHONE__) && !defined(__POCKETPC__)
     GetSizer()->SetItemMinSize(m_previewer, is_pda ? 100 : 430, is_pda ? 40 : 100);
     GetSizer()->SetSizeHints(this);
     GetSizer()->Fit(this);

     Centre(wxBOTH);
     #endif
     */

    // Don't block events any more
    m_useEvents = true;

}

void X11FontPicker::OnChangeFace ( wxCommandEvent& WXUNUSED ( event ) )
{
    if ( !m_useEvents ) return;

    //    Capture the current selections
    wxString facename = familyChoice->GetStringSelection();
    wxString pointsize = pointSizeChoice->GetStringSelection();
    wxString weight = weightChoice->GetStringSelection();

    SetChoiceOptionsFromFacename ( facename );

    //    Reset the choices
    familyChoice->SetStringSelection ( facename );
    weightChoice->SetStringSelection ( weight );
    pointSizeChoice->SetStringSelection ( pointsize );

    //    And make the font change
    DoFontChange();

}

void X11FontPicker::SetChoiceOptionsFromFacename (const wxString &facename)
{
    //    Get a list of matching fonts
    char face[101];
    strncpy ( face, facename.mb_str(), 100 );
    face[100] = '\0';

    char pattern[100];
    sprintf ( pattern, "-*-%s-*-*-*-*-*-*-*-*-*-*-iso8859-1", face );
//    wxString pattern;
//    pattern.Printf(wxT("-*-%s-*-*-*-*-*-*-*-*-*-*-iso8859-1"), facename.mb_str());

    int nFonts;
    char ** list = XListFonts ( ( Display * ) wxGetDisplay(), pattern, 32767, &nFonts );

    //    First, look thru all the point sizes looking for "0" to indicate scaleable (e.g. TrueType) font
    bool scaleable = false;
    for ( int i=0; i < nFonts; i++ )
    {
        wxStringTokenizer st ( wxString ( list[i] ), _T ( "-" ) );
        st.GetNextToken();
        st.GetNextToken();
        st.GetNextToken();
        st.GetNextToken();
        st.GetNextToken();
        st.GetNextToken();
        st.GetNextToken();
        wxString pointsize = st.GetNextToken();

        if ( pointsize.IsSameAs ( _T ( "0" ) ) )
        {
            scaleable = true;
            break;
        }
    }

    // make different pointsize selections for scaleable fonts
    wxArrayString PointSizeArray;

    if ( scaleable )
    {
        for ( int j=0; j < SCALEABLE_SIZES; j++ )
        PointSizeArray.Add ( scaleable_pointsize[j] );
    }

    else
    {
        //Get the Point Sizes Array
        unsigned int jname;
        for ( int i=0; i < nFonts; i++ )
        {
//                  printf("%s\n", list[i]);
            wxStringTokenizer st ( wxString ( list[i] ), _T ( "-" ) );
            st.GetNextToken();
            st.GetNextToken();
            st.GetNextToken();
            st.GetNextToken();
            st.GetNextToken();
            st.GetNextToken();
            st.GetNextToken();
            wxString pointsize = st.GetNextToken();
            //           printf("%s\n",facename.mb_str());
            for ( jname=0; jname<PointSizeArray.GetCount(); jname++ )
            {
                if ( pointsize == PointSizeArray.Item ( jname ) )
                break;
            }
            if ( jname >= PointSizeArray.GetCount() )
            {
                PointSizeArray.Add ( pointsize );
//                        printf("Added %s\n", pointsize.mb_str());
            }
        }
    }
    pointSizeChoice->Clear();
    pointSizeChoice->Append ( PointSizeArray );
    pointSizeChoice->SetSelection ( 0 );

    //Get the Weight Array
    wxArrayString WeightArray;
    for ( int i=0; i < nFonts; i++ )
    {
//            printf("%s\n", list[i]);
        wxStringTokenizer st ( wxString ( list[i] ), _T ( "-" ) );
        st.GetNextToken();
        st.GetNextToken();
        st.GetNextToken();
        wxString weight = st.GetNextToken();
        //           printf("%s\n",facename.mb_str());
        unsigned int jname;
        for ( jname=0; jname<WeightArray.GetCount(); jname++ )
        {
            if ( weight == WeightArray.Item ( jname ) )
            break;
        }
        if ( jname >= WeightArray.GetCount() )
        {
            WeightArray.Add ( weight );
//                  printf("Added weight %s\n", weight.mb_str());
        }
    }

    weightChoice->Clear();
    weightChoice->Append ( WeightArray );
    weightChoice->SetSelection ( 0 );

}

void X11FontPicker::InitializeFont()
{
    int fontFamily = wxSWISS;
    int fontWeight = wxNORMAL;
    int fontStyle = wxNORMAL;
    int fontSize = 12;
    bool fontUnderline = false;

    wxString fontnative;
    if ( m_fontData.m_initialFont.Ok() )
    {
        fontnative = m_fontData.m_initialFont.GetNativeFontInfoDesc();
        fontFamily = m_fontData.m_initialFont.GetFamily();
        fontWeight = m_fontData.m_initialFont.GetWeight();
        fontStyle = m_fontData.m_initialFont.GetStyle();
        fontSize = m_fontData.m_initialFont.GetPointSize();
        fontUnderline = m_fontData.m_initialFont.GetUnderlined();
    }

//      printf("Init Fetching    %s\n", fontnative.mb_str());

    wxFont tFont = wxFont ( fontSize, fontFamily, fontStyle,
            fontWeight, fontUnderline );

    wxFont *pdialogFont = tFont.New ( fontnative );

    //    Confirm
    /*
     wxNativeFontInfo *i = (wxNativeFontInfo *)pdialogFont->GetNativeFontInfo();

     XFontStruct *xfont = (XFontStruct*) pdialogFont->GetFontStruct( 1.0, (Display *)wxGetDisplay() );
     unsigned long ret;
     XGetFontProperty(xfont, 18, &ret);
     char* x = XGetAtomName((Display *)wxGetDisplay(), ret);
     printf(" Init Got %s\n", x);
     */

    dialogFont = *pdialogFont;

}

void X11FontPicker::OnChangeFont ( wxCommandEvent& WXUNUSED ( event ) )
{
    if ( !m_useEvents ) return;

    DoFontChange();
}

void X11FontPicker::DoFontChange ( void )
{
    wxString facename = familyChoice->GetStringSelection();
    wxString pointsize = pointSizeChoice->GetStringSelection();
    wxString weight = weightChoice->GetStringSelection();

    char font_x[200];
    sprintf ( font_x,"-*-%s-%s-r-normal-*-*-%s0-*-*-*-*-iso8859-1", facename.mb_str(), weight.mb_str(), pointsize.mb_str() );
    wxString font_xlfd ( font_x, wxConvUTF8 );

//                  printf("Fetching    %s\n", font_xlfd.mb_str());

    XFontStruct *test = XLoadQueryFont ( ( Display * ) wxGetDisplay(),font_xlfd.mb_str() );

    //    Confirm
    /*
     unsigned long ret0;
     if(test)
     {
     XGetFontProperty(test, 18, &ret0);
     char* x = XGetAtomName((Display *)wxGetDisplay(), ret0);
     printf("FGot %s\n", x);
     }
     */
    if ( test )
    {
        font_xlfd.Prepend ( "0;" );
        wxFont *ptf = new wxFont;
        pPreviewFont = ptf->New ( font_xlfd );

        /*
         wxNativeFontInfo *i = (wxNativeFontInfo *)pPreviewFont->GetNativeFontInfo();

         XFontStruct *xfont = (XFontStruct*) pPreviewFont->GetFontStruct( 1.0, (Display *)wxGetDisplay() );
         unsigned long ret;
         XGetFontProperty(xfont, 18, &ret);
         char* x = XGetAtomName((Display *)wxGetDisplay(), ret);
         printf("Got %s\n", x);
         */
        m_previewer->SetName ( _T( "ABCDEFGabcdefg12345" ) );
        m_previewer->SetFont ( *pPreviewFont );
        m_previewer->Refresh();
    }

    else
    {
        wxString err ( _T ( "No Font:" ) );
        err.Append ( font_xlfd );
        m_previewer->SetName ( err );
        m_previewer->SetFont ( *pPreviewFont );
        m_previewer->Refresh();
    }

    /*
     int fontFamily = wxFontFamilyStringToInt(WXSTRINGCAST familyChoice->GetStringSelection());
     int fontWeight = wxFontWeightStringToInt(WXSTRINGCAST weightChoice->GetStringSelection());
     int fontStyle = wxFontStyleStringToInt(WXSTRINGCAST styleChoice->GetStringSelection());
     int fontSize = wxAtoi(pointSizeChoice->GetStringSelection());
     // Start with previous underline setting, we want to retain it even if we can't edit it
     // dialogFont is always initialized because of the call to InitializeFont
     int fontUnderline = dialogFont.GetUnderlined();

     if (underLineCheckBox)
     {
     fontUnderline = underLineCheckBox->GetValue();
     }

     dialogFont = wxFont(fontSize, fontFamily, fontStyle, fontWeight, (fontUnderline != 0));
     m_previewer->SetFont(dialogFont);

     if ( colourChoice )
     {
     if ( !colourChoice->GetStringSelection().empty() )
     {
     wxColour col = wxTheColourDatabase->Find(colourChoice->GetStringSelection());
     if (col.Ok())
     {
     m_fontData.m_fontColour = col;
     }
     }
     }
     // Update color here so that we can also use the color originally passed in
     // (EnableEffects may be false)
     if (m_fontData.m_fontColour.Ok())
     m_previewer->SetForegroundColour(m_fontData.m_fontColour);

     m_previewer->Refresh();
     */
}

#endif            //__WXX11__

//---------------------------------------------------------------------------------
//          Vector Stuff for Hit Test Algorithm
//---------------------------------------------------------------------------------
double vGetLengthOfNormal( pVector2D a, pVector2D b, pVector2D n )
{
    vector2D c, vNormal;
    vNormal.x = 0;
    vNormal.y = 0;
    //
    //Obtain projection vector.
    //
    //c = ((a * b)/(|b|^2))*b
    //
    c.x = b->x * ( vDotProduct( a, b ) / vDotProduct( b, b ) );
    c.y = b->y * ( vDotProduct( a, b ) / vDotProduct( b, b ) );
//
    //Obtain perpendicular projection : e = a - c
    //
    vSubtractVectors( a, &c, &vNormal );
    //
    //Fill PROJECTION structure with appropriate values.
    //
    *n = vNormal;

    return ( vVectorMagnitude( &vNormal ) );
}

double vDotProduct( pVector2D v0, pVector2D v1 )
{
    double dotprod;

    dotprod = ( v0 == NULL || v1 == NULL ) ? 0.0 : ( v0->x * v1->x ) + ( v0->y * v1->y );

    return ( dotprod );
}

pVector2D vAddVectors( pVector2D v0, pVector2D v1, pVector2D v )
{
    if( v0 == NULL || v1 == NULL ) v = (pVector2D) NULL;
    else {
        v->x = v0->x + v1->x;
        v->y = v0->y + v1->y;
    }
    return ( v );
}

pVector2D vSubtractVectors( pVector2D v0, pVector2D v1, pVector2D v )
{
    if( v0 == NULL || v1 == NULL ) v = (pVector2D) NULL;
    else {
        v->x = v0->x - v1->x;
        v->y = v0->y - v1->y;
    }
    return ( v );
}

double vVectorSquared( pVector2D v0 )
{
    double dS;

    if( v0 == NULL ) dS = 0.0;
    else
        dS = ( ( v0->x * v0->x ) + ( v0->y * v0->y ) );
    return ( dS );
}

double vVectorMagnitude( pVector2D v0 )
{
    double dMagnitude;

    if( v0 == NULL ) dMagnitude = 0.0;
    else
        dMagnitude = sqrt( vVectorSquared( v0 ) );
    return ( dMagnitude );
}

/**************************************************************************/
/*          LogMessageOnce                                                */
/**************************************************************************/

bool LogMessageOnce(const wxString &msg)
{
    //    Search the array for a match

    for( unsigned int i = 0; i < pMessageOnceArray->GetCount(); i++ ) {
        if( msg.IsSameAs( pMessageOnceArray->Item( i ) ) ) return false;
    }

    // Not found, so add to the array
    pMessageOnceArray->Add( msg );

    //    And print it
    wxLogMessage( msg );
    return true;
}

/**************************************************************************/
/*          Some assorted utilities                                       */
/**************************************************************************/

/**************************************************************************/
/*          Converts the distance to the units selected by user           */
/**************************************************************************/
double toUsrDistance( double nm_distance, int unit  )
{
    double ret;
    if ( unit == -1 )
        unit = g_iDistanceFormat;
    switch( unit ){
        case DISTANCE_NMI: //Nautical miles
            ret = nm_distance;
            break;
        case DISTANCE_MI: //Statute miles
            ret = nm_distance * 1.15078;
            break;
        case DISTANCE_KM:
            ret = nm_distance * 1.852;
            break;
        case DISTANCE_M:
            ret = nm_distance * 1852;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the distance from the units selected by user to NMi  */
/**************************************************************************/
double fromUsrDistance( double usr_distance, int unit )
{
    double ret;
    if ( unit == -1 )
        unit = g_iDistanceFormat;
    switch( unit ){
        case DISTANCE_NMI: //Nautical miles
            ret = usr_distance;
            break;
        case DISTANCE_MI: //Statute miles
            ret = usr_distance / 1.15078;
            break;
        case DISTANCE_KM:
            ret = usr_distance / 1.852;
            break;
        case DISTANCE_M:
            ret = usr_distance / 1852;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Returns the abbreviation of user selected distance unit       */
/**************************************************************************/
wxString getUsrDistanceUnit( int unit )
{
    wxString ret;
    if ( unit == -1 )
        unit = g_iDistanceFormat;
    switch( unit ){
        case DISTANCE_NMI: //Nautical miles
            ret = _("NMi");
            break;
        case DISTANCE_MI: //Statute miles
            ret = _("mi");
            break;
        case DISTANCE_KM:
            ret = _("km");
            break;
        case DISTANCE_M:
            ret = _("m");
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the speed to the units selected by user              */
/**************************************************************************/
double toUsrSpeed( double kts_speed, int unit )
{
    double ret;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit )
    {
        case SPEED_KTS: //kts
            ret = kts_speed;
            break;
        case SPEED_MPH: //mph
            ret = kts_speed * 1.15078;
            break;
        case SPEED_KMH: //km/h
            ret = kts_speed * 1.852;
            break;
        case SPEED_MS: //m/s
            ret = kts_speed * 0.514444444;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the speed from the units selected by user to knots   */
/**************************************************************************/
double fromUsrSpeed( double usr_speed, int unit )
{
    double ret;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit )
    {
        case SPEED_KTS: //kts
            ret = usr_speed;
            break;
        case SPEED_MPH: //mph
            ret = usr_speed / 1.15078;
            break;
        case SPEED_KMH: //km/h
            ret = usr_speed / 1.852;
            break;
        case SPEED_MS: //m/s
            ret = usr_speed / 0.514444444;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Returns the abbreviation of user selected speed unit          */
/**************************************************************************/
wxString getUsrSpeedUnit( int unit )
{
    wxString ret;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit ){
        case SPEED_KTS: //kts
            ret = _("kts");
            break;
        case SPEED_MPH: //mph
            ret = _("mph");
            break;
        case SPEED_KMH:
            ret = _("km/h");
            break;
        case SPEED_MS:
            ret = _("m/s");
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Formats the coordinates to string                             */
/**************************************************************************/
wxString toSDMM( int NEflag, double a, bool hi_precision )
{
    wxString s;
    double mpy;
    short neg = 0;
    int d;
    long m;
    double ang = a;
    char c = 'N';

    if( a < 0.0 ) {
        a = -a;
        neg = 1;
    }
    d = (int) a;
    if( neg ) d = -d;
    if( NEflag ) {
        if( NEflag == 1 ) {
            c = 'N';

            if( neg ) {
                d = -d;
                c = 'S';
            }
        } else
            if( NEflag == 2 ) {
                c = 'E';

                if( neg ) {
                    d = -d;
                    c = 'W';
                }
            }
    }

    switch( g_iSDMMFormat ){
        case 0:
            mpy = 600.0;
            if( hi_precision ) mpy = mpy * 1000;

            m = (long) wxRound( ( a - (double) d ) * mpy );

            if( !NEflag || NEflag < 1 || NEflag > 2 ) //Does it EVER happen?
                    {
                if( hi_precision ) s.Printf( _T ( "%d %02ld.%04ld'" ), d, m / 10000, m % 10000 );
                else
                    s.Printf( _T ( "%d %02ld.%01ld'" ), d, m / 10, m % 10 );
            } else {
                if( hi_precision )
                    if (NEflag == 1)
                        s.Printf( _T ( "%02d %02ld.%04ld %c" ), d, m / 10000, ( m % 10000 ), c );
                    else
                        s.Printf( _T ( "%03d %02ld.%04ld %c" ), d, m / 10000, ( m % 10000 ), c );
                else
                    if (NEflag == 1)
                        s.Printf( _T ( "%02d %02ld.%01ld %c" ), d, m / 10, ( m % 10 ), c );
                    else
                        s.Printf( _T ( "%03d %02ld.%01ld %c" ), d, m / 10, ( m % 10 ), c );
            }
            break;
        case 1:
            if( hi_precision ) s.Printf( _T ( "%03.6f" ), ang ); //cca 11 cm - the GPX precision is higher, but as we use hi_precision almost everywhere it would be a little too much....
            else
                s.Printf( _T ( "%03.4f" ), ang ); //cca 11m
            break;
        case 2:
            m = (long) ( ( a - (double) d ) * 60 );
            mpy = 10.0;
            if( hi_precision ) mpy = mpy * 100;
            long sec = (long) ( ( a - (double) d - ( ( (double) m ) / 60 ) ) * 3600 * mpy );

            if( !NEflag || NEflag < 1 || NEflag > 2 ) //Does it EVER happen?
                    {
                if( hi_precision ) s.Printf( _T ( "%d %ld'%ld.%ld\"" ), d, m, sec / 1000,
                        sec % 1000 );
                else
                    s.Printf( _T ( "%d %ld'%ld.%ld\"" ), d, m, sec / 10, sec % 10 );
            } else {
                if( hi_precision )
                    if (NEflag == 1)
                        s.Printf( _T ( "%02d %02ld %02ld.%03ld %c" ), d, m, sec / 1000, sec % 1000, c );
                    else
                        s.Printf( _T ( "%03d %02ld %02ld.%03ld %c" ), d, m, sec / 1000, sec % 1000, c );
                else
                    if (NEflag == 1)
                        s.Printf( _T ( "%02d %02ld %02ld.%ld %c" ), d, m, sec / 10, sec % 10, c );
                    else
                        s.Printf( _T ( "%03d %02ld %02ld.%ld %c" ), d, m, sec / 10, sec % 10, c );
            }
            break;
    }
    return s;
}

/****************************************************************************/
// Modified from the code posted by Andy Ross at
//     http://www.mail-archive.com/flightgear-devel@flightgear.org/msg06702.html
// Basically, it looks for a list of decimal numbers embedded in the
// string and uses the first three as degree, minutes and seconds.  The
// presence of a "S" or "W character indicates that the result is in a
// hemisphere where the final answer must be negated.  Non-number
// characters are treated as whitespace separating numbers.
//
// So there are lots of bogus strings you can feed it to get a bogus
// answer, but that's not surprising.  It does, however, correctly parse
// all the well-formed strings I can thing of to feed it.  I've tried all
// the following:
//
// 37°54.204' N
// N37 54 12
// 37°54'12"
// 37.9034
// 122°18.621' W
// 122w 18 37
// -122.31035
/****************************************************************************/
double fromDMM( wxString sdms )
{
    wchar_t buf[64];
    char narrowbuf[64];
    int i, len, top = 0;
    double stk[32], sign = 1;

    //First round of string modifications to accomodate some known strange formats
    wxString replhelper;
    replhelper = wxString::FromUTF8( "´·" ); //UKHO PDFs
    sdms.Replace( replhelper, _T(".") );
    replhelper = wxString::FromUTF8( "\"·" ); //Don't know if used, but to make sure
    sdms.Replace( replhelper, _T(".") );
    replhelper = wxString::FromUTF8( "·" );
    sdms.Replace( replhelper, _T(".") );

    replhelper = wxString::FromUTF8( "s. š." ); //Another example: cs.wikipedia.org (someone was too active translating...)
    sdms.Replace( replhelper, _T("N") );
    replhelper = wxString::FromUTF8( "j. š." );
    sdms.Replace( replhelper, _T("S") );
    sdms.Replace( _T("v. d."), _T("E") );
    sdms.Replace( _T("z. d."), _T("W") );

    //If the string contains hemisphere specified by a letter, then '-' is for sure a separator...
    sdms.UpperCase();
    if( sdms.Contains( _T("N") ) || sdms.Contains( _T("S") ) || sdms.Contains( _T("E") )
            || sdms.Contains( _T("W") ) ) sdms.Replace( _T("-"), _T(" ") );

    wcsncpy( buf, sdms.wc_str( wxConvUTF8 ), 64 );
    len = wcslen( buf );

    for( i = 0; i < len; i++ ) {
        wchar_t c = buf[i];
        if( ( c >= '0' && c <= '9' ) || c == '-' || c == '.' || c == '+' ) {
            narrowbuf[i] = c;
            continue; /* Digit characters are cool as is */
        }
        if( c == ',' ) {
            narrowbuf[i] = '.'; /* convert to decimal dot */
            continue;
        }
        if( ( c | 32 ) == 'w' || ( c | 32 ) == 's' ) sign = -1; /* These mean "negate" (note case insensitivity) */
        narrowbuf[i] = 0; /* Replace everything else with nuls */
    }

    /* Build a stack of doubles */
    stk[0] = stk[1] = stk[2] = 0;
    for( i = 0; i < len; i++ ) {
        while( i < len && narrowbuf[i] == 0 )
            i++;
        if( i != len ) {
            stk[top++] = atof( narrowbuf + i );
            i += strlen( narrowbuf + i );
        }
    }

    return sign * ( stk[0] + ( stk[1] + stk[2] / 60 ) / 60 );
}

/* render a rectangle at a given color and transparency */
void AlphaBlending( ocpnDC &dc, int x, int y, int size_x, int size_y, float radius, wxColour color,
        unsigned char transparency )
{
    wxDC *pdc = dc.GetDC();
    if( pdc ) {
        //    Get wxImage of area of interest
        wxBitmap obm( size_x, size_y );
        wxMemoryDC mdc1;
        mdc1.SelectObject( obm );
        mdc1.Blit( 0, 0, size_x, size_y, pdc, x, y );
        mdc1.SelectObject( wxNullBitmap );
        wxImage oim = obm.ConvertToImage();

        //    Create destination image
        wxBitmap olbm( size_x, size_y );
        wxMemoryDC oldc( olbm );
        oldc.SetBackground( *wxBLACK_BRUSH );
        oldc.SetBrush( *wxWHITE_BRUSH );
        oldc.Clear();

        if( radius > 0.0 )
            oldc.DrawRoundedRectangle( 0, 0, size_x, size_y, radius );

        wxImage dest = olbm.ConvertToImage();
        unsigned char *dest_data = (unsigned char *) malloc(
                size_x * size_y * 3 * sizeof(unsigned char) );
        unsigned char *bg = oim.GetData();
        unsigned char *box = dest.GetData();
        unsigned char *d = dest_data;

        float alpha = 1.0 - (float)transparency / 255.0;
        int sb = size_x * size_y;
        for( int i = 0; i < sb; i++ ) {
            float a = alpha;
            if( *box == 0 && radius > 0.0 ) a = 1.0;
            int r = ( ( *bg++ ) * a ) + (1.0-a) * color.Red();
            *d++ = r; box++;
            int g = ( ( *bg++ ) * a ) + (1.0-a) * color.Green();
            *d++ = g; box++;
            int b = ( ( *bg++ ) * a ) + (1.0-a) * color.Blue();
            *d++ = b; box++;
        }

        dest.SetData( dest_data );

        //    Convert destination to bitmap and draw it
        wxBitmap dbm( dest );
        dc.DrawBitmap( dbm, x, y, false );

        // on MSW, the dc Bounding box is not updated on DrawBitmap() method.
        // Do it explicitely here for all platforms.
        dc.CalcBoundingBox( x, y );
        dc.CalcBoundingBox( x + size_x, y + size_y );
    } else {
        /* opengl version */
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glColor4ub( color.Red(), color.Green(), color.Blue(), transparency );

        glBegin( GL_QUADS );
        glVertex2i( x, y );
        glVertex2i( x + size_x, y );
        glVertex2i( x + size_x, y + size_y );
        glVertex2i( x, y + size_y );
        glEnd();

        glDisable( GL_BLEND );
    }
}

