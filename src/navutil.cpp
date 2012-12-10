/******************************************************************************
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
 ***************************************************************************
 *
 */


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
extern long             g_n_ownship_min_mm;

extern bool             g_bPreserveScaleOnX;

extern bool             g_bUseRMC;
extern bool             g_bUseGLL;

extern TTYWindow        *g_NMEALogWindow;
extern int              g_NMEALogWindow_x, g_NMEALogWindow_y;
extern int              g_NMEALogWindow_sx, g_NMEALogWindow_sy;

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

extern ocpnStyle::StyleManager* g_StyleManager;
extern wxArrayString    TideCurrentDataSet;
extern wxString         g_TCData_Dir;
extern Multiplexer      *g_pMUX;

//------------------------------------------------------------------------------
// Some wxWidgets macros for useful classes
//------------------------------------------------------------------------------
WX_DEFINE_LIST ( RouteList );
WX_DEFINE_LIST ( SelectableItemList );
WX_DEFINE_LIST ( RoutePointList );
WX_DEFINE_LIST ( HyperlinkList );         // toh, 2009.02.22
WX_DEFINE_LIST ( LayerList );

//-----------------------------------------------------------------------------
//          Selectable Item
//-----------------------------------------------------------------------------

SelectItem::SelectItem()
{
}

SelectItem::~SelectItem()
{
}

int SelectItem::GetUserData( void )
{
    return m_Data4;
}

void SelectItem::SetUserData( int data )
{
    m_Data4 = data;
}

//-----------------------------------------------------------------------------
//          Select
//-----------------------------------------------------------------------------

Select::Select()
{
    pSelectList = new SelectableItemList;
    pixelRadius = 8;
    int w,h;
    wxDisplaySize( &w, &h );
    if( h > 800 ) pixelRadius = 10;
    if( h > 1024 ) pixelRadius = 12;
}

Select::~Select()
{
    pSelectList->DeleteContents( true );
    pSelectList->Clear();
    delete pSelectList;

}

bool Select::AddSelectableRoutePoint( float slat, float slon, RoutePoint *pRoutePointAdd )
{
    SelectItem *pSelItem = new SelectItem;
    pSelItem->m_slat = slat;
    pSelItem->m_slon = slon;
    pSelItem->m_seltype = SELTYPE_ROUTEPOINT;
    pSelItem->m_bIsSelected = false;
    pSelItem->m_pData1 = pRoutePointAdd;

    if( pRoutePointAdd->m_bIsInLayer ) pSelectList->Append( pSelItem );
    else
        pSelectList->Insert( pSelItem );

    return true;
}

bool Select::AddSelectableRouteSegment( float slat1, float slon1, float slat2, float slon2,
        RoutePoint *pRoutePointAdd1, RoutePoint *pRoutePointAdd2, Route *pRoute )
{
    SelectItem *pSelItem = new SelectItem;
    pSelItem->m_slat = slat1;
    pSelItem->m_slon = slon1;
    pSelItem->m_slat2 = slat2;
    pSelItem->m_slon2 = slon2;
    pSelItem->m_seltype = SELTYPE_ROUTESEGMENT;
    pSelItem->m_bIsSelected = false;
    pSelItem->m_pData1 = pRoutePointAdd1;
    pSelItem->m_pData2 = pRoutePointAdd2;
    pSelItem->m_pData3 = pRoute;

    if( pRoute->m_bIsInLayer ) pSelectList->Append( pSelItem );
    else
        pSelectList->Insert( pSelItem );

    return true;
}

bool Select::DeleteAllSelectableRouteSegments( Route *pr )
{
    SelectItem *pFindSel;

//    Iterate on the select list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == SELTYPE_ROUTESEGMENT ) {

//                  RoutePoint *ps1 = (RoutePoint *)pFindSel->m_pData1;
//                  RoutePoint *ps2 = (RoutePoint *)pFindSel->m_pData2;

            if( (Route *) pFindSel->m_pData3 == pr ) {
                delete pFindSel;
                pSelectList->DeleteNode( node );   //delete node;

                node = pSelectList->GetFirst();     // reset the top node

                goto got_next_outer_node;
            }
        }

        node = node->GetNext();
        got_next_outer_node: continue;
    }

    return true;
}

bool Select::DeleteAllSelectableRoutePoints( Route *pr )
{
    SelectItem *pFindSel;

//    Iterate on the select list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == SELTYPE_ROUTEPOINT ) {
            RoutePoint *ps = (RoutePoint *) pFindSel->m_pData1;

            //    inner loop iterates on the route's point list
            wxRoutePointListNode *pnode = ( pr->pRoutePointList )->GetFirst();
            while( pnode ) {
                RoutePoint *prp = pnode->GetData();

                if( prp == ps ) {
                    delete pFindSel;
                    pSelectList->DeleteNode( node );   //delete node;
                    node = pSelectList->GetFirst();

                    goto got_next_outer_node;
                }
                pnode = pnode->GetNext();
            }
        }

        node = node->GetNext();
        got_next_outer_node: continue;
    }
    return true;
}

bool Select::AddAllSelectableRoutePoints( Route *pr )
{
    if( pr->pRoutePointList->GetCount() ) {
        wxRoutePointListNode *node = ( pr->pRoutePointList )->GetFirst();

        while( node ) {
            RoutePoint *prp = node->GetData();
            AddSelectableRoutePoint( prp->m_lat, prp->m_lon, prp );
            node = node->GetNext();
        }
        return true;
    } else
        return false;
}

