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
//#include <math.h>
#include <time.h>
#include <locale>
#include <list>

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
#include "AIS_Decoder.h"

#ifdef USE_S57
#include "s52plib.h"
#include "cm93.h"
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

//    Statics

extern ChartCanvas      *cc1;
extern MyFrame          *gFrame;

extern double           g_ChartNotRenderScaleFactor;
extern int              g_restore_stackindex;
extern int              g_restore_dbindex;
extern RouteList        *pRouteList;
extern LayerList        *pLayerList;
extern int              g_LayerIdx;
extern Select           *pSelect;
extern MyConfig         *pConfig;
extern ArrayOfCDI       g_ChartDirArray;
extern double           vLat, vLon, gLat, gLon;
extern double           kLat, kLon;
extern double           initial_scale_ppm, initial_rotation;
extern ColorScheme      global_color_scheme;
extern int              g_nbrightness;
extern bool             g_bShowMag;
extern double           g_UserVar;

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
extern bool             g_bdisable_opengl;
extern bool             g_bShowFPS;
extern bool             g_bsmoothpanzoom;
extern bool             g_fog_overzoom;
extern double           g_overzoom_emphasis_base;
extern bool             g_oz_vector_scale;

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
extern bool             g_bWplIsAprsPosition;

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
extern double           g_ownship_HDTpredictor_miles;

#ifdef USE_S57
extern s52plib          *ps52plib;
#endif

extern int              g_cm93_zoom_factor;
extern bool             g_b_legacy_input_filter_behaviour;
extern bool             g_bShowCM93DetailSlider;
extern int              g_cm93detail_dialog_x, g_cm93detail_dialog_y;

extern bool             g_bUseGreenShip;

extern bool             g_b_overzoom_x;                      // Allow high overzoom
extern int              g_nautosave_interval_seconds;
extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern int              g_n_ownship_min_mm;
extern double           g_n_arrival_circle_radius;

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
extern bool             g_bMagneticAPB;
extern bool             g_bShowChartBar;

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
extern int              g_cog_predictor_width;
extern int              g_ais_cog_predictor_width;

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

extern bool             g_btouch;
extern bool             g_bresponsive;

extern bool             bGPSValid;              // for track recording
extern bool             g_bexpert;

extern int              g_SENC_LOD_pixels;
extern ArrayOfMMSIProperties   g_MMSI_Props_Array;

extern int              g_chart_zoom_modifier;

extern int              g_NMEAAPBPrecision;
extern int              g_NMEAAPBXTEPrecision;

extern bool             g_bSailing;
extern double           g_display_size_mm;
extern double           g_config_display_size_mm;

#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

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
    m_CurrentTrackSeg = 0;
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
    if(m_bRunning){
        if(do_add_point)
            AddPointNow( true );                   // Force add last point
        else{    
            double delta = 0.0;
            if( m_lastStoredTP )
                delta = DistGreatCircle( gLat, gLon, m_lastStoredTP->m_lat, m_lastStoredTP->m_lon );

            if(  delta > m_minTrackpoint_delta ) 
                AddPointNow( true );                   // Add last point
        }
    }

    m_TimerTrack.Stop();
    m_bRunning = false;
    m_track_run = 0;
}

Track *Track::DoExtendDaily()
{
    Route *pExtendRoute = NULL;
    RoutePoint *pExtendPoint = NULL;

    RoutePoint *pLastPoint = this->GetPoint( 1 );

    wxRouteListNode *route_node = pRouteList->GetFirst();
    while( route_node ) {
        Route *proute = route_node->GetData();

        if( !proute->m_bIsInLayer && proute->m_bIsTrack && proute->m_GUID != this->m_GUID ) {
            RoutePoint *track_node = proute->GetLastPoint();
            if( track_node->GetCreateTime() <= pLastPoint->GetCreateTime() ) {
                if( !pExtendPoint  || track_node->GetCreateTime() > pExtendPoint->GetCreateTime() ) {
                    pExtendPoint = track_node;
                    pExtendRoute = proute;
                }
            }
        }
        route_node = route_node->GetNext();                         // next route
    }
    if( pExtendRoute
            && pExtendRoute->GetPoint( 1 )->GetCreateTime().FromTimezone( wxDateTime::GMT0 ).IsSameDate(
                    pLastPoint->GetCreateTime().FromTimezone( wxDateTime::GMT0 ) ) ) {
        int begin = 1;
        if( pLastPoint->GetCreateTime() == pExtendPoint->GetCreateTime() ) begin = 2;
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
        return (Track *)pExtendRoute;
    } else {
        if( this->m_RouteNameString.IsNull() )
            this->m_RouteNameString = wxDateTime::Today().FormatISODate();
        return NULL;
    }
}

void Track::AdjustCurrentTrackPoint( RoutePoint *prototype )
{
    if(prototype) {
        CloneAddedTrackPoint( m_lastStoredTP, prototype );
        m_prev_time = prototype->GetCreateTime().FromUTC();
    }
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
            wxRoutePointListNode *node = pRoutePointList->GetFirst();
            if(node)
                node->GetData()->SetCreateTime(now.ToUTC());
        }

    m_TimerTrack.Start( 1000, wxTIMER_CONTINUOUS );
}

