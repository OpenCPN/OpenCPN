/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Canvas
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include "wx/image.h"
#include <wx/graphics.h>
#include <wx/listbook.h>
#include <wx/clipbrd.h>
#include <wx/aui/aui.h>
#include "wx/progdlg.h"
#include "dychart.h"
#include "OCPNPlatform.h"

#ifdef __WXOSX__
#include "DarkMode.h"
#endif

#include <wx/listimpl.cpp>

#include "config.h" 
#include "chcanv.h"
#include "TCWin.h"
#include "geodesic.h"
#include "styles.h"
#include "routeman.h"
#include "piano.h"
#include "navutil.h"
#include "kml.h"
#include "concanv.h"
#include "thumbwin.h"
#include "chartdb.h"
#include "chartimg.h"
#include "chart1.h"
#include "cutil.h"
#include "MarkInfo.h"
#include "RoutePropDlgImpl.h"
#include "TrackPropDlg.h"
#include "tcmgr.h"
#include "routemanagerdialog.h"
#include "pluginmanager.h"
#include "ocpn_pixel.h"
#include "ocpndc.h"
#include "undo.h"
#include "toolbar.h"
#include "multiplexer.h"
#include "timers.h"
#include "tide_time.h"
#include "glTextureDescriptor.h"
#include "ChInfoWin.h"
#include "Quilt.h"
#include "SelectItem.h"
#include "Select.h"
#include "FontMgr.h"
#include "AIS_Decoder.h"
#include "AIS_Target_Data.h"
#include "AISTargetAlertDialog.h"
#include "SendToGpsDlg.h"
#include "compass.h"
#include "OCPNRegion.h"
#include "gshhs.h"
#include "canvasMenu.h"
#include "wx28compat.h"
#include "Track.h"
#include "Route.h"
#include "OCPN_AUIManager.h"
#include "MUIBar.h"
#include "CanvasConfig.h"
#include "CanvasOptions.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#ifdef USE_S57
#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#include "s52utils.h"
#endif

#include "ais.h"

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>


extern struct sigaction sa_all;
extern struct sigaction sa_all_old;

extern sigjmp_buf           env;                    // the context saved by sigsetjmp();
#endif

extern float  g_ChartScaleFactorExp;
extern float  g_ShipScaleFactorExp;

#include <vector>
//#include <wx-3.0/wx/aui/auibar.h>

#if defined(__MSVC__) &&  (_MSC_VER < 1700) 
#define  trunc(d) ((d>0) ? floor(d) : ceil(d))
#endif

//  Define to enable the invocation of a temporary menubar by pressing the Alt key.
//  Not implemented for Windows XP, as it interferes with Alt-Tab processing.
#define OCPN_ALT_MENUBAR 1


//    Profiling support
//#include "/usr/include/valgrind/callgrind.h"

// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------
extern bool G_FloatPtInPolygon ( MyFlPoint *rgpts, int wnumpts, float x, float y ) ;
extern void catch_signals(int signo);

extern void AlphaBlending( ocpnDC& dc, int x, int y, int size_x, int size_y, float radius,
                                       wxColour color, unsigned char transparency );

extern ChartBase        *Current_Vector_Ch;
extern double           g_ChartNotRenderScaleFactor;
extern double           gLat, gLon, gCog, gSog, gHdt;
//extern double           vLat, vLon;
extern ChartDB          *ChartData;
extern bool             bDBUpdateInProgress;
extern ColorScheme      global_color_scheme;
extern bool             g_bHDTValid;
extern int              g_nbrightness;

extern ConsoleCanvas    *console;
extern OCPNPlatform     *g_Platform;

extern RouteList        *pRouteList;
extern TrackList        *pTrackList;
extern MyConfig         *pConfig;
extern Select           *pSelect;
extern Routeman         *g_pRouteMan;
extern ThumbWin         *pthumbwin;
extern TCMgr            *ptcmgr;
extern Select           *pSelectTC;
extern Select           *pSelectAIS;
extern WayPointman      *pWayPointMan;
extern MarkInfoDlg      *g_pMarkInfoDialog;
extern RoutePropDlgImpl *pRoutePropDialog;
extern TrackPropDlg     *pTrackPropDialog;
extern ActiveTrack      *g_pActiveTrack;
extern bool             g_bConfirmObjectDelete;

extern IDX_entry        *gpIDX;
extern int               gpIDXn;

extern RoutePoint       *pAnchorWatchPoint1;
extern RoutePoint       *pAnchorWatchPoint2;
extern double           AnchorPointMinDist;
extern bool             AnchorAlertOn1;
extern bool             AnchorAlertOn2;
extern wxString         g_AW1GUID;
extern wxString         g_AW2GUID;
extern int              g_nAWDefault;
extern int              g_nAWMax;
extern int              g_iDistanceFormat;

extern RouteManagerDialog *pRouteManagerDialog;
extern GoToPositionDialog *pGoToPositionDialog;
extern wxString GetLayerName(int id);
extern wxString         g_uploadConnection;
extern bool             g_bsimplifiedScalebar;

extern bool             bDrawCurrentValues;

#ifdef USE_S57
extern s52plib          *ps52plib;
extern CM93OffsetDialog  *g_pCM93OffsetDialog;
#endif

extern bool             bGPSValid;
//extern bool             g_bShowOutlines;
//extern bool             g_bShowDepthUnits;
extern bool             g_bTempShowMenuBar;
extern bool             g_bShowMenuBar;
extern bool             g_bShowCompassWin;

extern AIS_Decoder      *g_pAIS;
extern bool             g_bShowAreaNotices;
extern int              g_Show_Target_Name_Scale;

extern MyFrame          *gFrame;

extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern bool             g_bWayPointPreventDragging;
extern bool             g_bEnableZoomToCursor;
extern bool             g_bShowChartBar;
extern bool             g_bInlandEcdis;

extern bool             g_bDarkDecorations;

extern AISTargetAlertDialog    *g_pais_alert_dialog_active;
extern AISTargetQueryDialog    *g_pais_query_dialog_active;
extern int              g_ais_query_dialog_x, g_ais_query_dialog_y;

extern int              g_S57_dialog_sx, g_S57_dialog_sy;

extern PopUpDSlide       *pPopupDetailSlider;
extern bool             g_bShowDetailSlider;
extern int              g_detailslider_dialog_x, g_detailslider_dialog_y;
extern int              g_cm93_zoom_factor;

extern bool             g_b_overzoom_x;                      // Allow high overzoom
//extern bool             g_bDisplayGrid;

extern bool             g_bUseGreenShip;

extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern int              g_n_ownship_min_mm;

extern bool             g_bUseRaster;
extern bool             g_bUseVector;
extern bool             g_bUseCM93;

extern double           g_COGAvg;               // only needed for debug....

extern int              g_click_stop;
extern double           g_ownship_predictor_minutes;
extern double           g_ownship_HDTpredictor_miles;

extern std::vector<int>      g_quilt_noshow_index_array;
extern bool              g_bquiting;
extern AISTargetListDialog *g_pAISTargetList;
extern wxString         g_sAIS_Alert_Sound_File;

extern PlugInManager    *g_pi_manager;

extern OCPN_AUIManager  *g_pauimgr;

extern bool             g_bopengl;
extern bool             g_bdisable_opengl;

extern bool             g_bFullScreenQuilt;

extern bool             g_bsmoothpanzoom;

bool                    g_bDebugOGL;

extern bool             g_b_assume_azerty;

extern ChartGroupArray  *g_pGroupArray;
extern wxString         g_default_wp_icon;
extern wxString         g_default_routepoint_icon;

extern int              g_current_arrow_scale;
extern int              g_tide_rectangle_scale;

extern S57QueryDialog   *g_pObjectQueryDialog;
extern ocpnStyle::StyleManager* g_StyleManager;
extern Multiplexer      *g_pMUX;
extern wxArrayOfConnPrm *g_pConnectionParams;

extern OcpnSound*        g_anchorwatch_sound;

extern bool              g_bShowTrue, g_bShowMag;
extern bool              g_btouch;
extern bool              g_bresponsive;

extern wxString         g_toolbarConfigSecondary;

#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;
#endif

extern bool              g_bShowFPS;
extern double            g_gl_ms_per_frame;
extern bool              g_benable_rotate;

extern bool              g_bSpaceDropMark;
extern bool              g_bAutoHideToolbar;
extern int               g_nAutoHideToolbar;
extern bool              g_bDeferredInitDone;

//  TODO why are these static?
static int mouse_x;
static int mouse_y;
static bool mouse_leftisdown;

bool g_brouteCreating;

int r_gamma_mult;
int g_gamma_mult;
int b_gamma_mult;
int gamma_state;
bool g_brightness_init;
int   last_brightness;

int                     g_cog_predictor_width;
extern double           g_display_size_mm;

//extern bool             g_bshowToolbar;
extern ocpnFloatingToolbarDialog *g_MainToolbar;
extern wxColour         g_colourOwnshipRangeRingsColour;

// LIVE ETA OPTION
bool                    g_bShowLiveETA;
double                  g_defaultBoatSpeed;
double                  g_defaultBoatSpeedUserUnit;

extern int              g_nAIS_activity_timer;
extern bool             g_bskew_comp;
extern float            g_compass_scalefactor;
extern int              g_COGAvgSec; // COG average period (sec.) for Course Up Mode

wxGLContext             *g_pGLcontext;   //shared common context

extern bool             g_useMUI;
extern unsigned int     g_canvasConfig;
extern wxString         g_lastPluginMessage;

extern ChartCanvas      *g_focusCanvas;
extern ChartCanvas      *g_overlayCanvas;

extern float            g_toolbar_scalefactor;

// "Curtain" mode parameters
wxDialog                *g_pcurtain;

#define MIN_BRIGHT 10
#define MAX_BRIGHT 100


//------------------------------------------------------------------------------
//    ChartCanvas Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE ( ChartCanvas, wxWindow )
    EVT_PAINT ( ChartCanvas::OnPaint )
    EVT_ACTIVATE ( ChartCanvas::OnActivate )
    EVT_SIZE ( ChartCanvas::OnSize )
    EVT_MOUSE_EVENTS ( ChartCanvas::MouseEvent )
    EVT_TIMER ( DBLCLICK_TIMER, ChartCanvas::MouseTimedEvent )
    EVT_TIMER ( PAN_TIMER, ChartCanvas::PanTimerEvent )
    EVT_TIMER ( MOVEMENT_TIMER, ChartCanvas::MovementTimerEvent )
    EVT_TIMER ( MOVEMENT_STOP_TIMER, ChartCanvas::MovementStopTimerEvent )
    EVT_TIMER ( CURTRACK_TIMER, ChartCanvas::OnCursorTrackTimerEvent )
    EVT_TIMER ( ROT_TIMER, ChartCanvas::RotateTimerEvent )
    EVT_TIMER ( ROPOPUP_TIMER, ChartCanvas::OnRolloverPopupTimerEvent )
    EVT_TIMER ( ROUTEFINISH_TIMER, ChartCanvas::OnRouteFinishTimerEvent )
    EVT_KEY_DOWN(ChartCanvas::OnKeyDown )
    EVT_KEY_UP(ChartCanvas::OnKeyUp )
    EVT_CHAR(ChartCanvas::OnKeyChar)
    EVT_MOUSE_CAPTURE_LOST(ChartCanvas::LostMouseCapture )
    EVT_KILL_FOCUS(ChartCanvas::OnKillFocus)
    EVT_SET_FOCUS(ChartCanvas::OnSetFocus)
    EVT_MENU(-1, ChartCanvas::OnToolLeftClick)
    EVT_TIMER ( DEFERRED_FOCUS_TIMER, ChartCanvas::OnDeferredFocusTimerEvent )
    
    
END_EVENT_TABLE()

// Define a constructor for my canvas
ChartCanvas::ChartCanvas ( wxFrame *frame, int canvasIndex ) :
     wxWindow ( frame, wxID_ANY,    wxPoint ( 20,20 ), wxSize ( 5,5 ), wxNO_BORDER )
{
    parent_frame = ( MyFrame * ) frame;       // save a pointer to parent
    m_canvasIndex = canvasIndex;
    
    pscratch_bm = NULL;

    SetBackgroundColour ( GetGlobalColor ( _T ( "NODTA" ) ) );
    SetBackgroundStyle ( wxBG_STYLE_CUSTOM );  // on WXMSW, this prevents flashing on color scheme change

    m_groupIndex = 0;
    m_bDrawingRoute = false;
    m_bRouteEditing = false;
    m_bMarkEditing = false;
	m_bRoutePoinDragging = false;
    m_bIsInRadius = false;
    m_bMayToggleMenuBar = true;

    m_bFollow = false;
    m_bShowNavobjects = true;
    m_bTCupdate = false;
    m_bAppendingRoute = false;          // was true in MSW, why??
    pThumbDIBShow = NULL;
    m_bShowCurrent = false;
    m_bShowTide = false;
    bShowingCurrent = false;
    pCwin = NULL;
    warp_flag = false;
    m_bzooming = false;
    m_b_paint_enable = true;
    m_routeState = 0;
    
    pss_overlay_bmp = NULL;
    pss_overlay_mask = NULL;
    m_bChartDragging = false;
    m_bMeasure_Active = false;
    m_bMeasure_DistCircle = false;
    m_pMeasureRoute = NULL;
    m_pTrackRolloverWin = NULL;
    m_pRouteRolloverWin = NULL;
    m_pAISRolloverWin = NULL;
    m_bedge_pan = false;
    m_disable_edge_pan = false;
    m_dragoffsetSet = false;
    m_bautofind = false;
    m_bFirstAuto = true;
    m_groupIndex = 0;
    m_singleChart = NULL;
    m_bCourseUp = false;
    m_bShowAIS = true;
    m_bShowAISScaled = false;
    
    m_vLat = 0.;
    m_vLon = 0.;
    
    m_pCIWin = NULL;

    m_pSelectedRoute              = NULL;
    m_pSelectedTrack              = NULL;
    m_pRoutePointEditTarget       = NULL;
    m_pFoundPoint                 = NULL;
    m_pMouseRoute                 = NULL;
    m_prev_pMousePoint            = NULL;
    m_pEditRouteArray             = NULL;
    m_pFoundRoutePoint            = NULL;
    m_FinishRouteOnKillFocus = true;


    m_pRolloverRouteSeg           = NULL;
    m_pRolloverTrackSeg           = NULL;
    m_bsectors_shown              = false;
    
    m_bbrightdir = false;
    r_gamma_mult = 1;
    g_gamma_mult = 1;
    b_gamma_mult = 1;


    m_pos_image_user_day        = NULL;
    m_pos_image_user_dusk       = NULL;
    m_pos_image_user_night      = NULL;
    m_pos_image_user_grey_day   = NULL;
    m_pos_image_user_grey_dusk  = NULL;
    m_pos_image_user_grey_night = NULL;

    m_zoom_factor = 1;
    m_rotation_speed = 0;
    m_mustmove = 0;

    m_OSoffsetx = 0.;
    m_OSoffsety = 0.;

    m_pos_image_user_yellow_day = NULL;
    m_pos_image_user_yellow_dusk = NULL;
    m_pos_image_user_yellow_night = NULL;

    SetOwnShipState( SHIP_INVALID );
    
    undo = new Undo(this);

    VPoint.Invalidate();

    m_glcc = NULL;
    
    m_toolBar = NULL;
    m_toolbar_scalefactor = 1.0;
    m_toolbarOrientation = wxTB_HORIZONTAL;
    m_focus_indicator_pix = 1;
   
    m_pCurrentStack = NULL;
    m_bpersistent_quilt = false;
    m_piano_ctx_menu = NULL;
    m_Compass = NULL;
    
    g_ChartNotRenderScaleFactor = 2.0;
    m_bShowScaleInStatusBar = true;
    
    m_muiBar = NULL;
    m_bShowScaleInStatusBar = false;

    m_bShowOutlines = false;
    m_bDisplayGrid = false;
    m_bShowDepthUnits = true;
    m_encDisplayCategory = (int)STANDARD;

    m_encShowLights = true;
    m_encShowAnchor = true;
    m_encShowDataQual = false;
    m_bShowGPS = true;
    SetQuiltMode(true);
 
    SetupGlCanvas( );
/*
#ifdef ocpnUSE_GL
    if ( !g_bdisable_opengl )
    {
        if(g_bopengl){
            wxLogMessage( _T("Creating glChartCanvas") );
            m_glcc = new glChartCanvas(this);

        // We use one context for all GL windows, so that textures etc will be automatically shared
            if(IsPrimaryCanvas()){
                wxGLContext *pctx = new wxGLContext(m_glcc);
                m_glcc->SetContext(pctx);
                g_pGLcontext = pctx;                // Save a copy of the common context
            }
            else{
                m_glcc->SetContext(g_pGLcontext);   // If not primary canvas, use the saved common context
            }
        }
    }
#endif
*/
    singleClickEventIsValid = false;

//    Build the cursors

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

#if !defined(__WXMSW__) && !defined(__WXQT__)

    wxImage ICursorLeft = style->GetIcon( _T("left") ).ConvertToImage();
    wxImage ICursorRight = style->GetIcon( _T("right") ).ConvertToImage();
    wxImage ICursorUp = style->GetIcon( _T("up") ).ConvertToImage();
    wxImage ICursorDown = style->GetIcon( _T("down") ).ConvertToImage();
    wxImage ICursorPencil = style->GetIcon( _T("pencil") ).ConvertToImage();
    wxImage ICursorCross = style->GetIcon( _T("cross") ).ConvertToImage();

//#if wxCHECK_VERSION(2, 8, 12)
//#else
    ICursorLeft.ConvertAlphaToMask(128);
    ICursorRight.ConvertAlphaToMask(128);
    ICursorUp.ConvertAlphaToMask(128);
    ICursorDown.ConvertAlphaToMask(128);
    ICursorPencil.ConvertAlphaToMask(10);
    ICursorCross.ConvertAlphaToMask(10);
//#endif

    if ( ICursorLeft.Ok() )
    {
        ICursorLeft.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 0 );
        ICursorLeft.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorLeft = new wxCursor ( ICursorLeft );
    }
    else
        pCursorLeft = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorRight.Ok() )
    {
        ICursorRight.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 31 );
        ICursorRight.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorRight = new wxCursor ( ICursorRight );
    }
    else
        pCursorRight = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorUp.Ok() )
    {
        ICursorUp.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 15 );
        ICursorUp.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0 );
        pCursorUp = new wxCursor ( ICursorUp );
    }
    else
        pCursorUp = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorDown.Ok() )
    {
        ICursorDown.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 15 );
        ICursorDown.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 31 );
        pCursorDown = new wxCursor ( ICursorDown );
    }
    else
        pCursorDown = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorPencil.Ok() )
    {
        ICursorPencil.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 0 );
        ICursorPencil.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
        pCursorPencil = new wxCursor ( ICursorPencil );
    }
    else
        pCursorPencil = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorCross.Ok() )
    {
        ICursorCross.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 13 );
        ICursorCross.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 12);
        pCursorCross = new wxCursor ( ICursorCross );
    }
    else
        pCursorCross = new wxCursor ( wxCURSOR_ARROW );

#else

    wxImage ICursorLeft = style->GetIcon( _T("left") ).ConvertToImage();
    wxImage ICursorRight = style->GetIcon( _T("right") ).ConvertToImage();
    wxImage ICursorUp = style->GetIcon( _T("up") ).ConvertToImage();
    wxImage ICursorDown = style->GetIcon( _T("down") ).ConvertToImage();
    wxImage ICursorPencil = style->GetIcon( _T("pencil") ).ConvertToImage();
    wxImage ICursorCross = style->GetIcon( _T("cross") ).ConvertToImage();

    if( ICursorLeft.Ok() ) {
        ICursorLeft.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 0 );
        ICursorLeft.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorLeft = new wxCursor( ICursorLeft );
    } else
        pCursorLeft = new wxCursor( wxCURSOR_ARROW );

    if( ICursorRight.Ok() ) {
        ICursorRight.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 31 );
        ICursorRight.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorRight = new wxCursor( ICursorRight );
    } else
        pCursorRight = new wxCursor( wxCURSOR_ARROW );

    if( ICursorUp.Ok() ) {
        ICursorUp.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 15 );
        ICursorUp.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0 );
        pCursorUp = new wxCursor( ICursorUp );
    } else
        pCursorUp = new wxCursor( wxCURSOR_ARROW );

    if( ICursorDown.Ok() ) {
        ICursorDown.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 15 );
        ICursorDown.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 31 );
        pCursorDown = new wxCursor( ICursorDown );
    } else
        pCursorDown = new wxCursor( wxCURSOR_ARROW );

    if( ICursorPencil.Ok() ) {
        ICursorPencil.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 0 );
        ICursorPencil.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorPencil = new wxCursor( ICursorPencil );
    } else
        pCursorPencil = new wxCursor( wxCURSOR_ARROW );

    if( ICursorCross.Ok() ) {
        ICursorCross.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 13 );
        ICursorCross.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 12 );
        pCursorCross = new wxCursor( ICursorCross );
    } else
        pCursorCross = new wxCursor( wxCURSOR_ARROW );

#endif      // MSW, X11
    pCursorArrow = new wxCursor( wxCURSOR_ARROW );
    pPlugIn_Cursor = NULL;

    SetCursor( *pCursorArrow );

    pPanTimer = new wxTimer( this, PAN_TIMER );
    pPanTimer->Stop();

    pMovementTimer = new wxTimer( this, MOVEMENT_TIMER );
    pMovementTimer->Stop();

    pMovementStopTimer = new wxTimer( this, MOVEMENT_STOP_TIMER );
    pMovementStopTimer->Stop();

    pRotDefTimer = new wxTimer( this, ROT_TIMER );
    pRotDefTimer->Stop();

    m_DoubleClickTimer = new wxTimer( this, DBLCLICK_TIMER );
    m_DoubleClickTimer->Stop();

    m_panx = m_pany = 0;
    m_panspeed = 0;

    pCurTrackTimer = new wxTimer( this, CURTRACK_TIMER );
    pCurTrackTimer->Stop();
    m_curtrack_timer_msec = 10;

    m_wheelzoom_stop_oneshot = 0;
    m_last_wheel_dir = 0;
    
    m_RolloverPopupTimer.SetOwner( this, ROPOPUP_TIMER );

    m_deferredFocusTimer.SetOwner( this, DEFERRED_FOCUS_TIMER );
    
    m_rollover_popup_timer_msec = 20;

    m_routeFinishTimer.SetOwner( this, ROUTEFINISH_TIMER );
    
    m_b_rot_hidef = true;
    
    proute_bm = NULL;
    m_prot_bm = NULL;

    m_bCourseUp = false;
    m_bLookAhead = false;
    
// Set some benign initial values

    m_cs = GLOBAL_COLOR_SCHEME_DAY;
    VPoint.clat = 0;
    VPoint.clon = 0;
    VPoint.view_scale_ppm = 1;
    VPoint.Invalidate();

    m_canvas_scale_factor = 1.;

    m_canvas_width = 1000;

    m_overzoomTextWidth = 0;
    m_overzoomTextHeight = 0;

//    Create the default world chart
    pWorldBackgroundChart = new GSHHSChart;

//    Create the default depth unit emboss maps
    m_pEM_Feet = NULL;
    m_pEM_Meters = NULL;
    m_pEM_Fathoms = NULL;

    CreateDepthUnitEmbossMaps( GLOBAL_COLOR_SCHEME_DAY );

    m_pEM_OverZoom = NULL;
    SetOverzoomFont();
    CreateOZEmbossMapData( GLOBAL_COLOR_SCHEME_DAY );

//    Build icons for tide/current points
    m_bmTideDay = style->GetIcon( _T("tidesml") );

//    Dusk
    m_bmTideDusk = CreateDimBitmap( m_bmTideDay, .50 );

//    Night
    m_bmTideNight = CreateDimBitmap( m_bmTideDay, .20 );

//    Build Dusk/Night  ownship icons
    double factor_dusk = 0.5;
    double factor_night = 0.25;

    //Red
    m_os_image_red_day = style->GetIcon( _T("ship-red") ).ConvertToImage();

    int rimg_width = m_os_image_red_day.GetWidth();
    int rimg_height = m_os_image_red_day.GetHeight();

    m_os_image_red_dusk = m_os_image_red_day.Copy();
    m_os_image_red_night = m_os_image_red_day.Copy();

    for( int iy = 0; iy < rimg_height; iy++ ) {
        for( int ix = 0; ix < rimg_width; ix++ ) {
            if( !m_os_image_red_day.IsTransparent( ix, iy ) ) {
                wxImage::RGBValue rgb( m_os_image_red_day.GetRed( ix, iy ),
                                       m_os_image_red_day.GetGreen( ix, iy ),
                                       m_os_image_red_day.GetBlue( ix, iy ) );
                wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_dusk;
                wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_red_dusk.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );

                hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_night;
                nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_red_night.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
            }
        }
    }

    //Grey
    m_os_image_grey_day = style->GetIcon( _T("ship-red") ).ConvertToImage().ConvertToGreyscale();

    int gimg_width = m_os_image_grey_day.GetWidth();
    int gimg_height = m_os_image_grey_day.GetHeight();

    m_os_image_grey_dusk = m_os_image_grey_day.Copy();
    m_os_image_grey_night = m_os_image_grey_day.Copy();

    for( int iy = 0; iy < gimg_height; iy++ ) {
        for( int ix = 0; ix < gimg_width; ix++ ) {
            if( !m_os_image_grey_day.IsTransparent( ix, iy ) ) {
                wxImage::RGBValue rgb( m_os_image_grey_day.GetRed( ix, iy ),
                                       m_os_image_grey_day.GetGreen( ix, iy ),
                                       m_os_image_grey_day.GetBlue( ix, iy ) );
                wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_dusk;
                wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_grey_dusk.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );

                hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_night;
                nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_grey_night.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
            }
        }
    }


    // Yellow
    m_os_image_yellow_day = m_os_image_red_day.Copy();

    gimg_width = m_os_image_yellow_day.GetWidth();
    gimg_height = m_os_image_yellow_day.GetHeight();

    m_os_image_yellow_dusk = m_os_image_red_day.Copy();
    m_os_image_yellow_night = m_os_image_red_day.Copy();

    for( int iy = 0; iy < gimg_height; iy++ ) {
        for( int ix = 0; ix < gimg_width; ix++ ) {
            if( !m_os_image_yellow_day.IsTransparent( ix, iy ) ) {
                wxImage::RGBValue rgb( m_os_image_yellow_day.GetRed( ix, iy ),
                                       m_os_image_yellow_day.GetGreen( ix, iy ),
                                       m_os_image_yellow_day.GetBlue( ix, iy ) );
                wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                hsv.hue += 60./360.;             //shift to yellow
                wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_yellow_day.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );

                hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_dusk;
                hsv.hue += 60./360.;             // shift to yellow
                nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_yellow_dusk.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );

                hsv = wxImage::RGBtoHSV( rgb );
                hsv.hue += 60./360.;             //shift to yellow
                hsv.value = hsv.value * factor_night;
                nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_yellow_night.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
            }
        }
    }


    //  Set initial pointers to ownship images
    m_pos_image_red = &m_os_image_red_day;
    m_pos_image_yellow = &m_os_image_yellow_day;
    m_pos_image_grey = &m_os_image_grey_day;
    
    SetUserOwnship();
        
    m_pBrightPopup = NULL;
    m_pQuilt = new Quilt( this );
    
#ifdef ocpnUSE_GL
    if ( !g_bdisable_opengl )
        m_pQuilt->EnableHighDefinitionZoom( true );
#endif    

    m_pgridFont = FontMgr::Get().FindOrCreateFont( 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
         wxFONTWEIGHT_NORMAL, FALSE, wxString( _T ( "Arial" ) ) );
    
    m_Piano = new Piano(this);

    m_Compass = new ocpnCompass(this);
    m_Compass->Show(g_bShowCompassWin);

    m_bToolbarEnable = false;
}

ChartCanvas::~ChartCanvas()
{

    delete pThumbDIBShow;

//    Delete Cursors
    delete pCursorLeft;
    delete pCursorRight;
    delete pCursorUp;
    delete pCursorDown;
    delete pCursorArrow;
    delete pCursorPencil;
    delete pCursorCross;

    delete pPanTimer;
    delete pMovementTimer;
    delete pMovementStopTimer;
    delete pCurTrackTimer;
    delete pRotDefTimer;
    delete m_DoubleClickTimer;

    delete m_pTrackRolloverWin;
    delete m_pRouteRolloverWin;
    delete m_pAISRolloverWin;
    delete m_pBrightPopup;

    delete m_pCIWin;

    delete pscratch_bm;

    m_dc_route.SelectObject( wxNullBitmap );
    delete proute_bm;

    delete pWorldBackgroundChart;
    delete pss_overlay_bmp;

    delete m_pEM_Feet;
    delete m_pEM_Meters;
    delete m_pEM_Fathoms;

    delete m_pEM_OverZoom;
//        delete m_pEM_CM93Offset;

    delete m_pQuilt;

    delete m_prot_bm;

    delete m_pos_image_user_day;
    delete m_pos_image_user_dusk;
    delete m_pos_image_user_night;
    delete m_pos_image_user_grey_day;
    delete m_pos_image_user_grey_dusk;
    delete m_pos_image_user_grey_night;
    delete m_pos_image_user_yellow_day;
    delete m_pos_image_user_yellow_dusk;
    delete m_pos_image_user_yellow_night;

    delete undo;
#ifdef ocpnUSE_GL
    if( !g_bdisable_opengl ) {
        delete m_glcc;
        
#if wxCHECK_VERSION(2, 9, 0)
        if(IsPrimaryCanvas() && g_bopengl)
            delete g_pGLcontext;
#endif        
    }
#endif
    
    // Delete the MUI bar, but make sure there is no pointer to it during destroy.
    // wx tries to deliver events to this canvas during destroy.
    MUIBar *muiBar = m_muiBar;
    m_muiBar = 0;
    delete muiBar;
}

void ChartCanvas::SetupGlCanvas( )
{
#ifdef ocpnUSE_GL
    if ( !g_bdisable_opengl )
    {
        if(g_bopengl){
            wxLogMessage( _T("Creating glChartCanvas") );
            m_glcc = new glChartCanvas(this);

        // We use one context for all GL windows, so that textures etc will be automatically shared
            if(IsPrimaryCanvas()){
                wxGLContext *pctx = new wxGLContext(m_glcc);
                m_glcc->SetContext(pctx);
                g_pGLcontext = pctx;                // Save a copy of the common context
            }
            else{
                m_glcc->SetContext(g_pGLcontext);   // If not primary canvas, use the saved common context
            }
        }
    }
#endif
}

void ChartCanvas::OnKillFocus( wxFocusEvent& WXUNUSED(event) )
{
    RefreshRect( wxRect(0, 0, GetClientSize().x, m_focus_indicator_pix ) );
    
    // Special logic:
    //  On OSX in GL mode, each mouse click causes a kill and immediate regain of canvas focus.  Why???  Who knows...
    //  So, we provide for this case by starting a timer if required to actually Finish() a route on a legitimate
    //  focus change, but not if the focus is quickly regained ( <20 msec.) on this canvas.
#ifdef __WXOSX__
    if(m_routeState && m_FinishRouteOnKillFocus)
        m_routeFinishTimer.Start(20, wxTIMER_ONE_SHOT);
#else
    if(m_routeState && m_FinishRouteOnKillFocus)
        FinishRoute();
#endif
}

void ChartCanvas::OnSetFocus( wxFocusEvent& WXUNUSED(event) )
{
    m_routeFinishTimer.Stop();
    
    // Try to keep the global top-line menubar selections up to date with the current "focus" canvas
    gFrame->UpdateGlobalMenuItems( this );

    RefreshRect( wxRect(0, 0, GetClientSize().x, m_focus_indicator_pix ) );
}


void ChartCanvas::OnRouteFinishTimerEvent( wxTimerEvent& event )
{
    if(m_routeState && m_FinishRouteOnKillFocus)
        FinishRoute();
}


void ChartCanvas::ApplyCanvasConfig(canvasConfig *pcc)
{
    SetViewPoint( pcc->iLat, pcc->iLon, pcc->iScale, 0., pcc->iRotation );
    m_vLat = pcc->iLat;
    m_vLon = pcc->iLon;

    m_restore_dbindex = pcc->DBindex;
    m_bFollow = pcc->bFollow;
    if ( pcc->GroupID < 0 )
        pcc->GroupID = 0;

    if( pcc->GroupID > (int) g_pGroupArray->GetCount() )
        m_groupIndex = 0;
    else
        m_groupIndex = pcc->GroupID;
    
    if( pcc->bQuilt != GetQuiltMode() )
        ToggleCanvasQuiltMode();
        
    ShowTides(pcc->bShowTides);
    ShowCurrents(pcc->bShowCurrents);
    
    SetShowDepthUnits( pcc->bShowDepthUnits );
    SetShowGrid( pcc->bShowGrid );
    SetShowOutlines( pcc->bShowOutlines );

    SetShowAIS(pcc->bShowAIS);
    SetAttenAIS(pcc->bAttenAIS);
    
    // ENC options
    SetShowENCText( pcc->bShowENCText );
    m_encDisplayCategory = pcc->nENCDisplayCategory;
    m_encShowDepth = pcc->bShowENCDepths;
    m_encShowLightDesc = pcc->bShowENCLightDescriptions;
    m_encShowBuoyLabels = pcc->bShowENCBuoyLabels;
    m_encShowLights = pcc->bShowENCLights;
    
    m_bCourseUp = pcc->bCourseUp;
    m_bLookAhead = pcc->bLookahead;
    
    m_singleChart = NULL;

}

void ChartCanvas::CheckGroupValid( bool showMessage, bool switchGroup0)
{
    bool groupOK = CheckGroup( m_groupIndex );
    
    if(!groupOK){
        SetGroupIndex( m_groupIndex, true );
    }

}

void ChartCanvas::SetShowGPS( bool bshow )
{
    if(m_bShowGPS != bshow){
        delete m_Compass;
        m_Compass = new ocpnCompass( this, bshow );
        m_Compass->Show(g_bShowCompassWin);
    }
    m_bShowGPS = bshow;
    
}

void ChartCanvas::SetToolbarEnable( bool bShow )
{
    return;
    
    //if(GetToolbarEnable() != bShow)
    {
        m_bToolbarEnable = bShow;
//         if(!m_toolBar)
//             RequestNewCanvasToolbar( true );
        if(GetToolbar())
            GetToolbar()->Show( bShow );
    }
}

int ChartCanvas::GetPianoHeight()
{
    int height = 0;
    if(g_bShowChartBar && GetPiano())
        height = m_Piano->GetHeight();
    
    return height;
}
    
void ChartCanvas::ConfigureChartBar()
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    m_Piano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
    m_Piano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );
    
    if( GetQuiltMode() ) {
        m_Piano->SetRoundedRectangles( true );
    }
    m_Piano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
    m_Piano->SetPolyIcon( new wxBitmap( style->GetIcon( _T("polyprj") ) ) );
    m_Piano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );
    
}

void ChartCanvas::ShowTides(bool bShow)
{
    gFrame->LoadHarmonics();
    
    if( ptcmgr->IsReady() ) {
        SetbShowTide( bShow );
        if( m_toolBar )
            m_toolBar->GetToolbar()->ToggleTool( ID_TIDE, bShow );
        wxString tip = _("Show Tides");
        if(bShow)
            tip = _("Hide Tides");
        if( m_toolBar )
            m_toolBar->SetToolShortHelp( ID_TIDE, tip );
        
        parent_frame->SetMenubarItemState( ID_MENU_SHOW_TIDES, bShow );
    } else {
        wxLogMessage( _T("Chart1::Event...TCMgr Not Available") );
        SetbShowTide( false );
        if( m_toolBar )
            m_toolBar->GetToolbar()->ToggleTool( ID_TIDE, false );
        parent_frame->SetMenubarItemState( ID_MENU_SHOW_TIDES, false );
    }
    
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();


    // TODO
//     if( GetbShowTide() ) {
//         FrameTCTimer.Start( TIMER_TC_VALUE_SECONDS * 1000, wxTIMER_CONTINUOUS );
//         SetbTCUpdate( true );                        // force immediate update
//     } else
//         FrameTCTimer.Stop();
    
}

void ChartCanvas::ShowCurrents(bool bShow)
{
    gFrame->LoadHarmonics();
    
    if( ptcmgr->IsReady() ) {
        SetbShowCurrent( bShow );
        if( m_toolBar )
            m_toolBar->GetToolbar()->ToggleTool( ID_CURRENT, bShow );
        wxString tip = _("Show Currents");
        if(bShow)
            tip = _("Hide Currents");
        if( m_toolBar )
            m_toolBar->SetToolShortHelp( ID_CURRENT, tip );
        
        parent_frame->SetMenubarItemState( ID_MENU_SHOW_CURRENTS, bShow );
    } else {
        wxLogMessage( _T("Chart1::Event...TCMgr Not Available") );
        SetbShowCurrent( false );
        if( m_toolBar )
            m_toolBar->GetToolbar()->ToggleTool( ID_CURRENT, false );
        parent_frame->SetMenubarItemState( ID_MENU_SHOW_CURRENTS, false );
    }
    
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    
    // TODO
//     if( GetbShowCurrent() ) {
//         FrameTCTimer.Start( TIMER_TC_VALUE_SECONDS * 1000, wxTIMER_CONTINUOUS );
//         SetbTCUpdate( true );                        // force immediate update
//     } else
//         FrameTCTimer.Stop();
    
}


//TODO
//extern bool     g_bLookAhead;
extern bool     g_bPreserveScaleOnX;
extern ChartDummy *pDummyChart;
extern int      g_sticky_chart;

void ChartCanvas::canvasRefreshGroupIndex( void )
{
    SetGroupIndex(m_groupIndex);
}

void ChartCanvas::SetGroupIndex( int index, bool autoSwitch )
{
    int new_index = index;
    if( index > (int) g_pGroupArray->GetCount() )
        new_index = 0;
    
    bool bgroup_override = false;
    int old_group_index = new_index;
    
    if( !CheckGroup( new_index ) ) {
        new_index = 0;
        bgroup_override = true;
    }
    
    if(!autoSwitch && ( index <= (int) g_pGroupArray->GetCount()))
        new_index = index;
    
    //    Get the currently displayed chart native scale, and the current ViewPort
    int current_chart_native_scale = GetCanvasChartNativeScale();
    ViewPort vp = GetVP();
    
    m_groupIndex = new_index;

    // Are there  ENCs in this group
    if(ChartData)
        m_bENCGroup = ChartData->IsENCInGroup( m_groupIndex );
    
    //  Update the MUIBar for ENC availability
    if(m_muiBar)
        m_muiBar->SetCanvasENCAvailable(m_bENCGroup);
    
    //  Invalidate the "sticky" chart on group change, since it might not be in the new group
    g_sticky_chart = -1;
    
    //    We need a chartstack and quilt to figure out which chart to open in the new group
    UpdateCanvasOnGroupChange();

    int dbi_now = -1;
    if(GetQuiltMode())
        dbi_now = GetQuiltReferenceChartIndex();
    
    int dbi_hint = FindClosestCanvasChartdbIndex( current_chart_native_scale );
    
    // If a new reference chart is indicated, set a good scale for it.
    if((dbi_now != dbi_hint) || !GetQuiltMode()){
        double best_scale = GetBestStartScale(dbi_hint, vp);
        SetVPScale( best_scale );
    }
    
    if(GetQuiltMode())
        dbi_hint = GetQuiltReferenceChartIndex();
    
    //    Refresh the canvas, selecting the "best" chart,
    //    applying the prior ViewPort exactly
    canvasChartsRefresh( dbi_hint );
        
    if(!autoSwitch && bgroup_override){
        // show a short timed message box
        wxString msg( _("Group \"") );

        ChartGroup *pGroup = g_pGroupArray->Item( new_index - 1 );
        msg += pGroup->m_group_name;
            
        msg += _("\" is empty.");
            
        OCPNMessageBox( this, msg, _("OpenCPN Group Notice"), wxICON_INFORMATION, 2 );
        
        return;
    }
    
    
        //    Message box is deferred so that canvas refresh occurs properly before dialog
    if( bgroup_override ) {
            wxString msg( _("Group \"") );

            ChartGroup *pGroup = g_pGroupArray->Item( old_group_index - 1 );
            msg += pGroup->m_group_name;
            
            msg += _("\" is empty, switching to \"All Active Charts\" group.");
            
            OCPNMessageBox( this, msg, _("OpenCPN Group Notice"), wxOK, 5 );
    }
}

bool ChartCanvas::CheckGroup( int igroup )
{
    if(!ChartData)
        return true;                            //  Not known yet...
        
    if( igroup == 0 )
        return true;              // "all charts" is always OK

    if ( igroup < 0 )               // negative group is an error
        return false;

    ChartGroup *pGroup = g_pGroupArray->Item( igroup - 1 );
    
    if( !pGroup->m_element_array.GetCount() )   //  truly empty group prompts a warning, and auto-shift to group 0
        return false; 
    
    bool b_chart_in_group = false;
    
    for( unsigned int j = 0; j < pGroup->m_element_array.GetCount(); j++ ) {
        wxString element_root = pGroup->m_element_array.Item( j )->m_element_name;
        
        for( unsigned int ic = 0; ic < (unsigned int) ChartData->GetChartTableEntries(); ic++ ) {
            ChartTableEntry *pcte = ChartData->GetpChartTableEntry( ic );
            wxString chart_full_path( pcte->GetpFullPath(), wxConvUTF8 );
            
            if( chart_full_path.StartsWith( element_root ) ) {
                b_chart_in_group = true;
                break;
            }
        }
        
        if( b_chart_in_group ) break;
    }
    
    //  If necessary, check for GSHHS
    if(!b_chart_in_group){
        for( unsigned int j = 0; j < pGroup->m_element_array.GetCount(); j++ ) {
            wxString element_root = pGroup->m_element_array.Item( j )->m_element_name;
            wxString test_string = _T("GSHH");
            if(element_root.Upper().Contains(test_string))
                b_chart_in_group = true;
         }
    }
    
    
    return b_chart_in_group;                           // this group is empty
    
}


void ChartCanvas::canvasChartsRefresh( int dbi_hint )
{
    if( !ChartData )
        return;
    
    OCPNPlatform::ShowBusySpinner();
    
    double old_scale = GetVPScale();
    InvalidateQuilt();
    SetQuiltRefChart( -1 );
    
    m_singleChart = NULL;
    
    //delete m_pCurrentStack;
    //m_pCurrentStack = NULL;
    
    //    Build a new ChartStack
    if(!m_pCurrentStack){
        m_pCurrentStack = new ChartStack;
        ChartData->BuildChartStack( m_pCurrentStack, m_vLat, m_vLon, m_groupIndex );
    }
    
    if( -1 != dbi_hint ) {
        if( GetQuiltMode() ) {
            GetpCurrentStack()->SetCurrentEntryFromdbIndex( dbi_hint );
            SetQuiltRefChart( dbi_hint );
        } else {
            //      Open the saved chart
            ChartBase *pTentative_Chart;
            pTentative_Chart = ChartData->OpenChartFromDB( dbi_hint, FULL_INIT );
            
            if( pTentative_Chart ) {
                /* m_singleChart is always NULL here, (set above) should this go before that? */
                if( m_singleChart )
                    m_singleChart->Deactivate();
                
                m_singleChart = pTentative_Chart;
                m_singleChart->Activate();
                
                GetpCurrentStack()->CurrentStackEntry = ChartData->GetStackEntry( GetpCurrentStack(),
                                                                                  m_singleChart->GetFullPath() );
            }
            //else
                //SetChartThumbnail( dbi_hint );       // need to reset thumbnail on failed chart open
        }
        
        //refresh_Piano();
    } else {
        //    Select reference chart from the stack, as though clicked by user
        //    Make it the smallest scale chart on the stack
        GetpCurrentStack()->CurrentStackEntry = GetpCurrentStack()->nEntry - 1;
        int selected_index = GetpCurrentStack()->GetCurrentEntrydbIndex();
        SetQuiltRefChart( selected_index );
    }
    
    //    Validate the correct single chart, or set the quilt mode as appropriate
    SetupCanvasQuiltMode();
    if( !GetQuiltMode() && m_singleChart == 0) {
        // use a dummy like in DoChartUpdate
        if (NULL == pDummyChart ) 
            pDummyChart = new ChartDummy;
        m_singleChart = pDummyChart;
        SetVPScale( old_scale );
    }
    
    ReloadVP();
    
    UpdateCanvasControlBar();
    UpdateGPSCompassStatusBox( true );
    
    SetCursor( wxCURSOR_ARROW );
    
    OCPNPlatform::HideBusySpinner();
}


bool ChartCanvas::DoCanvasUpdate( void )
{
    
    double tLat, tLon;           // Chart Stack location
    double vpLat, vpLon;         // ViewPort location
    
    bool bNewChart = false;
    bool bNewView = false;
    bool bCanvasChartAutoOpen = true;                             // debugging
    
    bool bNewPiano = false;
    bool bOpenSpecified;
    ChartStack LastStack;
    ChartBase *pLast_Ch;
    
    ChartStack WorkStack;
    
    if( bDBUpdateInProgress )
        return false;
    if( !ChartData ) return false;
    
    if(ChartData->IsBusy())
        return false;
    
    int last_nEntry = -1;
    if( m_pCurrentStack )
        last_nEntry = m_pCurrentStack->nEntry;
    
    //    Startup case:
    //    Quilting is enabled, but the last chart seen was not quiltable
    //    In this case, drop to single chart mode, set persistence flag,
    //    And open the specified chart
//TODO implement this    
//     if( m_bFirstAuto && ( g_restore_dbindex >= 0 ) ) {
//         if( GetQuiltMode() ) {
//             if( !IsChartQuiltableRef( g_restore_dbindex ) ) {
//                 gFrame->ToggleQuiltMode();
//                 m_bpersistent_quilt = true;
        //                 m_singleChart = NULL;
//             }
//         }
//     }
    
    //      If in auto-follow mode, use the current glat,glon to build chart stack.
    //      Otherwise, use vLat, vLon gotten from click on chart canvas, or other means
    
    if( m_bFollow ) {
        tLat = gLat;
        tLon = gLon;

        // Set the ViewPort center based on the OWNSHIP offset
        double dx = m_OSoffsetx;
        double dy = m_OSoffsety;
        double d_east = dx / GetVP().view_scale_ppm;
        double d_north = dy / GetVP().view_scale_ppm;

        fromSM( d_east, d_north, gLat, gLon, &vpLat, &vpLon );

        // on lookahead mode, adjust the vp center point
        if( m_bLookAhead ) {
            double angle = g_COGAvg + ( GetVPRotation() * 180. / PI );
            
            double pixel_deltay = fabs( cos( angle * PI / 180. ) ) * GetCanvasHeight() / 4;
            double pixel_deltax = fabs( sin( angle * PI / 180. ) ) * GetCanvasWidth() / 4;
            
            double pixel_delta_tent = sqrt(
                ( pixel_deltay * pixel_deltay ) + ( pixel_deltax * pixel_deltax ) );
            
            double pixel_delta = 0;
            
            //    The idea here is to cancel the effect of LookAhead for slow gSog, to avoid
            //    jumping of the vp center point during slow maneuvering, or at anchor....
            if( !std::isnan(gSog) ) {
                if( gSog < 1.0 ) pixel_delta = 0.;
                else
                    if( gSog >= 3.0 ) pixel_delta = pixel_delta_tent;
                    else
                        pixel_delta = pixel_delta_tent * ( gSog - 1.0 ) / 2.0;
            }
            
            double meters_to_shift = cos( gLat * PI / 180. ) * pixel_delta / GetVPScale();
            
            double dir_to_shift = g_COGAvg;
            
            ll_gc_ll( gLat, gLon, dir_to_shift, meters_to_shift / 1852., &vpLat, &vpLon );
        }
        
    } else {
        tLat = m_vLat;
        tLon = m_vLon;
        vpLat = m_vLat;
        vpLon = m_vLon;
        
    }
    
    // Calculate change in VP, in pixels, using a simple SM projection
    // if change in pixels is smaller than 2% of screen size, do not change the VP
    // This will avoid "jitters" at large scale.
    if(GetVP().view_scale_ppm > 1.0){ 
        double easting, northing;
        toSM( GetVP().clat, GetVP().clon, vpLat, vpLon,  &easting, &northing );
        if( (fabs(easting * GetVP().view_scale_ppm) < (GetVP().pix_width * 2 / 100)) ||
            (fabs(northing * GetVP().view_scale_ppm) < (GetVP().pix_height * 2 / 100)) ){
            vpLat = GetVP().clat;
            vpLon = GetVP().clon;
        }
    }
    
    
    if( GetQuiltMode() ) {
        int current_db_index = -1;
        if( m_pCurrentStack )
            current_db_index = m_pCurrentStack->GetCurrentEntrydbIndex(); // capture the currently selected Ref chart dbIndex
        else
            m_pCurrentStack = new ChartStack;
            
            //  This logic added to enable opening a chart when there is no
                //  previous chart indication, either from inital startup, or from adding new chart directory
            if( m_bautofind && (-1 == GetQuiltReferenceChartIndex()) && m_pCurrentStack ){
                if(m_pCurrentStack->nEntry){
                    int new_dbIndex = m_pCurrentStack->GetDBIndex(m_pCurrentStack->nEntry-1);    // smallest scale
                SelectQuiltRefdbChart(new_dbIndex, true);
                m_bautofind = false;
             }
         }
                
         ChartData->BuildChartStack( m_pCurrentStack, tLat, tLon, m_groupIndex );
         m_pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );
                
         if( m_bFirstAuto ) {
                    double proposed_scale_onscreen = GetCanvasScaleFactor() / GetVPScale(); // as set from config load
                        
                        int initial_db_index = m_restore_dbindex;
                        if( initial_db_index < 0 ) {
                            if( m_pCurrentStack->nEntry ) {
                                initial_db_index = m_pCurrentStack->GetDBIndex( m_pCurrentStack->nEntry - 1 );
                            } else
                                m_bautofind = true; //initial_db_index = 0;
                        }
                        
                        if( m_pCurrentStack->nEntry ) {
                            
                            int initial_type = ChartData->GetDBChartType( initial_db_index );
                            
                            //    Check to see if the target new chart is quiltable as a reference chart
                            
                            if( !IsChartQuiltableRef( initial_db_index ) ) {
                                // If it is not quiltable, then walk the stack up looking for a satisfactory chart
                                // i.e. one that is quiltable and of the same type
                                // XXX if there's none?
                                int stack_index = 0;
                                
                                if ( stack_index >= 0 ){
                                    while( ( stack_index < m_pCurrentStack->nEntry - 1 ) ) {
                                        int test_db_index = m_pCurrentStack->GetDBIndex( stack_index );
                                        if( IsChartQuiltableRef( test_db_index )
                                            && ( initial_type == ChartData->GetDBChartType( initial_db_index ) ) ) {
                                            initial_db_index = test_db_index;
                                            break;
                                        }
                                        stack_index++;
                                    }
                                }
                            }
                            
                            ChartBase *pc = ChartData->OpenChartFromDB( initial_db_index, FULL_INIT );
                            if( pc ) {
                                SetQuiltRefChart( initial_db_index );
                                m_pCurrentStack->SetCurrentEntryFromdbIndex( initial_db_index );
                            }
                            
                            // Check proposed scale, see how much underzoom results
                            // Adjust as necessary to prevent slow loading on initial startup
                            if(pc){
                                double chart_scale = pc->GetNativeScale();
                                proposed_scale_onscreen = wxMin(proposed_scale_onscreen, chart_scale * 4);
                            }
                        }
                        
                        bNewView |= SetViewPoint( vpLat, vpLon,
                                                       GetCanvasScaleFactor() / proposed_scale_onscreen, 0,
                                                       GetVPRotation() );
                        
                }
                // else
                bNewView |= SetViewPoint( vpLat, vpLon, GetVPScale(), 0, GetVPRotation() );
                
                goto update_finish;
                
    }
    
    //  Single Chart Mode from here....
    pLast_Ch = m_singleChart;
    ChartTypeEnum new_open_type;
    ChartFamilyEnum new_open_family;
    if( pLast_Ch ) {
        new_open_type = pLast_Ch->GetChartType();
        new_open_family = pLast_Ch->GetChartFamily();
    } else {
        new_open_type = CHART_TYPE_KAP;
        new_open_family = CHART_FAMILY_RASTER;
    }
    
    bOpenSpecified = m_bFirstAuto;
    
    //  Make sure the target stack is valid
    if( NULL == m_pCurrentStack )
        m_pCurrentStack = new ChartStack;
                                                                    
                                                                    // Build a chart stack based on tLat, tLon
    if( 0 == ChartData->BuildChartStack( &WorkStack, tLat, tLon, g_sticky_chart, m_groupIndex ) ) {      // Bogus Lat, Lon?
        if( NULL == pDummyChart ) {
            pDummyChart = new ChartDummy;
            bNewChart = true;
        }
        
        if( m_singleChart ) if( m_singleChart->GetChartType() != CHART_TYPE_DUMMY ) bNewChart = true;
                                                                    
        m_singleChart = pDummyChart;
        
        //    If the current viewpoint is invalid, set the default scale to something reasonable.
                                                                    double set_scale = GetVPScale();
                                                                    if( !GetVP().IsValid() ) set_scale = 1. / 20000.;
                                                                    
                                                                    bNewView |= SetViewPoint( tLat, tLon, set_scale, 0, GetVPRotation() );
                                                                    
                                                                    //      If the chart stack has just changed, there is new status
                                                                    if(WorkStack.nEntry && m_pCurrentStack->nEntry){
                                                                        if( !ChartData->EqualStacks( &WorkStack, m_pCurrentStack ) ) {
                                                                            bNewPiano = true;
                                                                            bNewChart = true;
                                                                        }
                                                                    }
                                                                    
                                                                    //      Copy the new (by definition empty) stack into the target stack
                                                                    ChartData->CopyStack( m_pCurrentStack, &WorkStack );
                                                                    
                                                                    goto update_finish;
    }
    
    //              Check to see if Chart Stack has changed
    if( !ChartData->EqualStacks( &WorkStack, m_pCurrentStack ) ) {
        //      New chart stack, so...
        bNewPiano = true;
        
        //      Save a copy of the current stack
        ChartData->CopyStack( &LastStack, m_pCurrentStack );
        
        //      Copy the new stack into the target stack
        ChartData->CopyStack( m_pCurrentStack, &WorkStack );
        
        //  Is Current Chart in new stack?
        
        int tEntry = -1;
        if( NULL != m_singleChart )                                  // this handles startup case
            tEntry = ChartData->GetStackEntry( m_pCurrentStack, m_singleChart->GetFullPath() );
        
        if( tEntry != -1 ) {                // m_singleChart is in the new stack
            m_pCurrentStack->CurrentStackEntry = tEntry;
            bNewChart = false;
        }
        
        else                           // m_singleChart is NOT in new stack
        {                                       // So, need to open a new chart
        //      Find the largest scale raster chart that opens OK
        
        ChartBase *pProposed = NULL;
        
        if( bCanvasChartAutoOpen ) {
            bool search_direction = false;        // default is to search from lowest to highest
            int start_index = 0;
            
            //    A special case:  If panning at high scale, open largest scale chart first
            if( ( LastStack.CurrentStackEntry == LastStack.nEntry - 1 ) || ( LastStack.nEntry == 0 ) ) {
                search_direction = true;
                start_index = m_pCurrentStack->nEntry - 1;
            }
                
                //    Another special case, open specified index on program start
            if( bOpenSpecified ) {
                search_direction = false;
                start_index = 0;
                if( ( start_index < 0 ) | ( start_index >= m_pCurrentStack->nEntry ) )
                    start_index = 0;
            
                new_open_type = CHART_TYPE_DONTCARE;
            }
                
            pProposed = ChartData->OpenStackChartConditional( m_pCurrentStack, start_index,
                                                                  search_direction, new_open_type, new_open_family );
                
                //    Try to open other types/families of chart in some priority
            if( NULL == pProposed ) pProposed = ChartData->OpenStackChartConditional(
                    m_pCurrentStack, start_index, search_direction, CHART_TYPE_CM93COMP,
                    CHART_FAMILY_VECTOR );
                
            if( NULL == pProposed ) pProposed = ChartData->OpenStackChartConditional(
                    m_pCurrentStack, start_index, search_direction, CHART_TYPE_CM93COMP,
                    CHART_FAMILY_RASTER );
                
            bNewChart = true;
                
        }     // bCanvasChartAutoOpen
        
        else
            pProposed = NULL;
        
        //  If no go, then
            //  Open a Dummy Chart
            if( NULL == pProposed ) {
                if( NULL == pDummyChart ) {
                    pDummyChart = new ChartDummy;
                    bNewChart = true;
                }
                
                if( pLast_Ch ) if( pLast_Ch->GetChartType() != CHART_TYPE_DUMMY ) bNewChart = true;
                                                                    
                                                                    pProposed = pDummyChart;
            }
            
            // Arriving here, pProposed points to an opened chart, or NULL.
            if( m_singleChart ) m_singleChart->Deactivate();
            m_singleChart = pProposed;
            
            if( m_singleChart ) {
                m_singleChart->Activate();
                m_pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( m_pCurrentStack,
                                                                               m_singleChart->GetFullPath() );
            }
        }   // need new chart
        
        // Arriving here, m_singleChart is opened and OK, or NULL
        if( NULL != m_singleChart ) {
            
            //      Setup the view using the current scale
            double set_scale = GetVPScale();
            
            //    If the current viewpoint is invalid, set the default scale to something reasonable.
            if( !GetVP().IsValid() )
                set_scale = 1. / 20000.;
            else {                                    // otherwise, match scale if elected.
                double proposed_scale_onscreen;
                
                if( m_bFollow ) {          // autoset the scale only if in autofollow
                    double new_scale_ppm = m_singleChart->GetNearestPreferredScalePPM( GetVPScale() );
                    proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;
                }
                else
                    proposed_scale_onscreen = GetCanvasScaleFactor() / set_scale;
                
                
                //  This logic will bring a new chart onscreen at roughly twice the true paper scale equivalent.
                    //  Note that first chart opened on application startup (bOpenSpecified = true) will open at the config saved scale
                    if( bNewChart && !g_bPreserveScaleOnX && !bOpenSpecified ) {
                        proposed_scale_onscreen = m_singleChart->GetNativeScale() / 2;
                        double equivalent_vp_scale = GetCanvasScaleFactor()
                        / proposed_scale_onscreen;
                        double new_scale_ppm = m_singleChart->GetNearestPreferredScalePPM(
                            equivalent_vp_scale );
                        proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;
                    }
                    
                    if( m_bFollow ) {     // bounds-check the scale only if in autofollow
                    proposed_scale_onscreen =
                    wxMin(proposed_scale_onscreen, m_singleChart->GetNormalScaleMax(GetCanvasScaleFactor(), GetCanvasWidth()));
                    proposed_scale_onscreen =
                    wxMax(proposed_scale_onscreen, m_singleChart->GetNormalScaleMin(GetCanvasScaleFactor(), g_b_overzoom_x));
                    }
                    
                    set_scale = GetCanvasScaleFactor() / proposed_scale_onscreen;
            }
            
            bNewView |= SetViewPoint( vpLat, vpLon, set_scale,
                                      m_singleChart->GetChartSkew() * PI / 180., GetVPRotation() );
            
        }
    }         // new stack
    
    else                                                                 // No change in Chart Stack
    {
        if( ( m_bFollow ) && m_singleChart )
            bNewView |= SetViewPoint( vpLat, vpLon, GetVPScale(), m_singleChart->GetChartSkew() * PI / 180., GetVPRotation() );
    }
    
    update_finish:
    
    //    Ask for a new tool bar if the stack is going to or coming from only one entry.
     if(GetToolbar()){
         if(m_pCurrentStack){
            bool toolbar_scale_tools_show = m_pCurrentStack && m_pCurrentStack->b_valid && ( m_pCurrentStack->nEntry > 1 );
            bool scale_tools_shown = m_toolBar->m_toolbar_scale_tools_shown;
         
            if( toolbar_scale_tools_show != scale_tools_shown){
                if( !m_bFirstAuto )
                    RequestNewCanvasToolbar( false );
            }
         }
     }
     
//TODO
//     if( bNewPiano ) UpdateControlBar();
                                                                    
                                                                    //  Update the ownship position on thumbnail chart, if shown
    if( pthumbwin && pthumbwin->IsShown() ) {
        if( pthumbwin->pThumbChart ){
            if( pthumbwin->pThumbChart->UpdateThumbData( gLat, gLon ) )
                pthumbwin->Refresh( TRUE );
        }
    }
    
    m_bFirstAuto = false;                           // Auto open on program start
    
    //  If we need a Refresh(), do it here...
    //  But don't duplicate a Refresh() done by SetViewPoint()
    if( bNewChart && !bNewView )
        Refresh( false );
                                                                    
#ifdef ocpnUSE_GL
    // If a new chart, need to invalidate gl viewport for refresh
    // so the fbo gets flushed
    if(m_glcc && g_bopengl & bNewChart)
        GetglCanvas()->Invalidate();
#endif
        
    return bNewChart | bNewView;
}

void ChartCanvas::SelectQuiltRefdbChart( int db_index, bool b_autoscale )
{
    if( m_pCurrentStack )
        m_pCurrentStack->SetCurrentEntryFromdbIndex( db_index );
    
    SetQuiltRefChart( db_index );
    if (ChartData) {
        ChartBase *pc = ChartData->OpenChartFromDB( db_index, FULL_INIT );
        if( pc ) {
            if(b_autoscale) {
                double best_scale_ppm = GetBestVPScale( pc );
                SetVPScale( best_scale_ppm );
            }
        }
        else
            SetQuiltRefChart( -1 );
    }
    else
        SetQuiltRefChart( -1 );
}

void ChartCanvas::SelectQuiltRefChart( int selected_index )
{
    std::vector<int>  piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
    int current_db_index = piano_chart_index_array[selected_index];
    
    SelectQuiltRefdbChart( current_db_index );
}

double ChartCanvas::GetBestVPScale( ChartBase *pchart )
{
    if( pchart ) {
        double proposed_scale_onscreen = GetCanvasScaleFactor() / GetVPScale();
        
        if( ( g_bPreserveScaleOnX ) || ( CHART_TYPE_CM93COMP == pchart->GetChartType() ) ) {
            double new_scale_ppm = GetVPScale();
            proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;
        } else {
            //  This logic will bring the new chart onscreen at roughly twice the true paper scale equivalent.
            proposed_scale_onscreen = pchart->GetNativeScale() / 2;
            double equivalent_vp_scale = GetCanvasScaleFactor() / proposed_scale_onscreen;
            double new_scale_ppm = pchart->GetNearestPreferredScalePPM( equivalent_vp_scale );
            proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;
        }
        
        // Do not allow excessive underzoom, even if the g_bPreserveScaleOnX flag is set.
        // Otherwise, we get severe performance problems on all platforms
        
        double max_underzoom_multiplier = 2.0;
        if(GetVP().b_quilt){
            double scale_max = m_pQuilt->GetNomScaleMin(pchart->GetNativeScale(), pchart->GetChartType(), pchart->GetChartFamily());
            max_underzoom_multiplier = scale_max / pchart->GetNativeScale();
        }
        
        proposed_scale_onscreen =
        wxMin(proposed_scale_onscreen,
              pchart->GetNormalScaleMax(GetCanvasScaleFactor(), GetCanvasWidth()) * max_underzoom_multiplier);
        
        //  And, do not allow excessive overzoom either
        proposed_scale_onscreen =
        wxMax(proposed_scale_onscreen, pchart->GetNormalScaleMin(GetCanvasScaleFactor(), false));
        
        return GetCanvasScaleFactor() / proposed_scale_onscreen;
    } else
        return 1.0;
}

void ChartCanvas::SetupCanvasQuiltMode( void )
{
    
    if( GetQuiltMode() )                               // going to quilt mode
    {
        ChartData->LockCache();
        
        m_Piano->SetNoshowIndexArray( g_quilt_noshow_index_array );
        
        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        
        m_Piano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
        m_Piano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );
        m_Piano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
        m_Piano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );
        
        m_Piano->SetRoundedRectangles( true );
        
        //    Select the proper Ref chart
        int target_new_dbindex = -1;
        if( m_pCurrentStack ) {
            target_new_dbindex = GetQuiltReferenceChartIndex();    //m_pCurrentStack->GetCurrentEntrydbIndex();
            
            if(-1 != target_new_dbindex){
                if( !IsChartQuiltableRef( target_new_dbindex ) ){
                    
                    int proj = ChartData->GetDBChartProj(target_new_dbindex);
                    int type = ChartData->GetDBChartType(target_new_dbindex);
                    
                    // walk the stack up looking for a satisfactory chart
                    int stack_index = m_pCurrentStack->CurrentStackEntry;
                    
                    while((stack_index < m_pCurrentStack->nEntry-1) && (stack_index >= 0)) {
                        int proj_tent = ChartData->GetDBChartProj( m_pCurrentStack->GetDBIndex(stack_index));
                        int type_tent = ChartData->GetDBChartType( m_pCurrentStack->GetDBIndex(stack_index));
                        
                        if(IsChartQuiltableRef(m_pCurrentStack->GetDBIndex(stack_index))){
                            if((proj == proj_tent) && (type_tent == type)){
                                target_new_dbindex = m_pCurrentStack->GetDBIndex(stack_index);
                                break;
                            }
                        }
                        stack_index++;
                    }
                }
            }
        }
        
        if( IsChartQuiltableRef( target_new_dbindex ) )
            SelectQuiltRefdbChart( target_new_dbindex, false );        // Try not to allow a scale change
        else
            SelectQuiltRefdbChart( -1, false );
            
        m_singleChart = NULL;                  // Bye....
            
            //TODOSetChartThumbnail( -1 );            //Turn off thumbnails for sure
            
            //  Re-qualify the quilt reference chart selection
            AdjustQuiltRefChart(  );
        
        //  Restore projection type saved on last quilt mode toggle
            //TODO
//             if(g_sticky_projection != -1)
//                 GetVP().SetProjectionType(g_sticky_projection);
//             else
//                 GetVP().SetProjectionType(PROJECTION_MERCATOR);
            
            
            
    } else                                                  // going to SC Mode
    {
        std::vector<int>  empty_array;
        m_Piano->SetActiveKeyArray( empty_array );
        m_Piano->SetNoshowIndexArray( empty_array );
        m_Piano->SetEclipsedIndexArray( empty_array );
        
        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        m_Piano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
        m_Piano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );
        m_Piano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
        m_Piano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );
        
        m_Piano->SetRoundedRectangles( false );
        //TODO  Make this a member g_sticky_projection = GetVP().m_projection_type;
        
    }
    
    //    When shifting from quilt to single chart mode, select the "best" single chart to show
    if( !GetQuiltMode() ) {
        if( ChartData && ChartData->IsValid() ) {
            UnlockQuilt();
            
            double tLat, tLon;
            if( m_bFollow == true ) {
                tLat = gLat;
                tLon = gLon;
            } else {
                tLat = m_vLat;
                tLon = m_vLon;
            }
            
            if( !m_singleChart ) {
                
                // Build a temporary chart stack based on tLat, tLon
                ChartStack TempStack;
                ChartData->BuildChartStack( &TempStack, tLat, tLon, g_sticky_chart, m_groupIndex );
                
                //    Iterate over the quilt charts actually shown, looking for the largest scale chart that will be in the new chartstack....
                //    This will (almost?) always be the reference chart....
                
                ChartBase *Candidate_Chart = NULL;
                int cur_max_scale = (int) 1e8;
                
                ChartBase *pChart = GetFirstQuiltChart();
                while( pChart ) {
                    //  Is this pChart in new stack?
                    int tEntry = ChartData->GetStackEntry( &TempStack, pChart->GetFullPath() );
                    if( tEntry != -1 ) {
                        if( pChart->GetNativeScale() < cur_max_scale ) {
                            Candidate_Chart = pChart;
                            cur_max_scale = pChart->GetNativeScale();
                        }
                    }
                    pChart = GetNextQuiltChart();
                }
                
                m_singleChart = Candidate_Chart;
                
                //    If the quilt is empty, there is no "best" chart.
                //    So, open the smallest scale chart in the current stack
                if( NULL == m_singleChart ) {
                    m_singleChart = ChartData->OpenStackChartConditional( &TempStack,
                                                                       TempStack.nEntry - 1, true, CHART_TYPE_DONTCARE,
                                                                       CHART_FAMILY_DONTCARE );
                }
            }
            
            //  Invalidate all the charts in the quilt,
            // as any cached data may be region based and not have fullscreen coverage
            InvalidateAllQuiltPatchs();
            
            if( m_singleChart ) {
                int dbi = ChartData->FinddbIndex( m_singleChart->GetFullPath() );
                std::vector<int>  one_array;
                one_array.push_back( dbi );
                m_Piano->SetActiveKeyArray( one_array );
            }
            
            if( m_singleChart ) {
                GetVP().SetProjectionType(m_singleChart->GetChartProjectionType());
            }
            
        }
        //    Invalidate the current stack so that it will be rebuilt on next tick
        if( m_pCurrentStack )
            m_pCurrentStack->b_valid = false;
    }
    
}


bool ChartCanvas::IsTempMenuBarEnabled()
{
#ifdef __WXMSW__
    int major;
    wxGetOsVersion(&major);
    return (major > 5);   //  For Windows, function is only available on Vista and above
#else
    return true;
#endif
}

double ChartCanvas::GetCanvasRangeMeters()
{
    int width, height;
    GetSize(&width, &height);
    int minDimension =  wxMin(width, height);
    
    double range  = (minDimension / GetVP().view_scale_ppm)/2;
    range *= cos(GetVP().clat *PI/180.);
    return range;
}

void ChartCanvas::SetCanvasRangeMeters( double range )
{
    int width, height;
    GetSize(&width, &height);
    int minDimension =  wxMin(width, height);
    
    double scale_ppm = minDimension / (range / cos(GetVP().clat *PI/180.));
    SetVPScale( scale_ppm / 2 );
    
}


bool ChartCanvas::SetUserOwnship(){
    //  Look for user defined ownship image
    //  This may be found in the shared data location along with other user defined icons.
    //  and will be called "ownship.xpm" or "ownship.png"
    if( pWayPointMan && pWayPointMan->DoesIconExist( _T("ownship") ) ) {
        
        double factor_dusk = 0.5;
        double factor_night = 0.25;
        
        wxBitmap *pbmp = pWayPointMan->GetIconBitmap( _T("ownship") );
        m_pos_image_user_day = new wxImage;
        *m_pos_image_user_day = pbmp->ConvertToImage();
        if(!m_pos_image_user_day->HasAlpha())
            m_pos_image_user_day->InitAlpha();
        
        int gimg_width = m_pos_image_user_day->GetWidth();
        int gimg_height = m_pos_image_user_day->GetHeight();
        
        // Make dusk and night images
        m_pos_image_user_dusk = new wxImage;
        m_pos_image_user_night = new wxImage;
        
        *m_pos_image_user_dusk = m_pos_image_user_day->Copy();
        *m_pos_image_user_night = m_pos_image_user_day->Copy();
        
        for( int iy = 0; iy < gimg_height; iy++ ) {
            for( int ix = 0; ix < gimg_width; ix++ ) {
                if( !m_pos_image_user_day->IsTransparent( ix, iy ) ) {
                    wxImage::RGBValue rgb( m_pos_image_user_day->GetRed( ix, iy ),
                                           m_pos_image_user_day->GetGreen( ix, iy ),
                                           m_pos_image_user_day->GetBlue( ix, iy ) );
                    wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_dusk;
                    wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_dusk->SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
                    
                    hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_night;
                    nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_night->SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
                }
            }
        }
        
        //  Make some alternate greyed out day/dusk/night images
        m_pos_image_user_grey_day = new wxImage;
        *m_pos_image_user_grey_day = m_pos_image_user_day->ConvertToGreyscale();
        
        m_pos_image_user_grey_dusk = new wxImage;
        m_pos_image_user_grey_night = new wxImage;
        
        *m_pos_image_user_grey_dusk = m_pos_image_user_grey_day->Copy();
        *m_pos_image_user_grey_night = m_pos_image_user_grey_day->Copy();
        
        for( int iy = 0; iy < gimg_height; iy++ ) {
            for( int ix = 0; ix < gimg_width; ix++ ) {
                if( !m_pos_image_user_grey_day->IsTransparent( ix, iy ) ) {
                    wxImage::RGBValue rgb( m_pos_image_user_grey_day->GetRed( ix, iy ),
                                           m_pos_image_user_grey_day->GetGreen( ix, iy ),
                                           m_pos_image_user_grey_day->GetBlue( ix, iy ) );
                    wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_dusk;
                    wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_grey_dusk->SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
                    
                    hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_night;
                    nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_grey_night->SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
                }
            }
        }
        
        //  Make a yellow image for rendering under low accuracy chart conditions
        m_pos_image_user_yellow_day = new wxImage;
        m_pos_image_user_yellow_dusk = new wxImage;
        m_pos_image_user_yellow_night = new wxImage;
        
        *m_pos_image_user_yellow_day = m_pos_image_user_grey_day->Copy();
        *m_pos_image_user_yellow_dusk = m_pos_image_user_grey_day->Copy();
        *m_pos_image_user_yellow_night = m_pos_image_user_grey_day->Copy();
        
        for( int iy = 0; iy < gimg_height; iy++ ) {
            for( int ix = 0; ix < gimg_width; ix++ ) {
                if( !m_pos_image_user_grey_day->IsTransparent( ix, iy ) ) {
                    wxImage::RGBValue rgb( m_pos_image_user_grey_day->GetRed( ix, iy ),
                                           m_pos_image_user_grey_day->GetGreen( ix, iy ),
                                           m_pos_image_user_grey_day->GetBlue( ix, iy ) );
                    
                    //  Simply remove all "blue" from the greyscaled image...
                    //  so, what is not black becomes yellow.
                    wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                    wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_yellow_day->SetRGB( ix, iy, nrgb.red, nrgb.green, 0 );
                    
                    hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_dusk;
                    nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_yellow_dusk->SetRGB( ix, iy, nrgb.red, nrgb.green, 0 );
                    
                    hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_night;
                    nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_yellow_night->SetRGB( ix, iy, nrgb.red, nrgb.green, 0 );
                }
            }
        }
 
        return true;
    }
    else
        return false;
}

void ChartCanvas::SetDisplaySizeMM( double size )
{
    m_display_size_mm = size;
    
    int sx, sy;
    wxDisplaySize( &sx, &sy );
    
    double max_physical = wxMax(sx, sy);
    
    m_pix_per_mm = ( max_physical ) / ( (double) m_display_size_mm );
    m_canvas_scale_factor = ( max_physical ) / (m_display_size_mm /1000.);
    
    // set values of tide_draw_scaler, current_draw_scaler
    // each "scaler" is intended to be about unity on a 100 dpi display,
    // when its respective scale parameter is 100
    // parameter g_tide_rectangle_scale is set in config file as value of TideRectangleScale
    // parameter g_current_arrow_scale is set in config file as value of CurrentArrowScale
    g_tide_rectangle_scale = wxMax(g_tide_rectangle_scale,0);
    tide_draw_scaler = m_pix_per_mm * 0.254 * g_tide_rectangle_scale / 100.0;
    g_current_arrow_scale = wxMax(g_current_arrow_scale,0);
    current_draw_scaler = m_pix_per_mm * 0.254 * g_current_arrow_scale / 100.0;
    
#ifdef USE_S57
    if( ps52plib )
        ps52plib->SetPPMM( m_pix_per_mm );
#endif
    
     wxString msg;
     msg.Printf(_T("Metrics:  m_display_size_mm: %g     wxDisplaySize:  %d:%d   "), m_display_size_mm, sx, sy);
     wxLogMessage(msg);
    
     m_focus_indicator_pix = std::round(2 * GetPixPerMM());

}
#if 0
void ChartCanvas::OnEvtCompressProgress( OCPN_CompressProgressEvent & event )
{
    wxString msg(event.m_string.c_str(), wxConvUTF8);
    // if cpus are removed between runs
    if(pprog_threads > 0 && compress_msg_array.GetCount() >  (unsigned int)pprog_threads) {
        compress_msg_array.RemoveAt(pprog_threads, compress_msg_array.GetCount() - pprog_threads);
    }

    if(compress_msg_array.GetCount() > (unsigned int)event.thread )
    {
        compress_msg_array.RemoveAt(event.thread);
        compress_msg_array.Insert( msg, event.thread);
    }
    else
        compress_msg_array.Add(msg);
    
    
    wxString combined_msg;
    for(unsigned int i=0 ; i < compress_msg_array.GetCount() ; i++) {
        combined_msg += compress_msg_array[i];
        combined_msg += _T("\n");
    }
    
    bool skip = false;
    pprog->Update(pprog_count, combined_msg, &skip );
    pprog->SetSize(pprog_size);
    if(skip)
        b_skipout = skip;
}
#endif
void ChartCanvas::InvalidateGL()
{
    if(!m_glcc)
        return;
#ifdef ocpnUSE_GL
        if(g_bopengl)
            m_glcc->Invalidate();
#endif
    if(m_Compass)
        m_Compass->UpdateStatus( true );
}

int ChartCanvas::GetCanvasChartNativeScale()
{
    int ret = 1;
    if( !VPoint.b_quilt ) {
        if( m_singleChart ) ret = m_singleChart->GetNativeScale();
    } else
        ret = (int) m_pQuilt->GetRefNativeScale();

    return ret;

}

ChartBase* ChartCanvas::GetChartAtCursor() {
    ChartBase* target_chart;
    if( m_singleChart && ( m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR ) )
        target_chart = m_singleChart;
    else
        if( VPoint.b_quilt )
            target_chart = m_pQuilt->GetChartAtPix( VPoint, wxPoint( mouse_x, mouse_y ) );
        else
            target_chart = NULL;
    return target_chart;
}

ChartBase* ChartCanvas::GetOverlayChartAtCursor() {
    ChartBase* target_chart;
    if( VPoint.b_quilt )
        target_chart = m_pQuilt->GetOverlayChartAtPix( VPoint, wxPoint( mouse_x, mouse_y ) );
    else
        target_chart = NULL;
    return target_chart;
}

int ChartCanvas::FindClosestCanvasChartdbIndex( int scale )
{
    int new_dbIndex = -1;
    if( !VPoint.b_quilt ) {
        if( m_pCurrentStack ) {
            for( int i = 0; i < m_pCurrentStack->nEntry; i++ ) {
                int sc = ChartData->GetStackChartScale( m_pCurrentStack, i, NULL, 0 );
                if( sc >= scale ) {
                    new_dbIndex = m_pCurrentStack->GetDBIndex( i );
                    break;
                }
            }
        }
    } else {
        //    Using the current quilt, select a useable reference chart
        //    Said chart will be in the extended (possibly full-screen) stack,
        //    And will have a scale equal to or just greater than the stipulated value
        unsigned int im = m_pQuilt->GetExtendedStackIndexArray().size();
        if( im > 0 ) {
            for( unsigned int is = 0; is < im; is++ ) {
                const ChartTableEntry &m = ChartData->GetChartTableEntry(
                                               m_pQuilt->GetExtendedStackIndexArray()[is] );
                if( ( m.Scale_ge(scale ) )/* && (m_reference_family == m.GetChartFamily())*/) {
                    new_dbIndex = m_pQuilt->GetExtendedStackIndexArray()[is];
                    break;
                }
            }
        }
    }

    return new_dbIndex;
}

void ChartCanvas::EnablePaint(bool b_enable)
{
    m_b_paint_enable = b_enable;
#ifdef ocpnUSE_GL
    if(m_glcc)
        m_glcc->EnablePaint(b_enable);
#endif
}

bool ChartCanvas::IsQuiltDelta()
{
    return m_pQuilt->IsQuiltDelta( VPoint );
}

void ChartCanvas::UnlockQuilt()
{
    m_pQuilt->UnlockQuilt();
}

std::vector<int>  ChartCanvas::GetQuiltIndexArray( void )
{
    return m_pQuilt->GetQuiltIndexArray();;
}

void ChartCanvas::SetQuiltMode( bool b_quilt )
{
    VPoint.b_quilt = b_quilt;
    VPoint.b_FullScreenQuilt = g_bFullScreenQuilt;
}

bool ChartCanvas::GetQuiltMode( void )
{
    return VPoint.b_quilt;
}

int ChartCanvas::GetQuiltReferenceChartIndex(void)
{
    return m_pQuilt->GetRefChartdbIndex();
}

void ChartCanvas::InvalidateAllQuiltPatchs( void )
{
    m_pQuilt->InvalidateAllQuiltPatchs();
}

ChartBase *ChartCanvas::GetLargestScaleQuiltChart()
{
    return m_pQuilt->GetLargestScaleChart();
}

ChartBase *ChartCanvas::GetFirstQuiltChart()
{
    return m_pQuilt->GetFirstChart();
}

ChartBase *ChartCanvas::GetNextQuiltChart()
{
    return m_pQuilt->GetNextChart();
}

int ChartCanvas::GetQuiltChartCount()
{
    return m_pQuilt->GetnCharts();
}

void ChartCanvas::SetQuiltChartHiLiteIndex( int dbIndex )
{
    m_pQuilt->SetHiliteIndex( dbIndex );
}

std::vector<int>  ChartCanvas::GetQuiltCandidatedbIndexArray( bool flag1, bool flag2 )
{
    return m_pQuilt->GetCandidatedbIndexArray( flag1, flag2 );
}

int ChartCanvas::GetQuiltRefChartdbIndex( void )
{
    return m_pQuilt->GetRefChartdbIndex();
}

std::vector<int>  ChartCanvas::GetQuiltExtendedStackdbIndexArray()
{
    return m_pQuilt->GetExtendedStackIndexArray();
}

std::vector<int>  ChartCanvas::GetQuiltEclipsedStackdbIndexArray()
{
    return m_pQuilt->GetEclipsedStackIndexArray();
}

void ChartCanvas::InvalidateQuilt( void )
{
    return m_pQuilt->Invalidate();
}

double ChartCanvas::GetQuiltMaxErrorFactor()
{
    return m_pQuilt->GetMaxErrorFactor();
}

bool ChartCanvas::IsChartQuiltableRef( int db_index )
{
    return m_pQuilt->IsChartQuiltableRef( db_index );
}

bool ChartCanvas::IsChartLargeEnoughToRender( ChartBase* chart, ViewPort& vp )
{
    double chartMaxScale = chart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
    return ( chartMaxScale*g_ChartNotRenderScaleFactor > vp.chart_scale );
}

void ChartCanvas::StartMeasureRoute()
{
    if( !m_routeState ) {  // no measure tool if currently creating route
        if( m_bMeasure_Active ) {
            g_pRouteMan->DeleteRoute( m_pMeasureRoute );
            m_pMeasureRoute = NULL;
        }
        
        m_bMeasure_Active = true;
        m_nMeasureState = 1;
        m_bDrawingRoute = false;

        SetCursor( *pCursorPencil );
        Refresh();
    }
}

void ChartCanvas::CancelMeasureRoute()
{
    m_bMeasure_Active = false;
    m_nMeasureState = 0;
    m_bDrawingRoute = false;

    g_pRouteMan->DeleteRoute( m_pMeasureRoute );
    m_pMeasureRoute = NULL;
}

ViewPort &ChartCanvas::GetVP()
{
    return VPoint;
}

void ChartCanvas::SetVP(ViewPort &vp)
{
    VPoint = vp;
}

// void ChartCanvas::SetFocus()
// {
//     printf("set %d\n", m_canvasIndex);
//     //wxWindow:SetFocus();
// }


void ChartCanvas::TriggerDeferredFocus()
{
//#if defined(__WXGTK__) || defined(__WXOSX__)

    m_deferredFocusTimer.Start(20, true);
    
#if defined(__WXGTK__) || defined(__WXOSX__)
    gFrame->Raise();
#endif    

//    gFrame->Raise();
//#else
//    SetFocus();
//    Refresh(true);
//#endif    
}

void ChartCanvas::OnDeferredFocusTimerEvent( wxTimerEvent &event)
{
    SetFocus();
    Refresh(true);
    

}

void ChartCanvas::OnKeyChar( wxKeyEvent &event )
{
    if(g_pi_manager)
        if(g_pi_manager->SendKeyEventToPlugins( event ))
            return;                     // PlugIn did something, and does not want the canvas to do anything else
            
    int key_char = event.GetKeyCode();
    
    if(g_benable_rotate){
        
        switch( key_char ) {
            case ']':
                RotateCanvas( 1 );
                break;
                
            case '[':
                RotateCanvas( -1 );
                break;
                
            case '\\':
                DoRotateCanvas(0);
                break;
        }
    }

    event.Skip();
}    

void ChartCanvas::OnKeyDown( wxKeyEvent &event )
{
    if(g_pi_manager)
        if(g_pi_manager->SendKeyEventToPlugins( event ))
            return;                     // PlugIn did something, and does not want the canvas to do anything else
            
    bool b_handled = false;
    
    m_modkeys = event.GetModifiers();

    int panspeed = m_modkeys == wxMOD_ALT ? 2 : 100;

#ifdef OCPN_ALT_MENUBAR        
#ifndef __WXOSX__
    // If the permanent menubar is disabled, we show it temporarily when Alt is pressed or when
    // Alt + a letter is presssed (for the top-menu-level hotkeys).
    // The toggling normally takes place in OnKeyUp, but here we handle some special cases.
    if ( IsTempMenuBarEnabled() && event.AltDown()  &&  !g_bShowMenuBar ) {
        // If Alt + a letter is pressed, and the menubar is hidden, show it now
        if ( event.GetKeyCode() >= 'A' && event.GetKeyCode() <= 'Z' ) {
            if ( !g_bTempShowMenuBar ) {
                g_bTempShowMenuBar = true;
                parent_frame->ApplyGlobalSettings(false);
            }
            m_bMayToggleMenuBar = false; // don't hide it again when we release Alt
            event.Skip();
            return;
        }
        // If another key is pressed while Alt is down, do NOT toggle the menus when Alt is released
        if ( event.GetKeyCode() != WXK_ALT ) {
            m_bMayToggleMenuBar = false;
        }
    }
#endif
#endif

    // HOTKEYS
    switch( event.GetKeyCode() ) {
        
    case WXK_TAB:
        //parent_frame->SwitchKBFocus( this );
        break;
            
    case WXK_MENU:
        int x, y;
        event.GetPosition( &x, &y );

        CallPopupMenu(x, y);
        break;

    case WXK_ALT:
        m_modkeys |= wxMOD_ALT;
        break;

    case WXK_CONTROL:
        m_modkeys |= wxMOD_CONTROL;
        break;

    case WXK_LEFT:
        if( m_modkeys == wxMOD_CONTROL ) parent_frame->DoStackDown( this );
        else if(g_bsmoothpanzoom) {
            StartTimedMovement();
            m_panx = -1;
        } else {
            PanCanvas( -panspeed, 0 );
        }
        b_handled = true;
        break;

    case WXK_UP:
        if(g_bsmoothpanzoom) {
            StartTimedMovement();
            m_pany = -1;
        } else
            PanCanvas( 0, -panspeed );
        b_handled = true;
        break;

    case WXK_RIGHT:
        if( m_modkeys == wxMOD_CONTROL ) parent_frame->DoStackUp( this );
        else if(g_bsmoothpanzoom) {
            StartTimedMovement();
            m_panx = 1;
        } else
            PanCanvas( panspeed, 0 );
        b_handled = true;
        
        break;

    case WXK_DOWN:
        if(g_bsmoothpanzoom) {
            StartTimedMovement();
            m_pany = 1;
        } else
            PanCanvas( 0, panspeed );
        b_handled = true;
        break;

    case WXK_F2:
        TogglebFollow();
        break;

    case WXK_F3: {
        SetShowENCText( !GetShowENCText() );
        Refresh(true);
        InvalidateGL();
        break;
    }
    case WXK_F4:
        if( !m_bMeasure_Active ) {
            if (event.ShiftDown())
                m_bMeasure_DistCircle = true;
            else
                m_bMeasure_DistCircle = false;

            StartMeasureRoute();
        }
        else{
            CancelMeasureRoute();
            
            SetCursor( *pCursorArrow );
            
            SurfaceToolbar();
            InvalidateGL();
            Refresh( false );
        }
        
        break;

    case WXK_F5:
        parent_frame->ToggleColorScheme();
        SetFocus();             
        gFrame->Raise();        
        break;

    case WXK_F6: {
        int mod = m_modkeys & wxMOD_SHIFT;
        if( mod != m_brightmod ) {
            m_brightmod = mod;
            m_bbrightdir = !m_bbrightdir;
        }

        if( !m_bbrightdir ) {
            g_nbrightness -= 10;
            if( g_nbrightness <= MIN_BRIGHT ) {
                g_nbrightness = MIN_BRIGHT;
                m_bbrightdir = true;
            }
        } else {
            g_nbrightness += 10;
            if( g_nbrightness >= MAX_BRIGHT ) {
                g_nbrightness = MAX_BRIGHT;
                m_bbrightdir = false;
            }
        }

        SetScreenBrightness( g_nbrightness );
        ShowBrightnessLevelTimedPopup( g_nbrightness / 10, 1, 10 );

        SetFocus();             // just in case the external program steals it....
        gFrame->Raise();        // And reactivate the application main

        break;
    }

    case WXK_F7:
        parent_frame->DoStackDown( this );
        break;

    case WXK_F8:
        parent_frame->DoStackUp( this );
        break;

#ifndef __WXOSX__        
    case WXK_F9:
        ToggleCanvasQuiltMode();
        break;
#endif        
        
    case WXK_F11:
        parent_frame->ToggleFullScreen();
        b_handled = true;
        break;

    case WXK_F12: {
        if( m_modkeys == wxMOD_ALT )
            m_nMeasureState = *(int *)(0);          // generate a fault for testing

        ToggleChartOutlines();
        break;
    }

    case WXK_PAUSE:                   // Drop MOB
         parent_frame->ActivateMOB();
         break;

    //NUMERIC PAD
    case WXK_NUMPAD_ADD:              // '+' on NUM PAD
    case WXK_PAGEUP:{
        ZoomCanvas( 2.0, false );
        break;
    }
    case WXK_NUMPAD_SUBTRACT:   // '-' on NUM PAD
    case WXK_PAGEDOWN:{
        ZoomCanvas( .5, false );
        break;
    }
	case WXK_DELETE:
	case WXK_BACK:
		if (m_bMeasure_Active) {
            if (m_nMeasureState > 2) {
                m_pMeasureRoute->DeletePoint(m_pMeasureRoute->GetLastPoint());
                m_pMeasureRoute->m_lastMousePointIndex = m_pMeasureRoute->GetnPoints();
                m_nMeasureState--;
                gFrame->RefreshAllCanvas();
            }
            else {
                CancelMeasureRoute();
                StartMeasureRoute();
            }
		}
	break;
    default:
        break;

    }

    if( event.GetKeyCode() < 128 )            //ascii
    {
        int key_char = event.GetKeyCode();

        //      Handle both QWERTY and AZERTY keyboard separately for a few control codes
        if( !g_b_assume_azerty ) {
            switch( key_char ) {
            case '+': case '=':
                ZoomCanvas( 2.0, false );
                break;

            case '-': case '_':
                ZoomCanvas( 0.5, false );
                break;

            }
            
#ifdef __WXMAC__
            if(g_benable_rotate){
                switch( key_char ) {
                    
            // On other platforms these are handled in OnKeyChar, which (apparently) works better in some locales.
            // On OS X it is better to handle them here, since pressing Alt (which should change the rotation speed)
            // changes the key char and so prevents the keys from working.
                case ']':
                    RotateCanvas( 1 );
                    break;
                
                case '[':
                    RotateCanvas( -1 );
                    break;
                
                case '\\':
                    DoRotateCanvas(0);
                    break;
                }
            }
#endif
        } else {   //AZERTY
            switch( key_char ) {
            case 43:
                ZoomCanvas( 2.0, false );
                break;

            case 54:                     // '-'  alpha/num pad
//            case 56:                     // '_'  alpha/num pad
                ZoomCanvas( 0.5, false );
                break;
            }
        }


        if ( event.ControlDown() )
            key_char -= 64;

        if (key_char >= '0' && key_char <= '9')
            SetGroupIndex( key_char - '0' );
        else

        switch( key_char ) {
        case 'A':
            SetShowENCAnchor(!GetShowENCAnchor());
            ReloadVP();
            
            break;

        case 'C':
            parent_frame->ToggleColorScheme();
            break;

        case 'D':
        {
            int x,y;
            event.GetPosition( &x, &y );
            ChartTypeEnum ChartType = CHART_TYPE_UNKNOWN;
            ChartFamilyEnum ChartFam = CHART_FAMILY_UNKNOWN;
                // First find out what kind of chart is being used
            if( !pPopupDetailSlider ) {
                if( VPoint.b_quilt ) 
                    {
                        if (m_pQuilt) 
                        { 
                            if (m_pQuilt->GetChartAtPix( VPoint, wxPoint( x, y )) ) // = null if no chart loaded for this point
                            {
                                ChartType = m_pQuilt->GetChartAtPix( VPoint, wxPoint( x, y ) )->GetChartType();
                                ChartFam = m_pQuilt->GetChartAtPix( VPoint, wxPoint( x, y ) )->GetChartFamily();
                            }
                        }                        
                    }
                else
                    {
                        if (m_singleChart){
                            ChartType = m_singleChart->GetChartType();
                            ChartFam =  m_singleChart->GetChartFamily();                            
                        }                        
                    }
                    //If a charttype is found show the popupslider
                if ( (ChartType != CHART_TYPE_UNKNOWN) || (ChartFam != CHART_FAMILY_UNKNOWN) )
                    {
                        pPopupDetailSlider = new PopUpDSlide( this, -1, ChartType, ChartFam,
                            wxPoint( g_detailslider_dialog_x, g_detailslider_dialog_y ),
                            wxDefaultSize, wxSIMPLE_BORDER, _T("") );
                        if (pPopupDetailSlider)
#ifdef __WXOSX__
                            pPopupDetailSlider->Show();
#else
                            pPopupDetailSlider->ShowModal();
#endif
                    }
                }
            else //( !pPopupDetailSlider ) close popupslider
            {
                if (pPopupDetailSlider) pPopupDetailSlider->Close();
                pPopupDetailSlider = NULL;
            }             
            break;
        }

       case 'L':
            SetShowENCLights(!GetShowENCLights());
            ReloadVP();
            
            break;

        case 'M':
            if (event.ShiftDown())
                m_bMeasure_DistCircle = true;
            else
                m_bMeasure_DistCircle = false;

            StartMeasureRoute();
            break;
            
        case 'N':
            if( g_bInlandEcdis && ps52plib){
                SetENCDisplayCategory( (_DisCat)STANDARD );
            }
            break;

        case 'O':
            ToggleChartOutlines();
            break;

        case 'Q':
            ToggleCanvasQuiltMode();
            break;

        case 'P':
            parent_frame->ToggleTestPause();
            break;
#if 0
        case 'R':
            parent_frame->ToggleRocks();
            break;
#endif
        case 'S':
            SetShowENCDepth( !m_encShowDepth );
            ReloadVP();
            break;

        case 'T':
            SetShowENCText(!GetShowENCText());
            ReloadVP();
            break;

        case 'U':
            SetShowENCDataQual(!GetShowENCDataQual());
            ReloadVP();
            break;

        case 'V':
            m_bShowNavobjects = !m_bShowNavobjects;
            Refresh( true );
            break;

        case 1:                      // Ctrl A
            TogglebFollow();
        
            break;

        case 2:                      // Ctrl B
            if ( g_bShowMenuBar == false )
                parent_frame->ToggleChartBar( this );
            break;

        case 13:             // Ctrl M // Drop Marker at cursor
        {
            if(event.ControlDown())
                gFrame->DropMarker(false);
            break;
        }

        case 14:             // Ctrl N - Activate next waypoint in a route
        {
            if( Route * r = g_pRouteMan->GetpActiveRoute() ) {
                int indexActive = r->GetIndexOf( r->m_pRouteActivePoint );
                if( ( indexActive + 1 ) <= r->GetnPoints() ) {
                    g_pRouteMan->ActivateNextPoint( r, true );
                    InvalidateGL();
                    Refresh( false );
                }
            }
            break;
        }

        case 15:             // Ctrl O - Drop Marker at boat's position
        {
            if(!g_bShowMenuBar)
                gFrame->DropMarker(true);
            break;
        }

        case 32:            // Special needs use space bar
        {
            if ( g_bSpaceDropMark )
                gFrame->DropMarker( true );
            break;
        }

        case -32:                     // Ctrl Space            //    Drop MOB
        {
            if( m_modkeys == wxMOD_CONTROL ) parent_frame->ActivateMOB();

            break;
        }

        case -20:                       // Ctrl ,
        {
            parent_frame->DoSettings();
            break;
        }
        case 17:                       // Ctrl Q
            parent_frame->Close();
            return;

        case 18:                       // Ctrl R
            StartRoute();
            return;

        case 20:                       // Ctrl T
            if( NULL == pGoToPositionDialog ) // There is one global instance of the Go To Position Dialog
                pGoToPositionDialog = new GoToPositionDialog( this );
            pGoToPositionDialog->SetCanvas( this );
            pGoToPositionDialog->Show();
            break;

        case 25:                       // Ctrl Y
            if( undo->AnythingToRedo() ) {
                undo->RedoNextAction();
                InvalidateGL();
                Refresh( false );
            }
            break;

        case 26:
            if ( event.ShiftDown() ) { // Shift-Ctrl-Z
                if( undo->AnythingToRedo() ) {
                    undo->RedoNextAction();
                    InvalidateGL();
                    Refresh( false );
                }
            } else {                   // Ctrl Z
                if( undo->AnythingToUndo() ) {
                    undo->UndoLastAction();
                    InvalidateGL();
                    Refresh( false );
                }
            }
            break;

        case 27:
            // Generic break
            if( m_bMeasure_Active ) {
                CancelMeasureRoute();

                SetCursor( *pCursorArrow );

                SurfaceToolbar();
                gFrame->RefreshAllCanvas();
            }

            if( m_routeState )         // creating route?
            {
                FinishRoute();
                SurfaceToolbar();
                InvalidateGL();
                Refresh( false );
            }

            break;

        case 7:                       // Ctrl G
            switch( gamma_state ) {
            case ( 0 ):
                r_gamma_mult = 0;
                g_gamma_mult = 1;
                b_gamma_mult = 0;
                gamma_state = 1;
                break;
            case ( 1 ):
                r_gamma_mult = 1;
                g_gamma_mult = 0;
                b_gamma_mult = 0;
                gamma_state = 2;
                break;
            case ( 2 ):
                r_gamma_mult = 1;
                g_gamma_mult = 1;
                b_gamma_mult = 1;
                gamma_state = 0;
                break;
            }
            SetScreenBrightness( g_nbrightness );

            break;

        case 9:                      // Ctrl I
//             if (m_Compass) {
//                 m_Compass->Show(!m_Compass->IsShown());
//                 if (m_Compass->IsShown())
//                     m_Compass->UpdateStatus();
//                 m_brepaint_piano = true;
//                 Refresh( false );
//             }
            break;

        default:
            break;

        }           // switch
    }

#ifndef __WXMAC__
    // Allow OnKeyChar to catch the key events too.
    // On OS X this is unnecessary since we handle all key events here.
    if(!b_handled)
        event.Skip();
#endif
}

void ChartCanvas::OnKeyUp( wxKeyEvent &event )
{
    if(g_pi_manager)
        if(g_pi_manager->SendKeyEventToPlugins( event ))
            return;                     // PlugIn did something, and does not want the canvas to do anything else
            
    switch( event.GetKeyCode() ) {
    case WXK_TAB:
        parent_frame->SwitchKBFocus( this );
        break;
                
    case WXK_LEFT:
    case WXK_RIGHT:
        m_panx = 0;
        if(!m_pany)
            m_panspeed = 0;
        break;

    case WXK_UP:
    case WXK_DOWN:
        m_pany = 0;
        if(!m_panx)
            m_panspeed = 0;
        break;

    case WXK_NUMPAD_ADD:              // '+' on NUM PAD
    case WXK_NUMPAD_SUBTRACT:   // '-' on NUM PAD
    case WXK_PAGEUP:
    case WXK_PAGEDOWN:
        if(m_mustmove)
            DoMovement(m_mustmove);

        m_zoom_factor = 1;
        break;

    case WXK_ALT:
        m_modkeys &= ~wxMOD_ALT;
#ifdef OCPN_ALT_MENUBAR        
#ifndef __WXOSX__
        // If the permanent menu bar is disabled, and we are not in the middle of another key combo,
        // then show the menu bar temporarily when Alt is released (or hide it if already visible).
        if ( IsTempMenuBarEnabled() && !g_bShowMenuBar  &&  m_bMayToggleMenuBar ) {
            g_bTempShowMenuBar = !g_bTempShowMenuBar;
            parent_frame->ApplyGlobalSettings(false);
        }
        m_bMayToggleMenuBar = true;
#endif
#endif        
        break;

    case WXK_CONTROL:
        m_modkeys &= ~wxMOD_CONTROL;
        break;

    }

    if( event.GetKeyCode() < 128 )            //ascii
    {
        int key_char = event.GetKeyCode();

        //      Handle both QWERTY and AZERTY keyboard separately for a few control codes
        if( !g_b_assume_azerty ) {
            switch( key_char ) {
            case '+':     case '=':    
            case '-':     case '_':
              case 54:    case 56:    // '_'  alpha/num pad
                DoMovement(m_mustmove);

                m_zoom_factor = 1;
                break;
            case '[': case ']':
                DoMovement(m_mustmove);
                m_rotation_speed = 0;
                break;
            }
        } else {
            switch( key_char ) {
            case 43:
            case 54:                     // '-'  alpha/num pad
            case 56:                     // '_'  alpha/num pad
                DoMovement(m_mustmove);

                m_zoom_factor = 1;
                break;
            }
        }
    }
    event.Skip();
}

void ChartCanvas::ToggleChartOutlines( void )
{
    m_bShowOutlines = !m_bShowOutlines;
    
    Refresh( false );
    
    #ifdef ocpnUSE_GL         // opengl renders chart outlines as part of the chart this needs a full refresh
    if( g_bopengl )
        InvalidateGL();
    #endif
}


void ChartCanvas::ToggleLookahead( )
{
    m_bLookAhead = !m_bLookAhead;
}


void ChartCanvas::ToggleCourseUp( )
{
    m_bCourseUp = !m_bCourseUp;
    
    if( m_bCourseUp ) {
        //    Stuff the COGAvg table in case COGUp is selected
        double stuff = 0;
        if( !std::isnan(gCog) )
            stuff = gCog;
        
        if( g_COGAvgSec > 0) {
            for( int i = 0; i < g_COGAvgSec; i++ )
                gFrame->COGTable[i] = stuff;
        }
        g_COGAvg = stuff;
        gFrame->FrameCOGTimer.Start( 100, wxTIMER_CONTINUOUS );
    } else {
        if ( !g_bskew_comp && (fabs(GetVPSkew()) > 0.0001))
            SetVPRotation(GetVPSkew());
        else
            SetVPRotation(0); /* reset to north up */
    }
    
    
    //DoCOGSet();
    UpdateGPSCompassStatusBox( true );
    gFrame->DoChartUpdate();
}

bool ChartCanvas::DoCanvasCOGSet( void )
{
    if( !m_bCourseUp )
        return false;
    
    if (std::isnan(g_COGAvg))
        return true;
    
    double old_VPRotate = m_VPRotate;
    m_VPRotate = -g_COGAvg * PI / 180.;
    
    SetVPRotation( m_VPRotate );
    bool bnew_chart = DoCanvasUpdate();
    
    if( ( bnew_chart ) || ( old_VPRotate != m_VPRotate ) )
        ReloadVP();
    
    return true;
}

void ChartCanvas::StopMovement( )
{
    m_panx = m_pany = 0;
    m_panspeed = 0;
    m_zoom_factor = 1;
    m_rotation_speed = 0;
    m_mustmove = 0;
#if 0    
#if !defined(__WXGTK__) && !defined(__WXQT__)
    SetFocus();
    gFrame->Raise();
#endif    
#endif
}

/* instead of integrating in timer callbacks
   (which do not always get called fast enough)
   we can perform the integration of movement
   at each render frame based on the time change */
bool ChartCanvas::StartTimedMovement( bool stoptimer )
{
    // Start/restart the stop movement timer
    if(stoptimer)
        pMovementStopTimer->Start( 1000, wxTIMER_ONE_SHOT ); 

    if(!pMovementTimer->IsRunning()){
//        printf("timer not running, starting\n");
        pMovementTimer->Start( 1, wxTIMER_ONE_SHOT ); 
    }
    
    if(m_panx || m_pany || m_zoom_factor!=1 || m_rotation_speed) {
        // already moving, gets called again because of key-repeat event
        return false;
    }

    m_last_movement_time = wxDateTime::UNow();

    /* jumpstart because paint gets called right away, if we want first frame to move */
//    m_last_movement_time -= wxTimeSpan::Milliseconds(100);

//    Refresh( false );

    return true;
}

void ChartCanvas::DoTimedMovement()
{
    if( m_pan_drag == wxPoint(0, 0) && !m_panx && !m_pany && m_zoom_factor==1 && !m_rotation_speed)
        return; /* not moving */

    wxDateTime now = wxDateTime::UNow();
    long dt = 0;
    if(m_last_movement_time.IsValid())
        dt = (now - m_last_movement_time).GetMilliseconds().ToLong();

    m_last_movement_time = now;
        
    if(dt > 500) /* if we are running very slow, don't integrate too fast */
        dt = 500;

    DoMovement(dt);
}

void ChartCanvas::DoMovement( long dt )
{
    /* if we get here quickly assume 1ms so that some movement occurs */
    if(dt == 0)
        dt = 1;

    m_mustmove -= dt;
    if(m_mustmove < 0)
        m_mustmove = 0;

    if(m_pan_drag.x || m_pan_drag.y) {
        PanCanvas( m_pan_drag.x, m_pan_drag.y );
        m_pan_drag.x = m_pan_drag.y = 0;
    }

    if(m_panx || m_pany) {
        const double slowpan = .1, maxpan = 2;
        if( m_modkeys == wxMOD_ALT )
            m_panspeed = slowpan;
        else {
            m_panspeed += (double)dt/500; /* apply acceleration */
            m_panspeed = wxMin( maxpan, m_panspeed );
        }
        PanCanvas( m_panspeed * m_panx * dt, m_panspeed * m_pany * dt);
    }

    if(m_zoom_factor != 1) {
        double alpha = 400, beta = 1.5;
        double zoom_factor = (exp(dt / alpha) - 1) / beta + 1;

        if( m_modkeys == wxMOD_ALT )
            zoom_factor = pow(zoom_factor, .15);

        if(m_zoom_factor < 1)
            zoom_factor = 1/zoom_factor;

        //  Try to hit the zoom target exactly.
        if(m_wheelzoom_stop_oneshot > 0) {
            if(zoom_factor > 1){
                if(  VPoint.chart_scale / zoom_factor <= m_zoom_target)
                    zoom_factor = VPoint.chart_scale / m_zoom_target;
            }

            else if(zoom_factor < 1){
                if(  VPoint.chart_scale / zoom_factor >= m_zoom_target)
                    zoom_factor = VPoint.chart_scale / m_zoom_target;
            }
        }

        if( fabs(zoom_factor - 1) > 1e-4)
            DoZoomCanvas( zoom_factor, m_bzooming_to_cursor );

        if(m_wheelzoom_stop_oneshot > 0) {
            if(m_wheelstopwatch.Time() > m_wheelzoom_stop_oneshot){
                m_wheelzoom_stop_oneshot = 0;
                StopMovement( );
            }
        
            //      Don't overshoot the zoom target.
            if(zoom_factor > 1){
                if(  VPoint.chart_scale <= m_zoom_target){
                    m_wheelzoom_stop_oneshot = 0;
                    StopMovement( );
                }
            }
            else if(zoom_factor < 1){
                if(  VPoint.chart_scale >= m_zoom_target){
                    m_wheelzoom_stop_oneshot = 0;
                    StopMovement( );
                }
            }
        }
    }

    if( m_rotation_speed ) { /* in degrees per second */
        double speed = m_rotation_speed;
        if( m_modkeys == wxMOD_ALT)
            speed /= 10;
        DoRotateCanvas( VPoint.rotation + speed * PI / 180 * dt / 1000.0);
    }
}

void ChartCanvas::SetColorScheme( ColorScheme cs )
{
    //    Setup ownship image pointers
    switch( cs ) {
    case GLOBAL_COLOR_SCHEME_DAY:
        m_pos_image_red = &m_os_image_red_day;
        m_pos_image_grey = &m_os_image_grey_day;
        m_pos_image_yellow = &m_os_image_yellow_day;
        m_pos_image_user = m_pos_image_user_day;
        m_pos_image_user_grey = m_pos_image_user_grey_day;
        m_pos_image_user_yellow = m_pos_image_user_yellow_day;
        m_cTideBitmap = m_bmTideDay;
        m_cCurrentBitmap = m_bmCurrentDay;
        
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        m_pos_image_red = &m_os_image_red_dusk;
        m_pos_image_grey = &m_os_image_grey_dusk;
        m_pos_image_yellow = &m_os_image_yellow_dusk;
        m_pos_image_user = m_pos_image_user_dusk;
        m_pos_image_user_grey = m_pos_image_user_grey_dusk;
        m_pos_image_user_yellow = m_pos_image_user_yellow_dusk;
        m_cTideBitmap = m_bmTideDusk;
        m_cCurrentBitmap = m_bmCurrentDusk;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        m_pos_image_red = &m_os_image_red_night;
        m_pos_image_grey = &m_os_image_grey_night;
        m_pos_image_yellow = &m_os_image_yellow_night;
        m_pos_image_user = m_pos_image_user_night;
        m_pos_image_user_grey = m_pos_image_user_grey_night;
        m_pos_image_user_yellow = m_pos_image_user_yellow_night;
        m_cTideBitmap = m_bmTideNight;
        m_cCurrentBitmap = m_bmCurrentNight;
        break;
    default:
        m_pos_image_red = &m_os_image_red_day;
        m_pos_image_grey = &m_os_image_grey_day;
        m_pos_image_yellow = &m_os_image_yellow_day;
        m_pos_image_user = m_pos_image_user_day;
        m_pos_image_user_grey = m_pos_image_user_grey_day;
        m_pos_image_user_yellow = m_pos_image_user_yellow_day;
        m_cTideBitmap = m_bmTideDay;
        m_cCurrentBitmap = m_bmCurrentDay;
        break;
    }

    CreateDepthUnitEmbossMaps( cs );
    CreateOZEmbossMapData( cs );
    
    //  Set up fog effect base color
    m_fog_color = wxColor( 170, 195, 240 );  // this is gshhs (backgound world chart) ocean color
    float dim = 1.0;
    switch( cs ){
        case GLOBAL_COLOR_SCHEME_DUSK:
            dim = 0.5;
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            dim = 0.25;
            break;
        default:
            break;
    }
    m_fog_color.Set( m_fog_color.Red()*dim, m_fog_color.Green()*dim, m_fog_color.Blue()*dim );

    //  Really dark
    if( cs == GLOBAL_COLOR_SCHEME_DUSK || cs == GLOBAL_COLOR_SCHEME_NIGHT ) {
        SetBackgroundColour( wxColour(0,0,0) );
        
        SetWindowStyleFlag( (GetWindowStyleFlag() & ~wxSIMPLE_BORDER) | wxNO_BORDER);
    }
    else{
        SetWindowStyleFlag( (GetWindowStyleFlag() & ~wxNO_BORDER) | wxSIMPLE_BORDER);
        SetBackgroundColour( wxNullColour );
    }
        
    UpdateToolbarColorScheme( cs );
    
    m_Piano->SetColorScheme( cs );
    
    m_Compass->SetColorScheme( cs );

    if(m_muiBar)
        m_muiBar->SetColorScheme( cs );
    
    if(pWorldBackgroundChart)
        pWorldBackgroundChart->SetColorScheme( cs );
#ifdef ocpnUSE_GL
    if( g_bopengl && m_glcc ){
        m_glcc->SetColorScheme( cs );
        g_glTextureManager->ClearAllRasterTextures();
        m_glcc->FlushFBO(); 
    }
#endif
    SetbTCUpdate( true );                        // force re-render of tide/current locators
    m_brepaint_piano = true;

    ReloadVP();

    m_cs = cs;
}

wxBitmap ChartCanvas::CreateDimBitmap( wxBitmap &Bitmap, double factor )
{
    wxImage img = Bitmap.ConvertToImage();
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

    wxBitmap ret = wxBitmap( new_img );

    return ret;

}

void ChartCanvas::ShowBrightnessLevelTimedPopup( int brightness, int min, int max )
{
    wxFont *pfont = FontMgr::Get().FindOrCreateFont( 40, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );

    if( !m_pBrightPopup ) {
        //    Calculate size
        int x, y;
        GetTextExtent( _T("MAX"), &x, &y, NULL, NULL, pfont );

        m_pBrightPopup = new TimedPopupWin( this, 3);

        m_pBrightPopup->SetSize(x, y);
        m_pBrightPopup->Move(120,120);
    }

    int bmpsx = m_pBrightPopup->GetSize().x;
    int bmpsy = m_pBrightPopup->GetSize().y;

    wxBitmap bmp( bmpsx, bmpsx );
    wxMemoryDC mdc( bmp );

    mdc.SetTextForeground( GetGlobalColor( _T("GREEN4") ) );
    mdc.SetBackground( wxBrush( GetGlobalColor( _T("UINFD") ) ) );
    mdc.SetPen( wxPen( wxColour( 0, 0, 0 ) ) );
    mdc.SetBrush( wxBrush( GetGlobalColor( _T("UINFD") ) ) );
    mdc.Clear();

    mdc.DrawRectangle( 0, 0, bmpsx, bmpsy );

    mdc.SetFont( *pfont );
    wxString val;

    if( brightness == max ) val = _T("MAX");
    else
        if( brightness == min ) val = _T("MIN");
        else
            val.Printf( _T("%3d"), brightness );

    mdc.DrawText( val, 0, 0 );

    mdc.SelectObject( wxNullBitmap );

    m_pBrightPopup->SetBitmap( bmp );
    m_pBrightPopup->Show();
    m_pBrightPopup->Refresh();


}


void ChartCanvas::RotateTimerEvent( wxTimerEvent& event )
{
    m_b_rot_hidef = true;
    ReloadVP();
}


void ChartCanvas::OnRolloverPopupTimerEvent( wxTimerEvent& event )
{
#ifdef __OCPN__ANDROID__
    return;
#endif
    
    bool b_need_refresh = false;

    //  Handle the AIS Rollover Window first
    bool showAISRollover = false;
    if( g_pAIS && g_pAIS->GetNumTargets() && m_bShowAIS ) {
        SelectItem *pFind = pSelectAIS->FindSelection( this, m_cursor_lat, m_cursor_lon, SELTYPE_AISTARGET );
        if( pFind ) {
            int FoundAIS_MMSI = (wxIntPtr) pFind->m_pData1;
            AIS_Target_Data *ptarget = g_pAIS->Get_Target_Data_From_MMSI( FoundAIS_MMSI );

            if( ptarget ) {
                showAISRollover = true;

                if( NULL == m_pAISRolloverWin ) {
                    m_pAISRolloverWin = new RolloverWin( this );
                    m_pAISRolloverWin->IsActive( false );
                    b_need_refresh = true;
                }
                else if( m_pAISRolloverWin->IsActive() && m_AISRollover_MMSI && m_AISRollover_MMSI != FoundAIS_MMSI ){
                    //      Sometimes the mouse moves fast enough to get over a new AIS target before
                    //      the one-shot has fired to remove the old target.
                    //      Result:  wrong target data is shown.
                    //      Detect this case,close the existing rollover ASAP, and restart the timer.
                    m_RolloverPopupTimer.Start( 50, wxTIMER_ONE_SHOT );
                    m_pAISRolloverWin->IsActive( false );
                    m_AISRollover_MMSI = 0;
                    Refresh();
                    return;
                }

                m_AISRollover_MMSI = FoundAIS_MMSI;

                if( !m_pAISRolloverWin->IsActive() ) {

                    wxString s = ptarget->GetRolloverString();
                    m_pAISRolloverWin->SetString( s );

                    wxSize win_size = GetSize();
                    if( console && console->IsShown() ) win_size.x -= console->GetSize().x;

                    m_pAISRolloverWin->SetBestPosition( mouse_x, mouse_y, 16, 16, AIS_ROLLOVER, win_size );

                    m_pAISRolloverWin->SetBitmap( AIS_ROLLOVER );
                    m_pAISRolloverWin->IsActive( true );
                    b_need_refresh = true;
                }
            }
        }
        else {
            m_AISRollover_MMSI = 0;
            showAISRollover = false;
        }
    }

    //  Maybe turn the rollover off
    if( m_pAISRolloverWin && m_pAISRolloverWin->IsActive() && !showAISRollover ) {
        m_pAISRolloverWin->IsActive( false );
        m_AISRollover_MMSI = 0;
        b_need_refresh = true;
    }

    // Now the Route info rollover
    // Show the route segment info
    bool showRouteRollover = false;

    if( NULL == m_pRolloverRouteSeg ) {
        //    Get a list of all selectable sgements, and search for the first visible segment as the rollover target.

        SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_ROUTESEGMENT );
        wxSelectableItemListNode *node = SelList.GetFirst();
        while( node ) {
            SelectItem *pFindSel = node->GetData();

            Route *pr = (Route *) pFindSel->m_pData3;        //candidate

            if( pr && pr->IsVisible() ) {
                m_pRolloverRouteSeg = pFindSel;
                showRouteRollover = true;

                if( NULL == m_pRouteRolloverWin ) {
                    m_pRouteRolloverWin = new RolloverWin( this, 10 );
                    m_pRouteRolloverWin->IsActive( false );
                }

                if( !m_pRouteRolloverWin->IsActive() ) {
                    wxString s;
                    RoutePoint *segShow_point_a = (RoutePoint *) m_pRolloverRouteSeg->m_pData1;
                    RoutePoint *segShow_point_b = (RoutePoint *) m_pRolloverRouteSeg->m_pData2;

                    double brg, dist;
                    DistanceBearingMercator( segShow_point_b->m_lat, segShow_point_b->m_lon,
                                             segShow_point_a->m_lat, segShow_point_a->m_lon, &brg, &dist );

                    if( !pr->m_bIsInLayer )
                        s.Append( _("Route") + _T(": ") );
                    else
                        s.Append( _("Layer Route: ") );

                    if( pr->m_RouteNameString.IsEmpty() ) s.Append( _("(unnamed)") );
                    else
                        s.Append( pr->m_RouteNameString );

                    s << _T("\n") << _("Total Length: ") << FormatDistanceAdaptive( pr->m_route_length)
                    << _T("\n") << _("Leg: from ") << segShow_point_a->GetName()
                    << _(" to ") << segShow_point_b->GetName()
                    << _T("\n");

                    if( g_bShowTrue )
                        s << wxString::Format( wxString("%03d°  ", wxConvUTF8 ), (int)brg );
                    if( g_bShowMag ){
                        double latAverage = (segShow_point_b->m_lat + segShow_point_a->m_lat)/2;
                        double lonAverage = (segShow_point_b->m_lon + segShow_point_a->m_lon)/2;
                        double varBrg = gFrame->GetMag( brg, latAverage, lonAverage);
                        
                        s << wxString::Format( wxString("%03d°(M)  ", wxConvUTF8 ), (int)varBrg );
                    }

                    s << FormatDistanceAdaptive( dist );

                    // Compute and display cumulative distance from route start point to current
                    // leg end point.

                    if( segShow_point_a != pr->pRoutePointList->GetFirst()->GetData() ) {
                        wxRoutePointListNode *node = (pr->pRoutePointList)->GetFirst()->GetNext();
                        RoutePoint *prp;
                        float dist_to_endleg = 0;
                        wxString t;

                        while( node ) {
                            prp = node->GetData();
                            dist_to_endleg += prp->m_seg_len;
                            if( prp->IsSame( segShow_point_a ) ) break;
                            node = node->GetNext();
                        }
                        s << _T(" (+") << FormatDistanceAdaptive( dist_to_endleg ) << _T(")");
                    }

                    m_pRouteRolloverWin->SetString( s );

                    wxSize win_size = GetSize();
                    if( console && console->IsShown() ) win_size.x -= console->GetSize().x;
                    m_pRouteRolloverWin->SetBestPosition( mouse_x, mouse_y, 16, 16, LEG_ROLLOVER,
                                                     win_size );
                    m_pRouteRolloverWin->SetBitmap( LEG_ROLLOVER );
                    m_pRouteRolloverWin->IsActive( true );
                    b_need_refresh = true;
                    showRouteRollover = true;
                    break;
                }
            } else
                node = node->GetNext();
        }
    } else {
        //    Is the cursor still in select radius, and not timed out?
        if( !pSelect->IsSelectableSegmentSelected( this, m_cursor_lat, m_cursor_lon, m_pRolloverRouteSeg ) )
            showRouteRollover = false;
        else if(m_pRouteRolloverWin && !m_pRouteRolloverWin->IsActive())
            showRouteRollover = false;
        else
            showRouteRollover = true;
    }

    //    If currently creating a route, do not show this rollover window
    if( m_routeState )
        showRouteRollover = false;

    //    Similar for AIS target rollover window
    if( m_pAISRolloverWin && m_pAISRolloverWin->IsActive() )
        showRouteRollover = false;

    if( m_pRouteRolloverWin /*&& m_pRouteRolloverWin->IsActive()*/ && !showRouteRollover ) {
        m_pRouteRolloverWin->IsActive( false );
        m_pRolloverRouteSeg = NULL;
        m_pRouteRolloverWin->Destroy();
        m_pRouteRolloverWin = NULL;
        b_need_refresh = true;
    } else if( m_pRouteRolloverWin && showRouteRollover ) {
        m_pRouteRolloverWin->IsActive( true );
        b_need_refresh = true;
    }

    // Now the Track info rollover
    // Show the track segment info
    bool showTrackRollover = false;

    if( NULL == m_pRolloverTrackSeg ) {
        //    Get a list of all selectable sgements, and search for the first visible segment as the rollover target.

        SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_TRACKSEGMENT );
        wxSelectableItemListNode *node = SelList.GetFirst();
        while( node ) {
            SelectItem *pFindSel = node->GetData();

            Track *pt = (Track *) pFindSel->m_pData3;        //candidate

            if( pt && pt->IsVisible() ) {
                m_pRolloverTrackSeg = pFindSel;
                showTrackRollover = true;

                if( NULL == m_pTrackRolloverWin ) {
		   m_pTrackRolloverWin = new RolloverWin( this, 10 );
                    m_pTrackRolloverWin->IsActive( false );
                }

                if( !m_pTrackRolloverWin->IsActive() ) {
                    wxString s;
                    TrackPoint *segShow_point_a = (TrackPoint *) m_pRolloverTrackSeg->m_pData1;
                    TrackPoint *segShow_point_b = (TrackPoint *) m_pRolloverTrackSeg->m_pData2;

                    double brg, dist;
                    DistanceBearingMercator( segShow_point_b->m_lat, segShow_point_b->m_lon,
                                             segShow_point_a->m_lat, segShow_point_a->m_lon, &brg, &dist );

                    if( !pt->m_bIsInLayer )
                        s.Append( _("Track") + _T(": ") );
                    else
                        s.Append( _("Layer Track: ") );

                    if( pt->GetName().IsEmpty() )
                        s.Append( _("(unnamed)") );
                    else
                        s.Append( pt->GetName() );

                    s << _T("\n") << _("Total Length: ") << FormatDistanceAdaptive( pt->Length())
                    << _T("\n");

                    if( g_bShowTrue )
                        s << wxString::Format( wxString("%03d°  ", wxConvUTF8 ), (int)brg );
                    if( g_bShowMag ){
                        double latAverage = (segShow_point_b->m_lat + segShow_point_a->m_lat)/2;
                        double lonAverage = (segShow_point_b->m_lon + segShow_point_a->m_lon)/2;
                        double varBrg = gFrame->GetMag( brg, latAverage, lonAverage);

                        s << wxString::Format( wxString("%03d°(M)  ", wxConvUTF8 ), (int)varBrg );
                    }

                    s << FormatDistanceAdaptive( dist );

                    if(segShow_point_a->GetTimeString() && segShow_point_b->GetTimeString()){
                        double segmentSpeed = toUsrSpeed( dist / ( (segShow_point_b->GetCreateTime() - segShow_point_a->GetCreateTime()).GetSeconds().ToDouble() / 3600.) );
                        s << wxString::Format( _T("  %.1f "), (float)segmentSpeed ) << getUsrSpeedUnit();
                    }

                    m_pTrackRolloverWin->SetString( s );

                    wxSize win_size = GetSize();
                    if( console && console->IsShown() ) win_size.x -= console->GetSize().x;
                    m_pTrackRolloverWin->SetBestPosition( mouse_x, mouse_y, 16, 16, LEG_ROLLOVER,
                                                     win_size );
                    m_pTrackRolloverWin->SetBitmap( LEG_ROLLOVER );
                    m_pTrackRolloverWin->IsActive( true );
                    b_need_refresh = true;
                    showTrackRollover = true;
                    break;
                }
            } else
                node = node->GetNext();
        }
    } else {
        //    Is the cursor still in select radius, and not timed out?
        if( !pSelect->IsSelectableSegmentSelected( this, m_cursor_lat, m_cursor_lon, m_pRolloverTrackSeg ) )
            showTrackRollover = false;
        else if(m_pTrackRolloverWin && !m_pTrackRolloverWin->IsActive())
            showTrackRollover = false;
        else
            showTrackRollover = true;
    }

    //    Similar for AIS target rollover window
    if( m_pAISRolloverWin && m_pAISRolloverWin->IsActive() )
        showTrackRollover = false;

    //    Similar for route rollover window
    if( m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive() )
        showTrackRollover = false;

    //TODO  We onlt show tracks on primary canvas....
    if(!IsPrimaryCanvas())
        showTrackRollover = false;
    
    if( m_pTrackRolloverWin /*&& m_pTrackRolloverWin->IsActive()*/ && !showTrackRollover ) {
        m_pTrackRolloverWin->IsActive( false );
        m_pRolloverTrackSeg = NULL;
        m_pTrackRolloverWin->Destroy();
        m_pTrackRolloverWin = NULL;
        b_need_refresh = true;
    } else if( m_pTrackRolloverWin && showTrackRollover ) {
        m_pTrackRolloverWin->IsActive( true );
        b_need_refresh = true;
    }

    if( b_need_refresh )
        Refresh();
}

void ChartCanvas::OnCursorTrackTimerEvent( wxTimerEvent& event )
{
#ifdef USE_S57
    if( s57_CheckExtendedLightSectors( this, mouse_x, mouse_y, VPoint, extendedSectorLegs ) ){
        if(!m_bsectors_shown) {
            ReloadVP( false );
            m_bsectors_shown = true;
        }
    }
    else {
        if( m_bsectors_shown ) {
            ReloadVP( false );
            m_bsectors_shown = false;
        }
    }
#endif

//      This is here because GTK status window update is expensive..
//            cairo using pango rebuilds the font every time so is very inefficient
//      Anyway, only update the status bar when this timer expires
#if defined(__WXGTK__) || defined(__WXQT__)
    {
        //    Check the absolute range of the cursor position
        //    There could be a window wherein the chart geoereferencing is not valid....
        double cursor_lat, cursor_lon;
        GetCanvasPixPoint ( mouse_x, mouse_y, cursor_lat, cursor_lon );

        if((fabs(cursor_lat) < 90.) && (fabs(cursor_lon) < 360.))
        {
            while(cursor_lon < -180.)
                cursor_lon += 360.;

            while(cursor_lon > 180.)
                cursor_lon -= 360.;

            SetCursorStatus(cursor_lat, cursor_lon);
        }
    }
#endif
}

void ChartCanvas::SetCursorStatus( double cursor_lat, double cursor_lon )
{
    if ( !parent_frame->m_pStatusBar )
        return;

    wxString s1;
    s1 += _T(" ");
    s1 += toSDMM(1, cursor_lat);
    s1 += _T("   ");
    s1 += toSDMM(2, cursor_lon);
    
    if(STAT_FIELD_CURSOR_LL >= 0)
        parent_frame->SetStatusText ( s1, STAT_FIELD_CURSOR_LL );

    if( STAT_FIELD_CURSOR_BRGRNG < 0 )
        return;

    double brg, dist;
    wxString s;
    DistanceBearingMercator(cursor_lat, cursor_lon, gLat, gLon, &brg, &dist);
    if( g_bShowMag )
        s.Printf( wxString("%03d°(M)  ", wxConvUTF8 ), (int)gFrame->GetMag( brg ) );
    else
        s.Printf( wxString("%03d°  ", wxConvUTF8 ), (int)brg );
    
    s << FormatDistanceAdaptive( dist );
    
    // CUSTOMIZATION - LIVE ETA OPTION
    // -------------------------------------------------------
    // Calculate an "live" ETA based on route starting from the current
    // position of the boat and goes to the cursor of the mouse.
    // In any case, an standard ETA will be calculated with a default speed
    // of the boat to give an estimation of the route (in particular if GPS
    // is off).
    
    // Display only if option "live ETA" is selected in Settings > Display > General.
    if (g_bShowLiveETA)
    {
    
        float realTimeETA;
        float boatSpeed;
        float boatSpeedDefault = g_defaultBoatSpeed;
        
        // Calculate Estimate Time to Arrival (ETA) in minutes
        // Check before is value not closed to zero (it will make an very big number...)
        if (!std::isnan(gSog))
        {
            boatSpeed = gSog;
            if (boatSpeed < 0.5)
            {
                realTimeETA = 0;
            }
            else
            {
                realTimeETA = dist / boatSpeed * 60;
            }
        }
        else
        {
            realTimeETA = 0;
        }
        
        // Add space after distance display
        s << " ";
        // Display ETA
        s << minutesToHoursDays(realTimeETA);
        
        // In any case, display also an ETA with default speed at 6knts
        
        s << " [@";
        s << wxString::Format(_T("%d"), (int)toUsrSpeed(boatSpeedDefault, -1));
        s << wxString::Format(_T("%s"), getUsrSpeedUnit(-1));
        s << " ";
        s << minutesToHoursDays(dist/boatSpeedDefault*60);
        s << "]";
    
    }
    // END OF - LIVE ETA OPTION
    
    parent_frame->SetStatusText ( s, STAT_FIELD_CURSOR_BRGRNG );
}

// CUSTOMIZATION - FORMAT MINUTES
// -------------------------------------------------------
// New function to format minutes into a more readable format:
//  * Hours + minutes, or
//  * Days + hours.
wxString minutesToHoursDays(float timeInMinutes)
{
    wxString s;
    
    if (timeInMinutes == 0)
    {
        s << "--min";
    }
    
    // Less than 60min, keep time in minutes
    else if (timeInMinutes < 60 && timeInMinutes != 0)
    {
        s << wxString::Format(_T("%d"), (int)timeInMinutes);
        s << "min";
    }
    
    // Between 1h and less than 24h, display time in hours, minutes
    else if (timeInMinutes >= 60 && timeInMinutes < 24 * 60)
    {
        
        int hours;
        int min;
        hours = (int)timeInMinutes / 60;
        min = (int)timeInMinutes % 60;
        
        if (min == 0)
        {
            s << wxString::Format(_T("%d"), hours );
            s << "h";
        }
        else
        {
            s << wxString::Format(_T("%d"), hours );
            s << "h";
            s << wxString::Format(_T("%d"), min );
            s << "min";
        }
        
    }
    
    // More than 24h, display time in days, hours
    else if (timeInMinutes > 24 * 60)
    {
        
        int days;
        int hours;
        days = (int)(timeInMinutes / 60) / 24;
        hours = (int)(timeInMinutes / 60) % 24;
        
        if (hours == 0)
        {
            s << wxString::Format(_T("%d"), days );
            s << "d";
        }
        else
        {
            s << wxString::Format(_T("%d"), days );
            s << "d";
            s << wxString::Format(_T("%d"), hours );
            s << "h";
        }
        
    }
    
    return s;
}

// END OF CUSTOMIZATION - FORMAT MINUTES
// Thanks open source code ;-)
// -------------------------------------------------------


void ChartCanvas::GetCursorLatLon( double *lat, double *lon )
{
    double clat, clon;
    GetCanvasPixPoint( mouse_x, mouse_y, clat, clon );
    *lat = clat;
    *lon = clon;
}

void ChartCanvas::GetDoubleCanvasPointPix( double rlat, double rlon, wxPoint2DDouble *r )
{
    return GetDoubleCanvasPointPixVP( GetVP(), rlat, rlon, r );
}

void ChartCanvas::GetDoubleCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, wxPoint2DDouble *r )
{
    // If the Current Chart is a raster chart, and the
    // requested lat/long is within the boundaries of the chart,
    // and the VP is not rotated,
    // then use the embedded BSB chart georeferencing algorithm
    // for greater accuracy
    // Additionally, use chart embedded georef if the projection is TMERC
    //  i.e. NOT MERCATOR and NOT POLYCONIC
    
    // If for some reason the chart rejects the request by returning an error,
    // then fall back to Viewport Projection estimate from canvas parameters
    if( !g_bopengl && m_singleChart && ( m_singleChart->GetChartFamily() == CHART_FAMILY_RASTER )
        && ( ( ( fabs( vp.rotation ) < .0001 ) && ( fabs( vp.skew ) < .0001 ) )
        || ( ( m_singleChart->GetChartProjectionType() != PROJECTION_MERCATOR )
        && ( m_singleChart->GetChartProjectionType() != PROJECTION_TRANSVERSE_MERCATOR )
        && ( m_singleChart->GetChartProjectionType() != PROJECTION_POLYCONIC ) ) )
        && ( m_singleChart->GetChartProjectionType() == vp.m_projection_type )
        && ( m_singleChart->GetChartType() != CHART_TYPE_PLUGIN) )
    {
        ChartBaseBSB *Cur_BSB_Ch = dynamic_cast<ChartBaseBSB *>( m_singleChart );
        //                        bool bInside = G_FloatPtInPolygon ( ( MyFlPoint * ) Cur_BSB_Ch->GetCOVRTableHead ( 0 ),
        //                                                            Cur_BSB_Ch->GetCOVRTablenPoints ( 0 ), rlon, rlat );
        //                        bInside = true;
        //                        if ( bInside )
        if( Cur_BSB_Ch ) {
            //    This is a Raster chart....
            //    If the VP is changing, the raster chart parameters may not yet be setup
            //    So do that before accessing the chart's embedded georeferencing
            Cur_BSB_Ch->SetVPRasterParms( vp );
            double rpixxd, rpixyd;
            if( 0 == Cur_BSB_Ch->latlong_to_pix_vp( rlat, rlon, rpixxd, rpixyd, vp ) ) {
                r->m_x = rpixxd;
                r->m_y = rpixyd;
                return;
            }
        }
    }
    
    //    if needed, use the VPoint scaling estimator,
    *r = vp.GetDoublePixFromLL( rlat, rlon );
}


// This routine might be deleted and all of the rendering improved
// to have floating point accuracy
bool ChartCanvas::GetCanvasPointPix( double rlat, double rlon, wxPoint *r )
{
    return GetCanvasPointPixVP( GetVP(), rlat, rlon, r);
}

bool ChartCanvas::GetCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, wxPoint *r )
{
    wxPoint2DDouble p;
    GetDoubleCanvasPointPixVP(vp, rlat, rlon, &p);

    // some projections give nan values when invisible values (other side of world) are requested
    // we should stop using integer coordinates or return false here (and test it everywhere)
    if(std::isnan(p.m_x)) {
        *r = wxPoint(INVALID_COORD, INVALID_COORD);
        return false;
    }

    *r = wxPoint(wxRound(p.m_x), wxRound(p.m_y));
    return true;
}


void ChartCanvas::GetCanvasPixPoint( double x, double y, double &lat, double &lon )
{
    // If the Current Chart is a raster chart, and the
    // requested x,y is within the boundaries of the chart,
    // and the VP is not rotated,
    // then use the embedded BSB chart georeferencing algorithm
    // for greater accuracy
    // Additionally, use chart embedded georef if the projection is TMERC
    //  i.e. NOT MERCATOR and NOT POLYCONIC

    // If for some reason the chart rejects the request by returning an error,
    // then fall back to Viewport Projection  estimate from canvas parameters
    bool bUseVP = true;

    if( !g_bopengl && m_singleChart && ( m_singleChart->GetChartFamily() == CHART_FAMILY_RASTER )
        && ( ( ( fabs( GetVP().rotation ) < .0001 ) && ( fabs( GetVP().skew ) < .0001 ) )
        || ( ( m_singleChart->GetChartProjectionType() != PROJECTION_MERCATOR )
        && ( m_singleChart->GetChartProjectionType() != PROJECTION_TRANSVERSE_MERCATOR )
        && ( m_singleChart->GetChartProjectionType() != PROJECTION_POLYCONIC ) ) )
        && ( m_singleChart->GetChartProjectionType() == GetVP().m_projection_type )
        && ( m_singleChart->GetChartType() != CHART_TYPE_PLUGIN ) )
    {
        ChartBaseBSB *Cur_BSB_Ch = dynamic_cast<ChartBaseBSB *>( m_singleChart );

        // TODO     maybe need iterative process to validate bInside
        //          first pass is mercator, then check chart boundaries

        if( Cur_BSB_Ch ) {
            //    This is a Raster chart....
            //    If the VP is changing, the raster chart parameters may not yet be setup
            //    So do that before accessing the chart's embedded georeferencing
            Cur_BSB_Ch->SetVPRasterParms( GetVP() );

            double slat, slon;
            if( 0 == Cur_BSB_Ch->vp_pix_to_latlong( GetVP(), x, y, &slat, &slon ) ) {
                lat = slat;

                if( slon < -180. ) slon += 360.;
                else if( slon > 180. ) slon -= 360.;

                lon = slon;
                bUseVP = false;
            }
        }
    }

    //    if needed, use the VPoint scaling estimator
    if( bUseVP ) {
        GetVP().GetLLFromPix( wxPoint2DDouble( x, y ), &lat, &lon );
    }
}

void ChartCanvas::ZoomCanvas( double factor, bool can_zoom_to_cursor, bool stoptimer )
{
    m_bzooming_to_cursor = can_zoom_to_cursor && g_bEnableZoomToCursor;

    if( g_bsmoothpanzoom ) {
        if(StartTimedMovement(stoptimer)) {
            m_mustmove += 150; /* for quick presses register as 200 ms duration */
            m_zoom_factor = factor;
        }
        m_zoom_target =  VPoint.chart_scale / factor;
    } else {
        if( m_modkeys == wxMOD_ALT )
            factor = pow(factor, .15);
        
        DoZoomCanvas( factor, can_zoom_to_cursor );
    }

    extendedSectorLegs.clear();
}

void ChartCanvas::DoZoomCanvas( double factor,  bool can_zoom_to_cursor )
{
    // possible on startup
    if( !ChartData )
        return;
    if(!m_pCurrentStack)
        return;

    /* TODO: queue the quilted loading code to a background thread
       so yield is never called from here, and also rendering is not delayed */

    //    Cannot allow Yield() re-entrancy here
    if( m_bzooming ) return;
    m_bzooming = true;

    double old_ppm = GetVP().view_scale_ppm;

    //  Capture current cursor position for zoom to cursor
    double zlat = m_cursor_lat;
    double zlon = m_cursor_lon;

    double proposed_scale_onscreen = GetVP().chart_scale / factor; // GetCanvasScaleFactor() / ( GetVPScale() * factor );
    bool b_do_zoom = false;
    
    if(factor > 1)
    {
        b_do_zoom = true;

        double zoom_factor = factor;

        ChartBase *pc = NULL;

        if( !VPoint.b_quilt ) {
            pc = m_singleChart;
        } else {
            int new_db_index = m_pQuilt->AdjustRefOnZoomIn( proposed_scale_onscreen );
            if( new_db_index >= 0 )
                pc = ChartData->OpenChartFromDB( new_db_index, FULL_INIT );

            if(m_pCurrentStack)
                m_pCurrentStack->SetCurrentEntryFromdbIndex( new_db_index ); // highlite the correct bar entry
        }

        if( pc ) {
//             double target_scale_ppm = GetVPScale() * zoom_factor;
//             proposed_scale_onscreen = GetCanvasScaleFactor() / target_scale_ppm;
            
            //  Query the chart to determine the appropriate zoom range
            double min_allowed_scale = 800;    // Roughly, latitude dependent for mercator charts
            
            if( proposed_scale_onscreen < min_allowed_scale ) {
                if( min_allowed_scale == GetCanvasScaleFactor() / ( GetVPScale() ) ) {
                    m_zoom_factor = 1; /* stop zooming */
                    b_do_zoom = false;
                } else
                    proposed_scale_onscreen = min_allowed_scale;
            }
            
        }
        else {
            proposed_scale_onscreen = wxMax( proposed_scale_onscreen, 800.);
        }
            
        
    } else if(factor < 1) {
        double zoom_factor = 1/factor;

        b_do_zoom = true;

        ChartBase *pc = NULL;

        bool b_smallest = false;

        if( !VPoint.b_quilt ) {             // not quilted
            pc = m_singleChart;

            if( pc ) {
                //      If m_singleChart is not on the screen, unbound the zoomout
                LLBBox viewbox = VPoint.GetBBox();
//                wxBoundingBox chart_box;
                int current_index = ChartData->FinddbIndex( pc->GetFullPath() );
                double max_allowed_scale;

                max_allowed_scale = GetCanvasScaleFactor() / m_absolute_min_scale_ppm;

                //  We can allow essentially unbounded zoomout in single chart mode
//                if( ChartData->GetDBBoundingBox( current_index, &chart_box ) &&
//                    !viewbox.IntersectOut( chart_box ) )
//                    //  Clamp the minimum scale zoom-out to the value specified by the chart
//                    max_allowed_scale = wxMin(max_allowed_scale, 4.0 *
//                                              pc->GetNormalScaleMax( GetCanvasScaleFactor(),
//                                                                     GetCanvasWidth() ) );
                if(proposed_scale_onscreen > max_allowed_scale) {
                    m_zoom_factor = 1; /* stop zooming */
                    proposed_scale_onscreen = max_allowed_scale;
                }
            }

        } else {
            int new_db_index = m_pQuilt->AdjustRefOnZoomOut( proposed_scale_onscreen );
            if( new_db_index >= 0 ) pc = ChartData->OpenChartFromDB( new_db_index, FULL_INIT );

            if(m_pCurrentStack)
                m_pCurrentStack->SetCurrentEntryFromdbIndex( new_db_index ); // highlite the correct bar entry
            
            b_smallest = m_pQuilt->IsChartSmallestScale( new_db_index );

            if( b_smallest || (0 == m_pQuilt->GetExtendedStackCount()))
                proposed_scale_onscreen = wxMin(proposed_scale_onscreen,
                                                GetCanvasScaleFactor() / m_absolute_min_scale_ppm);
        }

        //set a minimum scale
        if( ( GetCanvasScaleFactor() / proposed_scale_onscreen ) < m_absolute_min_scale_ppm )
            b_do_zoom = false;
    }

    double new_scale = GetVPScale() * (GetVP().chart_scale / proposed_scale_onscreen);
    if( b_do_zoom ) {
        if( can_zoom_to_cursor && g_bEnableZoomToCursor) {
            //  Arrange to combine the zoom and pan into one operation for smoother appearance
            SetVPScale( new_scale, false );   // adjust, but deferred refresh
 
            wxPoint r;
            GetCanvasPointPix( zlat, zlon, &r );
            PanCanvas( r.x - mouse_x, r.y - mouse_y );  // this will give the Refresh()

            //ClearbFollow();      // update the follow flag
        }
        else{
            if(m_bFollow){      //  Adjust the Viewpoint to keep ownship at the same pixel point on-screen
                double offx, offy;
                toSM(GetVP().clat, GetVP().clon, gLat, gLon, &offx, &offy);

                m_OSoffsetx = offx * old_ppm;
                m_OSoffsety = offy * old_ppm;

                double nlat, nlon;
                double dx = m_OSoffsetx;
                double dy = m_OSoffsety;
                double d_east = dx / new_scale;
                double d_north = dy / new_scale;

                fromSM( d_east, d_north, gLat, gLon, &nlat, &nlon );
                SetViewPoint( nlat, nlon, new_scale, GetVP().skew, GetVP().rotation); 
            }
            else
                SetVPScale( new_scale );
        }
    }
    
    m_bzooming = false;
    
}

void ChartCanvas::RotateCanvas( double dir )
{
    if(m_bCourseUp)
        m_bCourseUp = false;

    if(g_bsmoothpanzoom) {
        if(StartTimedMovement()) {
            m_mustmove += 150; /* for quick presses register as 200 ms duration */
            m_rotation_speed = dir*60;
        }
    } else {
        double speed = dir*10;
        if( m_modkeys == wxMOD_ALT)
            speed /= 20;
        DoRotateCanvas(VPoint.rotation + PI/180 * speed);
    }
}

void ChartCanvas::DoRotateCanvas( double rotation )
{
    while(rotation < 0) rotation += 2*PI;
    while(rotation > 2*PI) rotation -= 2*PI;

    if(rotation == VPoint.rotation || std::isnan(rotation))
        return;

    SetVPRotation( rotation );
    parent_frame->UpdateRotationState( VPoint.rotation);
}

void ChartCanvas::DoTiltCanvas( double tilt )
{
    while(tilt < 0) tilt = 0;
    while(tilt > .95) tilt = .95;

    if(tilt == VPoint.tilt || std::isnan(tilt))
        return;

    VPoint.tilt = tilt;
    Refresh( false );
}

void ChartCanvas::TogglebFollow( void )
{
    if( !m_bFollow )
        SetbFollow();
    else
        ClearbFollow();
}

void ChartCanvas::ClearbFollow( void )
{
    m_bFollow = false;      // update the follow flag
    
    //    Center the screen on the GPS position, for lack of a better place
    m_vLat = gLat;
    m_vLon = gLon;
    
    #ifdef __OCPN__ANDROID__
    androidSetFollowTool(false);
    #endif
   
    if( m_toolBar )
        m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, false );
    parent_frame->SetMenubarItemState( ID_MENU_NAV_FOLLOW, false );
    
    if(m_muiBar)
        m_muiBar->SetFollowButton(false);
    
    DoCanvasUpdate();
    ReloadVP();
    parent_frame->SetChartUpdatePeriod( );
    
}

void ChartCanvas::SetbFollow( void )
{
    JumpToPosition(gLat, gLon, GetVPScale());
    m_bFollow = true;
    
    if( m_toolBar )
        m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, true );
    parent_frame->SetMenubarItemState( ID_MENU_NAV_FOLLOW, true );

    if(m_muiBar)
        m_muiBar->SetFollowButton(true);
    
    #ifdef __OCPN__ANDROID__
    androidSetFollowTool(true);
    #endif
    
    // Is the OWNSHIP on-screen?
    // If not, then reset the OWNSHIP offset to 0 (center screen)
    if( (fabs(m_OSoffsetx) > VPoint.pix_width / 2) || (fabs(m_OSoffsety) > VPoint.pix_height / 2) ){
        m_OSoffsetx = 0;
        m_OSoffsety = 0;
    }

    DoCanvasUpdate();
    ReloadVP();
    parent_frame->SetChartUpdatePeriod( );
}

void ChartCanvas::JumpToPosition( double lat, double lon, double scale )
{
    if (lon > 180.0)
        lon -= 360.0;
    m_vLat = lat;
    m_vLon = lon;
    StopMovement();
    m_bFollow = false;
    
    if( !GetQuiltMode() ) {
        double skew = 0;
        if(m_singleChart)
            skew = m_singleChart->GetChartSkew() * PI / 180.;
        SetViewPoint( lat, lon, scale, skew, GetVPRotation() );
    } else {
        SetViewPoint( lat, lon, scale, 0, GetVPRotation() );
    }
    
    ReloadVP();
    
    if( m_toolBar )
        m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, false );
  
    //TODO
//    if( g_pi_manager ) {
//        g_pi_manager->SendViewPortToRequestingPlugIns( cc1->GetVP() );
//    }
}


bool ChartCanvas::PanCanvas( double dx, double dy )
{
    if( !ChartData )
        return false;

    extendedSectorLegs.clear();

    double clat = VPoint.clat, clon = VPoint.clon;
    double dlat, dlon;
    wxPoint2DDouble p(VPoint.pix_width / 2.0, VPoint.pix_height / 2.0);

    int iters = 0;
    for(;;) {
        GetCanvasPixPoint( p.m_x + trunc(dx), p.m_y + trunc(dy), dlat, dlon );

        if(iters++ > 5)
            return false;
        if(!std::isnan(dlat))
            break;

        dx *= .5, dy *= .5;
        if(fabs(dx) < 1 && fabs(dy) < 1)
            return false;
    }

    // avoid overshooting the poles
    if(dlat > 90)
        dlat = 90;
    else if(dlat < -90)
        dlat = -90;
    
    if( dlon > 360. ) dlon -= 360.;
    if( dlon < -360. ) dlon += 360.;

    //    This should not really be necessary, but round-trip georef on some charts is not perfect,
    //    So we can get creep on repeated unidimensional pans, and corrupt chart cacheing.......

    //    But this only works on north-up projections
    // TODO: can we remove this now?
    if( ( ( fabs( GetVP().skew ) < .001 ) ) && ( fabs( GetVP().rotation ) < .001 ) ) {

        if( dx == 0 ) dlon = clon;
        if( dy == 0 ) dlat = clat;
    }

    int cur_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();

    SetViewPoint( dlat, dlon, VPoint.view_scale_ppm, VPoint.skew, VPoint.rotation );

    if( VPoint.b_quilt) {
        int new_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();
        if( ( new_ref_dbIndex != cur_ref_dbIndex ) && ( new_ref_dbIndex != -1 ) ) {
            //Tweak the scale slightly for a new ref chart
            ChartBase *pc = ChartData->OpenChartFromDB( new_ref_dbIndex, FULL_INIT );
            if( pc ) {
                double tweak_scale_ppm = pc->GetNearestPreferredScalePPM( VPoint.view_scale_ppm );
                SetVPScale( tweak_scale_ppm );
            }
        }
    }

    //  Turn off bFollow only if the ownship has left the screen
    double offx, offy;
    toSM(dlat, dlon, gLat, gLon, &offx, &offy);
    m_OSoffsetx = offx * VPoint.view_scale_ppm;
    m_OSoffsety = offy * VPoint.view_scale_ppm;
    
    if( m_bFollow && ((fabs(m_OSoffsetx) > VPoint.pix_width / 2) || (fabs(m_OSoffsety) > VPoint.pix_height / 2)) ){
        m_bFollow = false;      // update the follow flag
        if( m_toolBar )
            m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, false );
    
        if(m_muiBar)
            m_muiBar->SetFollowButton( false );
    }
    
    Refresh( false );

    pCurTrackTimer->Start( m_curtrack_timer_msec, wxTIMER_ONE_SHOT );

    return true;
}

void ChartCanvas::ReloadVP( bool b_adjust )
{
    if( g_brightness_init ) SetScreenBrightness( g_nbrightness );

    LoadVP( VPoint, b_adjust );
}

void ChartCanvas::LoadVP( ViewPort &vp, bool b_adjust )
{
#ifdef ocpnUSE_GL
    if( g_bopengl && m_glcc ) {
        m_glcc->Invalidate();
        if( m_glcc->GetSize() != GetSize() ) {
            m_glcc->SetSize( GetSize() );
        }
    }
    else
#endif
    {
        m_cache_vp.Invalidate();
        m_bm_cache_vp.Invalidate();
    }

    VPoint.Invalidate();

    if( m_pQuilt ) m_pQuilt->Invalidate();

    //  Make sure that the Selected Group is sensible...
//    if( m_groupIndex > (int) g_pGroupArray->GetCount() )
//        m_groupIndex = 0;
//    if( !CheckGroup( m_groupIndex ) )
//        m_groupIndex = 0;
    
    SetViewPoint( vp.clat, vp.clon, vp.view_scale_ppm, vp.skew, vp.rotation, vp.m_projection_type, b_adjust );

}

void ChartCanvas::SetQuiltRefChart( int dbIndex )
{
    m_pQuilt->SetReferenceChart( dbIndex );
    VPoint.Invalidate();
    m_pQuilt->Invalidate();
}

double ChartCanvas::GetBestStartScale(int dbi_hint, const ViewPort &vp)
{
    if(m_pQuilt)
        return m_pQuilt->GetBestStartScale(dbi_hint, vp);
    else
        return vp.view_scale_ppm;
}


//      Verify and adjust the current reference chart,
//      so that it will not lead to excessive overzoom or underzoom onscreen
int ChartCanvas::AdjustQuiltRefChart( void )
{
    int ret = -1;
    if(m_pQuilt){
        wxASSERT(ChartData);
        ChartBase *pc = ChartData->OpenChartFromDB( m_pQuilt->GetRefChartdbIndex(), FULL_INIT );
        if( pc ) {
            double min_ref_scale = pc->GetNormalScaleMin( m_canvas_scale_factor, false );
            double max_ref_scale = pc->GetNormalScaleMax( m_canvas_scale_factor, m_canvas_width );
            
            if( VPoint.chart_scale < min_ref_scale )  {
                ret = m_pQuilt->AdjustRefOnZoomIn( VPoint.chart_scale );
            }
            else if( VPoint.chart_scale > max_ref_scale )  {
                ret = m_pQuilt->AdjustRefOnZoomOut( VPoint.chart_scale );
            }
            else {
                bool brender_ok = IsChartLargeEnoughToRender( pc, VPoint );
                
                int ref_family = pc->GetChartFamily();
                
                if( !brender_ok ) {
                    unsigned int target_stack_index = 0;
                    int target_stack_index_check = m_pQuilt->GetExtendedStackIndexArray()[m_pQuilt->GetRefChartdbIndex()]; // Lookup
                    
                    if( wxNOT_FOUND != target_stack_index_check )
                        target_stack_index = target_stack_index_check;
                    
                    int extended_array_count = m_pQuilt->GetExtendedStackIndexArray().size();
                    while( ( !brender_ok )  && ( (int)target_stack_index < ( extended_array_count - 1 ) ) ) {
                        target_stack_index++;
                        int test_db_index = m_pQuilt->GetExtendedStackIndexArray()[target_stack_index];
                    
                        if( ( ref_family == ChartData->GetDBChartFamily( test_db_index ) )
                            && IsChartQuiltableRef( test_db_index ) ) {
                        //    open the target, and check the min_scale
                            ChartBase *ptest_chart = ChartData->OpenChartFromDB( test_db_index, FULL_INIT );
                            if( ptest_chart ){
                                brender_ok = IsChartLargeEnoughToRender( ptest_chart, VPoint );
                            }
                        }
                    }
                    
                    if(brender_ok){             // found a better reference chart
                        int new_db_index = m_pQuilt->GetExtendedStackIndexArray()[target_stack_index];
                        if( ( ref_family == ChartData->GetDBChartFamily( new_db_index ) )
                            && IsChartQuiltableRef( new_db_index ) ) {
                            m_pQuilt->SetReferenceChart( new_db_index );
                            ret = new_db_index;
                        }
                        else
                            ret =m_pQuilt->GetRefChartdbIndex();
                    }
                    else
                        ret = m_pQuilt->GetRefChartdbIndex();
                    
                }
                else
                    ret = m_pQuilt->GetRefChartdbIndex();
            }
        }
        else
            ret = -1;
    }
    
    return ret;
}


void ChartCanvas::UpdateCanvasOnGroupChange( void )
{
    delete m_pCurrentStack;
    m_pCurrentStack = NULL;
    m_pCurrentStack = new ChartStack;
    wxASSERT(ChartData);
    ChartData->BuildChartStack( m_pCurrentStack, VPoint.clat, VPoint.clon, m_groupIndex );

    if( m_pQuilt ) {
        m_pQuilt->Compose( VPoint );
        SetFocus();
    }
}

bool ChartCanvas::SetViewPointByCorners( double latSW, double lonSW, double latNE, double lonNE )
{
    // Center Point
    double latc = (latSW + latNE)/2.0;
    double lonc = (lonSW + lonNE)/2.0;
    
    // Get scale in ppm (latitude)
    double ne_easting, ne_northing;
    toSM( latNE, lonNE, latc, lonc, &ne_easting, &ne_northing );
    
    double sw_easting, sw_northing;
    toSM( latSW, lonSW, latc, lonc, &sw_easting, &sw_northing );
    
    double scale_ppm = VPoint.pix_height / fabs(ne_northing - sw_northing);
        
    return SetViewPoint( latc, lonc, scale_ppm, VPoint.skew, VPoint.rotation );
}

bool ChartCanvas::SetVPScale( double scale, bool refresh )
{
    return SetViewPoint( VPoint.clat, VPoint.clon, scale, VPoint.skew, VPoint.rotation,
                         VPoint.m_projection_type, true, refresh );
}

bool ChartCanvas::SetVPProjection( int projection )
{
    if(!g_bopengl) // alternative projections require opengl
        return false;

    // the view scale varies depending on geographic location and projection
    // rescale to keep the relative scale on the screen the same
    double prev_true_scale_ppm = m_true_scale_ppm;
    return SetViewPoint( VPoint.clat, VPoint.clon, VPoint.view_scale_ppm, VPoint.skew, VPoint.rotation, projection ) &&
        SetVPScale(wxMax(VPoint.view_scale_ppm * prev_true_scale_ppm / m_true_scale_ppm, m_absolute_min_scale_ppm));
}

bool ChartCanvas::SetViewPoint( double lat, double lon )
{
    return SetViewPoint( lat, lon, VPoint.view_scale_ppm, VPoint.skew, VPoint.rotation );
}

bool ChartCanvas::SetViewPoint( double lat, double lon, double scale_ppm, double skew,
                                double rotation, int projection, bool b_adjust, bool b_refresh )
{
    bool b_ret = false;

    if(skew > PI) /* so our difference tests work, put in range of +-Pi */
        skew -= 2*PI;

    //  Any sensible change?
    if (VPoint.IsValid()) {
        if( ( fabs( VPoint.view_scale_ppm - scale_ppm )/ scale_ppm < 1e-5 )
            && ( fabs( VPoint.skew - skew ) < 1e-9 )
            && ( fabs( VPoint.rotation - rotation ) < 1e-9 )
            && ( fabs( VPoint.clat - lat ) < 1e-9 )
            && ( fabs( VPoint.clon - lon ) < 1e-9 )
            && (VPoint.m_projection_type == projection || projection == PROJECTION_UNKNOWN) ) 
                return false;
    }
    
    if(VPoint.m_projection_type != projection)
        VPoint.InvalidateTransformCache(); // invalidate

    //    Take a local copy of the last viewport
    ViewPort last_vp = VPoint;

    VPoint.skew = skew;
    VPoint.clat = lat;
    VPoint.clon = lon;
    VPoint.view_scale_ppm = scale_ppm;
    if(projection != PROJECTION_UNKNOWN)
        VPoint.SetProjectionType(projection);
    else
        if(VPoint.m_projection_type == PROJECTION_UNKNOWN)
            VPoint.SetProjectionType(PROJECTION_MERCATOR);

    // don't allow latitude above 88 for mercator (90 is infinity)
    if(VPoint.m_projection_type == PROJECTION_MERCATOR ||
       VPoint.m_projection_type == PROJECTION_TRANSVERSE_MERCATOR) {
        if(VPoint.clat > 89.5) VPoint.clat = 89.5;
        else if(VPoint.clat < -89.5) VPoint.clat = -89.5;
    }

    // don't zoom out too far for transverse mercator polyconic until we resolve issues
    if(VPoint.m_projection_type == PROJECTION_POLYCONIC ||
       VPoint.m_projection_type == PROJECTION_TRANSVERSE_MERCATOR)
        VPoint.view_scale_ppm = wxMax(VPoint.view_scale_ppm, 2e-4);

    SetVPRotation( rotation );

    if(!g_bopengl) // tilt is not possible without opengl
        VPoint.tilt = 0;

    if( ( VPoint.pix_width <= 0 ) || ( VPoint.pix_height <= 0 ) )    // Canvas parameters not yet set
        return false;

    VPoint.Validate();                     // Mark this ViewPoint as OK

    //  Has the Viewport scale changed?  If so, invalidate the vp
    if( last_vp.view_scale_ppm != scale_ppm ) {
        m_cache_vp.Invalidate();
        InvalidateGL();
    }

    //  A preliminary value, may be tweaked below
    VPoint.chart_scale = m_canvas_scale_factor / ( scale_ppm );

    // recompute cursor position

    GetCursorLatLon(&m_cursor_lat, &m_cursor_lon);

    if(g_pi_manager) g_pi_manager->SendCursorLatLonToAllPlugIns( m_cursor_lat, m_cursor_lon );

    if( !VPoint.b_quilt && m_singleChart ) {

        VPoint.SetBoxes();

        //  Allow the chart to adjust the new ViewPort for performance optimization
        //  This will normally be only a fractional (i.e.sub-pixel) adjustment...
        if( b_adjust ) m_singleChart->AdjustVP( last_vp, VPoint );

        // If there is a sensible change in the chart render, refresh the whole screen
        if( ( !m_cache_vp.IsValid() ) || ( m_cache_vp.view_scale_ppm != VPoint.view_scale_ppm ) ) {
            Refresh( false );
            b_ret = true;
        } else {
            wxPoint cp_last, cp_this;
            GetCanvasPointPix( m_cache_vp.clat, m_cache_vp.clon, &cp_last );
            GetCanvasPointPix( VPoint.clat, VPoint.clon, &cp_this );

            if( cp_last != cp_this ) {
                Refresh( false );
                b_ret = true;
            }
        }
        //  Create the stack
        if( m_pCurrentStack ) {
            assert(ChartData != 0);
            int current_db_index;
            current_db_index = m_pCurrentStack->GetCurrentEntrydbIndex();       // capture the current

            ChartData->BuildChartStack( m_pCurrentStack, lat, lon, current_db_index, m_groupIndex);
            m_pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );
        }

        if(!g_bopengl)
            VPoint.b_MercatorProjectionOverride = false;
    }

    //  Handle the quilted case
    if( VPoint.b_quilt) {

        if( last_vp.view_scale_ppm != scale_ppm ) m_pQuilt->InvalidateAllQuiltPatchs();

        //  Create the quilt
        if( ChartData /*&& ChartData->IsValid()*/ ) {
            if( !m_pCurrentStack ) return false;

            int current_db_index;
            current_db_index = m_pCurrentStack->GetCurrentEntrydbIndex();       // capture the current

            ChartData->BuildChartStack( m_pCurrentStack, lat, lon, m_groupIndex );
            m_pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );

            //   Check to see if the current quilt reference chart is in the new stack
            int current_ref_stack_index = -1;
            for( int i = 0; i < m_pCurrentStack->nEntry; i++ ) {
                if( m_pQuilt->GetRefChartdbIndex() == m_pCurrentStack->GetDBIndex( i ) ) current_ref_stack_index =
                        i;
            }

            if( g_bFullScreenQuilt ) {
                current_ref_stack_index = m_pQuilt->GetRefChartdbIndex();
            }
            
            //We might need a new Reference Chart
            bool b_needNewRef = false;

            //    If the new stack does not contain the current ref chart....
            if( ( -1 == current_ref_stack_index ) && ( m_pQuilt->GetRefChartdbIndex() >= 0 ) )
                b_needNewRef = true;
            
            // Would the current Ref Chart be excessively underzoomed?
            // We need to check this here to be sure, since we cannot know where the reference chart was assigned.
            // For instance, the reference chart may have been selected from the config file,
            // or from a long jump with a chart family switch implicit.
            // Anyway, we check to be sure....    
            bool renderable = true;
            ChartBase* referenceChart = ChartData->OpenChartFromDB( m_pQuilt->GetRefChartdbIndex(), FULL_INIT );
            if( referenceChart ) {
                double chartMaxScale = referenceChart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
                renderable = chartMaxScale * 64 >= VPoint.chart_scale;
            }
            if( !renderable )
                b_needNewRef = true;
            
                          

            //    Need new refchart?
            if( b_needNewRef ) {
                const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry(
                                                     m_pQuilt->GetRefChartdbIndex() );
                int target_scale = cte_ref.GetScale();
                int target_type = cte_ref.GetChartType();
                int candidate_stack_index;

                //    reset the ref chart in a way that does not lead to excessive underzoom, for performance reasons
                //    Try to find a chart that is the same type, and has a scale of just smaller than the current ref chart

                candidate_stack_index = 0;
                while( candidate_stack_index <= m_pCurrentStack->nEntry - 1 ) {
                    const ChartTableEntry &cte_candidate = ChartData->GetChartTableEntry(
                        m_pCurrentStack->GetDBIndex( candidate_stack_index ) );
                    int candidate_scale = cte_candidate.GetScale();
                    int candidate_type = cte_candidate.GetChartType();

                    if( ( candidate_scale >= target_scale ) && ( candidate_type == target_type ) ){
                        bool renderable = true;
                        ChartBase* tentative_referenceChart = ChartData->OpenChartFromDB( m_pCurrentStack->GetDBIndex( candidate_stack_index ),
                                                                                FULL_INIT );
                        if( tentative_referenceChart ) {
                            double chartMaxScale = tentative_referenceChart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
                            renderable = chartMaxScale*1.5 > VPoint.chart_scale;
                        }
                        
                        if(renderable)
                            break;
                    }

                    candidate_stack_index++;
                }

                //    If that did not work, look for a chart of just larger scale and same type
                if( candidate_stack_index >= m_pCurrentStack->nEntry ) {
                    candidate_stack_index = m_pCurrentStack->nEntry - 1;
                    while( candidate_stack_index >= 0 ) {
                        int idx = m_pCurrentStack->GetDBIndex( candidate_stack_index );
                        if ( idx >= 0) {
                            const ChartTableEntry &cte_candidate = ChartData->GetChartTableEntry(idx);
                            int candidate_scale = cte_candidate.GetScale();
                            int candidate_type = cte_candidate.GetChartType();

                            if( ( candidate_scale <= target_scale ) && ( candidate_type == target_type ) ) 
                                break;
                        }
                        candidate_stack_index--;
                    }
                }

                // and if that did not work, chose stack entry 0
                if( ( candidate_stack_index >= m_pCurrentStack->nEntry )
                        || ( candidate_stack_index < 0 ) ) candidate_stack_index = 0;

                int new_ref_index = m_pCurrentStack->GetDBIndex( candidate_stack_index );

                m_pQuilt->SetReferenceChart( new_ref_index ); //maybe???

            }

            if(!g_bopengl) {
                // Preset the VPoint projection type to match what the quilt projection type will be
                int ref_db_index = m_pQuilt->GetRefChartdbIndex(), proj;

                // Always keep the default Mercator projection if the reference chart is
                // not in the PatchList or the scale is too small for it to render.

                bool renderable = true;
                ChartBase* referenceChart = ChartData->OpenChartFromDB( ref_db_index, FULL_INIT );
                if( referenceChart ) {
                    double chartMaxScale = referenceChart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
                    renderable = chartMaxScale*1.5 > VPoint.chart_scale;
                    proj = ChartData->GetDBChartProj( ref_db_index );
                } else
                    proj = PROJECTION_MERCATOR;

                VPoint.b_MercatorProjectionOverride = ( m_pQuilt->GetnCharts() == 0 || !renderable );

                if( VPoint.b_MercatorProjectionOverride )
                    proj = PROJECTION_MERCATOR;

                VPoint.SetProjectionType( proj );
            }

            VPoint.SetBoxes();

            //    If this quilt will be a perceptible delta from the existing quilt, then refresh the entire screen
            if( m_pQuilt->IsQuiltDelta( VPoint ) ) {
                //  Allow the quilt to adjust the new ViewPort for performance optimization
                //  This will normally be only a fractional (i.e. sub-pixel) adjustment...
                if( b_adjust ) m_pQuilt->AdjustQuiltVP( last_vp, VPoint );

//                ChartData->ClearCacheInUseFlags();
//                unsigned long hash1 = m_pQuilt->GetXStackHash();
 
//                wxStopWatch sw;
                m_pQuilt->Compose( VPoint );
//                printf("comp time %ld\n", sw.Time());

                //      If the extended chart stack has changed, invalidate any cached render bitmap
//                if(m_pQuilt->GetXStackHash() != hash1) {
//                    m_bm_cache_vp.Invalidate();
//                    InvalidateGL();
//                }

                ChartData->PurgeCacheUnusedCharts( 0.7 );

                if(b_refresh)
                    Refresh( false );

                b_ret = true;
            }
        }

        VPoint.skew = 0.;  // Quilting supports 0 Skew
    } else
        if(!g_bopengl) {
            OcpnProjType projection = PROJECTION_UNKNOWN;
            if(m_singleChart) // viewport projection must match chart projection without opengl
                projection = m_singleChart->GetChartProjectionType(); 
            if(projection == PROJECTION_UNKNOWN)
                projection = PROJECTION_MERCATOR;
            VPoint.SetProjectionType(projection);
        }

    //  Has the Viewport projection changed?  If so, invalidate the vp
    if( last_vp.m_projection_type != VPoint.m_projection_type ) {
        m_cache_vp.Invalidate();
        InvalidateGL();
    }

    UpdateCanvasControlBar();           // Refresh the Piano

    VPoint.chart_scale = 1.0;           // fallback default value
    
    if( !VPoint.GetBBox().GetValid() ) VPoint.SetBoxes();

    if( VPoint.GetBBox().GetValid() ) {

        //      Update the viewpoint reference scale
        if( m_singleChart )
            VPoint.ref_scale = m_singleChart->GetNativeScale();
        else 
            VPoint.ref_scale = m_pQuilt->GetRefNativeScale();

        //    Calculate the on-screen displayed actual scale
        //    by a simple traverse northward from the center point
        //    of roughly one eighth of the canvas height
        wxPoint2DDouble r, r1;

        double delta_check = (VPoint.pix_height / VPoint.view_scale_ppm) / (1852. * 60);
        delta_check /= 8.;
        
        double check_point = wxMin(89., VPoint.clat);
            
        while((delta_check + check_point) > 90.)
            delta_check /= 2.;
            
        double rhumbDist;
        DistanceBearingMercator( check_point, VPoint.clon,
                                 check_point + delta_check, VPoint.clon,
                                     0, &rhumbDist );
                           
        GetDoubleCanvasPointPix( check_point, VPoint.clon, &r1 );
        GetDoubleCanvasPointPix( check_point + delta_check, VPoint.clon, &r );
        double delta_p = sqrt( ((r1.m_y - r.m_y) * (r1.m_y - r.m_y)) + ((r1.m_x - r.m_x) * (r1.m_x - r.m_x)) );
        
        m_true_scale_ppm = delta_p / (rhumbDist * 1852);
        
        //        A fall back in case of very high zoom-out, giving delta_y == 0
        //        which can probably only happen with vector charts
        if( 0.0 == m_true_scale_ppm )
            m_true_scale_ppm = scale_ppm;

        //        Another fallback, for highly zoomed out charts
        //        This adjustment makes the displayed TrueScale correspond to the
        //        same algorithm used to calculate the chart zoom-out limit for ChartDummy.
        if( scale_ppm < 1e-4 )
            m_true_scale_ppm = scale_ppm;

        if( m_true_scale_ppm )
            VPoint.chart_scale = m_canvas_scale_factor / ( m_true_scale_ppm );
        else
            VPoint.chart_scale = 1.0;

            
        // Create a nice renderable string
        double round_factor = 100.;
        if(VPoint.chart_scale < 1000.)
            round_factor = 10.;
        else if (VPoint.chart_scale < 10000.)
            round_factor = 50.;
            
        double true_scale_display =  wxRound(VPoint.chart_scale / round_factor ) * round_factor;
        wxString text;

        m_displayed_scale_factor = VPoint.ref_scale / VPoint.chart_scale;
            
        if( m_displayed_scale_factor > 10.0 )
            text.Printf( _T("%s %4.0f (%1.0fx)"), _("Scale"), true_scale_display, m_displayed_scale_factor );
        else if( m_displayed_scale_factor > 1.0 )
            text.Printf( _T("%s %4.0f (%1.1fx)"), _("Scale"), true_scale_display, m_displayed_scale_factor );
        else if( m_displayed_scale_factor > 0.1 ){
            double sfr = wxRound(m_displayed_scale_factor * 10.) / 10.;
            text.Printf( _T("%s %4.0f (%1.2fx)"), _("Scale"), true_scale_display, sfr );
        }
        else if( m_displayed_scale_factor > 0.01 ){
            double sfr = wxRound(m_displayed_scale_factor * 100.) / 100.;
            text.Printf( _T("%s %4.0f (%1.2fx)"), _("Scale"), true_scale_display, sfr );
        }
        else  {
            text.Printf( _T("%s %4.0f (---)"), _("Scale"), true_scale_display );      // Generally, no chart, so no chart scale factor
        }

#ifdef ocpnUSE_GL
            if( g_bopengl && g_bShowFPS){
                wxString fps_str;
                double fps = 0.;
                if( g_gl_ms_per_frame > 0){
                    fps = 1000./ g_gl_ms_per_frame;
                    fps_str.Printf(_T("  %3d fps"), (int)fps);
                }
                text += fps_str;
            }
#endif            
        
        m_scaleValue = true_scale_display;
        m_scaleText = text;
        if(m_muiBar)
            m_muiBar->UpdateDynamicValues();
        
        if( m_bShowScaleInStatusBar && parent_frame->GetStatusBar() && (parent_frame->GetStatusBar()->GetFieldsCount() > STAT_FIELD_SCALE) ) {
                // Check to see if the text will fit in the StatusBar field...
                bool b_noshow = false;
                {
                    int w = 0;
                    int h;
                    wxClientDC dc(parent_frame->GetStatusBar());
                    if( dc.IsOk() ){
                        wxFont* templateFont = FontMgr::Get().GetFont( _("StatusBar"), 0 );
                        dc.SetFont(*templateFont);
                        dc.GetTextExtent(text, &w, &h);
                        

                    // If text is too long for the allocated field, try to reduce the text string a bit.
                        wxRect rect;
                        parent_frame->GetStatusBar()->GetFieldRect(STAT_FIELD_SCALE, rect);
                        if(w && w > rect.width){
                            text.Printf( _T("%s (%1.1fx)"), _("Scale"), m_displayed_scale_factor );
                        }
                    
                    //  Test again...if too big still, then give it up.
                        dc.GetTextExtent(text, &w, &h);
                    
                        if(w && w > rect.width){
                            b_noshow = true;
                        }
                    }
                }
                
                if(!b_noshow)
                    parent_frame->SetStatusText( text, STAT_FIELD_SCALE );
        }
            
         
    }

    //  Maintain member vLat/vLon
    m_vLat = VPoint.clat;
    m_vLon = VPoint.clon;

    return b_ret;
}


//          Static Icon definitions for some symbols requiring scaling/rotation/translation
//          Very specific wxDC draw commands are necessary to properly render these icons...See the code in ShipDraw()

//      This icon was adapted and scaled from the S52 Presentation Library version 3_03.
//     Symbol VECGND02

static int s_png_pred_icon[] = { -10, -10, -10, 10, 10, 10, 10, -10 };

//      This ownship icon was adapted and scaled from the S52 Presentation Library version 3_03
//      Symbol OWNSHP05
static int s_ownship_icon[] = { 5, -42, 11, -28, 11, 42, -11, 42, -11, -28, -5, -42, -11, 0, 11, 0,
                                0, 42, 0, -42
                              };

wxColour ChartCanvas::PredColor()
{ 
    //  RAdjust predictor color change on LOW_ACCURACY ship state in interests of visibility.
    if( SHIP_NORMAL == m_ownship_state )
        return GetGlobalColor( _T ( "URED" ) );

    else if( SHIP_LOWACCURACY == m_ownship_state ) 
        return GetGlobalColor( _T ( "YELO1" ) );
         
    return GetGlobalColor( _T ( "NODTA" ) );
}

wxColour ChartCanvas::ShipColor()
{ 
    //      Establish ship color
    //     It changes color based on GPS and Chart accuracy/availability

    if( SHIP_NORMAL != m_ownship_state )
        return GetGlobalColor( _T ( "GREY1" ) );

    if( SHIP_LOWACCURACY == m_ownship_state )
        return GetGlobalColor( _T ( "YELO1" ) );

    return GetGlobalColor( _T ( "URED" ) );         // default is OK
}

void ChartCanvas::ShipDrawLargeScale( ocpnDC& dc, wxPoint lShipMidPoint )
{

    dc.SetPen( wxPen( PredColor(), 2 ) );

    if( SHIP_NORMAL == m_ownship_state )
        dc.SetBrush( wxBrush( ShipColor(), wxBRUSHSTYLE_TRANSPARENT ) );
    else
        dc.SetBrush( wxBrush( GetGlobalColor( _T ( "YELO1" ) ) ) );
 
    dc.DrawEllipse( lShipMidPoint.x - 10, lShipMidPoint.y - 10, 20, 20 );
    dc.DrawEllipse( lShipMidPoint.x - 6, lShipMidPoint.y - 6, 12, 12 );

    dc.DrawLine( lShipMidPoint.x - 12, lShipMidPoint.y, lShipMidPoint.x + 12, lShipMidPoint.y );
    dc.DrawLine( lShipMidPoint.x, lShipMidPoint.y - 12, lShipMidPoint.x, lShipMidPoint.y + 12 );
}

void ChartCanvas::ShipIndicatorsDraw( ocpnDC& dc, int img_height,
                                      wxPoint GPSOffsetPixels, wxPoint lGPSPoint)
{
    // Develop a uniform length for course predictor line dash length, based on physical display size
    // Use this reference length to size all other graphics elements
    float ref_dim = m_display_size_mm / 24;
    ref_dim = wxMin(ref_dim, 12);
    ref_dim = wxMax(ref_dim, 6);
    
    wxColour cPred = PredColor();
    
    //  Establish some graphic element line widths dependent on the platform display resolution
    //double nominal_line_width_pix = wxMax(1.0, floor(g_Platform->GetDisplayDPmm() / 2));             //0.5 mm nominal, but not less than 1 pixel
    double nominal_line_width_pix = wxMax(1.0, floor(m_pix_per_mm / 2));             //0.5 mm nominal, but not less than 1 pixel
    
    
    // If the calculated value is greater than the config file spec value, then use it.
    if(nominal_line_width_pix > g_cog_predictor_width)
        g_cog_predictor_width = nominal_line_width_pix;
    
    //    Calculate ownship Position Predictor
    wxPoint lPredPoint, lHeadPoint;

    float pCog = std::isnan(gCog) ? 0 : gCog;
    float pSog = std::isnan(gSog) ? 0 : gSog;

    double pred_lat, pred_lon;
    ll_gc_ll( gLat, gLon, pCog, pSog * g_ownship_predictor_minutes / 60., &pred_lat, &pred_lon );
    GetCanvasPointPix( pred_lat, pred_lon, &lPredPoint );
    
    // test to catch the case where COG/HDG line crosses the screen
    LLBBox box;

//    Should we draw the Head vector?
//    Compare the points lHeadPoint and lPredPoint
//    If they differ by more than n pixels, and the head vector is valid, then render the head vector


    float ndelta_pix = 10.;
    double hdg_pred_lat, hdg_pred_lon;
    bool b_render_hdt = false;
    if( !std::isnan( gHdt ) ) {
        //    Calculate ownship Heading pointer as a predictor
        ll_gc_ll( gLat, gLon, gHdt, g_ownship_HDTpredictor_miles, &hdg_pred_lat,
                  &hdg_pred_lon );
        GetCanvasPointPix( hdg_pred_lat, hdg_pred_lon, &lHeadPoint );
        float dist = sqrtf( powf(  (float) (lHeadPoint.x - lPredPoint.x), 2) +
                            powf(  (float) (lHeadPoint.y - lPredPoint.y), 2) );
        if( dist > ndelta_pix /*&& !std::isnan(gSog)*/ ) {
            box.SetFromSegment(gLat, gLon, hdg_pred_lat, hdg_pred_lon);
            if( !GetVP().GetBBox().IntersectOut(box))
                b_render_hdt = true;
        }
    }

    // draw course over ground if they are longer than the ship
    wxPoint lShipMidPoint;
    lShipMidPoint.x = lGPSPoint.x + GPSOffsetPixels.x;
    lShipMidPoint.y = lGPSPoint.y + GPSOffsetPixels.y;
    float lpp = sqrtf( powf( (float) (lPredPoint.x - lShipMidPoint.x), 2) +
                       powf( (float) (lPredPoint.y - lShipMidPoint.y), 2) );

    if(lpp >= img_height / 2) {
        box.SetFromSegment(gLat, gLon, pred_lat, pred_lon);
        if( !GetVP().GetBBox().IntersectOut(box) && !std::isnan(gCog) && !std::isnan(gSog) ) {
            
            //      COG Predictor
            float dash_length = ref_dim;
            wxDash dash_long[2];
            dash_long[0] = (int) ( floor(g_Platform->GetDisplayDPmm() * dash_length) / g_cog_predictor_width );  // Long dash , in mm <---------+
            dash_long[1] = dash_long[0] / 2.0;                                                                   // Short gap 
            
            // On ultra-hi-res displays, do not allow the dashes to be greater than 250, since it is defined as (char)
            if( dash_length > 250.){
                dash_long[0] = 250. /g_cog_predictor_width;
                dash_long[1] = dash_long[0] / 2;
            }
                
            wxPen ppPen2( cPred, g_cog_predictor_width, wxPENSTYLE_USER_DASH );
            ppPen2.SetDashes( 2, dash_long );
            dc.SetPen( ppPen2 );
            dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                               lPredPoint.x + GPSOffsetPixels.x, lPredPoint.y + GPSOffsetPixels.y );
                
            if( g_cog_predictor_width > 1 ) {
                float line_width = g_cog_predictor_width/3.;
                    
                 wxDash dash_long3[2];
                 dash_long3[0] = g_cog_predictor_width / line_width * dash_long[0];
                 dash_long3[1] = g_cog_predictor_width / line_width * dash_long[1];
                    
                 wxPen ppPen3( GetGlobalColor( _T ( "UBLCK" ) ), wxMax(1, line_width), wxPENSTYLE_USER_DASH );
                 ppPen3.SetDashes( 2, dash_long3 );
                 dc.SetPen( ppPen3 );
                 dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                               lPredPoint.x + GPSOffsetPixels.x, lPredPoint.y + GPSOffsetPixels.y );
            }
                
                
                // Prepare COG predictor endpoint icon
            double png_pred_icon_scale_factor = .4;
            if(g_ShipScaleFactorExp > 1.0)
                png_pred_icon_scale_factor *= (log(g_ShipScaleFactorExp) + 1.0) * 1.1;   
                
            wxPoint icon[4];
                
            float cog_rad = atan2f( (float) ( lPredPoint.y - lShipMidPoint.y ),
                                    (float) ( lPredPoint.x - lShipMidPoint.x ) );
            cog_rad += (float)PI;
                
            for( int i = 0; i < 4; i++ ) {
                int j = i * 2;
                double pxa = (double) ( s_png_pred_icon[j] );
                double pya = (double) ( s_png_pred_icon[j + 1] );
                   
                pya *= png_pred_icon_scale_factor;
                pxa *= png_pred_icon_scale_factor;
                    
                double px = ( pxa * sin( cog_rad ) ) + ( pya * cos( cog_rad ) );
                double py = ( pya * sin( cog_rad ) ) - ( pxa * cos( cog_rad ) );
                    
                icon[i].x = (int) wxRound( px ) + lPredPoint.x + GPSOffsetPixels.x;
                icon[i].y = (int) wxRound( py ) + lPredPoint.y + GPSOffsetPixels.y;
            }
                
                // Render COG endpoint icon
            wxPen ppPen1( GetGlobalColor( _T ( "UBLCK" ) ), g_cog_predictor_width/2, wxPENSTYLE_SOLID );
            dc.SetPen( ppPen1 );
            dc.SetBrush( wxBrush( cPred ) ); 
                
            dc.StrokePolygon( 4, icon );
        }
    }
        
        //      HDT Predictor
        if( b_render_hdt ) {
             float hdt_dash_length = ref_dim * 0.4;
            
            float hdt_width = g_cog_predictor_width * 0.8;
            wxDash dash_short[2];
            dash_short[0] = (int) ( floor(g_Platform->GetDisplayDPmm() * hdt_dash_length) / hdt_width );  // Short dash , in mm <---------+
            dash_short[1] = (int) ( floor(g_Platform->GetDisplayDPmm() * hdt_dash_length * 0.9 ) / hdt_width );  // Short gap            |
            
            wxPen ppPen2( cPred, hdt_width, wxPENSTYLE_USER_DASH );
            ppPen2.SetDashes( 2, dash_short );
            
            dc.SetPen( ppPen2 );
            dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                           lHeadPoint.x + GPSOffsetPixels.x, lHeadPoint.y + GPSOffsetPixels.y );
            
            wxPen ppPen1( cPred, g_cog_predictor_width/3, wxPENSTYLE_SOLID );
            dc.SetPen( ppPen1 );
            dc.SetBrush( wxBrush( GetGlobalColor( _T ( "GREY2" ) ) ) );
            
            double nominal_circle_size_pixels = wxMax(4.0, floor(m_pix_per_mm * (ref_dim / 5.0)));    // not less than 4 pixel
            
            // Scale the circle to ChartScaleFactor, slightly softened....
            if(g_ShipScaleFactorExp > 1.0)
                nominal_circle_size_pixels *= (log(g_ShipScaleFactorExp) + 1.0) * 1.1;   
            
            dc.StrokeCircle( lHeadPoint.x + GPSOffsetPixels.x, lHeadPoint.y + GPSOffsetPixels.y, nominal_circle_size_pixels/2 );
        }
        
        // Draw radar rings if activated
        if( g_iNavAidRadarRingsNumberVisible ) {
            double factor = 1.00;
            if( g_pNavAidRadarRingsStepUnits == 1 )          // nautical miles
            factor = 1 / 1.852;
            
            factor *= g_fNavAidRadarRingsStep;
            
            double tlat, tlon;
            wxPoint r;
            ll_gc_ll( gLat, gLon, 0, factor, &tlat, &tlon );
            GetCanvasPointPix( tlat, tlon, &r );
            
            double lpp = sqrt( pow( (double) (lGPSPoint.x - r.x), 2) +
            pow( (double) (lGPSPoint.y - r.y), 2 ) );
            int pix_radius = (int) lpp;
            
            extern wxColor GetDimColor(wxColor c);
            wxColor rangeringcolour = GetDimColor(g_colourOwnshipRangeRingsColour);
            
            wxPen ppPen1( rangeringcolour, g_cog_predictor_width );
            
            dc.SetPen( ppPen1 );
            dc.SetBrush( wxBrush( rangeringcolour, wxBRUSHSTYLE_TRANSPARENT ) );
            
            for( int i = 1; i <= g_iNavAidRadarRingsNumberVisible; i++ )
                dc.StrokeCircle( lGPSPoint.x, lGPSPoint.y, i * pix_radius );
        }
}

void ChartCanvas::ComputeShipScaleFactor(float icon_hdt,
                                         int ownShipWidth, int ownShipLength, 
                                         wxPoint &lShipMidPoint,
                                         wxPoint &GPSOffsetPixels, wxPoint lGPSPoint,
                                         float &scale_factor_x, float &scale_factor_y)
{
    float screenResolution = (float) ::wxGetDisplaySize().x / g_display_size_mm;

    //  Calculate the true ship length in exact pixels
    double ship_bow_lat, ship_bow_lon;
    ll_gc_ll( gLat, gLon, icon_hdt, g_n_ownship_length_meters / 1852., &ship_bow_lat,
              &ship_bow_lon );
    wxPoint lShipBowPoint;
    wxPoint2DDouble b_point = GetVP().GetDoublePixFromLL( ship_bow_lat, ship_bow_lon );
    wxPoint2DDouble a_point = GetVP().GetDoublePixFromLL( gLat, gLon );
    
    float shipLength_px = sqrtf( powf( (float) (b_point.m_x - a_point.m_x), 2) +
                                 powf( (float) (b_point.m_y - a_point.m_y), 2) );
    
    //  And in mm
    float shipLength_mm = shipLength_px / screenResolution;
    
    //  Set minimum ownship drawing size
    float ownship_min_mm = g_n_ownship_min_mm;
    ownship_min_mm = wxMax(ownship_min_mm, 1.0);
    
    //  Calculate Nautical Miles distance from midships to gps antenna
    float hdt_ant = icon_hdt + 180.;
    float dy = ( g_n_ownship_length_meters / 2 - g_n_gps_antenna_offset_y ) / 1852.;
    float dx = g_n_gps_antenna_offset_x / 1852.;
    if( g_n_gps_antenna_offset_y > g_n_ownship_length_meters / 2 )      //reverse?
    {
        hdt_ant = icon_hdt;
        dy = -dy;
    }
    
    //  If the drawn ship size is going to be clamped, adjust the gps antenna offsets
    if( shipLength_mm < ownship_min_mm ) {
        dy /= shipLength_mm / ownship_min_mm;
        dx /= shipLength_mm / ownship_min_mm;
    }

    double ship_mid_lat, ship_mid_lon, ship_mid_lat1, ship_mid_lon1;
    
    ll_gc_ll( gLat, gLon, hdt_ant, dy, &ship_mid_lat, &ship_mid_lon );
    ll_gc_ll( ship_mid_lat, ship_mid_lon, icon_hdt - 90., dx, &ship_mid_lat1, &ship_mid_lon1 );

    GetCanvasPointPix( ship_mid_lat1, ship_mid_lon1, &lShipMidPoint );
    GPSOffsetPixels.x = lShipMidPoint.x - lGPSPoint.x;
    GPSOffsetPixels.y = lShipMidPoint.y - lGPSPoint.y;
    
    float scale_factor = shipLength_px / ownShipLength;
    
    //  Calculate a scale factor that would produce a reasonably sized icon
    float scale_factor_min = ownship_min_mm / ( ownShipLength / screenResolution );
    
    //  And choose the correct one
    scale_factor = wxMax(scale_factor, scale_factor_min);
    
    scale_factor_y = scale_factor;
    scale_factor_x = scale_factor_y * ( (float) ownShipLength / ownShipWidth )
        / ( (float) g_n_ownship_length_meters / g_n_ownship_beam_meters );
}

void ChartCanvas::ShipDraw( ocpnDC& dc )
{
    if( !GetVP().IsValid() ) return;

    wxPoint lGPSPoint, lShipMidPoint, GPSOffsetPixels(0,0);

    //  COG/SOG may be undefined in NMEA data stream
    float pCog = std::isnan(gCog) ? 0 : gCog;
    float pSog = std::isnan(gSog) ? 0 : gSog;

    GetCanvasPointPix( gLat, gLon, &lGPSPoint );
    lShipMidPoint = lGPSPoint;

    //  Draw the icon rotated to the COG
    //  or to the Hdt if available
    float icon_hdt = pCog;
    if( !std::isnan( gHdt ) ) icon_hdt = gHdt;

    //  COG may be undefined in NMEA data stream
    if( std::isnan(icon_hdt) ) icon_hdt = 0.0;

//    Calculate the ownship drawing angle icon_rad using an assumed 10 minute predictor
    double osd_head_lat, osd_head_lon;
    wxPoint osd_head_point;

    ll_gc_ll( gLat, gLon, icon_hdt, pSog * 10. / 60., &osd_head_lat, &osd_head_lon );

    GetCanvasPointPix( osd_head_lat, osd_head_lon, &osd_head_point );

    float icon_rad = atan2f( (float) ( osd_head_point.y - lShipMidPoint.y ),
                             (float) ( osd_head_point.x - lShipMidPoint.x ) );
    icon_rad += (float)PI;

    if (pSog < 0.2) icon_rad = ((icon_hdt + 90.) * PI / 180) + GetVP().rotation;

//    Another draw test ,based on pixels, assuming the ship icon is a fixed nominal size
//    and is just barely outside the viewport        ....
    wxBoundingBox bb_screen( 0, 0, GetVP().pix_width, GetVP().pix_height );

    // TODO: fix to include actual size of boat that will be rendered
    int img_height = 0;
    if( bb_screen.PointInBox( lShipMidPoint, 20 ) ) {
        if( GetVP().chart_scale > 300000 )             // According to S52, this should be 50,000
        {
            ShipDrawLargeScale(dc, lShipMidPoint);
            img_height = 20;
        } else {

            wxImage pos_image;

            //      Substitute user ownship image if found
            if( m_pos_image_user )
                pos_image = m_pos_image_user->Copy();
            else if( SHIP_NORMAL == m_ownship_state )
                pos_image = m_pos_image_red->Copy();
            if( SHIP_LOWACCURACY == m_ownship_state )
                pos_image = m_pos_image_yellow->Copy();
            else if( SHIP_NORMAL != m_ownship_state )
                pos_image = m_pos_image_grey->Copy();


            //      Substitute user ownship image if found
            if( m_pos_image_user ) {
                pos_image = m_pos_image_user->Copy();
                
                if( SHIP_LOWACCURACY == m_ownship_state ) 
                    pos_image = m_pos_image_user_yellow->Copy();
                else if( SHIP_NORMAL != m_ownship_state )
                    pos_image = m_pos_image_user_grey->Copy();
            }

            img_height = pos_image.GetHeight();

            if( g_n_ownship_beam_meters > 0.0 &&
                g_n_ownship_length_meters > 0.0 &&
                g_OwnShipIconType > 0 ) // use large ship
            {
                int ownShipWidth = 22; // Default values from s_ownship_icon
                int ownShipLength= 84;
                if( g_OwnShipIconType == 1 ) {
                    ownShipWidth = pos_image.GetWidth();
                    ownShipLength= pos_image.GetHeight();
                }
                
                float scale_factor_x, scale_factor_y;
                ComputeShipScaleFactor
                    (icon_hdt, ownShipWidth, ownShipLength, lShipMidPoint,
                     GPSOffsetPixels, lGPSPoint, scale_factor_x, scale_factor_y);

                if( g_OwnShipIconType == 1 ) { // Scaled bitmap
                    pos_image.Rescale( ownShipWidth * scale_factor_x, ownShipLength * scale_factor_y,
                                       wxIMAGE_QUALITY_HIGH );
                    wxPoint rot_ctr( pos_image.GetWidth() / 2, pos_image.GetHeight() / 2 );
                    wxImage rot_image = pos_image.Rotate( -( icon_rad - ( PI / 2. ) ), rot_ctr, true );
                    
                    // Simple sharpening algorithm.....
                    for( int ip = 0; ip < rot_image.GetWidth(); ip++ )
                        for( int jp = 0; jp < rot_image.GetHeight(); jp++ )
                            if( rot_image.GetAlpha( ip, jp ) > 64 ) rot_image.SetAlpha( ip, jp, 255 );

                    wxBitmap os_bm( rot_image );

                    int w = os_bm.GetWidth();
                    int h = os_bm.GetHeight();
                    img_height = h;

                    dc.DrawBitmap( os_bm, lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2, true );
                    
                    // Maintain dirty box,, missing in __WXMSW__ library
                    dc.CalcBoundingBox( lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2 );
                    dc.CalcBoundingBox( lShipMidPoint.x - w / 2 + w, lShipMidPoint.y - h / 2 + h );
                }

                else if( g_OwnShipIconType == 2 ) { // Scaled Vector
                    wxPoint ownship_icon[10];

                    for( int i = 0; i < 10; i++ ) {
                        int j = i * 2;
                        float pxa = (float) ( s_ownship_icon[j] );
                        float pya = (float) ( s_ownship_icon[j + 1] );
                        pya *= scale_factor_y;
                        pxa *= scale_factor_x;

                        float px = ( pxa * sinf( icon_rad ) ) + ( pya * cosf( icon_rad ) );
                        float py = ( pya * sinf( icon_rad ) ) - ( pxa * cosf( icon_rad ) );

                        ownship_icon[i].x = (int) ( px ) + lShipMidPoint.x;
                        ownship_icon[i].y = (int) ( py ) + lShipMidPoint.y;
                    }

                    wxPen ppPen1( GetGlobalColor( _T ( "UBLCK" ) ), 1, wxPENSTYLE_SOLID );
                    dc.SetPen( ppPen1 );
                    dc.SetBrush( wxBrush( ShipColor() ) );
                    
                    dc.StrokePolygon( 6, &ownship_icon[0], 0, 0 );
                    
                    //     draw reference point (midships) cross
                    dc.StrokeLine( ownship_icon[6].x, ownship_icon[6].y, ownship_icon[7].x,
                                   ownship_icon[7].y );
                    dc.StrokeLine( ownship_icon[8].x, ownship_icon[8].y, ownship_icon[9].x,
                                   ownship_icon[9].y );
                }

                img_height = ownShipLength * scale_factor_y;

                //      Reference point, where the GPS antenna is
                int circle_rad = 3;
                if( m_pos_image_user ) circle_rad = 1;
                
                dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 1 ) );
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
                dc.StrokeCircle( lGPSPoint.x, lGPSPoint.y, circle_rad );
            }
            else { // Fixed bitmap icon.
                /* non opengl, or suboptimal opengl via ocpndc: */
                wxPoint rot_ctr( pos_image.GetWidth() / 2, pos_image.GetHeight() / 2 );
                wxImage rot_image = pos_image.Rotate( -( icon_rad - ( PI / 2. ) ), rot_ctr, true );
                
                // Simple sharpening algorithm.....
                for( int ip = 0; ip < rot_image.GetWidth(); ip++ )
                    for( int jp = 0; jp < rot_image.GetHeight(); jp++ )
                        if( rot_image.GetAlpha( ip, jp ) > 64 ) rot_image.SetAlpha( ip, jp, 255 );
                
                wxBitmap os_bm( rot_image );
                
                if(g_ShipScaleFactorExp > 1){
                    wxImage scaled_image = os_bm.ConvertToImage();
                    double factor = (log(g_ShipScaleFactorExp) + 1.0) * 1.0;   // soften the scale factor a bit
                    os_bm = wxBitmap(scaled_image.Scale(scaled_image.GetWidth() * factor,
                                                        scaled_image.GetHeight() * factor,
                                                        wxIMAGE_QUALITY_HIGH));
                }
                int w = os_bm.GetWidth();
                int h = os_bm.GetHeight();
                img_height = h;
                
                dc.DrawBitmap( os_bm, lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2, true );
                
                //      Reference point, where the GPS antenna is
                int circle_rad = 3;
                if( m_pos_image_user ) circle_rad = 1;
                
                dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 1 ) );
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
                dc.StrokeCircle( lShipMidPoint.x, lShipMidPoint.y, circle_rad );
                
                // Maintain dirty box,, missing in __WXMSW__ library
                dc.CalcBoundingBox( lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2 );
                dc.CalcBoundingBox( lShipMidPoint.x - w / 2 + w, lShipMidPoint.y - h / 2 + h );
            }
        }        // ownship draw
    }

    ShipIndicatorsDraw(dc, img_height, GPSOffsetPixels, lGPSPoint);
}

/* @ChartCanvas::CalcGridSpacing
 **
 ** Calculate the major and minor spacing between the lat/lon grid
 **
 ** @param [r] WindowDegrees [float] displayed number of lat or lan in the window
 ** @param [w] MajorSpacing [float &] Major distance between grid lines
 ** @param [w] MinorSpacing [float &] Minor distance between grid lines
 ** @return [void]
 */
void CalcGridSpacing( float view_scale_ppm, float& MajorSpacing, float&MinorSpacing )
{
    // table for calculating the distance between the grids
    // [0] view_scale ppm
    // [1] spacing between major grid lines in degrees
    // [2] spacing between minor grid lines in degrees
    const float lltab[][3] =
        { {  0.0f, 90.0f, 30.0f },
          { .000001f, 45.0f, 15.0f },
          { .0002f,   30.0f, 10.0f },
          { .0003f,   10.0f, 2.0f  },
          { .0008f,   5.0f, 1.0f },
          { .001f,    2.0f,          30.0f / 60.0f },
          { .003f,    1.0f,          20.0f / 60.0f },
          { .006f,    0.5f,          10.0f / 60.0f },
          { .03f,     15.0f / 60.0f, 5.0f / 60.0f },
          { .01f,     10.0f / 60.0f, 2.0f / 60.0f },
          { .06f,     5.0f / 60.0f,  1.0f / 60.0f },
          { .1f,      2.0f / 60.0f,  1.0f / 60.0f },
          { .4f,      1.0f / 60.0f,  0.5f / 60.0f },
          { .6f,      0.5f / 60.0f,  0.1f / 60.0f },
          { 1.0f,     0.2f / 60.0f,  0.1f / 60.0f },
          { 1e10f,    0.1f / 60.0f,  0.05f / 60.0f }
    };

    unsigned int tabi;
    for( tabi = 0; tabi < ((sizeof lltab) / (sizeof *lltab)) -1; tabi++ )
        if( view_scale_ppm < lltab[tabi][0] )
            break;
    MajorSpacing = lltab[tabi][1]; // major latitude distance
    MinorSpacing = lltab[tabi][2]; // minor latitude distance
    return;
}
/* @ChartCanvas::CalcGridText *************************************
 **
 ** Calculates text to display at the major grid lines
 **
 ** @param [r] latlon [float] latitude or longitude of grid line
 ** @param [r] spacing [float] distance between two major grid lines
 ** @param [r] bPostfix [bool] true for latitudes, false for longitudes
 **
 ** @return 
 */

wxString CalcGridText( float latlon, float spacing, bool bPostfix )
{
    int deg = (int) fabs( latlon ); // degrees
    float min = fabs( ( fabs( latlon ) - deg ) * 60.0 ); // Minutes
    char postfix;
    
    // calculate postfix letter (NSEW)
    if( latlon > 0.0 ) {
        if( bPostfix ) {
            postfix = 'N';
        } else {
            postfix = 'E';
        }
    } else if( latlon < 0.0 ) {
        if( bPostfix ) {
            postfix = 'S';
        } else {
            postfix = 'W';
        }
    } else {
        postfix = ' '; // no postfix for equator and greenwich
    }
    // calculate text, display minutes only if spacing is smaller than one degree

    wxString ret;
    if( spacing >= 1.0 ) {
        ret.Printf( _T("%3d%c %c"), deg, 0x00b0, postfix );
    } else if( spacing >= ( 1.0 / 60.0 ) ) {
        ret.Printf( _T("%3d%c%02.0f %c"), deg, 0x00b0, min, postfix );
    } else {
        ret.Printf( _T("%3d%c%02.2f %c"), deg, 0x00b0, min, postfix );
    }

    return ret;
}

/* @ChartCanvas::GridDraw *****************************************
 **
 ** Draws major and minor Lat/Lon Grid on the chart
 ** - distance between Grid-lm ines are calculated automatic
 ** - major grid lines will be across the whole chart window
 ** - minor grid lines will be 10 pixel at each edge of the chart window.
 **
 ** @param [w] dc [wxDC&] the wx drawing context
 **
 ** @return [void]
 ************************************************************************/
void ChartCanvas::GridDraw( ocpnDC& dc )
{
    if( !( m_bDisplayGrid && ( fabs( GetVP().rotation ) < 1e-5 ) ) )
        return;

    double nlat, elon, slat, wlon;
    float lat, lon;
    float dlat, dlon;
    float gridlatMajor, gridlatMinor, gridlonMajor, gridlonMinor;
    wxCoord w, h;
    wxPen GridPen( GetGlobalColor( _T ( "SNDG1" ) ), 1, wxPENSTYLE_SOLID );
    dc.SetPen( GridPen );
    dc.SetFont( *m_pgridFont );
    dc.SetTextForeground( GetGlobalColor( _T ( "SNDG1" ) ) );

    w = m_canvas_width;
    h = m_canvas_height;

    GetCanvasPixPoint( 0, 0, nlat, wlon ); // get lat/lon of upper left point of the window
    GetCanvasPixPoint( w, h, slat, elon ); // get lat/lon of lower right point of the window
    dlat = nlat - slat; // calculate how many degrees of latitude are shown in the window
    dlon = elon - wlon; // calculate how many degrees of longitude are shown in the window
    if( dlon < 0.0 ) // concider datum border at 180 degrees longitude
    {
        dlon = dlon + 360.0;
    }
    // calculate distance between latitude grid lines
    CalcGridSpacing( GetVP().view_scale_ppm, gridlatMajor, gridlatMinor );

    // calculate position of first major latitude grid line
    lat = ceil( slat / gridlatMajor ) * gridlatMajor;

    // Draw Major latitude grid lines and text
    while( lat < nlat ) {
        wxPoint r;
        wxString st = CalcGridText( lat, gridlatMajor, true ); // get text for grid line
        GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
        dc.DrawLine( 0, r.y, w, r.y, false );                             // draw grid line
        dc.DrawText( st, 0, r.y ); // draw text
        lat = lat + gridlatMajor;

        if( fabs( lat - wxRound( lat ) ) < 1e-5 ) lat = wxRound( lat );
    }

    // calculate position of first minor latitude grid line
    lat = ceil( slat / gridlatMinor ) * gridlatMinor;

    // Draw minor latitude grid lines
    while( lat < nlat ) {
        wxPoint r;
        GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
        dc.DrawLine( 0, r.y, 10, r.y, false );
        dc.DrawLine( w - 10, r.y, w, r.y, false );
        lat = lat + gridlatMinor;
    }

    // calculate distance between grid lines
    CalcGridSpacing( GetVP().view_scale_ppm, gridlonMajor, gridlonMinor );

    // calculate position of first major latitude grid line
    lon = ceil( wlon / gridlonMajor ) * gridlonMajor;

    // draw major longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMajor ); i <= itermax; i++ ) {
        wxPoint r;
        wxString st = CalcGridText( lon, gridlonMajor, false );
        GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
        dc.DrawLine( r.x, 0, r.x, h, false );
        dc.DrawText( st, r.x, 0 );
        lon = lon + gridlonMajor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }

        if( fabs( lon - wxRound( lon ) ) < 1e-5 ) lon = wxRound( lon );

    }

    // calculate position of first minor longitude grid line
    lon = ceil( wlon / gridlonMinor ) * gridlonMinor;
    // draw minor longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMinor ); i <= itermax; i++ ) {
        wxPoint r;
        GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
        dc.DrawLine( r.x, 0, r.x, 10, false );
        dc.DrawLine( r.x, h - 10, r.x, h, false );
        lon = lon + gridlonMinor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }
    }
}

void ChartCanvas::ScaleBarDraw( ocpnDC& dc )
{
    if(0 /*!g_bsimplifiedScalebar*/){
        double blat, blon, tlat, tlon;
        wxPoint r;

        int x_origin = m_bDisplayGrid ? 60 : 20;
        int y_origin = m_canvas_height - 50;

        float dist;
        int count;
        wxPen pen1, pen2;

        if( GetVP().chart_scale > 80000 )        // Draw 10 mile scale as SCALEB11
        {
            dist = 10.0;
            count = 5;
            pen1 = wxPen( GetGlobalColor( _T ( "SNDG2" ) ), 3, wxPENSTYLE_SOLID );
            pen2 = wxPen( GetGlobalColor( _T ( "SNDG1" ) ), 3, wxPENSTYLE_SOLID );
        } else                                // Draw 1 mile scale as SCALEB10
        {
            dist = 1.0;
            count = 10;
            pen1 = wxPen( GetGlobalColor( _T ( "SCLBR" ) ), 3, wxPENSTYLE_SOLID );
            pen2 = wxPen( GetGlobalColor( _T ( "CHGRD" ) ), 3, wxPENSTYLE_SOLID );
        }

        GetCanvasPixPoint( x_origin, y_origin, blat, blon );
        double rotation = -VPoint.rotation;

        ll_gc_ll( blat, blon, rotation * 180 / PI, dist, &tlat, &tlon );
        GetCanvasPointPix( tlat, tlon, &r );
        int l1 = ( y_origin - r.y ) / count;

        for( int i = 0; i < count; i++ ) {
            int y = l1 * i;
            if( i & 1 ) dc.SetPen( pen1 );
            else
                dc.SetPen( pen2 );
            
            dc.DrawLine( x_origin, y_origin - y, x_origin, y_origin - ( y + l1 ) );
        }
    }
    else {
        double blat, blon, tlat, tlon;

        int x_origin = 5.0 * GetPixPerMM();
        int chartbar_height = GetChartbarHeight();
//         ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
//         if (style->chartStatusWindowTransparent)
//             chartbar_height = 0;
        int y_origin = m_canvas_height - chartbar_height - 5;

        GetCanvasPixPoint( x_origin, y_origin, blat, blon );
        GetCanvasPixPoint( x_origin + m_canvas_width, y_origin, tlat, tlon );

        double d;
        ll_gc_ll_reverse(blat, blon, tlat, tlon, 0, &d);
        d /= 2;

        int unit = g_iDistanceFormat;
        if( d < .5 && ( unit == DISTANCE_KM || unit == DISTANCE_MI || unit == DISTANCE_NMI ) )
            unit = ( unit == DISTANCE_MI ) ? DISTANCE_FT : DISTANCE_M;
        
        // nice number
        float dist = toUsrDistance( d, unit ), logdist = log(dist) / log(10.F);
        float places = floor(logdist), rem = logdist - places;
        dist = pow(10, places);

        if(rem < .2)
            dist /= 5;
        else if(rem < .5)
            dist /= 2;

        wxString s = wxString::Format(_T("%g "), dist) + getUsrDistanceUnit( unit );
        wxPen pen1 = wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 3, wxPENSTYLE_SOLID );
        double rotation = -VPoint.rotation;

        ll_gc_ll( blat, blon, rotation * 180 / PI + 90, fromUsrDistance(dist, unit), &tlat, &tlon );
        wxPoint r;
        GetCanvasPointPix( tlat, tlon, &r );
        int l1 = r.x - x_origin;

        dc.SetPen(pen1);
        
        dc.DrawLine( x_origin, y_origin, x_origin, y_origin - 12);
        dc.DrawLine( x_origin, y_origin, x_origin + l1, y_origin);
        dc.DrawLine( x_origin + l1, y_origin, x_origin + l1, y_origin - 12);

        dc.SetFont( *m_pgridFont );
        dc.SetTextForeground( GetGlobalColor( _T ( "UBLCK" ) ) );
        int w, h;
        dc.GetTextExtent(s, &w, &h);
        dc.DrawText( s, x_origin + l1/2 - w/2, y_origin - h - 1 );
    }
}

void ChartCanvas::JaggyCircle( ocpnDC &dc, wxPen pen, int x, int y, int radius )
{
    //    Constants?
    double da_min = 2.;
    double da_max = 6.;
    double ra_min = 0.;
    double ra_max = 40.;

    wxPen pen_save = dc.GetPen();

    wxDateTime now = wxDateTime::Now();

    dc.SetPen( pen );

    int x0, y0, x1, y1;

    x0 = x1 = x + radius;                    // Start point
    y0 = y1 = y;
    double angle = 0.;
    int i = 0;

    while( angle < 360. ) {
        double da = da_min + ( ( (double) rand() / RAND_MAX ) * ( da_max - da_min ) );
        angle += da;

        if( angle > 360. ) angle = 360.;

        double ra = ra_min + ( ( (double) rand() / RAND_MAX ) * ( ra_max - ra_min ) );

        double r;
        if( i & 1 ) r = radius + ra;
        else
            r = radius - ra;

        x1 = (int) ( x + cos( angle * PI / 180. ) * r );
        y1 = (int) ( y + sin( angle * PI / 180. ) * r );

        dc.DrawLine( x0, y0, x1, y1 );

        x0 = x1;
        y0 = y1;

        i++;

    }

    dc.DrawLine( x + radius, y, x1, y1 );             // closure

    dc.SetPen( pen_save );
}

void ChartCanvas::AlertDraw( ocpnDC& dc )
{
// Just for prototyping, visual alert for anchorwatch goes here
    bool play_sound = false;
    if( pAnchorWatchPoint1 && AnchorAlertOn1 ) {
        if( AnchorAlertOn1 ) {
            wxPoint TargetPoint;
            GetCanvasPointPix( pAnchorWatchPoint1->m_lat, pAnchorWatchPoint1->m_lon, &TargetPoint );
            JaggyCircle( dc, wxPen( GetGlobalColor( _T("URED") ), 2 ), TargetPoint.x, TargetPoint.y,
                         100 );
            play_sound = true;
        }
    } else
        AnchorAlertOn1 = false;

    if( pAnchorWatchPoint2 && AnchorAlertOn2 ) {
        if( AnchorAlertOn2 ) {
            wxPoint TargetPoint;
            GetCanvasPointPix( pAnchorWatchPoint2->m_lat, pAnchorWatchPoint2->m_lon, &TargetPoint );
            JaggyCircle( dc, wxPen( GetGlobalColor( _T("URED") ), 2 ), TargetPoint.x, TargetPoint.y,
                         100 );
            play_sound = true;
        }
    } else
        AnchorAlertOn2 = false;

    if( play_sound ) {
        if( !g_anchorwatch_sound->IsOk() )
            g_anchorwatch_sound->Load( g_sAIS_Alert_Sound_File );
        g_anchorwatch_sound->Play();
    } else if( g_anchorwatch_sound->IsOk() ) {
        g_anchorwatch_sound->Stop();
    }

}
// End of prototype anchor watch alerting-----------------------

void ChartCanvas::UpdateShips()
{
    //  Get the rectangle in the current dc which bounds the "ownship" symbol

    wxClientDC dc( this );
    if( !dc.IsOk() ) return;

    wxBitmap test_bitmap( dc.GetSize().x, dc.GetSize().y );
    wxMemoryDC temp_dc( test_bitmap );

    temp_dc.ResetBoundingBox();
    temp_dc.DestroyClippingRegion();
    temp_dc.SetClippingRegion( 0, 0, dc.GetSize().x, dc.GetSize().y );

    // Draw the ownship on the temp_dc
    ocpnDC ocpndc = ocpnDC( temp_dc );
    ShipDraw( ocpndc );

    if( g_pActiveTrack && g_pActiveTrack->IsRunning() ) {
        TrackPoint* p = g_pActiveTrack->GetLastPoint();
        if( p ) {
            wxPoint px;
            GetCanvasPointPix( p->m_lat, p->m_lon, &px );
            ocpndc.CalcBoundingBox( px.x, px.y );
        }
    }

    ship_draw_rect = wxRect( temp_dc.MinX(), temp_dc.MinY(),
            temp_dc.MaxX() - temp_dc.MinX(),
            temp_dc.MaxY() - temp_dc.MinY() );

    wxRect own_ship_update_rect = ship_draw_rect;

    if( !own_ship_update_rect.IsEmpty() ) {
        //  The required invalidate rectangle is the union of the last drawn rectangle
        //  and this drawn rectangle
        own_ship_update_rect.Union( ship_draw_last_rect );
        own_ship_update_rect.Inflate( 2 );
    }

    if( !own_ship_update_rect.IsEmpty() ) RefreshRect( own_ship_update_rect, false );

    ship_draw_last_rect = ship_draw_rect;

    temp_dc.SelectObject( wxNullBitmap );
}

void ChartCanvas::UpdateAlerts()
{
    //  Get the rectangle in the current dc which bounds the detected Alert targets

    //  Use this dc
    wxClientDC dc( this );

    // Get dc boundary
    int sx, sy;
    dc.GetSize( &sx, &sy );

    //  Need a bitmap
    wxBitmap test_bitmap( sx, sy, -1 );

    // Create a memory DC
    wxMemoryDC temp_dc;
    temp_dc.SelectObject( test_bitmap );

    temp_dc.ResetBoundingBox();
    temp_dc.DestroyClippingRegion();
    temp_dc.SetClippingRegion( wxRect( 0, 0, sx, sy ) );

    // Draw the Alert Targets on the temp_dc
    ocpnDC ocpndc = ocpnDC( temp_dc );
    AlertDraw( ocpndc );

    //  Retrieve the drawing extents
    wxRect alert_rect( temp_dc.MinX(), temp_dc.MinY(), temp_dc.MaxX() - temp_dc.MinX(),
                       temp_dc.MaxY() - temp_dc.MinY() );

    if( !alert_rect.IsEmpty() ) alert_rect.Inflate( 2 );              // clear all drawing artifacts

    if( !alert_rect.IsEmpty() || !alert_draw_rect.IsEmpty() ) {
        //  The required invalidate rectangle is the union of the last drawn rectangle
        //  and this drawn rectangle
        wxRect alert_update_rect = alert_draw_rect;
        alert_update_rect.Union( alert_rect );

        //  Invalidate the rectangular region
        RefreshRect( alert_update_rect, false );
    }

    //  Save this rectangle for next time
    alert_draw_rect = alert_rect;

    temp_dc.SelectObject( wxNullBitmap );      // clean up
}

void ChartCanvas::UpdateAIS()
{
    if(!g_pAIS) return;

    //  Get the rectangle in the current dc which bounds the detected AIS targets

    //  Use this dc
    wxClientDC dc( this );

    // Get dc boundary
    int sx, sy;
    dc.GetSize( &sx, &sy );

    wxRect ais_rect;

    //  How many targets are there?

    //  If more than "some number", it will be cheaper to refresh the entire screen
    //  than to build update rectangles for each target.
    AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();
    if( current_targets->size() > 10 ) {
        ais_rect = wxRect( 0, 0, sx, sy );            // full screen
    } else {
        //  Need a bitmap
        wxBitmap test_bitmap( sx, sy, -1 );

        // Create a memory DC
        wxMemoryDC temp_dc;
        temp_dc.SelectObject( test_bitmap );

        temp_dc.ResetBoundingBox();
        temp_dc.DestroyClippingRegion();
        temp_dc.SetClippingRegion( wxRect( 0, 0, sx, sy ) );

        // Draw the AIS Targets on the temp_dc
        ocpnDC ocpndc = ocpnDC( temp_dc );
        AISDraw( ocpndc, GetVP(), this );
        AISDrawAreaNotices( ocpndc, GetVP(), this );

        //  Retrieve the drawing extents
        ais_rect = wxRect( temp_dc.MinX(), temp_dc.MinY(), temp_dc.MaxX() - temp_dc.MinX(),
                           temp_dc.MaxY() - temp_dc.MinY() );

        if( !ais_rect.IsEmpty() ) ais_rect.Inflate( 2 );              // clear all drawing artifacts

        temp_dc.SelectObject( wxNullBitmap );      // clean up

    }

    if( !ais_rect.IsEmpty() || !ais_draw_rect.IsEmpty() ) {
        //  The required invalidate rectangle is the union of the last drawn rectangle
        //  and this drawn rectangle
        wxRect ais_update_rect = ais_draw_rect;
        ais_update_rect.Union( ais_rect );

        //  Invalidate the rectangular region
        RefreshRect( ais_update_rect, false );
    }

    //  Save this rectangle for next time
    ais_draw_rect = ais_rect;

}

void ChartCanvas::OnActivate( wxActivateEvent& event )
{
    ReloadVP();
}

void ChartCanvas::OnSize( wxSizeEvent& event )
{

    GetClientSize( &m_canvas_width, &m_canvas_height );

//    Get some canvas metrics

//          Rescale to current value, in order to rebuild VPoint data structures
//          for new canvas size
    SetVPScale( GetVPScale() );

    m_absolute_min_scale_ppm = m_canvas_width / ( 1.2 * WGS84_semimajor_axis_meters * PI ); // something like 180 degrees

    //  Inform the parent Frame that I am being resized...
    gFrame->ProcessCanvasResize();

    //  Adjust the toolbar, if necessary
    if( m_toolBar ) {
        m_toolBar->RePosition();
        m_toolBar->SetGeometry(m_Compass->IsShown(), m_Compass->GetRect());
        m_toolBar->Realize();
        m_toolBar->RePosition();
    }
    
    //  if MUIBar is active, size the bar
//     if(g_useMUI && !m_muiBar){                          // rebuild if necessary
//         m_muiBar = new MUIBar(this, wxHORIZONTAL);
//         m_muiBarHOSize = m_muiBar->GetSize();
//     }
    
    
    if(m_muiBar){
        SetMUIBarPosition();
        m_muiBar->SetFollowButton( m_bFollow );
        m_muiBar->SetCanvasENCAvailable( m_bENCGroup );
        m_muiBar->Raise();
    }
    
    
        
    
//    Set up the scroll margins
    xr_margin = m_canvas_width * 95 / 100;
    xl_margin = m_canvas_width * 5 / 100;
    yt_margin = m_canvas_height * 5 / 100;
    yb_margin = m_canvas_height * 95 / 100;

    if( m_pQuilt ) m_pQuilt->SetQuiltParameters( m_canvas_scale_factor, m_canvas_width );

//    Resize the current viewport

    VPoint.pix_width = m_canvas_width;
    VPoint.pix_height = m_canvas_height;

    // Resize the scratch BM
    delete pscratch_bm;
    pscratch_bm = new wxBitmap( VPoint.pix_width, VPoint.pix_height, -1 );
    m_brepaint_piano = true;

    // Resize the Route Calculation BM
    m_dc_route.SelectObject( wxNullBitmap );
    delete proute_bm;
    proute_bm = new wxBitmap( VPoint.pix_width, VPoint.pix_height, -1 );
    m_dc_route.SelectObject( *proute_bm );

    //  Resize the saved Bitmap
    m_cached_chart_bm.Create( VPoint.pix_width, VPoint.pix_height, -1 );

    //  Resize the working Bitmap
    m_working_bm.Create( VPoint.pix_width, VPoint.pix_height, -1 );

    //  Rescale again, to capture all the changes for new canvas size
    SetVPScale( GetVPScale() );

#ifdef ocpnUSE_GL
    if( /*g_bopengl &&*/ m_glcc ) {
        m_glcc->OnSize( event );
    }
#endif
    //  Invalidate the whole window
    ReloadVP();
}

void ChartCanvas::ProcessNewGUIScale()
{
    m_muiBar->Hide();
    delete m_muiBar;
    m_muiBar = 0;
    
    CreateMUIBar();
}


void ChartCanvas::CreateMUIBar()
{
    if(g_useMUI && !m_muiBar){                          // rebuild if necessary
        
        // We need to update the m_bENCGroup flag, at least for the initial creation of a MUIBar
        if(ChartData)
            m_bENCGroup = ChartData->IsENCInGroup( m_groupIndex );

        m_muiBar = new MUIBar(this, wxHORIZONTAL, g_toolbar_scalefactor);
        m_muiBar->SetColorScheme( m_cs );
        m_muiBarHOSize = m_muiBar->GetSize();
    }
    
    
    if(m_muiBar){
        SetMUIBarPosition();
        m_muiBar->SetFollowButton( m_bFollow );
        m_muiBar->SetCanvasENCAvailable( m_bENCGroup );
        m_muiBar->Raise();
    }
    
}


void ChartCanvas::SetMUIBarPosition()
{
    //  if MUIBar is active, size the bar
    if(m_muiBar){
        // We precalculate the piano width based on the canvas width
        int pianoWidth = GetClientSize().x * (g_btouch ? 0.98f : 0.6f);
//        if(m_Piano)
//            pianoWidth = m_Piano->GetWidth();
        
        if((m_muiBar->GetOrientation() == wxHORIZONTAL)){
            if(m_muiBarHOSize.x > (GetClientSize().x - pianoWidth)){
                delete m_muiBar;
                m_muiBar = new MUIBar(this, wxVERTICAL, g_toolbar_scalefactor);
            }
        }
        
        if((m_muiBar->GetOrientation() == wxVERTICAL)){
            if(m_muiBarHOSize.x < (GetClientSize().x - pianoWidth)){
                delete m_muiBar;
                m_muiBar = new MUIBar(this, wxHORIZONTAL, g_toolbar_scalefactor);
            }
        }
        
        m_muiBar->SetBestPosition();
    }
}

void ChartCanvas::DestroyMuiBar()
{
    if(m_muiBar){
        m_muiBar->Destroy();
        m_muiBar = NULL;
    }
}

void ChartCanvas::ShowChartInfoWindow( int x, int dbIndex )
{
    if( dbIndex >= 0 ) {
        if( NULL == m_pCIWin ) {
            m_pCIWin = new ChInfoWin( this );
            m_pCIWin->Hide();
        }

        if( !m_pCIWin->IsShown() || (m_pCIWin->dbIndex != dbIndex) ) {
            wxString s;
            ChartBase *pc = NULL;

            // TOCTOU race but worst case will reload chart.
            // need to lock it or the background spooler may evict charts in 
            // OpenChartFromDBAndLock
            if( ( ChartData->IsChartInCache( dbIndex ) ) && ChartData->IsValid() )
                pc = ChartData->OpenChartFromDBAndLock( dbIndex, FULL_INIT );   // this must come from cache

            int char_width, char_height;
            s = ChartData->GetFullChartInfo( pc, dbIndex, &char_width, &char_height );
            if (pc)
                ChartData->UnLockCacheChart(dbIndex);

            m_pCIWin->SetString( s );
            m_pCIWin->FitToChars( char_width, char_height );

            wxPoint p;
            p.x = x;
             if( ( p.x + m_pCIWin->GetWinSize().x ) > m_canvas_width )
                 p.x = (m_canvas_width - m_pCIWin->GetWinSize().x)/2;    // centered

            p.y = m_canvas_height - m_Piano->GetHeight() - 4 - m_pCIWin->GetWinSize().y;

            m_pCIWin->dbIndex = dbIndex;
            m_pCIWin->SetPosition( p );
            m_pCIWin->SetBitmap();
            m_pCIWin->Refresh();
            m_pCIWin->Show();
        }
    } else {
        HideChartInfoWindow();
    }
}

void ChartCanvas::HideChartInfoWindow( void )
{
    if( m_pCIWin /*&& m_pCIWin->IsShown()*/ ){
        m_pCIWin->Hide();
        m_pCIWin->Destroy();
        m_pCIWin = NULL;

#ifdef __OCPN__ANDROID__        
        androidForceFullRepaint();
#endif        
            
    }
}

void ChartCanvas::PanTimerEvent( wxTimerEvent& event )
{
    wxMouseEvent ev( wxEVT_MOTION );
    ev.m_x = mouse_x;
    ev.m_y = mouse_y;
    ev.m_leftDown = mouse_leftisdown;

    wxEvtHandler *evthp = GetEventHandler();

    ::wxPostEvent( evthp, ev );

}

void ChartCanvas::MovementTimerEvent( wxTimerEvent& )
{
    DoTimedMovement();
}

void ChartCanvas::MovementStopTimerEvent( wxTimerEvent& )
{
    StopMovement( );
}

bool ChartCanvas::CheckEdgePan( int x, int y, bool bdragging, int margin, int delta )
{
    if(m_disable_edge_pan)
        return false;
    
    bool bft = false;
    int pan_margin = m_canvas_width * margin / 100;
    int pan_timer_set = 200;
    double pan_delta = GetVP().pix_width * delta / 100;
    int pan_x = 0;
    int pan_y = 0;

    if( x > m_canvas_width - pan_margin ) {
        bft = true;
        pan_x = pan_delta;
    }

    else if( x < pan_margin ) {
        bft = true;
        pan_x = -pan_delta;
    }

    if( y < pan_margin ) {
        bft = true;
        pan_y = -pan_delta;
    }

    else if( y > m_canvas_height - pan_margin ) {
        bft = true;
        pan_y = pan_delta;
    }

    //    Of course, if dragging, and the mouse left button is not down, we must stop the event injection
    if( bdragging ) {
        if( !g_btouch )
        {
            wxMouseState state = ::wxGetMouseState();
#if  wxCHECK_VERSION(3,0,0)
            if( !state.LeftIsDown() )
#else
            if( !state.LeftDown() )
#endif
                bft = false;
        }
    }
    if( ( bft ) && !pPanTimer->IsRunning() ) {
        PanCanvas( pan_x, pan_y );
        pPanTimer->Start( pan_timer_set, wxTIMER_ONE_SHOT );
        return true;
    }

    //    This mouse event must not be due to pan timer event injector
    //    Mouse is out of the pan zone, so prevent any orphan event injection
    if( ( !bft ) && pPanTimer->IsRunning() ) {
        pPanTimer->Stop();
    }

    return ( false );
}

// Look for waypoints at the current position.
// Used to determine what a mouse event should act on.

void ChartCanvas::FindRoutePointsAtCursor( float selectRadius, bool setBeingEdited )
{
    m_lastRoutePointEditTarget = m_pRoutePointEditTarget;       // save a copy
    m_pRoutePointEditTarget = NULL;
    m_pFoundPoint = NULL;

    SelectItem *pFind = NULL;
    SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_ROUTEPOINT );
    wxSelectableItemListNode *node = SelList.GetFirst();
    while( node ) {
        pFind = node->GetData();

        RoutePoint *frp = (RoutePoint *) pFind->m_pData1;

        //    Get an array of all routes using this point
        m_pEditRouteArray = g_pRouteMan->GetRouteArrayContaining( frp );

        // Use route array to determine actual visibility for the point
        bool brp_viz = false;
        if( m_pEditRouteArray ) {
            for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                if( pr->IsVisible() ) {
                    brp_viz = true;
                    break;
                }
            }
        } else
            brp_viz = frp->IsVisible();               // isolated point

        if( brp_viz ) {
            //    Use route array to rubberband all affected routes
            if( m_pEditRouteArray )                 // Editing Waypoint as part of route
            {
                for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                    Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                    pr->m_bIsBeingEdited = setBeingEdited;
                }
                m_bRouteEditing = setBeingEdited;
            } else                                      // editing Mark
            {
                frp->m_bIsBeingEdited = setBeingEdited;
                m_bMarkEditing = setBeingEdited;
            }

            m_pRoutePointEditTarget = frp;
            m_pFoundPoint = pFind;
            break;            // out of the while(node)
        }

        node = node->GetNext();
    }       // while (node)
}

void ChartCanvas::MouseTimedEvent( wxTimerEvent& event )
{
    if( singleClickEventIsValid ) MouseEvent( singleClickEvent );
    singleClickEventIsValid = false;
    m_DoubleClickTimer->Stop();
}

bool leftIsDown;


bool ChartCanvas::MouseEventOverlayWindows( wxMouseEvent& event )
{
    if (!m_bChartDragging && !m_bDrawingRoute) {
        if(m_Compass && m_Compass->IsShown() && m_Compass->GetRect().Contains(event.GetPosition())) { 
            if (m_Compass->MouseEvent( event )) {
                cursor_region = CENTER;
                if( !g_btouch )
                    SetCanvasCursor( event );
                return true;
            }
        }

        if(MouseEventChartBar( event ))
            return true;
    }
    return false;
}


bool ChartCanvas::MouseEventChartBar( wxMouseEvent& event )
{
    if(!g_bShowChartBar)
        return false;

    if (! m_Piano->MouseEvent(event) )
        return false;

    cursor_region = CENTER;
    if( !g_btouch )
        SetCanvasCursor( event );
    return true;
}

bool ChartCanvas::MouseEventSetup( wxMouseEvent& event,  bool b_handle_dclick )
{
    int x, y;
    int mx, my;

    bool bret = false;
    
    event.GetPosition( &x, &y );
    
    //  Some systems produce null drag events, where the pointer position has not changed from the previous value.
    //  Detect this case, and abort further processing (FS#1748)
#ifdef __WXMSW__    
    if(event.Dragging()){
        if((x == mouse_x) && (y == mouse_y))
            return true;
    }
#endif    
    
    mouse_x = x;
    mouse_y = y;
    mouse_leftisdown = event.LeftDown();
    mx = x;
    my = y;
    GetCanvasPixPoint( x, y, m_cursor_lat, m_cursor_lon );

    //  Establish the event region
    cursor_region = CENTER;
    
    int chartbar_height = GetChartbarHeight();

    if( m_Compass && m_Compass->IsShown() &&
        m_Compass->GetRect().Contains(event.GetPosition())) {
        cursor_region = CENTER;
    } else if( x > xr_margin ) {
        cursor_region = MID_RIGHT;
    } else if( x < xl_margin ) {
        cursor_region = MID_LEFT;
    } else if( y > yb_margin - chartbar_height &&
               y < m_canvas_height - chartbar_height) {
        cursor_region = MID_TOP;
    } else if( y < yt_margin ) {
        cursor_region = MID_BOT;
    } else {
        cursor_region = CENTER;
    }
    
    
    if( !g_btouch )
        SetCanvasCursor( event );
    
    
    // Protect from leftUp's coming from event handlers in child
    // windows who return focus to the canvas.
    leftIsDown = event.LeftDown();
        
    
#ifndef __WXOSX__
    if (event.LeftDown()) {
        if ( g_bShowMenuBar == false && g_bTempShowMenuBar == true ) {
            // The menu bar is temporarily visible due to alt having been pressed.
            // Clicking will hide it, and do nothing else.
            g_bTempShowMenuBar = false;
            parent_frame->ApplyGlobalSettings(false);
            return(true);
        }
    }
#endif
    
// Update modifiers here; some window managers never send the key event
    m_modkeys = 0;
    if(event.ControlDown())
        m_modkeys |= wxMOD_CONTROL;
    if(event.AltDown())
        m_modkeys |= wxMOD_ALT;

#ifdef __WXMSW__
    //TODO Test carefully in other platforms, remove ifdef....
    if( event.ButtonDown() && !HasCapture() ) CaptureMouse();
    if( event.ButtonUp() && HasCapture() ) ReleaseMouse();
#endif
    
    if(g_pi_manager)
        if(g_pi_manager->SendMouseEventToPlugins( event ))
            return(true);                     // PlugIn did something, and does not want the canvas to do anything else

            
        // Capture LeftUp's and time them, unless it already came from the timer.
            
    if( b_handle_dclick && event.LeftUp() && !singleClickEventIsValid ) {

        // Ignore the second LeftUp after the DClick.
        if( m_DoubleClickTimer->IsRunning() ) {
            m_DoubleClickTimer->Stop();
            return(true);
        }

        // Save the event for later running if there is no DClick.
        m_DoubleClickTimer->Start( 350, wxTIMER_ONE_SHOT );
        singleClickEvent = event;
        singleClickEventIsValid = true;
        return(true);
    }

    //  This logic is necessary on MSW to handle the case where
    //  a context (right-click) menu is dismissed without action
    //  by clicking on the chart surface.
    //  We need to avoid an unintentional pan by eating some clicks...
#ifdef __WXMSW__
    if( event.LeftDown() || event.LeftUp() || event.Dragging() ) {
        if( g_click_stop > 0 ) {
            g_click_stop--;
            return(true);
        }
    }
#endif



    //  Kick off the Rotation control timer
    if( m_bCourseUp ) {
        m_b_rot_hidef = false;
        pRotDefTimer->Start( 500, wxTIMER_ONE_SHOT );
    } else
        pRotDefTimer->Stop();

 
//      Retrigger the route leg / AIS target popup timer
    if( !g_btouch )
    {
       if( (m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive()) ||
	   (m_pTrackRolloverWin && m_pTrackRolloverWin->IsActive()) ||
	   (m_pAISRolloverWin && m_pAISRolloverWin->IsActive()) )
            m_RolloverPopupTimer.Start( 10, wxTIMER_ONE_SHOT );               // faster response while the rollover is turned on
        else
            m_RolloverPopupTimer.Start( m_rollover_popup_timer_msec, wxTIMER_ONE_SHOT );
    }

//  Retrigger the cursor tracking timer
    pCurTrackTimer->Start( m_curtrack_timer_msec, wxTIMER_ONE_SHOT );

//      Show cursor position on Status Bar, if present
//      except for GTK, under which status bar updates are very slow
//      due to Update() call.
//      In this case, as a workaround, update the status window
//      after an interval timer (pCurTrackTimer) pops, which will happen
//      whenever the mouse has stopped moving for specified interval.
//      See the method OnCursorTrackTimerEvent()
#if !defined(__WXGTK__) && !defined(__WXQT__)
    SetCursorStatus(m_cursor_lat, m_cursor_lon);
#endif

    //  Send the current cursor lat/lon to all PlugIns requesting it
    if( g_pi_manager ){
        //  Occasionally, MSW will produce nonsense events on right click....
        //  This results in an error in cursor geo position, so we skip this case
        if( (x >= 0) && (y >= 0) )
            g_pi_manager->SendCursorLatLonToAllPlugIns( m_cursor_lat, m_cursor_lon );
    }
    
    
    if(!g_btouch){
        if( ( m_bMeasure_Active && ( m_nMeasureState >= 2 ) ) || ( m_routeState > 1 ) )
        {
            wxPoint p = ClientToScreen( wxPoint( x, y ) );
            
            // Submerge the toolbar if necessary
            if( m_toolBar ) {
                wxRect rect = m_toolBar->GetScreenRect();
                rect.Inflate( 20 );
                if( rect.Contains( p.x, p.y ) )
                    m_toolBar->Submerge();
            }
        }
    }
    
    if(1/*!g_btouch*/ ){
        //    Route Creation Rubber Banding
        if( m_routeState >= 2 ) {
            r_rband.x = x;
            r_rband.y = y;
            m_bDrawingRoute = true;
            
            if(!g_btouch )
                CheckEdgePan( x, y, event.Dragging(), 5, 2 );
            Refresh( false );
        }
        
        
        //    Measure Tool Rubber Banding
        if( m_bMeasure_Active && ( m_nMeasureState >= 2 ) ) {
            r_rband.x = x;
            r_rband.y = y;
            m_bDrawingRoute = true;
            
            if(!g_btouch )
                CheckEdgePan( x, y, event.Dragging(), 5, 2 );
            Refresh( false );
        }
    }
    return bret; 
        
}

void ChartCanvas::CallPopupMenu(int x, int y)
{
    int mx, my;
    mx = x;
    my = y;

    last_drag.x = mx;
    last_drag.y = my;
    if( m_routeState ) {                    // creating route?
        InvokeCanvasMenu(x, y, SELTYPE_ROUTECREATE);
        return;
    }
    // General Right Click
    // Look for selectable objects
    double slat, slon;
    slat = m_cursor_lat;
    slon = m_cursor_lon;
    
    #if defined(__WXMAC__) || defined(__OCPN__ANDROID__)
    wxScreenDC sdc;
    ocpnDC dc( sdc );
    #else
    wxClientDC cdc( GetParent() );
    ocpnDC dc( cdc );
    #endif
    
    SelectItem *pFindAIS;
    SelectItem *pFindRP;
    SelectItem *pFindRouteSeg;
    SelectItem *pFindTrackSeg;
    SelectItem *pFindCurrent = NULL;
    SelectItem *pFindTide = NULL;
    
    //    Deselect any current objects
    if( m_pSelectedRoute ) {
        m_pSelectedRoute->m_bRtIsSelected = false;        // Only one selection at a time
        m_pSelectedRoute->DeSelectRoute();
        #ifdef ocpnUSE_GL
        if(g_bopengl && m_glcc){
            InvalidateGL();
            Update();
        }
        else
            #endif
            m_pSelectedRoute->Draw( dc, this, GetVP().GetBBox() );
    }
    
    if( m_pFoundRoutePoint ) {
        m_pFoundRoutePoint->m_bPtIsSelected = false;
        m_pFoundRoutePoint->Draw( dc, this );
        RefreshRect( m_pFoundRoutePoint->CurrentRect_in_DC );
    }

	/**in touch mode a route point could have been selected and draghandle icon shown so clear the selection*/
	if (g_btouch && m_pRoutePointEditTarget) {
		m_pRoutePointEditTarget->m_bIsBeingEdited = false;
		m_pRoutePointEditTarget->m_bPtIsSelected = false;
		m_pRoutePointEditTarget->EnableDragHandle(false);
	}
    
    //      Get all the selectable things at the cursor
    pFindAIS = pSelectAIS->FindSelection( this, slat, slon, SELTYPE_AISTARGET );
    pFindRP = pSelect->FindSelection( this, slat, slon, SELTYPE_ROUTEPOINT );
    pFindRouteSeg = pSelect->FindSelection( this, slat, slon, SELTYPE_ROUTESEGMENT );
    pFindTrackSeg = pSelect->FindSelection( this, slat, slon, SELTYPE_TRACKSEGMENT );
    
    if( m_bShowCurrent ) pFindCurrent = pSelectTC->FindSelection( this, slat, slon, SELTYPE_CURRENTPOINT );
    
    if( m_bShowTide )                                // look for tide stations
        pFindTide = pSelectTC->FindSelection( this, slat, slon, SELTYPE_TIDEPOINT );
    
    int seltype = 0;
    
    //    Try for AIS targets first
    if( pFindAIS ) {
        m_FoundAIS_MMSI = pFindAIS->GetUserData();
        
        //      Make sure the target data is available
        if( g_pAIS->Get_Target_Data_From_MMSI( m_FoundAIS_MMSI ) ) seltype |=
            SELTYPE_AISTARGET;
    }
    
    //    Now the various Route Parts
    
    m_pFoundRoutePoint = NULL;
    if( pFindRP ) {
        RoutePoint *pFirstVizPoint = NULL;
        RoutePoint *pFoundActiveRoutePoint = NULL;
        RoutePoint *pFoundVizRoutePoint = NULL;
        Route *pSelectedActiveRoute = NULL;
        Route *pSelectedVizRoute = NULL;
        
        //There is at least one routepoint, so get the whole list
        SelectableItemList SelList = pSelect->FindSelectionList( this, slat, slon, SELTYPE_ROUTEPOINT );
        wxSelectableItemListNode *node = SelList.GetFirst();
        while( node ) {
            SelectItem *pFindSel = node->GetData();
            
            RoutePoint *prp = (RoutePoint *) pFindSel->m_pData1;        //candidate
            
            //    Get an array of all routes using this point
            wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( prp );
            
            // Use route array (if any) to determine actual visibility for this point
            bool brp_viz = false;
            if( proute_array ) {
                for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
                    Route *pr = (Route *) proute_array->Item( ir );
                    if( pr->IsVisible() ) {
                        brp_viz = true;
                        break;
                    }
                }
                if( !brp_viz  && prp->m_bKeepXRoute)    // is not visible as part of route, but still exists as a waypoint
                    brp_viz = prp->IsVisible();         //  so treat as isolated point
                    
            } else
                brp_viz = prp->IsVisible();               // isolated point
                
            if( ( NULL == pFirstVizPoint ) && brp_viz ) pFirstVizPoint = prp;
                
            // Use route array to choose the appropriate route
            // Give preference to any active route, otherwise select the first visible route in the array for this point
            m_pSelectedRoute = NULL;
            if( proute_array ) {
                for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
                    Route *pr = (Route *) proute_array->Item( ir );
                    if( pr->m_bRtIsActive ) {
                        pSelectedActiveRoute = pr;
                        pFoundActiveRoutePoint = prp;
                        break;
                    }
                }
                
                if( NULL == pSelectedVizRoute ) {
                    for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
                        Route *pr = (Route *) proute_array->Item( ir );
                        if( pr->IsVisible() ) {
                            pSelectedVizRoute = pr;
                            pFoundVizRoutePoint = prp;
                            break;
                        }
                    }
                }
                
                delete proute_array;
            }
            
            node = node->GetNext();
        }
        
        //      Now choose the "best" selections
        if( pFoundActiveRoutePoint ) {
            m_pFoundRoutePoint = pFoundActiveRoutePoint;
            m_pSelectedRoute = pSelectedActiveRoute;
        } else if( pFoundVizRoutePoint ) {
            m_pFoundRoutePoint = pFoundVizRoutePoint;
            m_pSelectedRoute = pSelectedVizRoute;
        } else
            // default is first visible point in list
            m_pFoundRoutePoint = pFirstVizPoint;
        
        if( m_pSelectedRoute ) {
            if( m_pSelectedRoute->IsVisible() ) seltype |= SELTYPE_ROUTEPOINT;
        } else if( m_pFoundRoutePoint ) seltype |= SELTYPE_MARKPOINT;
        
        //      Highlite the selected point, to verify the proper right click selection
        if( m_pFoundRoutePoint) {
            m_pFoundRoutePoint->m_bPtIsSelected = true;
            wxRect wp_rect;
            m_pFoundRoutePoint->CalculateDCRect( m_dc_route, this, &wp_rect );
            RefreshRect( wp_rect, true );
        }
        
    }
    
    // Note here that we use SELTYPE_ROUTESEGMENT to select tracks as well as routes
    // But call the popup handler with identifier appropriate to the type
    if( pFindRouteSeg )                  // there is at least one select item
    {
        SelectableItemList SelList = pSelect->FindSelectionList( this, slat, slon, SELTYPE_ROUTESEGMENT );
        
        if( NULL == m_pSelectedRoute )  // the case where a segment only is selected
        {
            //  Choose the first visible route containing segment in the list
            wxSelectableItemListNode *node = SelList.GetFirst();
            while( node ) {
                SelectItem *pFindSel = node->GetData();
                
                Route *pr = (Route *) pFindSel->m_pData3;
                if( pr->IsVisible() ) {
                    m_pSelectedRoute = pr;
                    break;
                }
                node = node->GetNext();
            }
        }
        
        if( m_pSelectedRoute ) {
            if( NULL == m_pFoundRoutePoint )
                m_pFoundRoutePoint =   (RoutePoint *) pFindRouteSeg->m_pData1;
            
            m_pSelectedRoute->m_bRtIsSelected = !(seltype & SELTYPE_ROUTEPOINT);
            if( m_pSelectedRoute->m_bRtIsSelected ){
                #ifdef ocpnUSE_GL
                if(g_bopengl && m_glcc){
                    InvalidateGL();
                    Update();
                }
                else
                    #endif
                    m_pSelectedRoute->Draw( dc, this, GetVP().GetBBox() );
            }
            
            seltype |= SELTYPE_ROUTESEGMENT;
        }
        
    }
    
    if( pFindTrackSeg ) {
        m_pSelectedTrack = NULL;
        SelectableItemList SelList = pSelect->FindSelectionList( this, slat, slon, SELTYPE_TRACKSEGMENT );
        
        //  Choose the first visible track containing segment in the list
        wxSelectableItemListNode *node = SelList.GetFirst();
        while( node ) {
            SelectItem *pFindSel = node->GetData();
            
            Track *pt = (Track *) pFindSel->m_pData3;
            if( pt->IsVisible() ) {
                m_pSelectedTrack = pt;
                break;
            }
            node = node->GetNext();
        }
        
        if( m_pSelectedTrack ) seltype |= SELTYPE_TRACKSEGMENT;
    }
    
    bool bseltc = false;
    //                      if(0 == seltype)
    {
        if( pFindCurrent ) {
            // There may be multiple current entries at the same point.
            // For example, there often is a current substation (with directions specified)
            // co-located with its master.  We want to select the substation, so that
            // the direction will be properly indicated on the graphic.
            // So, we search the select list looking for IDX_type == 'c' (i.e substation)
            IDX_entry *pIDX_best_candidate;
            
            SelectItem *pFind = NULL;
            SelectableItemList SelList = pSelectTC->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_CURRENTPOINT );
            
            //      Default is first entry
            wxSelectableItemListNode *node = SelList.GetFirst();
            pFind = node->GetData();
            pIDX_best_candidate = (IDX_entry *) ( pFind->m_pData1 );
            
            if( SelList.GetCount() > 1 ) {
                node = node->GetNext();
                while( node ) {
                    pFind = node->GetData();
                    IDX_entry *pIDX_candidate = (IDX_entry *) ( pFind->m_pData1 );
                    if( pIDX_candidate->IDX_type == 'c' ) {
                        pIDX_best_candidate = pIDX_candidate;
                        break;
                    }
                    
                    node = node->GetNext();
                }       // while (node)
            } else {
                wxSelectableItemListNode *node = SelList.GetFirst();
                pFind = node->GetData();
                pIDX_best_candidate = (IDX_entry *) ( pFind->m_pData1 );
            }
            
            m_pIDXCandidate = pIDX_best_candidate;
            
            if( 0 == seltype ) {
                DrawTCWindow( x, y, (void *) pIDX_best_candidate );
                Refresh( false );
                bseltc = true;
            } else
                seltype |= SELTYPE_CURRENTPOINT;
        }
        
        else if( pFindTide ) {
            m_pIDXCandidate = (IDX_entry *) pFindTide->m_pData1;
            
            if( 0 == seltype ) {
                DrawTCWindow( x, y, (void *) pFindTide->m_pData1 );
                Refresh( false );
                bseltc = true;
            } else
                seltype |= SELTYPE_TIDEPOINT;
        }
    }
    
    if( 0 == seltype )
        seltype |= SELTYPE_UNKNOWN;
    
    if( !bseltc ){
        InvokeCanvasMenu(x, y, seltype);
        
        // Clean up if not deleted in InvokeCanvasMenu
        if( m_pSelectedRoute && g_pRouteMan->IsRouteValid(m_pSelectedRoute) ) {
            m_pSelectedRoute->m_bRtIsSelected = false;
        }
        
        m_pSelectedRoute = NULL;
        
        if( m_pFoundRoutePoint ) {
            if (pSelect->IsSelectableRoutePointValid(m_pFoundRoutePoint))
                m_pFoundRoutePoint->m_bPtIsSelected = false;
        }
        m_pFoundRoutePoint = NULL;
        
        Refresh( true );
        
    }                
    
    // Seth: Is this refresh needed?
    Refresh( false );            // needed for MSW, not GTK  Why??
}

bool ChartCanvas::MouseEventProcessObjects( wxMouseEvent& event )
{
    // For now just bail out completely if the point clicked is not on the chart
    if(std::isnan(m_cursor_lat))
        return false;

    //          Mouse Clicks
    bool ret = false;        // return true if processed
    
    int x, y, mx, my;
    event.GetPosition( &x, &y );
    mx = x;
    my = y;
    
    //    Calculate meaningful SelectRadius
    float SelectRadius;
    SelectRadius = g_Platform->GetSelectRadiusPix() / ( m_true_scale_ppm * 1852 * 60 );  // Degrees, approximately

///
    // We start with Double Click processing. The first left click just starts a timer and
    // is remembered, then we actually do something if there is a LeftDClick.
    // If there is, the two single clicks are ignored.
    
    if( event.LeftDClick() && ( cursor_region == CENTER ) ) {
        
        m_DoubleClickTimer->Start();
        singleClickEventIsValid = false;
        
        double zlat, zlon;
        GetCanvasPixPoint( x, y, zlat, zlon );
        
        if(m_bShowAIS){
            SelectItem *pFindAIS;
            pFindAIS = pSelectAIS->FindSelection( this, zlat, zlon, SELTYPE_AISTARGET );
        
            if( pFindAIS ) {
                m_FoundAIS_MMSI = pFindAIS->GetUserData();
                if( g_pAIS->Get_Target_Data_From_MMSI( m_FoundAIS_MMSI ) ) {
                    wxWindow *pwin = wxDynamicCast(this, wxWindow);
                    ShowAISTargetQueryDialog( pwin, m_FoundAIS_MMSI );
                }
                return true;
            }
        }
        
        SelectableItemList rpSelList = pSelect->FindSelectionList( this, zlat, zlon, SELTYPE_ROUTEPOINT );
        wxSelectableItemListNode *node = rpSelList.GetFirst();
        bool b_onRPtarget = false;
        while( node ) {
            SelectItem *pFind = node->GetData();
            RoutePoint *frp = (RoutePoint *) pFind->m_pData1;
            if(m_pRoutePointEditTarget && (frp == m_pRoutePointEditTarget) ){
                b_onRPtarget = true;
                break;
            }
            node = node->GetNext();
        }
        
        //      Double tap with selected RoutePoint or Mark
        bool bt1 = m_bMarkEditing;
        RoutePoint *pp = m_pRoutePointEditTarget;
        
        if(m_pRoutePointEditTarget){
            if( b_onRPtarget ) {
                ShowMarkPropertiesDialog( m_pRoutePointEditTarget );
                return true;
            }
            else {
                m_pRoutePointEditTarget->m_bIsBeingEdited = false;
                m_pRoutePointEditTarget->m_bPtIsSelected = false;
				if (g_btouch)
					m_pRoutePointEditTarget->EnableDragHandle(false);
                wxRect wp_rect;
                m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                m_pRoutePointEditTarget = NULL;         //cancel selection
                RefreshRect( wp_rect, true );
                return true;
            }
        }
        else{
            node = rpSelList.GetFirst();
            if( node ) {
                SelectItem *pFind = node->GetData();
                RoutePoint *frp = (RoutePoint *) pFind->m_pData1;
                if( frp ){
                    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( frp );

                    // Use route array (if any) to determine actual visibility for this point
                    bool brp_viz = false;
                    if( proute_array ){
                        for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ )
                        {
                            Route *pr = (Route *) proute_array->Item( ir );
                            if( pr->IsVisible() )
                            {
                                brp_viz = true;
                                break;
                            }
                        }
                        if( !brp_viz && frp->m_bKeepXRoute ) // is not visible as part of route, but still exists as a waypoint
                            brp_viz = frp->IsVisible(); // so treat as isolated point
                        } else
                            brp_viz = frp->IsVisible(); // isolated point

                    if( brp_viz ){
                        ShowMarkPropertiesDialog( frp );
                        return true;
                    }
                }
            }
        }
        
        
        
        SelectItem* cursorItem;
        cursorItem = pSelect->FindSelection( this, zlat, zlon, SELTYPE_ROUTESEGMENT );
        
        if( cursorItem ) {
            Route *pr = (Route *) cursorItem->m_pData3;
            if( pr->IsVisible() ) {
                ShowRoutePropertiesDialog( _("Route Properties"), pr );
                return true;
            }
        }
        
        cursorItem = pSelect->FindSelection( this, zlat, zlon, SELTYPE_TRACKSEGMENT );
        
        if( cursorItem ) {
            Track *pt = (Track *) cursorItem->m_pData3;
            if( pt->IsVisible() ) {
                ShowTrackPropertiesDialog( pt );
                return true;
            }
        }
        
        // Found no object to act on, so show chart info.
        
        ShowObjectQueryWindow( x, y, zlat, zlon );
        return true;
    }
    
    
    
///    
    if( event.LeftDown() ) {
        //  This really should not be needed, but....
        //  on Windows, when using wxAUIManager, sometimes the focus is lost
        //  when clicking into another pane, e.g.the AIS target list, and then back to this pane.
        //  Oddly, some mouse events are not lost, however.  Like this one....
        SetFocus();
        
        last_drag.x = mx;
        last_drag.y = my;
        leftIsDown = true;
        
        if(!g_btouch){
            if( m_routeState )                  // creating route?
            {
                double rlat, rlon;
                
                SetCursor( *pCursorPencil );
                rlat = m_cursor_lat;
                rlon = m_cursor_lon;
                
                m_bRouteEditing = true;
                
                if( m_routeState == 1 ) {
                    m_pMouseRoute = new Route();
                    pRouteList->Append( m_pMouseRoute );
                    r_rband.x = x;
                    r_rband.y = y;
                }
                
                //    Check to see if there is a nearby point which may be reused
                RoutePoint *pMousePoint = NULL;
                
                //    Calculate meaningful SelectRadius
                double nearby_radius_meters = g_Platform->GetSelectRadiusPix() / m_true_scale_ppm;
                
                RoutePoint *pNearbyPoint = pWayPointMan->GetNearbyWaypoint( rlat, rlon,
                                                                            nearby_radius_meters );
                if( pNearbyPoint && ( pNearbyPoint != m_prev_pMousePoint )
                    && !pNearbyPoint->m_bIsInLayer && pNearbyPoint->IsVisible() )
                {
                    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( pNearbyPoint );

                    // Use route array (if any) to determine actual visibility for this point
                    bool brp_viz = false;
                    if( proute_array ){
                        for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ){
                            Route *pr = (Route *) proute_array->Item( ir );
                            if( pr->IsVisible() ) {
                                brp_viz = true;
                                break;
                            }
                        }
            
                        if( !brp_viz && pNearbyPoint->m_bKeepXRoute ) // is not visible as part of route, but still exists as a waypoint
                            brp_viz = pNearbyPoint->IsVisible(); // so treat as isolated point
                    }
                    else
                        brp_viz = pNearbyPoint->IsVisible(); // isolated point
                    
                    
                    if( brp_viz ){
                        m_FinishRouteOnKillFocus = false;              // Avoid route finish on focus change for message dialog
                        int dlg_return = OCPNMessageBox( this, _("Use nearby waypoint?"),
                                                 _("OpenCPN Route Create"),
                                                   (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
                        m_FinishRouteOnKillFocus = true;

                        if( dlg_return == wxID_YES ) {
                            pMousePoint = pNearbyPoint;
                                                     
                            // Using existing waypoint, so nothing to delete for undo.
                            if( m_routeState > 1 )
                                undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_HasParent, NULL );
                                                     
                            // check all other routes to see if this point appears in any other route
                            // If it appears in NO other route, then it should e considered an isolated mark
                            if( !g_pRouteMan->FindRouteContainingWaypoint( pMousePoint ) )
                                pMousePoint->m_bKeepXRoute = true;
                        }
                    }
                }
                
                if( NULL == pMousePoint ) {                 // need a new point
                    pMousePoint = new RoutePoint( rlat, rlon, g_default_routepoint_icon, _T(""), wxEmptyString );
                    pMousePoint->SetNameShown( false );
                    
                    pConfig->AddNewWayPoint( pMousePoint, -1 );    // use auto next num
                    pSelect->AddSelectableRoutePoint( rlat, rlon, pMousePoint );
                    
                    if( m_routeState > 1 )
                        undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_IsOrphanded, NULL );
                }
                
                if(m_pMouseRoute){
                    if( m_routeState == 1 ) {
                        // First point in the route.
                        m_pMouseRoute->AddPoint( pMousePoint );
                    } else {
                        if( m_pMouseRoute->m_NextLegGreatCircle ) {
                            double rhumbBearing, rhumbDist, gcBearing, gcDist;
                            DistanceBearingMercator( rlat, rlon, m_prev_rlat, m_prev_rlon, &rhumbBearing, &rhumbDist );
                            Geodesic::GreatCircleDistBear( m_prev_rlon, m_prev_rlat, rlon, rlat, &gcDist, &gcBearing, NULL );
                            double gcDistNM = gcDist / 1852.0;
                            
                            // Empirically found expression to get reasonable route segments.
                            int segmentCount = (3.0 + (rhumbDist - gcDistNM)) / pow(rhumbDist-gcDistNM-1, 0.5 );
                            
                            wxString msg;
                            msg << _("For this leg the Great Circle route is ")
                            << FormatDistanceAdaptive( rhumbDist - gcDistNM ) << _(" shorter than rhumbline.\n\n")
                            << _("Would you like include the Great Circle routing points for this leg?");
                            
                            m_FinishRouteOnKillFocus = false;
                            m_disable_edge_pan = true;  // This helps on OS X if MessageBox does not fully capture mouse
                            
                            int answer = OCPNMessageBox( this, msg, _("OpenCPN Route Create"), wxYES_NO | wxNO_DEFAULT );
                            
                            m_disable_edge_pan = false;
                            m_FinishRouteOnKillFocus = true;
                            
                            if( answer == wxID_YES ) {
                                RoutePoint* gcPoint;
                                RoutePoint* prevGcPoint = m_prev_pMousePoint;
                                wxRealPoint gcCoord;
                                
                                for( int i = 1; i <= segmentCount; i++ ) {
                                    double fraction = (double) i * ( 1.0 / (double) segmentCount );
                                    Geodesic::GreatCircleTravel( m_prev_rlon, m_prev_rlat, gcDist * fraction,
                                                                 gcBearing, &gcCoord.x, &gcCoord.y, NULL );
                                    
                                    if( i < segmentCount ) {
                                        gcPoint = new RoutePoint( gcCoord.y, gcCoord.x, _T("xmblue"), _T(""),
                                                                  wxEmptyString );
                                        gcPoint->SetNameShown( false );
                                        pConfig->AddNewWayPoint( gcPoint, -1 );
                                        pSelect->AddSelectableRoutePoint( gcCoord.y, gcCoord.x, gcPoint );
                                    } else {
                                        gcPoint = pMousePoint; // Last point, previously exsisting!
                                    }
                                    
                                    m_pMouseRoute->AddPoint( gcPoint );
                                    pSelect->AddSelectableRouteSegment( prevGcPoint->m_lat, prevGcPoint->m_lon,
                                                                        gcPoint->m_lat, gcPoint->m_lon, prevGcPoint, gcPoint, m_pMouseRoute );
                                    prevGcPoint = gcPoint;
                                }
                                
                                undo->CancelUndoableAction( true );
                                
                            } else {
                                m_pMouseRoute->AddPoint( pMousePoint );
                                pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                                                    rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                                undo->AfterUndoableAction( m_pMouseRoute );
                            }
                        } else {
                            // Ordinary rhumblinesegment.
                            m_pMouseRoute->AddPoint( pMousePoint );
                            pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                                                rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                            undo->AfterUndoableAction( m_pMouseRoute );
                        }
                    }
                }
                
                m_prev_rlat = rlat;
                m_prev_rlon = rlon;
                m_prev_pMousePoint = pMousePoint;
                if(m_pMouseRoute)
                    m_pMouseRoute->m_lastMousePointIndex = m_pMouseRoute->GetnPoints();
                
                m_routeState++;
                gFrame->RefreshAllCanvas();
                ret = true;
            }
            
            else if( m_bMeasure_Active && m_nMeasureState )   // measure tool?
            {
                double rlat, rlon;
                
                SetCursor( *pCursorPencil );
                rlat = m_cursor_lat;
                rlon = m_cursor_lon;
                
                if( m_nMeasureState == 1 ) {
                    m_pMeasureRoute = new Route();
                    pRouteList->Append( m_pMeasureRoute );
                    r_rband.x = x;
                    r_rband.y = y;
                }
                
                RoutePoint *pMousePoint = new RoutePoint( m_cursor_lat, m_cursor_lon,
                                                          wxString( _T ( "circle" ) ), wxEmptyString, wxEmptyString );
                pMousePoint->m_bShowName = false;
                
                m_pMeasureRoute->AddPoint( pMousePoint );
                
                m_prev_rlat = m_cursor_lat;
                m_prev_rlon = m_cursor_lon;
                m_prev_pMousePoint = pMousePoint;
                m_pMeasureRoute->m_lastMousePointIndex = m_pMeasureRoute->GetnPoints();
                
                m_nMeasureState++;
                gFrame->RefreshAllCanvas();
                ret = true;
            }
            
            else {
                FindRoutePointsAtCursor( SelectRadius, true );    // Not creating Route
            }
        }  // !g_btouch
        else {                  // g_btouch

           if(( m_bMeasure_Active && m_nMeasureState ) || ( m_routeState )){
              
               // if near screen edge, pan with injection
               //                if( CheckEdgePan( x, y, true, 5, 10 ) ) {
                                //                    return;
                                //                }
                                
           }
        }
        
        if(ret)
            return true;
    }
    
    if( event.Dragging() ) {
        
        //in touch screen mode ensure the finger/cursor is on the selected point's radius to allow dragging
        if( g_btouch ) {
            if( m_pRoutePointEditTarget && !m_bIsInRadius ) {
                SelectItem *pFind = NULL;
                SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_ROUTEPOINT );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    pFind = node->GetData();
                    RoutePoint *frp = (RoutePoint *) pFind->m_pData1;
                    if( m_pRoutePointEditTarget == frp )
                        m_bIsInRadius = true;
                    node = node->GetNext();
                }
            }
            
            // Check for use of dragHandle
            if(m_pRoutePointEditTarget && m_pRoutePointEditTarget->IsDragHandleEnabled()){
                SelectItem *pFind = NULL;
                SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_DRAGHANDLE );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    pFind = node->GetData();
                    RoutePoint *frp = (RoutePoint *) pFind->m_pData1;
                    if( m_pRoutePointEditTarget == frp ){
                        m_bIsInRadius = true;
                        break;
                    }
                    node = node->GetNext();
                }
                
                if(!m_dragoffsetSet){
                    m_pRoutePointEditTarget->PresetDragOffset(this, mouse_x, mouse_y);
                    m_dragoffsetSet = true;
                }
            }
            
        }
        
        
        if( m_bRouteEditing && m_pRoutePointEditTarget ) {
            
            bool DraggingAllowed = g_btouch ? m_bIsInRadius : true;
            
            if( NULL == g_pMarkInfoDialog ) {
                if( g_bWayPointPreventDragging ) DraggingAllowed = false;
            } else if( !g_pMarkInfoDialog->IsShown() && g_bWayPointPreventDragging )
                DraggingAllowed = false;
            
            if( m_pRoutePointEditTarget && ( m_pRoutePointEditTarget->GetIconName() == _T("mob") ) )
                DraggingAllowed = false;
            
            if( m_pRoutePointEditTarget->m_bIsInLayer )
                DraggingAllowed = false;
                                                
            if( DraggingAllowed ) {
                                                    
                                                    if( !undo->InUndoableAction() ) {
                                                        undo->BeforeUndoableAction( Undo_MoveWaypoint, m_pRoutePointEditTarget,
                                                                                    Undo_NeedsCopy, m_pFoundPoint );
                                                    }
                                                    
                                                    // Get the update rectangle for the union of the un-edited routes
                                                    wxRect pre_rect;
                                                    
                                                    if( !g_bopengl && m_pEditRouteArray ) {
                                                        for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                                                            Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                                                            //      Need to validate route pointer
                                                            //      Route may be gone due to drgging close to ownship with
                                                            //      "Delete On Arrival" state set, as in the case of
                                                            //      navigating to an isolated waypoint on a temporary route
                                                            if( g_pRouteMan->IsRouteValid(pr) ) {
                                                                wxRect route_rect;
                                                                pr->CalculateDCRect( m_dc_route, this, &route_rect );
                                                                pre_rect.Union( route_rect );
                                                            }
                                                        }
                                                    }

                                                    double new_cursor_lat = m_cursor_lat;
                                                    double new_cursor_lon = m_cursor_lon;

                                                    if( CheckEdgePan( x, y, true, 5, 2 ) )
                                                        GetCanvasPixPoint( x, y, new_cursor_lat, new_cursor_lon );

                                                                           // update the point itself
                                                    if( g_btouch ) {
                                                        //m_pRoutePointEditTarget->SetPointFromDraghandlePoint(VPoint, new_cursor_lat, new_cursor_lon);
                                                        m_pRoutePointEditTarget->SetPointFromDraghandlePoint(this, mouse_x, mouse_y);
                                                        // update the Drag Handle entry in the pSelect list
                                                        pSelect->ModifySelectablePoint( new_cursor_lat, new_cursor_lon, m_pRoutePointEditTarget, SELTYPE_DRAGHANDLE );
                                                        m_pFoundPoint->m_slat = m_pRoutePointEditTarget->m_lat;             // update the SelectList entry
                                                        m_pFoundPoint->m_slon = m_pRoutePointEditTarget->m_lon;
                                                    }
                                                    else{
                                                        m_pRoutePointEditTarget->m_lat = new_cursor_lat;    // update the RoutePoint entry
                                                        m_pRoutePointEditTarget->m_lon = new_cursor_lon;
                                                        m_pFoundPoint->m_slat = new_cursor_lat;             // update the SelectList entry
                                                        m_pFoundPoint->m_slon = new_cursor_lon;
                                                    }
   
                                                   
                                                    //    Update the MarkProperties Dialog, if currently shown
                                                    if( ( NULL != g_pMarkInfoDialog ) && ( g_pMarkInfoDialog->IsShown() ) ) {
                                                        if( m_pRoutePointEditTarget == g_pMarkInfoDialog->GetRoutePoint() ) g_pMarkInfoDialog->UpdateProperties( true );
                                                    }
                                                    
                                                    if(g_bopengl) {
                                                        //InvalidateGL();
                                                        Refresh( false );
                                                    } else {
                                                        // Get the update rectangle for the edited route
                                                        wxRect post_rect;
                                                        
                                                        if( m_pEditRouteArray ) {
                                                            for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                                                                Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                                                                if( g_pRouteMan->IsRouteValid(pr) ) {
                                                                    wxRect route_rect;
                                                                    pr->CalculateDCRect( m_dc_route, this, &route_rect );
                                                                    post_rect.Union( route_rect );
                                                                }
                                                            }
                                                        }
                                                        
                                                        //    Invalidate the union region
                                                        pre_rect.Union( post_rect );
                                                        RefreshRect( pre_rect, false );
                                                    }
                                                    gFrame->RefreshCanvasOther( this );
                                                    m_bRoutePoinDragging = true;
                                                }
                                                ret = true;
        }     // if Route Editing
        
        else if( m_bMarkEditing && m_pRoutePointEditTarget ) {
            
            bool DraggingAllowed = g_btouch ? m_bIsInRadius : true;
            
            if( NULL == g_pMarkInfoDialog ) {
                if( g_bWayPointPreventDragging )
                    DraggingAllowed = false;
            } else if( !g_pMarkInfoDialog->IsShown() && g_bWayPointPreventDragging )
                DraggingAllowed = false;
            
            if( m_pRoutePointEditTarget
                && ( m_pRoutePointEditTarget->GetIconName() == _T("mob") ) )
                DraggingAllowed = false;
            
            if( m_pRoutePointEditTarget->m_bIsInLayer )
                DraggingAllowed = false;
                    
            if( DraggingAllowed ) {
                        if( !undo->InUndoableAction() ) {
                            undo->BeforeUndoableAction( Undo_MoveWaypoint, m_pRoutePointEditTarget,
                                                        Undo_NeedsCopy, m_pFoundPoint );
                        }
                        
                        //      The mark may be an anchorwatch
                        double lpp1 = 0.;
                        double lpp2 = 0.;
                        double lppmax;
                        
                        if( pAnchorWatchPoint1 == m_pRoutePointEditTarget ) {
                            lpp1 = fabs( GetAnchorWatchRadiusPixels( pAnchorWatchPoint1 ) );
                            
                        }
                        if( pAnchorWatchPoint2 == m_pRoutePointEditTarget ) {
                            lpp2 = fabs( GetAnchorWatchRadiusPixels( pAnchorWatchPoint2 ) );
                        }
                        lppmax = wxMax(lpp1 + 10, lpp2 + 10);         // allow for cruft
                        
                        // Get the update rectangle for the un-edited mark
                        wxRect pre_rect;
                        if(!g_bopengl) {
                            m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &pre_rect );
                            if( ( lppmax > pre_rect.width / 2 ) || ( lppmax > pre_rect.height / 2 ) )
                                pre_rect.Inflate( (int) ( lppmax - ( pre_rect.width / 2 ) ), (int) ( lppmax - ( pre_rect.height / 2 ) ) );
                        }
                        
                        // update the point itself
                        if( g_btouch ) {
//                            m_pRoutePointEditTarget->SetPointFromDraghandlePoint(VPoint, m_cursor_lat, m_cursor_lon);
                            m_pRoutePointEditTarget->SetPointFromDraghandlePoint(this, mouse_x, mouse_y);
                            // update the Drag Handle entry in the pSelect list
                            pSelect->ModifySelectablePoint( m_cursor_lat, m_cursor_lon, m_pRoutePointEditTarget, SELTYPE_DRAGHANDLE );
                            m_pFoundPoint->m_slat = m_pRoutePointEditTarget->m_lat;             // update the SelectList entry
                            m_pFoundPoint->m_slon = m_pRoutePointEditTarget->m_lon;
                        }
                        else{
                            m_pRoutePointEditTarget->m_lat = m_cursor_lat;    // update the RoutePoint entry
                            m_pRoutePointEditTarget->m_lon = m_cursor_lon;
                            m_pFoundPoint->m_slat = m_cursor_lat;             // update the SelectList entry
                            m_pFoundPoint->m_slon = m_cursor_lon;
                        }
                        
                        
                            
                        //    Update the MarkProperties Dialog, if currently shown
                        if( ( NULL != g_pMarkInfoDialog ) && ( g_pMarkInfoDialog->IsShown() ) ) {
                            if( m_pRoutePointEditTarget == g_pMarkInfoDialog->GetRoutePoint() )
                                g_pMarkInfoDialog->UpdateProperties( true );
                        }
                        
                        //    Invalidate the union region
                        if(g_bopengl) {
                            if(!g_btouch)
                                InvalidateGL();
                            Refresh( false );
                        } else {
                            // Get the update rectangle for the edited mark
                            wxRect post_rect;
                            m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &post_rect );
                            if( ( lppmax > post_rect.width / 2 ) || ( lppmax > post_rect.height / 2 ) )
                                post_rect.Inflate((int) ( lppmax - ( post_rect.width / 2 ) ),
                                                  (int) ( lppmax - ( post_rect.height / 2 ) ) );
                                
                            //    Invalidate the union region
                            pre_rect.Union( post_rect );
                            RefreshRect( pre_rect, false );
                        }
                        gFrame->RefreshCanvasOther( this );
                        m_bRoutePoinDragging = true;
                    }
                    ret = true;

        }
        
        if(ret)
            return true;
    }       //dragging
    
    if( event.LeftUp() ) {
        bool b_startedit_route = false;
        bool b_startedit_mark = false;
        m_dragoffsetSet = false;

        if(g_btouch) {
            m_bChartDragging = false;
            m_bIsInRadius = false;
            
            if( m_routeState )                  // creating route?
            {
                if(m_bedge_pan){
                    m_bedge_pan = false;
                    return false;
                }
                
                double rlat, rlon;
                
                rlat = m_cursor_lat;
                rlon = m_cursor_lon;
                
                if( m_pRoutePointEditTarget) {
                    m_pRoutePointEditTarget->m_bIsBeingEdited = false;
                    m_pRoutePointEditTarget->m_bPtIsSelected = false;
                    wxRect wp_rect;
                    m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                    RefreshRect( wp_rect, true );
                    m_pRoutePointEditTarget = NULL;
                }
                m_bRouteEditing = true;
                
                if( m_routeState == 1 ) {
                    m_pMouseRoute = new Route();
                    m_pMouseRoute->SetHiLite(50);
                    pRouteList->Append( m_pMouseRoute );
                    r_rband.x = x;
                    r_rband.y = y;
                }
                
                
                //    Check to see if there is a nearby point which may be reused
                RoutePoint *pMousePoint = NULL;
                
                //    Calculate meaningful SelectRadius
                double nearby_radius_meters = g_Platform->GetSelectRadiusPix() / m_true_scale_ppm;
                
                RoutePoint *pNearbyPoint = pWayPointMan->GetNearbyWaypoint( rlat, rlon,
                                                                            nearby_radius_meters );
                if( pNearbyPoint && ( pNearbyPoint != m_prev_pMousePoint )
                    && !pNearbyPoint->m_bIsInLayer && pNearbyPoint->IsVisible() )
                {
                    int dlg_return;
                    #ifndef __WXOSX__
                    dlg_return = OCPNMessageBox( this, _("Use nearby waypoint?"),
                                                 _("OpenCPN Route Create"),
                                                 (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
                                                 #else
                                                 dlg_return = wxID_YES;
                                                 #endif
                                                 if( dlg_return == wxID_YES ) {
                                                     pMousePoint = pNearbyPoint;
                                                     
                                                     // Using existing waypoint, so nothing to delete for undo.
                                                     if( m_routeState > 1 )
                                                         undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_HasParent, NULL );
                                                     
                                                     // check all other routes to see if this point appears in any other route
                                                         // If it appears in NO other route, then it should e considered an isolated mark
                                                         if( !g_pRouteMan->FindRouteContainingWaypoint( pMousePoint ) ) pMousePoint->m_bKeepXRoute =
                                                             true;
                                                 }
                }
                
                if( NULL == pMousePoint ) {                 // need a new point
                    pMousePoint = new RoutePoint( rlat, rlon, g_default_routepoint_icon, _T(""), wxEmptyString );
                    pMousePoint->SetNameShown( false );
                    
                    pConfig->AddNewWayPoint( pMousePoint, -1 );    // use auto next num
                    pSelect->AddSelectableRoutePoint( rlat, rlon, pMousePoint );
                    
                    if( m_routeState > 1 )
                        undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_IsOrphanded, NULL );
                }
                
                if( m_routeState == 1 ) {
                    // First point in the route.
                    m_pMouseRoute->AddPoint( pMousePoint );
                } else {
                    if( m_pMouseRoute->m_NextLegGreatCircle ) {
                        double rhumbBearing, rhumbDist, gcBearing, gcDist;
                        DistanceBearingMercator( rlat, rlon, m_prev_rlat, m_prev_rlon, &rhumbBearing, &rhumbDist );
                        Geodesic::GreatCircleDistBear( m_prev_rlon, m_prev_rlat, rlon, rlat, &gcDist, &gcBearing, NULL );
                        double gcDistNM = gcDist / 1852.0;
                        
                        // Empirically found expression to get reasonable route segments.
                        int segmentCount = (3.0 + (rhumbDist - gcDistNM)) / pow(rhumbDist-gcDistNM-1, 0.5 );
                        
                        wxString msg;
                        msg << _("For this leg the Great Circle route is ")
                        << FormatDistanceAdaptive( rhumbDist - gcDistNM ) << _(" shorter than rhumbline.\n\n")
                        << _("Would you like include the Great Circle routing points for this leg?");
                        
                        #ifndef __WXOSX__
                        int answer = OCPNMessageBox( this, msg, _("OpenCPN Route Create"), wxYES_NO | wxNO_DEFAULT );
                        #else
                        int answer = wxID_NO;
                        #endif
                        
                        if( answer == wxID_YES ) {
                            RoutePoint* gcPoint;
                            RoutePoint* prevGcPoint = m_prev_pMousePoint;
                            wxRealPoint gcCoord;
                            
                            for( int i = 1; i <= segmentCount; i++ ) {
                                double fraction = (double) i * ( 1.0 / (double) segmentCount );
                                Geodesic::GreatCircleTravel( m_prev_rlon, m_prev_rlat, gcDist * fraction,
                                                             gcBearing, &gcCoord.x, &gcCoord.y, NULL );
                                
                                if( i < segmentCount ) {
                                    gcPoint = new RoutePoint( gcCoord.y, gcCoord.x, _T("xmblue"), _T(""),
                                                              wxEmptyString );
                                    gcPoint->SetNameShown( false );
                                    pConfig->AddNewWayPoint( gcPoint, -1 );
                                    pSelect->AddSelectableRoutePoint( gcCoord.y, gcCoord.x, gcPoint );
                                } else {
                                    gcPoint = pMousePoint; // Last point, previously exsisting!
                                }
                                
                                m_pMouseRoute->AddPoint( gcPoint );
                                pSelect->AddSelectableRouteSegment( prevGcPoint->m_lat, prevGcPoint->m_lon,
                                                                    gcPoint->m_lat, gcPoint->m_lon, prevGcPoint, gcPoint, m_pMouseRoute );
                                prevGcPoint = gcPoint;
                            }
                            
                            undo->CancelUndoableAction( true );
                            
                        } else {
                            m_pMouseRoute->AddPoint( pMousePoint );
                            pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                                                rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                            undo->AfterUndoableAction( m_pMouseRoute );
                        }
                    } else {
                        // Ordinary rhumblinesegment.
                        m_pMouseRoute->AddPoint( pMousePoint );
                        pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                                            rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                        undo->AfterUndoableAction( m_pMouseRoute );
                    }
                }
                
                m_prev_rlat = rlat;
                m_prev_rlon = rlon;
                m_prev_pMousePoint = pMousePoint;
                m_pMouseRoute->m_lastMousePointIndex = m_pMouseRoute->GetnPoints();
                
                m_routeState++;
                Refresh( true );
                ret = true;
            }
            else if( m_bMeasure_Active && m_nMeasureState )   // measure tool?
            {
                if(m_bedge_pan){
                    m_bedge_pan = false;
                    return false;
                }
                
                double rlat, rlon;
                
                rlat = m_cursor_lat;
                rlon = m_cursor_lon;
                
                if( m_nMeasureState == 1 ) {
                    m_pMeasureRoute = new Route();
                    pRouteList->Append( m_pMeasureRoute );
                    r_rband.x = x;
                    r_rband.y = y;
                }
                
                
                RoutePoint *pMousePoint = new RoutePoint( m_cursor_lat, m_cursor_lon,
                                                          wxString( _T ( "circle" ) ), wxEmptyString, wxEmptyString );
                                                          pMousePoint->m_bShowName = false;
                                                          
                m_pMeasureRoute->AddPoint( pMousePoint );
                                                          
                m_prev_rlat = m_cursor_lat;
                m_prev_rlon = m_cursor_lon;
                m_prev_pMousePoint = pMousePoint;
                m_pMeasureRoute->m_lastMousePointIndex = m_pMeasureRoute->GetnPoints();
                                                          
                m_nMeasureState++;
                                                          
                Refresh( true );
                ret = true;
            }
            else {
                
                bool bSelectAllowed = true;
                if( NULL == g_pMarkInfoDialog ) {
                    if( g_bWayPointPreventDragging ) bSelectAllowed = false;
                } else if( !g_pMarkInfoDialog->IsShown() && g_bWayPointPreventDragging )
                    bSelectAllowed = false;

				/*if this left up happens at the end of a route point dragging and if the cursor/thumb is on the 
				draghandle icon, not on the point iself a new selection will select nothing and the drag will never
				be ended, so the legs around this point never selectable. At this step we don't need a new selection,
				just keep the previoulsly selected and dragged point */
				if (m_bRoutePoinDragging)
					bSelectAllowed = false;
                
                if(bSelectAllowed){
                    
                bool b_was_editing_mark = m_bMarkEditing;
                bool b_was_editing_route = m_bRouteEditing;
                FindRoutePointsAtCursor( SelectRadius, true );    // Possibly selecting a point in a route for later dragging
                
				/*route and a mark points in layer can't be dragged so should't be selected and no draghandle icon*/
				if (m_pRoutePointEditTarget && m_pRoutePointEditTarget->m_bIsInLayer)
					m_pRoutePointEditTarget = NULL;

                if( !b_was_editing_route ) {
                    if( m_pEditRouteArray ) {
                        b_startedit_route = true;
                        
                        
                        //  Hide the track and route rollover during route point edit, not needed, and may be confusing
                        if( m_pTrackRolloverWin && m_pTrackRolloverWin->IsActive()  ) {
                            m_pTrackRolloverWin->IsActive( false );
                        }
                        if( m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive()  ) {
                            m_pRouteRolloverWin->IsActive( false );
                        }
                        
                        wxRect pre_rect;
                        for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                            Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                            //      Need to validate route pointer
                            //      Route may be gone due to drgging close to ownship with
                            //      "Delete On Arrival" state set, as in the case of
                            //      navigating to an isolated waypoint on a temporary route
                            if( g_pRouteMan->IsRouteValid(pr) ) {
                                //                                pr->SetHiLite(50);
                                wxRect route_rect;
                                pr->CalculateDCRect( m_dc_route, this, &route_rect );
                                pre_rect.Union( route_rect );
                            }
                        }
                        RefreshRect( pre_rect, true );
                    }
                }
                else {
                    b_startedit_route = false; 
                }
                
                
                //  Mark editing
                if( m_pRoutePointEditTarget ) {
                    
                    if(b_was_editing_mark || b_was_editing_route) {            // kill previous hilight
                        if( m_lastRoutePointEditTarget) {
                            m_lastRoutePointEditTarget->m_bIsBeingEdited = false;
                            m_lastRoutePointEditTarget->m_bPtIsSelected = false;
                            m_lastRoutePointEditTarget->EnableDragHandle( false );
                            pSelect->DeleteSelectablePoint( m_lastRoutePointEditTarget, SELTYPE_DRAGHANDLE );

                        }
                    }
                    
                    if( m_pRoutePointEditTarget) {
                        m_pRoutePointEditTarget->m_bIsBeingEdited = true;
                        m_pRoutePointEditTarget->m_bPtIsSelected = true;
                        m_pRoutePointEditTarget->EnableDragHandle( true );
                        wxPoint2DDouble dragHandlePoint = m_pRoutePointEditTarget->GetDragHandlePoint(this);
                        pSelect->AddSelectablePoint(dragHandlePoint.m_y, dragHandlePoint.m_x, m_pRoutePointEditTarget, SELTYPE_DRAGHANDLE);

                    }
                }
                else {                  // Deselect everything
                    if( m_lastRoutePointEditTarget) {
                        m_lastRoutePointEditTarget->m_bIsBeingEdited = false;
                        m_lastRoutePointEditTarget->m_bPtIsSelected = false;
                        m_lastRoutePointEditTarget->EnableDragHandle( false );
                        pSelect->DeleteSelectablePoint( m_lastRoutePointEditTarget, SELTYPE_DRAGHANDLE );
 
                        //  Clear any routes being edited, probably orphans
                        wxArrayPtrVoid *lastEditRouteArray = g_pRouteMan->GetRouteArrayContaining( m_lastRoutePointEditTarget );
                        if( lastEditRouteArray ) {
                            for( unsigned int ir = 0; ir < lastEditRouteArray->GetCount(); ir++ ) {
                                Route *pr = (Route *) lastEditRouteArray->Item( ir );
                                if( g_pRouteMan->IsRouteValid(pr) ) {
                                    pr->m_bIsBeingEdited = false;
                                }
                            }
                        }
                    }
                }

                //  Do the refresh
                
                if(g_bopengl) {
                    InvalidateGL();
                    Refresh( false );
                } else {
                    if( m_lastRoutePointEditTarget) {
                        wxRect wp_rect;
                        m_lastRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                        RefreshRect( wp_rect, true );
                    }
                        
                        if( m_pRoutePointEditTarget) {
                        wxRect wp_rect;
                        m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                        RefreshRect( wp_rect, true );
                    }
                }
            }
            }       //  bSelectAllowed
            
            //      Check to see if there is a route or AIS target under the cursor
            //      If so, start the rollover timer which creates the popup
            bool b_start_rollover = false;
            if( g_pAIS && g_pAIS->GetNumTargets() && m_bShowAIS ) {
                SelectItem *pFind = pSelectAIS->FindSelection( this, m_cursor_lat, m_cursor_lon, SELTYPE_AISTARGET );
                if( pFind )
                    b_start_rollover = true;
            }
            
            if(!b_start_rollover && !b_startedit_route){
                SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_ROUTESEGMENT );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    SelectItem *pFindSel = node->GetData();
                    
                    Route *pr = (Route *) pFindSel->m_pData3;        //candidate
                    
                    if( pr && pr->IsVisible() ){
                        b_start_rollover = true;
                        break;
                    }
                    node = node->GetNext();
                }       // while
            }
            
            if(!b_start_rollover && !b_startedit_route){
                SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_TRACKSEGMENT );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    SelectItem *pFindSel = node->GetData();

                    Track *tr = (Track *) pFindSel->m_pData3;        //candidate

                    if( tr && tr->IsVisible() ){
                        b_start_rollover = true;
                        break;
                    }
                    node = node->GetNext();
                }       // while
            }

            if( b_start_rollover )
                m_RolloverPopupTimer.Start( m_rollover_popup_timer_msec, wxTIMER_ONE_SHOT );
            
            
            if( m_bRouteEditing/* && !b_startedit_route*/) {            // End of RoutePoint drag
            if( m_pRoutePointEditTarget ) {
                pSelect->UpdateSelectableRouteSegments( m_pRoutePointEditTarget );
                
                if( m_pEditRouteArray ) {
                    for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                        Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                        if( g_pRouteMan->IsRouteValid(pr) ) {
                            pr->FinalizeForRendering();
                            pr->UpdateSegmentDistances();
                            if( m_bRoutePoinDragging ) pConfig->UpdateRoute( pr );
                        }
                    }
                }
                
                //    Update the RouteProperties Dialog, if currently shown
                if( pRoutePropDialog && pRoutePropDialog->IsShown() ) {
                    if( m_pEditRouteArray ) {
                        for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                            Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                            if( g_pRouteMan->IsRouteValid(pr) ) {
                                if( pRoutePropDialog->GetRoute() == pr ) {
                                    pRoutePropDialog->SetRouteAndUpdate( pr, true );
                                }
/* cannot edit track points anyway
                                else if ( ( NULL != pTrackPropDialog ) && ( pTrackPropDialog->IsShown() ) && pTrackPropDialog->m_pTrack == pr ) {
                                    pTrackPropDialog->SetTrackAndUpdate( pr );
                                }
*/
                            }
                        }
                    }
                }
                
            }
            }
            
			else if(  m_bMarkEditing ) {				// End of way point drag
				if( m_pRoutePointEditTarget )
					if( m_bRoutePoinDragging ) pConfig->UpdateWayPoint( m_pRoutePointEditTarget );
			}

			if( m_pRoutePointEditTarget )
                undo->AfterUndoableAction( m_pRoutePointEditTarget );
            
            if(!m_pRoutePointEditTarget){
                delete m_pEditRouteArray;
                m_pEditRouteArray = NULL;
                m_bRouteEditing = false;
            }
            m_bRoutePoinDragging = false;
        }       // g_btouch
        
        
        else{                   // !g_btouch
        if( m_bRouteEditing ) {            // End of RoutePoint drag
            if( m_pRoutePointEditTarget ) {
                pSelect->UpdateSelectableRouteSegments( m_pRoutePointEditTarget );
                m_pRoutePointEditTarget->m_bBlink = false;
                
                if( m_pEditRouteArray ) {
                    for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                        Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                        if( g_pRouteMan->IsRouteValid(pr) ) {
                            pr->FinalizeForRendering();
                            pr->UpdateSegmentDistances();
                            pr->m_bIsBeingEdited = false;
                            
                            if( m_bRoutePoinDragging ) pConfig->UpdateRoute( pr );
                            
                            pr->SetHiLite( 0 );
                        }
                    }
                    Refresh( false );
                }
                
                //    Update the RouteProperties Dialog, if currently shown
                if( pRoutePropDialog && pRoutePropDialog->IsShown() ) {
                    if( m_pEditRouteArray ) {
                        for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                            Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                            if( g_pRouteMan->IsRouteValid(pr) ) {
                                if( pRoutePropDialog->GetRoute() == pr ) {
                                    pRoutePropDialog->SetRouteAndUpdate( pr, true );
                                }
                            }
                        }
                    }
                }
                
                m_pRoutePointEditTarget->m_bPtIsSelected = false;
                m_pRoutePointEditTarget->m_bIsBeingEdited = false;
                
                delete m_pEditRouteArray;
                m_pEditRouteArray = NULL;
                undo->AfterUndoableAction( m_pRoutePointEditTarget );
            }
            
            InvalidateGL();
            m_bRouteEditing = false;
            m_pRoutePointEditTarget = NULL;
            
            if( m_toolBar && !m_toolBar->IsToolbarShown())
                SurfaceToolbar();
            ret = true;
        }
        
        else if( m_bMarkEditing) {         // end of Waypoint drag
            if( m_pRoutePointEditTarget ) {
                if( m_bRoutePoinDragging ) pConfig->UpdateWayPoint( m_pRoutePointEditTarget );
                undo->AfterUndoableAction( m_pRoutePointEditTarget );
                m_pRoutePointEditTarget->m_bIsBeingEdited = false;
                wxRect wp_rect;
                m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                m_pRoutePointEditTarget->m_bPtIsSelected = false;
                RefreshRect( wp_rect, true );
                
            }
            m_pRoutePointEditTarget = NULL;
            m_bMarkEditing = false;
            if( m_toolBar && !m_toolBar->IsToolbarShown())
                SurfaceToolbar();
            ret = true;
        }

        else if( leftIsDown ) {  // left click for chart center
            leftIsDown = false;
            ret = false;
            
            if( !g_btouch ){
                if( !m_bChartDragging && !m_bMeasure_Active ) {
                 } else {
                    m_bChartDragging = false;
                }
            }
            
        }
		 m_bRoutePoinDragging = false;
        }       // !btouch
        
        if(ret)
            return true;
    }           // left up
    
    if( event.RightDown() ) {
        SetFocus();           //  This is to let a plugin know which canvas is right-clicked
        last_drag.x = mx;
        last_drag.y = my;
        
        if(g_btouch ){
//            if( m_pRoutePointEditTarget )
//                return false;
        }
        
        ret = true;
        CallPopupMenu(mx , my);
    }   //Right down

    return ret;
        
}

bool panleftIsDown;

bool ChartCanvas::MouseEventProcessCanvas( wxMouseEvent& event )
{
    int x, y;
    event.GetPosition( &x, &y );
    
    //        Check for wheel rotation
    // ideally, should be just longer than the time between
    // processing accumulated mouse events from the event queue
    // as would happen during screen redraws.
    int wheel_dir = event.GetWheelRotation();
    
    if( wheel_dir ) {
        int mouse_wheel_oneshot = abs(wheel_dir)*4;                  //msec
        wheel_dir = wheel_dir > 0 ? 1 : -1; // normalize
        
        double factor = 2.0;
        if(wheel_dir < 0)
            factor = 1/factor;
        
        if(g_bsmoothpanzoom){
            if( (m_wheelstopwatch.Time() < m_wheelzoom_stop_oneshot) ) {
                if( wheel_dir == m_last_wheel_dir ) {
                    m_wheelzoom_stop_oneshot += mouse_wheel_oneshot;
                    //                    m_zoom_target /= factor;
                }
                else 
                    StopMovement( );
            }
            else {    
                m_wheelzoom_stop_oneshot = mouse_wheel_oneshot;
                m_wheelstopwatch.Start(0);
                //                m_zoom_target =  VPoint.chart_scale / factor;
            }
        }
        
        m_last_wheel_dir = wheel_dir;
        
        
        ZoomCanvas( factor, true, false );
        
    }

    if( event.LeftDown() ) {
        last_drag.x = x, last_drag.y = y;
        panleftIsDown = true;
    }
    
    if( event.LeftUp() ) {
        if( panleftIsDown ) {  // leftUp for chart center, but only after a leftDown seen here.
            panleftIsDown = false;
            
            if( !g_btouch ){
                if( !m_bChartDragging && !m_bMeasure_Active ) {
                    switch( cursor_region ){
                        case MID_RIGHT: {
                            PanCanvas( 100, 0 );
                            break;
                        }
                        
                        case MID_LEFT: {
                            PanCanvas( -100, 0 );
                            break;
                        }
                        
                        case MID_TOP: {
                            PanCanvas( 0, 100 );
                            break;
                        }
                        
                        case MID_BOT: {
                            PanCanvas( 0, -100 );
                            break;
                        }
                        
                        case CENTER: {
                            PanCanvas( x - GetVP().pix_width / 2, y - GetVP().pix_height / 2 );
                            break;
                        }
                    }
                } else {
                    m_bChartDragging = false;
                }
            }
        }
    }
    
    if( event.Dragging() && event.LeftIsDown()){
            /*
             * fixed dragging.
             * On my Surface Pro 3 running Arch Linux there is no mouse down event before the drag event.
             * Hence, as there is no mouse down event, last_drag is not reset before the drag.
             * And that results in one single drag session, meaning you cannot drag the map a few miles
             * north, lift your finger, and the go even further north. Instead, the map resets itself
             * always to the very first drag start (since there is not reset of last_drag).
             *
             * Besides, should not left down and dragging be enough of a situation to start a drag procedure?
             *
             * Anyways, guarded it to be active in touch situations only.
             */
            if( g_btouch ) {
                if(false == m_bChartDragging) {
                    last_drag.x = x, last_drag.y = y;
                    m_bChartDragging = true;
                }
            }


            if( ( last_drag.x != x ) || ( last_drag.y != y ) ) {
                m_bChartDragging = true;
                StartTimedMovement();
                m_pan_drag.x += last_drag.x - x;
                m_pan_drag.y += last_drag.y - y;

                last_drag.x = x, last_drag.y = y;

                if( g_btouch ) {
                    if(( m_bMeasure_Active && m_nMeasureState ) || ( m_routeState )){
                        //deactivate next LeftUp to ovoid creating an unexpected point
                        m_DoubleClickTimer->Start();
                        singleClickEventIsValid = false;
                    }
                }
                
            }
    }
        
        

    return true;
    
                
}

void ChartCanvas::MouseEvent( wxMouseEvent& event )
{
    if (MouseEventOverlayWindows( event ))
        return;

    if(MouseEventSetup( event ))
        return;              // handled, no further action required
    
    if(!MouseEventProcessObjects( event ))
         MouseEventProcessCanvas( event );
}


void ChartCanvas::SetCanvasCursor( wxMouseEvent& event )
{
    //    Switch to the appropriate cursor on mouse movement

    wxCursor *ptarget_cursor = pCursorArrow;
    if( !pPlugIn_Cursor ) {
        ptarget_cursor = pCursorArrow;
        if( ( !m_routeState )
            && ( !m_bMeasure_Active ) /*&& ( !m_bCM93MeasureOffset_Active )*/) {
            
            if( cursor_region == MID_RIGHT ) {
                ptarget_cursor = pCursorRight;
            } else if( cursor_region == MID_LEFT ) {
                ptarget_cursor = pCursorLeft;
            } else if( cursor_region == MID_TOP ) {
                ptarget_cursor = pCursorDown;
            } else if( cursor_region == MID_BOT ) {
                ptarget_cursor = pCursorUp;
            } else {
                ptarget_cursor = pCursorArrow;
            }
            } else if( m_bMeasure_Active || m_routeState ) // If Measure tool use Pencil Cursor
                ptarget_cursor = pCursorPencil;
    }
    else {
        ptarget_cursor = pPlugIn_Cursor;
    }
    

        SetCursor( *ptarget_cursor );

}





void ChartCanvas::LostMouseCapture( wxMouseCaptureLostEvent& event )
{
    SetCursor( *pCursorArrow );
}



void ChartCanvas::ShowObjectQueryWindow( int x, int y, float zlat, float zlon )
{
#ifdef USE_S57
    
    ChartPlugInWrapper *target_plugin_chart = NULL;
    s57chart *Chs57 = NULL;

    ChartBase *target_chart = GetChartAtCursor();
    if( target_chart ){
        if( (target_chart->GetChartType() == CHART_TYPE_PLUGIN) && (target_chart->GetChartFamily() == CHART_FAMILY_VECTOR) )
            target_plugin_chart = dynamic_cast<ChartPlugInWrapper *>(target_chart);
        else
            Chs57 = dynamic_cast<s57chart*>( target_chart );
    }

    std::vector<Ais8_001_22*> area_notices;

    if( g_pAIS && m_bShowAIS && g_bShowAreaNotices ) {
        AIS_Target_Hash* an_sources = g_pAIS->GetAreaNoticeSourcesList();

        float vp_scale = GetVPScale();

        for( AIS_Target_Hash::iterator target = an_sources->begin(); target != an_sources->end(); ++target ) {
            AIS_Target_Data* target_data = target->second;
            if( !target_data->area_notices.empty() ) {
                for( AIS_Area_Notice_Hash::iterator ani = target_data->area_notices.begin(); ani != target_data->area_notices.end(); ++ani ) {
                    Ais8_001_22& area_notice = ani->second;

                    wxBoundingBox bbox;

                    for( Ais8_001_22_SubAreaList::iterator sa = area_notice.sub_areas.begin(); sa != area_notice.sub_areas.end(); ++sa ) {
                        switch( sa->shape ) {
                            case AIS8_001_22_SHAPE_CIRCLE: {
                                wxPoint target_point;
                                GetCanvasPointPix( sa->latitude, sa->longitude, &target_point );
                                bbox.Expand( target_point );
                                if( sa->radius_m > 0.0 )
                                    bbox.EnLarge( sa->radius_m * vp_scale );
                                break;
                            }
                            case AIS8_001_22_SHAPE_POLYGON:
                            case AIS8_001_22_SHAPE_POLYLINE: {
                                for( int i = 0; i < 4; ++i ) {
                                    double lat = sa->latitude;
                                    double lon = sa->longitude;
                                    ll_gc_ll( lat, lon, sa->angles[i], sa->dists_m[i] / 1852.0,
                                              &lat, &lon );
                                    wxPoint target_point;
                                    GetCanvasPointPix( lat, lon, &target_point );
                                    bbox.Expand( target_point );
                                }
                            }
                        }
                    }

                    if( bbox.PointInBox( x, y ) ) {
                        area_notices.push_back( &area_notice );
                    }
                }
            }
        }
    }


    if( target_plugin_chart || Chs57 || !area_notices.empty() ) {
        // Go get the array of all objects at the cursor lat/lon
        int sel_rad_pix = 5;
        float SelectRadius = sel_rad_pix / ( GetVP().view_scale_ppm * 1852 * 60 );

        // Make sure we always get the lights from an object, even if we are currently
        // not displaying lights on the chart.

        SetCursor( wxCURSOR_WAIT );
        bool lightsVis = m_encShowLights; //gFrame->ToggleLights( false );
        if( !lightsVis ) SetShowENCLights( true );
;

        ListOfObjRazRules* rule_list = NULL;
        ListOfPI_S57Obj* pi_rule_list = NULL;
        if( Chs57 )
            rule_list = Chs57->GetObjRuleListAtLatLon( zlat, zlon, SelectRadius, &GetVP() );
        else if( target_plugin_chart )
            pi_rule_list = g_pi_manager->GetPlugInObjRuleListAtLatLon( target_plugin_chart, zlat, zlon, SelectRadius, GetVP() );

        ListOfObjRazRules* overlay_rule_list = NULL;
        ChartBase *overlay_chart = GetOverlayChartAtCursor();
        s57chart *CHs57_Overlay = dynamic_cast<s57chart*>( overlay_chart );

        if( CHs57_Overlay ) {
            overlay_rule_list =
                CHs57_Overlay->GetObjRuleListAtLatLon( zlat, zlon, SelectRadius, &GetVP() );
        }

        if( !lightsVis ) SetShowENCLights( false );

        wxString objText;
        wxFont *dFont = FontMgr::Get().GetFont( _("ObjectQuery") );
        wxString face = dFont->GetFaceName();

        if( NULL == g_pObjectQueryDialog ) {
            g_pObjectQueryDialog = new S57QueryDialog();

            g_pObjectQueryDialog->Create( this, -1, _( "Object Query" ), wxDefaultPosition,
                                          wxSize( g_S57_dialog_sx, g_S57_dialog_sy ) );
            g_pObjectQueryDialog->Centre();
        }

        wxColor bg = g_pObjectQueryDialog->GetBackgroundColour();
        wxColor fg = FontMgr::Get().GetFontColor( _("ObjectQuery") );

        objText.Printf( _T("<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x>"),
                       bg.Red(), bg.Blue(), bg.Green(), fg.Red(), fg.Blue(), fg.Green() );

#ifdef __WXOSX__
        int points = dFont->GetPointSize();
#else
        int points = dFont->GetPointSize() + 1;
#endif

        int sizes[7];
        for ( int i=-2; i<5; i++ ) {
            sizes[i+2] = points + i + (i>0?i:0);
        }
        g_pObjectQueryDialog->m_phtml->SetFonts(face, face, sizes);

        if(wxFONTSTYLE_ITALIC == dFont->GetStyle())
            objText += _T("<i>");
        
        if( overlay_rule_list && CHs57_Overlay) {
            objText << CHs57_Overlay->CreateObjDescriptions( overlay_rule_list );
            objText << _T("<hr noshade>");
        }

        for( std::vector< Ais8_001_22* >::iterator an = area_notices.begin(); an != area_notices.end(); ++an ) {
            objText << _T( "<b>AIS Area Notice:</b> " );
            objText << ais8_001_22_notice_names[( *an )->notice_type];
            for( std::vector< Ais8_001_22_SubArea >::iterator sa = ( *an )->sub_areas.begin(); sa != ( *an )->sub_areas.end(); ++sa )
                if( !sa->text.empty() )
                    objText << sa->text;
            objText << _T( "<br>expires: " ) << ( *an )->expiry_time.Format();
            objText << _T( "<hr noshade>" );
        }

        if( Chs57 )
            objText << Chs57->CreateObjDescriptions( rule_list );
        else if( target_plugin_chart )
            objText << g_pi_manager->CreateObjDescriptions( target_plugin_chart, pi_rule_list );

        objText << _T("</font>");
        if(wxFONTSTYLE_ITALIC == dFont->GetStyle())
            objText << _T("</i>");
        
        objText << _T("</body></html>");
        
        g_pObjectQueryDialog->SetHTMLPage( objText );

        g_pObjectQueryDialog->Show();

        if( rule_list )
            rule_list->Clear();
        delete rule_list;

        if( overlay_rule_list )
            overlay_rule_list->Clear();
        delete overlay_rule_list;

        if( pi_rule_list )
            pi_rule_list->Clear();
        delete pi_rule_list;

        SetCursor( wxCURSOR_ARROW );
    }
#endif    
}


void ChartCanvas::ShowMarkPropertiesDialog( RoutePoint* markPoint ) {
    if ( !g_pMarkInfoDialog )    // There is one global instance of the MarkProp Dialog
        g_pMarkInfoDialog = new MarkInfoDlg(this);

    if( 1/*g_bresponsive*/ ) {

        wxSize canvas_size = GetSize();
        wxPoint canvas_pos = GetPosition();
        wxSize fitted_size = g_pMarkInfoDialog->GetSize();;

        bool newFit = false;
        if(canvas_size.x < fitted_size.x){
            fitted_size.x = canvas_size.x - 40;
            if(canvas_size.y < fitted_size.y)
                fitted_size.y -= 40;                // scrollbar added
        }
        if(canvas_size.y < fitted_size.y){
            fitted_size.y = canvas_size.y - 40;
            if(canvas_size.x < fitted_size.x)
                fitted_size.x -= 40;                // scrollbar added
        }

        if(newFit){
            g_pMarkInfoDialog->SetSize( fitted_size );
            g_pMarkInfoDialog->Centre();
        }
    }

    g_pMarkInfoDialog->SetRoutePoint( markPoint );
    g_pMarkInfoDialog->UpdateProperties();
    if( markPoint->m_bIsInLayer ) {
        wxString caption( wxString::Format( _T("%s, %s: %s"), _("Waypoint Properties"), _("Layer"), GetLayerName( markPoint->m_LayerID ) ) );
        g_pMarkInfoDialog->SetDialogTitle( caption );
    } else
        g_pMarkInfoDialog->SetDialogTitle( _("Waypoint Properties") );

    g_pMarkInfoDialog->Show();
    g_pMarkInfoDialog->Raise();
    g_pMarkInfoDialog->InitialFocus();
}

void ChartCanvas::ShowRoutePropertiesDialog(wxString title, Route* selected)
{
    pRoutePropDialog = RoutePropDlgImpl::getInstance( this );
    pRoutePropDialog->SetRouteAndUpdate( selected );
    //pNew->UpdateProperties();
    pRoutePropDialog->Show();
    pRoutePropDialog->Raise();
    return;
    pRoutePropDialog = RoutePropDlgImpl::getInstance( this ); // There is one global instance of the RouteProp Dialog

    if( g_bresponsive ) {

        wxSize canvas_size = GetSize();
        wxPoint canvas_pos = GetPosition();
        wxSize fitted_size = pRoutePropDialog->GetSize();;

        if(canvas_size.x < fitted_size.x){
            fitted_size.x = canvas_size.x;
            if(canvas_size.y < fitted_size.y)
                fitted_size.y -= 20;                // scrollbar added
        }
        if(canvas_size.y < fitted_size.y){
            fitted_size.y = canvas_size.y;
            if(canvas_size.x < fitted_size.x)
                fitted_size.x -= 20;                // scrollbar added
        }


        pRoutePropDialog->SetSize( fitted_size );
        pRoutePropDialog->Centre();

        int xp = (canvas_size.x - fitted_size.x)/2;
        int yp = (canvas_size.y - fitted_size.y)/2;

        wxPoint xxp = ClientToScreen(canvas_pos);
//        pRoutePropDialog->Move(xxp.x + xp, xxp.y + yp);

    }


    pRoutePropDialog->SetRouteAndUpdate( selected );

    pRoutePropDialog->Show();

    Refresh( false );
}

void ChartCanvas::ShowTrackPropertiesDialog( Track* selected )
{
    pTrackPropDialog = TrackPropDlg::getInstance( this );    // There is one global instance of the RouteProp Dialog

    pTrackPropDialog->SetTrackAndUpdate( selected );
    pTrackPropDialog->UpdateProperties();

    pTrackPropDialog->Show();

    Refresh( false );
}

void pupHandler_PasteWaypoint() {
    Kml kml;

    int pasteBuffer = kml.ParsePasteBuffer();
    RoutePoint* pasted = kml.GetParsedRoutePoint();
    if( ! pasted ) return;

    double nearby_radius_meters = g_Platform->GetSelectRadiusPix() / gFrame->GetPrimaryCanvas()->GetCanvasTrueScale();

    RoutePoint *nearPoint = pWayPointMan->GetNearbyWaypoint( pasted->m_lat, pasted->m_lon,
                               nearby_radius_meters );

    int answer = wxID_NO;
    if( nearPoint && !nearPoint->m_bIsInLayer ) {
        wxString msg;
        msg << _("There is an existing waypoint at the same location as the one you are pasting. Would you like to merge the pasted data with it?\n\n");
        msg << _("Answering 'No' will create a new waypoint at the same location.");
        answer = OCPNMessageBox( NULL, msg, _("Merge waypoint?"), (long) wxYES_NO | wxCANCEL | wxNO_DEFAULT );
    }

    if( answer == wxID_YES ) {
        nearPoint->SetName( pasted->GetName() );
        nearPoint->m_MarkDescription = pasted->m_MarkDescription;
        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
    }

    if( answer == wxID_NO ) {
        RoutePoint* newPoint = new RoutePoint( pasted );
        newPoint->m_bIsolatedMark = true;
        pSelect->AddSelectableRoutePoint( newPoint->m_lat, newPoint->m_lon, newPoint );
        pConfig->AddNewWayPoint( newPoint, -1 );
        pWayPointMan->AddRoutePoint( newPoint );
        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
        if( newPoint->GetScaMin() < g_focusCanvas->GetScaleValue() ) newPoint->ShowScaleWarningMessage(g_focusCanvas);
    }

    gFrame->InvalidateAllGL();
    gFrame->RefreshAllCanvas( false );
}

void pupHandler_PasteRoute() {
    Kml kml;

    int pasteBuffer = kml.ParsePasteBuffer();
    Route* pasted = kml.GetParsedRoute();
    if( ! pasted ) return;

    double nearby_radius_meters = g_Platform->GetSelectRadiusPix() / gFrame->GetPrimaryCanvas()->GetCanvasTrueScale();

    RoutePoint* curPoint;
    RoutePoint* nearPoint;
    RoutePoint* prevPoint = NULL;

    bool mergepoints = false;
    bool createNewRoute = true;
    int existingWaypointCounter = 0;

    for( int i = 1; i <= pasted->GetnPoints(); i++ ) {
		curPoint = pasted->GetPoint( i ); // NB! n starts at 1 !
        nearPoint = pWayPointMan->GetNearbyWaypoint( curPoint->m_lat,
                curPoint->m_lon, nearby_radius_meters );
        if( nearPoint ) {
            mergepoints = true;
            existingWaypointCounter++;
            // Small hack here to avoid both extending RoutePoint and repeating all the GetNearbyWaypoint
            // calculations. Use existin data field in RoutePoint as temporary storage.
            curPoint->m_bPtIsSelected = true;
        }
    }

    int answer = wxID_NO;
    if( mergepoints ) {
        wxString msg;
        msg << _("There are existing waypoints at the same location as some of the ones you are pasting. Would you like to just merge the pasted data into them?\n\n");
        msg << _("Answering 'No' will create all new waypoints for this route.");
        answer = OCPNMessageBox( NULL, msg, _("Merge waypoints?"), (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );

        if( answer == wxID_CANCEL ) {
            return;
        }
    }

    // If all waypoints exist since before, and a route with the same name, we don't create a new route.
    if( mergepoints && answer==wxID_YES && existingWaypointCounter==pasted->GetnPoints() ) {

        wxRouteListNode *route_node = pRouteList->GetFirst();
        while( route_node ) {
            Route *proute = route_node->GetData();

            if( pasted->m_RouteNameString == proute->m_RouteNameString ) {
                createNewRoute = false;
                break;
            }
            route_node = route_node->GetNext();
        }
    }

    Route* newRoute = NULL;
    RoutePoint* newPoint;

    if( createNewRoute ) {
        newRoute = new Route();
        newRoute->m_RouteNameString = pasted->m_RouteNameString;
    }

    for( int i = 1; i <= pasted->GetnPoints(); i++ ) {
        curPoint = pasted->GetPoint( i );
        if( answer == wxID_YES && curPoint->m_bPtIsSelected ) {
            curPoint->m_bPtIsSelected = false;
            newPoint = pWayPointMan->GetNearbyWaypoint( curPoint->m_lat, curPoint->m_lon,
                    nearby_radius_meters );
            newPoint->SetName( curPoint->GetName() );
            newPoint->m_MarkDescription = curPoint->m_MarkDescription;

            if( createNewRoute ) newRoute->AddPoint( newPoint );
        } else {
            curPoint->m_bPtIsSelected = false;

            newPoint = new RoutePoint( curPoint );
            newPoint->m_bIsolatedMark = false;
            newPoint->SetIconName( _T("circle") );
            newPoint->m_bIsVisible = true;
            newPoint->m_bShowName = false;
            newPoint->m_bKeepXRoute = false;

            newRoute->AddPoint( newPoint );
            pSelect->AddSelectableRoutePoint( newPoint->m_lat, newPoint->m_lon, newPoint );
            pConfig->AddNewWayPoint( newPoint, -1 );
            pWayPointMan->AddRoutePoint( newPoint );
        }
        if( i > 1 && createNewRoute ) pSelect->AddSelectableRouteSegment( prevPoint->m_lat,
                prevPoint->m_lon, curPoint->m_lat, curPoint->m_lon, prevPoint, newPoint, newRoute );
        prevPoint = newPoint;
    }

    if( createNewRoute ) {
        pRouteList->Append( newRoute );
        pConfig->AddNewRoute( newRoute );    // use auto next num

        if( pRoutePropDialog && pRoutePropDialog->IsShown() ) {
            pRoutePropDialog->SetRouteAndUpdate( newRoute );
        }

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
            pRouteManagerDialog->UpdateRouteListCtrl();
            pRouteManagerDialog->UpdateWptListCtrl();
        }
        gFrame->InvalidateAllGL();
        gFrame->RefreshAllCanvas( false );
    }
    if( newPoint->GetScaMin() < g_focusCanvas->GetScaleValue() ) newPoint->ShowScaleWarningMessage(g_focusCanvas);
}

void pupHandler_PasteTrack() {
    Kml kml;

    int pasteBuffer = kml.ParsePasteBuffer();
    Track* pasted = kml.GetParsedTrack();
    if( ! pasted ) return;

    TrackPoint* curPoint;

    Track* newTrack = new Track();
    TrackPoint* newPoint;
    TrackPoint* prevPoint = NULL;

    newTrack->SetName(pasted->GetName());

    for( int i = 0; i < pasted->GetnPoints(); i++ ) {
        curPoint = pasted->GetPoint( i );

        newPoint = new TrackPoint( curPoint );

        wxDateTime now = wxDateTime::Now();
        newPoint->SetCreateTime(curPoint->GetCreateTime());

        newTrack->AddPoint( newPoint );

        if( prevPoint )
            pSelect->AddSelectableTrackSegment(
                prevPoint->m_lat, prevPoint->m_lon,
                newPoint->m_lat, newPoint->m_lon,
                prevPoint, newPoint, newTrack );

        prevPoint = newPoint;
    }

    pTrackList->Append( newTrack );
    pConfig->AddNewTrack( newTrack );

    gFrame->InvalidateAllGL();
    gFrame->RefreshAllCanvas( false );
}

bool ChartCanvas::InvokeCanvasMenu(int x, int y, int seltype)
{
    m_canvasMenu = new CanvasMenuHandler(this, m_pSelectedRoute, m_pSelectedTrack,
                                         m_pFoundRoutePoint, m_FoundAIS_MMSI, m_pIDXCandidate);
    
    Connect(  wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction) (wxEventFunction) &ChartCanvas::PopupMenuHandler );

    m_canvasMenu->CanvasPopupMenu( x, y, seltype );

    Disconnect(  wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction) (wxEventFunction) &ChartCanvas::PopupMenuHandler );

    delete m_canvasMenu;
    m_canvasMenu = NULL;

#ifdef __WXQT__
    gFrame->SurfaceToolbar();
    //g_MainToolbar->Raise();
#endif
    
    return true;
}

void ChartCanvas::PopupMenuHandler( wxCommandEvent& event )
{
    //  Pass menu events from the canvas to the menu handler
    //  This is necessarily in ChartCanvas since that is the menu's parent.
    if(m_canvasMenu){
        m_canvasMenu->PopupMenuHandler( event );
    }
    return;
    
}

void ChartCanvas::StartRoute( void )
{
    // Do not allow more than one canvas to create a route at one time.
    if(g_brouteCreating)
        return;
    
    if(g_MainToolbar)
        g_MainToolbar->DisableTooltips();
    
    g_brouteCreating = true;
    m_routeState = 1;
    m_bDrawingRoute = false;
    SetCursor( *pCursorPencil );
    SetCanvasToolbarItemState( ID_ROUTE, true );
    gFrame->SetMasterToolbarItemState( ID_MENU_ROUTE_NEW, true );
   
    HideGlobalToolbar();

#ifdef __OCPN__ANDROID__
    androidSetRouteAnnunciator(true);
#endif        
    
}

void ChartCanvas::FinishRoute( void )
{
    m_routeState = 0;
    m_prev_pMousePoint = NULL;
    m_bDrawingRoute = false;

    SetCanvasToolbarItemState( ID_ROUTE, false );
    gFrame->SetMasterToolbarItemState( ID_MENU_ROUTE_NEW, false );
#ifdef __OCPN__ANDROID__
    androidSetRouteAnnunciator(false);
#endif        
    
    SetCursor( *pCursorArrow );

    if( m_pMouseRoute ) {
        if( m_bAppendingRoute ) 
            pConfig->UpdateRoute( m_pMouseRoute );
        else {
            if( m_pMouseRoute->GetnPoints() > 1 ) {
                pConfig->AddNewRoute( m_pMouseRoute );
            } else {
                g_pRouteMan->DeleteRoute( m_pMouseRoute );
                m_pMouseRoute = NULL;
            }
        }
        if( m_pMouseRoute )
            m_pMouseRoute->SetHiLite(0);

        if( pRoutePropDialog && pRoutePropDialog->IsShown() ) {
            pRoutePropDialog->SetRouteAndUpdate( m_pMouseRoute, true );
        }

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateRouteListCtrl();

    }
    m_bAppendingRoute = false;
    m_pMouseRoute = NULL;

    m_pSelectedRoute = NULL;

    undo->InvalidateUndo();
    gFrame->RefreshAllCanvas( true );
    
    if(g_MainToolbar)
        g_MainToolbar->EnableTooltips();

    ShowGlobalToolbar();

    g_brouteCreating = false;
}

void ChartCanvas::HideGlobalToolbar()
{
    if(m_canvasIndex == 0){
        m_last_TBviz = gFrame->SetGlobalToolbarViz( false );
    }
}

void ChartCanvas::ShowGlobalToolbar()
{
    if(m_canvasIndex == 0){
        if(m_last_TBviz)
            gFrame->SetGlobalToolbarViz( true );
    }
}


        
        
void ChartCanvas::ShowAISTargetList( void )
{
    if( NULL == g_pAISTargetList ) {         // There is one global instance of the Dialog
        g_pAISTargetList = new AISTargetListDialog( parent_frame, g_pauimgr, g_pAIS );
    }

    g_pAISTargetList->UpdateAISTargetList();

}

void ChartCanvas::RenderAllChartOutlines( ocpnDC &dc, ViewPort& vp )
{
    if( !m_bShowOutlines ) return;

    int nEntry = ChartData->GetChartTableEntries();

    for( int i = 0; i < nEntry; i++ ) {
        ChartTableEntry *pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( i );

        //    Check to see if the candidate chart is in the currently active group
        bool b_group_draw = false;
        if( m_groupIndex > 0 ) {
            for( unsigned int ig = 0; ig < pt->GetGroupArray().size(); ig++ ) {
                int index = pt->GetGroupArray()[ig];
                if( m_groupIndex == index ) {
                    b_group_draw = true;
                    break;
                }
            }
        } else
            b_group_draw = true;

        if( b_group_draw ) RenderChartOutline( dc, i, vp );
    }

#ifdef USE_S57
    //        On CM93 Composite Charts, draw the outlines of the next smaller scale cell
    cm93compchart *pcm93 = NULL;
    if( VPoint.b_quilt ) {
        for(ChartBase *pch = GetFirstQuiltChart(); pch; pch = GetNextQuiltChart())
            if( pch->GetChartType() == CHART_TYPE_CM93COMP ) {
                pcm93 = (cm93compchart *)pch;
                break;
            }
    } else
        if ( m_singleChart && ( m_singleChart->GetChartType() == CHART_TYPE_CM93COMP ) )
            pcm93 = (cm93compchart *) m_singleChart;

    if( pcm93 ) {
        double chart_native_ppm = m_canvas_scale_factor / pcm93->GetNativeScale();
        double zoom_factor = GetVP().view_scale_ppm / chart_native_ppm;

        if( zoom_factor > 8.0 ) {
            wxPen mPen( GetGlobalColor( _T("UINFM") ), 2, wxPENSTYLE_SHORT_DASH );
            dc.SetPen( mPen );
        } else {
            wxPen mPen( GetGlobalColor( _T("UINFM") ), 1, wxPENSTYLE_SOLID );
            dc.SetPen( mPen );
        } 
        
        pcm93->RenderNextSmallerCellOutlines( dc, vp, this );
    }
#endif
}

void ChartCanvas::RenderChartOutline( ocpnDC &dc, int dbIndex, ViewPort& vp )
{
#ifdef ocpnUSE_GL
    if(g_bopengl && m_glcc) {
        /* opengl version specially optimized */
        m_glcc->RenderChartOutline(dbIndex, vp);
        return;
    }
#endif

    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_PLUGIN ){
        if( !ChartData->IsChartAvailable( dbIndex ) )
            return;
    }

    float plylat, plylon;
    float plylat1, plylon1;

    int pixx, pixy, pixx1, pixy1;

    LLBBox box;
    ChartData->GetDBBoundingBox( dbIndex, box );

    // Don't draw an outline in the case where the chart covers the entire world */
    if(box.GetLonRange() == 360)
        return;

    double lon_bias = 0;
    // chart is outside of viewport lat/lon bounding box
    if( box.IntersectOutGetBias( vp.GetBBox(), lon_bias ) )
        return;

    int nPly = ChartData->GetDBPlyPoint( dbIndex, 0, &plylat, &plylon );

    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_CM93 )
        dc.SetPen( wxPen( GetGlobalColor( _T ( "YELO1" ) ), 1, wxPENSTYLE_SOLID ) );

    else if( ChartData->GetDBChartFamily( dbIndex ) == CHART_FAMILY_VECTOR )
        dc.SetPen( wxPen( GetGlobalColor( _T ( "UINFG" ) ), 1, wxPENSTYLE_SOLID ) );

    else
        dc.SetPen( wxPen( GetGlobalColor( _T ( "UINFR" ) ), 1, wxPENSTYLE_SOLID ) );

    //        Are there any aux ply entries?
    int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex );
    if( 0 == nAuxPlyEntries )                 // There are no aux Ply Point entries
    {
        wxPoint r, r1;

        ChartData->GetDBPlyPoint( dbIndex, 0, &plylat, &plylon );
        plylon += lon_bias;

        GetCanvasPointPix( plylat, plylon, &r );
        pixx = r.x;
        pixy = r.y;

        for( int i = 0; i < nPly - 1; i++ ) {
            ChartData->GetDBPlyPoint( dbIndex, i + 1, &plylat1, &plylon1 );
            plylon1 += lon_bias;

            GetCanvasPointPix( plylat1, plylon1, &r1 );
            pixx1 = r1.x;
            pixy1 = r1.y;

            int pixxs1 = pixx1;
            int pixys1 = pixy1;

            bool b_skip = false;

            if( vp.chart_scale > 5e7 ) {
                //    calculate projected distance between these two points in meters
                double dist = sqrt( pow( (double) (pixx1 - pixx), 2 ) +
                                    pow( (double) (pixy1 - pixy), 2 ) ) / vp.view_scale_ppm;
                                    
                if(dist > 0.0){
                    //    calculate GC distance between these two points in meters
                    double distgc = DistGreatCircle( plylat, plylon, plylat1, plylon1 ) * 1852.;

                    //    If the distances are nonsense, it means that the scale is very small and the segment wrapped the world
                    //    So skip it....
                    //    TODO improve this to draw two segments
                    if( fabs( dist - distgc ) > 10000. * 1852. )          //lotsa miles
                        b_skip = true;
                }
                else
                    b_skip = true;
            }

            ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                             vp.pix_width, 0, vp.pix_height );
            if( res != Invisible && !b_skip ) dc.DrawLine( pixx, pixy, pixx1, pixy1, false );

            plylat = plylat1;
            plylon = plylon1;
            pixx = pixxs1;
            pixy = pixys1;
        }

        ChartData->GetDBPlyPoint( dbIndex, 0, &plylat1, &plylon1 );
        plylon1 += lon_bias;

        GetCanvasPointPix( plylat1, plylon1, &r1 );
        pixx1 = r1.x;
        pixy1 = r1.y;

        ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                         vp.pix_width, 0, vp.pix_height );
        if( res != Invisible ) dc.DrawLine( pixx, pixy, pixx1, pixy1, false );
    }

    else                              // Use Aux PlyPoints
    {
        wxPoint r, r1;

        int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex );
        for( int j = 0; j < nAuxPlyEntries; j++ ) {

            int nAuxPly = ChartData->GetDBAuxPlyPoint( dbIndex, 0, j, &plylat, &plylon );
            GetCanvasPointPix( plylat, plylon, &r );
            pixx = r.x;
            pixy = r.y;

            for( int i = 0; i < nAuxPly - 1; i++ ) {
                ChartData->GetDBAuxPlyPoint( dbIndex, i + 1, j, &plylat1, &plylon1 );

                GetCanvasPointPix( plylat1, plylon1, &r1 );
                pixx1 = r1.x;
                pixy1 = r1.y;

                int pixxs1 = pixx1;
                int pixys1 = pixy1;

                bool b_skip = false;

                if( vp.chart_scale > 5e7 ) {
                    //    calculate projected distance between these two points in meters
                    double dist = sqrt(
                                      (double) ( ( pixx1 - pixx ) * ( pixx1 - pixx ) )
                                      + ( ( pixy1 - pixy ) * ( pixy1 - pixy ) ) ) / vp.view_scale_ppm;
                   if(dist > 0.0){
                   //    calculate GC distance between these two points in meters
                        double distgc = DistGreatCircle( plylat, plylon, plylat1, plylon1 ) * 1852.;

                    //    If the distances are nonsense, it means that the scale is very small and the segment wrapped the world
                    //    So skip it....
                    //    TODO improve this to draw two segments
                        if( fabs( dist - distgc ) > 10000. * 1852. )          //lotsa miles
                            b_skip = true;
                    }
                    else
                        b_skip = true;
                }

                ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                                 vp.pix_width, 0, vp.pix_height );
                if( res != Invisible && !b_skip ) dc.DrawLine( pixx, pixy, pixx1, pixy1 );

                plylat = plylat1;
                plylon = plylon1;
                pixx = pixxs1;
                pixy = pixys1;
            }

            ChartData->GetDBAuxPlyPoint( dbIndex, 0, j, &plylat1, &plylon1 );
            GetCanvasPointPix( plylat1, plylon1, &r1 );
            pixx1 = r1.x;
            pixy1 = r1.y;

            ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                             vp.pix_width, 0, vp.pix_height );
            if( res != Invisible ) dc.DrawLine( pixx, pixy, pixx1, pixy1, false );
        }
    }

}

static void RouteLegInfo( ocpnDC &dc, wxPoint ref_point, const wxString &first, const wxString &second )
{
    wxFont *dFont = FontMgr::Get().GetFont( _("RouteLegInfoRollover") );
    dc.SetFont( *dFont );

    int w1, h1;
    int w2 = 0;
    int h2 = 0;
    int h, w;
    
    int xp, yp;
    int hilite_offset = 3;
#ifdef __WXMAC__
    wxScreenDC sdc;
    sdc.GetTextExtent(first, &w1, &h1, NULL, NULL, dFont);
    if(second.Len())
        sdc.GetTextExtent(second, &w2, &h2, NULL, NULL, dFont);
#else
    dc.GetTextExtent( first, &w1, &h1 );
    if(second.Len())
        dc.GetTextExtent( second, &w2, &h2 );
#endif

    w = wxMax(w1, w2);
    h = h1 + h2;
    
    xp = ref_point.x - w;
    yp = ref_point.y;
    yp += hilite_offset;

    AlphaBlending( dc, xp, yp, w, h, 0.0, GetGlobalColor( _T ( "YELO1" ) ), 172 );
    
    dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ) ) );
    dc.SetTextForeground( FontMgr::Get().GetFontColor( _("RouteLegInfoRollover") ) );

    dc.DrawText( first, xp, yp );
    if(second.Len())
        dc.DrawText( second, xp, yp + h1 );
}

void ChartCanvas::RenderRouteLegs( ocpnDC &dc )
{
    if( (m_routeState >= 2) ||
        (m_pMeasureRoute && m_bMeasure_Active && ( m_nMeasureState >= 2 )) ) {

        Route* route = 0;
        int state;
        if( m_pMeasureRoute ) {
            route = m_pMeasureRoute;
            state = m_nMeasureState;
        } else {
            route = m_pMouseRoute;
            state = m_routeState;
        }
        
        if(!route)
            return;
    
        double render_lat = m_cursor_lat;
        double render_lon = m_cursor_lon;
        
        if(route){
            int np = route->GetnPoints();
            if(np){
                if(g_btouch && (np > 1))
                    np --;
                RoutePoint rp = route->GetPoint(np);
                render_lat = rp.m_lat;
                render_lon = rp.m_lon;
            }
        }
                
        double rhumbBearing, rhumbDist, gcBearing, gcBearing2, gcDist;
        DistanceBearingMercator( m_cursor_lat, m_cursor_lon, render_lat, render_lon, &rhumbBearing, &rhumbDist );
        Geodesic::GreatCircleDistBear( render_lon, render_lat, m_cursor_lon, m_cursor_lat, &gcDist, &gcBearing, &gcBearing2);
        double gcDistm = gcDist / 1852.0;

        if( ( render_lat == m_cursor_lat ) && ( render_lon == m_cursor_lon ) ) rhumbBearing = 90.;

        wxPoint destPoint, lastPoint;


        double brg = rhumbBearing;
        double dist = rhumbDist;
        route->m_NextLegGreatCircle = false;
        int milesDiff = rhumbDist - gcDistm;
        if( milesDiff > 1 ) {
            brg = gcBearing;
            dist = gcDistm;
            route->m_NextLegGreatCircle = true;
        }

        if( 1/*!g_btouch*/) {
            route->DrawPointWhich( dc, this, route->m_lastMousePointIndex, &lastPoint );

            if( route->m_NextLegGreatCircle ) {
                for( int i=1; i<=milesDiff; i++ ) {
                    double p = (double)i * (1.0/(double)milesDiff);
                    double pLat, pLon;
                    Geodesic::GreatCircleTravel( render_lon, render_lat, gcDist*p, brg, &pLon, &pLat, &gcBearing2 );
                    destPoint = VPoint.GetPixFromLL( pLat, pLon );
                    route->DrawSegment( dc, this, &lastPoint, &destPoint, GetVP(), false );
                    lastPoint = destPoint;
                }
            }
            else {
                if (r_rband.x && r_rband.y) {    // RubberBand disabled?
                    route->DrawSegment(dc, this, &lastPoint, &r_rband, GetVP(), false);

                    if (m_bMeasure_DistCircle) {
                        double distanceRad = sqrtf(powf((float)(r_rband.x - lastPoint.x), 2) +
                            powf((float)(r_rband.y - lastPoint.y), 2));

                        dc.SetPen(*g_pRouteMan->GetRoutePen());
                        dc.SetBrush(*wxTRANSPARENT_BRUSH);
                        dc.StrokeCircle(lastPoint.x, lastPoint.y, distanceRad);
                    }
                }
            }
        }

        wxString routeInfo;
        if( g_bShowTrue )
            routeInfo << wxString::Format( wxString("%03d°  ", wxConvUTF8 ), (int)brg );
        if( g_bShowMag ){
            double latAverage = (m_cursor_lat + render_lat)/2;
            double lonAverage = (m_cursor_lon + render_lon)/2;
            double varBrg = gFrame->GetMag( brg, latAverage, lonAverage);
            
            routeInfo << wxString::Format( wxString("%03d°(M)  ", wxConvUTF8 ), (int)varBrg );
        }

        routeInfo << _T(" ") << FormatDistanceAdaptive( dist );

        wxString s0;
        if( !route->m_bIsInLayer )
            s0.Append( _("Route") + _T(": ") );
        else
            s0.Append( _("Layer Route: ") );

        double disp_length = route->m_route_length;
        if( !g_btouch)
            disp_length += dist;
        s0 += FormatDistanceAdaptive( disp_length );

        RouteLegInfo( dc, r_rband, routeInfo, s0 );

        m_brepaint_piano = true;
    }
}

void ChartCanvas::WarpPointerDeferred( int x, int y )
{
    warp_x = x;
    warp_y = y;
    warp_flag = true;
}


int s_msg;

void ChartCanvas::UpdateCanvasS52PLIBConfig()
{
    if(!ps52plib)
        return;
    
    if( VPoint.b_quilt ){          // quilted
        if( !m_pQuilt || !m_pQuilt->IsComposed() ) 
            return;  // not ready
            
        if(m_pQuilt->IsQuiltVector()){    
            if(ps52plib->GetStateHash() != m_s52StateHash){
                UpdateS52State();
                m_s52StateHash = ps52plib->GetStateHash();
            }
        }
    }
    else{
        if(ps52plib->GetStateHash() != m_s52StateHash){
            UpdateS52State();
            m_s52StateHash = ps52plib->GetStateHash();
        }
    }
    
    // Plugin charts
    bool bSendPlibState = true;
    if( VPoint.b_quilt ){          // quilted
        if(!m_pQuilt->DoesQuiltContainPlugins())
            bSendPlibState = false;
    }
       
    if(bSendPlibState){   
        wxJSONValue v;
        v[_T("OpenCPN Version Major")] = VERSION_MAJOR;
        v[_T("OpenCPN Version Minor")] = VERSION_MINOR;
        v[_T("OpenCPN Version Patch")] = VERSION_PATCH;
        v[_T("OpenCPN Version Date")] = VERSION_DATE;
        
        //  S52PLIB state
        v[_T("OpenCPN S52PLIB ShowText")] = GetShowENCText();
        v[_T("OpenCPN S52PLIB ShowSoundings")] = GetShowENCDepth();
        v[_T("OpenCPN S52PLIB ShowLights")] = GetShowENCLights();
        v[_T("OpenCPN S52PLIB ShowAnchorConditions")] = m_encShowAnchor; //ps52plib->GetAnchorOn();
        v[_T("OpenCPN S52PLIB ShowQualityOfData")] = GetShowENCDataQual(); //ps52plib->GetQualityOfDataOn();
        v[_T("OpenCPN S52PLIB ShowATONLabel")] = GetShowENCBuoyLabels();
        v[_T("OpenCPN S52PLIB ShowLightDescription")] = GetShowENCLightDesc();

        v[_T("OpenCPN S52PLIB DisplayCategory")] = GetENCDisplayCategory();
        
        // Global options
/*        
        v[_T("OpenCPN S52PLIB MetaDisplay")] = ps52plib->m_bShowMeta;
        v[_T("OpenCPN S52PLIB DeclutterText")] = ps52plib->m_bDeClutterText;
        v[_T("OpenCPN S52PLIB ShowNationalText")] = ps52plib->m_bShowNationalTexts;
        v[_T("OpenCPN S52PLIB ShowImportantTextOnly")] = ps52plib->m_bShowS57ImportantTextOnly;
        v[_T("OpenCPN S52PLIB UseSCAMIN")] = ps52plib->m_bUseSCAMIN;
        v[_T("OpenCPN S52PLIB SymbolStyle")] = ps52plib->m_nSymbolStyle;
        v[_T("OpenCPN S52PLIB BoundaryStyle")] = ps52plib->m_nBoundaryStyle;
        v[_T("OpenCPN S52PLIB ColorShades")] = S52_getMarinerParam( S52_MAR_TWO_SHADES );
*/        
        wxJSONWriter w;
        wxString out;
        w.Write(v, out);
        
        if(!g_lastPluginMessage.IsSameAs(out)){
            //printf("message %d  %d\n", s_msg++, m_canvasIndex);
            g_pi_manager->SendMessageToAllPlugins(wxString(_T("OpenCPN Config")), out);
        }
    }
}
int spaint;
int s_in_update;
void ChartCanvas::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );

    //GetToolbar()->Show( m_bToolbarEnable );
    
    //  Paint updates may have been externally disabled (temporarily, to avoid Yield() recursion performance loss)
    //  It is important that the wxPaintDC is built, even if we elect to not process this paint message.
    //  Otherwise, the paint message may not be removed from the message queue, esp on Windows. (FS#1213)
    //  This would lead to a deadlock condition in ::wxYield()

    if(!m_b_paint_enable){
        return;
    }

    
    //  If necessary, reconfigure the S52 PLIB
    UpdateCanvasS52PLIBConfig();
    
    
#ifdef ocpnUSE_GL
    if( !g_bdisable_opengl && m_glcc )
        m_glcc->Show( g_bopengl );

    if( m_glcc && g_bopengl ) {
        if( !s_in_update ) {          // no recursion allowed, seen on lo-spec Mac
            s_in_update++;
            m_glcc->Update();
            s_in_update--;
        }

        return;
    }
#endif

    if( ( GetVP().pix_width == 0 ) || ( GetVP().pix_height == 0 ) ) return;

    wxRegion ru = GetUpdateRegion();

    int rx, ry, rwidth, rheight;
    ru.GetBox( rx, ry, rwidth, rheight );
        //printf("%d Onpaint update region box: %d %d %d %d\n", spaint++, rx, ry, rwidth, rheight);

#ifdef ocpnUSE_DIBSECTION
    ocpnMemDC temp_dc;
#else
    wxMemoryDC temp_dc;
#endif

    long height = GetVP().pix_height;

#ifdef __WXMAC__
    //On OS X we have to explicitly extend the region for the piano area
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    if(!style->chartStatusWindowTransparent && g_bShowChartBar)
        height += m_Piano->GetHeight();
#endif // __WXMAC__
    wxRegion rgn_chart( 0, 0, GetVP().pix_width, height );

//    In case Thumbnail is shown, set up dc clipper and blt iterator regions
    if( pthumbwin ) {
        int thumbx, thumby, thumbsx, thumbsy;
        pthumbwin->GetPosition( &thumbx, &thumby );
        pthumbwin->GetSize( &thumbsx, &thumbsy );
        wxRegion rgn_thumbwin( thumbx, thumby, thumbsx - 1, thumbsy - 1 );

        if( pthumbwin->IsShown() ) {
            rgn_chart.Subtract( rgn_thumbwin );
            ru.Subtract( rgn_thumbwin );
        }
    }

    // subtract the chart bar if it isn't transparent, and determine if we need to paint it
    wxRegion rgn_blit = ru;
    if(g_bShowChartBar) {
        wxRect chart_bar_rect(0, GetClientSize().y - m_Piano->GetHeight(),
                              GetClientSize().x, m_Piano->GetHeight());

        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        if(ru.Contains(chart_bar_rect) != wxOutRegion) {
            if(style->chartStatusWindowTransparent)
                m_brepaint_piano = true;
            else
                ru.Subtract(chart_bar_rect);
        }        
    }

    if(m_Compass && m_Compass->IsShown()){
        wxRect compassRect = m_Compass->GetRect();
        if(ru.Contains(compassRect) != wxOutRegion) {
            ru.Subtract(compassRect);
        }
    }
            
    
    
    //  Is this viewpoint the same as the previously painted one?
    bool b_newview = true;

    if( ( m_cache_vp.view_scale_ppm == VPoint.view_scale_ppm )
            && ( m_cache_vp.rotation == VPoint.rotation ) && ( m_cache_vp.clat == VPoint.clat )
            && ( m_cache_vp.clon == VPoint.clon ) && m_cache_vp.IsValid() ) {
        b_newview = false;
    }

    //  If the ViewPort is skewed or rotated, we may be able to use the cached rotated bitmap.
    bool b_rcache_ok = false;
    if( fabs( VPoint.skew ) > 0.01 || fabs( VPoint.rotation ) > 0.01)
        b_rcache_ok = !b_newview;

    //  Make a special VP
    if( VPoint.b_MercatorProjectionOverride ) VPoint.SetProjectionType( PROJECTION_MERCATOR );
    ViewPort svp = VPoint;

    svp.pix_width = svp.rv_rect.width;
    svp.pix_height = svp.rv_rect.height;

//        printf("Onpaint pix %d %d\n", VPoint.pix_width, VPoint.pix_height);
//        printf("OnPaint rv_rect %d %d\n", VPoint.rv_rect.width, VPoint.rv_rect.height);

    OCPNRegion chart_get_region( wxRect( 0, 0, svp.pix_width, svp.pix_height ) );

    //  If we are going to use the cached rotated image, there is no need to fetch any chart data
    //  and this will do it...
    if( b_rcache_ok ) chart_get_region.Clear();

    //  Blit pan acceleration
    if( VPoint.b_quilt )          // quilted
    {
        if( !m_pQuilt || !m_pQuilt->IsComposed() ) 
            return;  // not ready

        bool bvectorQuilt = m_pQuilt->IsQuiltVector();    
        
        bool busy = false;
        if( bvectorQuilt &&
            ( m_cache_vp.view_scale_ppm != VPoint.view_scale_ppm || m_cache_vp.rotation != VPoint.rotation))
        {
            OCPNPlatform::ShowBusySpinner();
            busy = true;
        }
        
        if( ( m_working_bm.GetWidth() != svp.pix_width )
                || ( m_working_bm.GetHeight() != svp.pix_height ) ) m_working_bm.Create(
                        svp.pix_width, svp.pix_height, -1 ); // make sure the target is big enoug

        if( fabs( VPoint.rotation ) < 0.01 ) {
            bool b_save = true;

            //  If the saved wxBitmap from last OnPaint is useable
            //  calculate the blit parameters

            //  We can only do screen blit painting if subsequent ViewPorts differ by whole pixels
            //  So, in small scale bFollow mode, force the full screen render.
            //  This seems a hack....There may be better logic here.....

//                  if(m_bFollow)
//                        b_save = false;

            if( m_bm_cache_vp.IsValid() && m_cache_vp.IsValid() /*&& !m_bFollow*/) {
                if( b_newview ) {
                    wxPoint c_old = VPoint.GetPixFromLL( VPoint.clat, VPoint.clon );
                    wxPoint c_new = m_bm_cache_vp.GetPixFromLL( VPoint.clat, VPoint.clon );

                    int dy = c_new.y - c_old.y;
                    int dx = c_new.x - c_old.x;

//                              printf("In OnPaint Trying Blit dx: %d  dy:%d\n\n", dx, dy);

                    if( m_pQuilt->IsVPBlittable( VPoint, dx, dy, true ) ) {
                        if( dx || dy ) {
                            //  Blit the reuseable portion of the cached wxBitmap to a working bitmap
                            temp_dc.SelectObject( m_working_bm );

                            wxMemoryDC cache_dc;
                            cache_dc.SelectObject( m_cached_chart_bm );

                            if( dy > 0 ) {
                                if( dx > 0 ) {
                                    temp_dc.Blit( 0, 0, VPoint.pix_width - dx,
                                                               VPoint.pix_height - dy, &cache_dc, dx, dy );
                                }
                                else {
                                    temp_dc.Blit( -dx, 0, VPoint.pix_width + dx,
                                                  VPoint.pix_height - dy, &cache_dc, 0, dy );
                                }

                            } else {
                                if( dx > 0 ) {
                                    temp_dc.Blit( 0, -dy, VPoint.pix_width - dx,
                                                               VPoint.pix_height + dy, &cache_dc, dx, 0 );
                                }
                                else {
                                    temp_dc.Blit( -dx, -dy, VPoint.pix_width + dx,
                                                  VPoint.pix_height + dy, &cache_dc, 0, 0 );
                                }
                            }

                            OCPNRegion update_region;
                            if( dy ) {
                                if( dy > 0 ) update_region.Union(
                                        wxRect( 0, VPoint.pix_height - dy, VPoint.pix_width, dy ) );
                                else
                                    update_region.Union( wxRect( 0, 0, VPoint.pix_width, -dy ) );
                            }

                            if( dx ) {
                                if( dx > 0 ) update_region.Union(
                                        wxRect( VPoint.pix_width - dx, 0, dx, VPoint.pix_height ) );
                                else
                                    update_region.Union( wxRect( 0, 0, -dx, VPoint.pix_height ) );
                            }

                            //  Render the new region
                            m_pQuilt->RenderQuiltRegionViewOnDCNoText( temp_dc, svp, update_region );
                            cache_dc.SelectObject( wxNullBitmap );
                        } else {
                            //    No sensible (dx, dy) change in the view, so use the cached member bitmap
                            temp_dc.SelectObject( m_cached_chart_bm );
                            b_save = false;

                        }
                        m_pQuilt->ComputeRenderRegion( svp, chart_get_region );

                    } else              // not blitable
                    {
                        temp_dc.SelectObject( m_working_bm );
                        m_pQuilt->RenderQuiltRegionViewOnDCNoText( temp_dc, svp, chart_get_region );
                    }
                } else {
                    //    No change in the view, so use the cached member bitmap2
                    temp_dc.SelectObject( m_cached_chart_bm );
                    b_save = false;
                }
            } else      //cached bitmap is not yet valid
            {
                temp_dc.SelectObject( m_working_bm );
                m_pQuilt->RenderQuiltRegionViewOnDCNoText( temp_dc, svp, chart_get_region );
            }

            //  Save the fully rendered quilt image as a wxBitmap member of this class
            if( b_save ) {
//                        if((m_cached_chart_bm.GetWidth() != svp.pix_width) || (m_cached_chart_bm.GetHeight() != svp.pix_height))
//                              m_cached_chart_bm.Create(svp.pix_width, svp.pix_height, -1); // target wxBitmap is big enough
                wxMemoryDC scratch_dc_0;
                scratch_dc_0.SelectObject( m_cached_chart_bm );
                scratch_dc_0.Blit( 0, 0, svp.pix_width, svp.pix_height, &temp_dc, 0, 0 );

                scratch_dc_0.SelectObject( wxNullBitmap );

                m_bm_cache_vp = VPoint;     // save the ViewPort associated with the cached wxBitmap
            }
        }

        else            // quilted, rotated
        {
            temp_dc.SelectObject( m_working_bm );
            OCPNRegion chart_get_all_region( wxRect( 0, 0, svp.pix_width, svp.pix_height ) );
            m_pQuilt->RenderQuiltRegionViewOnDCNoText( temp_dc, svp, chart_get_all_region );
        }
        
        if(busy)
            OCPNPlatform::HideBusySpinner();
        
    }

    else                  // not quilted
    {
        if( !m_singleChart ) {
            dc.SetBackground( wxBrush( *wxLIGHT_GREY ) );
            dc.Clear();
            return;
        }

        if(!chart_get_region.IsEmpty()){
            m_singleChart->RenderRegionViewOnDC( temp_dc, svp, chart_get_region );
        }            
    }

    if(temp_dc.IsOk() ) {
    
//    Arrange to render the World Chart vector data behind the rendered current chart
//    so that uncovered canvas areas show at least the world chart.
        OCPNRegion chartValidRegion;
        if( !VPoint.b_quilt ) {
            // Make a region covering the current chart on the canvas

            if(m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR)
                m_singleChart->GetValidCanvasRegion( svp, &chartValidRegion );
            else {
                // The raster calculations  in ChartBaseBSB::ComputeSourceRectangle
                // require that the viewport passed here have pix_width and pix_height
                // set to the actual display, not the virtual (rv_rect) sizes
                // (the vector calculations require the virtual sizes in svp)

                m_singleChart->GetValidCanvasRegion( VPoint, &chartValidRegion );
                chartValidRegion.Offset(-VPoint.rv_rect.x, -VPoint.rv_rect.y);
            }
        }
        else
            chartValidRegion = m_pQuilt->GetFullQuiltRenderedRegion();

        temp_dc.DestroyClippingRegion();
    
        //    Copy current chart region
        OCPNRegion backgroundRegion( wxRect(0, 0, svp.pix_width, svp.pix_height) );

        if( chartValidRegion.IsOk() )
            backgroundRegion.Subtract( chartValidRegion );

        if( ! backgroundRegion.IsEmpty() ) {
        
            //    Draw the Background Chart only in the areas NOT covered by the current chart view

            /* unfortunately wxDC::DrawRectangle and wxDC::Clear do not respect
               clipping regions with more than 1 rectangle so... */
            wxColour water = pWorldBackgroundChart->water;
            if(water.IsOk()){
                temp_dc.SetPen( *wxTRANSPARENT_PEN );
                temp_dc.SetBrush( wxBrush( water ) );
                OCPNRegionIterator upd( backgroundRegion ); // get the update rect list
                while( upd.HaveRects() ) {
                    wxRect rect = upd.GetRect();
                    temp_dc.DrawRectangle(rect);
                    upd.NextRect();
                }
            }
            //    Associate with temp_dc
            wxRegion *clip_region = backgroundRegion.GetNew_wxRegion();
            temp_dc.SetDeviceClippingRegion( *clip_region );
            delete clip_region;

            ocpnDC bgdc( temp_dc );
            double r = VPoint.rotation;
            SetVPRotation(VPoint.skew);

            pWorldBackgroundChart->RenderViewOnDC( bgdc, VPoint );
            SetVPRotation( r );
        }
    } // temp_dc.IsOk();

    wxMemoryDC *pChartDC = &temp_dc;
    wxMemoryDC rotd_dc;
    
    if( ( ( fabs( GetVP().rotation ) > 0.01 ) )
        ||   ( fabs( GetVP().skew ) > 0.01 ) )  {
        
        //  Can we use the current rotated image cache?
        if( !b_rcache_ok ) {
#ifdef __WXMSW__
            wxMemoryDC tbase_dc;
            wxBitmap bm_base( svp.pix_width, svp.pix_height, -1 );
            tbase_dc.SelectObject( bm_base );
            tbase_dc.Blit( 0, 0, svp.pix_width, svp.pix_height, &temp_dc, 0, 0 );
            tbase_dc.SelectObject( wxNullBitmap );
#else
            const wxBitmap &bm_base = temp_dc.GetSelectedBitmap();
#endif

            wxImage base_image;
            if( bm_base.IsOk() ) base_image = bm_base.ConvertToImage();

            //    Use a local static image rotator to improve wxWidgets code profile
            //    Especially, on GTK the wxRound and wxRealPoint functions are very expensive.....

            double angle = GetVP().skew - GetVP().rotation;
            wxImage ri;
            bool b_rot_ok = false;
            if( base_image.IsOk() ) {
                ViewPort rot_vp = GetVP();

                m_b_rot_hidef = false;

                ri = Image_Rotate( base_image, angle,
                                   wxPoint( GetVP().rv_rect.width / 2, GetVP().rv_rect.height / 2 ),
                                   m_b_rot_hidef, &m_roffset );

                if( ( rot_vp.view_scale_ppm == VPoint.view_scale_ppm )
                        && ( rot_vp.rotation == VPoint.rotation ) && ( rot_vp.clat == VPoint.clat )
                        && ( rot_vp.clon == VPoint.clon ) && rot_vp.IsValid() && ( ri.IsOk() ) ) {
                    b_rot_ok = true;
                }
            }

            if( b_rot_ok ) {
                delete m_prot_bm;
                m_prot_bm = new wxBitmap( ri );

            }

            m_roffset.x += VPoint.rv_rect.x;
            m_roffset.y += VPoint.rv_rect.y;

        }

        if( m_prot_bm && m_prot_bm->IsOk() ) {
            rotd_dc.SelectObject( *m_prot_bm );
            pChartDC = &rotd_dc;
        } else {
            pChartDC = &temp_dc;
            m_roffset = wxPoint( 0, 0 );
        }
    } else {            // unrotated
        pChartDC = &temp_dc;
        m_roffset = wxPoint( 0, 0 );
    }
        
    wxPoint offset = m_roffset;

    //        Save the PixelCache viewpoint for next time
    m_cache_vp = VPoint;

//    Set up a scratch DC for overlay objects
    wxMemoryDC mscratch_dc;
    mscratch_dc.SelectObject( *pscratch_bm );

    mscratch_dc.ResetBoundingBox();
    mscratch_dc.DestroyClippingRegion();
    mscratch_dc.SetDeviceClippingRegion( rgn_chart );

    //    Blit the externally invalidated areas of the chart onto the scratch dc
    wxRegionIterator upd( rgn_blit ); // get the update rect list
    while( upd ) {
        wxRect rect = upd.GetRect();

        mscratch_dc.Blit( rect.x, rect.y, rect.width, rect.height, pChartDC, rect.x - offset.x,
                          rect.y - offset.y );
        upd++;
    }

    // If multi-canvas, indicate which canvas has keyboard focus
    // by drawing a simple blue bar at the top.
    if(g_canvasConfig != 0){             // multi-canvas?
        if( this == wxWindow::FindFocus()){
            g_focusCanvas = this;

            wxColour colour = GetGlobalColor(_T("BLUE4"));
            mscratch_dc.SetPen(wxPen(colour));
            mscratch_dc.SetBrush(wxBrush(colour));
            
            wxRect activeRect(0, 0, GetClientSize().x, m_focus_indicator_pix);
            mscratch_dc.DrawRectangle(activeRect);
        }
    }

    
//    Draw the rest of the overlay objects directly on the scratch dc
    ocpnDC scratch_dc( mscratch_dc );
    DrawOverlayObjects( scratch_dc, ru );

    if( m_bShowTide ){
        RebuildTideSelectList( GetVP().GetBBox() );
        DrawAllTidesInBBox( scratch_dc, GetVP().GetBBox() );
    }

    if( m_bShowCurrent ){
        RebuildCurrentSelectList( GetVP().GetBBox() );
        DrawAllCurrentsInBBox( scratch_dc, GetVP().GetBBox() );
    }

    if( m_brepaint_piano && g_bShowChartBar ) {
        m_Piano->Paint(GetClientSize().y - m_Piano->GetHeight(), mscratch_dc);
        //m_brepaint_piano = false;
    }

    if(m_Compass)
        m_Compass->Paint(scratch_dc);

    //quiting?
    if( g_bquiting ) {
#ifdef ocpnUSE_DIBSECTION
        ocpnMemDC q_dc;
#else
        wxMemoryDC q_dc;
#endif
        wxBitmap qbm( GetVP().pix_width, GetVP().pix_height );
        q_dc.SelectObject( qbm );

        // Get a copy of the screen
        q_dc.Blit( 0, 0, GetVP().pix_width, GetVP().pix_height, &mscratch_dc, 0, 0 );

        //  Draw a rectangle over the screen with a stipple brush
        wxBrush qbr( *wxBLACK, wxBRUSHSTYLE_FDIAGONAL_HATCH );
        q_dc.SetBrush( qbr );
        q_dc.DrawRectangle( 0, 0, GetVP().pix_width, GetVP().pix_height );

        // Blit back into source
        mscratch_dc.Blit( 0, 0, GetVP().pix_width, GetVP().pix_height, &q_dc, 0, 0, wxCOPY );

        q_dc.SelectObject( wxNullBitmap );

    }
#if 0
    //  It is possible that this two-step method may be reuired for some platforms.
    //  So, retain in the code base to aid recovery if necessary
    
    // Create and Render the Vector quilt decluttered text overlay, omitting CM93 composite
    if( VPoint.b_quilt ) {
        if(m_pQuilt->IsQuiltVector() && ps52plib && ps52plib->GetShowS57Text()){
            ChartBase *chart = m_pQuilt->GetRefChart();
            if( chart && chart->GetChartType() != CHART_TYPE_CM93COMP){
                
                //        Clear the text Global declutter list
                ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                if(ChPI)
                    ChPI->ClearPLIBTextList();
                else{
                    if(ps52plib)
                        ps52plib->ClearTextList();
                }
                
                wxMemoryDC t_dc;
                wxBitmap qbm(  GetVP().pix_width, GetVP().pix_height );
                
                wxColor maskBackground = wxColour(1,0,0);
                t_dc.SelectObject( qbm );
                t_dc.SetBackground(wxBrush(maskBackground));
                t_dc.Clear();

                //  Copy the scratch DC into the new bitmap
                t_dc.Blit( 0, 0, GetVP().pix_width, GetVP().pix_height, scratch_dc.GetDC(), 0, 0, wxCOPY );

                //  Render the text to the new bitmap
                OCPNRegion chart_all_text_region( wxRect( 0, 0, GetVP().pix_width, GetVP().pix_height ) );
                m_pQuilt->RenderQuiltRegionViewOnDCTextOnly( t_dc, svp, chart_all_text_region );
                
                //  Copy the new bitmap back to the scratch dc
                wxRegionIterator upd_final( ru );
                while( upd_final ) {
                    wxRect rect = upd_final.GetRect();
                    scratch_dc.GetDC()->Blit( rect.x, rect.y, rect.width, rect.height, &t_dc, rect.x, rect.y, wxCOPY, true );
                    upd_final++;
                }
                
                t_dc.SelectObject( wxNullBitmap );
            }
        }
    }
#endif
    // Direct rendering model...
    if( VPoint.b_quilt ) {
        if(m_pQuilt->IsQuiltVector() && ps52plib && ps52plib->GetShowS57Text()){
            ChartBase *chart = m_pQuilt->GetRefChart();
            if( chart && chart->GetChartType() != CHART_TYPE_CM93COMP){
                
                //        Clear the text Global declutter list
                ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                if(ChPI)
                    ChPI->ClearPLIBTextList();
                else{
                    if(ps52plib)
                        ps52plib->ClearTextList();
                }
                
                //  Render the text directly to the scratch bitmap
                OCPNRegion chart_all_text_region( wxRect( 0, 0, GetVP().pix_width, GetVP().pix_height ) );
                
                if(g_bShowChartBar && m_Piano) {
                    wxRect chart_bar_rect(0, GetVP().pix_height - m_Piano->GetHeight(),
                                          GetVP().pix_width, m_Piano->GetHeight());
                    
                    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
                    if(!style->chartStatusWindowTransparent)
                        chart_all_text_region.Subtract(chart_bar_rect);
                }
                
                if(m_Compass && m_Compass->IsShown()){
                    wxRect compassRect = m_Compass->GetRect();
                    if(chart_all_text_region.Contains(compassRect) != wxOutRegion) {
                        chart_all_text_region.Subtract(compassRect);
                    }
                }
                
                
                mscratch_dc.DestroyClippingRegion();
                
                m_pQuilt->RenderQuiltRegionViewOnDCTextOnly( mscratch_dc, svp, chart_all_text_region );
                
            }
        }
    }
    
    
//    And finally, blit the scratch dc onto the physical dc
    wxRegionIterator upd_final( rgn_blit );
    while( upd_final ) {
        wxRect rect = upd_final.GetRect();
        dc.Blit( rect.x, rect.y, rect.width, rect.height, &mscratch_dc, rect.x, rect.y );
        upd_final++;
    }

    //  Test code to validate the dc drawing rectangle....
/*
    wxRegionIterator upd_ru ( rgn_blit ); // get the update rect list
     while ( upd_ru )
     {
     wxRect rect = upd_ru.GetRect();

     dc.SetPen(wxPen(*wxRED));
     dc.SetBrush(wxBrush(*wxRED, wxTRANSPARENT));
     dc.DrawRectangle(rect);
     upd_ru ++ ;
     }
*/

    
//    Deselect the chart bitmap from the temp_dc, so that it will not be destroyed in the temp_dc dtor
    temp_dc.SelectObject( wxNullBitmap );
//    And for the scratch bitmap
    mscratch_dc.SelectObject( wxNullBitmap );

    dc.DestroyClippingRegion();

    
    if(m_muiBar){
        m_muiBar->Refresh();
    }
    
    PaintCleanup();

}

void ChartCanvas::PaintCleanup()
{
//    Handle the current graphic window, if present

    if( pCwin ) {
        pCwin->Show();
        if( m_bTCupdate ) {
            pCwin->Refresh();
            pCwin->Update();
        }
    }

//    And set flags for next time
    m_bTCupdate = false;

//    Handle deferred WarpPointer
    if( warp_flag ) {
        WarpPointer( warp_x, warp_y );
        warp_flag = false;
    }

    // Start movement timer, this runs nearly immediately.
    // the reason we cannot simply call it directly is the
    // refresh events it emits may be blocked from this paint event
    pMovementTimer->Start( 1, wxTIMER_ONE_SHOT ); 
}

#if 0
wxColour GetErrorGraphicColor(double val)
{
    /*
     double valm = wxMin(val_max, val);

     unsigned char green = (unsigned char)(255 * (1 - (valm/val_max)));
     unsigned char red   = (unsigned char)(255 * (valm/val_max));

     wxImage::HSVValue hv = wxImage::RGBtoHSV(wxImage::RGBValue(red, green, 0));

     hv.saturation = 1.0;
     hv.value = 1.0;

     wxImage::RGBValue rv = wxImage::HSVtoRGB(hv);
     return wxColour(rv.red, rv.green, rv.blue);
     */

    //    HTML colors taken from NOAA WW3 Web representation
    wxColour c;
    if((val > 0) && (val < 1)) c.Set(_T("#002ad9"));
    else if((val >= 1) && (val < 2)) c.Set(_T("#006ed9"));
    else if((val >= 2) && (val < 3)) c.Set(_T("#00b2d9"));
    else if((val >= 3) && (val < 4)) c.Set(_T("#00d4d4"));
    else if((val >= 4) && (val < 5)) c.Set(_T("#00d9a6"));
    else if((val >= 5) && (val < 7)) c.Set(_T("#00d900"));
    else if((val >= 7) && (val < 9)) c.Set(_T("#95d900"));
    else if((val >= 9) && (val < 12)) c.Set(_T("#d9d900"));
    else if((val >= 12) && (val < 15)) c.Set(_T("#d9ae00"));
    else if((val >= 15) && (val < 18)) c.Set(_T("#d98300"));
    else if((val >= 18) && (val < 21)) c.Set(_T("#d95700"));
    else if((val >= 21) && (val < 24)) c.Set(_T("#d90000"));
    else if((val >= 24) && (val < 27)) c.Set(_T("#ae0000"));
    else if((val >= 27) && (val < 30)) c.Set(_T("#8c0000"));
    else if((val >= 30) && (val < 36)) c.Set(_T("#870000"));
    else if((val >= 36) && (val < 42)) c.Set(_T("#690000"));
    else if((val >= 42) && (val < 48)) c.Set(_T("#550000"));
    else if( val >= 48) c.Set(_T("#410000"));

    return c;
}

void ChartCanvas::RenderGeorefErrorMap( wxMemoryDC *pmdc, ViewPort *vp)
{
    wxImage gr_image(vp->pix_width, vp->pix_height);
    gr_image.InitAlpha();

    double maxval = -10000;
    double minval = 10000;

    double rlat, rlon;
    double glat, glon;

    GetCanvasPixPoint(0, 0, rlat, rlon);

    for(int i=1; i < vp->pix_height-1; i++)
    {
        for(int j=0; j < vp->pix_width; j++)
        {
            // Reference mercator value
//                  vp->GetMercatorLLFromPix(wxPoint(j, i), &rlat, &rlon);

            // Georef value
            GetCanvasPixPoint(j, i, glat, glon);

            maxval = wxMax(maxval, (glat - rlat));
            minval = wxMin(minval, (glat - rlat));

        }
        rlat = glat;
    }

    GetCanvasPixPoint(0, 0, rlat, rlon);
    for(int i=1; i < vp->pix_height-1; i++)
    {
        for(int j=0; j < vp->pix_width; j++)
        {
            // Reference mercator value
//                  vp->GetMercatorLLFromPix(wxPoint(j, i), &rlat, &rlon);

            // Georef value
            GetCanvasPixPoint(j, i, glat, glon);

            double f = ((glat - rlat)-minval)/(maxval - minval);

            double dy = (f * 40);

            wxColour c = GetErrorGraphicColor(dy);
            unsigned char r = c.Red();
            unsigned char g = c.Green();
            unsigned char b = c.Blue();

            gr_image.SetRGB(j, i, r,g,b);
            if((glat - rlat )!= 0)
                gr_image.SetAlpha(j, i, 128);
            else
                gr_image.SetAlpha(j, i, 255);

        }
        rlat = glat;
    }

    //    Create a Bitmap
    wxBitmap *pbm = new wxBitmap(gr_image);
    wxMask *gr_mask = new wxMask(*pbm, wxColour(0,0,0));
    pbm->SetMask(gr_mask);

    pmdc->DrawBitmap(*pbm, 0,0);

    delete pbm;

}

#endif

void ChartCanvas::CancelMouseRoute()
{
    m_routeState = 0;
    m_pMouseRoute = NULL;
    m_bDrawingRoute = false;
}

int ChartCanvas::GetNextContextMenuId()
{
    return CanvasMenuHandler::GetNextContextMenuId();
}

bool ChartCanvas::SetCursor( const wxCursor &c )
{
#ifdef ocpnUSE_GL
    if( g_bopengl && m_glcc )
        return m_glcc->SetCursor( c );
    else
#endif
        return wxWindow::SetCursor( c );
}

void ChartCanvas::Refresh( bool eraseBackground, const wxRect *rect )
{
    if( g_bquiting )
        return;
    //  Keep the mouse position members up to date
    GetCanvasPixPoint( mouse_x, mouse_y, m_cursor_lat, m_cursor_lon );

    //      Retrigger the route leg popup timer
    //      This handles the case when the chart is moving in auto-follow mode, but no user mouse input is made.
    //      The timer handler may Hide() the popup if the chart moved enough
    //      n.b.  We use slightly longer oneshot value to allow this method's Refresh() to complete before
    //      potentially getting another Refresh() in the popup timer handler.
    if( !m_RolloverPopupTimer.IsRunning() &&
        ((m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive()) ||
	(m_pTrackRolloverWin && m_pTrackRolloverWin->IsActive()) ||
	(m_pAISRolloverWin && m_pAISRolloverWin->IsActive())) )
        m_RolloverPopupTimer.Start( 500, wxTIMER_ONE_SHOT );

#ifdef ocpnUSE_GL
    if( m_glcc && g_bopengl ) {
        
        //      We need to invalidate the FBO cache to ensure repaint of "grounded" overlay objects.
        if( eraseBackground && m_glcc->UsingFBO() )
            m_glcc->Invalidate();
        

        m_glcc->Refresh( eraseBackground, NULL ); // We always are going to render the entire screen anyway, so make
        // sure that the window managers understand the invalid area
        // is actually the entire client area.

        //  We need to selectively Refresh some child windows, if they are visible.
        //  Note that some children are refreshed elsewhere on timer ticks, so don't need attention here.

        //      Thumbnail chart
        if( pthumbwin && pthumbwin->IsShown() ) {
            pthumbwin->Raise();
            pthumbwin->Refresh( false );
        }

        //      ChartInfo window
        if( m_pCIWin && m_pCIWin->IsShown() ) {
            m_pCIWin->Raise();
            m_pCIWin->Refresh( false );
        }
        
//        if(g_MainToolbar)
//            g_MainToolbar->UpdateRecoveryWindow(g_bshowToolbar);
        
    } else
#endif
        wxWindow::Refresh( eraseBackground, rect );

}

void ChartCanvas::Update()
{
    if( m_glcc && g_bopengl ) {
#ifdef ocpnUSE_GL
        m_glcc->Update();
#endif
    } else
        wxWindow::Update();
}

void ChartCanvas::DrawEmboss( ocpnDC &dc, emboss_data *pemboss)
{
    if( !pemboss ) return;
    int x = pemboss->x, y = pemboss->y;
    const double factor = 200;

    wxASSERT_MSG( dc.GetDC(), wxT ( "DrawEmboss has no dc (opengl?)" ) );
    wxMemoryDC *pmdc = dynamic_cast<wxMemoryDC*>( dc.GetDC() );
    wxASSERT_MSG ( pmdc, wxT ( "dc to EmbossCanvas not a memory dc" ) );
    
    //Grab a snipped image out of the chart
    wxMemoryDC snip_dc;
    wxBitmap snip_bmp( pemboss->width, pemboss->height, -1 );
    snip_dc.SelectObject( snip_bmp );
    
    snip_dc.Blit( 0, 0, pemboss->width, pemboss->height, pmdc, x, y );
    snip_dc.SelectObject( wxNullBitmap );
    
    wxImage snip_img = snip_bmp.ConvertToImage();
    
    //  Apply Emboss map to the snip image
    unsigned char* pdata = snip_img.GetData();
    if( pdata ) {
        for( int y = 0; y < pemboss->height; y++ ) {
            int map_index = ( y * pemboss->width );
            for( int x = 0; x < pemboss->width; x++ ) {
                double val = ( pemboss->pmap[map_index] * factor ) / 256.;
                
                int nred = (int) ( ( *pdata ) + val );
                nred = nred > 255 ? 255 : ( nred < 0 ? 0 : nred );
                *pdata++ = (unsigned char) nred;
                
                int ngreen = (int) ( ( *pdata ) + val );
                ngreen = ngreen > 255 ? 255 : ( ngreen < 0 ? 0 : ngreen );
                *pdata++ = (unsigned char) ngreen;
                
                int nblue = (int) ( ( *pdata ) + val );
                nblue = nblue > 255 ? 255 : ( nblue < 0 ? 0 : nblue );
                *pdata++ = (unsigned char) nblue;
                
                map_index++;
            }
        }
    }
    
    //  Convert embossed snip to a bitmap
    wxBitmap emb_bmp( snip_img );

    //  Map to another memoryDC
    wxMemoryDC result_dc;
    result_dc.SelectObject( emb_bmp );
    
    //  Blit to target
    pmdc->Blit( x, y, pemboss->width, pemboss->height, &result_dc, 0, 0 );
    
    result_dc.SelectObject( wxNullBitmap );
}

emboss_data *ChartCanvas::EmbossOverzoomIndicator( ocpnDC &dc )
{
    double zoom_factor = GetVP().ref_scale / GetVP().chart_scale;
    
    if( GetQuiltMode() ) {
        if( zoom_factor <= 3.9 )
            return NULL;
    } else {
        if( m_singleChart ) {
            if( zoom_factor <= 3.9 )
                return NULL;
        }
        else
            return NULL;
    }

    if(m_pEM_OverZoom){
        m_pEM_OverZoom->x = 0;
        m_pEM_OverZoom->y = 40;
    }
    return m_pEM_OverZoom;
}

void ChartCanvas::DrawOverlayObjects( ocpnDC &dc, const wxRegion& ru )
{
    GridDraw( dc );

    bool pluginOverlayRender = true;
    
    if(g_canvasConfig > 0){     // Multi canvas
        if(IsPrimaryCanvas())
            pluginOverlayRender = false;
    }
    
    if(pluginOverlayRender){
        g_overlayCanvas = this;

        if( g_pi_manager ) {
            g_pi_manager->SendViewPortToRequestingPlugIns( GetVP() );
            g_pi_manager->RenderAllCanvasOverlayPlugIns( dc, GetVP() );
        }
    }

    AISDrawAreaNotices( dc, GetVP(), this);
    DrawEmboss( dc, EmbossDepthScale( ) );
    DrawEmboss( dc, EmbossOverzoomIndicator( dc ) );

    wxDC *pdc = dc.GetDC();
    if( pdc ) {
        pdc->DestroyClippingRegion();
        wxDCClipper( *pdc, ru );
    }

    if( m_bShowNavobjects ) {
        DrawAllTracksInBBox( dc, GetVP().GetBBox() );
        DrawAllRoutesInBBox( dc, GetVP().GetBBox() );
        DrawAllWaypointsInBBox( dc, GetVP().GetBBox() );
        DrawAnchorWatchPoints( dc );
    } else {
        DrawActiveTrackInBBox( dc, GetVP().GetBBox() );
        DrawActiveRouteInBBox( dc, GetVP().GetBBox() );
    }
    
    AISDraw( dc, GetVP(), this );
    ShipDraw( dc );
    AlertDraw( dc );

    RenderAllChartOutlines( dc, GetVP() );
    RenderRouteLegs( dc );
    ScaleBarDraw( dc );
#ifdef USE_S57
    s57_DrawExtendedLightSectors( dc, VPoint, extendedSectorLegs );
#endif

    if( m_pTrackRolloverWin ) {
        m_pTrackRolloverWin->Draw(dc);
        m_brepaint_piano = true;
    }

    if( m_pRouteRolloverWin ) {
        m_pRouteRolloverWin->Draw(dc);
        m_brepaint_piano = true;
    }

    if( m_pAISRolloverWin ) {
        m_pAISRolloverWin->Draw(dc);
        m_brepaint_piano = true;
    }
}

emboss_data *ChartCanvas::EmbossDepthScale()
{
    if( !m_bShowDepthUnits ) return NULL;

    int depth_unit_type = DEPTH_UNIT_UNKNOWN;

    if( GetQuiltMode() ) {
        wxString s = m_pQuilt->GetQuiltDepthUnit();
        s.MakeUpper();
        if( s == _T("FEET") ) depth_unit_type = DEPTH_UNIT_FEET;
        else if( s.StartsWith( _T("FATHOMS") ) ) depth_unit_type = DEPTH_UNIT_FATHOMS;
        else if( s.StartsWith( _T("METERS") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.StartsWith( _T("METRES") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.StartsWith( _T("METRIC") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.StartsWith( _T("METER") ) ) depth_unit_type = DEPTH_UNIT_METERS;

    } else {
        if( m_singleChart ) {
            depth_unit_type = m_singleChart->GetDepthUnitType();
#ifdef USE_S57
            if( m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR ) depth_unit_type =
                    ps52plib->m_nDepthUnitDisplay + 1;
#endif
        }
    }

    emboss_data *ped = NULL;
    switch( depth_unit_type ) {
    case DEPTH_UNIT_FEET:
        ped = m_pEM_Feet;
        break;
    case DEPTH_UNIT_METERS:
        ped = m_pEM_Meters;
        break;
    case DEPTH_UNIT_FATHOMS:
        ped = m_pEM_Fathoms;
        break;
    default:
        return NULL;
    }

    ped->x = ( GetVP().pix_width - ped->width );

    if(m_Compass && g_bShowCompassWin){
        wxRect r = m_Compass->GetRect();
        ped->y = r.y + r.height;
     }
     else{
        ped->y = 40;
    }
    return ped;
}

void ChartCanvas::CreateDepthUnitEmbossMaps( ColorScheme cs )
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    wxFont font;
    if( style->embossFont == wxEmptyString ){
        wxFont *dFont = FontMgr::Get().GetFont( _("Dialog"), 0 );
        font = *dFont;
        font.SetPointSize(60);
        font.SetWeight(wxFONTWEIGHT_BOLD);
    }
    else
        font = wxFont( style->embossHeight, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, style->embossFont );

    int emboss_width = 500;
    int emboss_height = 200;

// Free any existing emboss maps
    delete m_pEM_Feet;
    delete m_pEM_Meters;
    delete m_pEM_Fathoms;

// Create the 3 DepthUnit emboss map structures
    m_pEM_Feet = CreateEmbossMapData( font, emboss_width, emboss_height, _("Feet"), cs );
    m_pEM_Meters = CreateEmbossMapData( font, emboss_width, emboss_height, _("Meters"), cs );
    m_pEM_Fathoms = CreateEmbossMapData( font, emboss_width, emboss_height, _("Fathoms"), cs );
}

#define OVERZOOM_TEXT _("OverZoom")

void ChartCanvas::SetOverzoomFont()
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    int w, h;

    wxFont font;
    if( style->embossFont == wxEmptyString ){
        wxFont *dFont = FontMgr::Get().GetFont( _("Dialog"), 0 );
        font = *dFont;
        font.SetPointSize(40);
        font.SetWeight(wxFONTWEIGHT_BOLD);
    }
    else
        font = wxFont( style->embossHeight, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, style->embossFont );
    
    wxClientDC dc( this );
    dc.SetFont( font );
    dc.GetTextExtent( OVERZOOM_TEXT, &w, &h );

    while( font.GetPointSize() > 10 && (w > 500 || h > 100) )
    {
        font.SetPointSize( font.GetPointSize() - 1 );
        dc.SetFont( font );
        dc.GetTextExtent( OVERZOOM_TEXT, &w, &h );
    }
    m_overzoomFont = font;
    m_overzoomTextWidth = w;
    m_overzoomTextHeight = h;
}

void ChartCanvas::CreateOZEmbossMapData( ColorScheme cs )
{
    delete m_pEM_OverZoom;

    if( m_overzoomTextWidth > 0 && m_overzoomTextHeight > 0 )
        m_pEM_OverZoom = CreateEmbossMapData( m_overzoomFont, m_overzoomTextWidth + 10, m_overzoomTextHeight + 10, OVERZOOM_TEXT, cs );
}

emboss_data *ChartCanvas::CreateEmbossMapData( wxFont &font, int width, int height,
        const wxString &str, ColorScheme cs )
{
    int *pmap;

    //  Create a temporary bitmap
    wxBitmap bmp( width, height, -1 );

    // Create a memory DC
    wxMemoryDC temp_dc;
    temp_dc.SelectObject( bmp );

    //  Paint on it
    temp_dc.SetBackground( *wxWHITE_BRUSH );
    temp_dc.SetTextBackground( *wxWHITE );
    temp_dc.SetTextForeground( *wxBLACK );

    temp_dc.Clear();

    temp_dc.SetFont( font );

    int str_w, str_h;
    temp_dc.GetTextExtent( str, &str_w, &str_h );
//    temp_dc.DrawText( str, width - str_w - 10, 10 );
    temp_dc.DrawText( str, 1, 1 );
    
    //  Deselect the bitmap
    temp_dc.SelectObject( wxNullBitmap );

    //  Convert bitmap the wxImage for manipulation
    wxImage img = bmp.ConvertToImage();

    int image_width = str_w * 105 / 100;
    int image_height = str_h * 105 / 100;
    wxRect r(0,0, wxMin(image_width, img.GetWidth()), wxMin(image_height, img.GetHeight()));
    wxImage imgs = img.GetSubImage(r);
    
    double val_factor;
    switch( cs ) {
    case GLOBAL_COLOR_SCHEME_DAY:
    default:
        val_factor = 1;
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        val_factor = .5;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        val_factor = .25;
        break;
    }

    int val;
    int index;
    const int w = imgs.GetWidth();
    const int h = imgs.GetHeight();
    pmap = (int *) calloc( w *  h * sizeof(int), 1 );
    //  Create emboss map by differentiating the emboss image
    //  and storing integer results in pmap
    //  n.b. since the image is B/W, it is sufficient to check
    //  one channel (i.e. red) only
    for( int y = 1; y < h - 1; y++ ) {
        for( int x = 1; x < w - 1; x++ ) {
            val = img.GetRed( x + 1, y + 1 ) - img.GetRed( x - 1, y - 1 );  // range +/- 256
            val = (int) ( val * val_factor );
            index = ( y * w ) + x;
            pmap[index] = val;

        }
    }

    emboss_data *pret = new emboss_data;
    pret->pmap = pmap;
    pret->width = w;
    pret->height = h;

    return pret;
}


void ChartCanvas::DrawAllTracksInBBox( ocpnDC& dc, LLBBox& BltBBox )
{
    Track *active_track = NULL;
    for(wxTrackListNode *node = pTrackList->GetFirst();
        node; node = node->GetNext()) {
        Track *pTrackDraw = node->GetData();

        if( g_pActiveTrack == pTrackDraw ) {
            active_track = pTrackDraw;
            continue;
        }

        pTrackDraw->Draw( this, dc, GetVP(), BltBBox );
    }

    if( active_track )
        active_track->Draw( this, dc, GetVP(), BltBBox );
}


void ChartCanvas::DrawActiveTrackInBBox( ocpnDC& dc, LLBBox& BltBBox )
{
    Track *active_track = NULL;
    for(wxTrackListNode *node = pTrackList->GetFirst();
        node; node = node->GetNext()) {
        Track *pTrackDraw = node->GetData();
        
        if( g_pActiveTrack == pTrackDraw ) {
            active_track = pTrackDraw;
            break;
        }
    }
    if( active_track )
        active_track->Draw( this, dc, GetVP(), BltBBox );
}


void ChartCanvas::DrawAllRoutesInBBox( ocpnDC& dc, LLBBox& BltBBox )
{
    Route *active_route = NULL;

    for(wxRouteListNode *node = pRouteList->GetFirst();
        node; node = node->GetNext()) {

        Route *pRouteDraw = node->GetData();
        if( pRouteDraw->IsActive() || pRouteDraw->IsSelected() ) {
            active_route = pRouteDraw;
            continue;
        }

//        if(m_canvasIndex == 1)
        pRouteDraw->Draw( dc, this, BltBBox );
    }

    //  Draw any active or selected route (or track) last, so that is is always on top
    if( active_route )
        active_route->Draw( dc, this, BltBBox );
}

void ChartCanvas::DrawActiveRouteInBBox( ocpnDC& dc, LLBBox& BltBBox )
{
    Route *active_route = NULL;
    
    for(wxRouteListNode *node = pRouteList->GetFirst();
        node; node = node->GetNext()) {
        Route *pRouteDraw = node->GetData();
        if( pRouteDraw->IsActive() || pRouteDraw->IsSelected() ) {
            active_route = pRouteDraw;
            break;
        }
        
    }
    if( active_route )
        active_route->Draw( dc, this, BltBBox );
}

void ChartCanvas::DrawAllWaypointsInBBox( ocpnDC& dc, LLBBox& BltBBox )
{
    if(!pWayPointMan)
        return;

    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

    while( node ) {
        RoutePoint *pWP = node->GetData();
        if( pWP ) {
            if( pWP->m_bIsInRoute ) {
                node = node->GetNext();
                continue;
            }

            /* technically incorrect... waypoint has bounding box */
            if( BltBBox.Contains( pWP->m_lat, pWP->m_lon ) )
                pWP->Draw( dc, this, NULL );
            else{
                // Are Range Rings enabled?
                if(pWP->GetShowWaypointRangeRings() && (pWP->GetWaypointRangeRingsNumber() > 0)){
                    double factor = 1.00;
                    if( pWP->GetWaypointRangeRingsStepUnits() == 1 )          // convert kilometers to NMi
                        factor = 1 / 1.852;
                    
                    double radius = factor * pWP->GetWaypointRangeRingsNumber() * pWP->GetWaypointRangeRingsStep()  / 60.;
                    radius *= 2;                // Fudge factor
                    
                    LLBBox radar_box;
                    radar_box.Set(pWP->m_lat-radius, pWP->m_lon-radius, pWP->m_lat+radius, pWP->m_lon+radius);
                    if( !BltBBox.IntersectOut( radar_box ) ){
                        pWP->Draw( dc, this, NULL );
                    }
                }
            }
        }

        node = node->GetNext();
    }
}

void ChartCanvas::DrawBlinkObjects( void )
{
    //  All RoutePoints
    wxRect update_rect;

    if(!pWayPointMan)
        return;

    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
    
    while( node ) {
        RoutePoint *pWP = node->GetData();
        if( pWP ) {
            if( pWP->m_bBlink ) {
                update_rect.Union( pWP->CurrentRect_in_DC ) ;
            }
        }
        
        node = node->GetNext();
    }
    if( !update_rect.IsEmpty() )
        RefreshRect(update_rect);
}




void ChartCanvas::DrawAnchorWatchPoints( ocpnDC& dc )
{
    // draw anchor watch rings, if activated

    if( pAnchorWatchPoint1 || pAnchorWatchPoint2 ) {
        wxPoint r1, r2;
        wxPoint lAnchorPoint1, lAnchorPoint2;
        double lpp1 = 0.0;
        double lpp2 = 0.0;
        if( pAnchorWatchPoint1 ) {
            lpp1 = GetAnchorWatchRadiusPixels( pAnchorWatchPoint1 );
            GetCanvasPointPix( pAnchorWatchPoint1->m_lat, pAnchorWatchPoint1->m_lon,
                               &lAnchorPoint1 );

        }
        if( pAnchorWatchPoint2 ) {
            lpp2 = GetAnchorWatchRadiusPixels( pAnchorWatchPoint2 );
            GetCanvasPointPix( pAnchorWatchPoint2->m_lat, pAnchorWatchPoint2->m_lon,
                               &lAnchorPoint2 );

        }

        wxPen ppPeng( GetGlobalColor( _T ( "UGREN" ) ), 2 );
        wxPen ppPenr( GetGlobalColor( _T ( "URED" ) ), 2 );

        wxBrush *ppBrush = wxTheBrushList->FindOrCreateBrush( wxColour( 0, 0, 0 ), wxBRUSHSTYLE_TRANSPARENT );
        dc.SetBrush( *ppBrush );

        if( lpp1 > 0 ) {
            dc.SetPen( ppPeng );
            dc.StrokeCircle( lAnchorPoint1.x, lAnchorPoint1.y, fabs( lpp1 ) );
        }

        if( lpp2 > 0 ) {
            dc.SetPen( ppPeng );
            dc.StrokeCircle( lAnchorPoint2.x, lAnchorPoint2.y, fabs( lpp2 ) );
        }

        if( lpp1 < 0 ) {
            dc.SetPen( ppPenr );
            dc.StrokeCircle( lAnchorPoint1.x, lAnchorPoint1.y, fabs( lpp1 ) );
        }

        if( lpp2 < 0 ) {
            dc.SetPen( ppPenr );
            dc.StrokeCircle( lAnchorPoint2.x, lAnchorPoint2.y, fabs( lpp2 ) );
        }
    }
}

double ChartCanvas::GetAnchorWatchRadiusPixels( RoutePoint *pAnchorWatchPoint )
{
    double lpp = 0.;
    wxPoint r1;
    wxPoint lAnchorPoint;
    double d1 = 0.0;
    double dabs;
    double tlat1, tlon1;

    if( pAnchorWatchPoint ) {
        ( pAnchorWatchPoint->GetName() ).ToDouble( &d1 );
        d1 = AnchorDistFix( d1, AnchorPointMinDist, g_nAWMax );
        dabs = fabs( d1 / 1852. );
        ll_gc_ll( pAnchorWatchPoint->m_lat, pAnchorWatchPoint->m_lon, 0, dabs, &tlat1, &tlon1 );
        GetCanvasPointPix( tlat1, tlon1, &r1 );
        GetCanvasPointPix( pAnchorWatchPoint->m_lat, pAnchorWatchPoint->m_lon, &lAnchorPoint );
        lpp = sqrt( pow( (double) (lAnchorPoint.x - r1.x), 2) +
                    pow( (double) (lAnchorPoint.y - r1.y), 2) );

        //    This is an entry watch
        if( d1 < 0 ) lpp = -lpp;
    }
    return lpp;
}

//------------------------------------------------------------------------------------------
//    Tides Support
//------------------------------------------------------------------------------------------
void ChartCanvas::RebuildTideSelectList( LLBBox& BBox )
{
    if(!ptcmgr)
        return;
    
    pSelectTC->DeleteAllSelectableTypePoints( SELTYPE_TIDEPOINT );
    
    for( int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++ ) {
        const IDX_entry *pIDX = ptcmgr->GetIDX_entry( i );
        double lon = pIDX->IDX_lon;
        double lat = pIDX->IDX_lat;
        
        char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
        if( ( type == 't' ) || ( type == 'T' ) ) {
            
            if( BBox.Contains( lat, lon ) ) {
                
                //    Manage the point selection list
                pSelectTC->AddSelectablePoint( lat, lon, pIDX, SELTYPE_TIDEPOINT );
            }
        }
    }
}

extern wxDateTime gTimeSource;

void ChartCanvas::DrawAllTidesInBBox( ocpnDC& dc, LLBBox& BBox )
{
    if(!ptcmgr)
        return;
    
    wxDateTime this_now = gTimeSource;
    bool cur_time = !gTimeSource.IsValid();
    if (cur_time)
        this_now = wxDateTime::Now();
    time_t t_this_now = this_now.GetTicks();

    wxPen *pblack_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UINFD" ) ), 1,
                        wxPENSTYLE_SOLID );
    wxPen *pyelo_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( cur_time?_T ( "YELO1" ): _T ( "YELO2" )), 1,
                       wxPENSTYLE_SOLID );
    wxPen *pblue_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( cur_time?_T ( "BLUE2" ):_T ( "BLUE3" ) ), 1,
                       wxPENSTYLE_SOLID );

    wxBrush *pgreen_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "GREEN1" ) ),
                            wxBRUSHSTYLE_SOLID );
//        wxBrush *pblack_brush = wxTheBrushList->FindOrCreateBrush ( GetGlobalColor ( _T ( "UINFD" ) ), wxSOLID );
    wxBrush *pblue_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor(cur_time? _T ( "BLUE2" ):_T ( "BLUE3" ) ), wxBRUSHSTYLE_SOLID );
    wxBrush *pyelo_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor(cur_time? _T ( "YELO1" ): _T ( "YELO2" ) ), wxBRUSHSTYLE_SOLID );

    wxFont *dFont = FontMgr::Get().GetFont( _("ExtendedTideIcon") );
    dc.SetTextForeground( FontMgr::Get().GetFontColor( _("ExtendedTideIcon") ) );
    int font_size = wxMax(8, dFont->GetPointSize());
    wxFont *plabelFont = FontMgr::Get().FindOrCreateFont( font_size, dFont->GetFamily(),
                         dFont->GetStyle(), dFont->GetWeight() );

    dc.SetPen( *pblack_pen );
    dc.SetBrush( *pgreen_brush );

    wxBitmap bm;
    switch( m_cs ) {
    case GLOBAL_COLOR_SCHEME_DAY:
        bm = m_bmTideDay;
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        bm = m_bmTideDusk;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        bm = m_bmTideNight;
        break;
    default:
        bm = m_bmTideDay;
        break;
    }

    int bmw = bm.GetWidth();
    int bmh = bm.GetHeight();


    {

        double lon_last = 0.;
        double lat_last = 0.;
        double marge = 0.05;
        for( int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++ ) {
            const IDX_entry *pIDX = ptcmgr->GetIDX_entry( i );

            char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
            if( ( type == 't' ) || ( type == 'T' ) )  // only Tides
            {
                double lon = pIDX->IDX_lon;
                double lat = pIDX->IDX_lat;

                if( BBox.ContainsMarge( lat, lon, marge ) &&
//try to eliminate double entry , but the only good way is to clean the file!
                    ( lat != lat_last ) && ( lon != lon_last ) ) {

                    wxPoint r;
                    GetCanvasPointPix( lat, lon, &r );
//draw standard icons
                    if( GetVP().chart_scale > 500000 ) {
                        dc.DrawBitmap( bm, r.x - bmw / 2, r.y - bmh / 2, true );
                    }
//draw "extended" icons
                    else {
//set rectangle size and position (max text lengh)
                        int wx, hx;
                        dc.SetFont( *plabelFont );
                        dc.GetTextExtent( _T("99.9ft "), &wx, &hx );
                        int w = r.x - 6;
                        int h = r.y - 22;
                        {
                            {
                                float val, nowlev;
                                float ltleve = 0.;
                                float htleve = 0.;
                                time_t tctime;
                                time_t lttime = 0;
                                time_t httime = 0;
                                bool wt;
                                //define if flood or ebb in the last ten minutes and verify if data are useable
                                if( ptcmgr->GetTideFlowSens( t_this_now, BACKWARD_TEN_MINUTES_STEP,
                                                             pIDX->IDX_rec_num, nowlev, val, wt ) ) {

                                    //search forward the first HW or LW near "now" ( starting at "now" - ten minutes )
                                    ptcmgr->GetHightOrLowTide(
                                        t_this_now + BACKWARD_TEN_MINUTES_STEP,
                                        FORWARD_TEN_MINUTES_STEP, FORWARD_ONE_MINUTES_STEP, val,
                                        wt, pIDX->IDX_rec_num, val, tctime );
                                    if( wt ) {
                                        httime = tctime;
                                        htleve = val;
                                    } else {
                                        lttime = tctime;
                                        ltleve = val;
                                    }
                                    wt = !wt;

                                    //then search opposite tide near "now"
                                    if( tctime > t_this_now )          // search backward
                                        ptcmgr->GetHightOrLowTide( t_this_now,
                                                                   BACKWARD_TEN_MINUTES_STEP, BACKWARD_ONE_MINUTES_STEP,
                                                                   nowlev, wt, pIDX->IDX_rec_num, val, tctime );
                                    else
                                        // or search forward
                                        ptcmgr->GetHightOrLowTide( t_this_now,
                                                                   FORWARD_TEN_MINUTES_STEP, FORWARD_ONE_MINUTES_STEP,
                                                                   nowlev, wt, pIDX->IDX_rec_num, val, tctime );
                                    if( wt ) {
                                        httime = tctime;
                                        htleve = val;
                                    } else {
                                        lttime = tctime;
                                        ltleve = val;
                                    }
                
				    // draw the tide rectangle:
				    
				    // tide icon rectangle has default pre-scaled width = 12 , height = 45
				    // scale default size by tide_draw_scaler factor
				    int width = (int) (12 * tide_draw_scaler + 0.5);
				    int height = (int) (45 * tide_draw_scaler + 0.5);
				    int linew = wxMax(1, (int) (tide_draw_scaler));

				    //std::cerr << "tide icon w=" << width << ", h=" << height;
				    //std::cerr << ", linew=" << linew << ", scale=" << tide_draw_scaler << std::endl;

                                    //process tide state  ( %height and flow sens )
                                    float ts = 1 - ( ( nowlev - ltleve ) / ( htleve - ltleve ) );
                                    int hs = ( httime > lttime ) ? -4 : 4;
				    hs *= (int) (tide_draw_scaler + 0.5); 
                                    if( ts > 0.995 || ts < 0.005 ) hs = 0;
                                    int ht_y = (int) ( height * ts );

				    //draw yellow tide rectangle outlined in black
				    pblack_pen->SetWidth(linew);
                                    dc.SetPen( *pblack_pen );
                                    dc.SetBrush( *pyelo_brush );
                                    dc.DrawRectangle( w, h, width, height );

                                    //draw blue rectangle as water height, smaller in width than yellow rectangle
                                    dc.SetPen( *pblue_pen );
                                    dc.SetBrush( *pblue_brush );
                                    dc.DrawRectangle( (w + 2*linew), h + ht_y, 
						      (width - (4*linew)), height - ht_y );

                                    //draw sens arrows (ensure they are not "under-drawn" by top line of blue rectangle )
                                    int hl;
                                    wxPoint arrow[3];
                                    arrow[0].x = w + 2*linew;
                                    arrow[1].x = w + width / 2;
                                    arrow[2].x = w + width - 2*linew;
				    pyelo_pen->SetWidth(linew);
				    pblue_pen->SetWidth(linew);
                                    if( ts > 0.35 || ts < 0.15 )      // one arrow at 3/4 hight tide
                                    {
                                        hl = (int) ( height * 0.25 ) + h;
                                        arrow[0].y = hl;
                                        arrow[1].y = hl + hs;
                                        arrow[2].y = hl;
                                        if( ts < 0.15 ) 
					  dc.SetPen( *pyelo_pen );
                                        else
					  dc.SetPen( *pblue_pen );
                                        dc.DrawLines( 3, arrow );
                                    }
                                    if( ts > 0.60 || ts < 0.40 )       //one arrow at 1/2 hight tide
                                    {
                                        hl = (int) ( height * 0.5 ) + h;
                                        arrow[0].y = hl;
                                        arrow[1].y = hl + hs;
                                        arrow[2].y = hl;
                                        if( ts < 0.40 ) 
					  dc.SetPen( *pyelo_pen );
                                        else
					  dc.SetPen( *pblue_pen );
                                        dc.DrawLines( 3, arrow );
                                    }
                                    if( ts < 0.65 || ts > 0.85 )       //one arrow at 1/4 Hight tide
                                    {
                                        hl = (int) ( height * 0.75 ) + h;
                                        arrow[0].y = hl;
                                        arrow[1].y = hl + hs;
                                        arrow[2].y = hl;
                                        if( ts < 0.65 ) 
					  dc.SetPen( *pyelo_pen );
                                        else
					  dc.SetPen( *pblue_pen );
                                        dc.DrawLines( 3, arrow );
                                    }
                                    //draw tide level text
                                    wxString s;
                                    s.Printf( _T("%3.1f"), nowlev );
                                    Station_Data *pmsd = pIDX->pref_sta_data;           //write unit
                                    if( pmsd ) s.Append(
                                            wxString( pmsd->units_abbrv, wxConvUTF8 ) );
                                    int wx1;
                                    dc.GetTextExtent( s, &wx1, NULL );
                                    dc.DrawText( s, r.x - ( wx1 / 2 ), h + height );
                                }
                            }
                        }
                    }
                }
                lon_last = lon;
                lat_last = lat;
            }
        }
    }
}

//------------------------------------------------------------------------------------------
//    Currents Support
//------------------------------------------------------------------------------------------

void ChartCanvas::RebuildCurrentSelectList( LLBBox& BBox )
{
    if(!ptcmgr)
        return;
    
    pSelectTC->DeleteAllSelectableTypePoints( SELTYPE_CURRENTPOINT );
    
    double lon_last = 0.;
    double lat_last = 0.;
    for( int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++ ) {
        const IDX_entry *pIDX = ptcmgr->GetIDX_entry( i );
        double lon = pIDX->IDX_lon;
        double lat = pIDX->IDX_lat;
        
        char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
        if( ( ( type == 'c' ) || ( type == 'C' ) ) && ( 1/*pIDX->IDX_Useable*/) ) {
            
            //  TODO This is a ---HACK---
            //  try to avoid double current arrows.  Select the first in the list only
            //  Proper fix is to correct the TCDATA index file for depth indication
            bool b_dup = false;
            if( ( type == 'c' ) && ( lat == lat_last ) && ( lon == lon_last ) )
                b_dup = true;
               
            if( !b_dup && ( BBox.Contains( lat, lon ) ) ) {
                   
                   //    Manage the point selection list
                   pSelectTC->AddSelectablePoint( lat, lon, pIDX, SELTYPE_CURRENTPOINT );
            }
        }
        lon_last = lon;
        lat_last = lat;
    }
}
                   

void ChartCanvas::DrawAllCurrentsInBBox( ocpnDC& dc, LLBBox& BBox )
{
    if(!ptcmgr)
        return;
    
    float tcvalue, dir;
    bool bnew_val;
    char sbuf[20];
    wxFont *pTCFont;
    double lon_last = 0.;
    double lat_last = 0.;
    // arrow size for Raz Blanchard : 12 knots north
    double marge = 0.2;
    bool cur_time = !gTimeSource.IsValid();

    double true_scale_display = floor( VPoint.chart_scale / 100. ) * 100.;
    bDrawCurrentValues =  true_scale_display < g_Show_Target_Name_Scale;

    wxPen *pblack_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UINFD" ) ), 1,
                        wxPENSTYLE_SOLID );
    wxPen *porange_pen = wxThePenList->FindOrCreatePen( GetGlobalColor(cur_time? _T ( "UINFO" ):_T ( "UINFB" ) ), 1,
                         wxPENSTYLE_SOLID );
    wxBrush *porange_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor(cur_time? _T ( "UINFO" ): _T ( "UINFB" ) ),
                             wxBRUSHSTYLE_SOLID );
    wxBrush *pgray_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "UIBDR" ) ),
                           wxBRUSHSTYLE_SOLID );
    wxBrush *pblack_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "UINFD" ) ),
                            wxBRUSHSTYLE_SOLID );

    double skew_angle = GetVPRotation();

    pTCFont = FontMgr::Get().GetFont( _("CurrentValue") );
    
    {

        for( int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++ ) {
            const IDX_entry *pIDX = ptcmgr->GetIDX_entry( i );
            double lon = pIDX->IDX_lon;
            double lat = pIDX->IDX_lat;

            char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
            if( ( ( type == 'c' ) || ( type == 'C' ) ) && ( 1/*pIDX->IDX_Useable*/) ) {

//  TODO This is a ---HACK---
//  try to avoid double current arrows.  Select the first in the list only
//  Proper fix is to correct the TCDATA index file for depth indication
                bool b_dup = false;
                if( ( type == 'c' ) && ( lat == lat_last ) && ( lon == lon_last ) ) b_dup = true;

                if( !b_dup && ( BBox.ContainsMarge( lat, lon, marge ) ) ) {

                    wxPoint r;
                    GetCanvasPointPix( lat, lon, &r );

                    wxPoint d[4]; // points of a diamond at the current station location
                    int dd = (int) (5.0 * current_draw_scaler + 0.5);  
                    d[0].x = r.x;
                    d[0].y = r.y + dd;
                    d[1].x = r.x + dd;
                    d[1].y = r.y;
                    d[2].x = r.x;
                    d[2].y = r.y - dd;
                    d[3].x = r.x - dd;
                    d[3].y = r.y;

                    
                    if( 1 ) {
		        pblack_pen->SetWidth( wxMax(1, (int) (current_draw_scaler + 0.5)) );
                        dc.SetPen( *pblack_pen );
                        dc.SetBrush( *porange_brush );
                        dc.DrawPolygon( 4, d );

                        if( type == 'C' ) {
                            dc.SetBrush( *pblack_brush );
                            dc.DrawCircle( r.x, r.y, (int)(2*current_draw_scaler) ); 
                        }

                        if( GetVP().chart_scale < 1000000 ){
                            if(!ptcmgr->GetTideOrCurrent15( 0 , i, tcvalue, dir, bnew_val ))
                                continue;
                        }
                        else
                            continue;
                        
                        if( ( type == 'c' ) )
                        {
                            {

//    Get the display pixel location of the current station
                                int pixxc, pixyc;
                                wxPoint cpoint;
                                GetCanvasPointPix( lat, lon, &cpoint ); // called above; why do this again?
                                pixxc = cpoint.x;
                                pixyc = cpoint.y;

//    Adjust drawing size using logarithmic scale. tcvalue is current in knots
                                double a1 = fabs( tcvalue ) * 10.;
				// Current values <= 0.1 knot will have no arrow
                                a1 = wxMax(1.0, a1);      
                                double a2 = log10( a1 );

				// scale to pass to DrawArrow. 0.4 is a reasonable factor here.
				// users can adjust value of CurrentArrowScale in config file as needed
                                float cscale = current_draw_scaler * a2 * 0.4;

                                porange_pen->SetWidth( wxMax(1, (int) (current_draw_scaler + 0.5)) );
                                dc.SetPen( *porange_pen );
                                DrawArrow( dc, pixxc, pixyc,
                                           dir - 90 + ( skew_angle * 180. / PI ), cscale );
// Draw text, if enabled

                                if( bDrawCurrentValues ) {
                                    dc.SetFont( *pTCFont );
                                    snprintf( sbuf, 19, "%3.1f", fabs( tcvalue ) );
                                    dc.DrawText( wxString( sbuf, wxConvUTF8 ), pixxc, pixyc );
                                }
                            }
                        }           // scale
                    }
                    /*          This is useful for debugging the TC database
                     else
                     {
                     dc.SetPen ( *porange_pen );
                     dc.SetBrush ( *pgray_brush );
                     dc.DrawPolygon ( 4, d );
                     }
                     */

                }
                lon_last = lon;
                lat_last = lat;

            }
        }
    }
}

void ChartCanvas::DrawTCWindow( int x, int y, void *pvIDX )
{
    pCwin = new TCWin( this, x, y, pvIDX );
}

#define NUM_CURRENT_ARROW_POINTS 9
static wxPoint CurrentArrowArray[NUM_CURRENT_ARROW_POINTS] = { wxPoint( 0, 0 ), wxPoint( 0, -10 ),
        wxPoint( 55, -10 ), wxPoint( 55, -25 ), wxPoint( 100, 0 ), wxPoint( 55, 25 ), wxPoint( 55,
                10 ), wxPoint( 0, 10 ), wxPoint( 0, 0 )
                                                             };

void ChartCanvas::DrawArrow( ocpnDC& dc, int x, int y, double rot_angle, double scale )
{
    if( scale > 1e-2 ) {

        float sin_rot = sin( rot_angle * PI / 180. );
        float cos_rot = cos( rot_angle * PI / 180. );

        // Move to the first point

        float xt = CurrentArrowArray[0].x;
        float yt = CurrentArrowArray[0].y;

        float xp = ( xt * cos_rot ) - ( yt * sin_rot );
        float yp = ( xt * sin_rot ) + ( yt * cos_rot );
        int x1 = (int) ( xp * scale );
        int y1 = (int) ( yp * scale );

        // Walk thru the point list
        for( int ip = 1; ip < NUM_CURRENT_ARROW_POINTS; ip++ ) {
            xt = CurrentArrowArray[ip].x;
            yt = CurrentArrowArray[ip].y;

            float xp = ( xt * cos_rot ) - ( yt * sin_rot );
            float yp = ( xt * sin_rot ) + ( yt * cos_rot );
            int x2 = (int) ( xp * scale );
            int y2 = (int) ( yp * scale );

            dc.DrawLine( x1 + x, y1 + y, x2 + x, y2 + y );

            x1 = x2;
            y1 = y2;
        }
    }
}

wxString ChartCanvas::FindValidUploadPort()
{
    wxString port;
    //  Try to use the saved persistent upload port first
    if( !g_uploadConnection.IsEmpty() &&  g_uploadConnection.StartsWith(_T("Serial") ) ) {
            port = g_uploadConnection;
    }

    else if( g_pConnectionParams ) {
    // If there is no persistent upload port recorded (yet)
            // then use the first available serial connection which has output defined.
            for( size_t i = 0; i < g_pConnectionParams->Count(); i++ ) {
                ConnectionParams *cp = g_pConnectionParams->Item( i );
                if( (cp->IOSelect != DS_TYPE_INPUT) && cp->Type == SERIAL )
                    port << _T("Serial:") << cp->Port;
            }
    }

    return port;
}

void ShowAISTargetQueryDialog( wxWindow *win, int mmsi )
{
    if( !win ) return;

    if( NULL == g_pais_query_dialog_active ) {
        int pos_x = g_ais_query_dialog_x;
        int pos_y = g_ais_query_dialog_y;

        if( g_pais_query_dialog_active ) {
            delete g_pais_query_dialog_active;
            g_pais_query_dialog_active = new AISTargetQueryDialog();
        }
        else {
            g_pais_query_dialog_active = new AISTargetQueryDialog();
        }

        g_pais_query_dialog_active->Create( win, -1, _( "AIS Target Query" ),
                                            wxPoint( pos_x, pos_y ) );

        g_pais_query_dialog_active->SetAutoCentre( g_btouch );
        g_pais_query_dialog_active->SetAutoSize( g_bresponsive );
        g_pais_query_dialog_active->SetMMSI( mmsi );
        g_pais_query_dialog_active->UpdateText();
        wxSize sz = g_pais_query_dialog_active->GetSize();

        bool b_reset_pos = false;
#ifdef __WXMSW__
        //  Support MultiMonitor setups which an allow negative window positions.
        //  If the requested window title bar does not intersect any installed monitor,
        //  then default to simple primary monitor positioning.
        RECT frame_title_rect;
        frame_title_rect.left = pos_x;
        frame_title_rect.top = pos_y;
        frame_title_rect.right = pos_x + sz.x;
        frame_title_rect.bottom = pos_y + 30;

        if( NULL == MonitorFromRect( &frame_title_rect, MONITOR_DEFAULTTONULL ) ) b_reset_pos =
                true;
#else

        //    Make sure drag bar (title bar) of window intersects wxClient Area of screen, with a little slop...
        wxRect window_title_rect;// conservative estimate
        window_title_rect.x = pos_x;
        window_title_rect.y = pos_y;
        window_title_rect.width = sz.x;
        window_title_rect.height = 30;

        wxRect ClientRect = wxGetClientDisplayRect();
        ClientRect.Deflate(60, 60);// Prevent the new window from being too close to the edge
        if(!ClientRect.Intersects(window_title_rect))
            b_reset_pos = true;

#endif

        if( b_reset_pos )
            g_pais_query_dialog_active->Move( 50, 200 );
        

    } else {
        g_pais_query_dialog_active->SetMMSI( mmsi );
        g_pais_query_dialog_active->UpdateText();
    }

    g_pais_query_dialog_active->Show();
}

void ChartCanvas::ToggleCanvasQuiltMode( void )
{
        bool cur_mode = GetQuiltMode();
        
        if( !GetQuiltMode() )
            SetQuiltMode( true );
        else
            if( GetQuiltMode() ) {
                SetQuiltMode( false );
                g_sticky_chart = GetQuiltReferenceChartIndex();
            }
            
            
        if( cur_mode != GetQuiltMode() ){
            SetupCanvasQuiltMode();
            DoCanvasUpdate();
            InvalidateGL();
            Refresh();
        }
            //  TODO What to do about this?
            //g_bQuiltEnable = GetQuiltMode();
            
#ifdef USE_S57
            // Recycle the S52 PLIB so that vector charts will flush caches and re-render
        if(ps52plib)
            ps52plib->GenerateStateHash();
#endif

        if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
            GetMUIBar()->GetCanvasOptions()->RefreshControlValues();
}

void ChartCanvas::DoCanvasStackDelta( int direction )
{
    if( !GetQuiltMode() ) {
        int current_stack_index = GetpCurrentStack()->CurrentStackEntry;
        if( (current_stack_index + direction) >= GetpCurrentStack()->nEntry )
            return;
        if( (current_stack_index + direction) < 0 )
            return;
        
        if( m_bpersistent_quilt /*&& g_bQuiltEnable*/ ) {
            int new_dbIndex = GetpCurrentStack()->GetDBIndex(current_stack_index + direction );
            
            if( IsChartQuiltableRef( new_dbIndex ) ) {
                ToggleCanvasQuiltMode();
                SelectQuiltRefdbChart( new_dbIndex );
                m_bpersistent_quilt = false;
            }
        }
        else {
            SelectChartFromStack( current_stack_index + direction );
        }
    } else {
        std::vector<int>  piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
        int refdb = GetQuiltRefChartdbIndex();
        
        //      Find the ref chart in the stack
        int current_index = -1;
        for(unsigned int i=0 ; i < piano_chart_index_array.size() ; i++){
            if(refdb == piano_chart_index_array[i]){
                current_index = i;
                break;
            }
        }
        if(current_index == -1)
            return;
        
        const ChartTableEntry &ctet = ChartData->GetChartTableEntry( refdb );
        int target_family= ctet.GetChartFamily();
        
        int new_index = -1;
        int check_index = current_index + direction;
        bool found = false;
        int check_dbIndex = -1;
        int new_dbIndex = -1;
        
        //      When quilted. switch within the same chart family
        while(!found && (unsigned int)check_index < piano_chart_index_array.size() && (check_index >= 0)){
            check_dbIndex = piano_chart_index_array[check_index];
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( check_dbIndex );
            if(target_family == cte.GetChartFamily()){
                found = true;
                new_index = check_index;
                new_dbIndex = check_dbIndex;
                break;
            }
            
            check_index += direction;
        }
        
        if(!found)
            return;
        
        
        if( !IsChartQuiltableRef( new_dbIndex ) ) {
            ToggleCanvasQuiltMode();
            SelectdbChart( new_dbIndex );
            m_bpersistent_quilt = true;
        } else {
            SelectQuiltRefChart( new_index );
        }
    }
    
    gFrame->UpdateGlobalMenuItems(); // update the state of the menu items (checkmarks etc)
    SetQuiltChartHiLiteIndex( -1 );
    
    ReloadVP();
}


//--------------------------------------------------------------------------------------------------------
//
//      Toolbar support
//
//--------------------------------------------------------------------------------------------------------

void ChartCanvas::OnToolLeftClick( wxCommandEvent& event )
{
    //  Handle the per-canvas toolbar clicks here
    
    switch( event.GetId() ){
        
        case ID_ZOOMIN: {
            ZoomCanvas( 2.0, false );
            break;
        }
        
        case ID_ZOOMOUT: {
            ZoomCanvas( 0.5, false );
            break;
        }
        
        case ID_STKUP:
            DoCanvasStackDelta( 1 );
            DoCanvasUpdate();
            break;
            
        case ID_STKDN:
            DoCanvasStackDelta( -1 );
            DoCanvasUpdate();
            break;
            
        case ID_FOLLOW: {
            TogglebFollow();
            break;
        }
        
        case ID_CURRENT: {
            ShowCurrents( !GetbShowCurrent() );
            ReloadVP();
            Refresh( false );
            break;
            
        }
        
        case ID_TIDE: {
            ShowTides( !GetbShowTide() );
            ReloadVP();
            Refresh( false );
            break;
            
        }
        
        case ID_ROUTE: {
            if( 0 == m_routeState ){
                StartRoute();
            }
            else {
                FinishRoute();
            }
            
#ifdef __OCPN__ANDROID__
            androidSetRouteAnnunciator(m_routeState == 1);
#endif
            break;
        }
        
        case ID_AIS: {
            SetAISCanvasDisplayStyle(-1);
            break;
        }
        
        case ID_TBSTATBOX: {
            ToggleCourseUp();
            break;
        }
        
        default:
            break;
    }
     
    //  And then let  gFrame handle the rest....
    event.Skip();
}
    

void ChartCanvas::SetToolbarPosition( wxPoint position )
{
    m_toolbarPosition = position;
}

void ChartCanvas::SetToolbarOrientation( long orient )
{
    m_toolbarOrientation = orient;
}

wxPoint ChartCanvas::GetToolbarPosition()
{
    if(m_toolBar){
        wxPoint tbp = m_toolBar->GetPosition();  //toolbar is a TLW, so this is screen coordinates.
        return ScreenToClient( tbp );
    }
    else
        return wxPoint(0,0);
}

long ChartCanvas::GetToolbarOrientation()
{
    if(m_toolBar)
        return m_toolBar->GetOrient();
    else
        return 0;
}

void ChartCanvas::SubmergeToolbar( void )
{
    if( m_toolBar )
        m_toolBar->Submerge();
}

void ChartCanvas::SurfaceToolbar( void )
{
    if( m_toolBar )
        m_toolBar->Surface();
}

bool ChartCanvas::IsToolbarShown()
{
    bool rv = false;
    if(m_toolBar)
        rv = m_toolBar->IsShown();
    return rv;
}

void ChartCanvas::ToggleToolbar( bool b_smooth )
{
    if( m_toolBar ) {
        if( m_toolBar->IsShown() ){
            SubmergeToolbar();
        }
        else{
            SurfaceToolbar();
            m_toolBar->Raise();
        }
    }
}

void ChartCanvas::DestroyToolbar()
{
    if( m_toolBar )
        m_toolBar->DestroyToolBar();
}
        

ocpnFloatingToolbarDialog *ChartCanvas::RequestNewCanvasToolbar(bool bforcenew)
{
    bool toolbar_scale_tools_shown = m_pCurrentStack && m_pCurrentStack->b_valid && ( m_pCurrentStack->nEntry > 1 );
    
    bool b_reshow = true;
    if( m_toolBar ) {
        b_reshow = m_toolBar->IsShown();
        float ff = fabs(m_toolBar->GetScaleFactor() - m_toolbar_scalefactor);
        if((ff > 0.01f) || bforcenew){
            m_toolBar->DestroyToolBar();
            delete m_toolBar;
            m_toolBar = NULL;
        }
    }

    if( !m_toolBar ) {
        m_toolBar = new ocpnFloatingToolbarDialog( this, m_toolbarPosition, m_toolbarOrientation, m_toolbar_scalefactor );
        m_toolBar->CreateConfigMenu();
        
        if(g_bDeferredInitDone){
            m_toolBar->SetAutoHide(g_bAutoHideToolbar);
            m_toolBar->SetAutoHideTimer(g_nAutoHideToolbar);
        
        }
        // Adjust toolbar position if necessary
        if(g_MainToolbar && IsPrimaryCanvas()){
            wxRect masterToolbarRect = g_MainToolbar->GetRect();
            m_toolBar->SetULDockPosition(wxPoint(masterToolbarRect.width + 8, -1));
        }
        
     }

    if( m_toolBar ) {
        if( m_toolBar->IsToolbarShown() )
            m_toolBar->DestroyToolBar();
        

        m_toolBar->m_toolbar_scale_tools_shown = toolbar_scale_tools_shown;
        
        m_toolBar->CreateMyToolbar();
        if (m_toolBar->isSubmergedToGrabber()) {
            m_toolBar->SubmergeToGrabber();
        } else {
            m_toolBar->RePosition();
            m_toolBar->SetColorScheme(global_color_scheme);
            m_toolBar->Show(b_reshow && m_bToolbarEnable);
        }
    }

    return m_toolBar;

}

//      Update inplace the current toolbar with bitmaps corresponding to the current color scheme
void ChartCanvas::UpdateToolbarColorScheme( ColorScheme cs )
{
    if( !m_toolBar )
        return;
    
    if( m_toolBar ) {
        if(m_toolBar->GetColorScheme() != cs){
            m_toolBar->SetColorScheme( cs );
            
            if( m_toolBar->IsToolbarShown() ) {
                m_toolBar->DestroyToolBar();
                m_toolBar->CreateMyToolbar();
                if (m_toolBar->isSubmergedToGrabber()) 
                    m_toolBar->SubmergeToGrabber(); //Surface(); //SubmergeToGrabber();
                
            }
        }
    }
    
    
    if( m_toolBar->GetToolbar() ) {
        //  Re-establish toggle states
        m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, m_bFollow );
        m_toolBar->GetToolbar()->ToggleTool( ID_CURRENT, GetbShowCurrent() );
        m_toolBar->GetToolbar()->ToggleTool( ID_TIDE, GetbShowTide() );
    }
    
    return;
}

void ChartCanvas::SetCanvasToolbarItemState( int tool_id, bool state )
{
    if( GetToolbar() && GetToolbar()->GetToolbar() )
        GetToolbar()->GetToolbar()->ToggleTool( tool_id, state );
}


extern bool    g_bAllowShowScaled;

void ChartCanvas::SetShowAIS( bool show )
{
    m_bShowAIS = show;
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();
}

void ChartCanvas::SetAttenAIS( bool show )
{
    m_bShowAISScaled = show;
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();
}


void ChartCanvas::SetAISCanvasDisplayStyle(int StyleIndx)
{
    // make some arrays to hold the dfferences between cycle steps
    //show all, scaled, hide all
    bool bShowAIS_Array[3] = {true, true, false}; 
    bool bShowScaled_Array[3] = {false, true, true};
    wxString ToolShortHelp_Array[3] = { _("Show all AIS Targets"),
                                        _("Attenuate less critical AIS targets"),
                                        _("Hide AIS Targets") };
    wxString iconName_Array[3] = { _T("AIS"),  _T("AIS_Suppressed"), _T("AIS_Disabled")};
    int ArraySize = 3;
    int AIS_Toolbar_Switch = 0;
    if (StyleIndx == -1){// -1 means coming from toolbar button
        //find current state of switch 
        for ( int i = 1; i < ArraySize; i++){
            if( (bShowAIS_Array[i] == m_bShowAIS) && (bShowScaled_Array[i] == m_bShowAISScaled) )
                AIS_Toolbar_Switch = i;
        }
        AIS_Toolbar_Switch++; // we did click so continu with next item
        if ( (!g_bAllowShowScaled) && (AIS_Toolbar_Switch == 1) )
            AIS_Toolbar_Switch++; 

    }
    else { // coming from menu bar.
        AIS_Toolbar_Switch = StyleIndx;
    }
     //make sure we are not above array
    if (AIS_Toolbar_Switch >= ArraySize )
        AIS_Toolbar_Switch=0;
    
    int AIS_Toolbar_Switch_Next = AIS_Toolbar_Switch+1; //Find out what will happen at next click
    if ( (!g_bAllowShowScaled) && (AIS_Toolbar_Switch_Next == 1) )
        AIS_Toolbar_Switch_Next++;                
    if (AIS_Toolbar_Switch_Next >= ArraySize )
        AIS_Toolbar_Switch_Next=0; // If at end of cycle start at 0
    
    //Set found values to global and member variables
    m_bShowAIS = bShowAIS_Array[AIS_Toolbar_Switch];
    m_bShowAISScaled = bShowScaled_Array[AIS_Toolbar_Switch];
    if( m_toolBar ) {
        m_toolBar->SetToolShortHelp( ID_AIS, ToolShortHelp_Array[AIS_Toolbar_Switch_Next] );
        if( m_toolBar->m_pTBAISTool ) {
            m_toolBar->GetToolbar()->SetToolNormalBitmapEx( m_toolBar->m_pTBAISTool, iconName_Array[AIS_Toolbar_Switch] );
            m_toolBar->GetToolbar()->Refresh();
            m_toolBar->m_tblastAISiconName = iconName_Array[AIS_Toolbar_Switch];
        }
    }

}

void ChartCanvas::TouchAISToolActive( void )
{
    if(!m_toolBar)
        return;
    
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    
    if( m_toolBar->m_pTBAISTool ) {
        if( ( !g_pAIS->IsAISSuppressed() ) && ( !g_pAIS->IsAISAlertGeneral() ) ) {
            g_nAIS_activity_timer = 5;                // seconds
            
            wxString iconName = _T("AIS_Normal_Active");
            if( g_pAIS->IsAISAlertGeneral() ) iconName = _T("AIS_AlertGeneral_Active");
            if( g_pAIS->IsAISSuppressed() ) iconName = _T("AIS_Suppressed_Active");
            if( !m_bShowAIS ) iconName = _T("AIS_Disabled");
            
            if( m_toolBar->m_tblastAISiconName != iconName ) {
                if( m_toolBar->GetToolbar()) {
                    m_toolBar->GetToolbar()->SetToolNormalBitmapEx( m_toolBar->m_pTBAISTool, iconName );
                    m_toolBar->GetToolbar()->Refresh();
                    m_toolBar->m_tblastAISiconName = iconName;
                }
            }
        }
    }
}

void ChartCanvas::UpdateAISTBTool( void )
{
    if(!g_pAIS) return;
    if(!m_toolBar) return;
    
    bool b_need_refresh = false;
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    
    wxString iconName;
    
    if( m_toolBar->m_pTBAISTool ) {
        bool b_update = false;
        
        iconName = _T("AIS");
        if( g_pAIS->IsAISSuppressed() )
            iconName = _T("AIS_Suppressed");
        if( g_pAIS->IsAISAlertGeneral() )
            iconName = _T("AIS_AlertGeneral");
        if( !m_bShowAIS )
            iconName = _T("AIS_Disabled");
        
        //  Manage timeout for AIS activity indicator
        if( g_nAIS_activity_timer ) {
                g_nAIS_activity_timer--;
                
                if( 0 == g_nAIS_activity_timer ) b_update = true;
                else {
                    iconName = _T("AIS_Normal_Active");
                    if( g_pAIS->IsAISSuppressed() )
                        iconName = _T("AIS_Suppressed_Active");
                    if( g_pAIS->IsAISAlertGeneral() )
                        iconName = _T("AIS_AlertGeneral_Active");
                    if( !m_bShowAIS )
                        iconName = _T("AIS_Disabled");
                }
            }
            
            if( ( m_toolBar->m_tblastAISiconName != iconName ) ) b_update = true;
            
            if( b_update && m_toolBar->GetToolbar()) {
                m_toolBar->GetToolbar()->SetToolNormalBitmapEx( m_toolBar->m_pTBAISTool, iconName );
                m_toolBar->GetToolbar()->Refresh();
                m_toolBar->m_tblastAISiconName = iconName;
        }
    }
}

//---------------------------------------------------------------------------------
//
//      Compass/GPS status icon support
//
//---------------------------------------------------------------------------------

void ChartCanvas::UpdateGPSCompassStatusBox( bool b_force_new )
{
    //    Look for change in overlap or positions
    bool b_update = false;
    int cc1_edge_comp = 2;
    wxRect rect = m_Compass->GetRect();
    wxSize parent_size = GetSize();

    // check to see if it would overlap if it was in its home position (upper right)
    wxPoint tentative_pt(parent_size.x - rect.width - cc1_edge_comp, g_StyleManager->GetCurrentStyle()->GetCompassYOffset());
    wxRect tentative_rect( tentative_pt, rect.GetSize() );
    
    if( m_toolBar ) {
        
        //  If the toolbar location has changed, or the proposed compassDialog location has changed
        if( m_toolBar->GetScreenRect() != m_mainlast_tb_rect || b_force_new) {
            
            wxRect tb_rect = m_toolBar->GetScreenRect();
            wxPoint tentative_pt_in_screen(ClientToScreen(tentative_pt));
            wxRect tentative_rect_in_screen(tentative_pt_in_screen.x, tentative_pt_in_screen.y,
                                            rect.width, rect.height);
            
            //    if they would not intersect, go ahead and move it to the upper right
            //      Else it has to be on lower right
            if( !tb_rect.Intersects( tentative_rect_in_screen ) )
                m_Compass->Move( tentative_pt );
            else
                m_Compass->Move( wxPoint( GetSize().x - rect.width - cc1_edge_comp,
                                          GetSize().y - ( rect.height + cc1_edge_comp ) ) );
                
                if(rect != m_Compass->GetRect()) {
                    Refresh(true);
                    m_brepaint_piano = true;
                    b_update = true;
                }
                m_mainlast_tb_rect = tb_rect;
            
        }
    }
    else{               // No toolbar, so just place compass in upper right.
        m_Compass->Move( tentative_pt );
    }
    
    
    if( m_Compass && m_Compass->IsShown())
        m_Compass->UpdateStatus( b_force_new | b_update );
    
    if( b_force_new | b_update )
        Refresh();
        
}



void ChartCanvas::SelectChartFromStack( int index, bool bDir, ChartTypeEnum New_Type,
                                    ChartFamilyEnum New_Family )
{
    if( !GetpCurrentStack() ) return;
    if( !ChartData ) return;
    
    if( index < GetpCurrentStack()->nEntry ) {
        //      Open the new chart
        ChartBase *pTentative_Chart;
        pTentative_Chart = ChartData->OpenStackChartConditional( GetpCurrentStack(), index, bDir,
                                                                 New_Type, New_Family );
        
        if( pTentative_Chart ) {
            if( m_singleChart ) m_singleChart->Deactivate();
            
            m_singleChart = pTentative_Chart;
            m_singleChart->Activate();
            
            GetpCurrentStack()->CurrentStackEntry = ChartData->GetStackEntry( GetpCurrentStack(), m_singleChart->GetFullPath() );
        }
        //else
        //    SetChartThumbnail( -1 );   // need to reset thumbnail on failed chart open
            
            //      Setup the view
            double zLat, zLon;
        if( m_bFollow ) {
            zLat = gLat;
            zLon = gLon;
        } else {
            zLat = m_vLat;
            zLon = m_vLon;
        }
        
        double best_scale_ppm = GetBestVPScale( m_singleChart );
        double rotation = GetVPRotation();
        double oldskew = GetVPSkew();
        double newskew = m_singleChart->GetChartSkew() * PI / 180.0;
        
        if (!g_bskew_comp && !m_bCourseUp) {
            if (fabs(oldskew) > 0.0001)
                rotation = 0.0;
            if (fabs(newskew) > 0.0001)
                rotation = newskew;
        }
        
        SetViewPoint( zLat, zLon, best_scale_ppm, newskew, rotation );
        
        
        UpdateGPSCompassStatusBox( true );           // Pick up the rotation
        
    }
    
    //  refresh Piano
    int idx = GetpCurrentStack()->GetCurrentEntrydbIndex();
    if (idx < 0)
        return;
    
    std::vector<int>  piano_active_chart_index_array;
    piano_active_chart_index_array.push_back( GetpCurrentStack()->GetCurrentEntrydbIndex() );
    m_Piano->SetActiveKeyArray( piano_active_chart_index_array );
}

void ChartCanvas::SelectdbChart( int dbindex )
{
    if( !GetpCurrentStack() ) return;
    if( !ChartData ) return;
    
    if( dbindex >= 0 ) {
        //      Open the new chart
        ChartBase *pTentative_Chart;
        pTentative_Chart = ChartData->OpenChartFromDB( dbindex, FULL_INIT );
        
        if( pTentative_Chart ) {
            if( m_singleChart ) m_singleChart->Deactivate();
            
            m_singleChart = pTentative_Chart;
            m_singleChart->Activate();
            
            GetpCurrentStack()->CurrentStackEntry = ChartData->GetStackEntry( GetpCurrentStack(),  m_singleChart->GetFullPath() );
        }
        //else
        //    SetChartThumbnail( -1 );       // need to reset thumbnail on failed chart open
            
            //      Setup the view
        double zLat, zLon;
        if( m_bFollow ) {
            zLat = gLat;
            zLon = gLon;
        } else {
            zLat = m_vLat;
            zLon = m_vLon;
        }
        
        double best_scale_ppm = GetBestVPScale( m_singleChart );
        
        if( m_singleChart )
            SetViewPoint( zLat, zLon, best_scale_ppm, m_singleChart->GetChartSkew() * PI / 180., GetVPRotation() );
            
        //SetChartUpdatePeriod( );
        
        //UpdateGPSCompassStatusBox();           // Pick up the rotation
        
    }
    
    // TODO refresh_Piano();
}


void ChartCanvas::selectCanvasChartDisplay( int type, int family)
{
    double target_scale = GetVP().view_scale_ppm;
    
    if( !GetQuiltMode() ) {
        if(GetpCurrentStack()){
            int stack_index = -1;
            for(int i = 0; i < GetpCurrentStack()->nEntry ; i++){
                int check_dbIndex = GetpCurrentStack()->GetDBIndex( i );
                if (check_dbIndex < 0)
                    continue;
                const ChartTableEntry &cte = ChartData->GetChartTableEntry( check_dbIndex );
                if(type == cte.GetChartType()){
                    stack_index = i;
                    break;
                }
                else if(family == cte.GetChartFamily()){
                    stack_index = i;
                    break;
                }
            }
            
            if(stack_index >= 0){
                SelectChartFromStack( stack_index );
            }
        }
    } else {
        int sel_dbIndex = -1;
        std::vector<int>  piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
        for(unsigned int i = 0; i < piano_chart_index_array.size() ; i++){
            int check_dbIndex = piano_chart_index_array[i];
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( check_dbIndex );
            if(type == cte.GetChartType()){
                if( IsChartQuiltableRef( check_dbIndex ) ) {
                    sel_dbIndex = check_dbIndex;
                    break;
                }
            }
            else if(family == cte.GetChartFamily()){
                if( IsChartQuiltableRef( check_dbIndex ) ) {
                    sel_dbIndex = check_dbIndex;
                    break;
                }
            }
        }
        
        if(sel_dbIndex >= 0){
            SelectQuiltRefdbChart( sel_dbIndex, false );  // no autoscale
            //  Re-qualify the quilt reference chart selection
            AdjustQuiltRefChart(  );
        }
        
        //  Now reset the scale to the target...
        SetVPScale(target_scale);
        
        
        
        
    }
    
    SetQuiltChartHiLiteIndex( -1 );
    
    ReloadVP();
}





//-------------------------------------------------------------------------------------------------------
//
//      Piano support
//
//-------------------------------------------------------------------------------------------------------

void ChartCanvas::HandlePianoClick( int selected_index, int selected_dbIndex )
{
    if( !m_pCurrentStack ) return;
    if( !ChartData) return;
    
    // stop movement or on slow computer we may get something like :
    // zoom out with the wheel (timer is set)
    // quickly click and display a chart, which may zoom in
    // but the delayed timer fires first and it zooms out again!
    StopMovement();
    
    if( !GetQuiltMode() ) {
        if( m_bpersistent_quilt/* && g_bQuiltEnable*/ ) {
            if( IsChartQuiltableRef( selected_dbIndex ) ) {
                //ToggleQuiltMode();
                SelectQuiltRefdbChart( selected_dbIndex );
                m_bpersistent_quilt = false;
            } else {
                SelectChartFromStack( selected_index );
            }
        } else {
            SelectChartFromStack( selected_index );
            g_sticky_chart = selected_dbIndex;
        }
        
        if( m_singleChart )
            GetVP().SetProjectionType(m_singleChart->GetChartProjectionType());
        
    } else {
        if( IsChartQuiltableRef( selected_dbIndex ) ){
            //            if( ChartData ) ChartData->PurgeCache();
            
            
            //  If the chart is a vector chart, and of very large scale,
            //  then we had better set the new scale directly to avoid excessive underzoom
            //  on, eg, Inland ENCs
            bool set_scale = false;
            if( CHART_TYPE_S57 == ChartData->GetDBChartType( selected_dbIndex ) ){
                if( ChartData->GetDBChartScale(selected_dbIndex) < 5000){
                    set_scale = true;
                }
            }
            
            if(!set_scale){
                SelectQuiltRefdbChart( selected_dbIndex, true );  // autoscale
            }
            else {
                SelectQuiltRefdbChart( selected_dbIndex, false );  // no autoscale
                
                
                //  Adjust scale so that the selected chart is underzoomed/overzoomed by a controlled amount
                ChartBase *pc = ChartData->OpenChartFromDB( selected_dbIndex, FULL_INIT );
                if( pc ) {
                    double proposed_scale_onscreen = GetCanvasScaleFactor() / GetVPScale();
                    
                    if(g_bPreserveScaleOnX){
                        proposed_scale_onscreen = wxMin(proposed_scale_onscreen,
                                                        100 * pc->GetNormalScaleMax(GetCanvasScaleFactor(), GetCanvasWidth()));
                    }
                    else{
                        proposed_scale_onscreen = wxMin(proposed_scale_onscreen,
                                                        20 * pc->GetNormalScaleMax(GetCanvasScaleFactor(), GetCanvasWidth()));
                        
                        proposed_scale_onscreen = wxMax(proposed_scale_onscreen,
                                                        pc->GetNormalScaleMin(GetCanvasScaleFactor(), g_b_overzoom_x));
                    }
                    
                    SetVPScale( GetCanvasScaleFactor() / proposed_scale_onscreen );
                }
            }
        }
        else {
            //TODOToggleQuiltMode();
            SelectdbChart( selected_dbIndex );
            m_bpersistent_quilt = true;
        }
    }
    
    SetQuiltChartHiLiteIndex( -1 );
    gFrame->UpdateGlobalMenuItems(); // update the state of the menu items (checkmarks etc)
    HideChartInfoWindow();
    DoCanvasUpdate();
    ReloadVP();                  // Pick up the new selections
}


void ChartCanvas::HandlePianoRClick( int x, int y, int selected_index, int selected_dbIndex )
{
    if( !GetpCurrentStack() ) return;
    
    PianoPopupMenu( x, y, selected_index, selected_dbIndex );
    UpdateCanvasControlBar();
    
    SetQuiltChartHiLiteIndex( -1 );
}

void ChartCanvas::HandlePianoRollover( int selected_index, int selected_dbIndex )
{
    if( !GetpCurrentStack() ) return;
    if( !ChartData ) return;
    
    if (ChartData->IsBusy())
        return;
    
    wxPoint key_location = m_Piano->GetKeyOrigin( selected_index );
    
    if( !GetQuiltMode() ) {
        //SetChartThumbnail( selected_index );
        ShowChartInfoWindow( key_location.x, selected_dbIndex );
    } else {
        std::vector<int>  piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
        
        if( ( GetpCurrentStack()->nEntry > 1 ) || ( piano_chart_index_array.size() >= 1 ) ) {
            ShowChartInfoWindow( key_location.x, selected_dbIndex );
            SetQuiltChartHiLiteIndex( selected_dbIndex );
            
            ReloadVP( false );         // no VP adjustment allowed
        } else if( GetpCurrentStack()->nEntry == 1 ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( GetpCurrentStack()->GetDBIndex( 0 ) );
            if( CHART_TYPE_CM93COMP != cte.GetChartType() ) {
                ShowChartInfoWindow( key_location.x, selected_dbIndex );
                ReloadVP( false );
            } else if( ( -1 == selected_index ) && ( -1 == selected_dbIndex ) ) {
                ShowChartInfoWindow( key_location.x, selected_dbIndex );
            }
        }
        //SetChartThumbnail( -1 );        // hide all thumbs in quilt mode
    }
}


void ChartCanvas::UpdateCanvasControlBar( void )
{
    
    if( !GetpCurrentStack() ) return;
    if( !ChartData) return;
    if ( !g_bShowChartBar ) return;
    
    int sel_type = -1;
    int sel_family = -1;
    
    std::vector<int>  piano_chart_index_array;
    std::vector<int>  empty_piano_chart_index_array;
    
    wxString old_hash = m_Piano->GetStoredHash();
    
    if( GetQuiltMode() ) {
        piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
        m_Piano->SetKeyArray( piano_chart_index_array );
        
        std::vector<int>  piano_active_chart_index_array = GetQuiltCandidatedbIndexArray();
        m_Piano->SetActiveKeyArray( piano_active_chart_index_array );
        
        std::vector<int>  piano_eclipsed_chart_index_array = GetQuiltEclipsedStackdbIndexArray();
        m_Piano->SetEclipsedIndexArray( piano_eclipsed_chart_index_array );
        
        m_Piano->SetNoshowIndexArray( g_quilt_noshow_index_array );
        
        sel_type = ChartData->GetDBChartType(GetQuiltReferenceChartIndex());
        sel_family = ChartData->GetDBChartFamily(GetQuiltReferenceChartIndex());
    } else {
        piano_chart_index_array = ChartData->GetCSArray( GetpCurrentStack() );
        m_Piano->SetKeyArray( piano_chart_index_array );
        // TODO refresh_Piano();
        
        if(m_singleChart){
            sel_type = m_singleChart->GetChartType();
            sel_family = m_singleChart->GetChartFamily();
        }
        
    }
    
    //    Set up the TMerc and Skew arrays
    std::vector<int>  piano_skew_chart_index_array;
    std::vector<int>  piano_tmerc_chart_index_array;
    std::vector<int>  piano_poly_chart_index_array;
    
    for( unsigned int ino = 0; ino < piano_chart_index_array.size(); ino++ ) {
        const ChartTableEntry &ctei = ChartData->GetChartTableEntry(
            piano_chart_index_array[ino] );
        double skew_norm = ctei.GetChartSkew();
        if( skew_norm > 180. ) skew_norm -= 360.;
        
        if( ctei.GetChartProjectionType() == PROJECTION_TRANSVERSE_MERCATOR )
            piano_tmerc_chart_index_array.push_back( piano_chart_index_array[ino] );
        
        //    Polyconic skewed charts should show as skewed
            else
                if( ctei.GetChartProjectionType() == PROJECTION_POLYCONIC ) {
                    if( fabs( skew_norm ) > 1. )
                        piano_skew_chart_index_array.push_back(piano_chart_index_array[ino] );
                    else
                        piano_poly_chart_index_array.push_back( piano_chart_index_array[ino] );
                } else
                    if( fabs( skew_norm ) > 1. )
                        piano_skew_chart_index_array.push_back(piano_chart_index_array[ino] );
                    
    }
    m_Piano->SetSkewIndexArray( piano_skew_chart_index_array );
    m_Piano->SetTmercIndexArray( piano_tmerc_chart_index_array );
    m_Piano->SetPolyIndexArray( piano_poly_chart_index_array );
    m_Piano->FormatKeys();
    
    wxString new_hash = m_Piano->GenerateAndStoreNewHash();
    if(new_hash != old_hash) {
        //SetChartThumbnail( -1 );
        HideChartInfoWindow();
        m_Piano->ResetRollover();
        SetQuiltChartHiLiteIndex( -1 );
        m_brepaint_piano = true;
    }
    
    // Create a bitmask int that describes what Family/Type of charts are shown in the bar,
    // and notify the platform.
    int mask = 0;
    for( unsigned int ino = 0; ino < piano_chart_index_array.size(); ino++ ) {
        const ChartTableEntry &ctei = ChartData->GetChartTableEntry( piano_chart_index_array[ino] );
        ChartFamilyEnum e = (ChartFamilyEnum)ctei.GetChartFamily();
        ChartTypeEnum t = (ChartTypeEnum)ctei.GetChartType();
        if(e == CHART_FAMILY_RASTER)
            mask |= 1;
        if(e == CHART_FAMILY_VECTOR){
            if(t == CHART_TYPE_CM93COMP)
                mask |= 4;
            else
                mask |= 2;
        }
    }
    
    wxString s_indicated;
    if(sel_type == CHART_TYPE_CM93COMP)
        s_indicated = _T("cm93");
    else{
        if(sel_family == CHART_FAMILY_RASTER)
            s_indicated = _T("raster");
        else if(sel_family == CHART_FAMILY_VECTOR)
            s_indicated = _T("vector");
    }
    
    g_Platform->setChartTypeMaskSel(mask, s_indicated);
    
}

void ChartCanvas::FormatPianoKeys( void )
{
    m_Piano->FormatKeys();
    
}

void ChartCanvas::PianoPopupMenu( int x, int y, int selected_index, int selected_dbIndex )
{
    if( !GetpCurrentStack() ) return;

    //    No context menu if quilting is disabled
    if( !GetQuiltMode() ) return;

    menu_selected_dbIndex = selected_dbIndex;
    menu_selected_index = selected_index;

    m_piano_ctx_menu = new wxMenu();

    //    Search the no-show array
    bool b_is_in_noshow = false;
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.size(); i++ ) {
        if( g_quilt_noshow_index_array[i] == selected_dbIndex ) // chart is in the noshow list
                {
            b_is_in_noshow = true;
            break;
        }
    }

    if( b_is_in_noshow ) {
        m_piano_ctx_menu->Append( ID_PIANO_ENABLE_QUILT_CHART, _("Show This Chart") );
        Connect( ID_PIANO_ENABLE_QUILT_CHART, wxEVT_COMMAND_MENU_SELECTED,
                wxCommandEventHandler(ChartCanvas::OnPianoMenuEnableChart) );
    } else
        if( GetpCurrentStack()->nEntry > 1 ) {
            m_piano_ctx_menu->Append( ID_PIANO_DISABLE_QUILT_CHART, _("Hide This Chart") );
            Connect( ID_PIANO_DISABLE_QUILT_CHART, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler(ChartCanvas::OnPianoMenuDisableChart) );
        }

    wxPoint pos = wxPoint(x, y - 30);

//        Invoke the drop-down menu
    if( m_piano_ctx_menu->GetMenuItems().GetCount() )
        PopupMenu( m_piano_ctx_menu, pos );

    delete m_piano_ctx_menu;
    m_piano_ctx_menu = NULL;

    HideChartInfoWindow();
    m_Piano->ResetRollover();

    SetQuiltChartHiLiteIndex( -1 );

    ReloadVP();
}

void ChartCanvas::OnPianoMenuEnableChart( wxCommandEvent& event )
{
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.size(); i++ ) {
        if( g_quilt_noshow_index_array[i] == menu_selected_dbIndex ) // chart is in the noshow list
                {
            g_quilt_noshow_index_array.erase(g_quilt_noshow_index_array.begin() + i );
            break;
        }
    }
}

void ChartCanvas::OnPianoMenuDisableChart( wxCommandEvent& event )
{
    if( !GetpCurrentStack() ) return;
    if( !ChartData ) return;

    RemoveChartFromQuilt( menu_selected_dbIndex );

//      It could happen that the chart being disabled is the reference chart....
    if( menu_selected_dbIndex == GetQuiltRefChartdbIndex() ) {
        int type = ChartData->GetDBChartType( menu_selected_dbIndex );

        int i = menu_selected_index + 1;          // select next smaller scale chart
        bool b_success = false;
        while( i < GetpCurrentStack()->nEntry - 1 ) {
            int dbIndex = GetpCurrentStack()->GetDBIndex( i );
            if( type == ChartData->GetDBChartType( dbIndex ) ) {
                SelectQuiltRefChart( i );
                b_success = true;
                break;
            }
            i++;
        }

        //    If that did not work, try to select the next larger scale compatible chart
        if( !b_success ) {
            i = menu_selected_index - 1;
            while( i > 0 ) {
                int dbIndex = GetpCurrentStack()->GetDBIndex( i );
                if( type == ChartData->GetDBChartType( dbIndex ) ) {
                    SelectQuiltRefChart( i );
                    b_success = true;
                    break;
                }
                i--;
            }
        }
    }
}

void ChartCanvas::RemoveChartFromQuilt( int dbIndex )
{
    //    Remove the item from the list (if it appears) to avoid multiple addition
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.size(); i++ ) {
        if( g_quilt_noshow_index_array[i] == dbIndex ) // chart is already in the noshow list
                {
                    g_quilt_noshow_index_array.erase(g_quilt_noshow_index_array.begin() + i );
                    break;
                }
    }
    
    g_quilt_noshow_index_array.push_back( dbIndex );
    
}



bool ChartCanvas::UpdateS52State()
{
    bool retval = false;
//    printf("    update %d\n", IsPrimaryCanvas());
    
    if(ps52plib){
        ps52plib->SetShowS57Text( m_encShowText );
        ps52plib->SetDisplayCategory( (DisCat) m_encDisplayCategory );
        ps52plib->m_bShowSoundg = m_encShowDepth;
        ps52plib->m_bShowAtonText = m_encShowBuoyLabels;
        ps52plib->m_bShowLdisText = m_encShowLightDesc;
        
        // Lights
        if(!m_encShowLights)                     // On, going off
            ps52plib->AddObjNoshow("LIGHTS");
        else                                   // Off, going on
            ps52plib->RemoveObjNoshow("LIGHTS");
        ps52plib->SetLightsOff( !m_encShowLights );
        ps52plib->m_bExtendLightSectors = true;
        
        // TODO ps52plib->m_bShowAtons = m_encShowBuoys;
        ps52plib->SetAnchorOn( m_encShowAnchor );
        ps52plib->SetQualityOfData( m_encShowDataQual );
    }
    
    return retval;
}
    
void ChartCanvas::SetShowENCDataQual( bool show )
{
    m_encShowDataQual = show;
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}


void ChartCanvas::SetShowENCText( bool show )
{ 
    m_encShowText = show;
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}
    
void ChartCanvas::SetENCDisplayCategory( int category )
{
    m_encDisplayCategory = category;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}
    

void ChartCanvas::SetShowENCDepth( bool show )
{
    m_encShowDepth = show;
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}
    
void ChartCanvas::SetShowENCLightDesc( bool show )
{
    m_encShowLightDesc = show;
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}
    
void ChartCanvas::SetShowENCBuoyLabels( bool show )
{
    m_encShowBuoyLabels = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}
    
void ChartCanvas::SetShowENCLights( bool show )
{
    m_encShowLights = show;
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}
    
void ChartCanvas::SetShowENCAnchor( bool show )
{
    m_encShowAnchor = show;
    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

wxRect ChartCanvas::GetMUIBarRect()
{
    wxRect rv;
    if(m_muiBar){
        rv = m_muiBar->GetRect();
    }
    
    return rv;
}

//--------------------------------------------------------------------------------------------------------
//    Screen Brightness Control Support Routines
//
//--------------------------------------------------------------------------------------------------------

#ifdef __UNIX__
#define BRIGHT_XCALIB
#define __OPCPN_USEICC__
#endif


#ifdef __OPCPN_USEICC__
int CreateSimpleICCProfileFile(const char *file_name, double co_red, double co_green, double co_blue);

wxString temp_file_name;
#endif

#if 0
class ocpnCurtain: public wxDialog
{
    DECLARE_CLASS( ocpnCurtain )
    DECLARE_EVENT_TABLE()

public:
    ocpnCurtain( wxWindow *parent, wxPoint position, wxSize size, long wstyle );
    ~ocpnCurtain( );
    bool ProcessEvent(wxEvent& event);

};

IMPLEMENT_CLASS ( ocpnCurtain, wxDialog )

BEGIN_EVENT_TABLE(ocpnCurtain, wxDialog)
END_EVENT_TABLE()

ocpnCurtain::ocpnCurtain( wxWindow *parent, wxPoint position, wxSize size, long wstyle )
{
    wxDialog::Create( parent, -1, _T("ocpnCurtain"), position, size, wxNO_BORDER | wxSTAY_ON_TOP );
}

ocpnCurtain::~ocpnCurtain()
{
}

bool ocpnCurtain::ProcessEvent(wxEvent& event)
{
    GetParent()->GetEventHandler()->SetEvtHandlerEnabled(true);
    return GetParent()->GetEventHandler()->ProcessEvent(event);
}
#endif

#ifdef __WIN32__
#include <windows.h>

HMODULE hGDI32DLL;
typedef BOOL (WINAPI *SetDeviceGammaRamp_ptr_type)( HDC hDC, LPVOID lpRampTable );
typedef BOOL (WINAPI *GetDeviceGammaRamp_ptr_type)( HDC hDC, LPVOID lpRampTable );
SetDeviceGammaRamp_ptr_type g_pSetDeviceGammaRamp;            // the API entry points in the dll
GetDeviceGammaRamp_ptr_type g_pGetDeviceGammaRamp;

WORD *g_pSavedGammaMap;

#endif

int InitScreenBrightness( void )
{
#ifdef __WIN32__
    if( gFrame->GetPrimaryCanvas()->GetglCanvas() && g_bopengl ) {
        HDC hDC;
        BOOL bbr;

        if( NULL == hGDI32DLL ) {
            hGDI32DLL = LoadLibrary( TEXT("gdi32.dll") );

            if( NULL != hGDI32DLL ) {
                //Get the entry points of the required functions
                g_pSetDeviceGammaRamp = (SetDeviceGammaRamp_ptr_type) GetProcAddress( hGDI32DLL,
                                        "SetDeviceGammaRamp" );
                g_pGetDeviceGammaRamp = (GetDeviceGammaRamp_ptr_type) GetProcAddress( hGDI32DLL,
                                        "GetDeviceGammaRamp" );

                //    If the functions are not found, unload the DLL and return false
                if( ( NULL == g_pSetDeviceGammaRamp ) || ( NULL == g_pGetDeviceGammaRamp ) ) {
                    FreeLibrary( hGDI32DLL );
                    hGDI32DLL = NULL;
                    return 0;
                }
            }
        }

        //    Interface is ready, so....
        //    Get some storage
        if( !g_pSavedGammaMap ) {
            g_pSavedGammaMap = (WORD *) malloc( 3 * 256 * sizeof(WORD) );

            hDC = GetDC( NULL );                                      // Get the full screen DC
            bbr = g_pGetDeviceGammaRamp( hDC, g_pSavedGammaMap );    // Get the existing ramp table
            ReleaseDC( NULL, hDC );                                       // Release the DC
        }

        //    On Windows hosts, try to adjust the registry to allow full range setting of Gamma table
        //    This is an undocumented Windows hack.....
        wxRegKey *pRegKey =
            new wxRegKey(
            _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ICM") );
        if( !pRegKey->Exists() ) pRegKey->Create();
        pRegKey->SetValue( _T("GdiIcmGammaRange"), 256 );

        g_brightness_init = true;
        return 1;
    }

    else {
        if( NULL == g_pcurtain ) {
            if( gFrame->CanSetTransparent() ) {
                //    Build the curtain window
                g_pcurtain = new wxDialog( gFrame->GetPrimaryCanvas(), -1, _T(""), wxPoint( 0, 0 ), ::wxGetDisplaySize(),
                                           wxNO_BORDER | wxTRANSPARENT_WINDOW | wxSTAY_ON_TOP | wxDIALOG_NO_PARENT );

                //                  g_pcurtain = new ocpnCurtain(gFrame, wxPoint(0,0),::wxGetDisplaySize(),
                //                      wxNO_BORDER | wxTRANSPARENT_WINDOW |wxSTAY_ON_TOP | wxDIALOG_NO_PARENT);

                g_pcurtain->Hide();

                HWND hWnd = GetHwndOf(g_pcurtain);
                SetWindowLong( hWnd, GWL_EXSTYLE,
                               GetWindowLong( hWnd, GWL_EXSTYLE ) | ~WS_EX_APPWINDOW );
                g_pcurtain->SetBackgroundColour( wxColour( 0, 0, 0 ) );
                g_pcurtain->SetTransparent( 0 );

                g_pcurtain->Maximize();
                g_pcurtain->Show();

                //    All of this is obtuse, but necessary for Windows...
                g_pcurtain->Enable();
                g_pcurtain->Disable();

                gFrame->Disable();
                gFrame->Enable();
                //SetFocus();

            }
        }
        g_brightness_init = true;

        return 1;
    }
#else
    //    Look for "xcalib" application
    wxString cmd ( _T ( "xcalib -version" ) );

    wxArrayString output;
    long r = wxExecute ( cmd, output );
    if(0 != r)
        wxLogMessage(_("   External application \"xcalib\" not found. Screen brightness not changed."));

    g_brightness_init = true;
    return 0;
#endif
}

int RestoreScreenBrightness( void )
{
#ifdef __WIN32__

    if( g_pSavedGammaMap ) {
        HDC hDC = GetDC( NULL );                                 // Get the full screen DC
        g_pSetDeviceGammaRamp( hDC, g_pSavedGammaMap );          // Restore the saved ramp table
        ReleaseDC( NULL, hDC );                                  // Release the DC

        free( g_pSavedGammaMap );
        g_pSavedGammaMap = NULL;
    }

    if( g_pcurtain ) {
        g_pcurtain->Close();
        g_pcurtain->Destroy();
        g_pcurtain = NULL;
    }

    g_brightness_init = false;
    return 1;

#endif

#ifdef BRIGHT_XCALIB
    if(g_brightness_init)
    {
        wxString cmd;
        cmd = _T("xcalib -clear");
        wxExecute(cmd, wxEXEC_ASYNC);
        g_brightness_init = false;
    }

    return 1;
#endif

    return 0;
}

//    Set brightness. [0..100]
int SetScreenBrightness( int brightness )
{
#ifdef __WIN32__

    //    Under Windows, we use the SetDeviceGammaRamp function which exists in some (most modern?) versions of gdi32.dll
    //    Load the required library dll, if not already in place
    if( gFrame->GetPrimaryCanvas()->GetglCanvas() && g_bopengl ) {
        if( g_pcurtain ) {
            g_pcurtain->Close();
            g_pcurtain->Destroy();
            g_pcurtain = NULL;
        }

        InitScreenBrightness();

        if( NULL == hGDI32DLL ) {
            // Unicode stuff.....
            wchar_t wdll_name[80];
            MultiByteToWideChar( 0, 0, "gdi32.dll", -1, wdll_name, 80 );
            LPCWSTR cstr = wdll_name;

            hGDI32DLL = LoadLibrary( cstr );

            if( NULL != hGDI32DLL ) {
                //Get the entry points of the required functions
                g_pSetDeviceGammaRamp = (SetDeviceGammaRamp_ptr_type) GetProcAddress( hGDI32DLL,
                                        "SetDeviceGammaRamp" );
                g_pGetDeviceGammaRamp = (GetDeviceGammaRamp_ptr_type) GetProcAddress( hGDI32DLL,
                                        "GetDeviceGammaRamp" );

                //    If the functions are not found, unload the DLL and return false
                if( ( NULL == g_pSetDeviceGammaRamp ) || ( NULL == g_pGetDeviceGammaRamp ) ) {
                    FreeLibrary( hGDI32DLL );
                    hGDI32DLL = NULL;
                    return 0;
                }
            }
        }

        HDC hDC = GetDC( NULL );                          // Get the full screen DC

        /*
         int cmcap = GetDeviceCaps(hDC, COLORMGMTCAPS);
         if (cmcap != CM_GAMMA_RAMP)
         {
         wxLogMessage(_T("    Video hardware does not support brightness control by gamma ramp adjustment."));
         return false;
         }
         */

        int increment = brightness * 256 / 100;

        // Build the Gamma Ramp table
        WORD GammaTable[3][256];

        int table_val = 0;
        for( int i = 0; i < 256; i++ ) {

            GammaTable[0][i] = r_gamma_mult * (WORD) table_val;
            GammaTable[1][i] = g_gamma_mult * (WORD) table_val;
            GammaTable[2][i] = b_gamma_mult * (WORD) table_val;

            table_val += increment;

            if( table_val > 65535 ) table_val = 65535;

        }

        g_pSetDeviceGammaRamp( hDC, GammaTable );          // Set the ramp table
        ReleaseDC( NULL, hDC );                                     // Release the DC

        return 1;
    } else {
        if( g_pSavedGammaMap ) {
            HDC hDC = GetDC( NULL );                                       // Get the full screen DC
            g_pSetDeviceGammaRamp( hDC, g_pSavedGammaMap );          // Restore the saved ramp table
            ReleaseDC( NULL, hDC );                                             // Release the DC
        }

        if(brightness < 100 ){
            if( NULL == g_pcurtain )
                InitScreenBrightness();
    
            if( g_pcurtain ) {
                int sbrite = wxMax(1, brightness);
                sbrite = wxMin(100, sbrite);

                g_pcurtain->SetTransparent( ( 100 - sbrite ) * 256 / 100 );
            }
        }
        else{
            if( g_pcurtain ) {
                g_pcurtain->Close();
                g_pcurtain->Destroy();
                g_pcurtain = NULL;
            }
        }
        
            
        return 1;
    }

#endif

#ifdef BRIGHT_XCALIB

    if(!g_brightness_init)
    {
        last_brightness = 100;
        g_brightness_init = true;
        temp_file_name = wxFileName::CreateTempFileName(_T(""));
        InitScreenBrightness();
    }

#ifdef __OPCPN_USEICC__
    //  Create a dead simple temporary ICC profile file, with gamma ramps set as desired,
    //  and then activate this temporary profile using xcalib <filename>
    if(!CreateSimpleICCProfileFile ( ( const char * ) temp_file_name.fn_str(),
                                     brightness * r_gamma_mult,
                                     brightness * g_gamma_mult,
                                     brightness * b_gamma_mult ))
    {
        wxString cmd ( _T ( "xcalib " ) );
        cmd += temp_file_name;

        wxExecute ( cmd, wxEXEC_ASYNC );
    }

#else
    //    Or, use "xcalib -co" to set overall contrast value
    //    This is not as nice, since the -co parameter wants to be a fraction of the current contrast,
    //    and values greater than 100 are not allowed.  As a result, increases of contrast must do a "-clear" step
    //    first, which produces objectionable flashing.
    if(brightness > last_brightness)
    {
        wxString cmd;
        cmd = _T("xcalib -clear");
        wxExecute(cmd, wxEXEC_ASYNC);

        ::wxMilliSleep(10);

        int brite_adj = wxMax(1, brightness);
        cmd.Printf(_T("xcalib -co %2d -a"), brite_adj);
        wxExecute(cmd, wxEXEC_ASYNC);
    }
    else
    {
        int brite_adj = wxMax(1, brightness);
        int factor = (brite_adj * 100) / last_brightness;
        factor = wxMax(1, factor);
        wxString cmd;
        cmd.Printf(_T("xcalib -co %2d -a"), factor);
        wxExecute(cmd, wxEXEC_ASYNC);
    }

#endif

    last_brightness = brightness;

#endif

    return 0;
}

#ifdef __OPCPN_USEICC__

#define MLUT_TAG     0x6d4c5554L
#define VCGT_TAG     0x76636774L

int GetIntEndian(unsigned char *s)
{
    int ret;
    unsigned char *p;
    int i;

    p = (unsigned char *)&ret;

    if(1)
        for(i=sizeof(int)-1; i>-1; --i)
            *p++ = s[i];
    else
        for(i=0; i<(int)sizeof(int); ++i)
            *p++ = s[i];

    return ret;
}

unsigned short GetShortEndian(unsigned char *s)
{
    unsigned short ret;
    unsigned char *p;
    int i;

    p = (unsigned char *)&ret;

    if(1)
        for(i=sizeof(unsigned short)-1; i>-1; --i)
            *p++ = s[i];
    else
        for(i=0; i<(int)sizeof(unsigned short); ++i)
            *p++ = s[i];

    return ret;
}

//    Create a very simple Gamma correction file readable by xcalib
int CreateSimpleICCProfileFile(const char *file_name, double co_red, double co_green, double co_blue)
{
    FILE *fp;

    if(file_name)
    {
        fp = fopen(file_name, "wb");
        if(!fp)
            return -1; /* file can not be created */
    }
    else
        return -1; /* filename char pointer not valid */

    //    Write header
    char header[128];
    for(int i=0; i< 128; i++)
        header[i] = 0;

    fwrite(header, 128, 1, fp);

    //    Num tags
    int numTags0 = 1;
    int numTags = GetIntEndian((unsigned char *)&numTags0);
    fwrite(&numTags, 1, 4, fp);

    int tagName0 = VCGT_TAG;
    int tagName = GetIntEndian((unsigned char *)&tagName0);
    fwrite(&tagName, 1, 4, fp);

    int tagOffset0 = 128 + 4 * sizeof(int);
    int tagOffset = GetIntEndian((unsigned char *)&tagOffset0);
    fwrite(&tagOffset, 1, 4, fp);

    int tagSize0 = 1;
    int tagSize = GetIntEndian((unsigned char *)&tagSize0);
    fwrite(&tagSize, 1, 4, fp);

    fwrite(&tagName, 1, 4, fp);// another copy of tag

    fwrite(&tagName, 1, 4, fp);// dummy

    //  Table type

    /* VideoCardGammaTable (The simplest type) */
    int gammatype0 = 0;
    int gammatype = GetIntEndian((unsigned char *)&gammatype0);
    fwrite(&gammatype, 1, 4, fp);

    int numChannels0 = 3;
    unsigned short numChannels = GetShortEndian((unsigned char *)&numChannels0);
    fwrite(&numChannels, 1, 2, fp);

    int numEntries0 = 256;
    unsigned short numEntries = GetShortEndian((unsigned char *)&numEntries0);
    fwrite(&numEntries, 1, 2, fp);

    int entrySize0 = 1;
    unsigned short entrySize = GetShortEndian((unsigned char *)&entrySize0);
    fwrite(&entrySize, 1, 2, fp);

    unsigned char ramp[256];

    //    Red ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_red/100.;
    fwrite(ramp, 256, 1, fp);

    //    Green ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_green/100.;
    fwrite(ramp, 256, 1, fp);

    //    Blue ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_blue/100.;
    fwrite(ramp, 256, 1, fp);

    fclose(fp);

    return 0;
}
#endif // __OPCPN_USEICC__