bool Select::AddAllSelectableRouteSegments( Route *pr )
{
    wxPoint rpt, rptn;
    float slat1, slon1, slat2, slon2;

    if( pr->pRoutePointList->GetCount() ) {
        wxRoutePointListNode *node = ( pr->pRoutePointList )->GetFirst();

        RoutePoint *prp0 = node->GetData();
        slat1 = prp0->m_lat;
        slon1 = prp0->m_lon;

        node = node->GetNext();

        while( node ) {
            RoutePoint *prp = node->GetData();
            slat2 = prp->m_lat;
            slon2 = prp->m_lon;

            AddSelectableRouteSegment( slat1, slon1, slat2, slon2, prp0, prp, pr );

            slat1 = slat2;
            slon1 = slon2;
            prp0 = prp;

            node = node->GetNext();
        }
        return true;
    } else
        return false;
}

bool Select::AddAllSelectableTrackSegments( Route *pr )
{
    wxPoint rpt, rptn;
    float slat1, slon1, slat2, slon2;

    if( pr->pRoutePointList->GetCount() ) {
        wxRoutePointListNode *node = ( pr->pRoutePointList )->GetFirst();

        RoutePoint *prp0 = node->GetData();
        slat1 = prp0->m_lat;
        slon1 = prp0->m_lon;

        node = node->GetNext();

        while( node ) {
            RoutePoint *prp = node->GetData();
            slat2 = prp->m_lat;
            slon2 = prp->m_lon;

            AddSelectableTrackSegment( slat1, slon1, slat2, slon2, prp0, prp, pr );

            slat1 = slat2;
            slon1 = slon2;
            prp0 = prp;

            node = node->GetNext();
        }
        return true;
    } else
        return false;
}

bool Select::UpdateSelectableRouteSegments( RoutePoint *prp )
{
    SelectItem *pFindSel;
    bool ret = false;

//    Iterate on the select list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == SELTYPE_ROUTESEGMENT ) {
            if( pFindSel->m_pData1 == prp ) {
                pFindSel->m_slat = prp->m_lat;
                pFindSel->m_slon = prp->m_lon;
                ret = true;
                ;
            }

            else
                if( pFindSel->m_pData2 == prp ) {
                    pFindSel->m_slat2 = prp->m_lat;
                    pFindSel->m_slon2 = prp->m_lon;
                    ret = true;
                }
        }
        node = node->GetNext();
    }

    return ret;
}

//-----------------------------------------------------------------------------------
//          Selectable Point Object Support
//-----------------------------------------------------------------------------------

SelectItem *Select::AddSelectablePoint( float slat, float slon, void *pdata, int fseltype )
{
    SelectItem *pSelItem = new SelectItem;
    if( pSelItem ) {
        pSelItem->m_slat = slat;
        pSelItem->m_slon = slon;
        pSelItem->m_seltype = fseltype;
        pSelItem->m_bIsSelected = false;
        pSelItem->m_pData1 = pdata;

        pSelectList->Append( pSelItem );
    }

    return pSelItem;
}

bool Select::DeleteAllPoints( void )
{
    pSelectList->DeleteContents( true );
    pSelectList->Clear();
    return true;
}

bool Select::DeleteSelectablePoint( void *pdata, int SeltypeToDelete )
{
    SelectItem *pFindSel;

    if( NULL != pdata ) {
//    Iterate on the list
        wxSelectableItemListNode *node = pSelectList->GetFirst();

        while( node ) {
            pFindSel = node->GetData();
            if( pFindSel->m_seltype == SeltypeToDelete ) {
                if( pdata == pFindSel->m_pData1 ) {
                    delete pFindSel;
                    delete node;
                    return true;
                }
            }
            node = node->GetNext();
        }
    }
    return false;
}

bool Select::DeleteAllSelectableTypePoints( int SeltypeToDelete )
{
    SelectItem *pFindSel;

//    Iterate on the list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == SeltypeToDelete ) {
            delete pFindSel;
            delete node;
            node = pSelectList->GetFirst();
            goto got_next_node;
        }

        node = node->GetNext();
        got_next_node: continue;
    }
    return true;
}

bool Select::ModifySelectablePoint( float lat, float lon, void *data, int SeltypeToModify )
{
    SelectItem *pFindSel;

//    Iterate on the list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == SeltypeToModify ) {
            if( data == pFindSel->m_pData1 ) {
                pFindSel->m_slat = lat;
                pFindSel->m_slon = lon;
                return true;
            }
        }

        node = node->GetNext();
    }
    return false;
}

bool Select::AddSelectableTrackSegment( float slat1, float slon1, float slat2, float slon2,
        RoutePoint *pRoutePointAdd1, RoutePoint *pRoutePointAdd2, Route *pRoute )
{
    SelectItem *pSelItem = new SelectItem;
    pSelItem->m_slat = slat1;
    pSelItem->m_slon = slon1;
    pSelItem->m_slat2 = slat2;
    pSelItem->m_slon2 = slon2;
    pSelItem->m_seltype = SELTYPE_TRACKSEGMENT;
    pSelItem->m_bIsSelected = false;
    pSelItem->m_pData1 = pRoutePointAdd1;
    pSelItem->m_pData2 = pRoutePointAdd2;
    pSelItem->m_pData3 = pRoute;

    if( pRoute->m_bIsInLayer ) pSelectList->Append( pSelItem );
    else
        pSelectList->Insert( pSelItem );

    return true;
}