RoutePoint* Track::AddNewPoint( vector2D point, wxDateTime time ) {
    RoutePoint *rPoint = new RoutePoint( point.lat, point.lon, wxString( _T ( "empty" ) ),
            wxString( _T ( "" ) ), GPX_EMPTY_STRING );
    rPoint->m_bShowName = false;
    rPoint->m_bIsVisible = true;
    rPoint->m_GPXTrkSegNo = 1;
    rPoint->SetCreateTime(time);
    AddPoint( rPoint );

    pConfig->AddNewTrackPoint( rPoint, m_GUID );        // This will update the "changes" file only

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
    if( m_bRunning || prp->GetIconName().StartsWith( _T("xmred") ) ) {
            basic_colour = GetGlobalColor( _T ( "URED" ) );
    } else
        if( prp->GetIconName().StartsWith( _T("xmblue") ) ) {
                basic_colour = GetGlobalColor( _T ( "BLUE3" ) );
        } else
            if( prp->GetIconName().StartsWith( _T("xmgreen") ) ) {
                    basic_colour = GetGlobalColor( _T ( "UGREN" ) );
            } else {
                    basic_colour = GetGlobalColor( _T ( "CHMGD" ) );
            }

    int style = wxSOLID;
    int width = g_track_line_width;
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

    std::list< std::list<wxPoint> > pointlists;
    std::list<wxPoint> pointlist;

    //    Get the dc boundary
    int sx, sy;
    dc.GetSize( &sx, &sy );

    //  Draw the first point
    wxPoint rpt;
    DrawPointWhich( dc, 1, &rpt );
    pointlist.push_back(rpt);

    node = node->GetNext();
    while( node ) {
        RoutePoint *prp = node->GetData();
        unsigned short int ToSegNo = prp->m_GPXTrkSegNo;

        wxPoint r;
        cc1->GetCanvasPointPix( prp->m_lat, prp->m_lon, &r );

        //  We do inline decomposition of the line segments, in a simple minded way
        //  If the line segment length is less than approximately 2 pixels, then simply don't render it,
        //  but continue on to the next point.

        int x0 = r.x, y0 = r.y, x1 = rpt.x, y1= rpt.y;
        if((abs(r.x - rpt.x) > 1) || (abs(r.y - rpt.y) > 1) ||
            cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, 0, sx, 0, sy ) != Visible ) {
            prp->Draw( dc, &rpt );

            if( ToSegNo != FromSegNo ) {
                if(pointlist.size()) {
                    pointlists.push_back(pointlist);
                    pointlist.clear();
                }
            }
            pointlist.push_back(rpt);
        }

        node = node->GetNext();
        FromSegNo = ToSegNo;
    }

    //    Add last segment, dynamically, maybe.....
    if( m_bRunning ) {
        cc1->GetCanvasPointPix( gLat, gLon, &rpt );
        pointlist.push_back(rpt);
    }
    pointlists.push_back(pointlist);

    for(std::list< std::list<wxPoint> >::iterator lines = pointlists.begin();
        lines != pointlists.end(); lines++) {
        wxPoint *points = new wxPoint[lines->size()];
        int i = 0;
        for(std::list<wxPoint>::iterator line = lines->begin();
            line != lines->end(); line++) {
            points[i] = *line;
            i++;
        }

        int hilite_width = radius;
        if( hilite_width ) {
            wxPen psave = dc.GetPen();

            dc.StrokeLines( i, points );

            wxColour y = GetGlobalColor( _T ( "YELO1" ) );
            wxColour hilt( y.Red(), y.Green(), y.Blue(), 128 );

            wxPen HiPen( hilt, hilite_width, wxSOLID );
            dc.SetPen( HiPen );

            dc.StrokeLines( i, points );

            dc.SetPen( psave );
        } else
            dc.StrokeLines( i, points );

        delete [] points;
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
    double delta_dist = 0.;
    double delta_hdg, xte;
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
    m_pNavObjectChangesSet = NULL;

    m_bSkipChangeSetUpdate = false;

    g_pConnectionParams = new wxArrayOfConnPrm();
}

