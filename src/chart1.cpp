/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
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
#ifdef __WXMSW__
//#include "c:\\Program Files\\visual leak detector\\include\\vld.h"
#endif

#include "wx/print.h"
#include "wx/printdlg.h"
#include "wx/artprov.h"
#include "wx/stdpaths.h"
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/aui/aui.h>
#include <version.h> //Gunther
#include <wx/dialog.h>
#include <wx/progdlg.h>

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
//  #include "scrollingdialog.h"
#endif

#include "dychart.h"

#ifdef __WXMSW__
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <psapi.h>
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>
#endif

#include "chart1.h"
#include "chcanv.h"
#include "chartdb.h"
#include "navutil.h"
#include "styles.h"
#include "routeman.h"
#include "statwin.h"
#include "concanv.h"
#include "options.h"
#include "about.h"
#include "thumbwin.h"
#include "tcmgr.h"
#include "ais.h"
#include "chartimg.h"               // for ChartBaseBSB
#include "routeprop.h"
#include "toolbar.h"
#include "compasswin.h"
#include "datastream.h"
#include "multiplexer.h"

#include "cutil.h"
#include "routemanagerdialog.h"
#include "pluginmanager.h"

#include <wx/image.h>
#include "wx/apptrait.h"

#ifdef __WXOSX__
#include "macutils.h"
#endif

#ifdef USE_S57
#include "cm93.h"
#include "s52plib.h"
#include "s57chart.h"
#include "cpl_csv.h"
#endif

#ifdef __WXMSW__
//#define __MSVC__LEAK
#ifdef __MSVC__LEAK
#include "Stackwalker.h"
#endif
#endif

#include <wx/jsonreader.h>

WX_DECLARE_OBJARRAY(wxDialog *, MyDialogPtrArray);

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( ArrayOfCDI );
WX_DEFINE_OBJARRAY( ArrayOfRect );
WX_DEFINE_OBJARRAY( MyDialogPtrArray );

#ifdef __WXMSW__
void RedirectIOToConsole();
#endif


//------------------------------------------------------------------------------
//      Static variable definition
//------------------------------------------------------------------------------

FILE                      *flog;
wxLog                     *logger;
wxLog                     *Oldlogger;
bool                      g_bFirstRun;
wxString                  glog_file;
wxString                  gConfig_File;

int                       g_unit_test_1;

MyFrame                   *gFrame;

ChartCanvas               *cc1;
ConsoleCanvas             *console;
StatWin                   *stats;

MyConfig                  *pConfig;

ChartBase                 *Current_Vector_Ch;
ChartBase                 *Current_Ch;
ChartDB                   *ChartData;
ChartStack                *pCurrentStack;
wxString                  *pdir_list[20];
int                       g_restore_stackindex;
int                       g_restore_dbindex;
double                    g_ChartNotRenderScaleFactor;

RouteList                 *pRouteList;
LayerList                 *pLayerList;
bool                      g_bIsNewLayer;
int                       g_LayerIdx;
bool                      g_bLayerViz;

Select                    *pSelect;
Select                    *pSelectTC;
Select                    *pSelectAIS;

Routeman                  *g_pRouteMan;
WayPointman               *pWayPointMan;
MarkInfoImpl              *pMarkPropDialog;
RouteProp                 *pRoutePropDialog;
MarkInfoImpl              *pMarkInfoDialog;
RouteManagerDialog        *pRouteManagerDialog;
GoToPositionDialog        *pGoToPositionDialog;

double                    gLat, gLon, gCog, gSog, gHdt, gHdm, gVar;
double                    vLat, vLon;
double                    initial_scale_ppm;

int                       g_nbrightness;

bool                      bDBUpdateInProgress;

ThumbWin                  *pthumbwin;
TCMgr                     *ptcmgr;


bool                      bDrawCurrentValues;

wxString                  g_PrivateDataDir;
wxString                  g_SData_Locn;
wxString                  *pChartListFileName;
wxString                  *pHome_Locn;
wxString                  *pWorldMapLocation;
wxString                  *pInit_Chart_Dir;
wxString                  g_csv_locn;
wxString                  g_SENCPrefix;
wxString                  g_UserPresLibData;
wxString                  g_Plugin_Dir;
wxString                  g_VisibleLayers;
wxString                  g_InvisibleLayers;

extern wxString           str_version_major;
extern wxString           str_version_minor;
extern wxString           str_version_patch;

int                       user_user_id;
int                       file_user_id;

int                       quitflag;
int                       g_tick;
int                       g_mem_total, g_mem_used, g_mem_initial;

bool                      s_bSetSystemTime;

wxString                  *phost_name;

static unsigned int       malloc_max;

wxArrayOfConnPrm          *g_pConnectionParams;
//OCP_GARMIN_Thread         *pGARMIN_Thread;

wxDateTime                g_start_time;
wxDateTime                g_loglast_time;
wxSound                   bells_sound[8];

RoutePoint                *pAnchorWatchPoint1;
RoutePoint                *pAnchorWatchPoint2;
double                    AnchorPointMinDist;
bool                      AnchorAlertOn1, AnchorAlertOn2;
bool                      g_bCruising;

ChartDummy                *pDummyChart;

ocpnToolBarSimple*        g_toolbar;
ocpnStyle::StyleManager*  g_StyleManager;

// Global print data, to remember settings during the session
wxPrintData               *g_printData = (wxPrintData*) NULL ;

// Global page setup data
wxPageSetupData*          g_pageSetupData = (wxPageSetupData*) NULL;

bool                      g_bShowOutlines;
bool                      g_bShowDepthUnits;
bool                      g_bDisplayGrid;  // Flag indicating weather the lat/lon grid should be displayed
bool                      g_bShowActiveRouteHighway;
int                       g_nNMEADebug;
int                       g_nAWDefault;
int                       g_nAWMax;
bool                      g_bPlayShipsBells;
bool                      g_bFullscreenToolbar;
bool                      g_bShowLayers;
bool                      g_bTransparentToolbar;
bool                      g_bPermanentMOBIcon;

int                       g_iSDMMFormat;

int                       g_iNavAidRadarRingsNumberVisible;
float                     g_fNavAidRadarRingsStep;
int                       g_pNavAidRadarRingsStepUnits;
bool                      g_bWayPointPreventDragging;
bool                      g_bConfirmObjectDelete;

FontMgr                   *pFontMgr;

ColorScheme               global_color_scheme;
int                       Usercolortable_index;
wxArrayPtrVoid            *UserColorTableArray;
wxArrayPtrVoid            *UserColourHashTableArray;
wxColorHashMap            *pcurrent_user_color_hash;

int                       gps_watchdog_timeout_ticks;
int                       sat_watchdog_timeout_ticks;

int                       gGPS_Watchdog;
bool                      bGPSValid;

int                       gHDx_Watchdog;
int                       gHDT_Watchdog;
int                       gVAR_Watchdog;
bool                      g_bHDxValid;
bool                      g_bHDTValid;
bool                      g_bVARValid;
bool                      g_bHDT_Rx;
bool                      g_bVAR_Rx;

int                       gSAT_Watchdog;
int                       g_SatsInView;
bool                      g_bSatValid;

bool                      g_bDebugCM93;
bool                      g_bDebugS57;

bool                      g_bfilter_cogsog;
int                       g_COGFilterSec;
int                       g_SOGFilterSec;

int                       g_ChartUpdatePeriod;
int                       g_SkewCompUpdatePeriod;

int                       g_lastClientRectx;
int                       g_lastClientRecty;
int                       g_lastClientRectw;
int                       g_lastClientRecth;

#ifdef USE_S57
s52plib                   *ps52plib;
S57ClassRegistrar         *g_poRegistrar;
s57RegistrarMgr           *m_pRegistrarMan;

CM93OffsetDialog          *g_pCM93OffsetDialog;
#endif

#ifdef __WXOSX__
#include "macutils.h"
#endif

// begin rms
#if defined( USE_S57) || defined ( __WXOSX__ )
#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL
// end rms
extern bool               s_glu_dll_ready;
extern HINSTANCE          s_hGLU_DLL; // Handle to DLL
#endif
#endif
#endif
#endif

double                    g_ownship_predictor_minutes;
int                       g_current_arrow_scale;

Multiplexer               *g_pMUX;

AIS_Decoder               *g_pAIS;
bool                      g_bAIS_CPA_Alert;
bool                      g_bAIS_CPA_Alert_Audio;
AISTargetAlertDialog      *g_pais_alert_dialog_active;
AISTargetQueryDialog      *g_pais_query_dialog_active;

int                       g_ais_alert_dialog_x, g_ais_alert_dialog_y;
int                       g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
int                       g_ais_query_dialog_x, g_ais_query_dialog_y;

int                       g_S57_dialog_sx, g_S57_dialog_sy;

int                       g_nframewin_x;
int                       g_nframewin_y;
int                       g_nframewin_posx;
int                       g_nframewin_posy;
bool                      g_bframemax;

bool                      g_bAutoAnchorMark;

wxRect                    g_blink_rect;
double                    g_PlanSpeed;
wxDateTime                g_StartTime;
int                       g_StartTimeTZ;
IDX_entry                 *gpIDX;
int                       gpIDXn;
long                      gStart_LMT_Offset;

wxArrayString             *pMessageOnceArray;

FILE                      *s_fpdebug;
bool                      bAutoOpen;
bool                      bFirstAuto;

bool                      g_bUseRMC;
bool                      g_bUseGLL;

int                       g_nCacheLimit;
int                       g_memCacheLimit;
bool                      g_bGDAL_Debug;

double                    g_VPRotate; // Viewport rotation angle, used on "Course Up" mode
bool                      g_bCourseUp;
int                       g_COGAvgSec; // COG average period (sec.) for Course Up Mode
double                    g_COGAvg;
bool                      g_bLookAhead;
bool                      g_bskew_comp;
bool                      g_bopengl;
bool                      g_bsmoothpanzoom;

int                       g_nCOMPortCheck;

bool                      g_bbigred;

PlugInManager             *g_pi_manager;

bool                      g_bAISRolloverShowClass;
bool                      g_bAISRolloverShowCOG;
bool                      g_bAISRolloverShowCPA;

bool                      g_bDebugGPSD;

bool                      g_bFullScreenQuilt;
bool                      g_bQuiltEnable;
bool                      g_bQuiltStart;

bool                      g_bportable;

ChartGroupArray           *g_pGroupArray;
int                       g_GroupIndex;

wxString                  g_GPS_Ident;

wxProgressDialog          *s_ProgDialog;

S57QueryDialog            *g_pObjectQueryDialog;

wxArrayString             TideCurrentDataSet;
wxString                  g_TCData_Dir;

#ifndef __WXMSW__
struct sigaction          sa_all;
struct sigaction          sa_all_old;
#endif

bool GetMemoryStatus(int *mem_total, int *mem_used);

#ifdef __WXMSW__
// System color control support

typedef DWORD (WINAPI *SetSysColors_t)(DWORD, DWORD *, DWORD *);
typedef DWORD (WINAPI *GetSysColor_t)(DWORD);

SetSysColors_t            pSetSysColors;
GetSysColor_t             pGetSysColor;

void SaveSystemColors(void);
void RestoreSystemColors(void);

DWORD                     color_3dface;
DWORD                     color_3dhilite;
DWORD                     color_3dshadow;
DWORD                     color_3ddkshadow;
DWORD                     color_3dlight;
DWORD                     color_activecaption;
DWORD                     color_gradientactivecaption;
DWORD                     color_captiontext;
DWORD                     color_windowframe;
DWORD                     color_inactiveborder;

#endif

// AIS Global configuration
bool                      g_bShowAIS;
bool                      g_bCPAMax;
double                    g_CPAMax_NM;
bool                      g_bCPAWarn;
double                    g_CPAWarn_NM;
bool                      g_bTCPA_Max;
double                    g_TCPA_Max;
bool                      g_bMarkLost;
double                    g_MarkLost_Mins;
bool                      g_bRemoveLost;
double                    g_RemoveLost_Mins;
bool                      g_bShowCOG;
double                    g_ShowCOG_Mins;
bool                      g_bAISShowTracks;
double                    g_AISShowTracks_Mins;
bool                      g_bShowMoored;
double                    g_ShowMoored_Kts;
wxString                  g_sAIS_Alert_Sound_File;
bool                      g_bAIS_CPA_Alert_Suppress_Moored;
bool                      g_bAIS_ACK_Timeout;
double                    g_AckTimeout_Mins;
bool                      g_bShowAreaNotices;
bool                      g_bDrawAISSize;

wxToolBarToolBase         *m_pAISTool;
int                       g_nAIS_activity_timer;

DummyTextCtrl             *g_pDummyTextCtrl;
bool                      g_bEnableZoomToCursor;

bool                      g_bTrackActive;
bool                      g_bTrackCarryOver;
bool                      g_bTrackDaily;
bool                      g_bHighliteTracks;
int                       g_route_line_width;
int                       g_track_line_width;
wxString                  g_default_wp_icon;

Track                     *g_pActiveTrack;
double                    g_TrackIntervalSeconds;
double                    g_TrackDeltaDistance;
int                       g_nTrackPrecision;

int                       g_total_NMEAerror_messages;

int                       g_cm93_zoom_factor;
CM93DSlide                *pCM93DetailSlider;
bool                      g_bShowCM93DetailSlider;
int                       g_cm93detail_dialog_x, g_cm93detail_dialog_y;

bool                      g_bUseGreenShip;

wxString                  g_AW1GUID;
wxString                  g_AW2GUID;

bool                      g_b_overzoom_x; // Allow high overzoom
bool                      g_bshow_overzoom_emboss;

int                      g_OwnShipIconType;
double                    g_n_ownship_length_meters;
double                    g_n_ownship_beam_meters;
double                    g_n_gps_antenna_offset_y;
double                    g_n_gps_antenna_offset_x;
long                      g_n_ownship_min_mm;

int                       g_nautosave_interval_seconds;

bool                      g_bPreserveScaleOnX;

about                     *g_pAboutDlg;

wxPlatformInfo            *g_pPlatform;
wxLocale                  *plocale_def_lang;
wxString                  g_locale;
bool                      g_b_assume_azerty;

TTYWindow                 *g_NMEALogWindow;
int                       g_NMEALogWindow_x, g_NMEALogWindow_y;
int                       g_NMEALogWindow_sx, g_NMEALogWindow_sy;

bool                      g_bUseRaster;
bool                      g_bUseVector;
bool                      g_bUseCM93;

int                       g_click_stop;

int                       g_MemFootSec;
int                       g_MemFootMB;

ArrayOfInts               g_quilt_noshow_index_array;

wxStaticBitmap            *g_pStatBoxTool;

bool                      g_bquiting;
int                       g_BSBImgDebug;

AISTargetListDialog       *g_pAISTargetList;
wxString                  g_AisTargetList_perspective;
int                       g_AisTargetList_range;
int                       g_AisTargetList_sortColumn;
bool                      g_bAisTargetList_sortReverse;
wxString                  g_AisTargetList_column_spec;
int                       g_AisTargetList_count;

wxAuiManager              *g_pauimgr;
wxAuiDefaultDockArt       *g_pauidockart;

bool                      g_blocale_changed;

wxMenu                    *g_FloatingToolbarConfigMenu;
wxString                  g_toolbarConfig = _T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
ocpnFloatingToolbarDialog *g_FloatingToolbarDialog;
ocpnFloatingCompassWindow *g_FloatingCompassDialog;

int                       g_toolbar_x;
int                       g_toolbar_y;
long                      g_toolbar_orient;

MyDialogPtrArray          g_MacShowDialogArray;

OCPNBitmapDialog          *g_pbrightness_indicator_dialog;
int                       g_brightness_timeout;

//                        OpenGL Globals
int                       g_GPU_MemSize;
bool                      g_b_useStencil;

char bells_sound_file_name[8][12] =    // pjotrc 2010.02.09

        { "1bells.wav", "2bells.wav", "3bells.wav", "4bells.wav", "5bells.wav", "6bells.wav",
                "7bells.wav", "8bells.wav"   // pjotrc 2010.02.09

        };

static char nmea_tick_chars[] = { '|', '/', '-', '\\', '|', '/', '-', '\\' };
static int tick_idx;

int               g_sticky_chart;

extern wxString OpenCPNVersion; //Gunther

double g_GLMinLineWidth;

int n_NavMessageShown;
wxString g_config_version_string;

#ifndef __WXMSW__
sigjmp_buf env;                    // the context saved by sigsetjmp();
#endif

// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
#ifdef  __WXMSW__
DEFINE_GUID( GARMIN_DETECT_GUID, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7,
        0x22, 0xc0 );
#endif

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

//    Some static helpers
void appendOSDirSlash( wxString* pString );
void InitializeUserColors( void );
void DeInitializeUserColors( void );
void SetSystemColors( ColorScheme cs );

DEFINE_EVENT_TYPE(EVT_THREADMSG)

//------------------------------------------------------------------------------
//    PNG Icon resources
//------------------------------------------------------------------------------

#ifdef __WXGTK__
#include "bitmaps/opencpn.xpm"
#endif

//------------------------------------------------------------------------------
//              Local constants
//------------------------------------------------------------------------------
enum {
    ID_PIANO_DISABLE_QUILT_CHART = 50000, ID_PIANO_ENABLE_QUILT_CHART
};

//------------------------------------------------------------------------------
//              Fwd Refs
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------
//      Signal Handlers
//-----------------------------------------------------------------------
#ifndef __WXMSW__

//These are the signals possibly expected
//      SIGUSR1
//      Raised externally to cause orderly termination of application
//      Intended to act just like pushing the "EXIT" button

//      SIGSEGV
//      Some undefined segfault......

void
catch_signals(int signo)
{
    switch(signo)
    {
        case SIGUSR1:
        quitflag++;                             // signal to the timer loop
        break;

        case SIGSEGV:
        siglongjmp(env, 1);// jump back to the setjmp() point
        break;

        default:
        break;
    }

}
#endif

int ShowNavWarning()
{
    wxString msg0(
            _("\n\
OpenCPN is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied\n\
warranty of MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.\n\
See the GNU General Public License for more details.\n\n\
OpenCPN must only be used in conjunction with approved\n\
paper charts and traditional methods of navigation.\n\n\
DO NOT rely upon OpenCPN for safety of life or property.\n\n\
Please click \"OK\" to agree and proceed, \"Cancel\" to quit.\n") );

    wxString vs = wxT(" .. Version ") + str_version_major + wxT(".") + str_version_minor + wxT(".")
            + str_version_patch;

    wxMessageDialog odlg( gFrame, msg0, _("Welcome to OpenCPN") + vs, wxCANCEL | wxOK );

    return ( odlg.ShowModal() );
}

// `Main program' equivalent, creating windows and returning main app frame
//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------

IMPLEMENT_APP( MyApp )
BEGIN_EVENT_TABLE(MyApp, wxApp) EVT_ACTIVATE_APP(MyApp::OnActivateApp)
END_EVENT_TABLE()

#include "wx/dynlib.h"

void MyApp::OnInitCmdLine( wxCmdLineParser& parser )
{
    //    Add some OpenCPN specific command line options
    parser.AddSwitch( _T("unit_test_1") );

    parser.AddSwitch( _T("p") );
}

bool MyApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
    g_unit_test_1 = parser.Found( _T("unit_test_1") );
    g_bportable = parser.Found( _T("p") );

    return true;
}

void MyApp::OnActivateApp( wxActivateEvent& event )
{
//    Code carefully in this method.
//    It is called in some unexpected places,
//    such as on closure of dialogs, etc.

//      Activating?

#ifdef __WXOSX__

//      On the Mac, this method gets hit when...
//      a) switching between apps by clicking title bars, coming and going
//      b) un-iconizing, activeate only/
//      It does NOT get hit on iconizing the app
    if(!event.GetActive())
    {
//        printf("App de-activate\n");
    }
    else
    {
//        printf("App Activate\n");
    }
#endif

    if( !event.GetActive() ) {
        if( g_FloatingToolbarDialog ) g_FloatingToolbarDialog->HideTooltip(); // Hide any existing tip
    }

    event.Skip();
}