bool Select::DeleteAllSelectableTrackSegments( Route *pr )
{
    SelectItem *pFindSel;

//    Iterate on the select list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == SELTYPE_TRACKSEGMENT ) {

            if( (Route *) pFindSel->m_pData3 == pr ) {
                delete pFindSel;
                pSelectList->DeleteNode( node );   //delete node;

                node = pSelectList->GetFirst();     // reset the top node
                goto got_next_outer_node;
            }
        }
        node = node->GetNext();
        got_next_outer_node: continue;
    }
    return true;
}

bool Select::DeletePointSelectableTrackSegments( RoutePoint *pr )
{
    SelectItem *pFindSel;

//    Iterate on the select list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == SELTYPE_TRACKSEGMENT ) {

            if( (RoutePoint *) pFindSel->m_pData1 == pr || (RoutePoint *) pFindSel->m_pData2 == pr ) {
                delete pFindSel;
                pSelectList->DeleteNode( node );   //delete node;

                node = pSelectList->GetFirst();     // reset the top node
                goto got_next_outer_node;
            }
        }
        node = node->GetNext();
        got_next_outer_node: continue;
    }
    return true;
}

bool Select::IsSegmentSelected( float a, float b, float c, float d, float slat, float slon )
{
    double adder = 0.;

    if( ( c * d ) < 0. ) {
        //    Arrange for points to be increasing longitude, c to d
        double dist, brg;
        DistanceBearingMercator( a, c, b, d, &brg, &dist );
        if( brg < 180. )             // swap points?
                {
            double tmp;
            tmp = c;
            c = d;
            d = tmp;
            tmp = a;
            a = b;
            b = tmp;
        }
        if( d < 0. )     // idl?
                {
            d += 360.;
            if( slon < 0. ) adder = 360.;
        }
    }

//    As a course test, use segment bounding box test
    if( ( slat >= ( fmin ( a,b ) - selectRadius ) ) && ( slat <= ( fmax ( a,b ) + selectRadius ) )
            && ( ( slon + adder ) >= ( fmin ( c,d ) - selectRadius ) )
            && ( ( slon + adder ) <= ( fmax ( c,d ) + selectRadius ) ) ) {
        //    Use vectors to do hit test....
        vector2D va, vb, vn;

        //    Assuming a Mercator projection
        double ap, cp;
        toSM( a, c, 0., 0., &cp, &ap );
        double bp, dp;
        toSM( b, d, 0., 0., &dp, &bp );
        double slatp, slonp;
        toSM( slat, slon + adder, 0., 0., &slonp, &slatp );

        va.x = slonp - cp;
        va.y = slatp - ap;
        vb.x = dp - cp;
        vb.y = bp - ap;

        double delta = vGetLengthOfNormal( &va, &vb, &vn );
        if( fabs( delta ) < ( selectRadius * 1852 * 60 ) ) return true;
    }
    return false;
}

void Select::CalcSelectRadius() {
    selectRadius = pixelRadius / ( cc1->GetCanvasTrueScale() * 1852 * 60 );
}

SelectItem *Select::FindSelection( float slat, float slon, int fseltype )
{
    float a, b, c, d;
    SelectItem *pFindSel;

    CalcSelectRadius();

//    Iterate on the list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == fseltype ) {
            switch( fseltype ){
                case SELTYPE_ROUTEPOINT:
                case SELTYPE_TIDEPOINT:
                case SELTYPE_CURRENTPOINT:
                case SELTYPE_AISTARGET:
                    a = fabs( slat - pFindSel->m_slat );
                    b = fabs( slon - pFindSel->m_slon );

                    if( ( fabs( slat - pFindSel->m_slat ) < selectRadius )
                            && ( fabs( slon - pFindSel->m_slon ) < selectRadius ) ) goto find_ok;
                    break;
                case SELTYPE_ROUTESEGMENT:
                case SELTYPE_TRACKSEGMENT: {
                    a = pFindSel->m_slat;
                    b = pFindSel->m_slat2;
                    c = pFindSel->m_slon;
                    d = pFindSel->m_slon2;

                    if( IsSegmentSelected( a, b, c, d, slat, slon ) ) goto find_ok;
                    break;
                }
                default:
                    break;
            }
        }

        node = node->GetNext();
    }

    return NULL;
    find_ok: return pFindSel;
}

bool Select::IsSelectableSegmentSelected( float slat, float slon, SelectItem *pFindSel )
{
    CalcSelectRadius();

    float a = pFindSel->m_slat;
    float b = pFindSel->m_slat2;
    float c = pFindSel->m_slon;
    float d = pFindSel->m_slon2;

    return IsSegmentSelected( a, b, c, d, slat, slon );
}

SelectableItemList Select::FindSelectionList( float slat, float slon, int fseltype )
{
    float a, b, c, d;
    SelectItem *pFindSel;
    SelectableItemList ret_list;

    CalcSelectRadius();

//    Iterate on the list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while( node ) {
        pFindSel = node->GetData();
        if( pFindSel->m_seltype == fseltype ) {
            switch( fseltype ){
                case SELTYPE_ROUTEPOINT:
                case SELTYPE_TIDEPOINT:
                case SELTYPE_CURRENTPOINT:
                case SELTYPE_AISTARGET:
                    if( ( fabs( slat - pFindSel->m_slat ) < selectRadius )
                            && ( fabs( slon - pFindSel->m_slon ) < selectRadius ) ) {
                        ret_list.Append( pFindSel );
                    }
                    break;
                case SELTYPE_ROUTESEGMENT:
                case SELTYPE_TRACKSEGMENT: {
                    a = pFindSel->m_slat;
                    b = pFindSel->m_slat2;
                    c = pFindSel->m_slon;
                    d = pFindSel->m_slon2;

                    if( IsSegmentSelected( a, b, c, d, slat, slon ) ) ret_list.Append( pFindSel );

                    break;
                }
                default:
                    break;
            }
        }

        node = node->GetNext();
    }

    return ret_list;
}