void MyConfig::CreateRotatingNavObjBackup()
{
    //Rotate navobj backups, but just in case there are some changes in the current version
    //to prevent the user trying to "fix" the problem by continuously starting the
    //application to overwrite all of his good backups...
    if( g_navobjbackups > 0 ) {
        wxFile f;
        wxString oldname = m_sNavObjSetFile;
        wxString newname = wxString::Format( _T("%s.1"), m_sNavObjSetFile.c_str() );

        wxFileOffset s_diff = 1;
        if( ::wxFileExists( newname ) ) {

            if( f.Open(oldname) ){
                s_diff = f.Length();
                f.Close();
            }

            if( f.Open(newname) ){
                s_diff -= f.Length();
                f.Close();
            }
        }


        if ( s_diff != 0 )
        {
            for( int i = g_navobjbackups - 1; i >= 1; i-- )
            {
                oldname = wxString::Format( _T("%s.%d"), m_sNavObjSetFile.c_str(), i );
                newname = wxString::Format( _T("%s.%d"), m_sNavObjSetFile.c_str(), i + 1 );
                if( wxFile::Exists( oldname ) )
                    wxCopyFile( oldname, newname );
            }

            if( wxFile::Exists( m_sNavObjSetFile ) )
            {
                newname = wxString::Format( _T("%s.1"), m_sNavObjSetFile.c_str() );
                wxCopyFile( m_sNavObjSetFile, newname );
            }
        }
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
    g_b_overzoom_x = true;
    Read( _T ( "AutosaveIntervalSeconds" ), &g_nautosave_interval_seconds, 300 );

    Read( _T ( "GPSIdent" ), &g_GPS_Ident, wxT("Generic") );
    Read( _T ( "UseGarminHostUpload" ),  &g_bGarminHostUpload, 0 );

    Read( _T ( "UseNMEA_RMC" ), &g_bUseRMC, 1 );
    Read( _T ( "UseNMEA_GLL" ), &g_bUseGLL, 1 );
    Read( _T ( "UseBigRedX" ), &g_bbigred, 0 );

    if(iteration == 0){
        int size_mm;
        Read( _T ( "DisplaySizeMM" ), &size_mm, -1 );
        g_config_display_size_mm = size_mm;
        if((size_mm > 100) && (size_mm < 2000)){
            g_display_size_mm = size_mm;
        }
    }
    
    Read( _T ( "FilterNMEA_Avg" ), &g_bfilter_cogsog, 0 );
    Read( _T ( "FilterNMEA_Sec" ), &g_COGFilterSec, 1 );
    g_COGFilterSec = wxMin(g_COGFilterSec, MAX_COGSOG_FILTER_SECONDS);
    g_COGFilterSec = wxMax(g_COGFilterSec, 1);
    g_SOGFilterSec = g_COGFilterSec;

    Read( _T ( "ShowMag" ), &g_bShowMag, 0 );
    g_UserVar = 0.0;
    wxString umv;
    Read( _T ( "UserMagVariation" ), &umv );
    if(umv.Len())
        umv.ToDouble( &g_UserVar );

    Read( _T ( "UseMagAPB" ), &g_bMagneticAPB, 0 );

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
    if ( g_bdisable_opengl )
        g_bopengl = false;
    
    Read( _T ( "ShowFPS" ), &g_bShowFPS, 0 );
    
    Read( _T ( "ActiveChartGroup" ), &g_GroupIndex, 0 );

    Read( _T( "NMEAAPBPrecision" ), &g_NMEAAPBPrecision, 3 );
    Read( _T( "NMEAAPBXTEPrecision" ), &g_NMEAAPBXTEPrecision, 2 );

    /* opengl options */
#ifdef ocpnUSE_GL
    Read( _T ( "OpenGLExpert" ), &g_bexpert, false );
    Read( _T ( "UseAcceleratedPanning" ), &g_GLOptions.m_bUseAcceleratedPanning, true );

    Read( _T ( "GPUTextureCompression" ), &g_GLOptions.m_bTextureCompression, 0);
    Read( _T ( "GPUTextureCompressionCaching" ), &g_GLOptions.m_bTextureCompressionCaching, 0);

    Read( _T ( "GPUTextureDimension" ), &g_GLOptions.m_iTextureDimension, 512 );
    Read( _T ( "GPUTextureMemSize" ), &g_GLOptions.m_iTextureMemorySize, 128 );
    if(!g_bexpert){
        g_GLOptions.m_iTextureMemorySize = wxMax(128, g_GLOptions.m_iTextureMemorySize);
        g_GLOptions.m_bTextureCompressionCaching = g_GLOptions.m_bTextureCompression;
    }

#endif
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

    Read( _T ( "MobileTouch" ), &g_btouch, 0 );
    Read( _T ( "ResponsiveGraphics" ), &g_bresponsive, 0 );

    Read( _T ( "ZoomDetailFactor" ), &g_chart_zoom_modifier, 0 );
    g_chart_zoom_modifier = wxMin(g_chart_zoom_modifier,5);
    g_chart_zoom_modifier = wxMax(g_chart_zoom_modifier,-5);

    Read( _T ( "FogOnOverzoom" ), &g_fog_overzoom, 1 );
    Read( _T ( "OverzoomVectorScale" ), &g_oz_vector_scale, 1 );
    Read( _T ( "OverzoomEmphasisBase" ), &g_overzoom_emphasis_base, 10.0 );
    
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

    Read( _T ( "SENC_LOD_Pixels" ), &g_SENC_LOD_pixels, 2 );

#endif

    Read( _T ( "SkewCompUpdatePeriod" ), &g_SkewCompUpdatePeriod, 10 );

    Read( _T ( "SetSystemTime" ), &s_bSetSystemTime, 0 );
    Read( _T ( "ShowStatusBar" ), &m_bShowStatusBar, 1 );
#ifndef __WXOSX__
    Read( _T ( "ShowMenuBar" ), &m_bShowMenuBar, 0 );
#endif
    Read( _T ( "ShowCompassWindow" ), &m_bShowCompassWin, 1 );
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
    Read( _T ( "ShowChartBar" ), &g_bShowChartBar, 1 );
    
    Read( _T ( "SDMMFormat" ), &g_iSDMMFormat, 0 ); //0 = "Degrees, Decimal minutes"), 1 = "Decimal degrees", 2 = "Degrees,Minutes, Seconds"
    Read( _T ( "DistanceFormat" ), &g_iDistanceFormat, 0 ); //0 = "Nautical miles"), 1 = "Statute miles", 2 = "Kilometers", 3 = "Meters"
    Read( _T ( "SpeedFormat" ), &g_iSpeedFormat, 0 ); //0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"

    Read( _T ( "OwnshipCOGPredictorMinutes" ), &g_ownship_predictor_minutes, 5 );
    Read( _T ( "OwnshipCOGPredictorWidth" ), &g_cog_predictor_width, 3 );
    Read( _T ( "OwnshipHDTPredictorMiles" ), &g_ownship_HDTpredictor_miles, 1 );

    Read( _T ( "OwnShipIconType" ), &g_OwnShipIconType, 0 );
    Read( _T ( "OwnShipLength" ), &g_n_ownship_length_meters, 0 );
    Read( _T ( "OwnShipWidth" ), &g_n_ownship_beam_meters, 0 );
    Read( _T ( "OwnShipGPSOffsetX" ), &g_n_gps_antenna_offset_x, 0 );
    Read( _T ( "OwnShipGPSOffsetY" ), &g_n_gps_antenna_offset_y, 0 );
    Read( _T ( "OwnShipMinSize" ), &g_n_ownship_min_mm, 1 );
    g_n_ownship_min_mm = wxMax(g_n_ownship_min_mm, 1);

    g_n_arrival_circle_radius = .050;           // default
    wxString racr;
    Read( _T ( "RouteArrivalCircleRadius" ), &racr );
    if(racr.Len())
        racr.ToDouble( &g_n_arrival_circle_radius);
    g_n_arrival_circle_radius = wxMax(g_n_arrival_circle_radius, .001);

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

    // Boolean to cater for legacy Input COM Port filer behaviour, i.e. show msg filtered but put msg on bus.
    Read( _T ( "LegacyInputCOMPortFilterBehaviour" ), &g_b_legacy_input_filter_behaviour, 0 );
    
    // Boolean to cater for sailing when not approaching waypoint
    Read( _T( "Sailing" ), &g_bSailing, 0);

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
    Read( _T ( "bWplIsAprsPositionReport" ), &g_bWplIsAprsPosition, 1 );
    Read( _T ( "AISCOGPredictorWidth" ), &g_ais_cog_predictor_width, 3 );

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
        ps52plib->SetDisplayCategory((enum _DisCat) read_int );

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

        Read( _T ( "bShowNationalText" ), &read_int, 0 );
        ps52plib->m_bShowNationalTexts = !( read_int == 0 );

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


    if( iteration == 0){
        SetPath( _T ( "/Settings/NMEADataSource" ) );

        wxString connectionconfigs;
        Read ( _T( "DataConnections" ),  &connectionconfigs, wxEmptyString );
        wxArrayString confs = wxStringTokenize(connectionconfigs, _T("|"));
        g_pConnectionParams->Clear();
        for (size_t i = 0; i < confs.Count(); i++)
        {
            ConnectionParams * prm = new ConnectionParams(confs[i]);
            if (!prm->Valid) {
                wxLogMessage( _T( "Skipped invalid DataStream config") );
                delete prm;
                continue;
            }
            g_pConnectionParams->Add(prm);
        }
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
                prm->IOSelect = DS_TYPE_INPUT_OUTPUT;

                g_pConnectionParams->Add(prm);
            }
            else {                                  // port was found, so make sure it is set for output
                cp->IOSelect = DS_TYPE_INPUT_OUTPUT;
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
    initial_rotation = 0;

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

    if( Read( wxString( _T ( "VPRotation" ) ), &st ) ) {
        sscanf( st.mb_str( wxConvUTF8 ), "%lf", &st_rotation );
//    Sanity check the rotation
        st_rotation = fmin ( st_rotation, 360 );
        st_rotation = fmax ( st_rotation, 0 );
        initial_rotation = st_rotation * PI / 180.;
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

            if( pval->IsEmpty() || pval->StartsWith(_T(":")) ) {
                deleteList.Add( str );
            }
            else
                FontMgr::Get().LoadFontNative( &str, pval );

            bCont = GetNextEntry( str, dummy );
        }

        for( unsigned int i=0; i<deleteList.Count(); i++ ) {
            DeleteEntry( deleteList[i] );
        }
        deleteList.Clear();
        delete pval;
    }

    if( 1 == iteration )
        FontMgr::Get().ScrubList();

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


    //    Layers
    if( 0 == iteration )
        pLayerList = new LayerList;

    //  Routes
    if( 0 == iteration )
        pRouteList = new RouteList;

    //    Groups
    if( 0 == iteration )
        LoadConfigGroups( g_pGroupArray );


//      next thing to do is read tracks, etc from the NavObject XML file,
    if( 0 == iteration ) {
        wxLogMessage( _T("Loading navobjects from navobj.xml") );
        CreateRotatingNavObjBackup();

        if( NULL == m_pNavObjectInputSet )
            m_pNavObjectInputSet = new NavObjectCollection1();

        if( ::wxFileExists( m_sNavObjSetFile ) ) {
            if( m_pNavObjectInputSet->load_file( m_sNavObjSetFile.fn_str() ) )
                m_pNavObjectInputSet->LoadAllGPXObjects();
        }
        wxLogMessage( _T("Done loading navobjects") );
        delete m_pNavObjectInputSet;


        if( ::wxFileExists( m_sNavObjSetChangesFile ) ) {

            wxULongLong size = wxFileName::GetSize(m_sNavObjSetChangesFile);

            //We crashed last time :(
            //That's why this file still exists...
            //Let's reconstruct the unsaved changes
            NavObjectChanges *pNavObjectChangesSet = new NavObjectChanges();
            pNavObjectChangesSet->load_file( m_sNavObjSetChangesFile.fn_str() );

            //  Remove the file before applying the changes,
            //  just in case the changes file itself causes a fault.
            //  If it does fault, at least the next restart will proceed without fault.
           if( ::wxFileExists( m_sNavObjSetChangesFile ) )
                ::wxRemoveFile( m_sNavObjSetChangesFile );

           if(size != 0){
                wxLogMessage( _T("Applying NavObjChanges") );
                pNavObjectChangesSet->ApplyChanges();
                UpdateNavObj();
           }

           delete pNavObjectChangesSet;
           
        }

        m_pNavObjectChangesSet = new NavObjectChanges(m_sNavObjSetChangesFile);

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
    Read( _T ( "TrackLineWidth" ), &g_track_line_width, 2 );
    Read( _T ( "CurrentArrowScale" ), &g_current_arrow_scale, 100 );
    Read( _T ( "DefaultWPIcon" ), &g_default_wp_icon, _T("triangle") );

    if(0 == iteration){
        SetPath( _T ( "/MMSIProperties" ) );
        int iPMax = GetNumberOfEntries();
        if( iPMax ) {
            g_MMSI_Props_Array.Empty();
            wxString str, val;
            long dummy;
            int iDir = 0;
            bool bCont = pConfig->GetFirstEntry( str, dummy );
            while( bCont ) {
                pConfig->Read( str, &val );              // Get an entry

                MMSIProperties *pProps = new MMSIProperties( val );
                g_MMSI_Props_Array.Add(pProps);

                bCont = pConfig->GetNextEntry( str, dummy );

            }
        }
    }


    return ( 0 );
}

bool MyConfig::LoadLayers(wxString &path)
{
    wxArrayString file_array;
    wxDir dir;
    Layer *l;
    dir.Open( path );
    if( dir.IsOpened() ) {
        wxString filename;
        bool cont = dir.GetFirst( &filename );
        while( cont ) {
            file_array.Clear();
            filename.Prepend( wxFileName::GetPathSeparator() );
            filename.Prepend( path );
            wxFileName f( filename );
            size_t nfiles = 0;
            if( f.GetExt().IsSameAs( wxT("gpx") ) )
                file_array.Add( filename); // single-gpx-file layer
            else{
                if(wxDir::Exists( filename ) ){
                    wxDir dir( filename );
                    if( dir.IsOpened() ){
                        nfiles = dir.GetAllFiles( filename, &file_array, wxT("*.gpx") );      // layers subdirectory set
                    }
                }
            }

            if( file_array.GetCount() ){
                l = new Layer();
                l->m_LayerID = ++g_LayerIdx;
                l->m_LayerFileName = file_array[0];
                if( file_array.GetCount() <= 1 )
                    wxFileName::SplitPath( file_array[0], NULL, NULL, &( l->m_LayerName ), NULL, NULL );
                else
                    wxFileName::SplitPath( filename, NULL, NULL, &( l->m_LayerName ), NULL, NULL );

                bool bLayerViz = g_bShowLayers;

                if( g_VisibleLayers.Contains( l->m_LayerName ) )
                    bLayerViz = true;
                if( g_InvisibleLayers.Contains( l->m_LayerName ) )
                    bLayerViz = false;

                l->m_bIsVisibleOnChart = bLayerViz;

                wxString laymsg;
                laymsg.Printf( wxT("New layer %d: %s"), l->m_LayerID, l->m_LayerName.c_str() );
                wxLogMessage( laymsg );

                pLayerList->Insert( l );

                //  Load the entire file array as a single layer

                for( unsigned int i = 0; i < file_array.GetCount(); i++ ) {
                    wxString file_path = file_array[i];

                    if( ::wxFileExists( file_path ) ) {
                        NavObjectCollection1 *pSet = new NavObjectCollection1;
                        pSet->load_file(file_path.fn_str());
                        long nItems = pSet->LoadAllGPXObjectsAsLayer(l->m_LayerID, bLayerViz);
                        l->m_NoOfItems += nItems;

                        wxString objmsg;
                        objmsg.Printf( wxT("Loaded GPX file %s with %ld items."), file_path.c_str(), nItems );
                        wxLogMessage( objmsg );

                        delete pSet;
                    }
                }
            }

            cont = dir.GetNext( &filename );
        }
    }

    return true;
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
    if( pr->m_bIsInLayer )
        return true;


    if( !m_bSkipChangeSetUpdate ) {
        if( pr->m_bIsTrack )
            m_pNavObjectChangesSet->AddTrack( (Track *)pr, "add" );
        else
            m_pNavObjectChangesSet->AddRoute( pr, "add" );
    }

    return true;
}

bool MyConfig::UpdateRoute( Route *pr )
{
    if( pr->m_bIsInLayer ) return true;


    if( !m_bSkipChangeSetUpdate ) {
        if( pr->m_bIsTrack )
            m_pNavObjectChangesSet->AddTrack( (Track *)pr, "update" );
        else
            m_pNavObjectChangesSet->AddRoute( pr, "update" );

    }

    return true;
}

bool MyConfig::DeleteConfigRoute( Route *pr )
{
    if( pr->m_bIsInLayer )
        return true;

    if( !m_bSkipChangeSetUpdate ) {
        if( !pr->m_bIsTrack )
            m_pNavObjectChangesSet->AddRoute( (Track *)pr, "delete" );
        else
            m_pNavObjectChangesSet->AddTrack( (Track *)pr, "delete" );

    }
    return true;
}

bool MyConfig::AddNewWayPoint( RoutePoint *pWP, int crm )
{
    if( pWP->m_bIsInLayer )
        return true;

    if(!pWP->m_bIsolatedMark)
        return true;

    if( !m_bSkipChangeSetUpdate ) {
        m_pNavObjectChangesSet->AddWP( pWP, "add" );
    }

    return true;
}

bool MyConfig::UpdateWayPoint( RoutePoint *pWP )
{
    if( pWP->m_bIsInLayer )
        return true;

    if( !m_bSkipChangeSetUpdate ) {
        m_pNavObjectChangesSet->AddWP( pWP, "update" );
    }

    return true;
}

bool MyConfig::DeleteWayPoint( RoutePoint *pWP )
{
    if( pWP->m_bIsInLayer )
        return true;

    if( !m_bSkipChangeSetUpdate ) {
        m_pNavObjectChangesSet->AddWP( pWP, "delete" );
    }

    return true;
}

bool MyConfig::AddNewTrackPoint( RoutePoint *pWP, const wxString& parent_GUID )
{
    if( !m_bSkipChangeSetUpdate ) {
        m_pNavObjectChangesSet->AddTrackPoint( pWP, "add", parent_GUID );
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

    Write( _T ( "ShowStatusBar" ), m_bShowStatusBar );
#ifndef __WXOSX__
    Write( _T ( "ShowMenuBar" ), m_bShowMenuBar );
#endif
    Write( _T ( "ShowCompassWindow" ), m_bShowCompassWin );
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
    Write( _T ( "ShowChartBar" ), g_bShowChartBar );
    
    Write( _T ( "FilterNMEA_Avg" ), g_bfilter_cogsog );
    Write( _T ( "FilterNMEA_Sec" ), g_COGFilterSec );

    Write( _T ( "ShowMag" ), g_bShowMag );
    Write( _T ( "UserMagVariation" ), wxString::Format( _T("%.2f"), g_UserVar ) );

    Write( _T ( "CM93DetailFactor" ), g_cm93_zoom_factor );
    Write( _T ( "CM93DetailZoomPosX" ), g_cm93detail_dialog_x );
    Write( _T ( "CM93DetailZoomPosY" ), g_cm93detail_dialog_y );
    Write( _T ( "ShowCM93DetailSlider" ), g_bShowCM93DetailSlider );

    Write( _T ( "SkewToNorthUp" ), g_bskew_comp );
    Write( _T ( "OpenGL" ), g_bopengl );
    Write( _T ( "ShowFPS" ), g_bShowFPS );
    
    Write( _T ( "ZoomDetailFactor" ), g_chart_zoom_modifier );
    
    Write( _T ( "FogOnOverzoom" ), g_fog_overzoom );
    Write( _T ( "OverzoomVectorScale" ), g_oz_vector_scale );
    Write( _T ( "OverzoomEmphasisBase" ), g_overzoom_emphasis_base );

#ifdef ocpnUSE_GL
    /* opengl options */
    Write( _T ( "UseAcceleratedPanning" ), g_GLOptions.m_bUseAcceleratedPanning );

    Write( _T ( "GPUTextureCompression" ), g_GLOptions.m_bTextureCompression);
    Write( _T ( "GPUTextureCompressionCaching" ), g_GLOptions.m_bTextureCompressionCaching);
    Write( _T ( "GPUTextureDimension" ), g_GLOptions.m_iTextureDimension );
    Write( _T ( "GPUTextureMemSize" ), g_GLOptions.m_iTextureMemorySize );
#endif
    Write( _T ( "SmoothPanZoom" ), g_bsmoothpanzoom );

    Write( _T ( "UseRasterCharts" ), g_bUseRaster );
    Write( _T ( "UseVectorCharts" ), g_bUseVector );
    Write( _T ( "UseCM93Charts" ), g_bUseCM93 );

    Write( _T ( "CourseUpMode" ), g_bCourseUp );
    Write( _T ( "LookAheadMode" ), g_bLookAhead );
    Write( _T ( "COGUPAvgSeconds" ), g_COGAvgSec );
    Write( _T ( "UseMagAPB" ), g_bMagneticAPB );

    Write( _T ( "OwnshipCOGPredictorMinutes" ), g_ownship_predictor_minutes );
    Write( _T ( "OwnshipCOGPredictorWidth" ), g_cog_predictor_width );
    Write( _T ( "OwnshipHDTPredictorMiles" ), g_ownship_HDTpredictor_miles );
    Write( _T ( "OwnShipIconType" ), g_OwnShipIconType );
    Write( _T ( "OwnShipLength" ), g_n_ownship_length_meters );
    Write( _T ( "OwnShipWidth" ), g_n_ownship_beam_meters );
    Write( _T ( "OwnShipGPSOffsetX" ), g_n_gps_antenna_offset_x );
    Write( _T ( "OwnShipGPSOffsetY" ), g_n_gps_antenna_offset_y );
    Write( _T ( "OwnShipMinSize" ), g_n_ownship_min_mm );

    wxString racr;
 //   racr.Printf( _T ( "%g" ), g_n_arrival_circle_radius );
 //   Write( _T ( "RouteArrivalCircleRadius" ), racr );
    Write( _T ( "RouteArrivalCircleRadius" ), wxString::Format( _T("%.2f"), g_n_arrival_circle_radius ));

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

    Write( _T( "NMEAAPBPrecision" ), g_NMEAAPBPrecision );

    Write( _T ( "AnchorWatch1GUID" ), g_AW1GUID );
    Write( _T ( "AnchorWatch2GUID" ), g_AW2GUID );

    Write( _T ( "ToolbarX" ), g_toolbar_x );
    Write( _T ( "ToolbarY" ), g_toolbar_y );
    Write( _T ( "ToolbarOrient" ), g_toolbar_orient );
    Write( _T ( "ToolbarConfig" ), g_toolbarConfig );

    Write( _T ( "GPSIdent" ), g_GPS_Ident );
    Write( _T ( "UseGarminHostUpload" ), g_bGarminHostUpload );

    Write( _T ( "MobileTouch" ), g_btouch );
    Write( _T ( "ResponsiveGraphics" ), g_bresponsive );
    
    Write( _T ( "DisplaySizeMM" ), g_config_display_size_mm );

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
    Write( _T ( "LegacyInputCOMPortFilterBehaviour" ), g_b_legacy_input_filter_behaviour );
    Write( _T( "Sailing" ), g_bSailing);
    
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
            st1.Printf( _T ( "%i" ), ((int)(vp.rotation * 180 / PI)) % 360 );
            Write( _T ( "VPRotation" ), st1 );
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
    Write( _T ( "bWplIsAprsPositionReport" ), g_bWplIsAprsPosition );
    Write( _T ( "AISCOGPredictorWidth" ), g_ais_cog_predictor_width );

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
        Write( _T ( "nDisplayCategory" ), (long) ps52plib->GetDisplayCategory() );
        Write( _T ( "nSymbolStyle" ), (int) ps52plib->m_nSymbolStyle );
        Write( _T ( "nBoundaryStyle" ), (int) ps52plib->m_nBoundaryStyle );

        Write( _T ( "bShowSoundg" ), ps52plib->m_bShowSoundg );
        Write( _T ( "bShowMeta" ), ps52plib->m_bShowMeta );
        Write( _T ( "bUseSCAMIN" ), ps52plib->m_bUseSCAMIN );
        Write( _T ( "bShowAtonText" ), ps52plib->m_bShowAtonText );
        Write( _T ( "bShowLightDescription" ), ps52plib->m_bShowLdisText );
        Write( _T ( "bExtendLightSectors" ), ps52plib->m_bExtendLightSectors );
        Write( _T ( "bDeClutterText" ), ps52plib->m_bDeClutterText );
        Write( _T ( "bShowNationalText" ), ps52plib->m_bShowNationalTexts );

        Write( _T ( "S52_MAR_SAFETY_CONTOUR" ), S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR ) );
        Write( _T ( "S52_MAR_SHALLOW_CONTOUR" ), S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR ) );
        Write( _T ( "S52_MAR_DEEP_CONTOUR" ), S52_getMarinerParam( S52_MAR_DEEP_CONTOUR ) );
        Write( _T ( "S52_MAR_TWO_SHADES" ), S52_getMarinerParam( S52_MAR_TWO_SHADES ) );
        Write( _T ( "S52_DEPTH_UNIT_SHOW" ), ps52plib->m_nDepthUnitDisplay );
    }
    SetPath( _T ( "/Directories" ) );
    Write( _T ( "S57DataLocation" ), _T("") );
//    Write( _T ( "SENCFileLocation" ), _T("") );

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

    DeleteGroup(font_path);
    
    SetPath( font_path );

    int nFonts = FontMgr::Get().GetNumFonts();

    for( int i = 0; i < nFonts; i++ ) {
        wxString cfstring(FontMgr::Get().GetConfigString(i));
        wxString valstring = FontMgr::Get().GetFullConfigDesc( i );
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

    DeleteGroup(_T ( "/MMSIProperties" ));
    SetPath( _T ( "/MMSIProperties" ) );
    for(unsigned int i=0 ; i < g_MMSI_Props_Array.GetCount() ; i++){
        wxString p;
        p.Printf(_T("Props%d"), i);
        Write( p, g_MMSI_Props_Array.Item(i)->Serialize() );
    }


    Flush();
}

void MyConfig::UpdateNavObj( void )
{

//   Create the NavObjectCollection, and save to specified file
    NavObjectCollection1 *pNavObjectSet = new NavObjectCollection1();

    pNavObjectSet->CreateAllGPXObjects();
    pNavObjectSet->SaveFile( m_sNavObjSetFile );

    delete pNavObjectSet;

    if( ::wxFileExists( m_sNavObjSetChangesFile ) )
        wxRemoveFile( m_sNavObjSetChangesFile );

    delete m_pNavObjectChangesSet;
    m_pNavObjectChangesSet = new NavObjectChanges(m_sNavObjSetChangesFile);

}

bool MyConfig::ExportGPXRoutes( wxWindow* parent, RouteList *pRoutes, const wxString suggestedName )
{
    wxFileDialog saveDialog( NULL, _( "Export GPX file" ), m_gpx_path, suggestedName,
            wxT ( "GPX files (*.gpx)|*.gpx" ), wxFD_SAVE );

#ifdef __WXOSX__
    if(parent)
        parent->HideWithEffect(wxSHOW_EFFECT_BLEND );
#endif

     int response = saveDialog.ShowModal();

#ifdef __WXOSX__
    if(parent)
        parent->ShowWithEffect(wxSHOW_EFFECT_BLEND );
#endif

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

        NavObjectCollection1 *pgpx = new NavObjectCollection1;
        pgpx->AddGPXRoutesList( pRoutes );
        pgpx->SaveFile(fn.GetFullPath());
        delete pgpx;

        return true;
    } else
        return false;
}