bool MyApp::OnInit()
{
    if( !wxApp::OnInit() ) return false;

    g_pPlatform = new wxPlatformInfo;

    //    On MSW, force the entire process to run on one CPU core only
    //    This resolves some difficulty with wxThread syncronization
#ifdef __WXMSW__
    //Gets the current process handle
    HANDLE hProc = GetCurrentProcess();
    DWORD procMask;
    DWORD sysMask;
    HANDLE hDup;
    DuplicateHandle( hProc, hProc, hProc, &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS );

//Gets the current process affinity mask
    GetProcessAffinityMask( hDup, &procMask, &sysMask );

// Take a simple approach, and assume up to 4 processors
    DWORD newMask;
    if( ( procMask & 1 ) == 1 ) newMask = 1;
    else
        if( ( procMask & 2 ) == 2 ) newMask = 2;
        else
            if( ( procMask & 4 ) == 4 ) newMask = 4;
            else
                if( ( procMask & 8 ) == 8 ) newMask = 8;

//Set te affinity mask for the process
    BOOL res = SetProcessAffinityMask( hDup, (DWORD_PTR) newMask );

    if( res == 0 ) {
        //Error setting affinity mask!!
    }
#endif

//Fulup: force floating point to use dot as separation.
// This needs to be set early to catch numerics in config file.
//#ifdef __POSIX__
    setlocale( LC_NUMERIC, "C" );
//#endif

//      CALLGRIND_STOP_INSTRUMENTATION

    g_start_time = wxDateTime::Now();

    g_loglast_time = g_start_time;   // pjotrc 2010.02.09
    g_loglast_time.MakeGMT();        // pjotrc 2010.02.09
    g_loglast_time.Subtract( wxTimeSpan( 0, 29, 0, 0 ) ); // give 1 minute for GPS to get a fix   // pjotrc 2010.02.09

    AnchorPointMinDist = 5.0;

#ifdef __WXMSW__

    //    Handle any Floating Point Exceptions which may leak thru from other
    //    processes.  The exception filter is in cutil.c
    //    Seems to only happen for W98

    if( g_pPlatform->GetOperatingSystemId() == wxOS_WINDOWS_9X ) SetUnhandledExceptionFilter (&MyUnhandledExceptionFilter);
#endif

#ifdef __WXMSW__
//     _CrtSetBreakAlloc(141542);
#endif

#ifndef __WXMSW__
//      Setup Linux SIGNAL handling, for external program control

//      Build the sigaction structure
    sa_all.sa_handler = catch_signals;// point to my handler
    sigemptyset(&sa_all.sa_mask);// make the blocking set
                                 // empty, so that all
                                 // other signals will be
                                 // unblocked during my handler
    sa_all.sa_flags = 0;

    sigaction(SIGUSR1, NULL, &sa_all_old);// save existing action for this signal

//      Register my request for some signals
    sigaction(SIGUSR1, &sa_all, NULL);

    sigaction(SIGUSR1, NULL, &sa_all_old);// inspect existing action for this signal
#endif

//      Initialize memory tracer
#ifndef __WXMSW__
//        mtrace();
#endif

//      Here is some experimental code for wxTheme support
//      Not also these lines included above....
//      They are necessary to ensure that the themes are statically loaded

//      #ifdef __WXUNIVERSAL__
//      WX_USE_THEME(gtk);
//      WX_USE_THEME(Metal);
//      #endif

#ifdef __WXUNIVERSAL__
//        wxTheme* theme = wxTheme::Create("gtk");
//        wxTheme::Set(theme);
#endif

//      Init the private memory manager
    malloc_max = 0;

    //      Record initial memory status
    GetMemoryStatus( &g_mem_total, &g_mem_initial );

//      wxHandleFatalExceptions(true);

// Set up default FONT encoding, which should have been done by wxWidgets some time before this......
    wxFont temp_font( 10, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, wxString( _T("") ),
            wxFONTENCODING_SYSTEM );
    temp_font.SetDefaultEncoding( wxFONTENCODING_SYSTEM );

//  Init my private font manager
    pFontMgr = new FontMgr();

//      Establish a "home" location
    wxStandardPathsBase& std_path = wxApp::GetTraits()->GetStandardPaths();
    std_path.Get();

    pHome_Locn = new wxString;
#ifdef __WXMSW__
    pHome_Locn->Append( std_path.GetConfigDir() );   // on w98, produces "/windows/Application Data"
#else
            pHome_Locn->Append(std_path.GetUserConfigDir());
#endif

    if( g_bportable ) {
        pHome_Locn->Clear();
        wxFileName f( std_path.GetExecutablePath() );
        pHome_Locn->Append( f.GetPath() );
    }

    appendOSDirSlash( pHome_Locn );

    //      Establish Log File location
    glog_file = *pHome_Locn;

#ifdef  __WXOSX__
    pHome_Locn->Append(_T("opencpn"));
    appendOSDirSlash(pHome_Locn);

    wxFileName LibPref(glog_file);          // starts like "~/Library/Preferences"
    LibPref.RemoveLastDir();// takes off "Preferences"

    glog_file = LibPref.GetFullPath();
    appendOSDirSlash(&glog_file);

    glog_file.Append(_T("Logs/"));// so, on OS X, opencpn.log ends up in ~/Library/Logs
                                  // which makes it accessible to Applications/Utilities/Console....
#endif

    // create the opencpn "home" directory if we need to
    wxFileName wxHomeFiledir( *pHome_Locn );
    if( true != wxHomeFiledir.DirExists( wxHomeFiledir.GetPath() ) ) if( !wxHomeFiledir.Mkdir(
            wxHomeFiledir.GetPath() ) ) {
        wxASSERT_MSG(false,_T("Cannot create opencpn home directory"));
        return false;
    }

    // create the opencpn "log" directory if we need to
    wxFileName wxLogFiledir( glog_file );
    if( true != wxLogFiledir.DirExists( wxLogFiledir.GetPath() ) ) {
        if( !wxLogFiledir.Mkdir( wxLogFiledir.GetPath() ) ) {
            wxASSERT_MSG(false,_T("Cannot create opencpn log directory"));
            return false;
        }
    }
    glog_file.Append( _T("opencpn.log") );

    //  Constrain the size of the log file
    if( ::wxFileExists( glog_file ) ) {
        if( wxFileName::GetSize( glog_file ) > 1000000 ) {
            wxString oldlog = glog_file;                      // pjotrc 2010.02.09
            oldlog.Append( _T(".log") );
            wxString msg1( _T("Old log will be moved to opencpn.log.log") );
            OCPNMessageBox ( NULL, msg1, wxString( _("OpenCPN Info") ),
                    wxICON_INFORMATION | wxOK );
//            int dlg_ret;
//            dlg_ret = mdlg.ShowModal();
            ::wxRenameFile( glog_file, oldlog );
        }
    }

    flog = fopen( glog_file.mb_str(), "a" );
    logger = new wxLogStderr( flog );

    Oldlogger = wxLog::SetActiveTarget( logger );

#ifdef __WXMSW__

//  Un-comment the following to establish a separate console window as a target for printf() in Windows   
//     RedirectIOToConsole();

#endif
    
//        wxLog::AddTraceMask("timer");               // verbose message traces to log output

#ifndef __WXMSW__
    logger->SetTimestamp(_T("%H:%M:%S %Z"));
#endif

//      Send init message
    wxLogMessage( _T("\n\n________\n") );

    wxDateTime date_now = wxDateTime::Now();

    wxString imsg = date_now.FormatISODate();
    wxLogMessage( imsg );

    imsg = _T(" ------- Starting OpenCPN -------");
    wxLogMessage( imsg );

    wxString version = OpenCPNVersion;
    wxString vs = version.Trim( true );
    vs = vs.Trim( false );
    wxLogMessage( vs );

    wxString wxver(wxVERSION_STRING);
    wxver.Prepend( _T("wxWidgets version: ") );
    wxLogMessage( wxver );

    wxLogMessage( _T("MemoryStatus:  mem_total: %d mb,  mem_initial: %d mb"), g_mem_total / 1024,
            g_mem_initial / 1024 );

    //    Initialize embedded PNG icon graphics
    ::wxInitAllImageHandlers();

//      Establish a "shared data" location
    /*  From the wxWidgets documentation...

     wxStandardPaths::GetDataDir
     wxString GetDataDir() const
     Return the location of the applications global, i.e. not user-specific, data files.
     * Unix: prefix/share/appname
     * Windows: the directory where the executable file is located
     * Mac: appname.app/Contents/SharedSupport bundle subdirectory
     */
    g_SData_Locn = std_path.GetDataDir();
    appendOSDirSlash( &g_SData_Locn );

    if( g_bportable ) g_SData_Locn = *pHome_Locn;

    imsg = _T("SData_Locn is ");
    imsg += g_SData_Locn;
    wxLogMessage( imsg );

//      Create some static strings
    pInit_Chart_Dir = new wxString();

    //  Establish an empty ChartCroupArray
    g_pGroupArray = new ChartGroupArray;

    //      Establish the prefix of the location of user specific data files
#ifdef __WXMSW__
    g_PrivateDataDir = *pHome_Locn;                     // should be {Documents and Settings}\......
#elif defined __WXOSX__
            g_PrivateDataDir = std_path.GetUserConfigDir();     // should be ~/Library/Preferences
#else
            g_PrivateDataDir = std_path.GetUserDataDir();       // should be ~/.opencpn
#endif

    if( g_bportable ) g_PrivateDataDir = *pHome_Locn;

    //  Get the PlugIns directory location
    g_Plugin_Dir = std_path.GetPluginsDir();   // linux:   {prefix}/lib/opencpn
                                               // Mac:     appname.app/Contents/PlugIns
#ifdef __WXMSW__
    g_Plugin_Dir += _T("\\plugins");             // Windows: {exe dir}/plugins
#endif

    if( g_bportable ) {
        g_Plugin_Dir = *pHome_Locn;
        g_Plugin_Dir += _T("plugins");
    }

//      Create an array string to hold repeating messages, so they don't
//      overwhelm the log
    pMessageOnceArray = new wxArrayString;

//      Init the Route Manager
    g_pRouteMan = new Routeman( this );

//      Init the Selectable Route Items List
    pSelect = new Select();

//      Init the Selectable Tide/Current Items List
    pSelectTC = new Select();

//      Init the Selectable AIS Target List
    pSelectAIS = new Select();

//      Initially AIS display is always on
    g_bShowAIS = true;
    g_pais_query_dialog_active = NULL;

//      Who am I?
    phost_name = new wxString( ::wxGetHostName() );

//      (Optionally) Capture the user and file(effective) ids
//  Some build environments may need root privileges for hardware
//  port I/O, as in the NMEA data input class.  Set that up here.

#ifndef __WXMSW__
#ifdef PROBE_PORTS__WITH_HELPER
    user_user_id = getuid ();
    file_user_id = geteuid ();
#endif
#endif

//      Establish the location of the config file
#ifdef __WXMSW__
    gConfig_File = _T("opencpn.ini");
    gConfig_File.Prepend( *pHome_Locn );

#elif defined __WXOSX__
    gConfig_File = std_path.GetUserConfigDir(); // should be ~/Library/Preferences
    appendOSDirSlash(&gConfig_File);
    gConfig_File.Append(_T("opencpn.ini"));
#else
    gConfig_File = std_path.GetUserDataDir(); // should be ~/.opencpn
    appendOSDirSlash(&gConfig_File);
    gConfig_File.Append(_T("opencpn.conf"));
#endif

    if( g_bportable ) {
        gConfig_File = *pHome_Locn;
#ifdef __WXMSW__
        gConfig_File += _T("opencpn.ini");
#elif defined __WXOSX__
        gConfig_File +=_T("opencpn.ini");
#else
        gConfig_File += _T("opencpn.conf");
#endif

    }

    bool b_novicemode = false;

    wxFileName config_test_file_name( gConfig_File );
    if( config_test_file_name.FileExists() ) wxLogMessage(
            _T("Using existing Config_File: ") + gConfig_File );
    else {
        {
            wxLogMessage( _T("Creating new Config_File: ") + gConfig_File );

            //    Flag to preset some options for initial config file creation
            b_novicemode = true;

            if( true != config_test_file_name.DirExists( config_test_file_name.GetPath() ) ) if( !config_test_file_name.Mkdir(
                    config_test_file_name.GetPath() ) ) wxLogMessage(
                    _T("Cannot create config file directory for ") + gConfig_File );
        }
    }

    // Now initialize UI Style.
    g_StyleManager = new ocpnStyle::StyleManager();

    if( !g_StyleManager->IsOK() ) {
        wxString msg = _("Failed to initialize the user interface. ");
        msg << _("OpenCPN cannot start. ");
        msg << _("The necessary configuration files were not found. ");
        msg << _("See the log file at ") << glog_file << _(" for details.");
        wxMessageDialog w( NULL, msg, _("Failed to initialize the user interface. "),
                wxCANCEL | wxICON_ERROR );
        w.ShowModal();
        exit( EXIT_FAILURE );
    }

    //      Init the WayPoint Manager (Must be after UI Style init).
    pWayPointMan = new WayPointman();
    pWayPointMan->ProcessIcons( g_StyleManager->GetCurrentStyle() );

    //      Open/Create the Config Object (Must be after UI Style init).
    MyConfig *pCF = new MyConfig( wxString( _T("") ), wxString( _T("") ), gConfig_File );
    pConfig = (MyConfig *) pCF;
    pConfig->LoadMyConfig( 0 );

    //        Is this the first run after a clean install?
    if( !n_NavMessageShown ) g_bFirstRun = true;

    //  Now we can set the locale

    //    Manage internationalization of embedded messages
    //    using wxWidgets/gettext methodology....

//        wxLog::SetVerbose(true);            // log all messages for debugging

    if( lang_list[0] ) {
    };                 // silly way to avoid compiler warnings

    // Add a new prefix for search order.
#ifdef __WXMSW__
    wxString locale_location = g_SData_Locn;
    locale_location += _T("share/locale");
    wxLocale::AddCatalogLookupPathPrefix( locale_location );
#endif

    //  Get the default language info
    wxString def_lang_canonical;
    const wxLanguageInfo* languageInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
    if( languageInfo ) {
        def_lang_canonical = languageInfo->CanonicalName;
        imsg = _T("System default Language:  ");
        imsg += def_lang_canonical;
        wxLogMessage( imsg );
    }

    //  For windows, installer may have left information in the registry defining the
    //  user's selected install language.
    //  If so, override the config file value and use this selection for opencpn...
#ifdef __WXMSW__
    if( g_bFirstRun ) {
        wxRegKey RegKey( wxString( _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN") ) );
        if( RegKey.Exists() ) {
            wxLogMessage( _("Retrieving initial language selection from Windows Registry") );
            RegKey.QueryValue( wxString( _T("InstallerLanguage") ), g_locale );
        }
    }
#endif
    //  Find the language specified by the config file
    const wxLanguageInfo *pli = wxLocale::FindLanguageInfo( g_locale );
    wxString loc_lang_canonical;
    bool b_initok;
    plocale_def_lang = new wxLocale;

    if( pli ) {
        b_initok = plocale_def_lang->Init( pli->Language, 0 );
        loc_lang_canonical = pli->CanonicalName;
    }

    if( !pli || !b_initok ) {
        delete plocale_def_lang;
        plocale_def_lang = new wxLocale;
        plocale_def_lang->Init( wxLANGUAGE_ENGLISH_US, 0 );
        loc_lang_canonical = wxLocale::GetLanguageInfo( wxLANGUAGE_ENGLISH_US )->CanonicalName;
    }

    imsg = _T("Opencpn language set to:  ");
    imsg += loc_lang_canonical;
    wxLogMessage( imsg );

    // Set filename without extension (example : opencpn_fr_FR)
    // i.e. : Set-up the filename needed for translation
//        wxString loc_lang_filename = _T("opencpn_") + loc_lang_canonical;
    wxString loc_lang_filename = _T("opencpn");

    // Get translation file (example : opencpn_fr_FR.mo)
    // No problem if the file doesn't exist
    // as this case is handled by wxWidgets
    if( plocale_def_lang ) plocale_def_lang->AddCatalog( loc_lang_filename );

    //    Always use dot as decimal
    setlocale( LC_NUMERIC, "C" );

    wxLog::SetVerbose( false );           // log no verbose messages

    //  French language locale is assumed to include the AZERTY keyboard
    //  This applies to either the system language, or to OpenCPN language selection
    if( loc_lang_canonical == _T("fr_FR") ) g_b_assume_azerty = true;
    if( def_lang_canonical == _T("fr_FR") ) g_b_assume_azerty = true;

//  Send the Welcome/warning message if it has never been sent before,
//  or if the version string has changed at all
//  We defer until here to allow for localization of the message
    if( !n_NavMessageShown || ( vs != g_config_version_string ) ) {
        if( wxID_CANCEL == ShowNavWarning() ) return false;
        n_NavMessageShown = 1;
    }

    g_config_version_string = vs;

 #ifdef USE_S57

//      Set up a useable CPL library error handler for S57 stuff
    CPLSetErrorHandler( MyCPLErrorHandler );

//      Init the s57 chart object, specifying the location of the required csv files
    g_csv_locn = g_SData_Locn;
    g_csv_locn.Append( _T("s57data") );

    if( g_bportable ) {
        g_csv_locn = _T(".");
        appendOSDirSlash( &g_csv_locn );
        g_csv_locn.Append( _T("s57data") );
    }

//      If the config file contains an entry for SENC file prefix, use it.
//      Otherwise, default to PrivateDataDir
    if( g_SENCPrefix.IsEmpty() ) {
        g_SENCPrefix = g_PrivateDataDir;
        appendOSDirSlash( &g_SENCPrefix );
        g_SENCPrefix.Append( _T("SENC") );
    }

    if( g_bportable ) {
        wxFileName f( g_SENCPrefix );
        if( f.MakeRelativeTo( g_PrivateDataDir ) ) g_SENCPrefix = f.GetFullPath();
        else
            g_SENCPrefix = _T("SENC");
    }

//      If the config file contains an entry for PresentationLibraryData, use it.
//      Otherwise, default to conditionally set spot under g_pcsv_locn
    wxString plib_data;
    bool b_force_legacy = false;

    if( g_UserPresLibData.IsEmpty() ) {
        plib_data = g_csv_locn;
        appendOSDirSlash( &plib_data );
        plib_data.Append( _T("S52RAZDS.RLE") );
    } else {
        plib_data = g_UserPresLibData;
        b_force_legacy = true;
    }

    ps52plib = new s52plib( plib_data, b_force_legacy );

    //  If the library load failed, try looking for the s57 data elsewhere

    //  First, look in UserDataDir
    /*    From wxWidgets documentation

     wxStandardPaths::GetUserDataDir
     wxString GetUserDataDir() const
     Return the directory for the user-dependent application data files:
     * Unix: ~/.appname
     * Windows: C:\Documents and Settings\username\Application Data\appname
     * Mac: ~/Library/Application Support/appname
     */

    if( !ps52plib->m_bOK ) {
        delete ps52plib;

        wxString look_data_dir;
        look_data_dir.Append( std_path.GetUserDataDir() );
        appendOSDirSlash( &look_data_dir );
        wxString tentative_SData_Locn = look_data_dir;
        look_data_dir.Append( _T("s57data") );

        plib_data = look_data_dir;
        appendOSDirSlash( &plib_data );
        plib_data.Append( _T("S52RAZDS.RLE") );

        wxLogMessage( _T("Looking for s57data in ") + look_data_dir );
        ps52plib = new s52plib( plib_data );

        if( ps52plib->m_bOK ) {
            g_csv_locn = look_data_dir;
            g_SData_Locn = tentative_SData_Locn;
        }
    }

    //  And if that doesn't work, look again in the original SData Location
    //  This will cover the case in which the .ini file entry is corrupted or moved

    if( !ps52plib->m_bOK ) {
        delete ps52plib;

        wxString look_data_dir;
        look_data_dir = g_SData_Locn;
        look_data_dir.Append( _T("s57data") );

        plib_data = look_data_dir;
        appendOSDirSlash( &plib_data );
        plib_data.Append( _T("S52RAZDS.RLE") );

        wxLogMessage( _T("Looking for s57data in ") + look_data_dir );
        ps52plib = new s52plib( plib_data );

        if( ps52plib->m_bOK ) g_csv_locn = look_data_dir;
    }

    if( ps52plib->m_bOK ) wxLogMessage( _T("Using s57data in ") + g_csv_locn );
    else
        wxLogMessage( _T("   S52PLIB Initialization failed, disabling Vector charts.") );

// Todo Maybe initialize only when an s57 chart is actually opened???
    if( ps52plib->m_bOK ) m_pRegistrarMan = new s57RegistrarMgr( g_csv_locn, flog );

    if( !ps52plib->m_bOK ) {
        delete ps52plib;
        ps52plib = NULL;
    }

#endif  // S57
// Set default color scheme
    global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;

    // On Windows platforms, establish a default cache managment policy
    // as allowing OpenCPN a percentage of available physical memory,
    // not to exceed 1 GB
    // Note that this logic implies that Windows platforms always use
    // the memCacheLimit policy, and never use the fallback nCacheLimit policy
#ifdef __WXMSW__
if( 0 == g_memCacheLimit )
    g_memCacheLimit = (int) ( g_mem_total * 0.5 );
    g_memCacheLimit = wxMin(g_memCacheLimit, 1024 * 1024); // math in kBytes
#endif

//      Establish location and name of chart database
#ifdef __WXMSW__
    pChartListFileName = new wxString( _T("CHRTLIST.DAT") );
    pChartListFileName->Prepend( *pHome_Locn );

#else
    pChartListFileName = new wxString(_T(""));
    pChartListFileName->Append(std_path.GetUserDataDir());
    appendOSDirSlash(pChartListFileName);
    pChartListFileName->Append(_T("chartlist.dat"));
#endif

    if( g_bportable ) {
        pChartListFileName->Clear();
#ifdef __WXMSW__
        pChartListFileName->Append( _T("CHRTLIST.DAT") );
#else
        pChartListFileName->Append(_T("chartlist.dat"));
#endif
        pChartListFileName->Prepend( *pHome_Locn );
    }

//      Establish guessed location of chart tree
    if( pInit_Chart_Dir->IsEmpty() ) {
        pInit_Chart_Dir->Append( std_path.GetDocumentsDir() );
    }

//      Establish the GSHHS Dataset location
    pWorldMapLocation = new wxString( _T("gshhs") );
    pWorldMapLocation->Prepend( g_SData_Locn );
    pWorldMapLocation->Append( wxFileName::GetPathSeparator() );

//      Reload the config data, to pick up any missing data class configuration info
//      e.g. s52plib, which could not be created until first config load completes
//      Think catch-22
    pConfig->LoadMyConfig( 1 );

    //  Override some config options for initial user startup with empty config file
    if( b_novicemode ) {
        g_bShowOutlines = true;

        g_CPAMax_NM = 20.;
        g_CPAWarn_NM = 2.;
        g_TCPA_Max = 30.;
        g_bMarkLost = true;
        ;
        g_MarkLost_Mins = 8;
        g_bRemoveLost = true;
        g_RemoveLost_Mins = 10;
        g_bShowCOG = true;
        g_ShowCOG_Mins = 6;
        g_bShowMoored = true;
        g_ShowMoored_Kts = 0.2;
        g_bTrackDaily = false;
        g_PlanSpeed = 6.;
        g_bFullScreenQuilt = true;
        g_bQuiltEnable = true;
        g_bskew_comp = false;
        g_bShowAreaNotices = false;
        g_bDrawAISSize = false;

#ifdef USE_S57
        if( ps52plib && ps52plib->m_bOK ) {
            ps52plib->m_bShowSoundg = true;
            ps52plib->m_nDisplayCategory = (enum _DisCat) STANDARD;
            ps52plib->m_nSymbolStyle = (LUPname) PAPER_CHART;
            ps52plib->m_nBoundaryStyle = (LUPname) PLAIN_BOUNDARIES;
            ps52plib->m_bUseSCAMIN = true;
            ps52plib->m_bShowAtonText = true;

            //    Preset some object class visibilites for "Mariner's Standard" disply category
            for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
                OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
                if( !strncmp( pOLE->OBJLName, "DEPARE", 6 ) ) pOLE->nViz = 1;
                if( !strncmp( pOLE->OBJLName, "LNDARE", 6 ) ) pOLE->nViz = 1;
                if( !strncmp( pOLE->OBJLName, "COALNE", 6 ) ) pOLE->nViz = 1;
            }

        }
#endif

    }

    //  Check the global Tide/Current data source array
    //  If empty, preset one default (US) Ascii data source
    if(!TideCurrentDataSet.GetCount())
        TideCurrentDataSet.Add( g_SData_Locn +
            _T("tcdata") +
            wxFileName::GetPathSeparator() +
            _T("HARMONIC.IDX"));



    g_StartTime = wxInvalidDateTime;
    g_StartTimeTZ = 1;				// start with local times
    gpIDX = NULL;
    gpIDXn = 0;

//  Set up the frame initial visual parameters
//      Default size, resized later
    wxSize new_frame_size( -1, -1 );
    int cx, cy, cw, ch;
    ::wxClientDisplayRect( &cx, &cy, &cw, &ch );

    InitializeUserColors();

    if( ( g_nframewin_x > 100 ) && ( g_nframewin_y > 100 ) && ( g_nframewin_x <= cw )
            && ( g_nframewin_y <= ch ) ) new_frame_size.Set( g_nframewin_x, g_nframewin_y );
    else
        new_frame_size.Set( cw * 7 / 10, ch * 7 / 10 );

    //  Try to detect any change in physical screen configuration
    //  This can happen when drivers are changed, for instance....
    //  and can confuse the WUI layout perspective stored in the config file.
    //  If detected, force a nominal window size and position....
    if( ( g_lastClientRectx != cx ) || ( g_lastClientRecty != cy ) || ( g_lastClientRectw != cw )
            || ( g_lastClientRecth != ch ) ) {
        new_frame_size.Set( cw * 7 / 10, ch * 7 / 10 );
        g_bframemax = false;
    }

    g_lastClientRectx = cx;
    g_lastClientRecty = cy;
    g_lastClientRectw = cw;
    g_lastClientRecth = ch;

    //  Validate config file position
    wxPoint position( 0, 0 );
    wxSize dsize = wxGetDisplaySize();

#ifdef __WXMAC__
    g_nframewin_posy = wxMax(g_nframewin_posy, 22);
#endif

    if( ( g_nframewin_posx < dsize.x ) && ( g_nframewin_posy < dsize.y ) ) position = wxPoint(
            g_nframewin_posx, g_nframewin_posy );

#ifdef __WXMSW__
    //  Support MultiMonitor setups which an allow negative window positions.
    RECT frame_rect;
    frame_rect.left = position.x;
    frame_rect.top = position.y;
    frame_rect.right = position.x + new_frame_size.x;
    frame_rect.bottom = position.y + new_frame_size.y;

    //  If the requested frame window does not intersect any installed monitor,
    //  then default to simple primary monitor positioning.
    if( NULL == MonitorFromRect( &frame_rect, MONITOR_DEFAULTTONULL ) ) position = wxPoint( 10,
            10 );
#endif

    //  For Windows and GTK, provide the expected application Minimize/Close bar
    long app_style = wxDEFAULT_FRAME_STYLE;
    app_style |= wxWANTS_CHARS;

// Create the main frame window
    wxString myframe_window_title = wxT("OpenCPN ") + str_version_major + wxT(".")
            + str_version_minor + wxT(".") + str_version_patch; //Gunther

    if( g_bportable ) {
        myframe_window_title += _(" -- [Portable(-p) executing from ");
        myframe_window_title += *pHome_Locn;
        myframe_window_title += _T("]");
    }

    gFrame = new MyFrame( NULL, myframe_window_title, position, new_frame_size, app_style ); //Gunther

    g_pauimgr = new wxAuiManager;
//        g_pauidockart= new wxAuiDefaultDockArt;
//        g_pauimgr->SetArtProvider(g_pauidockart);

// tell wxAuiManager to manage the frame
    g_pauimgr->SetManagedWindow( gFrame );

//      Create Children of Frame
//              n.b.  if only one child exists, wxWindows expands the child
//                        to the parent client area automatically, (as a favor?)
//                        Here, we'll do explicit sizing on SIZE events

    cc1 = new ChartCanvas( gFrame );                         // the chart display canvas
    gFrame->SetCanvasWindow( cc1 );

    cc1->SetQuiltMode( g_bQuiltEnable );                     // set initial quilt mode
    cc1->m_bFollow = pConfig->st_bFollow;               // set initial state
    cc1->SetViewPoint( vLat, vLon, initial_scale_ppm, 0., 0. );

    gFrame->Enable();

    cc1->SetFocus();

    console = new ConsoleCanvas( gFrame );                    // the console

    pthumbwin = new ThumbWin( cc1 );

    gFrame->ApplyGlobalSettings( 1, false );               // done once on init with resize

    g_toolbar_x = wxMax(g_toolbar_x, 0);
    g_toolbar_y = wxMax(g_toolbar_y, 0);

    g_toolbar_x = wxMin(g_toolbar_x, cw);
    g_toolbar_y = wxMin(g_toolbar_y, ch);

    g_FloatingToolbarDialog = new ocpnFloatingToolbarDialog( cc1,
            wxPoint( g_toolbar_x, g_toolbar_y ), g_toolbar_orient );
    g_FloatingToolbarDialog->LockPosition(true);

    gFrame->SetAndApplyColorScheme( global_color_scheme );

    //  The position and size of the static frame children (i.e. the canvas, and the status bar) are now set
    //  So now we can establish the AUI panes for them.
    //  It is important to have set the chartcanvas and status bar sizes before this point,
    //  so that the pane.BestSize values are correctly captured by the AuiManager.

    g_pauimgr->AddPane( cc1 );
    g_pauimgr->GetPane( cc1 ).Name( _T("ChartCanvas") );
    g_pauimgr->GetPane( cc1 ).Fixed();
    g_pauimgr->GetPane( cc1 ).CaptionVisible( false );
    g_pauimgr->GetPane( cc1 ).CenterPane();
    g_pauimgr->GetPane( cc1 ).BestSize( cc1->GetSize() );

//      Load and initialize any PlugIns
    g_pi_manager = new PlugInManager( gFrame );
    g_pi_manager->LoadAllPlugIns( g_Plugin_Dir );

// Show the frame

    gFrame->ClearBackground();
    gFrame->Show( TRUE );

    if( g_bframemax ) gFrame->Maximize( true );

    stats = new StatWin( cc1 );
    stats->SetColorScheme( global_color_scheme );

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    if( cc1->GetQuiltMode() ) {
        stats->pPiano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
        stats->pPiano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );

        stats->pPiano->SetRoundedRectangles( true );
    }
    stats->pPiano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
    stats->pPiano->SetPolyIcon( new wxBitmap( style->GetIcon( _T("polyprj") ) ) );
    stats->pPiano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

    //  Yield to pick up the OnSize() calls that result from Maximize()
    Yield();

    wxString perspective;
    pConfig->SetPath( _T ( "/AUI" ) );
    pConfig->Read( _T ( "AUIPerspective" ), &perspective );

    // Make sure the perspective saved in the config file is "reasonable"
    // In particular, the perspective should have an entry for every
    // windows added to the AUI manager so far.
    // If any are not found, then use the default layout

    bool bno_load = false;
    wxAuiPaneInfoArray pane_array_val = g_pauimgr->GetAllPanes();

    for( unsigned int i = 0; i < pane_array_val.GetCount(); i++ ) {
        wxAuiPaneInfo pane = pane_array_val.Item( i );
        if( perspective.Find( pane.name ) == wxNOT_FOUND ) {
            bno_load = true;
            break;
        }
    }

    if( !bno_load ) g_pauimgr->LoadPerspective( perspective, false );


    g_pauimgr->Update();

    //   Notify all the AUI PlugIns so that they may syncronize with the Perspective
    g_pi_manager->NotifyAuiPlugIns();

    //   Initialize and Save the existing Screen Brightness
//       InitScreenBrightness();

    bool b_SetInitialPoint = false;

    //   Build the initial chart dir array
    ArrayOfCDI ChartDirArray;
    pConfig->LoadChartDirArray( ChartDirArray );

    //  Windows installer may have left hints regarding the initial chart dir selection
#ifdef __WXMSW__
    if( g_bFirstRun ) {
        int ndirs = 0;

        wxRegKey RegKey( wxString( _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN") ) );
        if( RegKey.Exists() ) {
            wxLogMessage( _("Retrieving initial Chart Directory set from Windows Registry") );
            wxString dirs;
            RegKey.QueryValue( wxString( _T("ChartDirs") ), dirs );

            wxStringTokenizer tkz( dirs, _T(";") );
            while( tkz.HasMoreTokens() ) {
                wxString token = tkz.GetNextToken();

                ChartDirInfo cdi;
                cdi.fullpath = token.Trim();
                cdi.magic_number = _T("");

                ChartDirArray.Add( cdi );
                ndirs++;
            }

        }

        if( ndirs ) pConfig->UpdateChartDirs( ChartDirArray );

        //    As a favor to new users, poll the database and
        //    move the initial viewport so that a chart will come up.
        if( ndirs ) b_SetInitialPoint = true;

    }
#endif

//    If the ChartDirArray is empty at this point, any existing chart database file must be declared invalid,
//    So it is best to simply delete it if present.
//    TODO  There is a possibility of recreating the dir list from the database itself......

    if( !ChartDirArray.GetCount() ) ::wxRemoveFile( *pChartListFileName );

//      Try to load the current chart list Data file
    ChartData = new ChartDB( gFrame );
    if( !ChartData->LoadBinary( pChartListFileName, ChartDirArray ) ) {
        bDBUpdateInProgress = true;

        if( ChartDirArray.GetCount() ) {
//              Create and Save a new Chart Database based on the hints given in the config file

            /*
             wxString msg1(_("OpenCPN needs to update the chart database from config file entries...."));

             OCPNMessageDialog mdlg(gFrame, msg1, wxString(_("OpenCPN Info")),wxICON_INFORMATION | wxOK );
             int dlg_ret;
             dlg_ret = mdlg.ShowModal();
             */
            delete ChartData;
            ChartData = new ChartDB( gFrame );

            wxString line( _("Rebuilding chart database from configuration file entries...") );
            /* The following 3 strings are embeded in wxProgressDialog but must be included by xgettext
             * to be localized properly. See {wxWidgets}src/generic/progdlgg.cpp:190 */
            wxString dummy1 = _("Elapsed time : ");
            wxString dummy2 = _("Estimated time : ");
            wxString dummy3 = _("Remaining time : ");
            wxProgressDialog *pprog = new wxProgressDialog( _("OpenCPN Chart Update"), line, 100,
                    NULL,
                    wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );

            ChartData->Create( ChartDirArray, pprog );
            ChartData->SaveBinary( pChartListFileName );

            delete pprog;
        }

        else            // No chart database, no config hints, so bail to Options....
        {
            wxLogMessage(
                    _T("Chartlist file not found, config chart dir array is empty.  Chartlist target file is:")
                            + *pChartListFileName );

            wxString msg1(
                    _("No Charts Installed.\nPlease select chart folders in Options > Charts.") );

            OCPNMessageBox(gFrame, msg1, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK );

            gFrame->DoOptionsDialog();

            b_SetInitialPoint = true;

        }

        bDBUpdateInProgress = false;

        //    As a favor to new users, poll the database and
        //    move the initial viewport so that a chart will come up.

        if( b_SetInitialPoint ) {
            double clat, clon;
            if( ChartData->GetCentroidOfLargestScaleChart( &clat, &clon, CHART_FAMILY_RASTER ) ) {
                gLat = clat;
                gLon = clon;
                gFrame->ClearbFollow();
            } else {
                if( ChartData->GetCentroidOfLargestScaleChart( &clat, &clon,
                        CHART_FAMILY_VECTOR ) ) {
                    gLat = clat;
                    gLon = clon;
                    gFrame->ClearbFollow();
                }
            }
        }

    }

    //  Apply the inital Group Array structure to the chart data base
    ChartData->ApplyGroupArray( g_pGroupArray );

    //  Make sure that the Selected Group is sensible...
    if( g_GroupIndex > (int) g_pGroupArray->GetCount() ) g_GroupIndex = 0;
    if( !gFrame->CheckGroup( g_GroupIndex ) ) g_GroupIndex = 0;

    //  Delete any stack built by no-chart startup case
    if( pCurrentStack ) delete pCurrentStack;

    pCurrentStack = new ChartStack;

    //  A useability enhancement....
    //  if the chart database is truly empty on startup, switch to SCMode
    //  so that the WVS chart will at least be shown
    if( ChartData && ( 0 == ChartData->GetChartTableEntries() ) ) {
        cc1->SetQuiltMode( false );
        gFrame->SetupQuiltMode();
    }

//      All set to go.....

//      establish GPS timeout value as multiple of frame timer
//      This will override any nonsense or unset value from the config file
    if( ( gps_watchdog_timeout_ticks > 60 ) || ( gps_watchdog_timeout_ticks <= 0 ) ) gps_watchdog_timeout_ticks =
            ( GPS_TIMEOUT_SECONDS * 1000 ) / TIMER_GFRAME_1;

    wxString dogmsg;
    dogmsg.Printf( _T("GPS Watchdog Timeout is: %d sec."), gps_watchdog_timeout_ticks );
    wxLogMessage( dogmsg );

    sat_watchdog_timeout_ticks = 12;

    gGPS_Watchdog = 2;
    gHDx_Watchdog = 2;
    gHDT_Watchdog = 2;
    gSAT_Watchdog = 2;
    gVAR_Watchdog = 2;

    //  Most likely installations have no ownship heading information
    g_bHDxValid = false;
    g_bHDTValid = false;
    g_bVARValid = false;
    g_bHDT_Rx = false;
    g_bVAR_Rx = false;

    gHdt = NAN;
    gHdm = NAN;
    gVar = NAN;

//  Start up a new track if enabled in config file
//        test this
    if( g_bTrackCarryOver ) gFrame->TrackOn();

//    Re-enable anchor watches if set in config file
    if( !g_AW1GUID.IsEmpty() ) {
        pAnchorWatchPoint1 = pWayPointMan->FindRoutePointByGUID( g_AW1GUID );
    }
    if( !g_AW2GUID.IsEmpty() ) {
        pAnchorWatchPoint2 = pWayPointMan->FindRoutePointByGUID( g_AW2GUID );
    }

    stats->Show( true );
    
    gFrame->DoChartUpdate();

    g_FloatingToolbarDialog->LockPosition(false);

    gFrame->RequestNewToolbar();

//      Start up the ticker....
    gFrame->FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

//      Start up the ViewPort Rotation angle Averaging Timer....
    gFrame->FrameCOGTimer.Start( 10, wxTIMER_CONTINUOUS );

//        gFrame->MemFootTimer.Start(wxMax(g_MemFootSec * 1000, 60 * 1000), wxTIMER_CONTINUOUS);
//        gFrame->MemFootTimer.Start(1000, wxTIMER_CONTINUOUS);

    // Import Layer-wise any .gpx files from /Layers directory
    wxString layerdir = g_PrivateDataDir;  //g_SData_Locn;
    wxChar sep = wxFileName::GetPathSeparator();
    if( layerdir.Last() != sep ) layerdir.Append( sep );
    layerdir.Append( _T("layers") );
    wxArrayString file_array;
    g_LayerIdx = 0;

    if( wxDir::Exists( layerdir ) ) {
        wxString laymsg;
        laymsg.Printf( wxT("Getting .gpx layer files from: %s"), layerdir.c_str() );
        wxLogMessage( laymsg );

        wxDir dir;
        dir.Open( layerdir );
        if( dir.IsOpened() ) {
            wxString filename;
            layerdir.Append( wxFileName::GetPathSeparator() );
            bool cont = dir.GetFirst( &filename );
            while( cont ) {
                filename.Prepend( layerdir );
                wxFileName f( filename );
                if( f.GetExt().IsSameAs( wxT("gpx") ) ) pConfig->ImportGPX( gFrame, true, filename,
                        false ); // preload a single-gpx-file layer
                else
                    pConfig->ImportGPX( gFrame, true, filename, true ); // preload a layer from subdirectory
                cont = dir.GetNext( &filename );
            }
        }
    }

    cc1->ReloadVP();                  // once more, and good to go

    g_FloatingCompassDialog = new ocpnFloatingCompassWindow( cc1 );
    if( g_FloatingCompassDialog ) g_FloatingCompassDialog->UpdateStatus( true );

    g_FloatingToolbarDialog->Raise();
    g_FloatingToolbarDialog->Show();

    gFrame->Refresh( false );
    gFrame->Raise();

    cc1->Enable();
    cc1->SetFocus();

    //  This little hack fixes a problem seen with some UniChrome OpenGL drivers
    //  We need a deferred resize to get glDrawPixels() to work right.
    //  So we set a trigger to generate a resize after 5 seconds....
    //  See the "UniChrome" hack elsewhere
    glChartCanvas *pgl = (glChartCanvas *) cc1->GetglCanvas();
    if( pgl && ( pgl->GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND ) ) {
        gFrame->m_defer_size = gFrame->GetSize();
        gFrame->SetSize( gFrame->m_defer_size.x - 10, gFrame->m_defer_size.y );
        g_pauimgr->Update();
        gFrame->m_bdefer_resize = true;
    }
    return TRUE;
}

int MyApp::OnExit()
{
    //  Send current nav status data to log file   // pjotrc 2010.02.09

    wxDateTime lognow = wxDateTime::Now();
    lognow.MakeGMT();
    wxString day = lognow.FormatISODate();
    wxString utc = lognow.FormatISOTime();
    wxString navmsg = _T("LOGBOOK:  ");
    navmsg += day;
    navmsg += _T(" ");
    navmsg += utc;
    navmsg += _T(" UTC ");

    if( bGPSValid ) {
        wxString data;
        data.Printf( _T("OFF: Lat %10.5f Lon %10.5f "), gLat, gLon );
        navmsg += data;

        wxString cog;
        if( wxIsNaN(gCog) ) cog.Printf( _T("COG ----- ") );
        else
            cog.Printf( _T("COG %10.5f "), gCog );

        wxString sog;
        if( wxIsNaN(gSog) ) sog.Printf( _T("SOG -----  ") );
        else
            sog.Printf( _T("SOG %6.2f"), gSog );

        navmsg += cog;
        navmsg += sog;

    } else {
        wxString data;
        data.Printf( _T("OFF: Lat %10.5f Lon %10.5f"), gLat, gLon );
        navmsg += data;
    }
    wxLogMessage( navmsg );
    g_loglast_time = lognow;

    if( ptcmgr ) delete ptcmgr;

    wxLogMessage( _T("opencpn::MyApp exiting cleanly...\n") );
    delete pConfig;
    delete pSelect;
    delete pSelectTC;
    delete pSelectAIS;

#ifdef USE_S57
    delete ps52plib;
#endif

    delete g_pGroupArray;
    delete pDummyChart;

    if( logger ) {
        wxLog::SetActiveTarget( Oldlogger );
        delete logger;
    }

    delete pChartListFileName;
    delete pHome_Locn;
    delete phost_name;
    delete pInit_Chart_Dir;
    delete pWorldMapLocation;

    delete pFontMgr;

    delete g_pRouteMan;
    delete pWayPointMan;

    delete pMessageOnceArray;

    DeInitializeUserColors();

    delete pLayerList;

#ifdef USE_S57
    delete m_pRegistrarMan;
    CSVDeaccess( NULL );
#endif

    delete g_StyleManager;

#ifdef USE_S57
#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL
    if(s_glu_dll_ready)
    FreeLibrary(s_hGLU_DLL);           // free the glu32.dll
#endif
#endif
#endif
#endif

//        _CrtDumpMemoryLeaks( );

    //      Restore any changed system colors
#ifdef __WXMSW__
    RestoreSystemColors();
#endif

#ifdef __MSVC__LEAK
    DeInitAllocCheck();
#endif

    delete g_pPlatform;
    delete g_pauimgr;

    delete plocale_def_lang;
    return TRUE;
}

void MyApp::TrackOff( void )
{
    if( gFrame ) gFrame->TrackOff();
}
/*
 int MyApp::FilterEvent(wxEvent& event)
 {
 if ( event.GetEventType() == wxEVT_CHAR )
 {
 if(cc1->Do_Hotkeys((wxKeyEvent&)event))
 return true;
 }
 return -1;
 }
 */
//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------
//      Frame implementation
BEGIN_EVENT_TABLE(MyFrame, wxFrame) EVT_CLOSE(MyFrame::OnCloseWindow)
EVT_MENU(wxID_EXIT, MyFrame::OnExit)
EVT_SIZE(MyFrame::OnSize)
EVT_MOVE(MyFrame::OnMove)
EVT_MENU(-1, MyFrame::OnToolLeftClick)
EVT_TIMER(FRAME_TIMER_1, MyFrame::OnFrameTimer1)
EVT_TIMER(FRAME_TC_TIMER, MyFrame::OnFrameTCTimer)
EVT_TIMER(FRAME_COG_TIMER, MyFrame::OnFrameCOGTimer)
EVT_TIMER(MEMORY_FOOTPRINT_TIMER, MyFrame::OnMemFootTimer)
EVT_ACTIVATE(MyFrame::OnActivate)
EVT_MAXIMIZE(MyFrame::OnMaximize)
EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_TOOL_RCLICKED, MyFrame::RequestNewToolbarArgEvent)
EVT_ERASE_BACKGROUND(MyFrame::OnEraseBackground)
END_EVENT_TABLE()

// My frame constructor
MyFrame::MyFrame( wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size,
        long style ) :
        wxFrame( frame, -1, title, pos, size, style ) //wxSIMPLE_BORDER | wxCLIP_CHILDREN | wxRESIZE_BORDER)
//wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER
{
    m_ulLastNEMATicktime = 0;
    m_pStatusBar = NULL;

    g_toolbar = NULL;
    m_toolbar_scale_tools_shown = false;

    //      Redirect the global heartbeat timer to this frame
    FrameTimer1.SetOwner( this, FRAME_TIMER_1 );

    //      Redirect the Tide/Current update timer to this frame
    FrameTCTimer.SetOwner( this, FRAME_TC_TIMER );

    //      Redirect the COG Averager timer to this frame
    FrameCOGTimer.SetOwner( this, FRAME_COG_TIMER );

    //      Redirect the Memory Footprint Management timer to this frame
    MemFootTimer.SetOwner( this, MEMORY_FOOTPRINT_TIMER );

    //      Set up some assorted member variables
    nRoute_State = 0;
    m_bTimeIsSet = false;
    m_bdefer_resize = false;

    //    Clear the NMEA Filter tables
    for( int i = 0; i < MAX_COGSOG_FILTER_SECONDS; i++ ) {
        COGFilterTable[i] = 0.;
        SOGFilterTable[i] = 0.;
    }
    m_COGFilterLast = 0.;
    m_last_bGPSValid = false;

    m_bpersistent_quilt = false;

    m_ChartUpdatePeriod = 1;                  // set the default (1 sec.) period

//    Establish my children
    g_pMUX = new Multiplexer();

    g_pAIS = new AIS_Decoder( this );
   

    for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
    {
        ConnectionParams *cp = g_pConnectionParams->Item(i);
        dsPortType port_type;
        if (cp->Output)
            port_type = DS_TYPE_INPUT_OUTPUT;
        else
            port_type = DS_TYPE_INPUT;
        DataStream *dstr = new DataStream( g_pMUX,
                                           cp->GetDSPort(),
                                           wxString::Format(wxT("%i"),cp->Baudrate),
                                           port_type,
                                           cp->Priority,
                                           cp->Garmin
                                         );
        dstr->SetInputFilter(cp->InputSentenceList);
        dstr->SetInputFilterType(cp->InputSentenceListType);
        dstr->SetOutputFilter(cp->OutputSentenceList);
        dstr->SetOutputFilterType(cp->OutputSentenceListType);
        dstr->SetChecksumCheck(cp->ChecksumCheck);
        dstr->SetGarminUploadMode(cp->GarminUpload);
        g_pMUX->AddStream(dstr);
    }
    g_pMUX->SetAISHandler(g_pAIS);
    g_pMUX->SetGPSHandler(this);
    //  Create/connect a dynamic event handler slot
    Connect( wxEVT_OCPN_DATASTREAM, (wxObjectEventFunction) (wxEventFunction) &MyFrame::OnEvtOCPN_NMEA );

    bFirstAuto = true;

    //  Create/connect a dynamic event handler slot for OCPN_MsgEvent(s) coming from PlugIn system
    Connect( wxEVT_OCPN_MSG, (wxObjectEventFunction) (wxEventFunction) &MyFrame::OnEvtPlugInMessage );

    //        Establish the system icons for the frame.

#ifdef __WXMSW__
    SetIcon( wxICON(0) );           // this grabs the first icon in the integrated MSW resource file
#endif

#ifdef __WXGTK__
    wxIcon app_icon(opencpn);          // This comes from opencpn.xpm inclusion above
    SetIcon(app_icon);
#endif

#ifdef __WXMSW__

//    Establish the entry points in USER32.DLL for system color control

    wxDynamicLibrary dllUser32( _T("user32.dll") );

    pSetSysColors = (SetSysColors_t) dllUser32.GetSymbol( wxT("SetSysColors") );
    pGetSysColor = (GetSysColor_t) dllUser32.GetSymbol( wxT("GetSysColor") );

    SaveSystemColors();
#endif

    g_FloatingToolbarConfigMenu = new wxMenu();

    m_next_available_plugin_tool_id = ID_PLUGIN_BASE;

    m_COGFilterLast = 0.;

    g_sticky_chart = -1;
}

MyFrame::~MyFrame()
{
    FrameTimer1.Stop();
    delete ChartData;
    delete pCurrentStack;

//      Free the Route List
    wxRouteListNode *node = pRouteList->GetFirst();

    while( node ) {
        Route *pRouteDelete = node->GetData();
        delete pRouteDelete;

        node = node->GetNext();
    }
    delete pRouteList;
    delete g_FloatingToolbarConfigMenu;
    g_pMUX->ClearStreams();
    delete g_pMUX;
}

void MyFrame::OnEraseBackground( wxEraseEvent& event )
{
}

void MyFrame::OnMaximize( wxMaximizeEvent& event )
{
    g_click_stop = 0;
}

void MyFrame::OnActivate( wxActivateEvent& event )
{
//    Code carefully in this method.
//    It is called in some unexpected places,
//    such as on closure of dialogs, etc.

    if( cc1 ) cc1->SetFocus();       // This seems to be needed for MSW, to get key and wheel events
                                     // after minimize/maximize.

#ifdef __WXOSX__
    if(event.GetActive())
    {
        SurfaceToolbar();

        if(g_FloatingCompassDialog)
            g_FloatingCompassDialog->Show();

        if(stats)
            stats->Show();
        
        if(console) {
            if( g_pRouteMan->IsAnyRouteActive() )
                console->Show();
        }
        
        gFrame->Raise();
        
    }
    else {
    }


#endif

    event.Skip();
}

ColorScheme MyFrame::GetColorScheme()
{
    return global_color_scheme;
}

void MyFrame::SetAndApplyColorScheme( ColorScheme cs )
{
    global_color_scheme = cs;

    wxString SchemeName;
    switch( cs ){
        case GLOBAL_COLOR_SCHEME_DAY:
            SchemeName = _T("DAY");
            break;
        case GLOBAL_COLOR_SCHEME_DUSK:
            SchemeName = _T("DUSK");
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            SchemeName = _T("NIGHT");
            break;
        default:
            SchemeName = _T("DAY");
            break;
    }

    g_StyleManager->GetCurrentStyle()->SetColorScheme( cs );
    cc1->GetWorldBackgroundChart()->SetColorScheme( cs );

#ifdef USE_S57
    if( ps52plib ) ps52plib->SetPLIBColorScheme( SchemeName );
#endif

    //Search the user color table array to find the proper hash table
    Usercolortable_index = 0;
    for( unsigned int i = 0; i < UserColorTableArray->GetCount(); i++ ) {
        colTable *ct = (colTable *) UserColorTableArray->Item( i );
        if( SchemeName.IsSameAs( *ct->tableName ) ) {
            Usercolortable_index = i;
            break;
        }
    }

    //    Set up a pointer to the proper hash table
    pcurrent_user_color_hash = (wxColorHashMap *) UserColourHashTableArray->Item(
            Usercolortable_index );

    SetSystemColors( cs );

    if( cc1 ) cc1->SetColorScheme( cs );

    if( pWayPointMan ) pWayPointMan->SetColorScheme( cs );

    if( ChartData ) ChartData->ApplyColorSchemeToCachedCharts( cs );

    if( stats ) stats->SetColorScheme( cs );

    if( console ) console->SetColorScheme( cs );

    if( g_pRouteMan ) g_pRouteMan->SetColorScheme( cs );

    if( pMarkPropDialog ) pMarkPropDialog->SetColorScheme( cs );

    if( pMarkInfoDialog ) pMarkInfoDialog->SetColorScheme( cs );

    //    For the AIS target query dialog, we must rebuild it to incorporate the style desired for the colorscheme selected
    if( g_pais_query_dialog_active ) {
        bool b_isshown = g_pais_query_dialog_active->IsShown();
        int n_mmsi = g_pais_query_dialog_active->GetMMSI();
        if( b_isshown ) g_pais_query_dialog_active->Show( false );              // dismiss it

        g_pais_query_dialog_active->Close();

        g_pais_query_dialog_active = new AISTargetQueryDialog();
        g_pais_query_dialog_active->Create( this, -1, _( "AIS Target Query" ),
                wxPoint( g_ais_query_dialog_x, g_ais_query_dialog_y ) );
        g_pais_query_dialog_active->SetMMSI( n_mmsi );
        g_pais_query_dialog_active->UpdateText();
        if( b_isshown ) g_pais_query_dialog_active->Show();
    }

    if( pRouteManagerDialog ) pRouteManagerDialog->SetColorScheme();

    if( g_pAISTargetList ) g_pAISTargetList->SetColorScheme();

    if( g_pObjectQueryDialog ) g_pObjectQueryDialog->SetColorScheme();

    ApplyGlobalColorSchemetoStatusBar();

    UpdateToolbar( cs );

    if( g_pi_manager ) g_pi_manager->SetColorSchemeForAllPlugIns( cs );
}

void MyFrame::ApplyGlobalColorSchemetoStatusBar( void )
{
    if( m_pStatusBar != NULL ) {
        m_pStatusBar->SetBackgroundColour( GetGlobalColor( _T("UIBDR") ) );    //UINFF
        m_pStatusBar->ClearBackground();

        int styles[] = { wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT };
        m_pStatusBar->SetStatusStyles( m_StatusBarFieldCount, styles );
        int widths[] = { -6, -5, -5, -3, -4 };
        m_pStatusBar->SetStatusWidths( m_StatusBarFieldCount, widths );
    }
}

void MyFrame::DestroyMyToolbar()
{
    if( g_FloatingToolbarDialog ) {
        g_FloatingToolbarDialog->DestroyToolBar();
        g_toolbar = NULL;
    }
}

bool _toolbarConfigMenuUtil( int toolid, wxString tipString )
{
    wxMenuItem* menuitem;

    if( toolid == ID_MOB && g_bPermanentMOBIcon ) return true;

    // Item ID trickery is needed because the wxCommandEvents for menu item clicked and toolbar button
    // clicked are 100% identical, so if we use same id's we can't tell the events apart.

    int idOffset = ID_PLUGIN_BASE - ID_ZOOMIN + 100;  // Hopefully no more than 100 plugins loaded...
    int menuItemId = toolid + idOffset;

    menuitem = g_FloatingToolbarConfigMenu->FindItem( menuItemId );

    if( menuitem ) {
        return menuitem->IsChecked();
    }

    menuitem = g_FloatingToolbarConfigMenu->AppendCheckItem( menuItemId, tipString );
    menuitem->Check( g_toolbarConfig.GetChar( toolid - ID_ZOOMIN ) == _T('X') );
    return menuitem->IsChecked();
}

ocpnToolBarSimple *MyFrame::CreateAToolbar()
{
    ocpnToolBarSimple *tb = NULL;
    wxToolBarToolBase* newtool;

    if( g_FloatingToolbarDialog ) tb = g_FloatingToolbarDialog->GetToolbar();
    if( !tb ) return 0;

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    wxString tipString;

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Zoom In") ) << _T(" (+)");
    if( _toolbarConfigMenuUtil( ID_ZOOMIN, tipString ) )
        tb->AddTool( ID_ZOOMIN, _T("zoomin"),
            style->GetToolIcon( _T("zoomin"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Zoom Out") ) << _T(" (-)");
    if( _toolbarConfigMenuUtil( ID_ZOOMOUT, tipString ) )
        tb->AddTool( ID_ZOOMOUT, _T("zoomout"),
            style->GetToolIcon( _T("zoomout"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    m_toolbar_scale_tools_shown = pCurrentStack && pCurrentStack->b_valid
            && ( pCurrentStack->nEntry > 1 );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Shift to Larger Scale Chart") ) << _T(" (F7)");
    if( _toolbarConfigMenuUtil( ID_STKDN, tipString ) ) {
        newtool = tb->AddTool( ID_STKDN, _T("scin"),
                style->GetToolIcon( _T("scin"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );
        newtool->Enable( m_toolbar_scale_tools_shown );
    }

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Shift to Smaller Scale Chart") ) << _T(" (F8)");
    if( _toolbarConfigMenuUtil( ID_STKUP, tipString ) ) {
        newtool = tb->AddTool( ID_STKUP, _T("scout"),
                style->GetToolIcon( _T("scout"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );
        newtool->Enable( m_toolbar_scale_tools_shown );
    }

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Create Route") ) << _T(" (Ctrl-R)");
    if( _toolbarConfigMenuUtil( ID_ROUTE, tipString ) )
        tb->AddTool( ID_ROUTE, _T("route"),
            style->GetToolIcon( _T("route"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("route"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Auto Follow") ) << _T(" (F2)");
    if( _toolbarConfigMenuUtil( ID_FOLLOW, tipString ) )
        tb->AddTool( ID_FOLLOW, _T("follow"),
            style->GetToolIcon( _T("follow"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("follow"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = _("Options");
    if( _toolbarConfigMenuUtil( ID_SETTINGS, tipString ) )
        tb->AddTool( ID_SETTINGS, _T("settings"),
            style->GetToolIcon( _T("settings"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Show ENC Text") ) << _T(" (T)");
    if( _toolbarConfigMenuUtil( ID_TEXT, tipString ) )
        tb->AddTool( ID_TEXT, _T("text"),
            style->GetToolIcon( _T("text"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("text"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    m_pAISTool = NULL;
    CheckAndAddPlugInTool( tb );
    tipString = _("Hide AIS Targets");          // inital state is on
    if( _toolbarConfigMenuUtil( ID_AIS, tipString ) )
        m_pAISTool = tb->AddTool( ID_AIS, _T("AIS"), style->GetToolIcon( _T("AIS"), TOOLICON_NORMAL ), style->GetToolIcon( _T("AIS"), TOOLICON_DISABLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = _("Show Currents");
    if( _toolbarConfigMenuUtil( ID_CURRENT, tipString ) )
        tb->AddTool( ID_CURRENT, _T("current"),
            style->GetToolIcon( _T("current"), TOOLICON_NORMAL ), tipString, wxITEM_CHECK );

    CheckAndAddPlugInTool( tb );
    tipString = _("Show Tides");
    if( _toolbarConfigMenuUtil( ID_TIDE, tipString ) )
        tb->AddTool( ID_TIDE, _T("tide"),
            style->GetToolIcon( _T("tide"), TOOLICON_NORMAL ), tipString, wxITEM_CHECK );

    CheckAndAddPlugInTool( tb );
    tipString = _("Print Chart");
    if( _toolbarConfigMenuUtil( ID_PRINT, tipString ) )
        tb->AddTool( ID_PRINT, _T("print"),
            style->GetToolIcon( _T("print"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("Route Manager");
    if( _toolbarConfigMenuUtil( ID_ROUTEMANAGER, tipString ) )
        tb->AddTool( ID_ROUTEMANAGER,
            _T("route_manager"), style->GetToolIcon( _T("route_manager"), TOOLICON_NORMAL ),
            tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("Toggle Tracking");
    if( _toolbarConfigMenuUtil( ID_TRACK, tipString ) )
        tb->AddTool( ID_TRACK, _T("track"),
            style->GetToolIcon( _T("track"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("track"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Change Color Scheme") ) << _T(" (F5)");
    if( _toolbarConfigMenuUtil( ID_COLSCHEME, tipString ) )
        tb->AddTool( ID_COLSCHEME,
            _T("colorscheme"), style->GetToolIcon( _T("colorscheme"), TOOLICON_NORMAL ),
            tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Drop MOB Marker") ) << _(" (Ctrl-Space)");
    if( _toolbarConfigMenuUtil( ID_MOB, tipString ) )
        tb->AddTool( ID_MOB, _T("mob_btn"),
            style->GetToolIcon( _T("mob_btn"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("About OpenCPN");
    if( _toolbarConfigMenuUtil( ID_HELP, tipString ) )
        tb->AddTool( ID_HELP, _T("help"),
            style->GetToolIcon( _T("help"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    //      Add any PlugIn toolbar tools that request default positioning
    AddDefaultPositionPlugInTools( tb );

// Realize() the toolbar
    g_FloatingToolbarDialog->Realize();

//      Set up the toggle states

    if( cc1 ) {
        //  Re-establish toggle states
        tb->ToggleTool( ID_CURRENT, cc1->GetbShowCurrent() );
        tb->ToggleTool( ID_TIDE, cc1->GetbShowTide() );
    }

    if( pConfig ) tb->ToggleTool( ID_FOLLOW, cc1->m_bFollow );

#ifdef USE_S57
    if( ( pConfig ) && ( ps52plib ) ) if( ps52plib->m_bOK ) tb->ToggleTool( ID_TEXT,
            ps52plib->GetShowS57Text() );
#endif
    tb->ToggleTool( ID_AIS, g_bShowAIS );
    tb->ToggleTool( ID_TRACK, g_bTrackActive );

    m_lastAISiconName = _T("");

    SetStatusBarPane( -1 );                   // don't show help on status bar

    return tb;
}

bool MyFrame::CheckAndAddPlugInTool( ocpnToolBarSimple *tb )
{
    if( !g_pi_manager ) return false;

    bool bret = false;
    int n_tools = tb->GetToolsCount();

    //    Walk the PlugIn tool spec array, checking the requested position
    //    If a tool has been requested by a plugin at this position, add it
    ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();

    for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
        PlugInToolbarToolContainer *pttc = tool_array.Item( i );
        if( pttc->position == n_tools ) {
            wxBitmap *ptool_bmp;

            switch( global_color_scheme ){
                case GLOBAL_COLOR_SCHEME_DAY:
                    ptool_bmp = pttc->bitmap_day;
                    ;
                    break;
                case GLOBAL_COLOR_SCHEME_DUSK:
                    ptool_bmp = pttc->bitmap_dusk;
                    break;
                case GLOBAL_COLOR_SCHEME_NIGHT:
                    ptool_bmp = pttc->bitmap_night;
                    break;
                default:
                    ptool_bmp = pttc->bitmap_day;
                    ;
                    break;
            }

            tb->AddTool( pttc->id, wxString( pttc->label ), *( ptool_bmp ),
                    wxString( pttc->shortHelp ), pttc->kind );
            if( pttc->kind == wxITEM_CHECK ) tb->ToggleTool( pttc->id, pttc->b_toggle );
            bret = true;
        }
    }

    //    If we added a tool, call again (recursively) to allow for adding adjacent tools
    if( bret ) while( CheckAndAddPlugInTool( tb ) ) { /* nothing to do */
    }

    return bret;
}

bool MyFrame::AddDefaultPositionPlugInTools( ocpnToolBarSimple *tb )
{
    if( !g_pi_manager ) return false;

    bool bret = false;

    //    Walk the PlugIn tool spec array, checking the requested position
    //    If a tool has been requested by a plugin at this position, add it
    ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();

    for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
        PlugInToolbarToolContainer *pttc = tool_array.Item( i );
        if( pttc->position == -1 )                  // PlugIn has requested default positioning
                {
            wxBitmap *ptool_bmp;

            switch( global_color_scheme ){
                case GLOBAL_COLOR_SCHEME_DAY:
                    ptool_bmp = pttc->bitmap_day;
                    ;
                    break;
                case GLOBAL_COLOR_SCHEME_DUSK:
                    ptool_bmp = pttc->bitmap_dusk;
                    break;
                case GLOBAL_COLOR_SCHEME_NIGHT:
                    ptool_bmp = pttc->bitmap_night;
                    break;
                default:
                    ptool_bmp = pttc->bitmap_day;
                    ;
                    break;
            }

            tb->AddTool( pttc->id, wxString( pttc->label ), *( ptool_bmp ),
                    wxString( pttc->shortHelp ), pttc->kind );
            if( pttc->kind == wxITEM_CHECK ) tb->ToggleTool( pttc->id, pttc->b_toggle );
            bret = true;
        }
    }
    return bret;
}

void MyFrame::RequestNewToolbar()
{
    if( g_FloatingToolbarDialog ) {
        bool b_reshow = g_FloatingToolbarDialog->IsShown();
        if( g_FloatingToolbarDialog->IsToolbarShown() ) DestroyMyToolbar();

        g_toolbar = CreateAToolbar();
        g_FloatingToolbarDialog->RePosition();
        g_FloatingToolbarDialog->Show( b_reshow );
    }
}

//      Update inplace the current toolbar with bitmaps corresponding to the current color scheme
void MyFrame::UpdateToolbar( ColorScheme cs )
{
    if( g_FloatingToolbarDialog ) {
        g_FloatingToolbarDialog->SetColorScheme( cs );

        if( g_FloatingToolbarDialog->IsToolbarShown() ) {
            DestroyMyToolbar();
            g_toolbar = CreateAToolbar();
        }
    }

    if( g_FloatingCompassDialog ) g_FloatingCompassDialog->SetColorScheme( cs );

    if( g_toolbar ) {
        //  Re-establish toggle states
        g_toolbar->ToggleTool( ID_FOLLOW, cc1->m_bFollow );
        g_toolbar->ToggleTool( ID_CURRENT, cc1->GetbShowCurrent() );
        g_toolbar->ToggleTool( ID_TIDE, cc1->GetbShowTide() );
    }

    return;
}

void MyFrame::EnableToolbar( bool newstate )
{
    if( g_toolbar ) {
        g_toolbar->EnableTool( ID_ZOOMIN, newstate );
        g_toolbar->EnableTool( ID_ZOOMOUT, newstate );
        g_toolbar->EnableTool( ID_STKUP, newstate );
        g_toolbar->EnableTool( ID_STKDN, newstate );
        g_toolbar->EnableTool( ID_ROUTE, newstate );
        g_toolbar->EnableTool( ID_FOLLOW, newstate );
        g_toolbar->EnableTool( ID_SETTINGS, newstate );
        g_toolbar->EnableTool( ID_TEXT, newstate );
        g_toolbar->EnableTool( ID_CURRENT, newstate );
        g_toolbar->EnableTool( ID_TIDE, newstate );
        g_toolbar->EnableTool( ID_HELP, newstate );
        g_toolbar->EnableTool( ID_TBEXIT, newstate );
        g_toolbar->EnableTool( ID_TBSTAT, newstate );
        g_toolbar->EnableTool( ID_PRINT, newstate );
        g_toolbar->EnableTool( ID_COLSCHEME, newstate );
        g_toolbar->EnableTool( ID_ROUTEMANAGER, newstate );
        g_toolbar->EnableTool( ID_TRACK, newstate );
        g_toolbar->EnableTool( ID_AIS, newstate );
    }
}

// Intercept menu commands
void MyFrame::OnExit( wxCommandEvent& event )
{
    quitflag++;                             // signal to the timer loop

}

static bool b_inCloseWindow;

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    //    It is possible that double clicks on application exit box could cause re-entrance here
    //    Not good, and don't need it anyway, so simply return.
    if( b_inCloseWindow ) {
//            wxLogMessage(_T("opencpn::MyFrame re-entering OnCloseWindow"));
        return;
    }

    b_inCloseWindow = true;

    ::wxSetCursor( wxCURSOR_WAIT );

    // If we happen to have the measure tool open on Ctrl-Q quit
    cc1->CancelMeasureRoute();

    // We save perspective before closing to restore position next time
    // Pane is not closed so the child is not notified (OnPaneClose)
    if( g_pAISTargetList ) {
        wxAuiPaneInfo &pane = g_pauimgr->GetPane( g_pAISTargetList );
        g_AisTargetList_perspective = g_pauimgr->SavePaneInfo( pane );
        g_pauimgr->DetachPane( g_pAISTargetList );
    }

    pConfig->SetPath( _T ( "/AUI" ) );
    pConfig->Write( _T ( "AUIPerspective" ), g_pauimgr->SavePerspective() );

    g_bquiting = true;
    if( cc1 ) {
        cc1->SetCursor( wxCURSOR_WAIT );

        cc1->Refresh( false );
        cc1->Update();
    }

    //   Save the saved Screen Brightness
    RestoreScreenBrightness();

    //    Deactivate the PlugIns
    if( g_pi_manager ) {
        g_pi_manager->DeactivateAllPlugIns();
    }

    wxLogMessage( _T("opencpn::MyFrame exiting cleanly.") );

    quitflag++;

    FrameTimer1.Stop();

    /*
     Automatically drop an anchorage waypoint, if enabled
     On following conditions:
     1.  In "Cruising" mode, meaning that speed has at some point exceeded 3.0 kts.
     2.  Current speed is less than 0.5 kts.
     3.  Opencpn has been up at least 30 minutes
     4.  And, of course, opencpn is going down now.
     5.  And if there is no anchor watch set on "anchor..." icon mark           // pjotrc 2010.02.15
     */
    if( g_bAutoAnchorMark ) {
        bool watching_anchor = false;                                           // pjotrc 2010.02.15
        if( pAnchorWatchPoint1 )                                               // pjotrc 2010.02.15
        watching_anchor = ( pAnchorWatchPoint1->m_IconName.StartsWith( _T("anchor") ) ); // pjotrc 2010.02.15
        if( pAnchorWatchPoint2 )                                               // pjotrc 2010.02.15
        watching_anchor |= ( pAnchorWatchPoint2->m_IconName.StartsWith( _T("anchor") ) ); // pjotrc 2010.02.15

        wxDateTime now = wxDateTime::Now();
        wxTimeSpan uptime = now.Subtract( g_start_time );

        if( !watching_anchor && ( g_bCruising ) && ( gSog < 0.5 )
                && ( uptime.IsLongerThan( wxTimeSpan( 0, 30, 0, 0 ) ) ) )     // pjotrc 2010.02.15
                {
            //    First, delete any single anchorage waypoint closer than 0.25 NM from this point
            //    This will prevent clutter and database congestion....

            wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();
            while( node ) {
                RoutePoint *pr = node->GetData();
                if( pr->GetName().StartsWith( _T("Anchorage") ) ) {
                    double a = gLat - pr->m_lat;
                    double b = gLon - pr->m_lon;
                    double l = sqrt( ( a * a ) + ( b * b ) );

                    // caveat: this is accurate only on the Equator
                    if( ( l * 60. * 1852. ) < ( .25 * 1852. ) ) {
                        pConfig->DeleteWayPoint( pr );
                        pSelect->DeleteSelectablePoint( pr, SELTYPE_ROUTEPOINT );
                        delete pr;
                        break;
                    }
                }

                node = node->GetNext();
            }

            wxString name = now.Format();
            name.Prepend( _("Anchorage created ") );
            RoutePoint *pWP = new RoutePoint( gLat, gLon, _T("anchorage"), name, GPX_EMPTY_STRING );
            pWP->m_bShowName = false;
            pWP->m_bIsolatedMark = true;

            pConfig->AddNewWayPoint( pWP, -1 );       // use auto next num
        }
    }

    FrameTimer1.Stop();

    g_bframemax = IsMaximized();

    //    Record the current state of tracking
    g_bTrackCarryOver = g_bTrackActive;

    TrackOff();

    if( pCurrentStack ) {
        g_restore_stackindex = pCurrentStack->CurrentStackEntry;
        g_restore_dbindex = pCurrentStack->GetCurrentEntrydbIndex();
    }

    if( g_FloatingToolbarDialog ) {
        wxPoint tbp = g_FloatingToolbarDialog->GetPosition();
        wxPoint tbp_incanvas = cc1->ScreenToClient( tbp );
        g_toolbar_x = tbp_incanvas.x;
        g_toolbar_y = tbp_incanvas.y;
        g_toolbar_orient = g_FloatingToolbarDialog->GetOrient();
    }

    pConfig->UpdateSettings();
    pConfig->UpdateNavObj();

    pConfig->m_pNavObjectChangesSet->Clear();
    delete pConfig->m_pNavObjectChangesSet;

    //Remove any leftover Routes and Waypoints from config file as they were saved to navobj before
    pConfig->DeleteGroup( _T ( "/Routes" ) );
    pConfig->DeleteGroup( _T ( "/Marks" ) );
    pConfig->Flush();

    delete g_printData;
    delete g_pageSetupData;

    if( g_pAboutDlg ) g_pAboutDlg->Destroy();

//      Explicitely Close some children, especially the ones with event handlers
//      or that call GUI methods

#ifdef USE_S57
    if( g_pCM93OffsetDialog ) g_pCM93OffsetDialog->Destroy();
#endif

    g_FloatingToolbarDialog->Destroy();

    if( g_pAISTargetList ) {
        g_pAISTargetList->Disconnect_decoder();
        g_pAISTargetList->Destroy();
    }

    g_FloatingCompassDialog->Destroy();
    g_FloatingCompassDialog = NULL;

    cc1->Destroy();
    cc1 = NULL;

    //      Delete all open charts in the cache
    if( ChartData ) ChartData->PurgeCache();

    //    Unload the PlugIns
    //      Note that we are waiting until after the canvas is destroyed,
    //      since some PlugIns may have created children of canvas.
    //      Such a PlugIn must stay intact for the canvas dtor to call DestoryChildren()
    if( g_pi_manager ) {
        g_pi_manager->UnLoadAllPlugIns();
        delete g_pi_manager;
        g_pi_manager = NULL;
    }

    if( g_pAIS ) {
        if(g_pMUX)
            g_pMUX->SetAISHandler(NULL);
        delete g_pAIS;
        g_pAIS = NULL;
    }

    SetStatusBar( NULL );
    stats = NULL;

//    delete pthumbwin;
    pthumbwin = NULL;

//    delete g_FloatingToolbarDialog;
    g_FloatingToolbarDialog = NULL;

    g_pauimgr->UnInit();

    this->Destroy();

}

void MyFrame::OnMove( wxMoveEvent& event )
{
    if( g_FloatingToolbarDialog ) g_FloatingToolbarDialog->RePosition();

    if( stats ) stats->RePosition();

    UpdateGPSCompassStatusBox( true );

    if( console && console->IsShown() ) PositionConsole();

//    Somehow, this method does not work right on Windows....
//      g_nframewin_posx = event.GetPosition().x;
//      g_nframewin_posy = event.GetPosition().y;

    g_nframewin_posx = GetPosition().x;
    g_nframewin_posy = GetPosition().y;
}

void MyFrame::ProcessCanvasResize( void )
{
    if( stats ) {
        stats->ReSize();
        stats->RePosition();
    }

    if( g_FloatingToolbarDialog ) {
        g_FloatingToolbarDialog->RePosition();
        g_FloatingToolbarDialog->SetGeometry();
        g_FloatingToolbarDialog->Realize();
        g_FloatingToolbarDialog->RePosition();

    }

    UpdateGPSCompassStatusBox( true );

    if( console->IsShown() ) PositionConsole();

}

void MyFrame::OnSize( wxSizeEvent& event )
{
    DoSetSize();
}

void MyFrame::DoSetSize( void )
{
    int x, y;
    GetClientSize( &x, &y );

//      Resize the children

    if( m_pStatusBar ) {
        //  Maybe resize the font
        wxRect stat_box;
        m_pStatusBar->GetFieldRect( 0, stat_box );
        int font_size = stat_box.width / 28;                // 30 for linux

#ifdef __WXMAC__
        font_size = wxMax(10, font_size);             // beats me...
#endif

        wxFont* templateFont = pFontMgr->GetFont( _("StatusBar"), 12 );
        font_size += templateFont->GetPointSize() - 10;

        font_size = wxMin( font_size, 12 );
        font_size = wxMax( font_size, 5 );

        wxFont *pstat_font = wxTheFontList->FindOrCreateFont( font_size,
              wxFONTFAMILY_SWISS, templateFont->GetStyle(), templateFont->GetWeight(), false,
              templateFont->GetFaceName() );

        m_pStatusBar->SetFont( *pstat_font );
    }

    int cccw = x;
    int ccch = y;

    if( cc1 ) {
        cccw = x * 10 / 10;               // constrain to mod 4
        int wr = cccw / 4;
        cccw = wr * 4;
        cccw += 2;                              // account for simple border

        int cur_width, cur_height;
        cc1->GetSize( &cur_width, &cur_height );
        if( ( cur_width != cccw ) || ( cur_height != ccch ) ) {
            if( g_pauimgr->GetPane( cc1 ).IsOk() )
                g_pauimgr->GetPane( cc1 ).BestSize( cccw, ccch );
            else
                cc1->SetSize( 0, 0, cccw, ccch );
        }
    }

    if( g_FloatingToolbarDialog ) {
        wxSize oldSize = g_FloatingToolbarDialog->GetSize();
        g_FloatingToolbarDialog->RePosition();
        g_FloatingToolbarDialog->SetGeometry();
        g_FloatingToolbarDialog->Realize();

        if( oldSize != g_FloatingToolbarDialog->GetSize() )
            g_FloatingToolbarDialog->Refresh( false );

        g_FloatingToolbarDialog->RePosition();

    }

    UpdateGPSCompassStatusBox( true );

    if( console ) PositionConsole();

    if( stats ) {
        stats->ReSize();
        stats->FormatStat();
        stats->RePosition();
    }

//  Update the stored window size
    GetSize( &x, &y );
    g_nframewin_x = x;
    g_nframewin_y = y;

//  Inform the PlugIns
    if( g_pi_manager ) g_pi_manager->SendResizeEventToAllPlugIns( x, y );

//  Force redraw if in lookahead mode
    if( g_bLookAhead ) {
        if( g_bCourseUp ) DoCOGSet();
        else
            DoChartUpdate();
    }

    if( pthumbwin ) pthumbwin->SetMaxSize( cc1->GetParent()->GetSize() );
}

void MyFrame::PositionConsole( void )
{
    if( NULL == cc1 ) return;
    //    Reposition console based on its size and chartcanvas size
    int ccx, ccy, ccsx, ccsy, consx, consy;
    cc1->GetSize( &ccsx, &ccsy );
    cc1->GetPosition( &ccx, &ccy );

    console->GetSize( &consx, &consy );

    wxPoint screen_pos = ClientToScreen( wxPoint( ccx + ccsx - consx - 2, ccy + 45 ) );
    console->Move( screen_pos );
}

void MyFrame::UpdateAllFonts()
{
    if( console ) {
        console->UpdateFonts();
        //    Reposition console
        PositionConsole();
    }

    if( g_pais_query_dialog_active ) {
        g_pais_query_dialog_active->Destroy();
        g_pais_query_dialog_active = NULL;
    }

    if( pWayPointMan ) pWayPointMan->ClearRoutePointFonts();

    cc1->Refresh();
}

void MyFrame::SetGroupIndex( int index )
{
    int new_index = index;
    if( index > (int) g_pGroupArray->GetCount() ) new_index = 0;

    bool bgroup_override = false;
    int old_group_index = new_index;

    if( !CheckGroup( new_index ) ) {
        new_index = 0;
        bgroup_override = true;
    }

    //    Get the currently displayed chart native scale, and the current ViewPort
    int current_chart_native_scale = cc1->GetCanvasChartNativeScale();
    ViewPort vp = cc1->GetVP();

    g_GroupIndex = new_index;
    
    //  Invalidate the "sticky" chart on group change, since it might not be in the new group
    g_sticky_chart = -1;

    //    We need a new chartstack and quilt to figure out which chart to open in the new group
    cc1->UpdateCanvasOnGroupChange();

    int dbi_hint = cc1->FindClosestCanvasChartdbIndex( current_chart_native_scale );

    //    Refresh the canvas, selecting the "best" chart,
    //    applying the prior ViewPort exactly
    ChartsRefresh( dbi_hint, vp, false );

    //    Message box is deferred so that canvas refresh occurs properly before dialog
    if( bgroup_override ) {
        wxString msg( _("Group \"") );
        msg += GetGroupName( old_group_index );
        msg += _("\" is empty, switching to \"All Active Charts\" group.");

        wxMessageBox( msg, _("OpenCPN Group Notice"), wxOK, this );
    }
}

void MyFrame::ShowBrightnessLevelTimedDialog( int brightness, int min, int max )
{
    wxFont *pfont = wxTheFontList->FindOrCreateFont( 40, wxDEFAULT, wxNORMAL, wxBOLD );

    if( !g_pbrightness_indicator_dialog ) {
        //    Calculate size
        int x, y;
        GetTextExtent( _T("MAX"), &x, &y, NULL, NULL, pfont );

        g_pbrightness_indicator_dialog = new OCPNBitmapDialog( this, wxPoint( 200, 200 ),
                wxSize( x + 2, y + 2 ) );
    }

    int bmpsx = g_pbrightness_indicator_dialog->GetSize().x;
    int bmpsy = g_pbrightness_indicator_dialog->GetSize().y;

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

    g_pbrightness_indicator_dialog->SetBitmap( bmp );
    g_pbrightness_indicator_dialog->Show();
    g_pbrightness_indicator_dialog->Refresh();

    g_brightness_timeout = 3;           // seconds

}

void MyFrame::OnToolLeftClick( wxCommandEvent& event )
{
    if( s_ProgDialog ) return;

    switch( event.GetId() ){
        case ID_STKUP:
            DoStackUp();
            DoChartUpdate();
            break;

        case ID_STKDN:
            DoStackDown();
            DoChartUpdate();
            break;

        case ID_ZOOMIN: {
            cc1->ZoomCanvasIn( 2.0 );
            DoChartUpdate();
            break;
        }

        case ID_ZOOMOUT: {
            cc1->ZoomCanvasOut( 2.0 );
            DoChartUpdate();
            break;
        }

        case ID_ROUTE: {
            nRoute_State = 1;
            cc1->SetCursor( *cc1->pCursorPencil );
            break;
        }

        case ID_FOLLOW: {
            TogglebFollow();
            break;
        }

#ifdef USE_S57
        case ID_TEXT: {
            ps52plib->SetShowS57Text( !ps52plib->GetShowS57Text() );
            if( g_toolbar ) g_toolbar->ToggleTool( ID_TEXT, ps52plib->GetShowS57Text() );
            cc1->ReloadVP();
            break;
        }
#endif

        case ID_AIS: {
            g_bShowAIS = !g_bShowAIS;
            if( g_toolbar ) g_toolbar->ToggleTool( ID_AIS, g_bShowAIS );
            cc1->ReloadVP();
            
            if( g_toolbar ) {
                if( g_bShowAIS )
                    g_toolbar->SetToolShortHelp( ID_AIS, _("Hide AIS Targets") );
                else
                    g_toolbar->SetToolShortHelp( ID_AIS, _("Show AIS Targets") );
            }
            
            break;
        }

        case ID_SETTINGS: {

            bool bnewtoolbar = !( DoOptionsDialog() == 0 );

//              Apply various system settings
            ApplyGlobalSettings( true, bnewtoolbar );                 // flying update

            if( g_FloatingToolbarDialog ) g_FloatingToolbarDialog->RefreshFadeTimer();

            if( cc1->GetbShowCurrent() || cc1->GetbShowTide() ) LoadHarmonics();
//  The chart display options may have changed, especially on S57 ENC,
//  So, flush the cache and redraw
            cc1->ReloadVP();
            break;
        }

        case ID_CURRENT: {
            LoadHarmonics();

            if( ptcmgr->IsReady() ) {
                cc1->SetbShowCurrent( !cc1->GetbShowCurrent() );
                if( g_toolbar ) g_toolbar->ToggleTool( ID_CURRENT, cc1->GetbShowCurrent() );
                cc1->ReloadVP();
            } else {
                wxLogMessage( _T("Chart1::Event...TCMgr Not Available") );
                cc1->SetbShowCurrent( false );
                if( g_toolbar ) g_toolbar->ToggleTool( ID_CURRENT, false );
            }

            if( cc1->GetbShowCurrent() ) {
                FrameTCTimer.Start( TIMER_TC_VALUE_SECONDS * 1000, wxTIMER_CONTINUOUS );
                cc1->SetbTCUpdate( true );                        // force immediate update
            } else
                FrameTCTimer.Stop();

            cc1->Refresh( false );

            break;

        }

        case ID_TIDE: {
            LoadHarmonics();

            if( ptcmgr->IsReady() ) {
                cc1->SetbShowTide( !cc1->GetbShowTide() );
                if( g_toolbar ) g_toolbar->ToggleTool( ID_TIDE, cc1->GetbShowTide() );
                cc1->ReloadVP();
            } else {
                wxLogMessage( _("Chart1::Event...TCMgr Not Available") );
                cc1->SetbShowTide( false );
                if( g_toolbar ) g_toolbar->ToggleTool( ID_TIDE, false );
            }

            if( cc1->GetbShowTide() ) {
                FrameTCTimer.Start( TIMER_TC_VALUE_SECONDS * 1000, wxTIMER_CONTINUOUS );
                cc1->SetbTCUpdate( true );                        // force immediate update
            } else
                FrameTCTimer.Stop();

            cc1->Refresh( false );

            break;

        }

        case ID_HELP: {
            if( !g_pAboutDlg ) g_pAboutDlg = new about( this, &g_SData_Locn );

            g_pAboutDlg->Update();
            g_pAboutDlg->Show();

            break;
        }

        case ID_PRINT: {
            DoPrint();
            break;
        }

        case ID_COLSCHEME: {
            ToggleColorScheme();
            break;
        }

        case ID_TBEXIT: {
            Close();
            break;
        }

        case ID_ROUTEMANAGER: {
            if( NULL == pRouteManagerDialog )         // There is one global instance of the Dialog
            pRouteManagerDialog = new RouteManagerDialog( this );

            pRouteManagerDialog->UpdateRouteListCtrl();
            pRouteManagerDialog->UpdateTrkListCtrl();
            pRouteManagerDialog->UpdateWptListCtrl();
            pRouteManagerDialog->UpdateLayListCtrl();
            pRouteManagerDialog->Show();

            //    Required if RMDialog is not STAY_ON_TOP
#ifdef __WXOSX__
            pRouteManagerDialog->Centre();
            pRouteManagerDialog->Raise();
#endif
            break;
        }

        case ID_TRACK: {
            if( !g_bTrackActive ) TrackOn();
            else
                TrackOff( true );
            break;
        }

        case ID_TBSTATBOX: {
            ToggleCourseUp();
            break;
        }

        case ID_MOB: {
            ActivateMOB();
            break;
        }

        default: {
            //        Look for PlugIn tools
            //        If found, make the callback.
            //        TODO Modify this to allow multiple tools per plugin
            if( g_pi_manager ) {
                ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();
                for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
                    PlugInToolbarToolContainer *pttc = tool_array.Item( i );
                    if( event.GetId() == pttc->id ) {
                        if( pttc->m_pplugin ) pttc->m_pplugin->OnToolbarToolCallback( pttc->id );
                    }
                }
            }
            break;
        }

    }         // switch
}

void MyFrame::ToggleColorScheme()
{
    ColorScheme s = GetColorScheme();
    int is = (int) s;
    is++;
    s = (ColorScheme) is;
    if( s == N_COLOR_SCHEMES ) s = GLOBAL_COLOR_SCHEME_RGB;

    SetAndApplyColorScheme( s );
}

void MyFrame::ToggleFullScreen()
{
    bool to = !IsFullScreen();
    long style = wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION | wxFULLSCREEN_NOMENUBAR;

    if( g_FloatingToolbarDialog ) g_FloatingToolbarDialog->Show( g_bFullscreenToolbar | !to );

    ShowFullScreen( to, style );
    UpdateToolbar( global_color_scheme );
    Layout();
}

void MyFrame::ActivateMOB( void )
{
    //    The MOB point
    wxDateTime mob_time = wxDateTime::Now();
    wxString mob_label( _( "MAN OVERBOARD" ) );
    mob_label += _T(" at ");
    mob_label += mob_time.FormatTime();

    RoutePoint *pWP_MOB = new RoutePoint( gLat, gLon, _T ( "mob" ), mob_label, GPX_EMPTY_STRING );
    pWP_MOB->m_bKeepXRoute = true;
    pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_MOB );

    if( bGPSValid && !wxIsNaN(gCog) && !wxIsNaN(gSog) ) {
        //    Create a point that is one mile along the present course
        double zlat, zlon;
        ll_gc_ll( gLat, gLon, gCog, 1.0, &zlat, &zlon );

        RoutePoint *pWP_src = new RoutePoint( zlat, zlon, g_default_wp_icon,
                wxString( _( "1.0 NM along COG" ) ), GPX_EMPTY_STRING );
        pSelect->AddSelectableRoutePoint( zlat, zlon, pWP_src );

        Route *temp_route = new Route();
        pRouteList->Append( temp_route );

        temp_route->AddPoint( pWP_src );
        temp_route->AddPoint( pWP_MOB );

        pSelect->AddSelectableRouteSegment( gLat, gLon, zlat, zlon, pWP_src, pWP_MOB, temp_route );

        temp_route->m_RouteNameString = _("Temporary MOB Route");
        temp_route->m_RouteStartString = _("Assumed 1 Mile Point");
        ;
        temp_route->m_RouteEndString = mob_label;

        temp_route->m_bDeleteOnArrival = false;

        temp_route->SetRouteArrivalRadius( -1.0 );                    // never arrives

        temp_route->RebuildGUIDList();         // ensure the GUID list is intact and good

        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();
        g_pRouteMan->ActivateRoute( temp_route, pWP_MOB );
    }

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
        pRouteManagerDialog->UpdateRouteListCtrl();
        pRouteManagerDialog->UpdateWptListCtrl();
    }

    cc1->Refresh( false );

    wxString mob_message( _( "MAN OVERBOARD" ) );
    mob_message += _T(" Time: ");
    mob_message += mob_time.Format();
    mob_message += _T("  Position: ");
    mob_message += toSDMM( 1, gLat );
    mob_message += _T("   ");
    mob_message += toSDMM( 2, gLon );
    wxLogMessage( mob_message );

}
void MyFrame::TrackOn( void )
{
    g_bTrackActive = true;
    g_pActiveTrack = new Track();

    pRouteList->Append( g_pActiveTrack );
    g_pActiveTrack->Start();

    if( g_toolbar ) g_toolbar->ToggleTool( ID_TRACK, g_bTrackActive );

}

void MyFrame::TrackOff( bool do_add_point )
{
    if( g_pActiveTrack ) {
        g_pActiveTrack->Stop( do_add_point );

        if( g_pActiveTrack->GetnPoints() < 2 ) g_pRouteMan->DeleteRoute( g_pActiveTrack );
        else
            if( g_bTrackDaily ) {
                if( g_pActiveTrack->DoExtendDaily() ) g_pRouteMan->DeleteRoute( g_pActiveTrack );
            }

    }

    g_pActiveTrack = NULL;

    g_bTrackActive = false;

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
        pRouteManagerDialog->UpdateTrkListCtrl();
        pRouteManagerDialog->UpdateRouteListCtrl();
    }

    if( g_toolbar ) g_toolbar->ToggleTool( ID_TRACK, g_bTrackActive );
}

void MyFrame::TrackMidnightRestart( void )
{
    if( !g_pActiveTrack ) return;

    Track *pPreviousTrack = g_pActiveTrack;
    TrackOff( true );
    TrackOn();
    g_pActiveTrack->FixMidnight( pPreviousTrack );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
        pRouteManagerDialog->UpdateTrkListCtrl();
        pRouteManagerDialog->UpdateRouteListCtrl();
    }
}

void MyFrame::ToggleCourseUp( void )
{
    g_bCourseUp = !g_bCourseUp;

    if( g_bCourseUp ) {
        //    Stuff the COGAvg table in case COGUp is selected
        double stuff = 0.;
        if( !wxIsNaN(gCog) ) stuff = gCog;

        for( int i = 0; i < g_COGAvgSec; i++ )
            COGTable[i] = stuff;
        g_COGAvg = stuff;
    }

    DoCOGSet();
    UpdateGPSCompassStatusBox( true );
    DoChartUpdate();
    cc1->ReloadVP();
}

void MyFrame::ToggleENCText( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        ps52plib->SetShowS57Text( !ps52plib->GetShowS57Text() );
        if( g_toolbar ) g_toolbar->ToggleTool( ID_TEXT, ps52plib->GetShowS57Text() );
        cc1->ReloadVP();
    }

#endif
}

void MyFrame::ToggleSoundings( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        ps52plib->SetShowSoundings( !ps52plib->GetShowSoundings() );
        cc1->ReloadVP();
    }
#endif
}

bool MyFrame::ToggleLights( bool doToggle, bool temporary )
{
    bool oldstate = true;
#ifdef USE_S57
    if( ps52plib ) {
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "LIGHTS", 6 ) ) {
                oldstate = pOLE->nViz != 0;
                if( doToggle ) pOLE->nViz = !pOLE->nViz;
                break;
            }
        }
        if( doToggle ) {
            if( ! temporary ) {
                ps52plib->GenerateStateHash();
                cc1->ReloadVP();
            }
        }
    }
#endif
    return oldstate;
}

void MyFrame::ToggleRocks( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        int vis =  0;
        // Need to loop once for UWTROC, which is our "master", then for
        // other categories, since order is unknown?
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "UWTROC", 6 ) ) {
                pOLE->nViz = !pOLE->nViz;
                vis = pOLE->nViz;
            }
        }
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "OBSTRN", 6 ) ) {
                pOLE->nViz = vis;
            }
            if( !strncmp( pOLE->OBJLName, "WRECKS", 6 ) ) {
                pOLE->nViz = vis;
            }
        }
        ps52plib->GenerateStateHash();
        cc1->ReloadVP();
    }
#endif
}

void MyFrame::ToggleAnchor( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        int vis =  0;
        // Need to loop once for SBDARE, which is our "master", then for
        // other categories, since order is unknown?
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "SBDARE", 6 ) ) {
                pOLE->nViz = !pOLE->nViz;
                vis = pOLE->nViz;
                break;
            }
        }
        const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL" };
        unsigned int num = sizeof(categories) / sizeof(categories[0]);
        unsigned int cnt = 0;
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            for( unsigned int c = 0; c < num; c++ ) {
                if( !strncmp( pOLE->OBJLName, categories[c], 6 ) ) {
                    pOLE->nViz = vis;
                    cnt++;
                    break;
                }
            }
            if( cnt == num ) break;
        }
        ps52plib->GenerateStateHash();
        cc1->ReloadVP();
    }
#endif
}

void MyFrame::TogglebFollow( void )
{
    if( !cc1->m_bFollow ) SetbFollow();
    else
        ClearbFollow();
}

void MyFrame::SetbFollow( void )
{
    cc1->m_bFollow = true;
    if( g_toolbar ) g_toolbar->ToggleTool( ID_FOLLOW, cc1->m_bFollow );

    DoChartUpdate();
    cc1->ReloadVP();

}

void MyFrame::ClearbFollow( void )
{
    //    Center the screen on the GPS position, for lack of a better place
    vLat = gLat;
    vLon = gLon;
    cc1->m_bFollow = false;
    if( g_toolbar ) g_toolbar->ToggleTool( ID_FOLLOW, cc1->m_bFollow );
    DoChartUpdate();
    cc1->ReloadVP();

}

void MyFrame::ToggleChartOutlines( void )
{
    if( !g_bShowOutlines ) g_bShowOutlines = true;
    else
        g_bShowOutlines = false;

    cc1->Refresh( false );

}

void MyFrame::SetToolbarItemState( int tool_id, bool state )
{
    if( g_toolbar ) g_toolbar->ToggleTool( tool_id, state );
}

void MyFrame::SetToolbarItemBitmaps( int tool_id, wxBitmap *bmp, wxBitmap *bmpRollover )
{
    if( g_toolbar ) {
        g_toolbar->SetToolBitmaps( tool_id, bmp, bmpRollover );
        wxRect rect = g_toolbar->GetToolRect( tool_id );
        g_toolbar->RefreshRect( rect );
    }
}

void MyFrame::ApplyGlobalSettings( bool bFlyingUpdate, bool bnewtoolbar )
{
    //             ShowDebugWindow as a wxStatusBar
    m_StatusBarFieldCount = 5;

#ifdef __WXMSW__
    UseNativeStatusBar( false );              // better for MSW, undocumented in frame.cpp
#endif

    if( pConfig->m_bShowDebugWindows ) {
        if( !m_pStatusBar ) {
            m_pStatusBar = CreateStatusBar( m_StatusBarFieldCount, 0 );   // No wxST_SIZEGRIP needed
            ApplyGlobalColorSchemetoStatusBar();
            SendSizeEvent();                        // seem only needed for MSW...
        }

    } else {
        if( m_pStatusBar ) {
            m_pStatusBar->Destroy();
            m_pStatusBar = NULL;
            SetStatusBar( NULL );

            SendSizeEvent();                        // seem only needed for MSW...
            Refresh( false );
        }
    }

    if( bnewtoolbar ) UpdateToolbar( global_color_scheme );

}

void MyFrame::SubmergeToolbarIfOverlap( int x, int y, int margin )
{
    if( g_FloatingToolbarDialog ) {
        wxRect rect = g_FloatingToolbarDialog->GetScreenRect();
        rect.Inflate( margin );
        if( rect.Contains( x, y ) ) g_FloatingToolbarDialog->Submerge();
    }
}

void MyFrame::SubmergeToolbar( void )
{
    if( g_FloatingToolbarDialog ) g_FloatingToolbarDialog->Submerge();
}

void MyFrame::SurfaceToolbar( void )
{
    if( g_FloatingToolbarDialog && g_FloatingToolbarDialog->IsToolbarShown() ) {
        if( IsFullScreen() ) {
            if( g_bFullscreenToolbar ) {
                g_FloatingToolbarDialog->Surface();
            }
        } else
            g_FloatingToolbarDialog->Surface();
    }
    gFrame->Raise();
}

void MyFrame::JumpToPosition( double lat, double lon, double scale )
{
    vLat = lat;
    vLon = lon;
    cc1->m_bFollow = false;
    DoChartUpdate();

    cc1->SetViewPoint( lat, lon, scale, 0, cc1->GetVPRotation() );
    cc1->ReloadVP();

    SetToolbarItemState( ID_FOLLOW, false );

//      RequestNewToolbar();
}

int MyFrame::DoOptionsDialog()
{
    static int lastPage = -1;
    static wxPoint lastWindowPos( 0,0 );
    static wxSize lastWindowSize( 0,0 );

    ::wxBeginBusyCursor();
    options optionsDlg( this, -1, _("Options") );
    ::wxEndBusyCursor();

//    Set initial Chart Dir
    optionsDlg.SetInitChartDir( *pInit_Chart_Dir );

//      Pass two working pointers for Chart Dir Dialog
    optionsDlg.SetCurrentDirList( ChartData->GetChartDirArray() );
    ArrayOfCDI *pWorkDirArray = new ArrayOfCDI;
    optionsDlg.SetWorkDirListPtr( pWorkDirArray );

//      Pass a ptr to MyConfig, for updates
    optionsDlg.SetConfigPtr( pConfig );

    optionsDlg.SetInitialSettings();

    bDBUpdateInProgress = true;

    bPrevQuilt = g_bQuiltEnable;
    bPrevFullScreenQuilt = g_bFullScreenQuilt;
    bPrevOGL = g_bopengl;

    prev_locale = g_locale;

    bool b_sub = false;
    if( g_FloatingToolbarDialog && g_FloatingToolbarDialog->IsShown() ) {
        wxRect bx_rect = optionsDlg.GetScreenRect();
        wxRect tb_rect = g_FloatingToolbarDialog->GetScreenRect();
        if( tb_rect.Intersects( bx_rect ) ) b_sub = true;

        if( b_sub ) g_FloatingToolbarDialog->Submerge();
    }

#ifdef __WXOSX__
    if(stats) stats->Hide();
#endif

    if( lastPage >= 0 ) optionsDlg.m_pListbook->SetSelection( lastPage );
    optionsDlg.lastWindowPos = lastWindowPos;
    if( lastWindowPos != wxPoint(0,0) ) {
        optionsDlg.Move( lastWindowPos );
        optionsDlg.SetSize( lastWindowSize );
    } else {
        optionsDlg.Center();
    }

    if( g_FloatingToolbarDialog) g_FloatingToolbarDialog->DisableTooltips();

    int rr = optionsDlg.ShowModal();

    if( g_FloatingToolbarDialog) g_FloatingToolbarDialog->EnableTooltips();

    lastPage = optionsDlg.lastPage;
    lastWindowPos = optionsDlg.lastWindowPos;
    lastWindowSize = optionsDlg.lastWindowSize;

    if( b_sub ) {
        SurfaceToolbar();
        cc1->SetFocus();
    }

#ifdef __WXGTK__
    Raise();                      // I dunno why...
#endif

    bool ret_val = false;
    if( rr ) {
        ProcessOptionsDialog( rr, &optionsDlg );
        ret_val = true;
    }

    delete pWorkDirArray;

    bDBUpdateInProgress = false;
    if( g_FloatingToolbarDialog ) {
        if( IsFullScreen() && !g_bFullscreenToolbar ) g_FloatingToolbarDialog->Submerge();
    }

#ifdef __WXMAC__
    if(stats) stats->Show();
#endif

    Refresh( false );
    return ret_val;
}

int MyFrame::ProcessOptionsDialog( int rr, options* dialog )
{
    ArrayOfCDI *pWorkDirArray = dialog->GetWorkDirListPtr();
    if( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) ) {

        //    Capture the currently open chart
        wxString chart_file_name;
        if( cc1->GetQuiltMode() ) {
            int dbi = cc1->GetQuiltRefChartdbIndex();
            chart_file_name = ChartData->GetDBChartFileName( dbi );
        } else
            if( Current_Ch ) chart_file_name = Current_Ch->GetFullPath();

        UpdateChartDatabaseInplace( *pWorkDirArray, ( ( rr & FORCE_UPDATE ) == FORCE_UPDATE ),
                true, *pChartListFileName );

        //    Re-open the last open chart
        int dbii = ChartData->FinddbIndex( chart_file_name );
        ChartsRefresh( dbii, cc1->GetVP() );
    }

    if( ( rr & LOCALE_CHANGED ) || ( rr & STYLE_CHANGED ) ) {
        if( ( prev_locale != g_locale ) || ( rr & STYLE_CHANGED ) ) {
            OCPNMessageBox(NULL, _("Please restart OpenCPN to activate language or style changes."),
                    _("OpenCPN Info"), wxOK | wxICON_INFORMATION );
            if( rr & LOCALE_CHANGED ) g_blocale_changed = true;;
        }
    }

    if( ( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) )
            || ( rr & GROUPS_CHANGED ) ) {
        ScrubGroupArray();
        ChartData->ApplyGroupArray( g_pGroupArray );
        SetGroupIndex( g_GroupIndex );
    }

    if( rr & GROUPS_CHANGED ) {
        pConfig->DestroyConfigGroups();
        pConfig->CreateConfigGroups( g_pGroupArray );
    }

    if( rr & TIDES_CHANGED ) {
        LoadHarmonics();
    }

    pConfig->UpdateSettings();

    if( g_pActiveTrack ) {
        g_pActiveTrack->SetPrecision( g_nTrackPrecision );
    }

    if( ( bPrevQuilt != g_bQuiltEnable ) || ( bPrevFullScreenQuilt != g_bFullScreenQuilt ) ) {
        cc1->SetQuiltMode( g_bQuiltEnable );
        SetupQuiltMode();
    }

    if( g_bCourseUp ) {
        //    Stuff the COGAvg table in case COGUp is selected
        double stuff = 0.;
        if( !wxIsNaN(gCog) ) stuff = gCog;
        for( int i = 0; i < g_COGAvgSec; i++ )
            COGTable[i] = stuff;

        g_COGAvg = stuff;

        //    Short circuit the COG timer to force immediate refresh of canvas in case COGUp is selected
        FrameCOGTimer.Stop();
        FrameCOGTimer.Start( 100, wxTIMER_CONTINUOUS );
    }

    //    Stuff the Filter tables
    double stuffcog = 0.;
    double stuffsog = 0.;
    if( !wxIsNaN(gCog) ) stuffcog = gCog;
    if( !wxIsNaN(gSog) ) stuffsog = gSog;

    for( int i = 0; i < MAX_COGSOG_FILTER_SECONDS; i++ ) {
        COGFilterTable[i] = stuffcog;
        SOGFilterTable[i] = stuffsog;
    }
    m_COGFilterLast = stuffcog;

    SetChartUpdatePeriod( cc1->GetVP() );              // Pick up changes to skew compensator

#if 0
    bDBUpdateInProgress = false;

    if( g_FloatingToolbarDialog ) {
        if( IsFullScreen() && !g_bFullscreenToolbar ) g_FloatingToolbarDialog->Submerge();
    }

#ifdef __WXMAC__
    if(stats) stats->Show();
#endif

    Refresh( false );
#endif
    return 0;
}

wxString MyFrame::GetGroupName( int igroup )
{
    ChartGroup *pGroup = g_pGroupArray->Item( igroup - 1 );
    return pGroup->m_group_name;
}

bool MyFrame::CheckGroup( int igroup )
{
    if( igroup == 0 ) return true;              // "all charts" is always OK

    ChartGroup *pGroup = g_pGroupArray->Item( igroup - 1 );
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

    return b_chart_in_group;                           // this group is empty

}

void MyFrame::ScrubGroupArray()
{
    //    For each group,
    //    make sure that each group element (dir or chart) references at least oneitem in the database.
    //    If not, remove the element.

    unsigned int igroup = 0;
    while( igroup < g_pGroupArray->GetCount() ) {
        bool b_chart_in_element = false;
        ChartGroup *pGroup = g_pGroupArray->Item( igroup );

        for( unsigned int j = 0; j < pGroup->m_element_array.GetCount(); j++ ) {
            wxString element_root = pGroup->m_element_array.Item( j )->m_element_name;

            for( unsigned int ic = 0; ic < (unsigned int) ChartData->GetChartTableEntries();
                    ic++ ) {
                ChartTableEntry *pcte = ChartData->GetpChartTableEntry( ic );
                wxString chart_full_path( pcte->GetpFullPath(), wxConvUTF8 );

                if( chart_full_path.StartsWith( element_root ) ) {
                    b_chart_in_element = true;
                    break;
                }
            }

            if( !b_chart_in_element )             // delete the element
            {
                ChartGroupElement *pelement = pGroup->m_element_array.Item( j );
                pGroup->m_element_array.RemoveAt( j );
                delete pelement;
            }
        }

        igroup++;                                 // next group
    }
}

// Flav: This method reloads all charts for convenience
void MyFrame::ChartsRefresh( int dbi_hint, ViewPort &vp, bool b_purge )
{
    if( !ChartData ) return;

    ::wxBeginBusyCursor();

    bool b_run = FrameTimer1.IsRunning();

    FrameTimer1.Stop();                  // stop other asynchronous activity

    cc1->InvalidateQuilt();
    cc1->SetQuiltRefChart( -1 );

    Current_Ch = NULL;

    delete pCurrentStack;
    pCurrentStack = NULL;

    if( b_purge ) ChartData->PurgeCache();

    //    Build a new ChartStack
    pCurrentStack = new ChartStack;
    ChartData->BuildChartStack( pCurrentStack, vLat, vLon );

    if( -1 != dbi_hint ) {
        if( cc1->GetQuiltMode() ) {
            pCurrentStack->SetCurrentEntryFromdbIndex( dbi_hint );
            cc1->SetQuiltRefChart( dbi_hint );
        } else {
            //      Open the saved chart
            ChartBase *pTentative_Chart;
            pTentative_Chart = ChartData->OpenChartFromDB( dbi_hint, FULL_INIT );

            if( pTentative_Chart ) {
                if( Current_Ch ) Current_Ch->Deactivate();

                Current_Ch = pTentative_Chart;
                Current_Ch->Activate();

                pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                        Current_Ch->GetFullPath() );
            } else
                SetChartThumbnail( dbi_hint );       // need to reset thumbnail on failed chart open
        }

        //          Refresh the Piano Bar
        if( stats ) {
            ArrayOfInts piano_active_chart_index_array;
            piano_active_chart_index_array.Add( pCurrentStack->GetCurrentEntrydbIndex() );
            stats->pPiano->SetActiveKeyArray( piano_active_chart_index_array );

            stats->Refresh( true );
        }

    } else {
        //    Select reference chart from the stack, as though clicked by user
        //    Make it the smallest scale chart on the stack
        pCurrentStack->CurrentStackEntry = pCurrentStack->nEntry - 1;
        int selected_index = pCurrentStack->GetCurrentEntrydbIndex();
        cc1->SetQuiltRefChart( selected_index );
    }

    //    Validate the correct single chart, or set the quilt mode as appropriate
    SetupQuiltMode();

    if( vp.IsValid() ) cc1->LoadVP( vp );
    else
        cc1->ReloadVP();

    UpdateControlBar();

    UpdateGPSCompassStatusBox( true );

    cc1->SetCursor( wxCURSOR_ARROW );

    if( b_run ) FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

    ::wxEndBusyCursor();

}

bool MyFrame::UpdateChartDatabaseInplace( ArrayOfCDI &DirArray, bool b_force, bool b_prog,
        wxString &ChartListFileName )
{
    bool b_run = FrameTimer1.IsRunning();
    FrameTimer1.Stop();                  // stop other asynchronous activity

    cc1->InvalidateQuilt();
    cc1->SetQuiltRefChart( -1 );

    Current_Ch = NULL;

    delete pCurrentStack;
    pCurrentStack = NULL;

    ::wxBeginBusyCursor();

    wxProgressDialog *pprog = NULL;
    if( b_prog ) {
        pprog = new wxProgressDialog( _("OpenCPN Chart Update"), _T(""), 100, this,
                wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );

        //    Make sure the dialog is big enough to be readable
        pprog->Hide();
        wxSize sz = pprog->GetSize();
        wxSize csz = GetClientSize();
        sz.x = csz.x * 7 / 10;
        pprog->SetSize( sz );
        pprog->Centre();
        pprog->Update( 1, _T("") );
        pprog->Show();
        pprog->Raise();
    }

    wxLogMessage( _T("   ") );
    wxLogMessage( _T("Starting chart database Update...") );
    ChartData->Update( DirArray, b_force, pprog );
    ChartData->SaveBinary( &ChartListFileName );
    wxLogMessage( _T("Finished chart database Update") );
    wxLogMessage( _T("   ") );

    delete pprog;

    ::wxEndBusyCursor();

    pConfig->UpdateChartDirs( DirArray );

    if( b_run ) FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

    return true;
}

void MyFrame::ToggleQuiltMode( void )
{
    if( cc1 ) {
        bool cur_mode = cc1->GetQuiltMode();

        if( !cc1->GetQuiltMode() && g_bQuiltEnable ) cc1->SetQuiltMode( true );
        else
            if( cc1->GetQuiltMode() ) {
                cc1->SetQuiltMode( false );
                g_sticky_chart = cc1->GetQuiltReferenceChartIndex();
            }


        if( cur_mode != cc1->GetQuiltMode() ) SetupQuiltMode();
    }
}

void MyFrame::SetQuiltMode( bool bquilt )
{
    if( cc1 ) cc1->SetQuiltMode( bquilt );
}

bool MyFrame::GetQuiltMode( void )
{
    if( cc1 ) return cc1->GetQuiltMode();
    else
        return false;
}

void MyFrame::SetupQuiltMode( void )
{

    if( cc1->GetQuiltMode() )                               // going to quilt mode
    {
        ChartData->LockCache();

        stats->pPiano->SetNoshowIndexArray( g_quilt_noshow_index_array );

        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

        stats->pPiano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
        stats->pPiano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );
        stats->pPiano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
        stats->pPiano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

        stats->pPiano->SetRoundedRectangles( true );

        //    Select the proper Ref chart
        int target_new_dbindex = -1;
        if( pCurrentStack ) {
            target_new_dbindex = pCurrentStack->GetCurrentEntrydbIndex();

#ifdef QUILT_ONLY_MERC
            if(-1 != target_new_dbindex)
            {
                //    Check to see if the target new chart is Merc
                int proj = ChartData->GetDBChartProj(target_new_dbindex);
                int type = ChartData->GetDBChartType(target_new_dbindex);

                if(PROJECTION_MERCATOR != proj)
                {
                    // If it is not Merc, cannot use it for quilting
                    // walk the stack up looking for a satisfactory chart
                    int stack_index = pCurrentStack->CurrentStackEntry;

                    while((stack_index < pCurrentStack->nEntry-1) && (stack_index >= 0))
                    {
                        int proj_tent = ChartData->GetDBChartProj( pCurrentStack->GetDBIndex(stack_index));
                        int type_tent = ChartData->GetDBChartType( pCurrentStack->GetDBIndex(stack_index));

                        if((PROJECTION_MERCATOR ==proj_tent) && (type_tent == type))
                        {
                            target_new_dbindex = pCurrentStack->GetDBIndex(stack_index);
                            break;
                        }
                        stack_index++;
                    }
                }

            }
#endif
        }

        if( cc1->IsChartQuiltableRef( target_new_dbindex ) ) SelectQuiltRefdbChart(
                target_new_dbindex );
        else
            SelectQuiltRefdbChart( -1 );

        Current_Ch = NULL;                  // Bye....
        cc1->ReloadVP();

    } else                                                  // going to SC Mode
    {
        ArrayOfInts empty_array;
        stats->pPiano->SetActiveKeyArray( empty_array );
        stats->pPiano->SetNoshowIndexArray( empty_array );
        stats->pPiano->SetSubliteIndexArray( empty_array );
        stats->pPiano->SetVizIcon( NULL );
        stats->pPiano->SetInVizIcon( NULL );

        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

        stats->pPiano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
        stats->pPiano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

        stats->pPiano->SetRoundedRectangles( false );

    }

    //    When shifting from quilt to single chart mode, select the "best" single chart to show
    if( !cc1->GetQuiltMode() ) {
        if( ChartData && ChartData->IsValid() ) {
            ChartData->UnLockCache();

            double tLat, tLon;
            if( cc1->m_bFollow == true ) {
                tLat = gLat;
                tLon = gLon;
            } else {
                tLat = vLat;
                tLon = vLon;
            }

            if( !Current_Ch ) {

                // Build a temporary chart stack based on tLat, tLon
                ChartStack TempStack;
                ChartData->BuildChartStack( &TempStack, tLat, tLon, g_sticky_chart );

                //    Iterate over the quilt charts actually shown, looking for the largest scale chart that will be in the new chartstack....
                //    This will (almost?) always be the reference chart....

                ChartBase *Candidate_Chart = NULL;
                int cur_max_scale = (int) 1e8;

                ChartBase *pChart = cc1->GetFirstQuiltChart();
                while( pChart ) {
                    //  Is this pChart in new stack?
                    int tEntry = ChartData->GetStackEntry( &TempStack, pChart->GetFullPath() );
                    if( tEntry != -1 ) {
                        if( pChart->GetNativeScale() < cur_max_scale ) {
                            Candidate_Chart = pChart;
                            cur_max_scale = pChart->GetNativeScale();
                        }
                    }
                    pChart = cc1->GetNextQuiltChart();
                }

                Current_Ch = Candidate_Chart;

                //    If the quilt is empty, there is no "best" chart.
                //    So, open the smallest scale chart in the current stack
                if( NULL == Current_Ch ) {
                    Current_Ch = ChartData->OpenStackChartConditional( &TempStack,
                            TempStack.nEntry - 1, true, CHART_TYPE_DONTCARE,
                            CHART_FAMILY_DONTCARE );
                }
            }

            //  Invalidate all the charts in the quilt,
            // as any cached data may be region based and not have fullscreen coverage
            cc1->InvalidateAllQuiltPatchs();

            if( Current_Ch ) {
                int dbi = ChartData->FinddbIndex( Current_Ch->GetFullPath() );
                ArrayOfInts one_array;
                one_array.Add( dbi );
                stats->pPiano->SetActiveKeyArray( one_array );
            }

        }
        //    Invalidate the current stack so that it will be rebuilt on next tick
        if( pCurrentStack ) pCurrentStack->b_valid = false;
    }

}

void MyFrame::ClearRouteTool()
{
    if( g_toolbar ) g_toolbar->ToggleTool( ID_ROUTE, false );
}

void MyFrame::DoStackDown( void )
{
    int current_stack_index = pCurrentStack->CurrentStackEntry;

    if( 0 == current_stack_index ) return;

    if( !cc1->GetQuiltMode() ) SelectChartFromStack( current_stack_index - 1 );
    else {
        int new_dbIndex = pCurrentStack->GetDBIndex( current_stack_index - 1 );

        if( !cc1->IsChartQuiltableRef( new_dbIndex ) ) {
            ToggleQuiltMode();
            SelectChartFromStack( current_stack_index - 1 );
        } else
            SelectQuiltRefChart( current_stack_index - 1 );

    }

    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();
}

void MyFrame::DoStackUp( void )
{
    int current_stack_index = pCurrentStack->CurrentStackEntry;

    if( current_stack_index >= pCurrentStack->nEntry - 1 ) return;

    if( !cc1->GetQuiltMode() ) {
        SelectChartFromStack( current_stack_index + 1 );
    } else {
        int new_dbIndex = pCurrentStack->GetDBIndex( current_stack_index + 1 );

        if( !cc1->IsChartQuiltableRef( new_dbIndex ) ) {
            ToggleQuiltMode();
            SelectChartFromStack( current_stack_index + 1 );
        } else
            SelectQuiltRefChart( current_stack_index + 1 );
    }

    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();
}

//    Manage the application memory footprint on a periodic schedule
void MyFrame::OnMemFootTimer( wxTimerEvent& event )
{
    MemFootTimer.Stop();

    int memsize = GetApplicationMemoryUse();

    g_MemFootMB = 100;
    printf( "Memsize: %d  \n", memsize );
    // The application memory usage has exceeded the target, so try to manage it down....
    if( memsize > ( g_MemFootMB * 1000 ) ) {
        if( ChartData && cc1 ) {
            //    Get a local copy of the cache info
            wxArrayPtrVoid *pCache = ChartData->GetChartCache();
            unsigned int nCache = pCache->GetCount();
            CacheEntry *pcea = new CacheEntry[nCache];

            for( unsigned int i = 0; i < nCache; i++ ) {
                CacheEntry *pce = (CacheEntry *) ( pCache->Item( i ) );
                pcea[i] = *pce;                  //ChartBase *Ch = (ChartBase *)pce->pChart;
            }

            if( nCache > 1 ) {
                //    Bubble Sort the local cache entry array
                bool b_cont = true;
                while( b_cont ) {
                    b_cont = false;
                    for( unsigned int i = 0; i < nCache - 1; i++ ) {
                        if( pcea[i].RecentTime > pcea[i + 1].RecentTime ) {
                            CacheEntry tmp = pcea[i];
                            pcea[i] = pcea[i + 1];
                            pcea[i + 1] = tmp;
                            b_cont = true;
                            break;
                        }
                    }
                }

                //    Free up some chart cache entries until the memory footprint target is realized

                unsigned int idelete = 0;                 // starting at top. which is oldest
                unsigned int idelete_max = pCache->GetCount();

                //    How many can be deleted?
                unsigned int minimum_cache = 1;
                if( cc1->GetQuiltMode() ) minimum_cache = cc1->GetQuiltChartCount();

                while( ( memsize > ( g_MemFootMB * 1000 ) )
                        && ( pCache->GetCount() > minimum_cache ) && ( idelete < idelete_max ) ) {
                    int memsizeb = memsize;

                    ChartData->DeleteCacheChart( (ChartBase *) pcea[idelete].pChart );
                    idelete++;
                    memsize = GetApplicationMemoryUse();
                    printf( "delete, before: %d  after: %d\n", memsizeb, memsize );
                }

            }

            delete[] pcea;
        }

    }

//      MemFootTimer.Start(wxMax(g_MemFootSec * 1000, 60 * 1000), wxTIMER_CONTINUOUS);
    MemFootTimer.Start( 9000, wxTIMER_CONTINUOUS );
}

int ut_index;

void MyFrame::OnFrameTimer1( wxTimerEvent& event )
{

    if( s_ProgDialog ) {
        return;
    }

    if( g_unit_test_1 ) {
//            if((0 == ut_index) && GetQuiltMode())
//                  ToggleQuiltMode();

        cc1->m_bFollow = false;
        if( g_toolbar ) g_toolbar->ToggleTool( ID_FOLLOW, cc1->m_bFollow );

        if( ChartData ) {
            if( ut_index < ChartData->GetChartTableEntries() ) {
                const ChartTableEntry *cte = &ChartData->GetChartTableEntry( ut_index );
                double lat = ( cte->GetLatMax() + cte->GetLatMin() ) / 2;
                double lon = ( cte->GetLonMax() + cte->GetLonMin() ) / 2;

                vLat = lat;
                vLon = lon;

                cc1->SetViewPoint( lat, lon );

                if( cc1->GetQuiltMode() ) {
                    if( cc1->IsChartQuiltableRef( ut_index ) ) SelectQuiltRefdbChart( ut_index );
                } else
                    SelectdbChart( ut_index );

                double ppm = cc1->GetCanvasScaleFactor() / cte->GetScale();
                ppm /= 2;
                cc1->SetVPScale( ppm );

                cc1->ReloadVP();

                ut_index++;
            }
        }
    }
    g_tick++;

#ifdef __WXOSX__
    //    To fix an ugly bug ?? in wxWidgets for Carbon.....
    //    Or, maybe this is the way Macs work....
    //    Hide some non-UI Dialogs if the application is minimized....
    //    They will be re-Show()-n in MyFrame::OnActivate()
    if(IsIconized())
    {
        if(g_FloatingToolbarDialog) {
            if(g_FloatingToolbarDialog->IsShown())
            g_FloatingToolbarDialog->Submerge();
        }

        if(console && console->IsShown()) {
            console->Hide();
        }

        if(g_FloatingCompassDialog && g_FloatingCompassDialog->IsShown()) {
            g_FloatingCompassDialog->Hide();
        }

        if(stats && stats->IsShown()) {
            stats->Hide();
        }
    }
#endif

//      Listen for quitflag to be set, requesting application close
    if( quitflag ) {
        wxLogMessage( _T("Got quitflag from SIGUSR1") );
        FrameTimer1.Stop();
        Close();
        return;
    }

    if( bDBUpdateInProgress ) return;

    FrameTimer1.Stop();

//    Manage the brightness dialog timeout
    if( g_brightness_timeout > 0 ) {
        g_brightness_timeout--;

        if( g_brightness_timeout == 0 ) {
            g_pbrightness_indicator_dialog->Destroy();
            g_pbrightness_indicator_dialog = NULL;
        }
    }

//  Update and check watchdog timer for GPS data source
    gGPS_Watchdog--;
    if( gGPS_Watchdog <= 0 ) {
        bGPSValid = false;
        if( g_nNMEADebug && ( gGPS_Watchdog == 0 ) ) wxLogMessage(
                _T("   ***GPS Watchdog timeout...") );
    }

//  Update and check watchdog timer for Mag Heading data source
    gHDx_Watchdog--;
    if( gHDx_Watchdog <= 0 ) {
        g_bHDxValid = false;
        gHdm = NAN;
        if( g_nNMEADebug && ( gHDx_Watchdog == 0 ) ) wxLogMessage(
                _T("   ***HDx Watchdog timeout...") );
    }

//  Update and check watchdog timer for True Heading data source
    gHDT_Watchdog--;
    if( gHDT_Watchdog <= 0 ) {
        g_bHDT_Rx = false;
        if( g_nNMEADebug && ( gHDT_Watchdog == 0 ) ) wxLogMessage(
                _T("   ***HDT Watchdog timeout...") );
    }

    //  Update and check watchdog timer for Magnetic Variation data source
    gVAR_Watchdog--;
    if( gVAR_Watchdog <= 0 ) {
        g_bVAR_Rx = false;
        if( g_nNMEADebug && ( gVAR_Watchdog == 0 ) ) wxLogMessage(
            _T("   ***VAR Watchdog timeout...") );
    }
    //  Update and check watchdog timer for GSV (Satellite data)
    gSAT_Watchdog--;
    if( gSAT_Watchdog <= 0 ) {
        g_bSatValid = false;
        if( g_nNMEADebug && ( gSAT_Watchdog == 0 ) ) wxLogMessage(
                _T("   ***SAT Watchdog timeout...") );
    }

    //   Check for anchorwatch alarms                                 // pjotrc 2010.02.15
    if( pAnchorWatchPoint1 ) {
        double dist;
        double brg;
        DistanceBearingMercator( pAnchorWatchPoint1->m_lat, pAnchorWatchPoint1->m_lon, gLat, gLon,
                &brg, &dist );
        double d = g_nAWMax;
        ( pAnchorWatchPoint1->GetName() ).ToDouble( &d );
        d = AnchorDistFix( d, AnchorPointMinDist, g_nAWMax );
        bool toofar = false;
        bool tooclose = false;
        if( d >= 0.0 ) toofar = ( dist * 1852. > d );
        if( d < 0.0 ) tooclose = ( dist * 1852 < -d );

        if( tooclose || toofar ) AnchorAlertOn1 = true;
        else
            AnchorAlertOn1 = false;
    } else
        AnchorAlertOn1 = false;

    if( pAnchorWatchPoint2 ) {
        double dist;
        double brg;
        DistanceBearingMercator( pAnchorWatchPoint2->m_lat, pAnchorWatchPoint2->m_lon, gLat, gLon,
                &brg, &dist );

        double d = g_nAWMax;
        ( pAnchorWatchPoint2->GetName() ).ToDouble( &d );
        d = AnchorDistFix( d, AnchorPointMinDist, g_nAWMax );
        bool toofar = false;
        bool tooclose = false;
        if( d >= 0 ) toofar = ( dist * 1852. > d );
        if( d < 0 ) tooclose = ( dist * 1852 < -d );

        if( tooclose || toofar ) AnchorAlertOn2 = true;
        else
            AnchorAlertOn2 = false;
    } else
        AnchorAlertOn2 = false;

//  Send current nav status data to log file on every half hour   // pjotrc 2010.02.09

    wxDateTime lognow = wxDateTime::Now();   // pjotrc 2010.02.09
    int hour = lognow.GetHour();
    lognow.MakeGMT();
    int minute = lognow.GetMinute();
    wxTimeSpan logspan = lognow.Subtract( g_loglast_time );
    if( ( logspan.IsLongerThan( wxTimeSpan( 0, 30, 0, 0 ) ) ) || ( minute == 0 )
            || ( minute == 30 ) ) {
        if( logspan.IsLongerThan( wxTimeSpan( 0, 1, 0, 0 ) ) ) {
            wxString day = lognow.FormatISODate();
            wxString utc = lognow.FormatISOTime();
            wxString navmsg = _T("LOGBOOK:  ");
            navmsg += day;
            navmsg += _T(" ");
            navmsg += utc;
            navmsg += _T(" UTC ");

            if( bGPSValid ) {
                wxString data;
                data.Printf( _T(" GPS Lat %10.5f Lon %10.5f "), gLat, gLon );
                navmsg += data;

                wxString cog;
                if( wxIsNaN(gCog) ) cog.Printf( _T("COG ----- ") );
                else
                    cog.Printf( _T("COG %10.5f "), gCog );

                wxString sog;
                if( wxIsNaN(gSog) ) sog.Printf( _T("SOG -----  ") );
                else
                    sog.Printf( _T("SOG %6.2f"), gSog );

                navmsg += cog;
                navmsg += sog;
            } else {
                wxString data;
                data.Printf( _T(" DR Lat %10.5f Lon %10.5f"), gLat, gLon );
                navmsg += data;
            }
            wxLogMessage( navmsg );
            g_loglast_time = lognow;

            if( hour == 0 && minute == 0 && g_bTrackDaily ) TrackMidnightRestart();

            int bells = ( hour % 4 ) * 2;     // 2 bells each hour
            if( minute != 0 ) bells++;       // + 1 bell on 30 minutes
            if( !bells ) bells = 8;     // 0 is 8 bells

            if( g_bPlayShipsBells && ( ( minute == 0 ) || ( minute == 30 ) ) ) {
                if( !bells_sound[bells - 1].IsOk() )            // load the bells sound
                {
                    wxString soundfile = _T("sounds");
                    appendOSDirSlash( &soundfile );
                    soundfile += wxString( bells_sound_file_name[bells - 1], wxConvUTF8 );
                    soundfile.Prepend( g_SData_Locn );
                    bells_sound[bells - 1].Create( soundfile );
                    wxLogMessage( _T("Using bells sound file: ") + soundfile );

                }

                if( bells_sound[bells - 1].IsOk() ) bells_sound[bells - 1].Play();
            }
        }
    }

//      Update the Toolbar Status windows and lower status bar the first time watchdog times out
    if( ( gGPS_Watchdog == 0 ) || ( gSAT_Watchdog == 0 ) ) {
        wxString sogcog( wxString("SOG --- kts  COG ---°", wxConvUTF8 ) );
        if( GetStatusBar() ) SetStatusText( sogcog, STAT_FIELD_SOGCOG );

        gCog = 0.0;                                 // say speed is zero to kill ownship predictor
    }

    if( cc1 ) {
        double cursor_lat, cursor_lon;
        cc1->GetCursorLatLon( &cursor_lat, &cursor_lon );

        wxString s1;
        s1 += _T(" ");
        s1 += toSDMM( 1, cursor_lat );
        s1 += _T("   ");
        s1 += toSDMM( 2, cursor_lon );
        if( GetStatusBar() ) SetStatusText( s1, STAT_FIELD_CURSOR_LL );

        double brg, dist;
        DistanceBearingMercator( cursor_lat, cursor_lon, gLat, gLon, &brg, &dist );
        wxString s;
        s.Printf( wxString("%03d°  ", wxConvUTF8 ), (int) brg );
        s << cc1->FormatDistanceAdaptive( dist );
        if( GetStatusBar() ) SetStatusText( s, STAT_FIELD_CURSOR_BRGRNG );
    }

//      Update the chart database and displayed chart
    bool bnew_view = false;

//    Do the chart update based on the global update period currently set
//    If in COG UP mode, the chart update is handled by COG Update timer
    if( !g_bCourseUp && ( 0 == m_ChartUpdatePeriod-- ) ) {
        bnew_view = DoChartUpdate();
        m_ChartUpdatePeriod = g_ChartUpdatePeriod;
    }

//      Update the active route, if any
    if( g_pRouteMan->UpdateProgress() ) {
        nBlinkerTick++;
        //    This RefreshRect will cause any active routepoint to blink
        if( g_pRouteMan->GetpActiveRoute() ) cc1->RefreshRect( g_blink_rect, false );
    }

//  Possibly save the current configuration
    if( 0 == ( g_tick % ( g_nautosave_interval_seconds ) ) ) {
        pConfig->UpdateSettings();
        pConfig->UpdateNavObj();
    }

//  Force own-ship drawing parameters
    cc1->SetOwnShipState( SHIP_NORMAL );

    if( cc1->GetQuiltMode() ) {
        double erf = cc1->GetQuiltMaxErrorFactor();
        if( erf > 0.02 ) cc1->SetOwnShipState( SHIP_LOWACCURACY );
    } else {
        if( Current_Ch ) {
            if( Current_Ch->GetChart_Error_Factor() > 0.02 ) cc1->SetOwnShipState(
                    SHIP_LOWACCURACY );
        }
    }

    if( !bGPSValid ) {
        cc1->SetOwnShipState( SHIP_INVALID );
        if( cc1->m_bFollow ) cc1->UpdateShips();
    }

    if( bGPSValid != m_last_bGPSValid ) {
        cc1->UpdateShips();
        bnew_view = true;                  // force a full Refresh()
        m_last_bGPSValid = bGPSValid;
    }

    //    If any PlugIn requested dynamic overlay callbacks, force a full canvas refresh
    //    thus, ensuring at least 1 Hz. callback.
    bool brq_dynamic = false;
    if( g_pi_manager ) {
        ArrayOfPlugIns *pplugin_array = g_pi_manager->GetPlugInArray();
        for( unsigned int i = 0; i < pplugin_array->GetCount(); i++ ) {
            PlugInContainer *pic = pplugin_array->Item( i );
            if( pic->m_bEnabled && pic->m_bInitState ) {
                if( pic->m_cap_flag & WANTS_DYNAMIC_OPENGL_OVERLAY_CALLBACK ) {
                    brq_dynamic = true;
                    break;
                }
            }
        }
    }

    if( brq_dynamic ) {
        cc1->Refresh();
        bnew_view = true;
    }

    FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

//  Invalidate the ChartCanvas window appropriately
//    In non-follow mode, invalidate the rectangles containing the AIS targets and the ownship, etc...
//    In follow mode, if there has already been a full screen refresh, there is no need to check ownship or AIS,
//       since they will be always drawn on the full screen paint.
    if( ( !cc1->m_bFollow ) || g_bCourseUp ) {
        cc1->UpdateShips();
        cc1->UpdateAIS();
        cc1->UpdateAlerts();
    } else {
        if( !bnew_view )                    // There has not been a Refresh() yet.....
        {
            cc1->UpdateAIS();
            cc1->UpdateAlerts();
        }
    }

    if( g_pais_query_dialog_active && g_pais_query_dialog_active->IsShown() ) g_pais_query_dialog_active->UpdateText();

    // Refresh AIS target list every 5 seconds to avoid blinking
    if( g_pAISTargetList && ( 0 == ( g_tick % ( 5 ) ) ) ) g_pAISTargetList->UpdateAISTargetList();

    //  Pick up any change Toolbar status displays
    UpdateGPSCompassStatusBox();
    UpdateAISTool();

    if( console && console->IsShown() ) {
//            console->Raise();
        console->RefreshConsoleData();
    }

    //  This little hack fixes a problem seen with some UniChrome OpenGL drivers
    //  We need a deferred resize to get glDrawPixels() to work right.
    //  So we set a trigger to generate a resize after 5 seconds....
    //  See the "UniChrome" hack elsewhere
    if( m_bdefer_resize ) {
        if( 0 == ( g_tick % ( 5 ) ) ) {
            printf( "___RESIZE\n" );
            SetSize( m_defer_size );
            g_pauimgr->Update();
            m_bdefer_resize = false;
        }
    }
}

void MyFrame::TouchAISActive( void )
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    if( m_pAISTool ) {
        if( ( !g_pAIS->IsAISSuppressed() ) && ( !g_pAIS->IsAISAlertGeneral() ) ) {
            g_nAIS_activity_timer = 5;                // seconds

            wxString iconName = _T("AIS_Normal_Active");
            if( g_pAIS->IsAISAlertGeneral() ) iconName = _T("AIS_AlertGeneral_Active");
            if( g_pAIS->IsAISSuppressed() ) iconName = _T("AIS_Suppressed_Active");

            if( m_lastAISiconName != iconName ) {
                int flag = TOOLICON_NORMAL;
                if( m_pAISTool->IsToggled() ) flag = TOOLICON_TOGGLED;
                m_pAISTool->SetNormalBitmap( style->GetToolIcon( iconName, flag ) );
                g_toolbar->Refresh();
                m_lastAISiconName = iconName;
            }
        }
    }
}

void MyFrame::UpdateAISTool( void )
{
    if(!g_pAIS) return;

    bool b_need_refresh = false;
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    wxString iconName;

    if( m_pAISTool ) {
        bool b_update = false;

        iconName = _T("AIS");
        if( g_pAIS->IsAISSuppressed() ) iconName = _T("AIS_Suppressed");
        if( g_pAIS->IsAISAlertGeneral() ) iconName = _T("AIS_AlertGeneral");

        //  Manage timeout for AIS activity indicator
        if( g_nAIS_activity_timer ) {
            g_nAIS_activity_timer--;

            if( 0 == g_nAIS_activity_timer ) b_update = true;
            else {
                iconName = _T("AIS_Normal_Active");
                if( g_pAIS->IsAISSuppressed() ) iconName = _T("AIS_Suppressed_Active");
                if( g_pAIS->IsAISAlertGeneral() ) iconName = _T("AIS_AlertGeneral_Active");

                if( ( m_lastAISiconName != iconName ) ) b_update = true;
            }

        } else {
            if( ( m_lastAISiconName != iconName ) ) b_update = true;
        }

        if( b_update ) {
            int flag = TOOLICON_NORMAL;
            if( m_pAISTool->IsToggled() ) flag = TOOLICON_TOGGLED;
            m_pAISTool->SetNormalBitmap( style->GetToolIcon( iconName, flag ) );
            b_need_refresh = true;
        }

    }

    if( b_need_refresh ) {
        g_toolbar->Refresh();
        m_lastAISiconName = iconName;
    }
}

//    Cause refresh of active Tide/Current data, if displayed
void MyFrame::OnFrameTCTimer( wxTimerEvent& event )
{
    if( cc1 ) {
        cc1->SetbTCUpdate( true );
        cc1->Refresh( false );
    }
}

//    Keep and update the Viewport rotation angle according to average COG for COGUP mode
void MyFrame::OnFrameCOGTimer( wxTimerEvent& event )
{
//      return;
    FrameCOGTimer.Stop();

    DoCOGSet();

    //    Restart the timer
    FrameCOGTimer.Start( g_COGAvgSec * 1000, wxTIMER_CONTINUOUS );
}

void MyFrame::DoCOGSet( void )
{
    double old_VPRotate = g_VPRotate;

    if( g_bCourseUp ) g_VPRotate = -g_COGAvg * PI / 180.;
    else
        g_VPRotate = 0.;

    if( cc1 ) cc1->SetVPRotation( g_VPRotate );

    if( g_bCourseUp ) {
        bool bnew_chart = DoChartUpdate();

        if( ( bnew_chart ) || ( old_VPRotate != g_VPRotate ) ) if( cc1 ) cc1->ReloadVP();
    }
}

void RenderShadowText( wxDC *pdc, wxFont *pFont, wxString& str, int x, int y )
{
#ifdef DrawText
#undef DrawText
#define FIXIT
#endif

    wxFont oldfont = pdc->GetFont(); // save current font

    pdc->SetFont( *pFont );
    pdc->SetTextForeground( GetGlobalColor( _T("CHGRF") ) );
    pdc->SetBackgroundMode( wxTRANSPARENT );

    pdc->DrawText( str, x, y + 1 );
    pdc->DrawText( str, x, y - 1 );
    pdc->DrawText( str, x + 1, y );
    pdc->DrawText( str, x - 1, y );

    pdc->SetTextForeground( GetGlobalColor( _T("CHBLK") ) );

    pdc->DrawText( str, x, y );

    pdc->SetFont( oldfont );                  // restore last font

}

void MyFrame::UpdateGPSCompassStatusBox( bool b_force_new )
{
    if( !g_FloatingCompassDialog ) return;

    //    Look for overlap
    bool b_update = false;
    if( g_FloatingCompassDialog && g_FloatingToolbarDialog ) {
        int x_offset = g_FloatingCompassDialog->GetXOffset();
        int y_offset = g_FloatingCompassDialog->GetYOffset();
        int cc1_edge_comp = 2;

        // check to see if it would overlap if it was in its home position (upper right)
        wxSize parent_size = g_FloatingCompassDialog->GetParent()->GetSize();
        wxPoint tentative_pt_in_screen = g_FloatingCompassDialog->GetParent()->ClientToScreen(
                wxPoint(
                        parent_size.x - g_FloatingCompassDialog->GetSize().x
                        - x_offset - cc1_edge_comp, y_offset ) );
        wxRect tentative_rect( tentative_pt_in_screen.x, tentative_pt_in_screen.y,
                g_FloatingCompassDialog->GetSize().x, g_FloatingCompassDialog->GetSize().y );

        wxRect tb_rect = g_FloatingToolbarDialog->GetScreenRect();

        //    if they would not intersect, go ahead and move it to the upper right
        //      Else it has to be on lower right
        if( !tb_rect.Intersects( tentative_rect ) ) {
            g_FloatingCompassDialog->Move( tentative_pt_in_screen );
        }
        else {
            wxPoint posn_in_canvas =
                wxPoint(
                    cc1->GetSize().x - g_FloatingCompassDialog->GetSize().x - x_offset - cc1_edge_comp,
                    cc1->GetSize().y - ( g_FloatingCompassDialog->GetSize().y + y_offset + cc1_edge_comp ) );
            g_FloatingCompassDialog->Move( cc1->ClientToScreen( posn_in_canvas ) );
        }

        b_update = true;
    }

    if( g_FloatingCompassDialog && g_FloatingCompassDialog->IsShown()) {
        g_FloatingCompassDialog->UpdateStatus( b_force_new | b_update );
        g_FloatingCompassDialog->Update();
    }
}

int MyFrame::GetnChartStack( void )
{
    return pCurrentStack->nEntry;
}

//    Application memory footprint management

int MyFrame::GetApplicationMemoryUse( void )
{
    int memsize = -1;
#ifdef __LINUX__

    //    Use a contrived ps command to get the virtual memory size associated with this process
    wxWindow *fWin = wxWindow::FindFocus();

    wxArrayString outputArray;
    wxString cmd(_T("ps --no-headers -o vsize "));
    unsigned long pid = wxGetProcessId();
    wxString cmd1;
    cmd1.Printf(_T("%ld"), pid);
    cmd += cmd1;
    wxExecute(cmd, outputArray);

    if(outputArray.GetCount())
    {
        wxString s = outputArray.Item(0);
        long vtmp;
        if(s.ToLong(&vtmp))
        memsize = vtmp;
    }

    if(fWin)
    fWin->SetFocus();

#endif

#ifdef __WXMSW__
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    unsigned long processID = wxGetProcessId();

    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
    if( NULL == hProcess ) return 0;

    if( GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) ) {
        /*
         printf( "\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount );
         printf( "\tPeakWorkingSetSize: 0x%08X\n",
         pmc.PeakWorkingSetSize );
         printf( "\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize );
         printf( "\tQuotaPeakPagedPoolUsage: 0x%08X\n",
         pmc.QuotaPeakPagedPoolUsage );
         printf( "\tQuotaPagedPoolUsage: 0x%08X\n",
         pmc.QuotaPagedPoolUsage );
         printf( "\tQuotaPeakNonPagedPoolUsage: 0x%08X\n",
         pmc.QuotaPeakNonPagedPoolUsage );
         printf( "\tQuotaNonPagedPoolUsage: 0x%08X\n",
         pmc.QuotaNonPagedPoolUsage );
         printf( "\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage );
         printf( "\tPeakPagefileUsage: 0x%08X\n",
         pmc.PeakPagefileUsage );
         */
        memsize = pmc.WorkingSetSize / 1024;
    }

    CloseHandle( hProcess );

#endif

    return memsize;
}

void MyFrame::HandlePianoClick( int selected_index, int selected_dbIndex )
{
    if( !pCurrentStack ) return;
    if( s_ProgDialog ) return;

    if( !cc1->GetQuiltMode() ) {
        if( m_bpersistent_quilt && g_bQuiltEnable ) {
            if( cc1->IsChartQuiltableRef( selected_dbIndex ) ) {
                ToggleQuiltMode();
                SelectQuiltRefdbChart( selected_dbIndex );
                m_bpersistent_quilt = false;
            } else {
                SelectChartFromStack( selected_index );
            }
        } else {
            SelectChartFromStack( selected_index );
            g_sticky_chart = selected_dbIndex;
        }
    } else {
        if( cc1->IsChartQuiltableRef( selected_dbIndex ) ) SelectQuiltRefdbChart(
                selected_dbIndex );
        else {
            ToggleQuiltMode();
            SelectdbChart( selected_dbIndex );
            m_bpersistent_quilt = true;
        }
    }

    cc1->SetQuiltChartHiLiteIndex( -1 );
    cc1->HideChartInfoWindow();
    DoChartUpdate();
    cc1->ReloadVP();                  // Pick up the new selections
}

void MyFrame::HandlePianoRClick( int x, int y, int selected_index, int selected_dbIndex )
{
    if( !pCurrentStack ) return;
    if( s_ProgDialog ) return;

    PianoPopupMenu( x, y, selected_index, selected_dbIndex );
    UpdateControlBar();

    cc1->SetQuiltChartHiLiteIndex( -1 );

}

void MyFrame::HandlePianoRollover( int selected_index, int selected_dbIndex )
{
    if( !cc1 ) return;
    if( !pCurrentStack ) return;
    if( s_ProgDialog ) return;

    int sx, sy;
    stats->GetPosition( &sx, &sy );
    wxPoint key_location = stats->pPiano->GetKeyOrigin( selected_index );
    wxPoint rolloverPos = stats->GetParent()->ScreenToClient( wxPoint( sx, sy ) );
    rolloverPos.y -= 3;
    rolloverPos.x += key_location.x;

    if( !cc1->GetQuiltMode() ) {
        SetChartThumbnail( selected_index );
        cc1->ShowChartInfoWindow( key_location.x, sy + key_location.y, selected_dbIndex );
    } else {
        ArrayOfInts piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();

        if( ( pCurrentStack->nEntry > 1 ) || ( piano_chart_index_array.GetCount() > 1 ) ) {
            cc1->ShowChartInfoWindow( rolloverPos.x, rolloverPos.y, selected_dbIndex );
            cc1->SetQuiltChartHiLiteIndex( selected_dbIndex );

            cc1->ReloadVP( false );         // no VP adjustment allowed
        } else if( pCurrentStack->nEntry == 1 ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry(
                    pCurrentStack->GetDBIndex( 0 ) );
            if( CHART_TYPE_CM93COMP != cte.GetChartType() ) {
                cc1->ShowChartInfoWindow( rolloverPos.x, rolloverPos.y, selected_dbIndex );
                cc1->ReloadVP( false );
            } else if( ( -1 == selected_index ) && ( -1 == selected_dbIndex ) ) {
                cc1->ShowChartInfoWindow( rolloverPos.x, rolloverPos.y, selected_dbIndex );
            }
        }
        SetChartThumbnail( -1 );        // hide all thumbs in quilt mode
    }
}

void MyFrame::HandlePianoRolloverIcon( int selected_index, int selected_dbIndex )
{
    if( !cc1 ) return;

    if( !cc1->GetQuiltMode() ) {
        SetChartThumbnail( selected_index );
    } else {
        cc1->SetQuiltChartHiLiteIndex( selected_dbIndex );
    }
}

double MyFrame::GetBestVPScale( ChartBase *pchart )
{
    if( pchart ) {
        double proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / cc1->GetVPScale();

        if( ( g_bPreserveScaleOnX ) || ( CHART_TYPE_CM93COMP == pchart->GetChartType() ) ) {
            double new_scale_ppm = pchart->GetNearestPreferredScalePPM( cc1->GetVPScale() );
            proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
        } else {
            //  This logic will bring the new chart onscreen at roughly twice the true paper scale equivalent.
            proposed_scale_onscreen = pchart->GetNativeScale() / 2;
            double equivalent_vp_scale = cc1->GetCanvasScaleFactor() / proposed_scale_onscreen;
            double new_scale_ppm = pchart->GetNearestPreferredScalePPM( equivalent_vp_scale );
            proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
        }

        proposed_scale_onscreen =
                wxMin(proposed_scale_onscreen, pchart->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));
        proposed_scale_onscreen =
                wxMax(proposed_scale_onscreen, pchart->GetNormalScaleMin(cc1->GetCanvasScaleFactor(), g_b_overzoom_x));

        return cc1->GetCanvasScaleFactor() / proposed_scale_onscreen;
    } else
        return 1.0;
}

void MyFrame::SelectQuiltRefChart( int selected_index )
{
    ArrayOfInts piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();
    int current_db_index = piano_chart_index_array.Item( selected_index );

    SelectQuiltRefdbChart( current_db_index );
}

void MyFrame::SelectQuiltRefdbChart( int db_index )
{
    if( pCurrentStack ) pCurrentStack->SetCurrentEntryFromdbIndex( db_index );

    cc1->SetQuiltRefChart( db_index );

    ChartBase *pc = ChartData->OpenChartFromDB( db_index, FULL_INIT );
    if( pc ) {
        double best_scale = GetBestVPScale( pc );
        cc1->SetVPScale( best_scale );
    }

}

void MyFrame::SelectChartFromStack( int index, bool bDir, ChartTypeEnum New_Type,
        ChartFamilyEnum New_Family )
{
    if( index < pCurrentStack->nEntry ) {
//      Open the new chart
        ChartBase *pTentative_Chart;
        pTentative_Chart = ChartData->OpenStackChartConditional( pCurrentStack, index, bDir,
                New_Type, New_Family );

        if( pTentative_Chart ) {
            if( Current_Ch ) Current_Ch->Deactivate();

            Current_Ch = pTentative_Chart;
            Current_Ch->Activate();

            pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                    Current_Ch->GetFullPath() );
        } else
            SetChartThumbnail( -1 );   // need to reset thumbnail on failed chart open

//      Setup the view
        double zLat, zLon;
        if( cc1->m_bFollow ) {
            zLat = gLat;
            zLon = gLon;
        } else {
            zLat = vLat;
            zLon = vLon;
        }

        double best_scale = GetBestVPScale( Current_Ch );

        cc1->SetViewPoint( zLat, zLon, best_scale, Current_Ch->GetChartSkew() * PI / 180.,
                cc1->GetVPRotation() );

        SetChartUpdatePeriod( cc1->GetVP() );

        UpdateGPSCompassStatusBox();           // Pick up the rotation

    }

    //          Refresh the Piano Bar
    if( stats ) {
        ArrayOfInts piano_active_chart_index_array;
        piano_active_chart_index_array.Add( pCurrentStack->GetCurrentEntrydbIndex() );
        stats->pPiano->SetActiveKeyArray( piano_active_chart_index_array );

        stats->Refresh( true );
    }
}

void MyFrame::SelectdbChart( int dbindex )
{
    if( dbindex >= 0 ) {
//      Open the new chart
        ChartBase *pTentative_Chart;
        pTentative_Chart = ChartData->OpenChartFromDB( dbindex, FULL_INIT );

        if( pTentative_Chart ) {
            if( Current_Ch ) Current_Ch->Deactivate();

            Current_Ch = pTentative_Chart;
            Current_Ch->Activate();

            pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                    Current_Ch->GetFullPath() );
        } else
            SetChartThumbnail( -1 );       // need to reset thumbnail on failed chart open

//      Setup the view
        double zLat, zLon;
        if( cc1->m_bFollow ) {
            zLat = gLat;
            zLon = gLon;
        } else {
            zLat = vLat;
            zLon = vLon;
        }

        double best_scale = GetBestVPScale( Current_Ch );

        cc1->SetViewPoint( zLat, zLon, best_scale, Current_Ch->GetChartSkew() * PI / 180.,
                cc1->GetVPRotation() );

        SetChartUpdatePeriod( cc1->GetVP() );

        UpdateGPSCompassStatusBox();           // Pick up the rotation

    }

    //          Refresh the Piano Bar
    if( stats ) {
        ArrayOfInts piano_active_chart_index_array;
        piano_active_chart_index_array.Add( pCurrentStack->GetCurrentEntrydbIndex() );
        stats->pPiano->SetActiveKeyArray( piano_active_chart_index_array );

        stats->Refresh( true );
    }
}

void MyFrame::SetChartUpdatePeriod( ViewPort &vp )
{
    //    Set the chart update period based upon chart skew and skew compensator

    g_ChartUpdatePeriod = 1;            // General default

    if( !vp.b_quilt ) {
        if( g_bskew_comp && ( fabs( vp.skew ) ) > 0.01 ) g_ChartUpdatePeriod =
                g_SkewCompUpdatePeriod;
    }

    m_ChartUpdatePeriod = g_ChartUpdatePeriod;
}

void MyFrame::SetChartThumbnail( int index )
{
    if( bDBUpdateInProgress ) return;

    if( NULL == pCurrentStack ) return;

    if( NULL == pthumbwin ) return;

    if( NULL == cc1 ) return;

    if( index == -1 ) {
        wxRect thumb_rect_in_parent = pthumbwin->GetRect();

        pthumbwin->pThumbChart = NULL;
        pthumbwin->Show( false );
        cc1->RefreshRect( thumb_rect_in_parent, FALSE );
    }

    else
        if( index < pCurrentStack->nEntry ) {
            if( ( ChartData->GetCSChartType( pCurrentStack, index ) == CHART_TYPE_KAP )
                    || ( ChartData->GetCSChartType( pCurrentStack, index ) == CHART_TYPE_GEO )
                    || ( ChartData->GetCSChartType( pCurrentStack, index ) == CHART_TYPE_PLUGIN ) ) {
                ChartBase *new_pThumbChart = ChartData->OpenChartFromStack( pCurrentStack, index );
                if( new_pThumbChart )         // chart opened ok
                {

                    ThumbData *pTD = new_pThumbChart->GetThumbData( 150, 150, gLat, gLon );
                    if( pTD ) {
                        pthumbwin->pThumbChart = new_pThumbChart;

                        pthumbwin->Resize();
                        pthumbwin->Show( true );
                        pthumbwin->Refresh( FALSE );
                        pthumbwin->Move( wxPoint( 4, 4 ) );

                        // Simplistic overlap avoidance works only when toolbar is at top of screen.
                        if( g_FloatingToolbarDialog )
                            if( g_FloatingToolbarDialog->GetScreenRect().Intersects( pthumbwin->GetScreenRect() ) ) {
                                pthumbwin->Move( wxPoint( 4, g_FloatingToolbarDialog->GetSize().y + 4 ) );
                        }
                    }

                    else {
                        wxLogMessage(
                                _T("    chart1.cpp:SetChartThumbnail...Could not create thumbnail") );
                        pthumbwin->pThumbChart = NULL;
                        pthumbwin->Show( false );
                        cc1->Refresh( FALSE );
                    }

                } else                            // some problem opening chart
                {
                    wxString fp = ChartData->GetFullPath( pCurrentStack, index );
                    fp.Prepend( _T("    chart1.cpp:SetChartThumbnail...Could not open chart ") );
                    wxLogMessage( fp );
                    pthumbwin->pThumbChart = NULL;
                    pthumbwin->Show( false );
                    cc1->Refresh( FALSE );
                }

            } else {
                ChartBase *new_pThumbChart = ChartData->OpenChartFromStack( pCurrentStack, index,
                        THUMB_ONLY );

                pthumbwin->pThumbChart = new_pThumbChart;

                if( new_pThumbChart ) {
                    ThumbData *pTD = new_pThumbChart->GetThumbData( 200, 200, gLat, gLon );
                    if( pTD ) {
                        pthumbwin->Resize();
                        pthumbwin->Show( true );
                        pthumbwin->Refresh( true );
                    } else
                        pthumbwin->Show( false );

                    cc1->Refresh( FALSE );
                }
            }
        }

}

void MyFrame::UpdateControlBar( void )
{
    if( !cc1 ) return;

    if( !stats ) return;

    if( !pCurrentStack ) return;

    ArrayOfInts piano_chart_index_array;
    ArrayOfInts empty_piano_chart_index_array;

    if( cc1->GetQuiltMode() ) {
        piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();
        stats->pPiano->SetKeyArray( piano_chart_index_array );

        ArrayOfInts piano_active_chart_index_array = cc1->GetQuiltCandidatedbIndexArray();
        stats->pPiano->SetActiveKeyArray( piano_active_chart_index_array );

        ArrayOfInts piano_eclipsed_chart_index_array = cc1->GetQuiltEclipsedStackdbIndexArray();
        stats->pPiano->SetSubliteIndexArray( piano_eclipsed_chart_index_array );

        stats->pPiano->SetNoshowIndexArray( g_quilt_noshow_index_array );

    } else {
        piano_chart_index_array = ChartData->GetCSArray( pCurrentStack );
        stats->pPiano->SetKeyArray( piano_chart_index_array );

        ArrayOfInts piano_active_chart_index_array;
        piano_active_chart_index_array.Add( pCurrentStack->GetCurrentEntrydbIndex() );
        stats->pPiano->SetActiveKeyArray( piano_active_chart_index_array );

    }

    //    Set up the TMerc and Skew arrays
    ArrayOfInts piano_skew_chart_index_array;
    ArrayOfInts piano_tmerc_chart_index_array;
    ArrayOfInts piano_poly_chart_index_array;

    for( unsigned int ino = 0; ino < piano_chart_index_array.GetCount(); ino++ ) {
        const ChartTableEntry &ctei = ChartData->GetChartTableEntry(
                piano_chart_index_array.Item( ino ) );
        double skew_norm = ctei.GetChartSkew();
        if( skew_norm > 180. ) skew_norm -= 360.;

        if( ctei.GetChartProjectionType() == PROJECTION_TRANSVERSE_MERCATOR ) piano_tmerc_chart_index_array.Add(
                piano_chart_index_array.Item( ino ) );

        //    Polyconic skewed charts should show as skewed
        else
            if( ctei.GetChartProjectionType() == PROJECTION_POLYCONIC ) {
                if( fabs( skew_norm ) > 1. ) piano_skew_chart_index_array.Add(
                        piano_chart_index_array.Item( ino ) );
                else
                    piano_poly_chart_index_array.Add( piano_chart_index_array.Item( ino ) );
            } else
                if( fabs( skew_norm ) > 1. ) piano_skew_chart_index_array.Add(
                        piano_chart_index_array.Item( ino ) );

    }
    stats->pPiano->SetSkewIndexArray( piano_skew_chart_index_array );
    stats->pPiano->SetTmercIndexArray( piano_tmerc_chart_index_array );
    stats->pPiano->SetPolyIndexArray( piano_poly_chart_index_array );

    stats->FormatStat();
    stats->Refresh( true );

}

//----------------------------------------------------------------------------------
//      DoChartUpdate
//      Create a chartstack based on current lat/lon.
//      Update Current_Ch, using either current chart, if still in stack, or
//      smallest scale new chart in stack if not.
//      Return true if a Refresh(false) was called within.
//----------------------------------------------------------------------------------
bool MyFrame::DoChartUpdate( void )
{

    double tLat, tLon;           // Chart Stack location
    double vpLat, vpLon;         // ViewPort location

    bool bNewChart = false;
    bool bNewView = false;

    bool bNewPiano = false;
    bool bOpenSpecified;
    ChartStack LastStack;
    ChartBase *pLast_Ch;

    ChartStack WorkStack;

    if( !cc1 ) return false;
    if( bDBUpdateInProgress ) return false;
    if( !ChartData ) return false;

    int last_nEntry = -1;
    if( pCurrentStack ) last_nEntry = pCurrentStack->nEntry;

    //    Startup case:
    //    Quilting is enabled, but the last chart seen was not quiltable
    //    In this case, drop to single chart mode, set persistence flag,
    //    And open the specified chart
    if( bFirstAuto && ( g_restore_dbindex >= 0 ) ) {
        if( cc1->GetQuiltMode() ) {
            if( !cc1->IsChartQuiltableRef( g_restore_dbindex ) ) {
                ToggleQuiltMode();
                m_bpersistent_quilt = true;
                Current_Ch = NULL;
            }
        }
    }

    //      If in auto-follow mode, use the current glat,glon to build chart stack.
    //      Otherwise, use vLat, vLon gotten from click on chart canvas, or other means

    if( cc1->m_bFollow == true ) {
        tLat = gLat;
        tLon = gLon;
        vpLat = gLat;
        vpLon = gLon;

        // on lookahead mode, adjust the vp center point
        if( cc1 && g_bLookAhead ) {
            double angle = g_COGAvg + ( cc1->GetVPRotation() * 180. / PI );

            double pixel_deltay = fabs( cos( angle * PI / 180. ) ) * cc1->GetCanvasHeight() / 4;
            double pixel_deltax = fabs( sin( angle * PI / 180. ) ) * cc1->GetCanvasWidth() / 4;

            double pixel_delta_tent = sqrt(
                    ( pixel_deltay * pixel_deltay ) + ( pixel_deltax * pixel_deltax ) );

            double pixel_delta = 0;

            //    The idea here is to cancel the effect of LookAhead for slow gSog, to avoid
            //    jumping of the vp center point during slow maneuvering, or at anchor....
            if( !wxIsNaN(gSog) ) {
                if( gSog < 1.0 ) pixel_delta = 0.;
                else
                    if( gSog >= 3.0 ) pixel_delta = pixel_delta_tent;
                    else
                        pixel_delta = pixel_delta_tent * ( gSog - 1.0 ) / 2.0;
            }

            double meters_to_shift = cos( gLat * PI / 180. ) * pixel_delta / cc1->GetVPScale();

            double dir_to_shift = g_COGAvg;

            ll_gc_ll( gLat, gLon, dir_to_shift, meters_to_shift / 1852., &vpLat, &vpLon );
        }

    } else {
        tLat = vLat;
        tLon = vLon;
        vpLat = vLat;
        vpLon = vLon;

    }

    if( cc1->GetQuiltMode() ) {
        int current_db_index = -1;
        if( pCurrentStack ) current_db_index = pCurrentStack->GetCurrentEntrydbIndex(); // capture the currently selected Ref chart dbIndex
        else
            pCurrentStack = new ChartStack;

        ChartData->BuildChartStack( pCurrentStack, tLat, tLon );
        pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );

        if( bFirstAuto ) {
            double proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / cc1->GetVPScale(); // as set from config load

            int initial_db_index = g_restore_dbindex;
            if( initial_db_index < 0 ) {
                if( pCurrentStack->nEntry ) {
                    if( ( g_restore_stackindex < pCurrentStack->nEntry )
                            && ( g_restore_stackindex >= 0 ) ) initial_db_index =
                            pCurrentStack->GetDBIndex( g_restore_stackindex );
                    else
                        initial_db_index = pCurrentStack->GetDBIndex( pCurrentStack->nEntry - 1 );
                } else
                    initial_db_index = 0;
            }

            if( pCurrentStack->nEntry ) {

                int initial_type = ChartData->GetDBChartType( initial_db_index );

                //    Check to see if the target new chart is quiltable as a reference chart

                if( !cc1->IsChartQuiltableRef( initial_db_index ) ) {
                    // If it is not quiltable, then walk the stack up looking for a satisfactory chart
                    // i.e. one that is quiltable and of the same type
                    int stack_index = g_restore_stackindex;

                    while( ( stack_index < pCurrentStack->nEntry - 1 ) && ( stack_index >= 0 ) ) {
                        int test_db_index = pCurrentStack->GetDBIndex( stack_index );
                        if( cc1->IsChartQuiltableRef( test_db_index )
                                && ( initial_type == ChartData->GetDBChartType( initial_db_index ) ) ) {
                            initial_db_index = test_db_index;
                            break;
                        }
                        stack_index++;
                    }
                }

                cc1->SetQuiltRefChart( initial_db_index );
                pCurrentStack->SetCurrentEntryFromdbIndex( initial_db_index );

                // Try to bound the inital Viewport scale to something reasonable for the selected reference chart
                if( ChartData ) {
                    ChartBase *pc = ChartData->OpenChartFromDB( initial_db_index, FULL_INIT );
                    if( pc ) {
                        proposed_scale_onscreen =
                                wxMin(proposed_scale_onscreen, pc->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));
                        proposed_scale_onscreen =
                                wxMax(proposed_scale_onscreen, pc->GetNormalScaleMin(cc1->GetCanvasScaleFactor(), g_b_overzoom_x));
                    }
                }

            }

            bNewView |= cc1->SetViewPoint( vpLat, vpLon,
                    cc1->GetCanvasScaleFactor() / proposed_scale_onscreen, 0,
                    cc1->GetVPRotation() );

        }

        bNewView |= cc1->SetViewPoint( vpLat, vpLon, cc1->GetVPScale(), 0, cc1->GetVPRotation() );

        goto update_finish;

    }

    //  Single Chart Mode from here....
    pLast_Ch = Current_Ch;
    ChartTypeEnum new_open_type;
    ChartFamilyEnum new_open_family;
    if( pLast_Ch ) {
        new_open_type = pLast_Ch->GetChartType();
        new_open_family = pLast_Ch->GetChartFamily();
    } else {
        new_open_type = CHART_TYPE_KAP;
        new_open_family = CHART_FAMILY_RASTER;
    }

    bOpenSpecified = bFirstAuto;
    bAutoOpen = true;                             // debugging

    //  Make sure the target stack is valid
    if( NULL == pCurrentStack ) pCurrentStack = new ChartStack;

    // Build a chart stack based on tLat, tLon
    if( 0 == ChartData->BuildChartStack( &WorkStack, tLat, tLon, g_sticky_chart ) )       // Bogus Lat, Lon?
            {
        if( NULL == pDummyChart ) {
            pDummyChart = new ChartDummy;
            bNewChart = true;
        }

        if( Current_Ch ) if( Current_Ch->GetChartType() != CHART_TYPE_DUMMY ) bNewChart = true;

        Current_Ch = pDummyChart;

//    If the current viewpoint is invalid, set the default scale to something reasonable.
        double set_scale = cc1->GetVPScale();
        if( !cc1->GetVP().IsValid() ) set_scale = 1. / 200000.;

        bNewView |= cc1->SetViewPoint( tLat, tLon, set_scale, 0, cc1->GetVPRotation() );

        //      If the chart stack has just changed, there is new status
        if( !ChartData->EqualStacks( &WorkStack, pCurrentStack ) ) {
            bNewPiano = true;
            bNewChart = true;
        }

        //      Copy the new (by definition empty) stack into the target stack
        ChartData->CopyStack( pCurrentStack, &WorkStack );

        goto update_finish;
    }

    //              Check to see if Chart Stack has changed
    if( !ChartData->EqualStacks( &WorkStack, pCurrentStack ) ) {
        //      New chart stack, so...
        bNewPiano = true;

        //      Save a copy of the current stack
        ChartData->CopyStack( &LastStack, pCurrentStack );

        //      Copy the new stack into the target stack
        ChartData->CopyStack( pCurrentStack, &WorkStack );

        //  Is Current Chart in new stack?

        int tEntry = -1;
        if( NULL != Current_Ch )                                  // this handles startup case
            tEntry = ChartData->GetStackEntry( pCurrentStack, Current_Ch->GetFullPath() );

        if( tEntry != -1 ) {                // Current_Ch is in the new stack
            pCurrentStack->CurrentStackEntry = tEntry;
            bNewChart = false;
        }

        else                           // Current_Ch is NOT in new stack
        {                                       // So, need to open a new chart
                                                //      Find the largest scale raster chart that opens OK

            ChartBase *pProposed = NULL;

            if( bAutoOpen ) {
                bool search_direction = false;        // default is to search from lowest to highest
                int start_index = 0;

                //    A special case:  If panning at high scale, open largest scale chart first
                if( ( LastStack.CurrentStackEntry == LastStack.nEntry - 1 )
                        || ( LastStack.nEntry == 0 ) ) {
                    search_direction = true;
                    start_index = pCurrentStack->nEntry - 1;
                }

                //    Another special case, open specified index on program start
                if( bOpenSpecified ) {
                    search_direction = false;
                    start_index = g_restore_stackindex;
                    if( ( start_index < 0 ) | ( start_index >= pCurrentStack->nEntry ) ) start_index =
                            0;
                    new_open_type = CHART_TYPE_DONTCARE;
                }

                pProposed = ChartData->OpenStackChartConditional( pCurrentStack, start_index,
                        search_direction, new_open_type, new_open_family );

//    Try to open other types/families of chart in some priority
                if( NULL == pProposed ) pProposed = ChartData->OpenStackChartConditional(
                        pCurrentStack, start_index, search_direction, CHART_TYPE_CM93COMP,
                        CHART_FAMILY_VECTOR );

                if( NULL == pProposed ) pProposed = ChartData->OpenStackChartConditional(
                        pCurrentStack, start_index, search_direction, CHART_TYPE_CM93COMP,
                        CHART_FAMILY_RASTER );

                bNewChart = true;

            }     // bAutoOpen

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
            if( Current_Ch ) Current_Ch->Deactivate();
            Current_Ch = pProposed;

            if( Current_Ch ) {
                Current_Ch->Activate();
                pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                        Current_Ch->GetFullPath() );
            }
        }   // need new chart

// Arriving here, Current_Ch is opened and OK, or NULL
        if( NULL != Current_Ch ) {

//      Setup the view using the current scale
            double set_scale = cc1->GetVPScale();

//    If the current viewpoint is invalid, set the default scale to something reasonable.
            if( !cc1->GetVP().IsValid() )
                set_scale = 1. / 200000.;
            else {                                    // otherwise, match scale if elected.
                double proposed_scale_onscreen;

                if( cc1->m_bFollow ) {          // autoset the scale only if in autofollow
                    double new_scale_ppm = Current_Ch->GetNearestPreferredScalePPM( cc1->GetVPScale() );
                    proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
                }
                else
                    proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / set_scale;


                //  This logic will bring a new chart onscreen at roughly twice the true paper scale equivalent.
                //  Note that first chart opened on application startup (bOpenSpecified = true) will open at the config saved scale
                if( bNewChart && !g_bPreserveScaleOnX && !bOpenSpecified ) {
                    proposed_scale_onscreen = Current_Ch->GetNativeScale() / 2;
                    double equivalent_vp_scale = cc1->GetCanvasScaleFactor()
                            / proposed_scale_onscreen;
                    double new_scale_ppm = Current_Ch->GetNearestPreferredScalePPM(
                            equivalent_vp_scale );
                    proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
                }

                if( cc1->m_bFollow ) {     // bounds-check the scale only if in autofollow
                    proposed_scale_onscreen =
                        wxMin(proposed_scale_onscreen, Current_Ch->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));
                    proposed_scale_onscreen =
                        wxMax(proposed_scale_onscreen, Current_Ch->GetNormalScaleMin(cc1->GetCanvasScaleFactor(), g_b_overzoom_x));
                }

                set_scale = cc1->GetCanvasScaleFactor() / proposed_scale_onscreen;
            }

            bNewView |= cc1->SetViewPoint( vpLat, vpLon, set_scale,
                    Current_Ch->GetChartSkew() * PI / 180., cc1->GetVPRotation() );

        }
    }         // new stack

    else                                                                 // No change in Chart Stack
    {
        if( ( cc1->m_bFollow ) && Current_Ch ) bNewView |= cc1->SetViewPoint( vpLat, vpLon,
                cc1->GetVPScale(), Current_Ch->GetChartSkew() * PI / 180., cc1->GetVPRotation() );
    }

    update_finish:

    //    Ask for a new tool bar if the stack is going to or coming from only one entry.
    if( pCurrentStack
            && ( ( ( pCurrentStack->nEntry <= 1 ) && m_toolbar_scale_tools_shown )
                    || ( ( pCurrentStack->nEntry > 1 ) && !m_toolbar_scale_tools_shown ) ) ) if( !bFirstAuto ) RequestNewToolbar();

    if( bNewPiano ) UpdateControlBar();

    //  Update the ownship position on thumbnail chart, if shown
    if( pthumbwin->IsShown() ) {
        if( pthumbwin->pThumbChart ) if( pthumbwin->pThumbChart->UpdateThumbData( gLat, gLon ) ) pthumbwin->Refresh(
                TRUE );
    }

    bFirstAuto = false;                           // Auto open on program start

    //  If we need a Refresh(), do it here...
    //  But don't duplicate a Refresh() done by SetViewPoint()
    if( bNewChart && !bNewView ) cc1->Refresh( false );

    return bNewChart | bNewView;
}

void MyFrame::MouseEvent( wxMouseEvent& event )
{
    int x, y;
    event.GetPosition( &x, &y );

}

void MyFrame::RemoveChartFromQuilt( int dbIndex )
{
    //    Remove the item from the list (if it appears) to avoid multiple addition
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.GetCount(); i++ ) {
        if( g_quilt_noshow_index_array.Item( i ) == dbIndex ) // chart is already in the noshow list
                {
            g_quilt_noshow_index_array.RemoveAt( i );
            break;
        }
    }

    g_quilt_noshow_index_array.Add( dbIndex );

}

//          Piano window Popup Menu Handlers and friends

static int menu_selected_dbIndex;
static int menu_selected_index;

void MyFrame::PianoPopupMenu( int x, int y, int selected_index, int selected_dbIndex )
{
    //    No context menu if quilting is disabled
    if( !cc1->GetQuiltMode() ) return;

    menu_selected_dbIndex = selected_dbIndex;
    menu_selected_index = selected_index;

    wxMenu *pctx_menu = new wxMenu();

    //    Search the no-show array
    bool b_is_in_noshow = false;
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.GetCount(); i++ ) {
        if( g_quilt_noshow_index_array.Item( i ) == selected_dbIndex ) // chart is in the noshow list
                {
            b_is_in_noshow = true;
            break;
        }
    }

    if( b_is_in_noshow ) {
        pctx_menu->Append( ID_PIANO_ENABLE_QUILT_CHART, _("Show This Chart") );
        Connect( ID_PIANO_ENABLE_QUILT_CHART, wxEVT_COMMAND_MENU_SELECTED,
                wxCommandEventHandler(MyFrame::OnPianoMenuEnableChart) );
    } else
        if( pCurrentStack->nEntry > 1 ) {
            pctx_menu->Append( ID_PIANO_DISABLE_QUILT_CHART, _("Hide This Chart") );
            Connect( ID_PIANO_DISABLE_QUILT_CHART, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler(MyFrame::OnPianoMenuDisableChart) );
        }

    int sx, sy;
    stats->GetPosition( &sx, &sy );
    wxPoint pos = stats->GetParent()->ScreenToClient( wxPoint( sx, sy ) );
    wxPoint key_location = stats->pPiano->GetKeyOrigin( selected_index );
    pos.x += key_location.x;
    pos.y -= 30;

//        Invoke the drop-down menu
    if( pctx_menu->GetMenuItems().GetCount() ) PopupMenu( pctx_menu, pos );

    cc1->HideChartInfoWindow();
    stats->pPiano->ResetRollover();

    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();

    delete pctx_menu;
}

void MyFrame::OnPianoMenuEnableChart( wxCommandEvent& event )
{
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.GetCount(); i++ ) {
        if( g_quilt_noshow_index_array.Item( i ) == menu_selected_dbIndex ) // chart is in the noshow list
                {
            g_quilt_noshow_index_array.RemoveAt( i );
            break;
        }
    }
}

void MyFrame::OnPianoMenuDisableChart( wxCommandEvent& event )
{
    RemoveChartFromQuilt( menu_selected_dbIndex );

//      It could happen that the chart being disabled is the reference chart....
    if( menu_selected_dbIndex == cc1->GetQuiltRefChartdbIndex() ) {
        int type = ChartData->GetDBChartType( menu_selected_dbIndex );

        int i = menu_selected_index + 1;          // select next smaller scale chart
        bool b_success = false;
        while( i < pCurrentStack->nEntry - 1 ) {
            int dbIndex = pCurrentStack->GetDBIndex( i );
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
                int dbIndex = pCurrentStack->GetDBIndex( i );
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

//      Memory monitor support

bool GetMemoryStatus( int *mem_total, int *mem_used )
{

#ifdef __LINUX__

//      Use filesystem /proc/pid/status to determine memory status

    unsigned long processID = wxGetProcessId();
    wxTextFile file;
    wxString file_name;

    if(mem_used)
    {
        *mem_used = 0;
        file_name.Printf(_T("/proc/%d/status"), (int)processID);
        if(file.Open(file_name))
        {
            bool b_found = false;
            wxString str;
            for ( str = file.GetFirstLine(); !file.Eof(); str = file.GetNextLine() )
            {
                wxStringTokenizer tk(str, _T(" :"));
                while ( tk.HasMoreTokens() )
                {
                    wxString token = tk.GetNextToken();
                    if(token == _T("VmSize"))
                    {
                        wxStringTokenizer tkm(str, _T(" "));
                        wxString mem = tkm.GetNextToken();
                        long mem_extract = 0;
                        while(mem.Len())
                        {
                            mem.ToLong(&mem_extract);
                            if(mem_extract)
                            break;
                            mem = tkm.GetNextToken();
                        }

                        *mem_used = mem_extract;
                        b_found = true;
                        break;
                    }
                    else
                    break;
                }
                if(b_found)
                break;
            }
        }
    }

    if(mem_total)
    {
        *mem_total = 0;
        wxTextFile file_info;
        file_name = _T("/proc/meminfo");
        if(file_info.Open(file_name))
        {
            bool b_found = false;
            wxString str;
            for ( str = file_info.GetFirstLine(); !file_info.Eof(); str = file_info.GetNextLine() )
            {
                wxStringTokenizer tk(str, _T(" :"));
                while ( tk.HasMoreTokens() )
                {
                    wxString token = tk.GetNextToken();
                    if(token == _T("MemTotal"))
                    {
                        wxStringTokenizer tkm(str, _T(" "));
                        wxString mem = tkm.GetNextToken();
                        long mem_extract = 0;
                        while(mem.Len())
                        {
                            mem.ToLong(&mem_extract);
                            if(mem_extract)
                            break;
                            mem = tkm.GetNextToken();
                        }

                        *mem_total = mem_extract;
                        b_found = true;
                        break;
                    }
                    else
                    break;
                }
                if(b_found)
                break;
            }
        }
    }

#endif

#ifdef __WXMSW__
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    unsigned long processID = wxGetProcessId();

    if( mem_used ) {
        hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );

        if( hProcess && GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) ) {
            /*
             printf( "\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount );
             printf( "\tPeakWorkingSetSize: 0x%08X\n",
             pmc.PeakWorkingSetSize );
             printf( "\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize );
             printf( "\tQuotaPeakPagedPoolUsage: 0x%08X\n",
             pmc.QuotaPeakPagedPoolUsage );
             printf( "\tQuotaPagedPoolUsage: 0x%08X\n",
             pmc.QuotaPagedPoolUsage );
             printf( "\tQuotaPeakNonPagedPoolUsage: 0x%08X\n",
             pmc.QuotaPeakNonPagedPoolUsage );
             printf( "\tQuotaNonPagedPoolUsage: 0x%08X\n",
             pmc.QuotaNonPagedPoolUsage );
             printf( "\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage );
             printf( "\tPeakPagefileUsage: 0x%08X\n",
             pmc.PeakPagefileUsage );
             */
            *mem_used = pmc.WorkingSetSize / 1024;
        }

        CloseHandle( hProcess );
    }

    if( mem_total ) {
        MEMORYSTATUSEX statex;

        statex.dwLength = sizeof( statex );

        GlobalMemoryStatusEx( &statex );
        /*
         _tprintf (TEXT("There is  %*ld percent of memory in use.\n"),
         WIDTH, statex.dwMemoryLoad);
         _tprintf (TEXT("There are %*I64d total Kbytes of physical memory.\n"),
         WIDTH, statex.ullTotalPhys/DIV);
         _tprintf (TEXT("There are %*I64d free Kbytes of physical memory.\n"),
         WIDTH, statex.ullAvailPhys/DIV);
         _tprintf (TEXT("There are %*I64d total Kbytes of paging file.\n"),
         WIDTH, statex.ullTotalPageFile/DIV);
         _tprintf (TEXT("There are %*I64d free Kbytes of paging file.\n"),
         WIDTH, statex.ullAvailPageFile/DIV);
         _tprintf (TEXT("There are %*I64d total Kbytes of virtual memory.\n"),
         WIDTH, statex.ullTotalVirtual/DIV);
         _tprintf (TEXT("There are %*I64d free Kbytes of virtual memory.\n"),
         WIDTH, statex.ullAvailVirtual/DIV);
         */

        *mem_total = statex.ullTotalPhys / 1024;
    }
#endif

    return true;

}

void MyFrame::DoPrint( void )
{
    if( NULL == g_printData ) {
        g_printData = new wxPrintData;
        g_printData->SetOrientation( wxLANDSCAPE );
        g_pageSetupData = new wxPageSetupDialogData;
    }

    wxPrintDialogData printDialogData( *g_printData );
    printDialogData.EnablePageNumbers( false );

    wxPrinter printer( &printDialogData );

    MyPrintout printout( _("Chart Print") );
    if( !printer.Print( this, &printout, true ) ) {
        if( wxPrinter::GetLastError() == wxPRINTER_ERROR ) OCPNMessageBox(NULL,
                _("There was a problem printing.\nPerhaps your current printer is not set correctly?"),
                _T("OpenCPN"), wxOK );
//        else
//            OCPNMessageBox(_T("Print Cancelled"), _T("OpenCPN"), wxOK);
    } else {
        ( *g_printData ) = printer.GetPrintDialogData().GetPrintData();
    }

// Pass two printout objects: for preview, and possible printing.
    /*
     wxPrintDialogData printDialogData(* g_printData);
     wxPrintPreview *preview = new wxPrintPreview(new MyPrintout, new MyPrintout, & printDialogData);
     if (!preview->Ok())
     {
     delete preview;
     OCPNMessageBox(_T("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _T("Previewing"), wxOK);
     return;
     }

     wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _T("Demo Print Preview"), wxPoint(100, 100), wxSize(600, 650));
     frame->Centre(wxBOTH);
     frame->Initialize();
     frame->Show();
     */

}

// toh, 2009.02.15
void MyFrame::DoExportGPX( void )
{
    if( pConfig ) pConfig->ExportGPX( this );
}

// toh, 2009.02.16
void MyFrame::DoImportGPX( void )
{
    if( pConfig ) {
        pConfig->ImportGPX( this );
        Refresh();
    }
}

//---------------------------------------------------------------------------------------------------------
//
//              Private Memory Management
//
//---------------------------------------------------------------------------------------------------------

void *x_malloc( size_t t )
{
    void *pr = malloc( t );

    //      malloc fails
    if( NULL == pr ) {
        wxLogMessage( _T("x_malloc...malloc fails with request of %d bytes."), t );

        // Cat the /proc/meminfo file

        char *p;
        char buf[2000];
        int len;

        int fd = open( "/proc/meminfo", O_RDONLY );

        if( fd == -1 ) exit( 1 );

        len = read( fd, buf, sizeof( buf ) - 1 );
        if( len <= 0 ) {
            close( fd );
            exit( 1 );
        }
        close( fd );
        buf[len] = 0;

        p = buf;
        while( *p ) {
//                        printf("%c", *p++);
        }

        exit( 0 );
        return NULL;                            // for MSVC
    }

    else {
        if( t > malloc_max ) {
            malloc_max = t;
//                      wxLogMessage(_T("New malloc_max: %d", malloc_max));
        }

        return pr;                                      // good return
    }

}


void MyFrame::OnEvtPlugInMessage( OCPN_MsgEvent & event )
{
    wxString message_ID = event.GetID();
    wxString message_JSONText = event.GetJSONText();

    //  We are free to use or ignore any or all of the PlugIn messages flying thru this pipe tee.

    //  We can possibly use the estimated magnetic variation if WMM_pi is present and active
    //  and we have no other source of Variation
    if(!g_bVAR_Rx) {
        if(message_ID == _T("WMM_VARIATION_BOAT")) {

        // construct the JSON root object
            wxJSONValue  root;
        // construct a JSON parser
            wxJSONReader reader;

        // now read the JSON text and store it in the 'root' structure
        // check for errors before retreiving values...
            int numErrors = reader.Parse( message_JSONText, &root );
            if ( numErrors > 0 )  {
//              const wxArrayString& errors = reader.GetErrors();
                return;
            }

            // get the DECL value from the JSON message
            wxString decl = root[_T("Decl")].AsString();
            double decl_val;
            decl.ToDouble(&decl_val);

            gVar = decl_val;
            g_bVARValid = true;
        }
    }
}

void MyFrame::OnEvtTHREADMSG( wxCommandEvent & event )
{
    wxLogMessage( event.GetString() );
}


bool MyFrame::EvalPriority( wxString str_buf, DataStream *pDS )
{
    bool bret = true;
    wxString msg_type = str_buf.Mid(1, 5);
    
    int priority = 0;
    if(pDS)
        priority = pDS->GetPriority();
    
    //  If the message type has never been seen before...
    if( NMEA_Msg_Hash.find( msg_type ) == NMEA_Msg_Hash.end() ) {
        NMEA_Msg_Container *pcontainer = new NMEA_Msg_Container;
        pcontainer-> current_priority = -1;     //  guarantee to execute the next clause
        pcontainer->pDataStream = pDS;
        pcontainer->receipt_time = wxDateTime::Now();
        
        NMEA_Msg_Hash[msg_type] = pcontainer;
    }
    
    NMEA_Msg_Container *pcontainer = NMEA_Msg_Hash[msg_type];
    wxString old_port;
    if( pcontainer->pDataStream )
        old_port = pcontainer->pDataStream->GetPort();
    else
        old_port = _T("PlugIn Virtual");
    
    int old_priority = pcontainer->current_priority;
    
    //  If the message has been seen before, and the priority is greater than or equal to current priority,
    //  then simply update the record
    if( priority >= pcontainer->current_priority ) {
        pcontainer->receipt_time = wxDateTime::Now();
        pcontainer-> current_priority = priority;
        pcontainer->pDataStream = pDS;
        
        bret = true;
    }
    
    //  If the message has been seen before, and the priority is less than the current priority,
    //  then if the time since the last recorded message is greater than GPS_TIMEOUT_SECONDS
    //  then update the record with the new priority and stream.
    //  Otherwise, ignore the message as too low a priority
    else {
        if( (wxDateTime::Now().GetTicks() - pcontainer->receipt_time.GetTicks()) > GPS_TIMEOUT_SECONDS ) {
            pcontainer->receipt_time = wxDateTime::Now();
            pcontainer-> current_priority = priority;
            pcontainer->pDataStream = pDS;
            
            bret = true;
        }
        else
            bret = false;
    }
 
    wxString new_port;
    if( pcontainer->pDataStream )
        new_port = pcontainer->pDataStream->GetPort();
    else
        new_port = _T("PlugIn Virtual");
 
    //  If the data source or priority has changed for this message type, emit a log entry
    if (pcontainer->current_priority != old_priority ||
        new_port != old_port )
        {
            wxLogMessage(wxString::Format(_T("Changing NMEA Datasource for %s to %s (Priority: %i)"),
                                          msg_type.c_str(),
                                          new_port.c_str(),
                                          pcontainer->current_priority) );
        }
        
    return bret;
}

void MyFrame::OnEvtOCPN_NMEA( OCPN_DataStreamEvent & event )
{
    wxString sfixtime;
    bool bshow_tick = false;
    bool bis_recognized_sentence = true; //PL

    wxString str_buf = wxString(event.GetNMEAString().c_str(), wxConvUTF8);
    
    if( g_nNMEADebug && ( g_total_NMEAerror_messages < g_nNMEADebug ) ) {
        g_total_NMEAerror_messages++;
        wxString msg( _T("MEH.NMEA Sentence received...") );
        msg.Append( str_buf );
        wxLogMessage( msg );
    }
    
    //    Send NMEA sentences to PlugIns
    if( g_pi_manager ) g_pi_manager->SendNMEASentenceToAllPlugIns( str_buf );

    bool b_accept = EvalPriority( str_buf, event.GetDataStream() );
    if( b_accept ) {
        m_NMEA0183 << str_buf;
        if( m_NMEA0183.PreParse() ) {
            if( m_NMEA0183.LastSentenceIDReceived == _T("RMC") ) {
                if( m_NMEA0183.Parse() ) {
                    if( m_NMEA0183.Rmc.IsDataValid == NTrue ) {
                        if( !wxIsNaN(m_NMEA0183.Rmc.Position.Latitude.Latitude) ) {
                            double llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
                            int lat_deg_int = (int) ( llt / 100 );
                            double lat_deg = lat_deg_int;
                            double lat_min = llt - ( lat_deg * 100 );
                            gLat = lat_deg + ( lat_min / 60. );
                            if( m_NMEA0183.Rmc.Position.Latitude.Northing == South ) gLat = -gLat;
                        }

                        if( !wxIsNaN(m_NMEA0183.Rmc.Position.Longitude.Longitude) ) {
                            double lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                            int lon_deg_int = (int) ( lln / 100 );
                            double lon_deg = lon_deg_int;
                            double lon_min = lln - ( lon_deg * 100 );
                            gLon = lon_deg + ( lon_min / 60. );
                            if( m_NMEA0183.Rmc.Position.Longitude.Easting == West ) gLon = -gLon;
                        }
                        gSog = m_NMEA0183.Rmc.SpeedOverGroundKnots;
                        gCog = m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue;

                        if( !wxIsNaN(m_NMEA0183.Rmc.MagneticVariation) ) {
                            if( m_NMEA0183.Rmc.MagneticVariationDirection == East ) gVar =
                                    m_NMEA0183.Rmc.MagneticVariation;
                            else
                                if( m_NMEA0183.Rmc.MagneticVariationDirection == West ) gVar =
                                        -m_NMEA0183.Rmc.MagneticVariation;

                            g_bVARValid = true;
                            g_bVAR_Rx = true;
                            gVAR_Watchdog = gps_watchdog_timeout_ticks;

                        }

                        sfixtime = m_NMEA0183.Rmc.UTCTime;

                        gGPS_Watchdog = gps_watchdog_timeout_ticks;

                        bshow_tick = true;
                    }
                } else
                    if( g_nNMEADebug ) {
                        wxString msg( _T("   ") );
                        msg.Append( m_NMEA0183.ErrorMessage );
                        msg.Append( _T(" : ") );
                        msg.Append( str_buf );
                        wxLogMessage( msg );
                    }

            }

            else
                if( m_NMEA0183.LastSentenceIDReceived == _T("HDT") ) {
                    if( m_NMEA0183.Parse() ) {
                        if( !wxIsNaN(m_NMEA0183.Hdt.DegreesTrue) ) {
                            gHdt = m_NMEA0183.Hdt.DegreesTrue;
                            g_bHDTValid = true;
                            g_bHDT_Rx = true;
                            gHDT_Watchdog = gps_watchdog_timeout_ticks;
                        }
                    } else
                        if( g_nNMEADebug ) {
                            wxString msg( _T("   ") );
                            msg.Append( m_NMEA0183.ErrorMessage );
                            msg.Append( _T(" : ") );
                            msg.Append( str_buf );
                            wxLogMessage( msg );
                        }

                }

                else
                    if( m_NMEA0183.LastSentenceIDReceived == _T("HDG") ) {
                        if( m_NMEA0183.Parse() ) {
                            if( !wxIsNaN(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees) ) {
                                gHdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;

                                if( !wxIsNaN(m_NMEA0183.Hdg.MagneticVariationDegrees) ) {
                                    if( m_NMEA0183.Hdg.MagneticVariationDirection == East ) gVar =
                                            m_NMEA0183.Hdg.MagneticVariationDegrees;
                                    else
                                        if( m_NMEA0183.Hdg.MagneticVariationDirection == West ) gVar =
                                                -m_NMEA0183.Hdg.MagneticVariationDegrees;

                                    g_bVARValid = true;
                                    g_bVAR_Rx = true;
                                    gVAR_Watchdog = gps_watchdog_timeout_ticks;
                                }

                                g_bHDxValid = true;
                                gHDx_Watchdog = gps_watchdog_timeout_ticks;
                            }
                        } else
                            if( g_nNMEADebug ) {
                                wxString msg( _T("   ") );
                                msg.Append( m_NMEA0183.ErrorMessage );
                                msg.Append( _T(" : ") );
                                msg.Append( str_buf );
                                wxLogMessage( msg );
                            }

                    }

                    else
                        if( m_NMEA0183.LastSentenceIDReceived == _T("HDM") ) {
                            if( m_NMEA0183.Parse() ) {
                                if( !wxIsNaN(m_NMEA0183.Hdm.DegreesMagnetic) ) {
                                    gHdm = m_NMEA0183.Hdm.DegreesMagnetic;

                                    g_bHDxValid = true;
                                    gHDx_Watchdog = gps_watchdog_timeout_ticks;
                                }
                            } else
                                if( g_nNMEADebug ) {
                                    wxString msg( _T("   ") );
                                    msg.Append( m_NMEA0183.ErrorMessage );
                                    msg.Append( _T(" : ") );
                                    msg.Append( str_buf );
                                    wxLogMessage( msg );
                                }

                        }

                        else
                            if( m_NMEA0183.LastSentenceIDReceived == _T("VTG") ) {
                                if( m_NMEA0183.Parse() ) {
                                    if( !wxIsNaN(m_NMEA0183.Vtg.SpeedKnots) ) gSog =
                                            m_NMEA0183.Vtg.SpeedKnots;
                                    if( !wxIsNaN(m_NMEA0183.Vtg.TrackDegreesTrue) ) gCog =
                                            m_NMEA0183.Vtg.TrackDegreesTrue;
                                } else
                                    if( g_nNMEADebug ) {
                                        wxString msg( _T("   ") );
                                        msg.Append( m_NMEA0183.ErrorMessage );
                                        msg.Append( _T(" : ") );
                                        msg.Append( str_buf );
                                        wxLogMessage( msg );
                                    }
                            }

                            else
                                if( m_NMEA0183.LastSentenceIDReceived == _T("GSV") ) {
                                    if( m_NMEA0183.Parse() ) {
                                        g_SatsInView = m_NMEA0183.Gsv.SatsInView;
                                        gSAT_Watchdog = sat_watchdog_timeout_ticks;
                                        g_bSatValid = true;

                                    } else
                                        if( g_nNMEADebug ) {
                                            wxString msg( _T("   ") );
                                            msg.Append( m_NMEA0183.ErrorMessage );
                                            msg.Append( _T(" : ") );
                                            msg.Append( str_buf );
                                            wxLogMessage( msg );
                                        }
                                }

                                else
                                    if( g_bUseGLL && m_NMEA0183.LastSentenceIDReceived == _T("GLL") ) {
                                        if( m_NMEA0183.Parse() ) {
                                            if( m_NMEA0183.Gll.IsDataValid == NTrue ) {
                                                if( !wxIsNaN(m_NMEA0183.Gll.Position.Latitude.Latitude) ) {
                                                    double llt =
                                                            m_NMEA0183.Gll.Position.Latitude.Latitude;
                                                    int lat_deg_int = (int) ( llt / 100 );
                                                    double lat_deg = lat_deg_int;
                                                    double lat_min = llt - ( lat_deg * 100 );
                                                    gLat = lat_deg + ( lat_min / 60. );
                                                    if( m_NMEA0183.Gll.Position.Latitude.Northing
                                                            == South ) gLat = -gLat;
                                                }

                                                if( !wxIsNaN(m_NMEA0183.Gll.Position.Longitude.Longitude) ) {
                                                    double lln =
                                                            m_NMEA0183.Gll.Position.Longitude.Longitude;
                                                    int lon_deg_int = (int) ( lln / 100 );
                                                    double lon_deg = lon_deg_int;
                                                    double lon_min = lln - ( lon_deg * 100 );
                                                    gLon = lon_deg + ( lon_min / 60. );
                                                    if( m_NMEA0183.Gll.Position.Longitude.Easting
                                                            == West ) gLon = -gLon;
                                                }

                                                sfixtime = m_NMEA0183.Gll.UTCTime;

                                                gGPS_Watchdog = gps_watchdog_timeout_ticks;

                                                bshow_tick = true;
                                            }
                                        } else
                                            if( g_nNMEADebug ) {
                                                wxString msg( _T("   ") );
                                                msg.Append( m_NMEA0183.ErrorMessage );
                                                msg.Append( _T(" : ") );
                                                msg.Append( str_buf );
                                                wxLogMessage( msg );
                                            }
                                    }

                                    else
                                        if( m_NMEA0183.LastSentenceIDReceived == _T("GGA") ) {
                                            if( m_NMEA0183.Parse() ) {
                                                if( m_NMEA0183.Gga.GPSQuality > 0 ) {
                                                    if( !wxIsNaN(m_NMEA0183.Gll.Position.Latitude.Latitude) ) {
                                                        double llt =
                                                                m_NMEA0183.Gga.Position.Latitude.Latitude;
                                                        int lat_deg_int = (int) ( llt / 100 );
                                                        double lat_deg = lat_deg_int;
                                                        double lat_min = llt - ( lat_deg * 100 );
                                                        gLat = lat_deg + ( lat_min / 60. );
                                                        if( m_NMEA0183.Gga.Position.Latitude.Northing
                                                                == South ) gLat = -gLat;
                                                    }

                                                    if( !wxIsNaN(m_NMEA0183.Gga.Position.Longitude.Longitude) ) {
                                                        double lln =
                                                                m_NMEA0183.Gga.Position.Longitude.Longitude;
                                                        int lon_deg_int = (int) ( lln / 100 );
                                                        double lon_deg = lon_deg_int;
                                                        double lon_min = lln - ( lon_deg * 100 );
                                                        gLon = lon_deg + ( lon_min / 60. );
                                                        if( m_NMEA0183.Gga.Position.Longitude.Easting
                                                                == West ) gLon = -gLon;
                                                    }

                                                    sfixtime = m_NMEA0183.Gga.UTCTime;

                                                    gGPS_Watchdog = gps_watchdog_timeout_ticks;

                                                    g_SatsInView =
                                                            m_NMEA0183.Gga.NumberOfSatellitesInUse;
                                                    gSAT_Watchdog = sat_watchdog_timeout_ticks;
                                                    g_bSatValid = true;

                                                    bshow_tick = true;
                                                }
                                            } else
                                                if( g_nNMEADebug ) {
                                                    wxString msg( _T("   ") );
                                                    msg.Append( m_NMEA0183.ErrorMessage );
                                                    msg.Append( _T(" : ") );
                                                    msg.Append( str_buf );
                                                    wxLogMessage( msg );
                                                }
                                        }
        }
        //      Process ownship (AIVDO) messages from any source
        else if(str_buf.Mid( 1, 5 ).IsSameAs( _T("AIVDO") ) ) {
            GenericPosDatEx gpd;
            AIS_Error nerr = AIS_GENERIC_ERROR;
            if(g_pAIS) 
                nerr = g_pAIS->DecodeSingleVDO(str_buf, &gpd);
            if(nerr == AIS_NoError){
                if( !wxIsNaN(gpd.kLat) )
                    gLat = gpd.kLat;
                if( !wxIsNaN(gpd.kLon) ) 
                    gLon = gpd.kLon;
                
                gCog = gpd.kCog;
                gSog = gpd.kSog;
                
                if( !wxIsNaN(gpd.kVar) ) {
                    gVar = gpd.kVar;
                    g_bVARValid = true;
                    g_bVAR_Rx = true;
                    gVAR_Watchdog = gps_watchdog_timeout_ticks;
                }
                if( !wxIsNaN(gpd.kHdt) ) {
                    gHdt = gpd.kHdt;
                    g_bHDTValid = true;
                    g_bHDT_Rx = true;
                    gHDT_Watchdog = gps_watchdog_timeout_ticks;
                }
                if( !wxIsNaN(gpd.kHdm) ) {
                    gHdm = gpd.kHdm;
                    g_bHDxValid = true;
                    gHDx_Watchdog = gps_watchdog_timeout_ticks;
                }
                
                gGPS_Watchdog = gps_watchdog_timeout_ticks;
                bshow_tick = true;
            }
            else {
                if( g_nNMEADebug && ( g_total_NMEAerror_messages < g_nNMEADebug ) ) {
                    g_total_NMEAerror_messages++;
                    wxString msg( _T("   Invalid AIVDO Sentence...") );
                    msg.Append( str_buf );
                    wxLogMessage( msg );
                }
            }
        }
        
        else {
            bis_recognized_sentence = false;
            if( g_nNMEADebug && ( g_total_NMEAerror_messages < g_nNMEADebug ) ) {
                g_total_NMEAerror_messages++;
                wxString msg( _T("   Unrecognized NMEA Sentence...") );
                msg.Append( str_buf );
                wxLogMessage( msg );
            }
        }

        //    Build and send a Position Fix event to PlugIns
        if( bis_recognized_sentence ) {
            if( g_pi_manager ) {
                GenericPosDatEx GPSData;
                GPSData.kLat = gLat;
                GPSData.kLon = gLon;
                GPSData.kCog = gCog;
                GPSData.kSog = gSog;
                GPSData.kVar = gVar;
                GPSData.kHdm = gHdm;
                GPSData.kHdt = gHdt;

                GPSData.nSats = g_SatsInView;

                //TODO  This really should be the fix time obtained from the NMEA sentence.
                //  To do this, we need to cleanly parse the NMEA date and time fields....
                //  Until that is done, use the current system time.
                wxDateTime now = wxDateTime::Now();
                GPSData.FixTime = now.GetTicks();

                g_pi_manager->SendPositionFixToAllPlugIns( &GPSData );
            }
        }

        if( bis_recognized_sentence ) PostProcessNNEA( bshow_tick, sfixtime );
    }
}

void MyFrame::PostProcessNNEA( bool brx_rmc, wxString &sfixtime )
{
    FilterCogSog();

    //    If gSog is greater than some threshold, we determine that we are "cruising"
    if( gSog > 3.0 ) g_bCruising = true;

    //    Here is the one place we try to create gHdt from gHdm and gVar,
    //    but only if NMEA HDT sentence is not being received

    if( !g_bHDT_Rx ) {
        if( !wxIsNaN(gVar) && !wxIsNaN(gHdm) ) {
            gHdt = gHdm + gVar;
            g_bHDTValid = true;
        }
    }

    if( brx_rmc ) {
        if( g_nNMEADebug ) {
            wxString msg( _T("PostProcess NMEA with valid position") );
            wxLogMessage( msg );
        }

        //      Maintain the validity flags
        bool last_bGPSValid = bGPSValid;
        bGPSValid = true;
        if( !last_bGPSValid ) UpdateGPSCompassStatusBox();

        //      Show a little heartbeat tick in StatusWindow0 on NMEA events
        //      But no faster than 10 hz.
        unsigned long uiCurrentTickCount;
        m_MMEAeventTime.SetToCurrent();
        uiCurrentTickCount = m_MMEAeventTime.GetMillisecond() / 100;           // tenths of a second
        uiCurrentTickCount += m_MMEAeventTime.GetTicks() * 10;
        if( uiCurrentTickCount > m_ulLastNEMATicktime + 1 ) {
            m_ulLastNEMATicktime = uiCurrentTickCount;

            if( tick_idx++ > 6 ) tick_idx = 0;
        }
    }

//    Show gLat/gLon in StatusWindow0

    if( NULL != GetStatusBar() ) {
        char tick_buf[2];
        tick_buf[0] = nmea_tick_chars[tick_idx];
        tick_buf[1] = 0;

        wxString s1( tick_buf, wxConvUTF8 );
        s1 += _(" Ship ");
        s1 += toSDMM( 1, gLat );
        s1 += _T("   ");
        s1 += toSDMM( 2, gLon );
        SetStatusText( s1, STAT_FIELD_TICK );

        wxString sogcog;
        if( wxIsNaN(gSog) ) sogcog.Printf( _T("SOG --- kts  ") );
        else
            sogcog.Printf( _T("SOG %2.2f kts  "), gSog );

        wxString cogs;
        if( wxIsNaN(gCog) ) cogs.Printf( wxString( "COG ---°", wxConvUTF8 ) );
        else
            cogs.Printf( wxString("COG %2.0f°", wxConvUTF8 ), gCog );

        sogcog.Append( cogs );
        SetStatusText( sogcog, STAT_FIELD_SOGCOG );
    }

//    Maintain average COG for Course Up Mode

    if( !wxIsNaN(gCog) ) {
        //    Make a hole
        for( int i = g_COGAvgSec - 1; i > 0; i-- )
            COGTable[i] = COGTable[i - 1];
        COGTable[0] = gCog;

        //
        double sum = 0.;
        for( int i = 0; i < g_COGAvgSec; i++ ) {
            double adder = COGTable[i];

            if( fabs( adder - g_COGAvg ) > 180. ) {
                if( ( adder - g_COGAvg ) > 0. ) adder -= 360.;
                else
                    adder += 360.;
            }

            sum += adder;
        }
        sum /= g_COGAvgSec;

        if( sum < 0. ) sum += 360.;
        else
            if( sum >= 360. ) sum -= 360.;

        g_COGAvg = sum;
    }

#ifdef ocpnUPDATE_SYSTEM_TIME
//      Use the fix time to update the local system clock, only once per session
    if( ( sfixtime.Len() ) && s_bSetSystemTime && ( m_bTimeIsSet == false ) ) {
        wxDateTime Fix_Time;

        if( 6 == sfixtime.Len() )                   // perfectly recognised format?
                {
            wxString a;
            long b;
            int hr = 0;
            int min = 0;
            int sec = 0;

            a = sfixtime.Mid( 0, 2 );
            if( a.ToLong( &b ) ) hr = b;
            a = sfixtime.Mid( 2, 2 );
            if( a.ToLong( &b ) ) min = b;
            a = sfixtime.Mid( 4, 2 );
            if( a.ToLong( &b ) ) sec = b;

            Fix_Time.Set( hr, min, sec );
        }
        wxString fix_time_format = Fix_Time.Format( _T("%Y-%m-%dT%H:%M:%S") ); // this should show as LOCAL

        //          Compare the server (fix) time to the current system time
        wxDateTime sdt;
        sdt.SetToCurrent();
        wxDateTime cwxft = Fix_Time;                  // take a copy
        wxTimeSpan ts;
        ts = cwxft.Subtract( sdt );

        int b = ( ts.GetSeconds() ).ToLong();

        //          Correct system time if necessary
        //      Only set the time if wrong by more than 1 minute, and less than 2 hours
        //      This should eliminate bogus times which may come from faulty GPS units

        if( ( abs( b ) > 60 ) && ( abs( b ) < ( 2 * 60 * 60 ) ) ) {

#ifdef __WXMSW__
            //      Fix up the fix_time to convert to GMT
            Fix_Time = Fix_Time.ToGMT();

            //    Code snippet following borrowed from wxDateCtrl, MSW

            const wxDateTime::Tm tm( Fix_Time.GetTm() );

            SYSTEMTIME stm;
            stm.wYear = (WXWORD) tm.year;
            stm.wMonth = (WXWORD) ( tm.mon - wxDateTime::Jan + 1 );
            stm.wDay = tm.mday;

            stm.wDayOfWeek = 0;
            stm.wHour = Fix_Time.GetHour();
            stm.wMinute = tm.min;
            stm.wSecond = tm.sec;
            stm.wMilliseconds = 0;

            ::SetSystemTime( &stm );            // in GMT

#else

            //      This contortion sets the system date/time on POSIX host
            //      Requires the following line in /etc/sudoers
            //          nav ALL=NOPASSWD:/bin/date -s *

            wxString msg;
            msg.Printf(_T("Setting system time, delta t is %d seconds"), b);
            wxLogMessage(msg);

            wxString sdate(Fix_Time.Format(_T("%D")));
            sdate.Prepend(_T("sudo /bin/date -s \""));

            wxString stime(Fix_Time.Format(_T("%T")));
            stime.Prepend(_T(" "));
            sdate.Append(stime);
            sdate.Append(_T("\""));

            msg.Printf(_T("Linux command is:"));
            msg += sdate;
            wxLogMessage(msg);
            wxExecute(sdate, wxEXEC_ASYNC);

#endif      //__WXMSW__
            m_bTimeIsSet = true;

        }           // if needs correction
    }               // if valid time

#endif            //ocpnUPDATE_SYSTEM_TIME
}

void MyFrame::FilterCogSog( void )
{
    if( g_bfilter_cogsog ) {
        //    If the data are undefined, leave the array intact
        if( !wxIsNaN(gCog) ) {
            //    Simple averaging filter for COG
            double cog_last = gCog;       // most recent reported value

            //    Make a hole in array
            for( int i = g_COGFilterSec - 1; i > 0; i-- )
                COGFilterTable[i] = COGFilterTable[i - 1];
            COGFilterTable[0] = cog_last;

            //
            double sum = 0.;
            for( int i = 0; i < g_COGFilterSec; i++ ) {
                double adder = COGFilterTable[i];

                if( fabs( adder - m_COGFilterLast ) > 180. ) {
                    if( ( adder - m_COGFilterLast ) > 0. ) adder -= 360.;
                    else
                        adder += 360.;
                }

                sum += adder;
            }
            sum /= g_COGFilterSec;

            if( sum < 0. ) sum += 360.;
            else
                if( sum >= 360. ) sum -= 360.;

            gCog = sum;
            m_COGFilterLast = sum;
        }

        //    If the data are undefined, leave the array intact
        if( !wxIsNaN(gSog) ) {
            //    Simple averaging filter for SOG
            double sog_last = gSog;       // most recent reported value

            //    Make a hole in array
            for( int i = g_SOGFilterSec - 1; i > 0; i-- )
                SOGFilterTable[i] = SOGFilterTable[i - 1];
            SOGFilterTable[0] = sog_last;

            double sum = 0.;
            for( int i = 0; i < g_SOGFilterSec; i++ ) {
                sum += SOGFilterTable[i];
            }
            sum /= g_SOGFilterSec;

            gSog = sum;
        }
    }
}

void MyFrame::StopSockets( void )
{
//TODO: Can be removed?
}

void MyFrame::ResumeSockets( void )
{
//TODO: Can be removed?
}

void MyFrame::LoadHarmonics()
{
    if(!ptcmgr) {
        ptcmgr = new TCMgr;
        ptcmgr->LoadDataSources(TideCurrentDataSet);
    }
    else {
        bool b_newdataset = false;

        //      Test both ways
        wxArrayString test = ptcmgr->GetDataSet();
        for(unsigned int i=0 ; i < test.GetCount() ; i++) {
            bool b_foundi = false;
            for(unsigned int j=0 ; j < TideCurrentDataSet.GetCount() ; j++) {
                if(TideCurrentDataSet.Item(j) == test.Item(i)) {
                    b_foundi = true;
                    break;              // j loop
                }
            }
            if(!b_foundi) {
                b_newdataset = true;
                break;                  //  i loop
            }
        }

        test = TideCurrentDataSet;
        for(unsigned int i=0 ; i < test.GetCount() ; i++) {
            bool b_foundi = false;
            for(unsigned int j=0 ; j < ptcmgr->GetDataSet().GetCount() ; j++) {
                if(ptcmgr->GetDataSet().Item(j) == test.Item(i)) {
                    b_foundi = true;
                    break;              // j loop
                }
            }
            if(!b_foundi) {
                b_newdataset = true;
                break;                  //  i loop
            }
        }

        if(b_newdataset)
            ptcmgr->LoadDataSources(TideCurrentDataSet);
    }
}

//----------------------------------------------------------------------------------------------------------
//      Application-wide CPL Error handler
//----------------------------------------------------------------------------------------------------------
#ifdef USE_S57
void MyCPLErrorHandler( CPLErr eErrClass, int nError, const char * pszErrorMsg )

{
    char msg[256];

    if( eErrClass == CE_Debug )
    snprintf( msg, 255, "CPL: %s", pszErrorMsg );
    else
        if( eErrClass == CE_Warning )
        snprintf( msg, 255, "CPL Warning %d: %s", nError, pszErrorMsg );
        else
            snprintf( msg, 255, "CPL ERROR %d: %s", nError, pszErrorMsg );

    wxString str( msg, wxConvUTF8 );
    wxLogMessage( str );
}
#endif

//----------------------------------------------------------------------------------------------------------
//      Printing Framework Support
//----------------------------------------------------------------------------------------------------------

bool MyPrintout::OnPrintPage( int page )
{
    wxDC *dc = GetDC();
    if( dc ) {
        if( page == 1 ) DrawPageOne( dc );

        return true;
    } else
        return false;
}

bool MyPrintout::OnBeginDocument( int startPage, int endPage )
{
    if( !wxPrintout::OnBeginDocument( startPage, endPage ) ) return false;

    return true;
}

void MyPrintout::GetPageInfo( int *minPage, int *maxPage, int *selPageFrom, int *selPageTo )
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool MyPrintout::HasPage( int pageNum )
{
    return ( pageNum == 1 );
}

void MyPrintout::DrawPageOne( wxDC *dc )
{

    // Get the Size of the Chart Canvas
    int sx, sy;
    cc1->GetClientSize( &sx, &sy );                       // of the canvas

    float maxX = sx;
    float maxY = sy;

    // Let's have at least some device units margin
    float marginX = 50;
    float marginY = 50;

    // Add the margin to the graphic size
    maxX += ( 2 * marginX );
    maxY += ( 2 * marginY );

    // Get the size of the DC in pixels
    int w, h;
    dc->GetSize( &w, &h );

    // Calculate a suitable scaling factor
    float scaleX = (float) ( w / maxX );
    float scaleY = (float) ( h / maxY );

    // Use x or y scaling factor, whichever fits on the DC
    float actualScale = wxMin(scaleX,scaleY);

    // Calculate the position on the DC for centring the graphic
    float posX = (float) ( ( w - ( maxX * actualScale ) ) / 2.0 );
    float posY = (float) ( ( h - ( maxY * actualScale ) ) / 2.0 );

    posX = wxMax(posX, marginX);
    posY = wxMax(posY, marginY);

    // Set the scale and origin
    dc->SetUserScale( actualScale, actualScale );
    dc->SetDeviceOrigin( (long) posX, (long) posY );

//  Get the latest bitmap as rendered by the ChartCanvas

    if(g_bopengl) {
        int gsx = cc1->GetglCanvas()->GetSize().x;
        int gsy = cc1->GetglCanvas()->GetSize().y;

        unsigned char *buffer = (unsigned char *)malloc( gsx * gsy * 3 );
        glReadPixels(0, 0, gsx, gsy, GL_RGB, GL_UNSIGNED_BYTE, buffer );
        wxImage image( gsx,gsy );
        image.SetData(buffer);
        wxImage mir_imag = image.Mirror( false );
        wxBitmap bmp( mir_imag );
        wxMemoryDC mdc;
        mdc.SelectObject( bmp );
        dc->Blit( 0, 0, bmp.GetWidth(), bmp.GetHeight(), &mdc, 0, 0 );
        mdc.SelectObject( wxNullBitmap );
    }
    else {

//  And Blit/scale it onto the Printer DC
        wxMemoryDC mdc;
        mdc.SelectObject( *( cc1->pscratch_bm ) );

        dc->Blit( 0, 0, cc1->pscratch_bm->GetWidth(), cc1->pscratch_bm->GetHeight(), &mdc, 0, 0 );

        mdc.SelectObject( wxNullBitmap );
    }

}

//---------------------------------------------------------------------------------------
//
//		GPS Positioning Device Detection
//
//---------------------------------------------------------------------------------------

/*
 *     Enumerate all the serial ports on the system
 *
 *     wxArrayString *EnumerateSerialPorts(void)

 *     Very system specific, unavoidably.
 */

#ifdef __WXGTK__
extern "C" int wait(int *);                     // POSIX wait() for process
#endif

wxArrayString *EnumerateSerialPorts( void )
{
    wxArrayString *preturn = new wxArrayString;

#ifdef __WXGTK__

    //    Looking for user privilege openable devices in /dev

    wxString sdev;

    for(int idev=0; idev < 8; idev++)
    {
        sdev.Printf(_T("/dev/ttyS%1d"), idev);

        int fd = open(sdev.mb_str(), O_RDWR|O_NDELAY|O_NOCTTY);
        if(fd > 0)
        {
            /*  add to the output array  */
            preturn->Add(wxString(sdev));
            close(fd);
        }
    }

    for(int idev=0; idev < 8; idev++)
    {
        sdev.Printf(_T("/dev/ttyUSB%1d"), idev);

        int fd = open(sdev.mb_str(), O_RDWR|O_NDELAY|O_NOCTTY);
        if(fd > 0)
        {
            /*  add to the output array  */
            preturn->Add(wxString(sdev));
            close(fd);
        }
    }

    //    Looking for BlueTooth GPS devices
    for(int idev=0; idev < 8; idev++)
    {
        sdev.Printf(_T("/dev/rfcomm%1d"), idev);

        int fd = open(sdev.mb_str(), O_RDWR|O_NDELAY|O_NOCTTY);
        if(fd > 0)
        {
            /*  add to the output array  */
            preturn->Add(wxString(sdev));
            close(fd);
        }
    }

    //    A Fallback position, in case udev has failed or something.....
    if(preturn->IsEmpty())
    {
        preturn->Add( _T("/dev/ttyS0"));
        preturn->Add( _T("/dev/ttyS1"));
        preturn->Add( _T("/dev/ttyUSB0"));
        preturn->Add( _T("/dev/ttyUSB1"));
    }

#endif

#ifdef PROBE_PORTS__WITH_HELPER

    /*
     *     For modern Linux/(Posix??) systems, we may use
     *     the system files /proc/tty/driver/serial
     *     and /proc/tty/driver/usbserial to identify
     *     available serial ports.
     *     A complicating factor is that most (all??) linux
     *     systems require root privileges to access these files.
     *     We will use a helper program method here, despite implied vulnerability.
     */

    char buf[256]; // enough to hold one line from serial devices list
    char left_digit;
    char right_digit;
    int port_num;
    FILE *f;

    pid_t pID = vfork();

    if (pID == 0)// child
    {
//    Temporarily gain root privileges
        seteuid(file_user_id);

//  Execute the helper program
        execlp("ocpnhelper", "ocpnhelper", "-SB", NULL);

//  Return to user privileges
        seteuid(user_user_id);

        wxLogMessage(_T("Warning: ocpnhelper failed...."));
        _exit(0);// If exec fails then exit forked process.
    }

    wait(NULL);                  // for the child to quit

//    Read and parse the files

    /*
     * see if we have any traditional ttySx ports available
     */
    f = fopen("/var/tmp/serial", "r");

    if (f != NULL)
    {
        wxLogMessage(_T("Parsing copy of /proc/tty/driver/serial..."));

        /* read in each line of the file */
        while(fgets(buf, sizeof(buf), f) != NULL)
        {
            wxString sm(buf, wxConvUTF8);
            sm.Prepend(_T("   "));
            sm.Replace(_T("\n"), _T(" "));
            wxLogMessage(sm);

            /* if the line doesn't start with a number get the next line */
            if (buf[0] < '0' || buf[0] > '9')
            continue;

            /*
             * convert digits to an int
             */
            left_digit = buf[0];
            right_digit = buf[1];
            if (right_digit < '0' || right_digit > '9')
            port_num = left_digit - '0';
            else
            port_num = (left_digit - '0') * 10 + right_digit - '0';

            /* skip if "unknown" in the string */
            if (strstr(buf, "unknown") != NULL)
            continue;

            /* upper limit of 15 */
            if (port_num > 15)
            continue;

            /* create string from port_num  */

            wxString s;
            s.Printf(_T("/dev/ttyS%d"), port_num);

            /*  add to the output array  */
            preturn->Add(wxString(s));

        }

        fclose(f);
    }

    /*
     * Same for USB ports
     */
    f = fopen("/var/tmp/usbserial", "r");

    if (f != NULL)
    {
        wxLogMessage(_T("Parsing copy of /proc/tty/driver/usbserial..."));

        /* read in each line of the file */
        while(fgets(buf, sizeof(buf), f) != NULL)
        {

            wxString sm(buf, wxConvUTF8);
            sm.Prepend(_T("   "));
            sm.Replace(_T("\n"), _T(" "));
            wxLogMessage(sm);

            /* if the line doesn't start with a number get the next line */
            if (buf[0] < '0' || buf[0] > '9')
            continue;

            /*
             * convert digits to an int
             */
            left_digit = buf[0];
            right_digit = buf[1];
            if (right_digit < '0' || right_digit > '9')
            port_num = left_digit - '0';
            else
            port_num = (left_digit - '0') * 10 + right_digit - '0';

            /* skip if "unknown" in the string */
            if (strstr(buf, "unknown") != NULL)
            continue;

            /* upper limit of 15 */
            if (port_num > 15)
            continue;

            /* create string from port_num  */

            wxString s;
            s.Printf(_T("/dev/ttyUSB%d"), port_num);

            /*  add to the output array  */
            preturn->Add(wxString(s));

        }

        fclose(f);
    }

    //    As a fallback, in case seteuid doesn't work....
    //    provide some defaults
    //    This is currently the case for GTK+, which
    //    refuses to run suid.  sigh...

    if(preturn->IsEmpty())
    {
        preturn->Add( _T("/dev/ttyS0"));
        preturn->Add( _T("/dev/ttyS1"));
        preturn->Add( _T("/dev/ttyUSB0"));
        preturn->Add( _T("/dev/ttyUSB1"));
    }

//    Clean up the temporary files created by helper.
    pid_t cpID = vfork();

    if (cpID == 0)// child
    {
//    Temporarily gain root privileges
        seteuid(file_user_id);

//  Execute the helper program
        execlp("ocpnhelper", "ocpnhelper", "-U", NULL);

//  Return to user privileges
        seteuid(user_user_id);
        _exit(0);// If exec fails then exit forked process.
    }

#endif      // __WXGTK__
#ifdef __WXOSX__
#include "macutils.h"
    char* paPortNames[MAX_SERIAL_PORTS];
    int iPortNameCount;

    memset(paPortNames,0x00,sizeof(paPortNames));
    iPortNameCount = FindSerialPortNames(&paPortNames[0],MAX_SERIAL_PORTS);
    for (int iPortIndex=0; iPortIndex<iPortNameCount; iPortIndex++)
    {
        wxString sm(paPortNames[iPortIndex], wxConvUTF8);
        preturn->Add(sm);
        free(paPortNames[iPortIndex]);
    }
#endif      //__WXOSX__
#ifdef __WXMSW__
    /*************************************************************************
     * Windows provides no system level enumeration of available serial ports
     * There are several ways of doing this.
     *
     *************************************************************************/

#include <windows.h>

    //    Method 1:  Use GetDefaultCommConfig()
    // Try first {g_nCOMPortCheck} possible COM ports, check for a default configuration
    for( int i = 1; i < g_nCOMPortCheck; i++ ) {
        wxString s;
        s.Printf( _T("COM%d"), i );

        COMMCONFIG cc;
        DWORD dwSize = sizeof(COMMCONFIG);
        if( GetDefaultCommConfig( s.fn_str(), &cc, &dwSize ) ) preturn->Add( wxString( s ) );
    }

#if 0
    // Method 2:  Use FileOpen()
    // Try all 255 possible COM ports, check to see if it can be opened, or if
    // not, that an expected error is returned.

    BOOL bFound;
    for (int j=1; j<256; j++)
    {
        char s[20];
        sprintf(s, "\\\\.\\COM%d", j);

        // Open the port tentatively
        BOOL bSuccess = FALSE;
        HANDLE hComm = ::CreateFile(s, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

        //  Check for the error returns that indicate a port is there, but not currently useable
        if (hComm == INVALID_HANDLE_VALUE)
        {
            DWORD dwError = GetLastError();

            if (dwError == ERROR_ACCESS_DENIED ||
                    dwError == ERROR_GEN_FAILURE ||
                    dwError == ERROR_SHARING_VIOLATION ||
                    dwError == ERROR_SEM_TIMEOUT)
            bFound = TRUE;
        }
        else
        {
            bFound = TRUE;
            CloseHandle(hComm);
        }

        if (bFound)
        preturn->Add(wxString(s));
    }
#endif

//    Search for Garmin device driver on Windows platforms

    HDEVINFO hdeviceinfo = INVALID_HANDLE_VALUE;

    hdeviceinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_DETECT_GUID, NULL, NULL,
            DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

    if( hdeviceinfo != INVALID_HANDLE_VALUE ) {
        wxLogMessage( _T("EnumerateSerialPorts() Found Garmin USB Driver.") );
        preturn->Add( _T("Garmin-USB") );         // Add generic Garmin selectable device
    }

#if 0
    SP_DEVICE_INTERFACE_DATA deviceinterface;
    deviceinterface.cbSize = sizeof(deviceinterface);

    if (SetupDiEnumDeviceInterfaces(hdeviceinfo,
                    NULL,
                    (GUID *) &GARMIN_DETECT_GUID,
                    0,
                    &deviceinterface))
    {
        wxLogMessage(_T("Found Garmin Device."));

        preturn->Add(_T("GARMIN"));         // Add generic Garmin selectable device
    }
#endif

#endif      //__WXMSW__
    return preturn;
}

void appendOSDirSlash( wxString* pString )
{
    wxChar sep = wxFileName::GetPathSeparator();
    if( pString->Last() != sep ) pString->Append( sep );
}

/*************************************************************************
 * Global color management routines
 *
 *************************************************************************/

wxColour GetGlobalColor( wxString colorName )
{
    wxColour ret_color;

#ifdef USE_S57
    //    Use the S52 Presentation library if present
    if( ps52plib ) {
        ret_color = ps52plib->getwxColour( colorName );

        if( !ret_color.Ok() )           //261 likes Ok(), 283 likes IsOk()...
        {
            if( NULL != pcurrent_user_color_hash ) ret_color =
                    ( *pcurrent_user_color_hash )[colorName];
        }
    } else
#endif
    {
        if( NULL != pcurrent_user_color_hash ) ret_color = ( *pcurrent_user_color_hash )[colorName];
    }

    //    Default
    if( !ret_color.Ok() ) ret_color.Set( 128, 128, 128 );  // Simple Grey

    return ret_color;
}

static const char *usercolors[] = { "Table:DAY", "GREEN1;120;255;120;", "GREEN2; 45;150; 45;",
        "GREEN3;200;220;200;", "GREEN4;  0;255;  0;", "BLUE1; 170;170;255;", "BLUE2;  45; 45;170;",
        "BLUE3;   0;  0;255;", "GREY1; 200;200;200;", "GREY2; 230;230;230;", "RED1;  220;200;200;",
        "UBLCK;   0;  0;  0;", "UWHIT; 255;255;255;", "URED;  255;  0;  0;", "UGREN;   0;255;  0;",
        "YELO1; 243;229; 47;", "YELO2; 128; 80;  0;", "TEAL1;   0;128;128;",
        "DILG0; 238;239;242;",              // Dialog Background white
        "DILG1; 212;208;200;",              // Dialog Background
        "DILG2; 255;255;255;",              // Control Background
        "DILG3;   0;  0;  0;",              // Text
        "UITX1;   0;  0;  0;",              // Menu Text, derived from UINFF
        "UDKRD; 124; 16;  0;",
        "UARTE; 200;  0;  0;",              // Active Route, Grey on Dusk/Night
        "DASHB; 255;255;255;",              // Dashboard Instr background
        "DASHL; 190;190;190;",              // Dashboard Instr Label
        "DASHF;  50; 50; 50;",              // Dashboard Foreground
        "DASHR; 200;  0;  0;",              // Dashboard Red
        "DASHG;   0;200;  0;",              // Dashboard Green
        "DASHN; 200;120;  0;",              // Dashboard Needle
        "DASH1; 204;204;255;",              // Dashboard Illustrations
        "DASH2; 122;131;172;",              // Dashboard Illustrations

        "Table:DUSK", "GREEN1; 60;128; 60;", "GREEN2; 22; 75; 22;", "GREEN3; 80;100; 80;",
        "GREEN4;  0;128;  0;", "BLUE1;  80; 80;160;", "BLUE2;  30; 30;120;", "BLUE3;   0;  0;128;",
        "GREY1; 100;100;100;", "GREY2; 128;128;128;", "RED1;  150;100;100;", "UBLCK;   0;  0;  0;",
        "UWHIT; 255;255;255;", "URED;  120; 54; 11;", "UGREN;  35;110; 20;", "YELO1; 120;115; 24;",
        "YELO2;  64; 40;  0;", "TEAL1;   0; 64; 64;",
        "DILG0; 110;110;110;",              // Dialog Background
        "DILG1; 110;110;110;",              // Dialog Background
        "DILG2;   0;  0;  0;",              // Control Background
        "DILG3; 130;130;130;",              // Text
        "UITX1;  41; 46; 46;",              // Menu Text, derived from UINFF
        "UDKRD;  80;  0;  0;",
        "UARTE;  64; 64; 64;",              // Active Route, Grey on Dusk/Night
        "DASHB;  77; 77; 77;",              // Dashboard Instr background
        "DASHL;  54; 54; 54;",              // Dashboard Instr Label
        "DASHF;   0;  0;  0;",              // Dashboard Foreground
        "DASHR;  58; 21; 21;",              // Dashboard Red
        "DASHG;  21; 58; 21;",              // Dashboard Green
        "DASHN; 100; 50;  0;",              // Dashboard Needle
        "DASH1;  76; 76;113;",              // Dashboard Illustrations
        "DASH2;  48; 52; 72;",              // Dashboard Illustrations

        "Table:NIGHT", "GREEN1; 30; 80; 30;", "GREEN2; 15; 60; 15;", "GREEN3; 12; 23;  9;",
        "GREEN4;  0; 64;  0;", "BLUE1;  60; 60;100;", "BLUE2;  22; 22; 85;", "BLUE3;   0;  0; 40;",
        "GREY1;  48; 48; 48;", "GREY2;  64; 64; 64;", "RED1;  100; 50; 50;", "UWHIT; 255;255;255;",
        "UBLCK;   0;  0;  0;", "URED;   60; 27;  5;", "UGREN;  17; 55; 10;", "YELO1;  60; 65; 12;",
        "YELO2;  32; 20;  0;", "TEAL1;   0; 32; 32;",
        "DILG0;  80; 80; 80;",              // Dialog Background
        "DILG1;  80; 80; 80;",              // Dialog Background
        "DILG2;   0;  0;  0;",              // Control Background
        "DILG3;  65; 65; 65;",              // Text
        "UITX1;  31; 34; 35;",              // Menu Text, derived from UINFF
        "UDKRD;  50;  0;  0;",
        "UARTE;  64; 64; 64;",              // Active Route, Grey on Dusk/Night
        "DASHB;   0;  0;  0;",              // Dashboard Instr background
        "DASHL;  20; 20; 20;",              // Dashboard Instr Label
        "DASHF;  64; 64; 64;",              // Dashboard Foreground
        "DASHR;  70; 15; 15;",              // Dashboard Red
        "DASHG;  15; 70; 15;",              // Dashboard Green
        "DASHN;  17; 80; 56;",              // Dashboard Needle
        "DASH1;  48; 52; 72;",              // Dashboard Illustrations
        "DASH2;  36; 36; 53;",              // Dashboard Illustrations

        "*****" };

int get_static_line( char *d, const char **p, int index, int n )
{
    if( !strcmp( p[index], "*****" ) ) return 0;

    strncpy( d, p[index], n );
    return strlen( d );
}

void InitializeUserColors( void )
{
    const char **p = usercolors;
    char buf[80];
    int index = 0;
    char TableName[20];
    colTable *ctp;
    colTable *ct;
    int colIdx = 0;
    int R, G, B;

    UserColorTableArray = new wxArrayPtrVoid;
    UserColourHashTableArray = new wxArrayPtrVoid;

    //    Create 3 color table entries
    ct = new colTable;
    ct->tableName = new wxString( _T("DAY") );
    ct->color = new wxArrayPtrVoid;
    UserColorTableArray->Add( (void *) ct );

    ct = new colTable;
    ct->tableName = new wxString( _T("DUSK") );
    ct->color = new wxArrayPtrVoid;
    UserColorTableArray->Add( (void *) ct );

    ct = new colTable;
    ct->tableName = new wxString( _T("NIGHT") );
    ct->color = new wxArrayPtrVoid;
    UserColorTableArray->Add( (void *) ct );

    while( ( get_static_line( buf, p, index, 80 ) ) ) {
        if( !strncmp( buf, "Table", 5 ) ) {
            sscanf( buf, "Table:%s", TableName );

            for( unsigned int it = 0; it < UserColorTableArray->GetCount(); it++ ) {
                ctp = (colTable *) ( UserColorTableArray->Item( it ) );
                if( !strcmp( TableName, ctp->tableName->mb_str() ) ) {
                    ct = ctp;
                    colIdx = 0;
                    break;
                }
            }

        } else {
            char name[80];
            int j = 0;
            while( buf[j] != ';' ) {
                name[j] = buf[j];
                j++;
            }
            name[j] = 0;

            S52color *c = new S52color;
            strcpy( c->colName, name );

            sscanf( &buf[j], ";%i;%i;%i", &R, &G, &B );
            c->R = (char) R;
            c->G = (char) G;
            c->B = (char) B;

            ct->color->Add( c );

        }

        index++;
    }

    //    Now create the Hash tables

    for( unsigned int its = 0; its < UserColorTableArray->GetCount(); its++ ) {
        wxColorHashMap *phash = new wxColorHashMap;
        UserColourHashTableArray->Add( (void *) phash );

        colTable *ctp = (colTable *) ( UserColorTableArray->Item( its ) );

        for( unsigned int ic = 0; ic < ctp->color->GetCount(); ic++ ) {
            S52color *c2 = (S52color *) ( ctp->color->Item( ic ) );

            wxColour c( c2->R, c2->G, c2->B );
            wxString key( c2->colName, wxConvUTF8 );
            ( *phash )[key] = c;

        }
    }

    //    Establish a default hash table pointer
    //    in case a color is needed before ColorScheme is set
    pcurrent_user_color_hash = (wxColorHashMap *) UserColourHashTableArray->Item( 0 );
}

void DeInitializeUserColors( void )
{
    unsigned int i;
    for( i = 0; i < UserColorTableArray->GetCount(); i++ ) {
        colTable *ct = (colTable *) UserColorTableArray->Item( i );

        for( unsigned int j = 0; j < ct->color->GetCount(); j++ ) {
            S52color *c = (S52color *) ct->color->Item( j );
            delete c;                     //color
        }

        delete ct->tableName;               // wxString
        delete ct->color;                   // wxArrayPtrVoid

        delete ct;                          // colTable
    }

    delete UserColorTableArray;

    for( i = 0; i < UserColourHashTableArray->GetCount(); i++ ) {
        wxColorHashMap *phash = (wxColorHashMap *) UserColourHashTableArray->Item( i );
        delete phash;
    }

    delete UserColourHashTableArray;

}

#ifdef __WXMSW__

#define NCOLORS 40

typedef struct _MSW_COLOR_SPEC {
    int COLOR_NAME;
    wxString S52_RGB_COLOR;
    int SysRGB_COLOR;
} MSW_COLOR_SPEC;

MSW_COLOR_SPEC color_spec[] = { { COLOR_MENU, _T("UIBCK"), 0 }, { COLOR_MENUTEXT, _T("UITX1"), 0 },
        { COLOR_BTNSHADOW, _T("UIBCK"), 0 },                        // Menu Frame
        { -1, _T(""), 0 } };

void SaveSystemColors()
{
    /*
     color_3dface = pGetSysColor(COLOR_3DFACE);
     color_3dhilite = pGetSysColor(COLOR_3DHILIGHT);
     color_3dshadow = pGetSysColor(COLOR_3DSHADOW);
     color_3ddkshadow = pGetSysColor(COLOR_3DDKSHADOW);
     color_3dlight = pGetSysColor(COLOR_3DLIGHT);
     color_activecaption = pGetSysColor(COLOR_ACTIVECAPTION);
     color_gradientactivecaption = pGetSysColor(27); //COLOR_3DLIGHT);
     color_captiontext = pGetSysColor(COLOR_CAPTIONTEXT);
     color_windowframe = pGetSysColor(COLOR_WINDOWFRAME);
     color_inactiveborder = pGetSysColor(COLOR_INACTIVEBORDER);
     */
    //    Record the default system color in my substitution structure
    MSW_COLOR_SPEC *pcspec = &color_spec[0];
    while( pcspec->COLOR_NAME != -1 ) {
        pcspec->SysRGB_COLOR = pGetSysColor( pcspec->COLOR_NAME );
        pcspec++;
    }
}

void RestoreSystemColors()
{
    int element[NCOLORS];
    int rgbcolor[NCOLORS];
    int i = 0;

    MSW_COLOR_SPEC *pcspec = &color_spec[0];
    while( pcspec->COLOR_NAME != -1 ) {
        element[i] = pcspec->COLOR_NAME;
        rgbcolor[i] = pcspec->SysRGB_COLOR;

        pcspec++;
        i++;
    }

    pSetSysColors( i, (unsigned long *) &element[0], (unsigned long *) &rgbcolor[0] );

}

#endif

void SetSystemColors( ColorScheme cs )
{
//---------------
#if 0
    //    This is the list of Color Types from winuser.h
    /*
     * Color Types
     */
#define CTLCOLOR_MSGBOX         0
#define CTLCOLOR_EDIT           1
#define CTLCOLOR_LISTBOX        2
#define CTLCOLOR_BTN            3
#define CTLCOLOR_DLG            4
#define CTLCOLOR_SCROLLBAR      5
#define CTLCOLOR_STATIC         6
#define CTLCOLOR_MAX            7

#define COLOR_SCROLLBAR         0         //??
#define COLOR_BACKGROUND        1         //??
#define COLOR_ACTIVECAPTION     2       //??
#define COLOR_INACTIVECAPTION   3         //??
#define COLOR_MENU              4         // Menu background
#define COLOR_WINDOW            5         // default window background
#define COLOR_WINDOWFRAME       6         // Sub-Window frames, like status bar, etc..
#define COLOR_MENUTEXT          7         // Menu text
#define COLOR_WINDOWTEXT        8         //??
#define COLOR_CAPTIONTEXT       9         //??
#define COLOR_ACTIVEBORDER      10        //??
#define COLOR_INACTIVEBORDER    11       //??
#define COLOR_APPWORKSPACE      12       //??
#define COLOR_HIGHLIGHT         13       //Highlited text background  in query box tree
#define COLOR_HIGHLIGHTTEXT     14        //??
#define COLOR_BTNFACE           15        //??
#define COLOR_BTNSHADOW         16        // Menu Frame
#define COLOR_GRAYTEXT          17        // Greyed out text in menu
#define COLOR_BTNTEXT           18        //??
#define COLOR_INACTIVECAPTIONTEXT 19      //??
#define COLOR_BTNHIGHLIGHT      20        //??
#if(WINVER >= 0x0400)
#define COLOR_3DDKSHADOW        21        //??
#define COLOR_3DLIGHT           22        // Grid rule lines in list control
#define COLOR_INFOTEXT          23        //??
#define COLOR_INFOBK            24
#endif /* WINVER >= 0x0400 */

#if(WINVER >= 0x0500)
#define COLOR_HOTLIGHT          26              //??
#define COLOR_GRADIENTACTIVECAPTION 27        //??
#define COLOR_GRADIENTINACTIVECAPTION 28        //??
#if(WINVER >= 0x0501)
#define COLOR_MENUHILIGHT       29              // Selected item in menu, maybe needs reset on popup menu?
#define COLOR_MENUBAR           30              //??
#endif /* WINVER >= 0x0501 */
#endif /* WINVER >= 0x0500 */

#if(WINVER >= 0x0400)
#define COLOR_DESKTOP           COLOR_BACKGROUND
#define COLOR_3DFACE            COLOR_BTNFACE
#define COLOR_3DSHADOW          COLOR_BTNSHADOW
#define COLOR_3DHIGHLIGHT       COLOR_BTNHIGHLIGHT
#define COLOR_3DHILIGHT         COLOR_BTNHIGHLIGHT
#define COLOR_BTNHILIGHT        COLOR_BTNHIGHLIGHT
#endif /* WINVER >= 0x0400 */
#endif

#ifdef __WXMSW__
    int element[NCOLORS];
    int rgbcolor[NCOLORS];
    int i = 0;
    if( ( GLOBAL_COLOR_SCHEME_DUSK == cs ) || ( GLOBAL_COLOR_SCHEME_NIGHT == cs ) ) {
        MSW_COLOR_SPEC *pcspec = &color_spec[0];
        while( pcspec->COLOR_NAME != -1 ) {
            wxColour color = GetGlobalColor( pcspec->S52_RGB_COLOR );
            rgbcolor[i] = ( color.Red() << 16 ) + ( color.Green() << 8 ) + color.Blue();
            element[i] = pcspec->COLOR_NAME;

            i++;
            pcspec++;
        }

        pSetSysColors( i, (unsigned long *) &element[0], (unsigned long *) &rgbcolor[0] );

    } else {         // for daylight colors, use default windows colors as saved....

        RestoreSystemColors();
    }
#endif
}

int OCPNMessageBox( wxWindow *parent, const wxString& message, const wxString& caption, int style,
        int x, int y )
{

#ifdef __WXOSX__
    if(g_FloatingToolbarDialog)
        g_FloatingToolbarDialog->Hide();
    
    if( g_FloatingCompassDialog )
        g_FloatingCompassDialog->Hide();
    
    if( stats )
        stats->Hide();
#endif
    wxMessageDialog dlg( parent, message, caption, style | wxSTAY_ON_TOP, wxPoint( x, y ) );
    int ret = dlg.ShowModal();

#ifdef __WXOSX__
    gFrame->SurfaceToolbar();

    if( g_FloatingCompassDialog )
        g_FloatingCompassDialog->Show();
    
    if( stats )
        stats->Show();
    
    if(parent)
        parent->Raise();
#endif
    
    return ret;
}

//               A helper function to check for proper parameters of anchor watch
//
double AnchorDistFix( double const d, double const AnchorPointMinDist,
        double const AnchorPointMaxDist )   //  pjotrc 2010.02.22
{
    if( d >= 0.0 ) if( d < AnchorPointMinDist ) return AnchorPointMinDist;
    else
        if( d > AnchorPointMaxDist ) return AnchorPointMaxDist;
        else
            return d;

    else
        //if ( d < 0.0 )
        if( d > -AnchorPointMinDist ) return -AnchorPointMinDist;
        else
            if( d < -AnchorPointMaxDist ) return -AnchorPointMaxDist;
            else
                return d;
}
//  Generic on-screen bitmap dialog
BEGIN_EVENT_TABLE(OCPNBitmapDialog, wxDialog) EVT_PAINT(OCPNBitmapDialog::OnPaint)
END_EVENT_TABLE()

OCPNBitmapDialog::OCPNBitmapDialog( wxWindow *frame, wxPoint position, wxSize size )
{
    long wstyle = wxSIMPLE_BORDER;
    wxDialog::Create( frame, wxID_ANY, _T(""), position, size, wstyle );

    Hide();
}

OCPNBitmapDialog::~OCPNBitmapDialog()
{
}

void OCPNBitmapDialog::SetBitmap( wxBitmap bitmap )
{
    m_bitmap = bitmap;
}

void OCPNBitmapDialog::OnPaint( wxPaintEvent& event )
{

    wxPaintDC dc( this );

    if( m_bitmap.IsOk() ) dc.DrawBitmap( m_bitmap, 0, 0 );
}



//      Console supporting printf functionality for Windows GUI app

// maximum mumber of lines the output console should have

#ifdef __WXMSW__
static const WORD MAX_CONSOLE_LINES = 500;

//#ifdef _DEBUG

void RedirectIOToConsole()

{
    
    int hConHandle;
    
    long lStdHandle;
    
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    
    FILE *fp;
    
    // allocate a console for this app
    
    AllocConsole();
    
    // set the screen buffer to be big enough to let us scroll text
    
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);
    
    // redirect unbuffered STDOUT to the console
    
    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    setvbuf( stdout, NULL, _IONBF, 0 );
    
    
    // redirect unbuffered STDIN to the console
    
    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;
    setvbuf( stdin, NULL, _IONBF, 0 );
    
    // redirect unbuffered STDERR to the console
    
    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf( stderr, NULL, _IONBF, 0 );
    
    // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
    
    //ios::sync_with_stdio();
    
}

//#endif
#endif