//-----------------------------------------------------------------------------
//          WayPoint Implementation
//-----------------------------------------------------------------------------

RoutePoint::RoutePoint() {
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
RoutePoint::RoutePoint( RoutePoint* orig ) {
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

void RoutePoint::SetName( wxString name )
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

bool RoutePoint::SendToGPS( wxString& com_name, wxGauge *pProgress )
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

//-----------------------------------------------------------------------------
//          Route Implementation
//-----------------------------------------------------------------------------

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

    m_ArrivalRadius = .05;        // default, Miles

    RBBox.Reset();
    m_bcrosses_idl = false;

    m_bIsInLayer = g_bIsNewLayer;
    if( m_bIsInLayer ) {
        m_LayerID = g_LayerIdx;
        m_bListed = false;
    } else
        m_LayerID = 0;
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
void Route::CloneRoute( Route *psourceroute, int start_nPoint, int end_nPoint, wxString suffix )
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

void Route::CloneTrack( Route *psourceroute, int start_nPoint, int end_nPoint, wxString suffix )
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

        if( ( start_nPoint == 2 ) /*&& (psourcepoint->m_GPXTrkSegNo == startTrkSegNo)*/) segment_shift =
                psourcepoint->m_GPXTrkSegNo - 1; // continue first segment if tracks share the first point

        if( b_splitting ) m_pLastAddedPoint->m_GPXTrkSegNo = ( psourcepoint->m_GPXTrkSegNo
                - startTrkSegNo ) + 1;
        else
            m_pLastAddedPoint->m_GPXTrkSegNo = startTrkSegNo + segment_shift;
    }

    CalculateBBox();

}