bool MyConfig::ExportGPXWaypoints( wxWindow* parent, RoutePointList *pRoutePoints, const wxString suggestedName )
{
    wxFileDialog saveDialog( NULL, _( "Export GPX file" ), m_gpx_path, suggestedName,
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

        NavObjectCollection1 *pgpx = new NavObjectCollection1;
        pgpx->AddGPXPointsList( pRoutePoints );
        pgpx->SaveFile(fn.GetFullPath());
        delete pgpx;

        return true;
    } else
        return false;
}

void MyConfig::ExportGPX( wxWindow* parent, bool bviz_only, bool blayer )
{
    wxFileDialog saveDialog( NULL, _( "Export GPX file" ), m_gpx_path, wxT ( "" ),
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

        NavObjectCollection1 *pgpx = new NavObjectCollection1;

        wxProgressDialog *pprog = NULL;
        int count = pWayPointMan->GetWaypointList()->GetCount();
        if( count > 200) {
            pprog = new wxProgressDialog( _("Export GPX file"), _T("0/0"), count, NULL,
                                          wxPD_APP_MODAL | wxPD_SMOOTH |
                                          wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );
            pprog->SetSize( 400, wxDefaultCoord );
            pprog->Centre();
        }

        //WPTs
        int ic = 0;

        wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
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
                    pgpx->AddGPXWaypoint( pr);
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
                    pgpx->AddGPXRoute( pRoute ); //gpxroot->AddRoute( CreateGPXRte( pRoute ) );
                else
                    pgpx->AddGPXTrack( (Track *)pRoute  );
                }
            node1 = node1->GetNext();
        }

        pgpx->SaveFile( fn.GetFullPath() );
        delete pgpx;
        ::wxEndBusyCursor();

        if( pprog)
            delete pprog;

    }
}

void MyConfig::UI_ImportGPX( wxWindow* parent, bool islayer, wxString dirpath, bool isdirectory )
{
    int response = wxID_CANCEL;
    wxArrayString file_array;
    Layer *l = NULL;

    if( !islayer || dirpath.IsSameAs( _T("") ) ) {
        wxFileDialog openDialog( NULL, _( "Import GPX file" ), m_gpx_path, wxT ( "" ),
                wxT ( "GPX files (*.gpx)|*.gpx|All files (*.*)|*.*" ),
                wxFD_OPEN | wxFD_MULTIPLE );
        openDialog.Centre();
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
            if( wxDir::GetAllFiles( dirpath, &file_array, wxT("*.gpx") ) )
                response = wxID_OK;
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

            bool bLayerViz = g_bShowLayers;
            if( g_VisibleLayers.Contains( l->m_LayerName ) )
                bLayerViz = true;
            if( g_InvisibleLayers.Contains( l->m_LayerName ) )
                bLayerViz = false;
            l->m_bIsVisibleOnChart = bLayerViz;

            wxString laymsg;
            laymsg.Printf( wxT("New layer %d: %s"), l->m_LayerID, l->m_LayerName.c_str() );
            wxLogMessage( laymsg );

            pLayerList->Insert( l );
        }

        for( unsigned int i = 0; i < file_array.GetCount(); i++ ) {
            wxString path = file_array[i];

            if( ::wxFileExists( path ) ) {

                NavObjectCollection1 *pSet = new NavObjectCollection1;
                pSet->load_file(path.fn_str());

                if(islayer){
                    l->m_NoOfItems = pSet->LoadAllGPXObjectsAsLayer(l->m_LayerID, l->m_bIsVisibleOnChart);
                }
                else
                    pSet->LoadAllGPXObjects( true );    // Import with full vizibility of names and objects

                delete pSet;
            }
        }
    }
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
    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
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
    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
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