void Route::CloneAddedRoutePoint( RoutePoint *ptargetpoint, RoutePoint *psourcepoint )
{
    ptargetpoint->m_MarkDescription = psourcepoint->m_MarkDescription;
    ptargetpoint->m_prop_string_format = psourcepoint->m_prop_string_format;
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
    ptargetpoint->m_CreateTime = psourcepoint->m_CreateTime;
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
    ptargetpoint->m_prop_string_format = psourcepoint->m_prop_string_format;
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
    ptargetpoint->m_CreateTime = psourcepoint->m_CreateTime;
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

bool Route::SendToGPS( wxString& com_name, bool bsend_waypoints, wxGauge *pProgress )
{
    bool result = false;
    if( g_pMUX ) result = g_pMUX->SendRouteToGPS( this, com_name, bsend_waypoints, pProgress );

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
//---------------------------------------------------------------------------------
//    Track Implementation
//---------------------------------------------------------------------------------
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

    if( b_addpoint ) AddPointNow();
    else
        if( ( GetnPoints() < 2 ) && !g_bTrackDaily ) //continuously update track beginning point timestamp if no movement.
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

//-----------------------------------------------------------------------------
//          Layer Implementation
//-----------------------------------------------------------------------------

Layer::Layer( void )
{
    m_bIsVisibleOnChart = g_bShowLayers;
    m_bIsVisibleOnListing = false;
    m_bHasVisibleNames = true;
    m_NoOfItems = 0;

    m_LayerName = _T("");
    m_LayerFileName = _T("");
    m_LayerDescription = _T("");
    m_CreateTime = wxDateTime::Now();
}

Layer::~Layer( void )
{
//  Remove this layer from the global layer list
    if( NULL != pLayerList ) pLayerList->DeleteObject( this );

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

    Read( _T ( "SDMMFormat" ), &g_iSDMMFormat, 0 ); //0 = "Degrees, Decimal minutes"), 1 = "Decimal degrees", 2 = "Degrees,Minutes, Seconds"

    Read( _T ( "OwnshipCOGPredictorMinutes" ), &g_ownship_predictor_minutes, 5 );
    Read( _T ( "OwnShipIconType" ), &g_OwnShipIconType, 0 );
    Read( _T ( "OwnShipLength" ), &g_n_ownship_length_meters, 0 );
    Read( _T ( "OwnShipWidth" ), &g_n_ownship_beam_meters, 0 );
    Read( _T ( "OwnShipGPSOffsetX" ), &g_n_gps_antenna_offset_x, 0 );
    Read( _T ( "OwnShipGPSOffsetY" ), &g_n_gps_antenna_offset_y, 0 );
    Read( _T ( "OwnShipMinSize" ), &g_n_ownship_min_mm, 0 );

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

    g_NMEALogWindow_sx = Read( _T ( "NMEALogWindowSizeX" ), 400L );
    g_NMEALogWindow_sy = Read( _T ( "NMEALogWindowSizeY" ), 100L );
    g_NMEALogWindow_x = Read( _T ( "NMEALogWindowPosX" ), 10L );
    g_NMEALogWindow_y = Read( _T ( "NMEALogWindowPosY" ), 10L );
    
    if( ( g_NMEALogWindow_x < 0 ) || ( g_NMEALogWindow_x > display_width ) ) g_NMEALogWindow_x = 5;
    if( ( g_NMEALogWindow_y < 0 ) || ( g_NMEALogWindow_y > display_height ) ) g_NMEALogWindow_y = 5;
                              
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
    Read( _T ( "bAISAlertDialog" ), &g_bAIS_CPA_Alert );

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
                prm->Baudrate = wxAtoi(xRate);
                prm->Port = port;
                
                g_pConnectionParams->Add(prm);
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

    Write( _T ( "NMEALogWindowSizeX" ), g_NMEALogWindow_sx );
    Write( _T ( "NMEALogWindowSizeY" ), g_NMEALogWindow_sy );
    Write( _T ( "NMEALogWindowPosX" ), g_NMEALogWindow_x );
    Write( _T ( "NMEALogWindowPosY" ), g_NMEALogWindow_y );

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

bool MyConfig::ExportGPXRoute( wxWindow* parent, Route *pRoute )
{
    //FIXME: get rid of the Dialogs and unite with the other
    wxFileDialog saveDialog( parent, _( "Export GPX file" ), m_gpx_path, pRoute->m_RouteNameString,
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
        // TODO this is awkward
        if( !pRoute->m_bIsTrack ) {
            gpxroot->AddRoute( CreateGPXRte( pRoute ) );
        } else {
            gpxroot->AddTrack( CreateGPXTrk( pRoute ) );
        }

        gpx->SaveFile( fn.GetFullPath() );
        gpx->Clear();
        delete gpx;
        return true;
    } else
        return false;
}

bool MyConfig::ExportGPXWaypoint( wxWindow* parent, RoutePoint *pRoutePoint )
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
        if( !WptIsInRouteList( pRoutePoint ) || pRoutePoint->m_bKeepXRoute ) {
            gpxroot->AddWaypoint( ::CreateGPXWpt( pRoutePoint, GPX_WPT_WAYPOINT ) );
        }
        gpx->SaveFile( fn.GetFullPath() );

        gpx->Clear();
        delete gpx;
        return true;
    } else
        return false;
}

void MyConfig::ExportGPX( wxWindow* parent )
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

        GpxDocument *gpx = new GpxDocument();
        GpxRootElement *gpxroot = (GpxRootElement *) gpx->RootElement();
        //WPTs
        wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();
        RoutePoint *pr;
        while( node ) {
            pr = node->GetData();

//                  if ( pr->m_bKeepXRoute || !WptIsInRouteList ( pr ) )
            if( ( pr->m_bKeepXRoute || !WptIsInRouteList( pr ) ) && !( pr->m_bIsInLayer ) ) {
                gpxroot->AddWaypoint( CreateGPXWpt( pr, GPX_WPT_WAYPOINT ) );
            }
            node = node->GetNext();
        }
        //RTEs and TRKs
        wxRouteListNode *node1 = pRouteList->GetFirst();
        while( node1 ) {
            Route *pRoute = node1->GetData();
            if( !( pRoute->m_bIsInLayer ) ) {
                if( !pRoute->m_bIsTrack ) {
                    gpxroot->AddRoute( CreateGPXRte( pRoute ) );
                } else {
                    gpxroot->AddTrack( CreateGPXTrk( pRoute ) );
                }
            }
            node1 = node1->GetNext();
        }

        gpx->SaveFile( fn.GetFullPath() );
        gpx->Clear();
        delete gpx;
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
    if( g_bIsNewLayer ) return NULL;
    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();
    bool Exists = false;
    while( node ) {
        RoutePoint *pr = node->GetData();

        if( pr->m_bIsInLayer ) return NULL;

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
    if( g_bIsNewLayer ) return NULL;
    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();
    while( node ) {
        RoutePoint *pr = node->GetData();

        if( pr->m_bIsInLayer ) return NULL;

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
    if( g_bIsNewLayer ) pTentRoute->SetVisible( g_bLayerViz, false );
    else
        if( b_propviz ) pTentRoute->SetVisible( b_viz, false );
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
//    XML Support for Navigation Objects
//---------------------------------------------------------------------------------

NavObjectCollection::NavObjectCollection() :
        GpxDocument()
{
    m_pXMLrootnode = (GpxRootElement *) RootElement();
}

NavObjectCollection::~NavObjectCollection()
{
//     delete m_pXMLrootnode;            // done in base class
}

bool NavObjectCollection::CreateNavObjGPXPoints( void )
{

    //    Iterate over the Routepoint list, creating Nodes for
    //    Routepoints that are not in any Route
    //    as indicated by m_bIsolatedMark == false

    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

    RoutePoint *pr;

    while( node ) {
        pr = node->GetData();

        if( ( pr->m_bIsolatedMark ) && !( pr->m_bIsInLayer ) )      //( !WptIsInRouteList ( pr ) )
                {
            m_pXMLrootnode->AddWaypoint( CreateGPXWpt( pr, GPX_WPT_WAYPOINT ) );
        }
        node = node->GetNext();
    }

    return true;
}

bool NavObjectCollection::CreateNavObjGPXRoutes( void )
{
    // Routes
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();

        if( !pRoute->m_bIsTrack && !( pRoute->m_bIsInLayer ) )                         // Not tracks
                {
            m_pXMLrootnode->AddRoute( CreateGPXRte( pRoute ) );
        }
        node1 = node1->GetNext();
    }

    return true;
}

bool NavObjectCollection::CreateNavObjGPXTracks( void )
{
    // Tracks
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        RoutePointList *pRoutePointList = pRoute->pRoutePointList;             //->GetCount(); do if

        if( pRoutePointList->GetCount() ) {
            if( pRoute->m_bIsTrack && !( pRoute->m_bIsInLayer ) )                     // Tracks only
                    {
                //Redundant - RoutePointList *pRoutePointList = pRoute->pRoutePointList;
                m_pXMLrootnode->AddTrack( CreateGPXTrk( pRoute ) );
            }
        }
        node1 = node1->GetNext();
    }

    return true;
}

bool NavObjectCollection::LoadAllGPXObjects()
{
    //FIXME: unite with MyConfig::ImportGPX
    TiXmlNode *root = RootElement();

    wxString RootName = wxString::FromUTF8( root->Value() );
    if( RootName == _T ( "gpx" ) ) {
        TiXmlNode *child;
        for( child = root->FirstChild(); child != 0; child = child->NextSibling() ) {
            wxString ChildName = wxString::FromUTF8( child->Value() );
            if( ChildName == _T ( "trk" ) ) ::GPXLoadTrack( (GpxTrkElement *) child );
            else
                if( ChildName == _T ( "rte" ) ) {
                    int m_NextRouteNum = 0; //FIXME: we do not need it for GPX
                    ::GPXLoadRoute( (GpxRteElement *) child, m_NextRouteNum );
                } else
                    if( ChildName == _T ( "wpt" ) ) {
                        int m_NextWPNum = 0; //FIXME: we do not need it for GPX
                        RoutePoint *pWp = ::LoadGPXWaypoint( (GpxWptElement *) child,
                                _T("circle") );
                        RoutePoint *pExisting = WaypointExists( pWp->GetName(), pWp->m_lat,
                                pWp->m_lon );
                        if( !pExisting ) {
                            if( NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->Append( pWp );
                            pWp->m_bIsolatedMark = true;      // This is an isolated mark
                            pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                            pWp->m_ConfigWPNum = m_NextWPNum;
                            m_NextWPNum++;
                        }
                    }
        }
    }

    return true;
}

//---------------------------------------------------------------------------------
//          Private Font Manager and Helpers
//---------------------------------------------------------------------------------
#include <wx/fontdlg.h>
#include <wx/fontenum.h>
#include "wx/encinfo.h"
#include "wx/fontutil.h"

MyFontDesc::MyFontDesc( wxString DialogString, wxString ConfigString, wxFont *pFont,
        wxColour color )
{
    m_dialogstring = DialogString;
    m_configstring = ConfigString;

    m_nativeInfo = pFont->GetNativeFontInfoDesc();

    m_font = pFont;
    m_color = color;
}

MyFontDesc::~MyFontDesc()
{
    delete m_font;
}

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( FontList );

FontMgr::FontMgr()
{

    //    Create the list of fonts
    m_fontlist = new FontList;
    m_fontlist->DeleteContents( true );

    //    Get a nice generic font as default
    pDefFont = wxTheFontList->FindOrCreateFont( 12, wxDEFAULT, wxNORMAL, wxBOLD, FALSE,
            wxString( _T ( "" ) ), wxFONTENCODING_SYSTEM );

}

FontMgr::~FontMgr()
{
    delete m_fontlist;
}

wxColour FontMgr::GetFontColor( const wxString &TextElement )
{
    //    Look thru the font list for a match
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );
    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_dialogstring == TextElement ) return pmfd->m_color;
        node = node->GetNext();
    }

    return wxColour( 0, 0, 0 );
}