// This function formats the input date/time into a valid GPX ISO 8601
// time string specified in the UTC time zone.

wxString FormatGPXDateTime( wxDateTime dt )
{
//      return dt.Format(wxT("%Y-%m-%dT%TZ"), wxDateTime::GMT0);
    return dt.Format( wxT("%Y-%m-%dT%H:%M:%SZ") );
}


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

#define NUM_COLS 49
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
    wxT ( "SIENNA" )
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
    double ret = NAN;
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
        case DISTANCE_FT:
            ret = nm_distance * 6076.12;
            break;
        case DISTANCE_FA:
            ret = nm_distance * 1012.68591;
            break;
        case DISTANCE_IN:
            ret = nm_distance * 72913.4;
            break;
        case DISTANCE_CM:
            ret = nm_distance * 185200;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the distance from the units selected by user to NMi  */
/**************************************************************************/
double fromUsrDistance( double usr_distance, int unit )
{
    double ret = NAN;
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
        case DISTANCE_FT:
            ret = _("ft");
            break;
        case DISTANCE_FA:
            ret = _("fa");
            break;
        case DISTANCE_IN:
            ret = _("in");
            break;
        case DISTANCE_CM:
            ret = _("cm");
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the speed to the units selected by user              */
/**************************************************************************/
double toUsrSpeed( double kts_speed, int unit )
{
    double ret = NAN;
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
    double ret = NAN;
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
        if(!oldc.IsOk())
            return;

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

        //  Sometimes, on Windows, the destination image is corrupt...
        if(NULL == box)
            return;

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
#ifdef ocpnUSE_GL
        /* opengl version */
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        if(radius > 1.0f){
            wxColour c(color.Red(), color.Green(), color.Blue(), transparency);
            dc.SetBrush(wxBrush(c));
            dc.DrawRoundedRectangle( x, y, size_x, size_y, radius );
        }
        else {
            glColor4ub( color.Red(), color.Green(), color.Blue(), transparency );
            glBegin( GL_QUADS );
            glVertex2i( x, y );
            glVertex2i( x + size_x, y );
            glVertex2i( x + size_x, y + size_y );
            glVertex2i( x, y + size_y );
            glEnd();
        }
        glDisable( GL_BLEND );
#endif
    }
}



//      CRC calculation for a byte buffer

static unsigned int crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

#define UPDC32(octet,crc) (crc_32_tab[((crc)\
^ ((unsigned char)octet)) & 0xff] ^ ((crc) >> 8))


unsigned int crc32buf(unsigned char *buf, size_t len)
{
    unsigned int oldcrc32;

    oldcrc32 = 0xFFFFFFFF;

    for ( ; len; --len, ++buf)
    {
        oldcrc32 = UPDC32(*buf, oldcrc32);
    }

    return ~oldcrc32;

}