wxString FontMgr::GetFontConfigKey( const wxString &description )
{
    // Create the configstring by combining the locale with
    // a hash of the font description. Hash is used because the i18n
    // description can contain characters that mess up the config file.

    wxString configkey;
    configkey = g_locale;
    configkey.Append( _T("-") );

    using namespace std;
    locale loc;
    const collate<char>& coll = use_facet<collate<char> >( loc );
    char cFontDesc[101];
    wcstombs( cFontDesc, description.c_str(), 100 );
    cFontDesc[100] = 0;
    int fdLen = strlen( cFontDesc );

    configkey.Append(
            wxString::Format( _T("%08lx"),
                    coll.hash( cFontDesc, cFontDesc + fdLen ) ) );
    return configkey;
}

wxFont *FontMgr::GetFont( const wxString &TextElement, int default_size )
{
    //    Look thru the font list for a match
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );
    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_dialogstring == TextElement ) return pmfd->m_font;
        node = node->GetNext();
    }

    // Found no font, so create a nice one and add to the list
    wxString configkey = GetFontConfigKey( TextElement );

    //    Now create a benign, always present native string
    //    Optional user requested default size
    int new_size;
    if( 0 == default_size ) new_size = 12;
    else
        new_size = default_size;

    wxString nativefont = GetSimpleNativeFont( new_size );

    wxFont *nf = wxFont::New( nativefont );

    wxColor color( *wxBLACK );

    MyFontDesc *pnewfd = new MyFontDesc( TextElement, configkey, nf, color );
    m_fontlist->Append( pnewfd );

    return pnewfd->m_font;

}

wxString FontMgr::GetSimpleNativeFont( int size )
{
    //    Now create a benign, always present native string
    wxString nativefont;

//    For those platforms which have no native font description string format
    nativefont.Printf( _T ( "%d;%d;%d;%d;%d;%d;%s;%d" ),
            0,                                 // version
            size, wxFONTFAMILY_DEFAULT, (int) wxFONTSTYLE_NORMAL, (int) wxFONTWEIGHT_NORMAL, false,
            "", (int) wxFONTENCODING_DEFAULT );

//    If we know of a detailed description string format, use it.
#ifdef __WXGTK__
    nativefont.Printf ( _T ( "Fixed %2d" ), size );
#endif

#ifdef __WXX11__
    nativefont = _T ( "0;-*-fixed-*-*-*-*-*-120-*-*-*-*-iso8859-1" );
#endif

#ifdef __WXMSW__
//      nativefont = _T ( "0;-11;0;0;0;400;0;0;0;0;0;0;0;0;MS Sans Serif" );

    int h, w, hm, wm;
    ::wxDisplaySize( &w, &h );            // pixels
    ::wxDisplaySizeMM( &wm, &hm );        // MM
    double pix_per_inch_v = wxMax( 72.0, ( h / hm ) * 25.4);
    int lfHeight = -(int) ( ( size * ( pix_per_inch_v / 72.0 ) ) + 0.5 );

    nativefont.Printf( _T("%d;%ld;%ld;%ld;%ld;%ld;%d;%d;%d;%d;%d;%d;%d;%d;"), 0, // version, in case we want to change the format later
            lfHeight,            //lf.lfHeight
            0,                   //lf.lfWidth,
            0,                   //lf.lfEscapement,
            0,                   //lf.lfOrientation,
            400,                 //lf.lfWeight,
            0,                   //lf.lfItalic,
            0,                   //lf.lfUnderline,
            0,                   //lf.lfStrikeOut,
            0,                   //lf.lfCharSet,
            0,                   //lf.lfOutPrecision,
            0,                   //lf.lfClipPrecision,
            0,                   //lf.lfQuality,
            0 );                    //lf.lfPitchAndFamily,

    nativefont.Append( _T("Verdana") );
#endif

    return nativefont;
}

bool FontMgr::SetFont( wxString &TextElement, wxFont *pFont, wxColour color )
{
    //    Look thru the font list for a match
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );
    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_dialogstring == TextElement ) {
            // Todo Think about this
            //

//      Cannot delete the present font, since it may be in use elsewhere
//      This WILL leak....but only on font changes

//              delete pmfd->m_font;                            // purge any old value

            pmfd->m_font = pFont;
            pmfd->m_nativeInfo = pFont->GetNativeFontInfoDesc();
            pmfd->m_color = color;

            return true;
        }
        node = node->GetNext();
    }

    return false;
}

int FontMgr::GetNumFonts( void )
{
    return m_fontlist->GetCount();
}

wxString *FontMgr::GetConfigString( int i )
{
    MyFontDesc *pfd = (MyFontDesc *) ( m_fontlist->Item( i )->GetData() );
    wxString *ret = &pfd->m_configstring;
    return ret;
}

wxString *FontMgr::GetDialogString( int i )
{
    MyFontDesc *pfd = (MyFontDesc *) ( m_fontlist->Item( i )->GetData() );
    wxString *ret = &pfd->m_dialogstring;
    return ret;
}

wxString *FontMgr::GetNativeDesc( int i )
{
    MyFontDesc *pfd = (MyFontDesc *) ( m_fontlist->Item( i )->GetData() );
    wxString *ret = &pfd->m_nativeInfo;
    return ret;
}

wxString FontMgr::GetFullConfigDesc( int i )
{
    MyFontDesc *pfd = (MyFontDesc *) ( m_fontlist->Item( i )->GetData() );
    wxString ret = pfd->m_dialogstring;
    ret.Append( _T ( ":" ) );
    ret.Append( pfd->m_nativeInfo );
    ret.Append( _T ( ":" ) );

    wxString cols( _T("rgb(0,0,0)") );
    if( pfd->m_color.IsOk() ) cols = pfd->m_color.GetAsString( wxC2S_CSS_SYNTAX );

    ret.Append( cols );
    return ret;
}

void FontMgr::LoadFontNative( wxString *pConfigString, wxString *pNativeDesc )
{
    //    Parse the descriptor string

    wxStringTokenizer tk( *pNativeDesc, _T ( ":" ) );
    wxString dialogstring = tk.GetNextToken();
    wxString nativefont = tk.GetNextToken();

    wxString c = tk.GetNextToken();
    wxColour color( c );            // from string description

    //    Search for a match in the list
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );

    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_configstring == *pConfigString ) {
            pmfd->m_nativeInfo = nativefont;
            wxFont *nf = pmfd->m_font->New( pmfd->m_nativeInfo );
            pmfd->m_font = nf;
            break;
        }
        node = node->GetNext();
    }

    //    Create and add the font to the list
    if( !node ) {

        wxFont *nf0 = new wxFont();
        wxFont *nf = nf0->New( nativefont );

        //    Scrub the native font string for bad unicode conversion
#ifdef __WXMSW__
        wxString face = nf->GetFaceName();
        const wxChar *t = face.c_str();
        if( *t > 255 ) {
            delete nf;
            wxString substitute_native = GetSimpleNativeFont( 12 );
            nf = nf0->New( substitute_native );
        }
#endif
        delete nf0;

        MyFontDesc *pnewfd = new MyFontDesc( dialogstring, *pConfigString, nf, color );
        m_fontlist->Append( pnewfd );

    }

}

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

void X11FontPicker::SetChoiceOptionsFromFacename ( wxString &facename )
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

bool LogMessageOnce( wxString &msg )
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

//    TTYScroll and TTYWindow Implemetation

IMPLEMENT_DYNAMIC_CLASS( TTYWindow, wxDialog )
BEGIN_EVENT_TABLE( TTYWindow, wxDialog )
EVT_CLOSE(TTYWindow::OnCloseWindow)
EVT_MOVE( TTYWindow::OnMove )
EVT_SIZE( TTYWindow::OnSize )
END_EVENT_TABLE()

TTYWindow::TTYWindow()
{
}

TTYWindow::TTYWindow(wxWindow *parent, int n_lines)
{
    wxDialog::Create( parent, -1, _T("Title"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );
    
    wxBoxSizer* bSizerOuterContainer = new wxBoxSizer( wxVERTICAL );
    SetSizer( bSizerOuterContainer );
    
    m_pScroll = new TTYScroll(this, n_lines);
    m_pScroll->Scroll(-1, 1000);        // start with full scroll down
    
    bSizerOuterContainer->Add( m_pScroll, 1, wxEXPAND, 5 );

    wxBoxSizer* bSizerBottomContainer = new wxBoxSizer( wxHORIZONTAL );
    bSizerOuterContainer->Add( bSizerBottomContainer, 0, wxEXPAND, 5 );
    
 
    wxStaticBox *psb = new wxStaticBox( this,  wxID_ANY, _("Legend")) ;
    wxStaticBoxSizer* sbSizer1 = new wxStaticBoxSizer( psb , wxVERTICAL );
    
    CreateLegendBitmap();
    wxBitmapButton *bb = new wxBitmapButton(this, wxID_ANY, m_bm_legend);
    sbSizer1->Add( bb, 1, wxALL|wxEXPAND, 5 );
    bSizerBottomContainer->Add( sbSizer1, 0, wxALIGN_LEFT | wxALL, 5 );

    m_buttonPause = new wxButton( this, wxID_ANY, _("Pause"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizerBottomContainer->Add( m_buttonPause, 0, wxALIGN_RIGHT | wxALL, 5 );
    
    m_buttonPause->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TTYWindow::OnPauseClick ), NULL, this );
   
    bpause = false;
}

TTYWindow::~TTYWindow()
{
    delete m_pScroll;
    g_NMEALogWindow = NULL;
}

void TTYWindow::CreateLegendBitmap()
{
    m_bm_legend.Create(400, 80);
    wxMemoryDC dc;
    dc.SelectObject( m_bm_legend );
    if( m_bm_legend.IsOk()) {
        
        dc.SetBackground( wxBrush(GetGlobalColor(_T("DILG1"))) );
        dc.Clear();
        
        wxFont f(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(f);
        
        int yp = 25;
        int y = 5;
        
        wxBrush b1(wxColour( _T("DARK GREEN")) );
        dc.SetBrush(b1);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("DARK GREEN")) );
        dc.DrawText(  _("Message accepted"), 30, y );
        
        y += yp;
        wxBrush b2(wxColour( _T("#a0832a")) );
        dc.SetBrush(b2);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("#a0832a")) );
        dc.DrawText(  _("Message filtered and dropped"), 30, y );
        
        y += yp;
        wxBrush b3(wxColour( _T("BLUE")) );
        dc.SetBrush(b3);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("BLUE")) );
        dc.DrawText(  _("Output Message"), 30, y );
        
        
        
    }
        
    dc.SelectObject( wxNullBitmap );
        
}




void TTYWindow::OnPauseClick( wxCommandEvent& event )
{
    if(!bpause) {
        bpause = true;
        m_pScroll->Pause( true );
        
        m_buttonPause->SetLabel( _("Resume") );
    }
    else {
        bpause = false;
        m_pScroll->Pause( false );
        
        m_buttonPause->SetLabel( _("Pause") );
    }
}


void TTYWindow::OnCloseWindow( wxCloseEvent& event )

{
    Destroy();
}

void TTYWindow::OnSize( wxSizeEvent& event )
{
    //    Record the dialog size
    wxSize p = event.GetSize();
    g_NMEALogWindow_sx = p.x;
    g_NMEALogWindow_sy = p.y;

    event.Skip();
}

void TTYWindow::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_NMEALogWindow_x = p.x;
    g_NMEALogWindow_y = p.y;

    event.Skip();
}

void TTYScroll::Add( wxString &line )
{
    if(!bpause) {
        if( m_plineArray->GetCount() > m_nLines - 1 ) {                       // shuffle the arraystring
            wxArrayString *p_newArray = new wxArrayString;

            for( unsigned int i = 1; i < m_plineArray->GetCount(); i++ ) 
                p_newArray->Add( m_plineArray->Item( i ) );

            delete m_plineArray;
            m_plineArray = p_newArray;
        }

    m_plineArray->Add( line );
    }
}

void TTYScroll::OnDraw( wxDC& dc )
{
    // update region is always in device coords, translate to logical ones
    wxRect rectUpdate = GetUpdateRegion().GetBox();
    CalcUnscrolledPosition( rectUpdate.x, rectUpdate.y, &rectUpdate.x, &rectUpdate.y );

    size_t lineFrom = rectUpdate.y / m_hLine, lineTo = rectUpdate.GetBottom() / m_hLine;

    if( lineTo > m_nLines - 1 ) lineTo = m_nLines - 1;

    wxCoord y = lineFrom * m_hLine;
    wxString lss;
    for( size_t line = lineFrom; line <= lineTo; line++ ) {
        wxCoord yPhys;
        CalcScrolledPosition( 0, y, NULL, &yPhys );

        wxString ls = m_plineArray->Item( line );
        if(ls.Mid(0, 7) == _T("<GREEN>") ){
            dc.SetTextForeground( wxColour(_T("DARK GREEN")) );
            lss = ls.Mid(7);
        }
        else if(ls.Mid(0, 7) == _T("<AMBER>") ){
            dc.SetTextForeground( wxColour(_T("#a0832a")) );
            lss = ls.Mid(7);
        }
        else if(ls.Mid(0, 6) == _T("<BLUE>") ){
                dc.SetTextForeground( wxColour(_T("BLUE")) );
                lss = ls.Mid(6);
        }
        
       dc.DrawText( lss, 0, y );
       y += m_hLine;
    }
}

void TTYWindow::Add( wxString &line )
{
    if( m_pScroll ) m_pScroll->Add( line );
}

