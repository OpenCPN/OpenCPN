/***************************************************************************
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
 **************************************************************************/
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
#include <version.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/clrpicker.h>
#include "wx/tokenzr.h"
#include "wx/dir.h"

#include <wx/dialog.h>

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

#ifdef OCPN_HAVE_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#include "chart1.h"
#include "chcanv.h"
#include "chartdb.h"
#include "navutil.h"
#include "styles.h"
#include "routeman.h"
#include "chartbarwin.h"
#include "concanv.h"
#include "options.h"
#include "about.h"
#include "thumbwin.h"
#include "tcmgr.h"
#include "ais.h"
#include "chartimg.h"               // for ChartBaseBSB
#include "routeprop.h"
#include "toolbar.h"
#include "compass.h"
#include "datastream.h"
#include "OCPN_DataStreamEvent.h"
#include "multiplexer.h"
#include "routeprintout.h"
#include "Select.h"
#include "FontMgr.h"
#include "NMEALogWindow.h"
#include "Layer.h"
#include "NavObjectCollection.h"
#include "AISTargetListDialog.h"
#include "AISTargetAlertDialog.h"
#include "AIS_Decoder.h"
#include "OCP_DataStreamInput_Thread.h"
#include "TrackPropDlg.h"
#include "gshhs.h"
#include "cutil.h"
#include "routemanagerdialog.h"
#include "pluginmanager.h"
#include "AIS_Target_Data.h"
#include "OCPNPlatform.h"
#include "AISTargetQueryDialog.h"
#include "S57QueryDialog.h"
#include "glTexCache.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

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
#include "s52utils.h"
#endif

#ifdef __WXMSW__
//#define __MSVC__LEAK
#ifdef __MSVC__LEAK
#include "Stackwalker.h"
#endif
#endif

#include <wx/jsonreader.h>

#ifdef OCPN_USE_PORTAUDIO
#include "portaudio.h"
#endif

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#ifdef LINUX_CRASHRPT
#include "crashprint.h"
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( ArrayOfCDI );

#ifdef __WXMSW__
void RedirectIOToConsole();
#endif

//------------------------------------------------------------------------------
//      Static variable definition
//------------------------------------------------------------------------------

OCPNPlatform              *g_Platform;

bool                      g_bFirstRun;

int                       g_unit_test_1;
bool                      g_start_fullscreen;
bool                      g_rebuild_gl_cache;

MyFrame                   *gFrame;

ChartCanvas               *cc1;
ConsoleCanvas             *console;
ChartBarWin               *g_ChartBarWin;
Piano                     *g_Piano;
wxWindowList              AppActivateList;

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
TrackPropDlg              *pTrackPropDialog;
RouteManagerDialog        *pRouteManagerDialog;
GoToPositionDialog        *pGoToPositionDialog;

double                    gLat, gLon, gCog, gSog, gHdt, gHdm, gVar;
double                    vLat, vLon;
double                    initial_scale_ppm, initial_rotation;

int                       g_nbrightness;

bool                      bDBUpdateInProgress;

ThumbWin                  *pthumbwin;
TCMgr                     *ptcmgr;

bool                      g_bshowToolbar = true;
bool                      g_bBasicMenus = false;;

bool                      bDrawCurrentValues;

wxString                  ChartListFileName;
wxString                  AISTargetNameFileName;
wxString                  *pWorldMapLocation;
wxString                  *pInit_Chart_Dir;
wxString                  g_csv_locn;
wxString                  g_SENCPrefix;
wxString                  g_UserPresLibData;
wxString                  g_VisibleLayers;
wxString                  g_InvisibleLayers;

#ifdef ocpnUSE_GL
extern CompressionWorkerPool   *g_CompressorPool;
#endif

bool                      g_bcompression_wait;

wxString                  g_uploadConnection;

int                       user_user_id;
int                       file_user_id;

int                       quitflag;
int                       g_tick;
int                       g_mem_total, g_mem_used, g_mem_initial;

bool                      s_bSetSystemTime;

wxString                  *phost_name;

static unsigned int       malloc_max;

wxArrayOfConnPrm          *g_pConnectionParams;

wxDateTime                g_start_time;
wxDateTime                g_loglast_time;
OCPN_Sound                bells_sound[2];

OCPN_Sound                g_anchorwatch_sound;

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
bool                      g_bShowChartBar;
bool                      g_bShowActiveRouteHighway;
int                       g_nNMEADebug;
int                       g_nAWDefault;
int                       g_nAWMax;
bool                      g_bPlayShipsBells;
bool                      g_bFullscreenToolbar;
bool                      g_bShowLayers;
bool                      g_bTransparentToolbar;
bool                      g_bTransparentToolbarInOpenGLOK;
int                       g_nAutoHideToolbar;
bool                      g_bAutoHideToolbar;

bool                      g_bPermanentMOBIcon;
bool                      g_bTempShowMenuBar;

int                       g_iSDMMFormat;
int                       g_iDistanceFormat;
int                       g_iSpeedFormat;

int                       g_iNavAidRadarRingsNumberVisible;
float                     g_fNavAidRadarRingsStep;
int                       g_pNavAidRadarRingsStepUnits;
int                       g_iWaypointRangeRingsNumber;
float                     g_fWaypointRangeRingsStep;
int                       g_iWaypointRangeRingsStepUnits;
wxColour                  g_colourWaypointRangeRingsColour;
bool                      g_bWayPointPreventDragging;
bool                      g_bConfirmObjectDelete;

// Set default color scheme
ColorScheme               global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;

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
double                    g_display_size_mm;
double                    g_config_display_size_mm;
bool                      g_config_display_size_manual;

int                       g_GUIScaleFactor;
int                       g_ChartScaleFactor;
float                     g_ChartScaleFactorExp;

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
double                    g_ownship_HDTpredictor_miles;

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
bool                      g_bSoftwareGL;
bool                      g_bShowFPS;
bool                      g_bsmoothpanzoom;
bool                      g_fog_overzoom;
double                    g_overzoom_emphasis_base;
bool                      g_oz_vector_scale;

int                       g_nCOMPortCheck;

bool                      g_b_legacy_input_filter_behaviour;  // Support original input filter process or new process

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

bool                      g_bdisable_opengl;

ChartGroupArray           *g_pGroupArray;
int                       g_GroupIndex;

wxString                  g_GPS_Ident;

wxProgressDialog          *s_ProgDialog;

S57QueryDialog            *g_pObjectQueryDialog;

wxArrayString             TideCurrentDataSet;
wxString                  g_TCData_Dir;


bool                      g_boptionsactive;
options                   *g_options;
bool                      g_bDeferredInitDone;
int                       options_lastPage = 0;
wxPoint                   options_lastWindowPos( 0,0 );
wxSize                    options_lastWindowSize( 0,0 );

bool                      g_bSleep;

int                       g_grad_default;
wxColour                  g_border_color_default;
int                       g_border_size_default;
int                       g_sash_size_default;
wxColour                  g_caption_color_default;
wxColour                  g_sash_color_default;

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
bool                      g_bHideMoored;
bool                      g_bAllowShowScaled;
double                    g_ShowMoored_Kts;
wxString                  g_sAIS_Alert_Sound_File;
bool                      g_bAIS_CPA_Alert_Suppress_Moored;
bool                      g_bAIS_ACK_Timeout;
double                    g_AckTimeout_Mins;
bool                      g_bShowScaled;
bool                      g_bShowAreaNotices;
bool                      g_bDrawAISSize;
bool                      g_bShowAISName;
int                       g_Show_Target_Name_Scale;
bool                      g_bWplIsAprsPosition;

wxToolBarToolBase         *m_pAISTool;

int                       g_nAIS_activity_timer;

DummyTextCtrl             *g_pDummyTextCtrl;
bool                      g_bEnableZoomToCursor;

bool                      g_bTrackActive;
bool                      g_bTrackCarryOver;
bool                      g_bDeferredStartTrack;
bool                      g_bTrackDaily;
int                       g_track_rotate_time;
int                       g_track_rotate_time_type;
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

int                       g_OwnShipIconType;
double                    g_n_ownship_length_meters;
double                    g_n_ownship_beam_meters;
double                    g_n_gps_antenna_offset_y;
double                    g_n_gps_antenna_offset_x;
int                       g_n_ownship_min_mm;

double                    g_n_arrival_circle_radius;

int                       g_nautosave_interval_seconds;

bool                      g_bPreserveScaleOnX;

about                     *g_pAboutDlg;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
wxLocale                  *plocale_def_lang;
#endif

wxString                  g_locale;
bool                      g_b_assume_azerty;

bool                      g_bUseRaster;
bool                      g_bUseVector;
bool                      g_bUseCM93;

int                       g_click_stop;

int                       g_MemFootSec;
int                       g_MemFootMB;

ArrayOfInts               g_quilt_noshow_index_array;

wxStaticBitmap            *g_pStatBoxTool;
bool                      g_bShowStatusBar;

bool                      g_bquiting;
int                       g_BSBImgDebug;

AISTargetListDialog       *g_pAISTargetList;
wxString                  g_AisTargetList_perspective;
int                       g_AisTargetList_range;
int                       g_AisTargetList_sortColumn;
bool                      g_bAisTargetList_sortReverse;
wxString                  g_AisTargetList_column_spec;
int                       g_AisTargetList_count;
bool                      g_bAisTargetList_autosort;

bool                      g_bGarminHostUpload;
bool                      g_bFullscreen;

wxAuiManager              *g_pauimgr;
wxAuiDefaultDockArt       *g_pauidockart;

bool                      g_blocale_changed;

RoutePrintSelection       *pRoutePrintSelection;

wxMenu                    *g_FloatingToolbarConfigMenu;
wxString                  g_toolbarConfig = _T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

ocpnFloatingToolbarDialog *g_FloatingToolbarDialog;
int                       g_toolbar_x;
int                       g_toolbar_y;
long                      g_toolbar_orient;
wxRect                    g_last_tb_rect;
float                     g_toolbar_scalefactor;
float                     g_compass_scalefactor;

ocpnCompass              *g_Compass;

bool                      g_benable_rotate;

bool                      g_bShowMag;
double                    g_UserVar;
bool                      g_bMagneticAPB;


//                        OpenGL Globals
int                       g_GPU_MemSize;

bool                      g_bserial_access_checked;
wxString                  g_uiStyle;

//      Values returned from WMM_PI for variation computation request
double                    gQueryVar;


char bells_sound_file_name[2][12] = { "1bells.wav", "2bells.wav" };

int                       portaudio_initialized;

static char nmea_tick_chars[] = { '|', '/', '-', '\\', '|', '/', '-', '\\' };
static int tick_idx;

int               g_sticky_chart;
int               g_sticky_projection;

extern wxString OpenCPNVersion; //Gunther
extern options          *g_pOptions;

int n_NavMessageShown;
wxString g_config_version_string;

bool             g_btouch;
bool             g_bresponsive;

bool             b_inCompressAllCharts;
bool             g_bGLexpert;
bool             g_bUIexpert;

int              g_chart_zoom_modifier;

int              g_NMEAAPBPrecision;

wxString         g_TalkerIdText;
int              g_maxWPNameLength;

bool             g_bAdvanceRouteWaypointOnArrivalOnly;

wxArrayString    g_locale_catalog_array;
bool             b_reloadForPlugins;

#ifdef LINUX_CRASHRPT
wxCrashPrint g_crashprint;
#endif

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
extern "C" bool CheckSerialAccess( void );


//------------------------------------------------------------------------------
//    PNG Icon resources
//------------------------------------------------------------------------------

#if defined(__WXGTK__) || defined(__WXQT__)
#include "bitmaps/opencpn.xpm"
#endif

//------------------------------------------------------------------------------
//              Local constants
//------------------------------------------------------------------------------
enum {
    ID_PIANO_DISABLE_QUILT_CHART = 32000, ID_PIANO_ENABLE_QUILT_CHART
};

//------------------------------------------------------------------------------
//              Fwd Refs
//------------------------------------------------------------------------------




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

    wxString vs =
        wxString::Format(wxT(" .. Version %i.%i.%i"),
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    wxMessageDialog odlg( gFrame, msg0, _("Welcome to OpenCPN") + vs, wxCANCEL | wxOK );

    return ( odlg.ShowModal() );
}


wxString newPrivateFileName(wxString home_locn, const char *name, const char *windowsName)
{
    wxString fname = wxString::FromUTF8(name);
    wxString fwname = wxString::FromUTF8(windowsName);
    wxString filePathAndName;

#ifdef __WXMSW__
    filePathAndName = fwname;
    filePathAndName.Prepend( home_locn );

#else
    filePathAndName = g_Platform->GetPrivateDataDir();
    appendOSDirSlash(&filePathAndName);
    filePathAndName.Append( fname );
#endif

    if( g_bportable ) {
        filePathAndName.Clear();
#ifdef __WXMSW__
        filePathAndName.Append( fwname );
#else
        filePathAndName.Append( fname );
#endif
        filePathAndName.Prepend( home_locn );
    }
    return filePathAndName;
}


// `Main program' equivalent, creating windows and returning main app frame
//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------
#ifndef OCPN_USE_WRAPPER
IMPLEMENT_APP( MyApp )
#endif

BEGIN_EVENT_TABLE(MyApp, wxApp)
EVT_ACTIVATE_APP(MyApp::OnActivateApp)
END_EVENT_TABLE()

#include "wx/dynlib.h"

#if wxUSE_CMDLINE_PARSER
void MyApp::OnInitCmdLine( wxCmdLineParser& parser )
{
    //    Add some OpenCPN specific command line options
    parser.AddSwitch( _T("unit_test_1") );
    parser.AddSwitch( _T("p") );
    parser.AddSwitch( _T("no_opengl") );
    parser.AddSwitch( _T("fullscreen") );
    parser.AddSwitch( _T("rebuild_gl_raster_cache") );
}

bool MyApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
    g_unit_test_1 = parser.Found( _T("unit_test_1") );
    g_bportable = parser.Found( _T("p") );
    g_bdisable_opengl = parser.Found( _T("no_opengl") );
    g_start_fullscreen = parser.Found( _T("fullscreen") );
    g_rebuild_gl_cache = parser.Found( _T("rebuild_gl_raster_cache") );

    return true;
}
#endif

#ifdef __WXMSW__
    //  Handle any exception not handled by CrashRpt
    //  Most probable:  Malloc/new failure

bool MyApp::OnExceptionInMainLoop()
{
    wxLogWarning(_T("Caught MainLoopException, continuing..."));
    return true;
}
#endif

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
        if(g_FloatingToolbarDialog) {
            if(g_FloatingToolbarDialog->IsShown())
                g_FloatingToolbarDialog->Submerge();
        }


        AppActivateList.Clear();
        if(cc1){
            for ( wxWindowList::iterator it = cc1->GetChildren().begin(); it != cc1->GetChildren().end(); ++it ) {
                if( (*it)->IsShown() ) {
                    (*it)->Hide();
                    AppActivateList.Append(*it);
                }
            }
        }

        if(gFrame){
            for ( wxWindowList::iterator it = gFrame->GetChildren().begin(); it != gFrame->GetChildren().end(); ++it ) {
                if( (*it)->IsShown() ) {
                    if( !(*it)->IsKindOf( CLASSINFO(ChartCanvas) ) ) {
                        (*it)->Hide();
                        AppActivateList.Append(*it);
                    }
                }
            }
        }
    }
    else
    {
        if(gFrame){
//        printf("App Activate\n");
            gFrame->SubmergeToolbar();              // This is needed to reset internal wxWidgets logic
                                                    // Also required for other TopLevelWindows here
                                                    // reportedly not required for wx 2.9
            gFrame->SurfaceToolbar();

            wxWindow *pOptions = NULL;

            wxWindowListNode *node = AppActivateList.GetFirst();
            while (node) {
                wxWindow *win = node->GetData();
                win->Show();
                if( win->IsKindOf( CLASSINFO(options) ) )
                    pOptions = win;

                node = node->GetNext();
            }

            if( pOptions )
                pOptions->Raise();
            else
                gFrame->Raise();
        }
    }
#endif

    if( !event.GetActive() ) {

        //  Remove a temporary Menubar when the application goes inactive
        //  This is one way to handle properly ALT-TAB navigation on the Windows desktop
        //  without accidentally leaving an unwanted Menubar shown.
#ifdef __WXMSW__
        if( g_bTempShowMenuBar ) {
            g_bTempShowMenuBar = false;
            if(gFrame)
                gFrame->ApplyGlobalSettings(false, false);
        }
#endif

        if( g_FloatingToolbarDialog )
            g_FloatingToolbarDialog->HideTooltip(); // Hide any existing tip
    }

    event.Skip();
}

void LoadS57()
{
#ifndef USE_S57
    return;
#else
    if(ps52plib) // already loaded?
        return;

//      Set up a useable CPL library error handler for S57 stuff
    CPLSetErrorHandler( MyCPLErrorHandler );

//      Init the s57 chart object, specifying the location of the required csv files
    g_csv_locn = g_Platform->GetSharedDataDir();
    g_csv_locn.Append( _T("s57data") );

    if( g_bportable ) {
        g_csv_locn = _T(".");
        appendOSDirSlash( &g_csv_locn );
        g_csv_locn.Append( _T("s57data") );
    }

//      If the config file contains an entry for SENC file prefix, use it.
//      Otherwise, default to PrivateDataDir
    if( g_SENCPrefix.IsEmpty() ) {
        g_SENCPrefix = g_Platform->GetPrivateDataDir();
        appendOSDirSlash(&g_SENCPrefix);
        g_SENCPrefix.Append( _T("SENC") );
    }

    if( g_bportable ) {
        wxFileName f( g_SENCPrefix );
        if( f.MakeRelativeTo( g_Platform->GetPrivateDataDir() ) )
            g_SENCPrefix = f.GetFullPath();
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

        wxStandardPaths& std_path = g_Platform->GetStdPaths();

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
///???            g_SData_Locn = tentative_SData_Locn;
        }
    }

    //  And if that doesn't work, look again in the original SData Location
    //  This will cover the case in which the .ini file entry is corrupted or moved

    if( !ps52plib->m_bOK ) {
        delete ps52plib;

        wxString look_data_dir;
        look_data_dir = g_Platform->GetSharedDataDir();
        look_data_dir.Append( _T("s57data") );

        plib_data = look_data_dir;
        appendOSDirSlash( &plib_data );
        plib_data.Append( _T("S52RAZDS.RLE") );

        wxLogMessage( _T("Looking for s57data in ") + look_data_dir );
        ps52plib = new s52plib( plib_data );

        if( ps52plib->m_bOK ) g_csv_locn = look_data_dir;
    }

    if( ps52plib->m_bOK ) {
        wxLogMessage( _T("Using s57data in ") + g_csv_locn );
        m_pRegistrarMan = new s57RegistrarMgr( g_csv_locn, g_Platform->GetLogFilePtr() );


            //    Preset some object class visibilites for "Mariner's Standard" disply category
            //  They may be overridden in LoadS57Config
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "DEPARE", 6 ) ) pOLE->nViz = 1;
            if( !strncmp( pOLE->OBJLName, "LNDARE", 6 ) ) pOLE->nViz = 1;
            if( !strncmp( pOLE->OBJLName, "COALNE", 6 ) ) pOLE->nViz = 1;
        }

        pConfig->LoadS57Config();
        ps52plib->SetPLIBColorScheme( global_color_scheme );
        
        if(cc1)
            ps52plib->SetPPMM( cc1->GetPixPerMM() );
    } else {
        wxLogMessage( _T("   S52PLIB Initialization failed, disabling Vector charts.") );
        delete ps52plib;
        ps52plib = NULL;
    }
#endif
}

#if defined(__WXGTK__) && defined(OCPN_HAVE_X11)
static char *get_X11_property (Display *disp, Window win,
                            Atom xa_prop_type, const char *prop_name) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    char *ret;

    xa_prop_name = XInternAtom(disp, prop_name, False);

    if (XGetWindowProperty(disp, win, xa_prop_name, 0, 1024, False,
                           xa_prop_type, &xa_ret_type, &ret_format,
                           &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size = (ret_format / 8) * ret_nitems;
    ret = (char*)malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    XFree(ret_prop);
    return ret;
}
#endif

bool MyApp::OnInit()
{
    wxStopWatch sw;

    if( !wxApp::OnInit() ) return false;

#if defined(__WXGTK__) && defined(ARMHF) && defined(ocpnUSE_GLES)
    // There is a race condition between cairo which is used for text rendering
    // by gtk and EGL which without the below code causes a bus error and the
    // program aborts before startup
    // this hack forces cairo to load right now by rendering some text

    wxBitmap bmp( 10, 10, -1 );
    wxMemoryDC dc;
    dc.SelectObject( bmp );
    dc.DrawText( _T("X"), 0, 0 );
#endif

    //  On Windows
    //  We allow only one instance unless the portable option is used
#ifdef __WXMSW__
    m_checker = new wxSingleInstanceChecker(_T("OpenCPN"));
    if(!g_bportable) {
        if ( m_checker->IsAnotherRunning() )
            return false;               // exit quietly
    }
#endif

    // Instantiate the global OCPNPlatform class
    g_Platform = new OCPNPlatform;

    //  Perform first stage initialization
    OCPNPlatform::Initialize_1( );

#if wxCHECK_VERSION(3,0,0)
    // Set the name of the app as displayed to the user.
    // This is necessary at least on OS X, for the capitalisation to be correct in the system menus.
    MyApp::SetAppDisplayName("OpenCPN");
#endif




    //  Seed the random number generator
    wxDateTime x = wxDateTime::UNow();
    long seed = x.GetMillisecond();
    seed *= x.GetTicks();
    srand(seed);


//Fulup: force floating point to use dot as separation.
// This needs to be set early to catch numerics in config file.
    setlocale( LC_NUMERIC, "C" );



    g_start_time = wxDateTime::Now();

    g_loglast_time = g_start_time;
    g_loglast_time.MakeGMT();
    g_loglast_time.Subtract( wxTimeSpan( 0, 29, 0, 0 ) ); // give 1 minute for GPS to get a fix

    AnchorPointMinDist = 5.0;


//      Init the private memory manager
    malloc_max = 0;

    //      Record initial memory status
    GetMemoryStatus( &g_mem_total, &g_mem_initial );


// Set up default FONT encoding, which should have been done by wxWidgets some time before this......
    wxFont temp_font( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxString( _T("") ),
            wxFONTENCODING_SYSTEM );
    temp_font.SetDefaultEncoding( wxFONTENCODING_SYSTEM );


    //      Establish Log File location
    if(!g_Platform->InitializeLogFile())
        return false;


#ifdef __WXMSW__

    //  Un-comment the following to establish a separate console window as a target for printf() in Windows
    //     RedirectIOToConsole();

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

    wxPlatformInfo platforminfo = wxPlatformInfo::Get();

    wxString os_name;
#ifndef __WXQT__
    os_name = platforminfo.GetOperatingSystemIdName();
#else
    os_name = platforminfo.GetOperatingSystemFamilyName();
#endif

    wxString platform = os_name + _T(" ") +
    platforminfo.GetArchName()+ _T(" ") +
    platforminfo.GetPortIdName();

    wxLogMessage( wxver + _T(" ") + platform );

    wxLogMessage( _T("MemoryStatus:  mem_total: %d mb,  mem_initial: %d mb"), g_mem_total / 1024,
            g_mem_initial / 1024 );

    //    Initialize embedded PNG icon graphics
    ::wxInitAllImageHandlers();


    imsg = _T("SData_Locn is ");
    imsg += g_Platform->GetSharedDataDir();
    wxLogMessage( imsg );

#ifdef __WXQT__
    //  Now we can load a Qt StyleSheet, if present
    wxString style_file = g_Platform->GetSharedDataDir();
    style_file += _T("styles");
    appendOSDirSlash( &style_file );
    style_file += _T("qtstylesheet.qss");
    if(LoadQtStyleSheet(style_file)){
        wxString smsg = _T("Loaded Qt Stylesheet: ") + style_file;
        wxLogMessage( smsg );
    }
    else{
        wxString smsg = _T("Qt Stylesheet not found: ") + style_file;
        wxLogMessage( smsg );
    }
#endif

    //      Create some static strings
    pInit_Chart_Dir = new wxString();

    //  Establish an empty ChartCroupArray
    g_pGroupArray = new ChartGroupArray;


    imsg = _T("PrivateDataDir is ");
    imsg += g_Platform->GetPrivateDataDir();
    wxLogMessage( imsg );


//      Create an array string to hold repeating messages, so they don't
//      overwhelm the log
    pMessageOnceArray = new wxArrayString;

//      Init the Route Manager
    g_pRouteMan = new Routeman( this );

    //      Init the Selectable Route Items List
    pSelect = new Select();
    pSelect->SetSelectPixelRadius( 12 );

    //      Init the Selectable Tide/Current Items List
    pSelectTC = new Select();
    //  Increase the select radius for tide/current stations
    pSelectTC->SetSelectPixelRadius( 25 );

    //      Init the Selectable AIS Target List
    pSelectAIS = new Select();
    pSelectAIS->SetSelectPixelRadius( 12 );

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


     bool b_initial_load = false;

    wxFileName config_test_file_name( g_Platform->GetConfigFileName() );
    if( config_test_file_name.FileExists() ) wxLogMessage(
        _T("Using existing Config_File: ") + g_Platform->GetConfigFileName() );
    else {
        {
            wxLogMessage( _T("Creating new Config_File: ") + g_Platform->GetConfigFileName() );

            b_initial_load = true;

            if( true != config_test_file_name.DirExists( config_test_file_name.GetPath() ) )
                if( !config_test_file_name.Mkdir(config_test_file_name.GetPath() ) )
                    wxLogMessage( _T("Cannot create config file directory for ") + g_Platform->GetConfigFileName() );
        }
    }

    //      Open/Create the Config Object
    pConfig = g_Platform->GetConfigObject();
    pConfig->LoadMyConfig();

    //  Override for some safe and nice default values if the config file was created from scratch
    if(b_initial_load)
        g_Platform->SetDefaultOptions();

    g_Platform->applyExpertMode(g_bUIexpert);

    // Now initialize UI Style.
    g_StyleManager = new ocpnStyle::StyleManager();
    g_StyleManager->SetStyle( g_uiStyle );
    if( !g_StyleManager->IsOK() ) {
        wxString msg = _("Failed to initialize the user interface. ");
        msg << _("OpenCPN cannot start. ");
        msg << _("The necessary configuration files were not found. ");
        msg << _("See the log file at ") << g_Platform->GetLogFileName() << _(" for details.") << _T("\n\n");
        msg << g_Platform->GetSharedDataDir();

        wxMessageDialog w( NULL, msg, _("Failed to initialize the user interface. "),
                           wxCANCEL | wxICON_ERROR );
        w.ShowModal();
        exit( EXIT_FAILURE );
    }

    //      Init the WayPoint Manager
    pWayPointMan = NULL;

    g_display_size_mm = wxMax(100, g_Platform->GetDisplaySizeMM());

    // User override....
    if((g_config_display_size_mm > 0) &&(g_config_display_size_manual)){
        g_display_size_mm = g_config_display_size_mm;
        wxString msg;
        msg.Printf(_T("Display size (horizontal) config override: %d mm"), (int) g_display_size_mm);
        wxLogMessage(msg);
    }

    if(g_btouch){
        int SelectPixelRadius = 50;

        pSelect->SetSelectPixelRadius(SelectPixelRadius);
        pSelectTC->SetSelectPixelRadius( wxMax(25, SelectPixelRadius) );
        pSelectAIS->SetSelectPixelRadius(SelectPixelRadius);
    }


    //        Is this the first run after a clean install?
    if( !n_NavMessageShown ) g_bFirstRun = true;

    //  Now we can set the locale

    //    Manage internationalization of embedded messages
    //    using wxWidgets/gettext methodology....

//    wxLog::SetVerbose(true);            // log all messages for debugging language stuff

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
    if( lang_list[0] ) {
    };                 // silly way to avoid compiler warnings

    // Add a new prefix for search order.
#ifdef __WXMSW__
    wxString locale_location = g_Platform->GetSharedDataDir();
    locale_location += _T("share/locale");
    wxLocale::AddCatalogLookupPathPrefix( locale_location );
#endif

    //  Get the default language info
    wxString def_lang_canonical;
#ifdef __WXMSW__
    LANGID lang_id = GetUserDefaultUILanguage();
    wxChar lngcp[100];
    const wxLanguageInfo* languageInfo = 0;
    if (0 != GetLocaleInfo(MAKELCID(lang_id, SORT_DEFAULT), LOCALE_SENGLANGUAGE, lngcp, 100)){
        languageInfo = wxLocale::FindLanguageInfo(lngcp);
    }
    else
        languageInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
#else
    const wxLanguageInfo* languageInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
#endif
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
        b_initok = plocale_def_lang->Init( pli->Language, 1 );
        // If the locale was not initialized OK, it may be that the wxstd.mo translations
        // of the wxWidgets strings is not present.
        // So try again, without attempting to load defaults wxstd.mo.
        if( !b_initok ){
            delete plocale_def_lang;
            plocale_def_lang = new wxLocale;
            b_initok = plocale_def_lang->Init( pli->Language, 0 );
        }
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

//    wxLog::SetVerbose( false );           // log no more verbose messages

    //  French language locale is assumed to include the AZERTY keyboard
    //  This applies to either the system language, or to OpenCPN language selection
    if( loc_lang_canonical == _T("fr_FR") ) g_b_assume_azerty = true;
    if( def_lang_canonical == _T("fr_FR") ) g_b_assume_azerty = true;
#else
    wxLogMessage( _T("wxLocale support not available") );
#endif

//  Send the Welcome/warning message if it has never been sent before,
//  or if the version string has changed at all
//  We defer until here to allow for localization of the message
    if( !n_NavMessageShown || ( vs != g_config_version_string ) ) {
        if( wxID_CANCEL == ShowNavWarning() ) return false;
        n_NavMessageShown = 1;
    }

    g_config_version_string = vs;

    //  log deferred log restart message, if it exists.
    if( !g_Platform->GetLargeLogMessage().IsEmpty() )
        wxLogMessage( g_Platform->GetLargeLogMessage() );

    //  Validate OpenGL functionality, if selected
#ifdef ocpnUSE_GL

#ifdef __WXMSW__
#if !wxCHECK_VERSION(2, 9, 0)           // The OpenGL test app only runs on wx 2.8, unavailable on wx3.x
        
    if( /*g_bopengl &&*/ !g_bdisable_opengl ) {
        wxFileName fn(g_Platform->GetExePath());
        bool b_test_result = TestGLCanvas(fn.GetPathWithSep() );

        if( !b_test_result )
            wxLogMessage( _T("OpenGL disabled due to test app failure.") );

        g_bdisable_opengl = !b_test_result;
    }
#endif
#endif

#else
    g_bdisable_opengl = true;;
#endif

    if(g_bdisable_opengl)
        g_bopengl = false;

#if defined(__UNIX__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)
    if(g_bSoftwareGL)
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
#endif

    // Determine if a transparent toolbar is possible under linux with opengl
    g_bTransparentToolbarInOpenGLOK = false;
#ifndef __WXQT__
#ifdef OCPN_HAVE_X11
    if(!g_bdisable_opengl) {
        Display *disp = XOpenDisplay(NULL);
        Window *sup_window;
        if ((sup_window = (Window *)get_X11_property(disp, DefaultRootWindow(disp),
                                                 XA_WINDOW, "_NET_SUPPORTING_WM_CHECK")) ||
            (sup_window = (Window *)get_X11_property(disp, DefaultRootWindow(disp),
                                                 XA_CARDINAL, "_WIN_SUPPORTING_WM_CHECK"))) {
            /* WM_NAME */
            char *wm_name;
            if ((wm_name = get_X11_property(disp, *sup_window,
                                        XInternAtom(disp, "UTF8_STRING", False), "_NET_WM_NAME")) ||
                (wm_name = get_X11_property(disp, *sup_window,
                                        XA_STRING, "_NET_WM_NAME"))) {
                // we know it works in xfce4, add other checks as we can validate them
                if(strstr(wm_name, "Xfwm4") || strstr(wm_name, "Compiz"))
                    g_bTransparentToolbarInOpenGLOK = true;

                free(wm_name);
            }
            free(sup_window);
        }
        XCloseDisplay(disp);
    }
#endif
#endif


    // On Windows platforms, establish a default cache managment policy
    // as allowing OpenCPN a percentage of available physical memory,
    // not to exceed 1 GB
    // Note that this logic implies that Windows platforms always use
    // the memCacheLimit policy, and never use the fallback nCacheLimit policy
#ifdef __WXMSW__
    if( 0 == g_memCacheLimit )
        g_memCacheLimit = (int) ( g_mem_total * 0.5 );
    g_memCacheLimit = wxMin(g_memCacheLimit, 1024 * 1024); // math in kBytes, Max is 1 GB
#else
    if( 0 ==  g_nCacheLimit && 0 == g_memCacheLimit ){
        g_memCacheLimit = (int) ( (g_mem_total - g_mem_initial) * 0.5 );
        g_memCacheLimit = wxMin(g_memCacheLimit, 1024 * 1024); // Max is 1 GB if unspecified
    }
#endif
    if( 0 ==  g_nCacheLimit)
        g_nCacheLimit = CACHE_N_LIMIT_DEFAULT;
#ifdef __OCPN__ANDROID__
    g_memCacheLimit = 100 * 1024;
#endif

//      Establish location and name of chart database
    ChartListFileName = newPrivateFileName(g_Platform->GetPrivateDataDir(), "chartlist.dat", "CHRTLIST.DAT");

//      Establish location and name of AIS MMSI -> Target Name mapping
    AISTargetNameFileName = newPrivateFileName(g_Platform->GetPrivateDataDir(), "mmsitoname.csv", "MMSINAME.CSV");

//      Establish guessed location of chart tree
    if( pInit_Chart_Dir->IsEmpty() ) {
        wxStandardPaths& std_path = g_Platform->GetStdPaths();

        if( !g_bportable )
#ifndef __OCPN__ANDROID__
        pInit_Chart_Dir->Append( std_path.GetDocumentsDir() );
#else
        pInit_Chart_Dir->Append( g_Platform->GetPrivateDataDir() );
#endif
    }

//      Establish the GSHHS Dataset location
    pWorldMapLocation = new wxString( _T("gshhs") );
    pWorldMapLocation->Prepend( g_Platform->GetSharedDataDir() );
    pWorldMapLocation->Append( wxFileName::GetPathSeparator() );

    //  Check the global Tide/Current data source array
    //  If empty, preset one default (US) Ascii data source
    wxString default_tcdata =  ( g_Platform->GetSharedDataDir() + _T("tcdata") +
             wxFileName::GetPathSeparator() + _T("HARMONIC.IDX"));
    wxFileName fdefault( default_tcdata );

    if(!TideCurrentDataSet.GetCount()) {
        if( g_bportable ) {
            fdefault.MakeRelativeTo( g_Platform->GetPrivateDataDir() );
            TideCurrentDataSet.Add( fdefault.GetFullPath() );
        }
        else
            TideCurrentDataSet.Add( default_tcdata );
    }
    else {
        wxString first_tide = TideCurrentDataSet.Item(0);
        wxFileName ft(first_tide);
        if(!ft.FileExists()){
            TideCurrentDataSet.RemoveAt(0);
            TideCurrentDataSet.Insert( default_tcdata, 0 );
        }
    }


    //  Check the global AIS alarm sound file
    //  If empty, preset default
    if(g_sAIS_Alert_Sound_File.IsEmpty()) {
        wxString default_sound =  ( g_Platform->GetSharedDataDir() + _T("sounds") +
        wxFileName::GetPathSeparator() +
        _T("2bells.wav"));

        if( g_bportable ) {
            wxFileName f( default_sound );
            f.MakeRelativeTo( g_Platform->GetPrivateDataDir() );
            g_sAIS_Alert_Sound_File = f.GetFullPath();
        }
        else
            g_sAIS_Alert_Sound_File = default_sound ;
    }


    g_StartTime = wxInvalidDateTime;
    g_StartTimeTZ = 1;                // start with local times
    gpIDX = NULL;
    gpIDXn = 0;

    g_Platform->Initialize_2();

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

#ifdef __OCPN__ANDROID__
    wxSize asz = getAndroidDisplayDimensions();
    ch = asz.y;
    cw = asz.x;
//    qDebug() << cw << ch;

    if((cw > 200) && (ch > 200) )
        new_frame_size.Set( cw, ch );
    else
        new_frame_size.Set( 800, 400 );
#endif

    //  For Windows and GTK, provide the expected application Minimize/Close bar
    long app_style = wxDEFAULT_FRAME_STYLE;
    app_style |= wxWANTS_CHARS;

// Create the main frame window
    wxString myframe_window_title = wxString::Format(wxT("OpenCPN %i.%i.%i"),
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); //Gunther

    if( g_bportable ) {
        myframe_window_title += _(" -- [Portable(-p) executing from ");
        myframe_window_title += g_Platform->GetHomeDir();
        myframe_window_title += _T("]");
    }

    wxString fmsg;
    fmsg.Printf(_T("Creating MyFrame...size(%d, %d)  position(%d, %d)"), new_frame_size.x, new_frame_size.y, position.x, position.y);
    wxLogMessage(fmsg);

    gFrame = new MyFrame( NULL, myframe_window_title, position, new_frame_size, app_style ); //Gunther

//  Initialize the Plugin Manager
    g_pi_manager = new PlugInManager( gFrame );

    g_pauimgr = new wxAuiManager;
    g_pauidockart= new wxAuiDefaultDockArt;
    g_pauimgr->SetArtProvider(g_pauidockart);
        
    g_grad_default = g_pauidockart->GetMetric(wxAUI_DOCKART_GRADIENT_TYPE);
    g_border_color_default = g_pauidockart->GetColour(wxAUI_DOCKART_BORDER_COLOUR );
    g_border_size_default = g_pauidockart->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE );
    g_sash_size_default = g_pauidockart->GetMetric(wxAUI_DOCKART_SASH_SIZE);
    g_caption_color_default = g_pauidockart->GetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR);
    g_sash_color_default = g_pauidockart->GetColour(wxAUI_DOCKART_SASH_COLOUR );
    
        
         
// tell wxAuiManager to manage the frame
    g_pauimgr->SetManagedWindow( gFrame );

//      Create Children of Frame
//              n.b.  if only one child exists, wxWindows expands the child
//                        to the parent client area automatically, (as a favor?)
//                        Here, we'll do explicit sizing on SIZE events


    cc1 = new ChartCanvas( gFrame );                         // the chart display canvas
    gFrame->SetCanvasWindow( cc1 );

    cc1->SetDisplaySizeMM(g_display_size_mm);

    cc1->SetQuiltMode( g_bQuiltEnable );                     // set initial quilt mode
    cc1->m_bFollow = pConfig->st_bFollow;               // set initial state
    cc1->SetViewPoint( vLat, vLon, initial_scale_ppm, 0., 0. );

    gFrame->Enable();

    cc1->SetFocus();

    pthumbwin = new ThumbWin( cc1 );

    g_Piano = new Piano();
    g_Piano->SetColorScheme( global_color_scheme );

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    g_Piano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
    g_Piano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );

    if( cc1->GetQuiltMode() ) {
        g_Piano->SetRoundedRectangles( true );
    }
    g_Piano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
    g_Piano->SetPolyIcon( new wxBitmap( style->GetIcon( _T("polyprj") ) ) );
    g_Piano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

    gFrame->ApplyGlobalSettings( false, false );               // done once on init with resize

    g_toolbar_x = wxMax(g_toolbar_x, 0);
    g_toolbar_y = wxMax(g_toolbar_y, 0);

    g_toolbar_x = wxMin(g_toolbar_x, cw);
    g_toolbar_y = wxMin(g_toolbar_y, ch);

    gFrame->SetToolbarScale();
    gFrame->SetGPSCompassScale();

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

// Show the frame

    gFrame->Show( TRUE );

#ifdef __OCPN__ANDROID__
    androidShowBusyIcon();
#endif

    gFrame->SetAndApplyColorScheme( global_color_scheme );

    if( g_bframemax ) gFrame->Maximize( true );

    if( g_bresponsive  && ( cc1->GetPixPerMM() > 4.0))
        gFrame->Maximize( true );

    // enable this to use a window for the chart bar instead of rendering it
    // to the chart canvas.  If it can be determined that rendering works well
    // in all cases for all platforms we can remove the ChartBarWin class completely
#if 0
    g_ChartBarWin = new ChartBarWin( cc1 );
    g_ChartBarWin->Show();
#endif

    //  Yield to pick up the OnSize() calls that result from Maximize()
    Yield();

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

		if (g_bportable)
		{
			ChartDirInfo cdi;
			cdi.fullpath =_T("charts");
			cdi.fullpath.Prepend(g_Platform->GetSharedDataDir());
			cdi.magic_number = _T("");
			ChartDirArray.Add(cdi);
			ndirs++;
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

    if( !ChartDirArray.GetCount() )
        if(::wxFileExists( ChartListFileName ))
            ::wxRemoveFile( ChartListFileName );

//      Try to load the current chart list Data file
    ChartData = new ChartDB( );
    if (!ChartData->LoadBinary(ChartListFileName, ChartDirArray)) {
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
            ChartData = new ChartDB( );

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
            ChartData->SaveBinary(ChartListFileName);

            delete pprog;
        }

        else            // No chart database, no config hints, so bail to Options....
        {
            wxLogMessage(
                    _T("Chartlist file not found, config chart dir array is empty.  Chartlist target file is:")
                            + ChartListFileName );

            wxString msg1(
                    _("No Charts Installed.\nPlease select chart folders in Options > Charts.") );

 ///           OCPNMessageBox(gFrame, msg1, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK );


///            gFrame->DoOptionsDialog();

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

    //  Verify any saved chart database startup index
    if(g_restore_dbindex >= 0){
        if(ChartData->GetChartTableEntries() == 0)
            g_restore_dbindex = -1;
        
        else if(g_restore_dbindex > (ChartData->GetChartTableEntries()-1))
            g_restore_dbindex = 0;
    }

    //  Apply the inital Group Array structure to the chart data base
    ChartData->ApplyGroupArray( g_pGroupArray );

    //  Make sure that the Selected Group is sensible...
    if( g_GroupIndex > (int) g_pGroupArray->GetCount() ) g_GroupIndex = 0;
    if( !gFrame->CheckGroup( g_GroupIndex ) ) g_GroupIndex = 0;

    pCurrentStack = new ChartStack;

//      All set to go.....

    // Process command line option to rebuild cache
#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;

    if(g_rebuild_gl_cache && g_bopengl &&
        g_GLOptions.m_bTextureCompression && g_GLOptions.m_bTextureCompressionCaching ) {

        cc1->ReloadVP();                  //  Get a nice chart background loaded

        //      Turn off the toolbar as a clear signal that the system is busy right now.
        // Note: I commented this out because the toolbar never comes back for me
        // and is unusable until I restart opencpn without generating the cache
//        if( g_FloatingToolbarDialog )
//            g_FloatingToolbarDialog->Hide();

        BuildCompressedCache();

    }
#endif


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
    g_bHDT_Rx = false;
    g_bVAR_Rx = false;

//  Start up a new track if enabled in config file
    if( g_bTrackCarryOver )
        g_bDeferredStartTrack = true;

    pAnchorWatchPoint1 = NULL;
    pAnchorWatchPoint2 = NULL;

    Yield();

    gFrame->DoChartUpdate();

    FontMgr::Get().ScrubList(); // Clean the font list, removing nonsensical entries

//      Start up the ticker....
    gFrame->FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

//      Start up the ViewPort Rotation angle Averaging Timer....
    if(g_bCourseUp)
        gFrame->FrameCOGTimer.Start( 10, wxTIMER_CONTINUOUS );

    cc1->ReloadVP();                  // once more, and good to go

    g_Compass = new ocpnCompass;
    g_Compass->SetScaleFactor(g_compass_scalefactor);
    g_Compass->Show(pConfig->m_bShowCompassWin);

    gFrame->Refresh( false );
    gFrame->Raise();

    gFrame->RequestNewToolbar();


    cc1->Enable();
    cc1->SetFocus();

    //  This little hack fixes a problem seen with some UniChrome OpenGL drivers
    //  We need a deferred resize to get glDrawPixels() to work right.
    //  So we set a trigger to generate a resize after 5 seconds....
    //  See the "UniChrome" hack elsewhere
#ifdef ocpnUSE_GL
    if ( !g_bdisable_opengl )
    {
        glChartCanvas *pgl = (glChartCanvas *) cc1->GetglCanvas();
        if( pgl && ( pgl->GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND ) )
        {
            gFrame->m_defer_size = gFrame->GetSize();
            gFrame->SetSize( gFrame->m_defer_size.x - 10, gFrame->m_defer_size.y );
            g_pauimgr->Update();
            gFrame->m_bdefer_resize = true;
        }
    }
#endif

    if ( g_start_fullscreen )
        gFrame->ToggleFullScreen();

    if(g_ChartBarWin)
        g_ChartBarWin->Show( g_bShowChartBar );

#ifdef __OCPN__ANDROID__
    //  We need a resize to pick up height adjustment after building android ActionBar
    if(pConfig->m_bShowMenuBar)
        gFrame->SetSize(getAndroidDisplayDimensions());
    androidSetFollowTool(cc1->m_bFollow);
#endif

    gFrame->Raise();
    cc1->Enable();
    cc1->SetFocus();

#ifdef __WXQT__
    if(g_FloatingToolbarDialog)
        g_FloatingToolbarDialog->Raise();
#endif

    // Start delayed initialization chain after 100 milliseconds
    gFrame->InitTimer.Start( 100, wxTIMER_CONTINUOUS );

    wxLogMessage( wxString::Format(_("OpenCPN Initialized in %ld ms."), sw.Time() ) );

#ifdef __OCPN__ANDROID__
    androidHideBusyIcon();
#endif

    return TRUE;
}

int MyApp::OnExit()
{
    wxLogMessage( _T("opencpn::MyApp starting exit.") );

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
            sog.Printf( _T("SOG %6.2f ") + getUsrSpeedUnit(), toUsrSpeed( gSog ) );

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

    delete pConfig;
    delete pSelect;
    delete pSelectTC;
    delete pSelectAIS;

#ifdef USE_S57
    delete ps52plib;
#endif

    if(g_pGroupArray){
        for(unsigned int igroup = 0; igroup < g_pGroupArray->GetCount(); igroup++){
            delete g_pGroupArray->Item(igroup);
        }

        g_pGroupArray->Clear();
        delete g_pGroupArray;
    }

    delete pDummyChart;

    wxLogMessage( _T("opencpn::MyApp exiting cleanly...\n") );
    wxLog::FlushActive();

    g_Platform->CloseLogFile();

    delete phost_name;
    delete pInit_Chart_Dir;
    delete pWorldMapLocation;

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

#ifdef OCPN_USE_PORTAUDIO
    if(portaudio_initialized)
        Pa_Terminate();
#endif


    //      Restore any changed system colors
#ifdef __WXMSW__
    RestoreSystemColors();
#endif

#ifdef __MSVC__LEAK
    DeInitAllocCheck();
#endif

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
    delete plocale_def_lang;
#endif

    FontMgr::Shutdown();

#ifdef __WXMSW__
    delete m_checker;
#endif


    g_Platform->OnExit_2();

    return TRUE;
}

#ifdef LINUX_CRASHRPT
void MyApp::OnFatalException () {
    g_crashprint.Report();
}
#endif

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




#include <wx/power.h>

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

//      Frame implementation
BEGIN_EVENT_TABLE(MyFrame, wxFrame) EVT_CLOSE(MyFrame::OnCloseWindow)
EVT_MENU(wxID_EXIT, MyFrame::OnExit)
EVT_SIZE(MyFrame::OnSize)
EVT_MOVE(MyFrame::OnMove)
EVT_MENU(-1, MyFrame::OnToolLeftClick)
EVT_TIMER(INIT_TIMER, MyFrame::OnInitTimer)
EVT_TIMER(FRAME_TIMER_1, MyFrame::OnFrameTimer1)
EVT_TIMER(FRAME_TC_TIMER, MyFrame::OnFrameTCTimer)
EVT_TIMER(FRAME_COG_TIMER, MyFrame::OnFrameCOGTimer)
EVT_TIMER(MEMORY_FOOTPRINT_TIMER, MyFrame::OnMemFootTimer)
EVT_TIMER(BELLS_TIMER, MyFrame::OnBellsTimer)
EVT_ACTIVATE(MyFrame::OnActivate)
EVT_MAXIMIZE(MyFrame::OnMaximize)
EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_TOOL_RCLICKED, MyFrame::RequestNewToolbarArgEvent)
EVT_ERASE_BACKGROUND(MyFrame::OnEraseBackground)
EVT_TIMER(RESIZE_TIMER, MyFrame::OnResizeTimer)
#ifdef wxHAS_POWER_EVENTS
EVT_POWER_SUSPENDING(MyFrame::OnSuspending)
EVT_POWER_SUSPENDED(MyFrame::OnSuspended)
EVT_POWER_SUSPEND_CANCEL(MyFrame::OnSuspendCancel)
EVT_POWER_RESUME(MyFrame::OnResume)
#endif // wxHAS_POWER_EVENTS
END_EVENT_TABLE()

// My frame constructor
MyFrame::MyFrame( wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size,
        long style ) :
        wxFrame( frame, -1, title, pos, size, style ) //wxSIMPLE_BORDER | wxCLIP_CHILDREN | wxRESIZE_BORDER)
//wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER
{
    m_last_track_rotation_ts = 0;
    m_ulLastNEMATicktime = 0;

    m_pStatusBar = NULL;
    m_StatusBarFieldCount = g_Platform->GetStatusBarFieldCount();

    m_pMenuBar = NULL;
    g_toolbar = NULL;
    g_options = NULL;
    m_toolbar_scale_tools_shown = false;
    piano_ctx_menu = NULL;

    //      Redirect the initialization timer to this frame
    InitTimer.SetOwner( this, INIT_TIMER );
    m_iInitCount = 0;
    m_initializing = false;

    //      Redirect the global heartbeat timer to this frame
    FrameTimer1.SetOwner( this, FRAME_TIMER_1 );

    //      Redirect the Tide/Current update timer to this frame
    FrameTCTimer.SetOwner( this, FRAME_TC_TIMER );

    //      Redirect the COG Averager timer to this frame
    FrameCOGTimer.SetOwner( this, FRAME_COG_TIMER );

    //      Redirect the Memory Footprint Management timer to this frame
    MemFootTimer.SetOwner( this, MEMORY_FOOTPRINT_TIMER );

    //      Redirect the Bells timer to this frame
    BellsTimer.SetOwner( this, BELLS_TIMER );

#ifdef __OCPN__ANDROID__
//    m_PrefTimer.SetOwner( this, ANDROID_PREF_TIMER );
//    Connect( m_PrefTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler( MyFrame::OnPreferencesResultTimer ), NULL, this );
#endif

    //      Set up some assorted member variables
    nRoute_State = 0;
    m_bTimeIsSet = false;
    nBlinkerTick = 0;

    m_bdefer_resize = false;

    //    Clear the NMEA Filter tables
    for( int i = 0; i < MAX_COGSOG_FILTER_SECONDS; i++ ) {
        COGFilterTable[i] = 0.;
        SOGFilterTable[i] = 0.;
    }
    m_COGFilterLast = 0.;
    m_last_bGPSValid = false;

    gHdt = NAN;
    gHdm = NAN;
    gVar = NAN;
    gSog = NAN;
    gCog = NAN;

    for (int i = 0; i < MAX_COG_AVERAGE_SECONDS; i++ ) {
        COGTable[i] = 0.;
    }

    m_fixtime = 0;

    m_bpersistent_quilt = false;

    m_ChartUpdatePeriod = 1;                  // set the default (1 sec.) period

//    Establish my children
    g_pMUX = new Multiplexer();

    g_pAIS = new AIS_Decoder( this );

    g_pMUX->SetAISHandler(g_pAIS);
    g_pMUX->SetGPSHandler(this);
    //  Create/connect a dynamic event handler slot
    Connect( wxEVT_OCPN_DATASTREAM, (wxObjectEventFunction) (wxEventFunction) &MyFrame::OnEvtOCPN_NMEA );

    bFirstAuto = true;
    b_autofind = false;
    
    //  Create/connect a dynamic event handler slot for OCPN_MsgEvent(s) coming from PlugIn system
    Connect( wxEVT_OCPN_MSG, (wxObjectEventFunction) (wxEventFunction) &MyFrame::OnEvtPlugInMessage );

    Connect( wxEVT_OCPN_THREADMSG, (wxObjectEventFunction) (wxEventFunction) &MyFrame::OnEvtTHREADMSG );


    //        Establish the system icons for the frame.

#ifdef __WXMSW__
    SetIcon( wxICON(0) );           // this grabs the first icon in the integrated MSW resource file
#endif

#if defined(__WXGTK__) || defined(__WXQT__)
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
    g_sticky_projection = -1;
    m_BellsToPlay = 0;

    m_resizeTimer.SetOwner(this, RESIZE_TIMER);
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
}

void MyFrame::OnEraseBackground( wxEraseEvent& event )
{
}

void MyFrame::OnMaximize( wxMaximizeEvent& event )
{
    g_click_stop = 0;
#ifdef __WXOSX__
    event.Skip();
#endif
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

        if(g_ChartBarWin)
            g_ChartBarWin->Show(g_bShowChartBar);
    }
#endif

    event.Skip();
}

ColorScheme GetColorScheme()
{
    return global_color_scheme;
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

    
    if( cs == GLOBAL_COLOR_SCHEME_DUSK || cs == GLOBAL_COLOR_SCHEME_NIGHT ) {
        g_pauidockart->SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, wxAUI_GRADIENT_NONE);
        
        g_pauidockart->SetColour(wxAUI_DOCKART_BORDER_COLOUR, wxColour(0,0,0));
        g_pauidockart->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, 1);
        g_pauidockart->SetColour(wxAUI_DOCKART_SASH_COLOUR, wxColour(0,0,0));
        g_pauidockart->SetMetric(wxAUI_DOCKART_SASH_SIZE, 0);
        g_pauidockart->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, wxColour(0,0,0));
        
    }
    else{
        g_pauidockart->SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, g_grad_default);
        g_pauidockart->SetColour(wxAUI_DOCKART_BORDER_COLOUR, g_border_color_default);
        g_pauidockart->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, g_border_size_default);
        g_pauidockart->SetColour(wxAUI_DOCKART_SASH_COLOUR, g_sash_color_default);
        g_pauidockart->SetMetric(wxAUI_DOCKART_SASH_SIZE, g_sash_size_default);
        g_pauidockart->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, g_caption_color_default);
    
    }
    
    g_pauimgr->Update();
    
    g_StyleManager->GetCurrentStyle()->SetColorScheme( cs );
    cc1->GetWorldBackgroundChart()->SetColorScheme( cs );

    //Search the user color table array to find the proper hash table
    Usercolortable_index = 0;
    for( unsigned int i = 0; i < UserColorTableArray->GetCount(); i++ ) {
        colTable *ct = (colTable *) UserColorTableArray->Item( i );
        if( SchemeName.IsSameAs( *ct->tableName ) ) {
            Usercolortable_index = i;
            break;
        }
    }

#ifdef USE_S57
    if( ps52plib ) ps52plib->SetPLIBColorScheme( SchemeName );
#endif

    //    Set up a pointer to the proper hash table
    pcurrent_user_color_hash = (wxColorHashMap *) UserColourHashTableArray->Item(
            Usercolortable_index );

    SetSystemColors( cs );

    cc1->SetColorScheme( cs );

    if( pWayPointMan ) pWayPointMan->SetColorScheme( cs );

    if( ChartData ) ChartData->ApplyColorSchemeToCachedCharts( cs );

    SetChartThumbnail( -1 );
    cc1->HideChartInfoWindow();
    cc1->SetQuiltChartHiLiteIndex( -1 );

    g_Piano->ResetRollover();
    g_Piano->SetColorScheme( cs );

    if( g_options ) g_options->SetColorScheme( cs );

    if( console ) console->SetColorScheme( cs );

    if( g_pRouteMan ) g_pRouteMan->SetColorScheme( cs );

    if( pMarkPropDialog ) pMarkPropDialog->SetColorScheme( cs );

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
        m_pStatusBar->SetBackgroundColour(GetGlobalColor(_T("UIBDR")));    //UINFF
        m_pStatusBar->ClearBackground();

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

    if( g_FloatingToolbarDialog ){
        tb = g_FloatingToolbarDialog->GetToolbar();
        if(tb){
            if(g_Compass)
                g_FloatingToolbarDialog->SetGeometry(g_Compass->IsShown(), g_Compass->GetRect());
            else
                g_FloatingToolbarDialog->SetGeometry(false, wxRect(0,0,1,1));
        }
            
    }
    if( !tb )
        return 0;

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
    bool gs = false;
#ifdef USE_S57
    if (ps52plib)
        gs = ps52plib->GetShowS57Text();
#endif

    if (gs)
        tipString = wxString( _("Hide ENC Text") ) << _T(" (T)");
    else
        tipString = wxString( _("Show ENC Text") ) << _T(" (T)");

    if( _toolbarConfigMenuUtil( ID_ENC_TEXT, tipString ) )
        tb->AddTool( ID_ENC_TEXT, _T("text"),
            style->GetToolIcon( _T("text"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("text"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    m_pAISTool = NULL;
    CheckAndAddPlugInTool( tb );
    tipString = _("Hide AIS Targets");          // inital state is on
    if( _toolbarConfigMenuUtil( ID_AIS, tipString ) )
        m_pAISTool = tb->AddTool( ID_AIS, _T("AIS"), style->GetToolIcon( _T("AIS"), TOOLICON_NORMAL ),
                                  style->GetToolIcon( _T("AIS"), TOOLICON_DISABLED ),
                                  wxITEM_NORMAL, tipString );

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
    tipString = _("Route & Mark Manager");
    if( _toolbarConfigMenuUtil( ID_ROUTEMANAGER, tipString ) )
        tb->AddTool( ID_ROUTEMANAGER,
            _T("route_manager"), style->GetToolIcon( _T("route_manager"), TOOLICON_NORMAL ),
            tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("Enable Tracking");
    if( _toolbarConfigMenuUtil( ID_TRACK, tipString ) )
        tb->AddTool( ID_TRACK, _T("track"),
            style->GetToolIcon( _T("track"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("track"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Change Color Scheme") ) << _T(" (F5)");
    if( _toolbarConfigMenuUtil( ID_COLSCHEME, tipString ) ){
        tb->AddTool( ID_COLSCHEME,
            _T("colorscheme"), style->GetToolIcon( _T("colorscheme"), TOOLICON_NORMAL ),
            tipString, wxITEM_NORMAL );
        tb->SetToolTooltipHiViz( ID_COLSCHEME, true );  // cause the Tooltip to always be visible, whatever
                                                        //  the colorscheme
    }

    CheckAndAddPlugInTool( tb );
    tipString = _("About OpenCPN");
    if( _toolbarConfigMenuUtil( ID_ABOUT, tipString ) )
        tb->AddTool( ID_ABOUT, _T("help"),
            style->GetToolIcon( _T("help"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    //      Add any PlugIn toolbar tools that request default positioning
    AddDefaultPositionPlugInTools( tb );

    //  And finally add the MOB tool
    tipString = wxString( _("Drop MOB Marker") ) << _(" (Ctrl-Space)");
    if( _toolbarConfigMenuUtil( ID_MOB, tipString ) )
        tb->AddTool( ID_MOB, _T("mob_btn"),
                     style->GetToolIcon( _T("mob_btn"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );


// Realize() the toolbar
    style->Unload();
    g_FloatingToolbarDialog->Realize();

//      Set up the toggle states

    if( cc1 ) {
        //  Re-establish toggle states
        tb->ToggleTool( ID_CURRENT, cc1->GetbShowCurrent() );
        tb->ToggleTool( ID_TIDE, cc1->GetbShowTide() );
    }

    if( pConfig && cc1 )
        tb->ToggleTool( ID_FOLLOW, cc1->m_bFollow );

#ifdef USE_S57
    if( ( pConfig ) && ( ps52plib ) ) if( ps52plib->m_bOK ) tb->ToggleTool( ID_ENC_TEXT,
            ps52plib->GetShowS57Text() );
#endif

    wxString initiconName;
    if( g_bShowAIS ) {
        if (g_bAllowShowScaled){
            if(!g_bShowScaled)
                tb->SetToolShortHelp( ID_AIS, _("Attenuate less critical AIS Targets") );
            else
                tb->SetToolShortHelp( ID_AIS, _("Hide AIS Targets") );
        }
        else
            tb->SetToolShortHelp( ID_AIS, _("Hide AIS Targets") );
        initiconName = _T("AIS");
    }
    else {
        tb->SetToolShortHelp( ID_AIS, _("Show AIS Targets") );
        initiconName = _T("AIS_Disabled");
    }
    tb->SetToolNormalBitmapEx( m_pAISTool, initiconName );
    m_lastAISiconName = initiconName;

    tb->ToggleTool( ID_TRACK, g_bTrackActive );

    //  Set PlugIn tool toggle states
    ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();
    for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
        PlugInToolbarToolContainer *pttc = tool_array.Item( i );
        if( !pttc->b_viz )
            continue;

        if( pttc->kind == wxITEM_CHECK )
            tb->ToggleTool( pttc->id, pttc->b_toggle );
    }


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

            wxToolBarToolBase * tool = tb->AddTool( pttc->id, wxString( pttc->label ), *( ptool_bmp ),
                    wxString( pttc->shortHelp ), pttc->kind );
            
            tb->SetToolBitmapsSVG( pttc->id, pttc->pluginNormalIconSVG,
                                   pttc->pluginRolloverIconSVG,
                                   pttc->pluginToggledIconSVG );
            
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

        //      Tool is currently tagged as invisible
        if( !pttc->b_viz )
            continue;

        if( pttc->position == -1 )                  // PlugIn has requested default positioning
                {
            wxBitmap *ptool_bmp;
            wxBitmap *ptool_bmp_Rollover;

            switch( global_color_scheme ){
                case GLOBAL_COLOR_SCHEME_DAY:
                    ptool_bmp = pttc->bitmap_day;
                    ptool_bmp_Rollover = pttc->bitmap_Rollover_day;
                    ;
                    break;
                case GLOBAL_COLOR_SCHEME_DUSK:
                    ptool_bmp = pttc->bitmap_dusk;
                    ptool_bmp_Rollover = pttc->bitmap_Rollover_dusk;
                    break;
                case GLOBAL_COLOR_SCHEME_NIGHT:
                    ptool_bmp = pttc->bitmap_night;
                    ptool_bmp_Rollover = pttc->bitmap_Rollover_night;
                    break;
                default:
                    ptool_bmp = pttc->bitmap_day;
                    ptool_bmp_Rollover = pttc->bitmap_Rollover_day;
                    break;
            }

            wxToolBarToolBase * tool = tb->AddTool( pttc->id, wxString( pttc->label ), *( ptool_bmp ),
                                                    wxString( pttc->shortHelp ), pttc->kind );
            
            tb->SetToolBitmapsSVG( pttc->id, pttc->pluginNormalIconSVG,
                                   pttc->pluginRolloverIconSVG,
                                   pttc->pluginToggledIconSVG );
            
            bret = true;
        }
    }
    return bret;
}

static bool b_inCloseWindow;

void MyFrame::RequestNewToolbar(bool bforcenew)
{
    if( b_inCloseWindow ) {
        return;
    }
    
    bool b_reshow = true;
    if( g_FloatingToolbarDialog ) {
        b_reshow = g_FloatingToolbarDialog->IsShown();
        float ff = fabs(g_FloatingToolbarDialog->GetScaleFactor() - g_toolbar_scalefactor);
        if((ff > 0.01f) || bforcenew){
            DestroyMyToolbar();
            delete g_FloatingToolbarDialog;
            g_FloatingToolbarDialog = NULL;
        }
    }

    if( !g_FloatingToolbarDialog ) {
        g_FloatingToolbarDialog = new ocpnFloatingToolbarDialog( cc1,
             wxPoint( g_toolbar_x, g_toolbar_y ), g_toolbar_orient, g_toolbar_scalefactor );
    }

    if( g_FloatingToolbarDialog ) {
        if( g_FloatingToolbarDialog->IsToolbarShown() )
            DestroyMyToolbar();

        g_toolbar = CreateAToolbar();
        if (g_FloatingToolbarDialog->isSubmergedToGrabber()) {
            g_FloatingToolbarDialog->SubmergeToGrabber();
        } else {
            g_FloatingToolbarDialog->RePosition();
            g_FloatingToolbarDialog->SetColorScheme(global_color_scheme);
            g_FloatingToolbarDialog->Show(b_reshow && g_bshowToolbar);
        }

#ifndef __WXQT__
        gFrame->Raise(); // ensure keyboard focus to the chart window (needed by gtk+)
#endif
    }

#ifdef __OCPN__ANDROID__
    DoChartUpdate();
#endif

}

//      Update inplace the current toolbar with bitmaps corresponding to the current color scheme
void MyFrame::UpdateToolbar( ColorScheme cs )
{
    if( g_FloatingToolbarDialog ) {
        if(g_FloatingToolbarDialog->GetColorScheme() != cs){
            g_FloatingToolbarDialog->SetColorScheme( cs );

            if( g_FloatingToolbarDialog->IsToolbarShown() ) {
                DestroyMyToolbar();
                g_toolbar = CreateAToolbar();
                if (g_FloatingToolbarDialog->isSubmergedToGrabber()) 
                    g_FloatingToolbarDialog->SubmergeToGrabber(); //Surface(); //SubmergeToGrabber();
            
            }
        }
    }

    if(g_Compass)
        g_Compass->SetColorScheme( cs );

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
        g_toolbar->EnableTool( ID_ENC_TEXT, newstate );
        g_toolbar->EnableTool( ID_CURRENT, newstate );
        g_toolbar->EnableTool( ID_TIDE, newstate );
        g_toolbar->EnableTool( ID_ABOUT, newstate );
        g_toolbar->EnableTool( ID_TBEXIT, newstate );
        g_toolbar->EnableTool( ID_TBSTAT, newstate );
        g_toolbar->EnableTool( ID_PRINT, newstate );
        g_toolbar->EnableTool( ID_COLSCHEME, newstate );
        g_toolbar->EnableTool( ID_ROUTEMANAGER, newstate );
        g_toolbar->EnableTool( ID_TRACK, newstate );
        g_toolbar->EnableTool( ID_AIS, newstate );
    }
}

void MyFrame::SetToolbarScale()
{
    g_toolbar_scalefactor = g_Platform->GetToolbarScaleFactor( g_GUIScaleFactor );

    //  Round to the nearest "quarter", to avoid rendering artifacts
    g_toolbar_scalefactor = wxRound( g_toolbar_scalefactor * 4.0 )/ 4.0;

}

void MyFrame::SetGPSCompassScale()
{
    g_compass_scalefactor = g_Platform->GetCompassScaleFactor( g_GUIScaleFactor );

    //  Round to the nearest "quarter", to avoid rendering artifacts
//    g_compass_scalefactor = wxRound( g_toolbar_scalefactor * 4.0 )/ 4.0;

}

void MyFrame::RaiseToolbarRecoveryWindow()
{
    if(g_bshowToolbar){
        if(g_FloatingToolbarDialog && g_FloatingToolbarDialog->m_pRecoverwin ){
            g_FloatingToolbarDialog->m_pRecoverwin->Raise();
            g_FloatingToolbarDialog->m_pRecoverwin->Refresh( false );
        }
    }
    else{
        if(g_FloatingToolbarDialog && g_FloatingToolbarDialog->m_pRecoverwin ){
            g_FloatingToolbarDialog->m_pRecoverwin->Hide();
        }
    }

}


// Intercept menu commands
void MyFrame::OnExit( wxCommandEvent& event )
{
    quitflag++;                             // signal to the timer loop

}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    //    It is possible that double clicks on application exit box could cause re-entrance here
    //    Not good, and don't need it anyway, so simply return.
    if( b_inCloseWindow ) {
//            wxLogMessage(_T("opencpn::MyFrame re-entering OnCloseWindow"));
        return;
    }

    // The Options dialog, and other deferred init items, are not fully initialized.
    // Best to just cancel the close request. 
    // This is probably only reachable on slow hardware...
    if(!g_bDeferredInitDone)
        return;
    
    
    if(g_options){
        delete g_options;
        g_options = NULL;
        g_pOptions = NULL;
    }
    
    //  If the multithread chart compressor engine is running, cancel the close command
    if( b_inCompressAllCharts ) {
        return;
    }

    if( bDBUpdateInProgress )
        return;

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

#ifdef ocpnUSE_GL
    // cancel compression jobs
    if(g_bopengl && g_CompressorPool){
        g_CompressorPool->PurgeJobList();

        if(g_CompressorPool->GetRunningJobCount())
            g_bcompression_wait = true;
    }
#endif

//  cc1->SetCursor( wxCURSOR_WAIT );

    cc1->Refresh( true );
    cc1->Update();
    wxYield();

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
        watching_anchor = ( pAnchorWatchPoint1->GetIconName().StartsWith( _T("anchor") ) ); // pjotrc 2010.02.15
        if( pAnchorWatchPoint2 )                                               // pjotrc 2010.02.15
        watching_anchor |= ( pAnchorWatchPoint2->GetIconName().StartsWith( _T("anchor") ) ); // pjotrc 2010.02.15

        wxDateTime now = wxDateTime::Now();
        wxTimeSpan uptime = now.Subtract( g_start_time );

        if( !watching_anchor && ( g_bCruising ) && ( gSog < 0.5 )
                && ( uptime.IsLongerThan( wxTimeSpan( 0, 30, 0, 0 ) ) ) )     // pjotrc 2010.02.15
                {
            //    First, delete any single anchorage waypoint closer than 0.25 NM from this point
            //    This will prevent clutter and database congestion....

            wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
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
    FrameCOGTimer.Stop();

    g_bframemax = IsMaximized();

    //    Record the current state of tracking
//    g_bTrackCarryOver = g_bTrackActive;

    TrackOff();

    if( pCurrentStack ) {
        g_restore_stackindex = pCurrentStack->CurrentStackEntry;
        g_restore_dbindex = pCurrentStack->GetCurrentEntrydbIndex();
        if(cc1 && cc1->GetQuiltMode())
            g_restore_dbindex = cc1->GetQuiltReferenceChartIndex();
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

//    pConfig->m_pNavObjectChangesSet->Clear();
    delete pConfig->m_pNavObjectChangesSet;

    //Remove any leftover Routes and Waypoints from config file as they were saved to navobj before
    pConfig->DeleteGroup( _T ( "/Routes" ) );
    pConfig->DeleteGroup( _T ( "/Marks" ) );
    pConfig->Flush();

    delete pConfig;             // All done
    pConfig = NULL;

    delete g_printData;
    delete g_pageSetupData;

    if( g_pAboutDlg ) g_pAboutDlg->Destroy();

//      Explicitely Close some children, especially the ones with event handlers
//      or that call GUI methods

#ifdef USE_S57
    if( g_pCM93OffsetDialog ) {
        g_pCM93OffsetDialog->Destroy();
        g_pCM93OffsetDialog = NULL;
    }
#endif

    if(g_FloatingToolbarDialog)
        g_FloatingToolbarDialog->Destroy();
    g_FloatingToolbarDialog = NULL;

    if( g_pAISTargetList ) {
        g_pAISTargetList->Disconnect_decoder();
        g_pAISTargetList->Destroy();
    }

    delete g_Compass;
    g_Compass = NULL;


#ifndef __OCPN__ANDROID__
    SetStatusBar( NULL );
#endif
    g_ChartBarWin = NULL;

    if(RouteManagerDialog::getInstanceFlag()){
        if( pRouteManagerDialog ) {
            pRouteManagerDialog->Destroy();
            pRouteManagerDialog = NULL;
        }
    }

    // pthumbwin is a cc1 child 
    pthumbwin = NULL;
    cc1->Destroy();
    cc1 = NULL;

    g_pauimgr->UnInit();
    delete g_pauimgr;
    g_pauimgr = NULL;

    //    Unload the PlugIns
    //      Note that we are waiting until after the canvas is destroyed,
    //      since some PlugIns may have created children of canvas.
    //      Such a PlugIn must stay intact for the canvas dtor to call DestoryChildren()

    if(ChartData)
        ChartData->PurgeCachePlugins();

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

    delete g_pMUX;

    //  Clear some global arrays, lists, and hash maps...
    for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
    {
        ConnectionParams *cp = g_pConnectionParams->Item(i);
        delete cp;
    }
    delete g_pConnectionParams;

    if(pLayerList){
        LayerList::iterator it;
        while(pLayerList->GetCount()){
            Layer *lay = pLayerList->GetFirst()->GetData();
            delete lay;                 // automatically removes the layer from list, see Layer dtor
        }
    }

    MsgPriorityHash::iterator it;
    for( it = NMEA_Msg_Hash.begin(); it != NMEA_Msg_Hash.end(); ++it ){
        NMEA_Msg_Container *pc = it->second;
        delete pc;
    }
    NMEA_Msg_Hash.clear();


    NMEALogWindow::Shutdown();

    g_FloatingToolbarDialog = NULL;
    g_bTempShowMenuBar = false;


    #define THREAD_WAIT_SECONDS  5
#ifdef ocpnUSE_GL
    // The last thing we do is finish the compression threads.
    // This way the main window is already invisible and to the user
    // it appears to have finished rather than hanging for several seconds
    // while the compression threads exit
    if(g_bopengl && g_CompressorPool && g_CompressorPool->GetRunningJobCount()){

        wxLogMessage(_T("Starting compressor pool drain"));
        wxDateTime now = wxDateTime::Now();
        time_t stall = now.GetTicks();
        time_t end = stall + THREAD_WAIT_SECONDS;

        int n_comploop = 0;
        while(stall < end ) {
            wxDateTime later = wxDateTime::Now();
            stall = later.GetTicks();

            wxString msg;
            msg.Printf(_T("Time: %d  Job Count: %d"), n_comploop, g_CompressorPool->GetRunningJobCount());
            wxLogMessage(msg);
            if(!g_CompressorPool->GetRunningJobCount())
                break;
            wxYield();
            wxSleep(1);
        }

        wxString fmsg;
        fmsg.Printf(_T("Finished compressor pool drain..Time: %d  Job Count: %d"),
                    n_comploop, g_CompressorPool->GetRunningJobCount());
        wxLogMessage(fmsg);
    }

#endif

    this->Destroy();
    gFrame = NULL;

#ifdef __OCPN__ANDROID__
    qDebug() << "Calling OnExit()";
    wxTheApp->OnExit();
#endif

}

void MyFrame::OnMove( wxMoveEvent& event )
{
    if( g_FloatingToolbarDialog ) g_FloatingToolbarDialog->RePosition();

    if( g_ChartBarWin && g_ChartBarWin->IsVisible()) g_ChartBarWin->RePosition();

//    UpdateGPSCompassStatusBox( );

    if( console && console->IsShown() ) PositionConsole();

//    Somehow, this method does not work right on Windows....
//      g_nframewin_posx = event.GetPosition().x;
//      g_nframewin_posy = event.GetPosition().y;

    g_nframewin_posx = GetPosition().x;
    g_nframewin_posy = GetPosition().y;
}

void MyFrame::ProcessCanvasResize( void )
{
    if( g_ChartBarWin ) {
        g_ChartBarWin->ReSize();
        g_ChartBarWin->RePosition();
    }

    if( g_FloatingToolbarDialog ) {
        g_FloatingToolbarDialog->RePosition();
        g_FloatingToolbarDialog->SetGeometry(g_Compass->IsShown(), g_Compass->GetRect());
        g_FloatingToolbarDialog->Realize();
        g_FloatingToolbarDialog->RePosition();

    }

    UpdateGPSCompassStatusBox( true );

    if( console && console->IsShown() ) PositionConsole();
}


int timer_sequence;
void MyFrame::TriggerResize(wxSize sz)
{
#ifdef __OCPN__ANDROID__
    m_newsize = sz;

    timer_sequence = 0;
    m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);

    resizeAndroidPersistents();

#endif
}


void MyFrame::OnResizeTimer(wxTimerEvent &event)
{
    if(timer_sequence == 0){
    //  On QT, we need to clear the status bar item texts to prevent the status bar from
    //  growing the parent frame due to unexpected width changes.
        if( m_pStatusBar != NULL ){
            int widths[] = { 2,2,2,2,2 };
           m_pStatusBar->SetStatusWidths( m_StatusBarFieldCount, widths );

            for(int i=0 ; i <  m_pStatusBar->GetFieldsCount() ; i++){
                m_pStatusBar->SetStatusText(_T(""), i);
            }
        }

        timer_sequence++;
        m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
        return;
    }



    if(timer_sequence == 1){
        SetSize(m_newsize);
        timer_sequence++;
        m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
        return;
    }

    if(timer_sequence == 2){
        if( g_FloatingToolbarDialog ) {
            g_Platform->GetDisplaySizeMM();             // causes a reload of all display metrics
            SetToolbarScale();
            g_FloatingToolbarDialog->RePosition();
            g_FloatingToolbarDialog->SetGeometry(g_Compass->IsShown(), g_Compass->GetRect());
            g_FloatingToolbarDialog->Realize();
            g_FloatingToolbarDialog->Refresh( false );
        }
        timer_sequence++;
        m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
        return;
    }

    if(timer_sequence == 3){
        g_Platform->onStagedResizeFinal();

        return;
    }



}



void MyFrame::OnSize( wxSizeEvent& event )
{
    ODoSetSize();
}

void MyFrame::ODoSetSize( void )
{
    int x, y;
    GetClientSize( &x, &y );
//      Resize the children

        if( m_pStatusBar != NULL ) {
            m_StatusBarFieldCount = g_Platform->GetStatusBarFieldCount();
            m_pStatusBar->SetFieldsCount(m_StatusBarFieldCount);
            
            if(m_StatusBarFieldCount){

                //  If the status bar layout is "complex", meaning more than two columns,
                //  then use custom crafted relative widths for the fields.
                //  Otherwise, just split the frame client width into equal spaces

                if(m_StatusBarFieldCount > 2){
                    int widths[] = { -6, -5, -5, -3, -4 };
                    m_pStatusBar->SetStatusWidths( m_StatusBarFieldCount, widths );
                }
                else if(m_StatusBarFieldCount == 2){
                    int cwidth = x * 90 / 100;
                    int widths[] = { 100, 100 };
                    widths[0] = cwidth * 6.4 / 10.0;
                    widths[1] = cwidth * 3.6 /  10.0;
                    m_pStatusBar->SetStatusWidths( m_StatusBarFieldCount, widths );
                }
                else{
                    int widths[] = { 100, 100 };
                    widths[0] = x * 90 / 100 ;
                    m_pStatusBar->SetStatusWidths( m_StatusBarFieldCount, widths );
                }

                int styles[] = { wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT };
                m_pStatusBar->SetStatusStyles( m_StatusBarFieldCount, styles );

                wxString sogcog( _T("SOG --- ") + getUsrSpeedUnit() + + _T("     ") + _T(" COG ---\u00B0") );
                m_pStatusBar->SetStatusText( sogcog, STAT_FIELD_SOGCOG );
                                    
            }
        }



    if( m_pStatusBar ) {
        //  Maybe resize the font so the text fits in the boxes

        wxRect stat_box;
        m_pStatusBar->GetFieldRect( 0, stat_box );
        // maximum size is 1/28 of the box width, or the box height - whicever is less
        int max_font_size = wxMin( (stat_box.width / 28), (stat_box.height) );

        wxFont sys_font = *wxNORMAL_FONT;
        int try_font_size = sys_font.GetPointSize();

#ifdef __WXOSX__
        int min_font_size = 10; // much less than 10pt is unreadably small on OS X
        try_font_size += 1;     // default to 1pt larger than system UI font
#else
        int min_font_size = 7;  // on Win/Linux the text does not shrink quite so fast
        try_font_size += 2;     // default to 2pt larger than system UI font
#endif

        // get the user's preferred font, or if none set then the system default with the size overridden
        wxFont* templateFont = FontMgr::Get().GetFont( _("StatusBar"), try_font_size );
        int font_size = templateFont->GetPointSize();

        font_size = wxMin( font_size, max_font_size );  // maximum to fit in the statusbar boxes
        font_size = wxMax( font_size, min_font_size );  // minimum to stop it being unreadable

#ifdef __OCPN__ANDROID__
        font_size = templateFont->GetPointSize();
#endif


        wxFont *pstat_font = FontMgr::Get().FindOrCreateFont( font_size,
              wxFONTFAMILY_DEFAULT, templateFont->GetStyle(), templateFont->GetWeight(), false,
              templateFont->GetFaceName() );

        int min_height = stat_box.height;

        m_pStatusBar->SetFont( *pstat_font );
#ifdef __OCPN__ANDROID__
        min_height = ( pstat_font->GetPointSize() * getAndroidDisplayDensity() ) + 10;
        m_pStatusBar->SetMinHeight( min_height );
//        qDebug() <<"StatusBar min height:" << min_height << "StatusBar font points:" << pstat_font->GetPointSize();
#endif
//        wxString msg;
//        msg.Printf(_T("StatusBar min height: %d    StatusBar font points: %d"), min_height, pstat_font->GetPointSize());
//        wxLogMessage(msg);

    }

    int cccw = x;
    int ccch = y;

    if( cc1 ) {
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
        g_FloatingToolbarDialog->SetGeometry(g_Compass->IsShown(), g_Compass->GetRect());
        g_FloatingToolbarDialog->Realize();

        if( oldSize != g_FloatingToolbarDialog->GetSize() )
            g_FloatingToolbarDialog->Refresh( false );

        g_FloatingToolbarDialog->RePosition();

    }


    UpdateGPSCompassStatusBox( true );

    if( console ) PositionConsole();

    if( cc1 ) {
        g_Piano->FormatKeys();
        if( g_ChartBarWin ) {
            g_ChartBarWin->ReSize();
            g_ChartBarWin->RePosition();
        }
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

    if( pthumbwin )
        pthumbwin->SetMaxSize( cc1->GetParent()->GetClientSize() );

    //  Reset the options dialog size logic
    options_lastWindowSize = wxSize(0,0);
    options_lastWindowPos = wxPoint(0,0);

}

void MyFrame::PositionConsole( void )
{
    if( NULL == cc1 ) return;
    //    Reposition console based on its size and chartcanvas size
    int ccx, ccy, ccsx, ccsy, consx, consy;
    cc1->GetSize( &ccsx, &ccsy );
    cc1->GetPosition( &ccx, &ccy );

    console->GetSize( &consx, &consy );

    int yOffset = 60;
    if(g_Compass){
        if(g_Compass->GetRect().y < 100)        // Compass is is normal upper right position.                
            yOffset = g_Compass->GetRect().y + g_Compass->GetRect().height + 45;
    }
    
    wxPoint screen_pos = ClientToScreen( wxPoint( ccx + ccsx - consx - 2, ccy + yOffset ) );
    console->Move( screen_pos );
}

void MyFrame::UpdateAllFonts()
{
    if( console ) {
        console->UpdateFonts();
        //    Reposition console
        PositionConsole();
    }

    //  Close and destroy any persistent dialogs, so that new fonts will be utilized
    DestroyPersistentDialogs();

    if( pWayPointMan ) pWayPointMan->ClearRoutePointFonts();

    cc1->Refresh();
}

void MyFrame::DestroyPersistentDialogs()
{
    if( g_pais_query_dialog_active ) {
        g_pais_query_dialog_active->Hide();
        g_pais_query_dialog_active->Destroy();
        g_pais_query_dialog_active = NULL;
    }

    if( pRoutePropDialog ) {
        pRoutePropDialog->Hide();
        pRoutePropDialog->Destroy();
        pRoutePropDialog = NULL;
    }

    if( pTrackPropDialog ) {
        pTrackPropDialog->Hide();
        pTrackPropDialog->Destroy();
        pTrackPropDialog = NULL;
    }

    if( pMarkPropDialog ) {
        pMarkPropDialog->Hide();
        pMarkPropDialog->Destroy();
        pMarkPropDialog = NULL;
    }

    if( g_pObjectQueryDialog ) {
        g_pObjectQueryDialog->Hide();
        g_pObjectQueryDialog->Destroy();
        g_pObjectQueryDialog = NULL;
    }

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

    double best_scale = cc1->GetBestStartScale(dbi_hint, vp);

    cc1->SetVPScale( best_scale );

    if(cc1->GetQuiltMode())
        dbi_hint = cc1->GetQuiltReferenceChartIndex();

    //    Refresh the canvas, selecting the "best" chart,
    //    applying the prior ViewPort exactly
    ChartsRefresh( dbi_hint, vp, true );

    //    Message box is deferred so that canvas refresh occurs properly before dialog
    if( bgroup_override ) {
        wxString msg( _("Group \"") );
        msg += GetGroupName( old_group_index );
        msg += _("\" is empty, switching to \"All Active Charts\" group.");

        OCPNMessageBox( this, msg, _("OpenCPN Group Notice"), wxOK );
    }
}

void MyFrame::OnToolLeftClick( wxCommandEvent& event )
{
    if( s_ProgDialog ) return;

    switch( event.GetId() ){
        case ID_MENU_SCALE_OUT:
        case ID_STKUP:
            DoStackDelta( 1 );
            DoChartUpdate();
            break;

        case ID_MENU_SCALE_IN:
        case ID_STKDN:
            DoStackDelta( -1 );
            DoChartUpdate();
            break;

        case ID_MENU_ZOOM_IN:
        case ID_ZOOMIN: {
            cc1->ZoomCanvas( 2.0, false );
            DoChartUpdate();
            break;
        }

        case ID_MENU_ZOOM_OUT:
        case ID_ZOOMOUT: {
            cc1->ZoomCanvas( 0.5, false );
            DoChartUpdate();
            break;
        }

        case ID_MENU_ROUTE_NEW:
        case ID_ROUTE: {
            if( 0 == nRoute_State ){
                cc1->StartRoute();
            }
            else {
                cc1->FinishRoute();
            }

#ifdef __OCPN__ANDROID__
            androidSetRouteAnnunciator(nRoute_State == 1);
#endif
            break;
        }

        case ID_MENU_TOOL_MEASURE: {
            cc1->StartMeasureRoute();
            break;
        }

        case ID_MENU_MARK_BOAT: {
            cc1->DropMarker(true);
            break;
        }

        case ID_MENU_MARK_CURSOR: {
            cc1->DropMarker(false);
            break;
        }

        case ID_MENU_NAV_FOLLOW:
        case ID_FOLLOW: {
            TogglebFollow();
            break;
        }

        case ID_MENU_CHART_OUTLINES: {
            ToggleChartOutlines();
            break;
        }

        case ID_MENU_CHART_QUILTING: {
            ToggleQuiltMode();
            break;
        }

        case ID_MENU_UI_CHARTBAR: {
            ToggleChartBar();
            break;
        }

#ifdef USE_S57
        case ID_MENU_ENC_TEXT:
        case ID_ENC_TEXT: {
            ToggleENCText();
            break;
        }
        case ID_MENU_ENC_LIGHTS: {
            ToggleLights();
            break;
        }
        case ID_MENU_ENC_SOUNDINGS: {
            ToggleSoundings();
            break;
        }
        case ID_MENU_ENC_ANCHOR: {
            ToggleAnchor();
            break;
        }
#endif

        case ID_MENU_AIS_TARGETS: {
            if ( g_bShowAIS ) SetAISDisplayStyle(2);
            else SetAISDisplayStyle(0);
            break;
        }
         case ID_MENU_AIS_MOORED_TARGETS: {
            g_bHideMoored = !g_bHideMoored;
            break;
        }
         case ID_MENU_AIS_SCALED_TARGETS: {
             if(g_bShowScaled)
                SetAISDisplayStyle(0);
            else
                SetAISDisplayStyle(1);
            
            break;
        }
        case ID_AIS: {
            SetAISDisplayStyle(-1);
            break;
        }

        case ID_MENU_AIS_TARGETLIST: {
            if ( cc1 ) cc1->ShowAISTargetList();
            break;
        }

        case ID_MENU_AIS_TRACKS: {
            g_bAISShowTracks = !g_bAISShowTracks;
            SetMenubarItemState(ID_MENU_AIS_TRACKS, g_bAISShowTracks);
            break;
        }

        case ID_MENU_AIS_CPADIALOG: {
            g_bAIS_CPA_Alert = !g_bAIS_CPA_Alert;
            SetMenubarItemState(ID_MENU_AIS_CPADIALOG, g_bAIS_CPA_Alert);
            break;
        }

        case ID_MENU_AIS_CPASOUND: {
            g_bAIS_CPA_Alert_Audio = !g_bAIS_CPA_Alert_Audio;
            SetMenubarItemState(ID_MENU_AIS_CPASOUND, g_bAIS_CPA_Alert_Audio);
            break;
        }

        case wxID_PREFERENCES:
        case ID_SETTINGS: {
            DoSettings();
            break;
        }


        case ID_MENU_SETTINGS_BASIC:
        {
 #ifdef __OCPN__ANDROID__
            ///LoadS57();
            DoAndroidPreferences();
 #else
            DoSettings();
 #endif
        }

        case ID_MENU_UI_FULLSCREEN: {
            ToggleFullScreen();
            break;
        }

        case ID_MENU_SHOW_CURRENTS:
        case ID_CURRENT: {
            LoadHarmonics();

            if( ptcmgr->IsReady() ) {
                cc1->SetbShowCurrent( !cc1->GetbShowCurrent() );
                SetToolbarItemState( ID_CURRENT, cc1->GetbShowCurrent() );
                wxString tip = _("Show Currents");
                if(cc1->GetbShowCurrent())
                    tip = _("Hide Currents");
                if( g_toolbar )
                    g_toolbar->SetToolShortHelp( ID_CURRENT, tip );

                SetMenubarItemState( ID_MENU_SHOW_CURRENTS, cc1->GetbShowCurrent() );
                cc1->ReloadVP();
            } else {
                wxLogMessage( _T("Chart1::Event...TCMgr Not Available") );
                cc1->SetbShowCurrent( false );
                SetToolbarItemState( ID_CURRENT, false );
                SetMenubarItemState( ID_MENU_SHOW_CURRENTS, false );
            }

            if( cc1->GetbShowCurrent() ) {
                FrameTCTimer.Start( TIMER_TC_VALUE_SECONDS * 1000, wxTIMER_CONTINUOUS );
                cc1->SetbTCUpdate( true );                        // force immediate update
            } else
                FrameTCTimer.Stop();

            cc1->Refresh( false );

            break;

        }

        case ID_MENU_SHOW_TIDES:
        case ID_TIDE: {
            LoadHarmonics();

            if( ptcmgr->IsReady() ) {
                cc1->SetbShowTide( !cc1->GetbShowTide() );
                SetToolbarItemState( ID_TIDE, cc1->GetbShowTide() );
                wxString tip = _("Show Tides");
                if(cc1->GetbShowTide())
                    tip = _("Hide Tides");
                if( g_toolbar )
                    g_toolbar->SetToolShortHelp( ID_TIDE, tip );

                SetMenubarItemState( ID_MENU_SHOW_TIDES, cc1->GetbShowTide() );
                cc1->ReloadVP();
            } else {
                wxLogMessage( _("Chart1::Event...TCMgr Not Available") );
                cc1->SetbShowTide( false );
                SetToolbarItemState( ID_TIDE, false );
                SetMenubarItemState( ID_MENU_SHOW_TIDES, false );
            }

            if( cc1->GetbShowTide() ) {
                FrameTCTimer.Start( TIMER_TC_VALUE_SECONDS * 1000, wxTIMER_CONTINUOUS );
                cc1->SetbTCUpdate( true );                        // force immediate update
            } else
                FrameTCTimer.Stop();

            cc1->Refresh( false );

            break;

        }

        case wxID_ABOUT:
        case ID_ABOUT: {
            if( !g_pAboutDlg )
                g_pAboutDlg = new about( this, g_Platform->GetSharedDataDir() );
            else
                g_pAboutDlg->SetFocus();
            g_pAboutDlg->Show();

            break;
        }

        case wxID_HELP: {
            g_Platform->LaunchLocalHelp();
            break;
        }

        case ID_PRINT: {
            DoPrint();
            break;
        }

        case ID_MENU_UI_COLSCHEME:
        case ID_COLSCHEME: {
            ToggleColorScheme();
            break;
        }

        case ID_TBEXIT: {
            Close();
            break;
        }

        case ID_MENU_OQUIT: {
            Close();
            break;
        }

        case ID_MENU_ROUTE_MANAGER:
        case ID_ROUTEMANAGER: {
            pRouteManagerDialog = RouteManagerDialog::getInstance( cc1 ); // There is one global instance of the Dialog

            pRouteManagerDialog->UpdateRouteListCtrl();
            pRouteManagerDialog->UpdateTrkListCtrl();
            pRouteManagerDialog->UpdateWptListCtrl();
            pRouteManagerDialog->UpdateLayListCtrl();

            if(g_bresponsive){
                if(g_ChartBarWin && g_ChartBarWin->IsShown() )
                    g_ChartBarWin->Hide();
            }
            pRouteManagerDialog->Show();

            //    Required if RMDialog is not STAY_ON_TOP
#ifdef __WXOSX__
            pRouteManagerDialog->Centre();
            pRouteManagerDialog->Raise();
#endif
            break;
        }

        case ID_MENU_NAV_TRACK:
        case ID_TRACK: {
            if( !g_bTrackActive ) {
                TrackOn();
                g_bTrackCarryOver = true;
            } else {
                TrackOff( true );
                g_bTrackCarryOver = false;
                cc1->Refresh(true);
            }
            break;
        }

        case ID_MENU_CHART_NORTHUP:
        case ID_MENU_CHART_COGUP:
        case ID_TBSTATBOX: {
            ToggleCourseUp();
            break;
        }

        case ID_MENU_MARK_MOB:
        case ID_MOB: {
            ActivateMOB();
            break;
        }


        //  Various command events coming from (usually) other threads,
        //  used to control OCPN modes in a thread-safe way.

        case ID_CMD_SELECT_CHART_TYPE:{
            selectChartDisplay( event.GetExtraLong(), -1);
            break;
        }

        case ID_CMD_SELECT_CHART_FAMILY:{
            selectChartDisplay( -1, event.GetExtraLong());
            break;
        }

        case ID_CMD_APPLY_SETTINGS:{
            applySettingsString(event.GetString());
            break;
        }

        case ID_CMD_TRIGGER_RESIZE:{
            #ifdef __OCPN__ANDROID__
            TriggerResize( getAndroidConfigSize() );
            #endif
            break;
        }

        case ID_CMD_NULL_REFRESH:{
            Refresh(true);
            break;
        }

        case ID_CMD_SETVP:{
            setStringVP(event.GetString());
            break;
        }

        case ID_CMD_INVALIDATE:{
            if(cc1)
                cc1->InvalidateGL();
            Refresh(true);
            break;
        }

        case ID_CMD_POST_JSON_TO_PLUGINS:{
            
            // Extract the Message ID which is embedded in the JSON string passed in the event
            wxJSONValue  root;
            wxJSONReader reader;
            
            int numErrors = reader.Parse( event.GetString(), &root );
            if ( numErrors == 0 )  {
                if(root[_T("MessageID")].IsString()){
                    wxString MsgID = root[_T("MessageID")].AsString();
                    SendPluginMessage( MsgID, event.GetString() );  // Send to all PlugIns
                }
            }
            
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
                        return; // required to prevent event.Skip() being called
                    }
                }
            }

            // If we didn't handle the event, allow it to bubble up to other handlers.
            // This is required for the system menu items (Hide, etc) on OS X to work.
            // This must only be called if we did NOT handle the event, otherwise it
            // stops the menu items from working on Windows.
            event.Skip();

            break;
        }

    }         // switch

}

void MyFrame::SetAISDisplayStyle(int StyleIndx)
{
    // make some arrays to hold the dfferences between cycle steps
    //show all, scaled, hide all
    bool g_bShowAIS_Array[3] = {true, true, false}; 
    bool g_bShowScaled_Array[3] = {false, true, true};
    wxString ToolShortHelp_Array[3] = { _("Show all AIS Targets"),
                                        _("Attenuate less critical AIS Targets"),
                                        _("Hide AIS Targets") };
    wxString iconName_Array[3] = { _("AIS"),  _("AIS_Suppressed"), _("AIS_Disabled")};
    int ArraySize = 3;
    int AIS_Toolbar_Switch = 0;
    if (StyleIndx == -1){// -1 means coming from toolbar button
        //find current state of switch 
        for ( int i = 1; i < ArraySize; i++){
            if( (g_bShowAIS_Array[i] == g_bShowAIS) && (g_bShowScaled_Array[i] == g_bShowScaled) )
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
    
    //Set found values to variables
    g_bShowAIS = g_bShowAIS_Array[AIS_Toolbar_Switch];
    g_bShowScaled = g_bShowScaled_Array[AIS_Toolbar_Switch];
    if( g_toolbar ) {
        g_toolbar->SetToolShortHelp( ID_AIS, ToolShortHelp_Array[AIS_Toolbar_Switch_Next] );
        if( m_pAISTool ) {
            g_toolbar->SetToolNormalBitmapEx( m_pAISTool, iconName_Array[AIS_Toolbar_Switch] );
            g_toolbar->Refresh();
            m_lastAISiconName = iconName_Array[AIS_Toolbar_Switch];
        }
    }

    UpdateGlobalMenuItems();
    cc1->ReloadVP();    
}

void MyFrame::setStringVP(wxString VPS)
{
    if(!cc1)
        return;
    
    wxStringTokenizer tkz(VPS, _T(";"));
    
    wxString token = tkz.GetNextToken();
    double lat = gLat;
    token.ToDouble(&lat);
    
    token = tkz.GetNextToken();
    double lon = gLon;
    token.ToDouble(&lon);
    
    token = tkz.GetNextToken();
    double scale_ppm = cc1->GetVP().view_scale_ppm;
    token.ToDouble(&scale_ppm);
    
    cc1->SetViewPoint( lat, lon, scale_ppm, 0, cc1->GetVPRotation() );
    
}




void MyFrame::DoSettings()
{
    if (g_boptionsactive)
        return;

    bool bnewtoolbar = !( DoOptionsDialog() == 0 );

    //              Apply various system settings
    ApplyGlobalSettings( true, bnewtoolbar );                 // flying update

    if( g_FloatingToolbarDialog )
        g_FloatingToolbarDialog->RefreshFadeTimer();

    if( cc1->GetbShowCurrent() || cc1->GetbShowTide() )
        LoadHarmonics();

    //  The chart display options may have changed, especially on S57 ENC,
    //  So, flush the cache and redraw
    cc1->ReloadVP();

}

void MyFrame::ShowChartBarIfEnabled()
{
    if(g_ChartBarWin){
        g_ChartBarWin->Show(g_bShowChartBar);
        if(g_bShowChartBar){
            g_ChartBarWin->Move(0,0);
            g_ChartBarWin->RePosition();
         }
    }

}


void MyFrame::ToggleChartBar()
{
    g_bShowChartBar = !g_bShowChartBar;

    if( g_ChartBarWin ) {
        g_ChartBarWin->Show(g_bShowChartBar);

        if(g_bShowChartBar) {
            g_ChartBarWin->Move(0,0);
            g_ChartBarWin->RePosition();
            gFrame->Raise();
        }
        SendSizeEvent();
    } else if(g_bShowChartBar)
        cc1->m_brepaint_piano = true;

    cc1->ReloadVP(); // needed to set VP.pix_height
    Refresh();

    if(g_bShowChartBar) {
        DoChartUpdate();
        UpdateControlBar();
    }

    SetMenubarItemState( ID_MENU_UI_CHARTBAR, g_bShowChartBar );
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
    long style = wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION;; // | wxFULLSCREEN_NOMENUBAR;

    ShowFullScreen( to, style );
    UpdateToolbar( global_color_scheme );
    SurfaceToolbar();
    UpdateControlBar();
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
    pWP_MOB->m_bIsolatedMark = true;
    pWP_MOB->SetWaypointArrivalRadius( -1.0 ); // Negative distance is code to signal "Never Arrive"

    pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_MOB );
    pConfig->AddNewWayPoint( pWP_MOB, -1 );       // use auto next num


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

        wxJSONValue v;
        v[_T("GUID")] = temp_route->m_GUID;
        wxString msg_id( _T("OCPN_MAN_OVERBOARD") );
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
    }

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
        pRouteManagerDialog->UpdateRouteListCtrl();
        pRouteManagerDialog->UpdateWptListCtrl();
    }

    cc1->InvalidateGL();
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
    if(pConfig)
        pConfig->AddNewRoute( g_pActiveTrack, 0 );

    g_pActiveTrack->Start();

    SetToolbarItemState( ID_TRACK, g_bTrackActive );
    if( g_toolbar )
        g_toolbar->SetToolShortHelp( ID_TRACK, _("Disable Tracking") );

    SetMenubarItemState( ID_MENU_NAV_TRACK, g_bTrackActive );

    #ifdef __OCPN__ANDROID__
    androidSetTrackTool(true);
    #endif


    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
    {
        pRouteManagerDialog->UpdateTrkListCtrl();
        pRouteManagerDialog->UpdateRouteListCtrl();
    }

    wxJSONValue v;
    wxDateTime now;
    now = now.Now().ToUTC();
    wxString name = g_pActiveTrack->m_RouteNameString;
    if(name.IsEmpty())
    {
        RoutePoint *rp = g_pActiveTrack->GetPoint( 1 );
        if( rp && rp->GetCreateTime().IsValid() )
            name = rp->GetCreateTime().FormatISODate() + _T(" ") + rp->GetCreateTime().FormatISOTime();
        else
            name = _("(Unnamed Track)");
    }
    v[_T("Name")] = name;
    v[_T("GUID")] = g_pActiveTrack->m_GUID;
    wxString msg_id( _T("OCPN_TRK_ACTIVATED") );
    g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
}

Track *MyFrame::TrackOff( bool do_add_point )
{
    Track *return_val = g_pActiveTrack;

    if( g_pActiveTrack )
    {
        wxJSONValue v;
        wxString msg_id( _T("OCPN_TRK_DEACTIVATED") );
        v[_T("GUID")] = g_pActiveTrack->m_GUID;
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );

        g_pActiveTrack->Stop( do_add_point );

        if( g_pActiveTrack->GetnPoints() < 2 ) {
            g_pRouteMan->DeleteRoute( g_pActiveTrack );
            return_val = NULL;
        }
        else {
            if( g_bTrackDaily ) {
                Track *pExtendTrack = g_pActiveTrack->DoExtendDaily();
                if(pExtendTrack) {
                    g_pRouteMan->DeleteRoute( g_pActiveTrack );
                    return_val = pExtendTrack;
                }
            }
        }
    }

    g_pActiveTrack = NULL;

    g_bTrackActive = false;

    if(RouteManagerDialog::getInstanceFlag()){
        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ){
            pRouteManagerDialog->UpdateTrkListCtrl();
            pRouteManagerDialog->UpdateRouteListCtrl();
        }
    }

    SetToolbarItemState( ID_TRACK, g_bTrackActive );
    if( g_toolbar )
        g_toolbar->SetToolShortHelp( ID_TRACK, _("Enable Tracking") );
    SetMenubarItemState( ID_MENU_NAV_TRACK, g_bTrackActive );

    #ifdef __OCPN__ANDROID__
    androidSetTrackTool(false);
    #endif

    return return_val;
}

bool MyFrame::ShouldRestartTrack( void )
{
    if( !g_pActiveTrack || !g_bTrackDaily)
        return false;
    time_t now = wxDateTime::Now().GetTicks();
    time_t today = wxDateTime::Today().GetTicks();
    int rotate_at = 0;
    switch( g_track_rotate_time_type )
    {
        case TIME_TYPE_LMT:
            rotate_at = g_track_rotate_time + wxRound(gLon * 3600. / 15.);
            break;
        case TIME_TYPE_COMPUTER:
            rotate_at = g_track_rotate_time;
            break;
        case TIME_TYPE_UTC:
            int utc_offset = wxDateTime::Now().GetTicks() - wxDateTime::Now().ToUTC().GetTicks();
            rotate_at = g_track_rotate_time + utc_offset;
            break;
    }
    if( rotate_at > 86400 )
        rotate_at -= 86400;
    else if (rotate_at < 0 )
        rotate_at += 86400;
    if( now >= m_last_track_rotation_ts + 86400 - 3600 &&
        now - today >= rotate_at )
    {
        if( m_last_track_rotation_ts == 0 )
        {
            if( now - today > rotate_at)
                m_last_track_rotation_ts = today + rotate_at;
            else
                m_last_track_rotation_ts = today + rotate_at - 86400;
            return false;
        }
        m_last_track_rotation_ts = now;
        return true;
    }
    return false;
}

void MyFrame::TrackDailyRestart( void )
{
    if( !g_pActiveTrack )
        return;

    Track *pPreviousTrack = TrackOff( true );
    TrackOn();

    //  Set the restarted track's current state such that the current track point's attributes match the
    //  attributes of the last point of the track that was just stopped at midnight.

    if( pPreviousTrack ) {
        RoutePoint *pMidnightPoint = pPreviousTrack->GetLastPoint();
        g_pActiveTrack->AdjustCurrentTrackPoint(pMidnightPoint);
    }

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

        if( g_COGAvgSec > 0) {
            for( int i = 0; i < g_COGAvgSec; i++ )
                COGTable[i] = stuff;
        }
        g_COGAvg = stuff;
        gFrame->FrameCOGTimer.Start( 100, wxTIMER_CONTINUOUS );
    } else {
        if ( !g_bskew_comp && (fabs(cc1->GetVPSkew()) > 0.0001))
            cc1->SetVPRotation(cc1->GetVPSkew());
        else
            cc1->SetVPRotation(0); /* reset to north up */
    }


    SetMenubarItemState( ID_MENU_CHART_COGUP, g_bCourseUp );
    SetMenubarItemState( ID_MENU_CHART_NORTHUP, !g_bCourseUp );

    if(m_pMenuBar)
        m_pMenuBar->SetLabel( ID_MENU_CHART_NORTHUP, _("North Up Mode") );

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
        SetToolbarItemState( ID_ENC_TEXT, ps52plib->GetShowS57Text() );
        wxString tip = _("Show ENC Text (T)");
        if(ps52plib->GetShowS57Text())
            tip = _("Hide ENC Text (T)");
        if( g_toolbar )
            g_toolbar->SetToolShortHelp( ID_ENC_TEXT, tip );

        SetMenubarItemState( ID_MENU_ENC_TEXT, ps52plib->GetShowS57Text() );
        cc1->ReloadVP();
    }

#endif
}

void MyFrame::ToggleSoundings( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        ps52plib->SetShowSoundings( !ps52plib->GetShowSoundings() );
        SetMenubarItemState( ID_MENU_ENC_SOUNDINGS, ps52plib->GetShowSoundings() );
        cc1->ReloadVP();
    }
#endif
}

bool MyFrame::ToggleLights( bool doToggle, bool temporary )
{
    bool oldstate = true;

#ifdef USE_S57
    OBJLElement *pOLE = NULL;
    if( ps52plib ) {
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "LIGHTS", 6 ) ) {
                oldstate = pOLE->nViz != 0;
                break;
            }
	    pOLE = NULL;
        }

        oldstate &= !ps52plib->IsObjNoshow("LIGHTS");

        if( doToggle ){
            if(oldstate)                            // On, going off
                ps52plib->AddObjNoshow("LIGHTS");
            else{                                   // Off, going on
                if(pOLE)
                    pOLE->nViz = 1;
                ps52plib->RemoveObjNoshow("LIGHTS");
            }

            SetMenubarItemState( ID_MENU_ENC_LIGHTS, !oldstate );
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
        int old_vis =  0;
        OBJLElement *pOLE = NULL;

        if(  MARINERS_STANDARD == ps52plib->GetDisplayCategory()){
            // Need to loop once for SBDARE, which is our "master", then for
            // other categories, since order is unknown?
            for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
                OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
                if( !strncmp( pOLE->OBJLName, "SBDARE", 6 ) ) {
                    old_vis = pOLE->nViz;
                    break;
                }
		pOLE = NULL;
            }
        }
        else if(OTHER == ps52plib->GetDisplayCategory())
            old_vis = true;

        const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL", "SBDARE" };
        unsigned int num = sizeof(categories) / sizeof(categories[0]);

        old_vis &= !ps52plib->IsObjNoshow("SBDARE");

        if(old_vis){                            // On, going off
            for( unsigned int c = 0; c < num; c++ ) {
                ps52plib->AddObjNoshow(categories[c]);
            }
        }
        else{                                   // Off, going on
            for( unsigned int c = 0; c < num; c++ ) {
                ps52plib->RemoveObjNoshow(categories[c]);
            }

            unsigned int cnt = 0;
            for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
                OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
                for( unsigned int c = 0; c < num; c++ ) {
                    if( !strncmp( pOLE->OBJLName, categories[c], 6 ) ) {
                        pOLE->nViz = 1;         // force on
                        cnt++;
                        break;
                    }
                }
                if( cnt == num ) break;
            }

        }

        SetMenubarItemState( ID_MENU_ENC_ANCHOR, !old_vis );

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
    JumpToPosition(gLat, gLon, cc1->GetVPScale());
    cc1->m_bFollow = true;

    SetToolbarItemState( ID_FOLLOW, true );
    SetMenubarItemState( ID_MENU_NAV_FOLLOW, true );
    
    #ifdef __OCPN__ANDROID__
    androidSetFollowTool(true);
    #endif

    DoChartUpdate();
    cc1->ReloadVP();
}

void MyFrame::ClearbFollow( void )
{
    //    Center the screen on the GPS position, for lack of a better place
    vLat = gLat;
    vLon = gLon;

    #ifdef __OCPN__ANDROID__
    androidSetFollowTool(false);
    #endif

    cc1->m_bFollow = false;
    SetToolbarItemState( ID_FOLLOW, false );
    SetMenubarItemState( ID_MENU_NAV_FOLLOW, false );

    DoChartUpdate();
    cc1->ReloadVP();
}

void MyFrame::ToggleChartOutlines( void )
{
    if( !g_bShowOutlines ) g_bShowOutlines = true;
    else
        g_bShowOutlines = false;

    cc1->Refresh( false );

#ifdef ocpnUSE_GL         // opengl renders chart outlines as part of the chart this needs a full refresh
    if( g_bopengl )
        cc1->GetglCanvas()->Invalidate();
#endif

    SetMenubarItemState( ID_MENU_CHART_OUTLINES, g_bShowOutlines );
}

void MyFrame::SetMenubarItemState( int item_id, bool state )
{
    if( m_pMenuBar ) {
        bool enabled = m_pMenuBar->IsEnabled( item_id );
        m_pMenuBar->Enable( item_id, false );
        m_pMenuBar->Check( item_id, state );
        m_pMenuBar->Enable( item_id, enabled );
     }
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

void MyFrame::SetToolbarItemSVG( int tool_id, wxString normalSVGfile, wxString rolloverSVGfile, wxString toggledSVGfile )
{
    if( g_toolbar ) {
        g_toolbar->SetToolBitmapsSVG( tool_id, normalSVGfile, rolloverSVGfile, toggledSVGfile );
        wxRect rect = g_toolbar->GetToolRect( tool_id );
        g_toolbar->RefreshRect( rect );
    }
}


void MyFrame::ApplyGlobalSettings( bool bFlyingUpdate, bool bnewtoolbar )
{
    //             ShowDebugWindow as a wxStatusBar
    m_StatusBarFieldCount = g_Platform->GetStatusBarFieldCount();

#ifdef __WXMSW__
    UseNativeStatusBar( false );              // better for MSW, undocumented in frame.cpp
#endif

    if( g_bShowStatusBar ) {
        if( !m_pStatusBar ) {
            m_pStatusBar = CreateStatusBar( m_StatusBarFieldCount, 0 );   // No wxST_SIZEGRIP needed
            ApplyGlobalColorSchemetoStatusBar();
        }

    } else {
        if( m_pStatusBar ) {
            m_pStatusBar->Destroy();
            m_pStatusBar = NULL;
            SetStatusBar( NULL );
        }
    }

    SendSizeEvent();

    /*
     * Menu Bar - add or remove it if necessary, and update the state of the menu items
     */
#ifdef __WXOSX__
    bool showMenuBar = true;    // the menu bar is always visible in OS X
#else
    bool showMenuBar = pConfig->m_bShowMenuBar; // get visibility from options

    if (!showMenuBar && g_bTempShowMenuBar)     // allows pressing alt to temporarily show
        showMenuBar = true;
#endif

    if ( showMenuBar ) {
        //  Menu bar has some dependencies on S52 PLIB, so be sure it is loaded.
        LoadS57();
        
        if ( !m_pMenuBar ) {    // add the menu bar if it is enabled
            m_pMenuBar = new wxMenuBar();
            RegisterGlobalMenuItems();
            SetMenuBar(m_pMenuBar); // must be after RegisterGlobalMenuItems for wx to populate the OS X App Menu correctly
        }
        UpdateGlobalMenuItems(); // update the state of the menu items (checkmarks etc)
    } else {
        if ( m_pMenuBar ) {     // remove the menu bar if it is disabled
            SetMenuBar( NULL );
            m_pMenuBar->Destroy();
            m_pMenuBar = NULL;
        }
    }

    SendSizeEvent();

    if( bFlyingUpdate )
        g_Compass->Show(pConfig->m_bShowCompassWin);

    if( bnewtoolbar ) UpdateToolbar( global_color_scheme );

}


wxString _menuText( wxString name, wxString shortcut ) {
    wxString menutext;
    menutext << name;
    if(!g_bresponsive)
        menutext << _T("\t") << shortcut;
    return menutext;
}

void MyFrame::RegisterGlobalMenuItems()
{
    if ( !m_pMenuBar ) return;  // if there isn't a menu bar


    wxMenu *nav_menu = new wxMenu();
    nav_menu->AppendCheckItem( ID_MENU_NAV_FOLLOW, _menuText(_("Auto Follow"), _T("Ctrl-A")) );
    nav_menu->AppendCheckItem( ID_MENU_NAV_TRACK, _("Enable Tracking") );
    nav_menu->AppendSeparator();
    nav_menu->AppendRadioItem( ID_MENU_CHART_NORTHUP, _("North Up Mode") );
    nav_menu->AppendRadioItem( ID_MENU_CHART_COGUP, _("Course Up Mode") );
    nav_menu->AppendSeparator();
#ifndef __WXOSX__
    nav_menu->Append( ID_MENU_ZOOM_IN, _menuText(_("Zoom In"), _T("+")) );
    nav_menu->Append( ID_MENU_ZOOM_OUT, _menuText(_("Zoom Out"), _T("-")) );
#else
    nav_menu->Append( ID_MENU_ZOOM_IN, _menuText(_("Zoom In"), _T("Alt-+")) );
    nav_menu->Append( ID_MENU_ZOOM_OUT, _menuText(_("Zoom Out"), _T("Alt--")) );
#endif
    nav_menu->AppendSeparator();
    nav_menu->Append( ID_MENU_SCALE_IN, _menuText(_("Larger Scale Chart"), _T("Ctrl-Left")) );
    nav_menu->Append( ID_MENU_SCALE_OUT, _menuText(_("Smaller Scale Chart"), _T("Ctrl-Right")) );
#ifndef __WXOSX__
    nav_menu->AppendSeparator();
    nav_menu->Append( ID_MENU_OQUIT, _menuText(_("Exit OpenCPN"), _T("Ctrl-Q")) );
#endif
    m_pMenuBar->Append( nav_menu, _("&Navigate") );


    wxMenu* view_menu = new wxMenu();
#ifndef __WXOSX__
    view_menu->AppendCheckItem( ID_MENU_CHART_QUILTING, _menuText(_("Enable Chart Quilting"), _T("Q")) );
    view_menu->AppendCheckItem( ID_MENU_CHART_OUTLINES, _menuText(_("Show Chart Outlines"), _T("O")) );
#else
    view_menu->AppendCheckItem( ID_MENU_CHART_QUILTING, _menuText(_("Enable Chart Quilting"), _T("Alt-Q")) );
    view_menu->AppendCheckItem( ID_MENU_CHART_OUTLINES, _menuText(_("Show Chart Outlines"), _T("Alt-O")) );
#endif
    view_menu->AppendCheckItem( ID_MENU_UI_CHARTBAR, _menuText(_("Show Chart Bar"), _T("Ctrl-B")) );

#ifdef USE_S57
    view_menu->AppendSeparator();
#ifndef __WXOSX__
    view_menu->AppendCheckItem( ID_MENU_ENC_TEXT, _menuText(_("Show ENC Text"), _T("T")) );
    view_menu->AppendCheckItem( ID_MENU_ENC_LIGHTS, _menuText(_("Show ENC Lights"), _T("L")) );
    view_menu->AppendCheckItem( ID_MENU_ENC_SOUNDINGS, _menuText(_("Show ENC Soundings"), _T("S")) );
    view_menu->AppendCheckItem( ID_MENU_ENC_ANCHOR, _menuText(_("Show ENC Anchoring Info"), _T("A")) );
#else
    view_menu->AppendCheckItem( ID_MENU_ENC_TEXT, _menuText(_("Show ENC Text"), _T("Alt-T")) );
    view_menu->AppendCheckItem( ID_MENU_ENC_LIGHTS, _menuText(_("Show ENC Lights"), _T("Alt-L")) );
    view_menu->AppendCheckItem( ID_MENU_ENC_SOUNDINGS, _menuText(_("Show ENC Soundings"), _T("Alt-S")) );
    view_menu->AppendCheckItem( ID_MENU_ENC_ANCHOR, _menuText(_("Show ENC Anchoring Info"), _T("Alt-A")) );
    #endif
#endif
    view_menu->AppendSeparator();
    view_menu->AppendCheckItem( ID_MENU_SHOW_TIDES, _("Show Tides") );
    view_menu->AppendCheckItem( ID_MENU_SHOW_CURRENTS, _("Show Currents") );
    view_menu->AppendSeparator();
#ifndef __WXOSX__
    view_menu->Append( ID_MENU_UI_COLSCHEME, _menuText(_("Change Color Scheme"), _T("C")) );
#else
    view_menu->Append( ID_MENU_UI_COLSCHEME, _menuText(_("Change Color Scheme"), _T("Alt-C")) );
#endif

    view_menu->AppendSeparator();
#ifdef __WXOSX__
    view_menu->Append(ID_MENU_UI_FULLSCREEN, _menuText(_("Enter Full Screen"), _T("RawCtrl-Ctrl-F")) );
#else
    view_menu->Append(ID_MENU_UI_FULLSCREEN, _menuText(_("Enter Full Screen"), _T("F11")) );
#endif
    m_pMenuBar->Append( view_menu, _("&View") );


    wxMenu* ais_menu = new wxMenu();
    ais_menu->AppendCheckItem( ID_MENU_AIS_TARGETS, _("Show AIS Targets") );
    ais_menu->AppendCheckItem( ID_MENU_AIS_MOORED_TARGETS, _("Hide Moored AIS Targets") );
    ais_menu->AppendCheckItem( ID_MENU_AIS_SCALED_TARGETS, _("Attenuate Less Critical AIS Targets") );    
    ais_menu->AppendCheckItem( ID_MENU_AIS_TRACKS, _("Show AIS Target Tracks") );
    ais_menu->AppendCheckItem( ID_MENU_AIS_CPADIALOG, _("Show CPA Alert Dialogs") );
    ais_menu->AppendCheckItem( ID_MENU_AIS_CPASOUND, _("Sound CPA Alarms") );
    ais_menu->AppendSeparator();
    ais_menu->Append( ID_MENU_AIS_TARGETLIST, _("AIS Target List...") );
    m_pMenuBar->Append( ais_menu, _("&AIS") );

    wxMenu* tools_menu = new wxMenu();
#ifndef __WXOSX__
    tools_menu->Append( ID_MENU_TOOL_MEASURE, _menuText(_("Measure Distance"), _T("M")) );
#else
    tools_menu->Append( ID_MENU_TOOL_MEASURE, _menuText(_("Measure Distance"), _T("Alt-M")) );
#endif

    tools_menu->AppendSeparator();
    tools_menu->Append( ID_MENU_ROUTE_MANAGER, _("Route && Mark Manager...") );
    tools_menu->Append( ID_MENU_ROUTE_NEW, _menuText(_("Create Route"), _T("Ctrl-R")) );
    tools_menu->AppendSeparator();
    tools_menu->Append( ID_MENU_MARK_BOAT, _menuText(_("Drop Mark at Boat"), _T("Ctrl-O")) );
    tools_menu->Append( ID_MENU_MARK_CURSOR, _menuText(_("Drop Mark at Cursor"), _T("Ctrl-M")) );
    tools_menu->AppendSeparator();
#ifdef __WXOSX__
    tools_menu->Append( ID_MENU_MARK_MOB, _menuText(_("Drop MOB Marker"), _T("RawCtrl-Space")) ); // NOTE Cmd+Space is reserved for Spotlight
    tools_menu->AppendSeparator();
    tools_menu->Append( wxID_PREFERENCES, _menuText(_("Preferences..."), _T("Ctrl-,")) );
#else
    tools_menu->Append( ID_MENU_MARK_MOB, _menuText(_("Drop MOB Marker"), _T("Ctrl-Space")) );
    tools_menu->AppendSeparator();
    tools_menu->Append( wxID_PREFERENCES, _menuText(_("Options..."), _T("Ctrl-,")) );
#endif

    m_pMenuBar->Append( tools_menu, _("&Tools") );
    wxMenu* help_menu = new wxMenu();
    help_menu->Append( wxID_ABOUT, _("About OpenCPN") );
    help_menu->Append( wxID_HELP, _("OpenCPN Help") );
    m_pMenuBar->Append( help_menu, _("&Help") );


    // Set initial values for menu check items and radio items
    UpdateGlobalMenuItems();
}

void MyFrame::UpdateGlobalMenuItems()
{
    if ( !m_pMenuBar ) return;  // if there isn't a menu bar

    if ( pConfig ) m_pMenuBar->FindItem( ID_MENU_NAV_FOLLOW )->Check( pConfig->st_bFollow );
    m_pMenuBar->FindItem( ID_MENU_CHART_NORTHUP )->Check( !g_bCourseUp );
    m_pMenuBar->FindItem( ID_MENU_CHART_COGUP )->Check( g_bCourseUp );
    m_pMenuBar->FindItem( ID_MENU_NAV_TRACK )->Check( g_bTrackActive );
    m_pMenuBar->FindItem( ID_MENU_CHART_OUTLINES )->Check( g_bShowOutlines );
    m_pMenuBar->FindItem( ID_MENU_CHART_QUILTING )->Check( g_bQuiltEnable );
    m_pMenuBar->FindItem( ID_MENU_UI_CHARTBAR )->Check( g_bShowChartBar );
    m_pMenuBar->FindItem( ID_MENU_AIS_TARGETS )->Check( g_bShowAIS );
    m_pMenuBar->FindItem( ID_MENU_AIS_MOORED_TARGETS )->Check( g_bHideMoored );
    m_pMenuBar->FindItem( ID_MENU_AIS_SCALED_TARGETS )->Check( g_bShowScaled );
    m_pMenuBar->FindItem( ID_MENU_AIS_SCALED_TARGETS )->Enable(g_bAllowShowScaled);
    m_pMenuBar->FindItem( ID_MENU_AIS_TRACKS )->Check( g_bAISShowTracks );
    m_pMenuBar->FindItem( ID_MENU_AIS_CPADIALOG )->Check( g_bAIS_CPA_Alert );
    m_pMenuBar->FindItem( ID_MENU_AIS_CPASOUND )->Check( g_bAIS_CPA_Alert_Audio );
#ifdef USE_S57
    if( ps52plib ) {
        m_pMenuBar->FindItem( ID_MENU_ENC_TEXT )->Check( ps52plib->GetShowS57Text() );
        m_pMenuBar->FindItem( ID_MENU_ENC_SOUNDINGS )->Check( ps52plib->GetShowSoundings() );

        bool light_state = false;
        if( ps52plib ) {
            for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
                OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
                if( !strncmp( pOLE->OBJLName, "LIGHTS", 6 ) ) {
                    light_state = (pOLE->nViz == 1);
                    break;
                }
            }
        }
        m_pMenuBar->FindItem( ID_MENU_ENC_LIGHTS )->Check( (!ps52plib->IsObjNoshow("LIGHTS")) && light_state );

        m_pMenuBar->FindItem( ID_MENU_ENC_ANCHOR )->Check( !ps52plib->IsObjNoshow("SBDARE") );
    }
#endif
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

    if(g_bshowToolbar){
        if( g_FloatingToolbarDialog && g_FloatingToolbarDialog->IsToolbarShown() ) {
            if( IsFullScreen() ) {
                if( g_bFullscreenToolbar ) {
                    g_FloatingToolbarDialog->Surface();
                }
            } else{
                g_FloatingToolbarDialog->Surface();
            }
        }
#ifndef __WXQT__
       Raise();
#endif
    }
}

bool MyFrame::IsToolbarShown()
{
    return g_FloatingToolbarDialog->IsShown();
}

void MyFrame::ToggleToolbar( bool b_smooth )
{
    if( g_FloatingToolbarDialog ) {
        if( g_FloatingToolbarDialog->IsShown() ){
            SubmergeToolbar();
        }
        else{
            SurfaceToolbar();
            g_FloatingToolbarDialog->Raise();
        }
    }
}


void MyFrame::JumpToPosition( double lat, double lon, double scale )
{
    if (lon > 180.0)
        lon -= 360.0;
    vLat = lat;
    vLon = lon;
    cc1->StopMovement();
    cc1->m_bFollow = false;

    //  is the current chart available at the target location?
    int currently_selected_index = pCurrentStack->GetCurrentEntrydbIndex();

    //  If not, then select the smallest scale chart at the target location (may be empty)
    ChartData->BuildChartStack( pCurrentStack, lat, lon );
    if(!pCurrentStack->DoesStackContaindbIndex(currently_selected_index)){
        pCurrentStack->CurrentStackEntry = pCurrentStack->nEntry - 1;
        int selected_index = pCurrentStack->GetCurrentEntrydbIndex();
        if( cc1->GetQuiltMode() )
            cc1->SetQuiltRefChart( selected_index );
    }

    if( !cc1->GetQuiltMode() ) {
        cc1->SetViewPoint( lat, lon, scale, Current_Ch->GetChartSkew() * PI / 180., cc1->GetVPRotation() );
    } else {
        cc1->SetViewPoint( lat, lon, scale, 0, cc1->GetVPRotation() );
    }

    cc1->ReloadVP();

    SetToolbarItemState( ID_FOLLOW, false );

    if( g_pi_manager ) {
        g_pi_manager->SendViewPortToRequestingPlugIns( cc1->GetVP() );
    }
}

int MyFrame::DoOptionsDialog()
{
    if (g_boptionsactive)
        return 0;

    g_boptionsactive = true;

    if(NULL == g_options) {
        g_Platform->ShowBusySpinner();
        g_options = new options( this, -1, _("Options") );
    }
    g_Platform->HideBusySpinner();

//    Set initial Chart Dir
    g_options->SetInitChartDir( *pInit_Chart_Dir );

//      Pass two working pointers for Chart Dir Dialog
    g_options->SetCurrentDirList( ChartData->GetChartDirArray() );
    ArrayOfCDI *pWorkDirArray = new ArrayOfCDI;
    g_options->SetWorkDirListPtr( pWorkDirArray );

//      Pass a ptr to MyConfig, for updates
    g_options->SetConfigPtr( pConfig );

    g_options->SetInitialSettings();

    bDBUpdateInProgress = true;

    bPrevQuilt = g_bQuiltEnable;
    bPrevFullScreenQuilt = g_bFullScreenQuilt;
    bPrevOGL = g_bopengl;

    prev_locale = g_locale;

    bool b_sub = false;
    if( g_FloatingToolbarDialog && g_FloatingToolbarDialog->IsShown() ) {
        wxRect bx_rect = g_options->GetScreenRect();
        wxRect tb_rect = g_FloatingToolbarDialog->GetScreenRect();
        if( tb_rect.Intersects( bx_rect ) ) b_sub = true;

        if( b_sub ) g_FloatingToolbarDialog->Submerge();
    }

#if defined(__WXOSX__) || defined(__WXQT__)
    if(g_ChartBarWin) g_ChartBarWin->Hide();

    bool b_restoreAIS = false;
    if( g_pAISTargetList  && g_pAISTargetList->IsShown() ){
        b_restoreAIS = true;
        g_pAISTargetList->Shutdown();
        g_pAISTargetList = NULL;
    }
#endif

    g_options->SetInitialPage(options_lastPage );

    if(!g_bresponsive){
        g_options->lastWindowPos = options_lastWindowPos;
        if( options_lastWindowPos != wxPoint(0,0) ) {
            g_options->Move( options_lastWindowPos );
            g_options->SetSize( options_lastWindowSize );
        } else {
            g_options->Center();
        }
        if( options_lastWindowSize != wxSize(0,0) ) {
            g_options->SetSize( options_lastWindowSize );
        }
    }

    if( g_FloatingToolbarDialog)
        g_FloatingToolbarDialog->DisableTooltips();

    int rr = g_options->ShowModal();

    if( g_FloatingToolbarDialog)
        g_FloatingToolbarDialog->EnableTooltips();

    options_lastPage = g_options->lastPage;
    options_lastWindowPos = g_options->lastWindowPos;
    options_lastWindowSize = g_options->lastWindowSize;

    if( 1/*b_sub*/ ) {          // always surface toolbar, and restart the timer if needed
        SurfaceToolbar();
        cc1->SetFocus();
    }

#ifdef __WXGTK__
    Raise();                      // I dunno why...
#endif

    bool ret_val = false;
    rr = g_options->GetReturnCode();
    if( rr ) {
        ProcessOptionsDialog( rr,  g_options->GetWorkDirListPtr() );
        ret_val = true;
    }

    delete pWorkDirArray;

    ShowChartBarIfEnabled();

    gFrame->Raise();
    DoChartUpdate();
    UpdateControlBar();
    Refresh();

    //  We set the compass size first, since that establishes the available space for the toolbar.
    SetGPSCompassScale();
    g_Compass->SetScaleFactor(g_compass_scalefactor);
    UpdateGPSCompassStatusBox();

    SetToolbarScale();
    RequestNewToolbar();

    bDBUpdateInProgress = false;
    if( g_FloatingToolbarDialog ) {
        if( IsFullScreen() && !g_bFullscreenToolbar )
            g_FloatingToolbarDialog->Submerge();
    }

#if defined(__WXOSX__) || defined(__WXQT__)
    if( b_restoreAIS ){
        g_pAISTargetList = new AISTargetListDialog( this, g_pauimgr, g_pAIS );
        g_pAISTargetList->UpdateAISTargetList();
    }
#endif

    if(console && console->IsShown())
        console->Raise();

    Refresh( false );


    if (NMEALogWindow::Get().Active())
        NMEALogWindow::Get().GetTTYWindow()->Raise();

    //  Force reload of options dialog to pick up font changes
    if(rr & FONT_CHANGED){
        delete g_options;
        g_options = NULL;
        g_pOptions = NULL;
    }
    
    //  Pick up chart object icon size changes (g_ChartScaleFactorExp)
    if( pMarkPropDialog ) {
        pMarkPropDialog->Hide();
        pMarkPropDialog->Destroy();
        pMarkPropDialog = NULL;
    }
    

    g_boptionsactive = false;
    return ret_val;
}

int MyFrame::ProcessOptionsDialog( int rr, ArrayOfCDI *pNewDirArray )
{
    bool b_need_refresh = false;                // Do we need a full reload?

    //    Capture the name of the currently open chart
    wxString chart_file_name;
    if( cc1->GetQuiltMode() ) {
        int dbi = cc1->GetQuiltRefChartdbIndex();
        chart_file_name = ChartData->GetDBChartFileName( dbi );
    } else
        if( Current_Ch )
            chart_file_name = Current_Ch->GetFullPath();

    if( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) ) {
        if(pNewDirArray){
            UpdateChartDatabaseInplace( *pNewDirArray, ( ( rr & FORCE_UPDATE ) == FORCE_UPDATE ),
                true, ChartListFileName );

            b_need_refresh = true;
        }
    }

    if( ( rr & LOCALE_CHANGED ) || ( rr & STYLE_CHANGED ) ) {
        if( ( prev_locale != g_locale ) || ( rr & STYLE_CHANGED ) ) {
            OCPNMessageBox(NULL, _("Please restart OpenCPN to activate language or style changes."),
                    _("OpenCPN Info"), wxOK | wxICON_INFORMATION );
            if( rr & LOCALE_CHANGED ) g_blocale_changed = true;;
        }
    }

    bool b_groupchange = false;
    if( ( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) )
            || ( rr & GROUPS_CHANGED ) ) {
        b_groupchange = ScrubGroupArray();
        ChartData->ApplyGroupArray( g_pGroupArray );
        SetGroupIndex( g_GroupIndex );
    }

    if( rr & GROUPS_CHANGED || b_groupchange) {
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
        if( g_COGAvgSec > 0 ) {
            for( int i = 0; i < g_COGAvgSec; i++ )
                COGTable[i] = stuff;
        }

        g_COGAvg = stuff;

        DoCOGSet();
    }

    g_pRouteMan->SetColorScheme(global_color_scheme);           // reloads pens and brushes

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

    if(rr & GL_CHANGED){
        //    Refresh the chart display, after flushing cache.
        //      This will allow all charts to recognise new OpenGL configuration, if any
        b_need_refresh = true;
    }

    if(rr & S52_CHANGED){
        b_need_refresh = true;
    }

#ifdef ocpnUSE_GL
    if(rr & REBUILD_RASTER_CACHE){
        cc1->Disable();
        BuildCompressedCache();
        cc1->Enable();
    }
#endif

    if(g_config_display_size_mm > 0){
        g_display_size_mm = g_config_display_size_mm;
    }
    else{
        g_display_size_mm = wxMax(100, g_Platform->GetDisplaySizeMM());
    }

    cc1->SetDisplaySizeMM( g_display_size_mm );

    if(g_FloatingToolbarDialog){
        g_FloatingToolbarDialog->SetAutoHide(g_bAutoHideToolbar);
        g_FloatingToolbarDialog->SetAutoHideTimer(g_nAutoHideToolbar);
    }

    //    Do a full Refresh, trying to open the last open chart
    if(b_need_refresh){
        int index_hint = ChartData->FinddbIndex( chart_file_name );
        if( -1 == index_hint )
            b_autofind = true;
        ChartsRefresh( index_hint, cc1->GetVP() );
    }
    
    //  The zoom-scale factor may have changed
    //  so, trigger a recalculation of the reference chart
    cc1->DoZoomCanvas(1.0001);

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

    if( !pGroup->m_element_array.GetCount() )   //  truly empty group is OK
        return true;

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

bool MyFrame::ScrubGroupArray()
{
    //    For each group,
    //    make sure that each group element (dir or chart) references at least oneitem in the database.
    //    If not, remove the element.

    bool b_change = false;
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
                j--;
                delete pelement;
                b_change = true;
            }
        }

        igroup++;                                 // next group
    }

    return b_change;
}

// Flav: This method reloads all charts for convenience
void MyFrame::ChartsRefresh( int dbi_hint, ViewPort &vp, bool b_purge )
{
    if( !ChartData ) return;

    OCPNPlatform::ShowBusySpinner();

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
                /* Current_Ch is always NULL here, (set above) should this go before that? */
                if( Current_Ch ) Current_Ch->Deactivate();

                Current_Ch = pTentative_Chart;
                Current_Ch->Activate();

                pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                        Current_Ch->GetFullPath() );
            } else
                SetChartThumbnail( dbi_hint );       // need to reset thumbnail on failed chart open
        }

        //          Refresh the Piano Bar
        ArrayOfInts piano_active_chart_index_array;
        piano_active_chart_index_array.Add( pCurrentStack->GetCurrentEntrydbIndex() );
        g_Piano->SetActiveKeyArray( piano_active_chart_index_array );

        if( g_ChartBarWin )
            g_ChartBarWin->Refresh( true );

    } else {
        //    Select reference chart from the stack, as though clicked by user
        //    Make it the smallest scale chart on the stack
        pCurrentStack->CurrentStackEntry = pCurrentStack->nEntry - 1;
        int selected_index = pCurrentStack->GetCurrentEntrydbIndex();
        cc1->SetQuiltRefChart( selected_index );
    }

    //    Validate the correct single chart, or set the quilt mode as appropriate
    SetupQuiltMode();

    cc1->ReloadVP();

    UpdateControlBar();

    UpdateGPSCompassStatusBox( );

    cc1->SetCursor( wxCURSOR_ARROW );

    if( b_run ) FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

    OCPNPlatform::HideBusySpinner();

}

bool MyFrame::UpdateChartDatabaseInplace( ArrayOfCDI &DirArray, bool b_force, bool b_prog,
        const wxString &ChartListFileName )
{
    bool b_run = FrameTimer1.IsRunning();
    FrameTimer1.Stop();                  // stop other asynchronous activity

    cc1->InvalidateQuilt();
    cc1->SetQuiltRefChart( -1 );
    ChartData->PurgeCache();

    Current_Ch = NULL;

    delete pCurrentStack;
    pCurrentStack = NULL;

    OCPNPlatform::ShowBusySpinner();

    wxProgressDialog *pprog = NULL;
    if( b_prog ) {
        wxString longmsg = _("OpenCPN Chart Update");
        longmsg += _T("..........................................................................");
        pprog = new wxProgressDialog( _("OpenCPN Chart Update"), longmsg,
                100, this,
                wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );
    }


    wxLogMessage( _T("   ") );
    wxLogMessage( _T("Starting chart database Update...") );
    ChartData->Update( DirArray, b_force, pprog );
    ChartData->SaveBinary(ChartListFileName);
    wxLogMessage( _T("Finished chart database Update") );
    wxLogMessage( _T("   ") );

    delete pprog;

    OCPNPlatform::HideBusySpinner();

    pConfig->UpdateChartDirs( DirArray );

    if( b_run ) FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

    return true;
}

void MyFrame::ToggleQuiltMode( void )
{
    if( cc1 ) {
        bool cur_mode = cc1->GetQuiltMode();

        if( !cc1->GetQuiltMode() )
            cc1->SetQuiltMode( true );
        else
            if( cc1->GetQuiltMode() ) {
                cc1->SetQuiltMode( false );
                g_sticky_chart = cc1->GetQuiltReferenceChartIndex();
            }


        if( cur_mode != cc1->GetQuiltMode() ){
            SetupQuiltMode();
            DoChartUpdate();
            cc1->InvalidateGL();
            Refresh();
        }
        g_bQuiltEnable = cc1->GetQuiltMode();
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

        g_Piano->SetNoshowIndexArray( g_quilt_noshow_index_array );

        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

        g_Piano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
        g_Piano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );
        g_Piano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
        g_Piano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

        g_Piano->SetRoundedRectangles( true );

        //    Select the proper Ref chart
        int target_new_dbindex = -1;
        if( pCurrentStack ) {
            target_new_dbindex = cc1->GetQuiltReferenceChartIndex();    //pCurrentStack->GetCurrentEntrydbIndex();
            
            if(-1 != target_new_dbindex){
                if( !cc1->IsChartQuiltableRef( target_new_dbindex ) ){

                    int proj = ChartData->GetDBChartProj(target_new_dbindex);
                    int type = ChartData->GetDBChartType(target_new_dbindex);

                // walk the stack up looking for a satisfactory chart
                    int stack_index = pCurrentStack->CurrentStackEntry;

                    while((stack_index < pCurrentStack->nEntry-1) && (stack_index >= 0)) {
                        int proj_tent = ChartData->GetDBChartProj( pCurrentStack->GetDBIndex(stack_index));
                        int type_tent = ChartData->GetDBChartType( pCurrentStack->GetDBIndex(stack_index));

                        if(cc1->IsChartQuiltableRef(pCurrentStack->GetDBIndex(stack_index))){
                            if((proj == proj_tent) && (type_tent == type)){
                                target_new_dbindex = pCurrentStack->GetDBIndex(stack_index);
                                break;
                            }
                        }
                        stack_index++;
                    }
                }
            }
        }

        if( cc1->IsChartQuiltableRef( target_new_dbindex ) )
            SelectQuiltRefdbChart( target_new_dbindex, false );        // Try not to allow a scale change
        else
            SelectQuiltRefdbChart( -1 );

        Current_Ch = NULL;                  // Bye....

        SetChartThumbnail( -1 );            //Turn off thumbnails for sure

        //  Re-qualify the quilt reference chart selection
        cc1->AdjustQuiltRefChart(  );
       
        //  Restore projection type saved on last quilt mode toggle
        if(g_sticky_projection != -1)
            cc1->GetVP().SetProjectionType(g_sticky_projection);
        else
            cc1->GetVP().SetProjectionType(PROJECTION_MERCATOR);
        
        

    } else                                                  // going to SC Mode
    {
        ArrayOfInts empty_array;
        g_Piano->SetActiveKeyArray( empty_array );
        g_Piano->SetNoshowIndexArray( empty_array );
        g_Piano->SetEclipsedIndexArray( empty_array );

        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        g_Piano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
        g_Piano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );
        g_Piano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
        g_Piano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

        g_Piano->SetRoundedRectangles( false );
        g_sticky_projection = cc1->GetVP().m_projection_type;

    }

    //    When shifting from quilt to single chart mode, select the "best" single chart to show
    if( !cc1->GetQuiltMode() ) {
        if( ChartData && ChartData->IsValid() ) {
            cc1->UnlockQuilt();

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
                g_Piano->SetActiveKeyArray( one_array );
            }
            
            if( Current_Ch ) {
                cc1->GetVP().SetProjectionType(Current_Ch->GetChartProjectionType());
            }

        }
        //    Invalidate the current stack so that it will be rebuilt on next tick
        if( pCurrentStack ) pCurrentStack->b_valid = false;
    }

}

void MyFrame::ClearRouteTool()
{
    if( g_toolbar )
        g_toolbar->ToggleTool( ID_ROUTE, false );

#ifdef __OCPN__ANDROID__
        androidSetRouteAnnunciator(false);
#endif
}

void MyFrame::DoStackDown( void )
{
    DoStackDelta( -1 );
}

void MyFrame::DoStackUp( void )
{
    DoStackDelta( 1 );
}

void MyFrame::DoStackDelta( int direction )
{
    if( !cc1->GetQuiltMode() ) {
        int current_stack_index = pCurrentStack->CurrentStackEntry;
        if( (current_stack_index + direction) >= pCurrentStack->nEntry )
            return;
        if( (current_stack_index + direction) < 0 )
            return;

        if( m_bpersistent_quilt /*&& g_bQuiltEnable*/ ) {
            int new_dbIndex = pCurrentStack->GetDBIndex(current_stack_index + direction );

            if( cc1->IsChartQuiltableRef( new_dbIndex ) ) {
                ToggleQuiltMode();
                SelectQuiltRefdbChart( new_dbIndex );
                m_bpersistent_quilt = false;
            }
        }
        else {
            SelectChartFromStack( current_stack_index + direction );
        }
    } else {
        ArrayOfInts piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();
        int refdb = cc1->GetQuiltRefChartdbIndex();

        //      Find the ref chart in the stack
        int current_index = -1;
        for(unsigned int i=0 ; i < piano_chart_index_array.Count() ; i++){
            if(refdb == piano_chart_index_array.Item( i )){
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
        while(!found && (unsigned int)check_index < piano_chart_index_array.Count() && (check_index >= 0)){
            check_dbIndex = piano_chart_index_array.Item( check_index );
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


        if( !cc1->IsChartQuiltableRef( new_dbIndex ) ) {
            ToggleQuiltMode();
            SelectdbChart( new_dbIndex );
            m_bpersistent_quilt = true;
        } else {
            SelectQuiltRefChart( new_index );
        }
    }

    UpdateGlobalMenuItems(); // update the state of the menu items (checkmarks etc)
    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();
}

// Defered initialization for anything that is not required to render the initial frame
// and takes a while to initialize.  This gets opencpn up and running much faster.
void MyFrame::OnInitTimer(wxTimerEvent& event)
{
    switch(m_iInitCount++) {
        case 0:
        {
            if( g_toolbar )
                g_toolbar->EnableTool( ID_SETTINGS, false );
            
            // Set persistent Fullscreen mode
            g_Platform->SetFullscreen(g_bFullscreen);
            
            // Load the waypoints.. both of these routines are very slow to execute which is why
            // they have been to defered until here
            pWayPointMan = new WayPointman();
            
            // Reload the ownship icon from UserIcons, if present
            if(cc1->SetUserOwnship())
                cc1->SetColorScheme(global_color_scheme);
            
            pConfig->LoadNavObjects();

            //    Re-enable anchor watches if set in config file
            if( !g_AW1GUID.IsEmpty() ) {
                pAnchorWatchPoint1 = pWayPointMan->FindRoutePointByGUID( g_AW1GUID );
            }
            if( !g_AW2GUID.IsEmpty() ) {
                pAnchorWatchPoint2 = pWayPointMan->FindRoutePointByGUID( g_AW2GUID );
            }

            // Import Layer-wise any .gpx files from /Layers directory
            wxString layerdir = g_Platform->GetPrivateDataDir();
            appendOSDirSlash( &layerdir );
            layerdir.Append( _T("layers") );

            if( wxDir::Exists( layerdir ) ) {
                wxString laymsg;
                laymsg.Printf( wxT("Getting .gpx layer files from: %s"), layerdir.c_str() );
                wxLogMessage( laymsg );

                pConfig->LoadLayers(layerdir);
            }

            break;
        }
        case 1:
            // Connect Datastreams


            for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
            {
                ConnectionParams *cp = g_pConnectionParams->Item(i);
                if( cp->bEnabled ) {

    #ifdef __unix__
                    if( cp->GetDSPort().Contains(_T("Serial"))) {
                        if( ! g_bserial_access_checked ){
                            if( !CheckSerialAccess() ){
                            }
                            g_bserial_access_checked = true;
                        }
                    }
    #endif

                    dsPortType port_type = cp->IOSelect;
                    DataStream *dstr = new DataStream( g_pMUX,
                                                    cp->Type,
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

                    cp->b_IsSetup = true;

                    g_pMUX->AddStream(dstr);
                }
            }

            console = new ConsoleCanvas( gFrame );                    // the console
            console->SetColorScheme( global_color_scheme );
            break;

        case 2:
        {
            if (m_initializing)
                break;
            m_initializing = true;
            g_pi_manager->LoadAllPlugIns( g_Platform->GetPluginDir(), true, false );

            RequestNewToolbar();
            if( g_toolbar )
                g_toolbar->EnableTool( ID_SETTINGS, false );
            
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

            if( !bno_load )
                g_pauimgr->LoadPerspective( perspective, false );

            g_pauimgr->Update();

            //   Notify all the AUI PlugIns so that they may syncronize with the Perspective
            g_pi_manager->NotifyAuiPlugIns();
            g_pi_manager->ShowDeferredBlacklistMessages(); //  Give the use dialog on any blacklisted PlugIns
            g_pi_manager->CallLateInit();
            
            //  If any PlugIn implements PlugIn Charts, we need to re-run the initial chart load logic
            //  to select the correct chart as saved from the last run of the app.
            //  This will be triggered at the next DoChartUpdate()
            if( g_pi_manager->IsAnyPlugInChartEnabled() ){
                bFirstAuto = true;
                b_reloadForPlugins = true;
            }
                
            break;
        }

        case 3:
        {
            if(g_FloatingToolbarDialog){
                g_FloatingToolbarDialog->SetAutoHide(g_bAutoHideToolbar);
                g_FloatingToolbarDialog->SetAutoHideTimer(g_nAutoHideToolbar);
            }
            
            break;
        }

        case 4:
        {
            g_options = new options( this, -1, _("Options") );
    
            if( g_toolbar )
                g_toolbar->EnableTool( ID_SETTINGS, true );
            
            break;
        }

        default:
        {
            // Last call....

            InitTimer.Stop(); // Initialization complete
            g_bDeferredInitDone = true;
            
            if(b_reloadForPlugins)
                ChartsRefresh(g_restore_dbindex, cc1->GetVP());            
            break;
        }
    }   // switch
    cc1->Refresh( true );
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

// play an arbitrary number of bells by using 1 and 2 bell sounds
void MyFrame::OnBellsTimer(wxTimerEvent& event)
{
    int bells = wxMin(m_BellsToPlay, 2);

    if(bells <= 0)
        return;

    if( !bells_sound[bells - 1].IsOk() )            // load the bells sound
    {
        wxString soundfile = _T("sounds");
        appendOSDirSlash( &soundfile );
        soundfile += wxString( bells_sound_file_name[bells - 1], wxConvUTF8 );
        soundfile.Prepend( g_Platform->GetSharedDataDir() );
        bells_sound[bells - 1].Create( soundfile );
        if( !bells_sound[bells - 1].IsOk() ) {
            wxLogMessage( _T("Failed to load bells sound file: ") + soundfile );
            return;
        }

        wxLogMessage( _T("Using bells sound file: ") + soundfile );
    }

    bells_sound[bells - 1].Play();
    m_BellsToPlay -= bells;

    BellsTimer.Start(2000, wxTIMER_ONE_SHOT);
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

        AppActivateList.Clear();
        if(cc1){
            for ( wxWindowList::iterator it = cc1->GetChildren().begin(); it != cc1->GetChildren().end(); ++it ) {
                if( (*it)->IsShown() ) {
                    (*it)->Hide();
                    AppActivateList.Append(*it);
                }
            }
        }

        if(gFrame){
            for ( wxWindowList::iterator it = gFrame->GetChildren().begin(); it != gFrame->GetChildren().end(); ++it ) {
                if( (*it)->IsShown() ) {
                    if( !(*it)->IsKindOf( CLASSINFO(ChartCanvas) ) ) {
                        (*it)->Hide();
                        AppActivateList.Append(*it);
                    }
                }
            }
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

    //  If tracking carryover was found in config file, enable tracking as soon as
    //  GPS become valid
    if(g_bDeferredStartTrack){
        if(!g_bTrackActive){
            if(bGPSValid){
                gFrame->TrackOn();
                g_bDeferredStartTrack = false;
            }
        }
        else {                                  // tracking has been manually activated
            g_bDeferredStartTrack = false;
        }
    }

//  Update and check watchdog timer for GPS data source
    gGPS_Watchdog--;
    if( gGPS_Watchdog <= 0 ) {
        bGPSValid = false;
        if( gGPS_Watchdog == 0  ){
            wxString msg;
            msg.Printf( _T("   ***GPS Watchdog timeout at Lat:%g   Lon: %g"), gLat, gLon );
            wxLogMessage(msg);
        }
        gSog = NAN;
        gCog = NAN;
    }

//  Update and check watchdog timer for Mag Heading data source
    gHDx_Watchdog--;
    if( gHDx_Watchdog <= 0 ) {
        gHdm = NAN;
        if( g_nNMEADebug && ( gHDx_Watchdog == 0 ) ) wxLogMessage(
                _T("   ***HDx Watchdog timeout...") );
    }

//  Update and check watchdog timer for True Heading data source
    gHDT_Watchdog--;
    if( gHDT_Watchdog <= 0 ) {
        g_bHDT_Rx = false;
        gHdt = NAN;
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
        g_SatsInView = 0;
        if( g_nNMEADebug && ( gSAT_Watchdog == 0 ) ) wxLogMessage(
                _T("   ***SAT Watchdog timeout...") );
    }

    //    Build and send a Position Fix event to PlugIns
    if( g_pi_manager )
    {
        GenericPosDatEx GPSData;
        GPSData.kLat = gLat;
        GPSData.kLon = gLon;
        GPSData.kCog = gCog;
        GPSData.kSog = gSog;
        GPSData.kVar = gVar;
        GPSData.kHdm = gHdm;
        GPSData.kHdt = gHdt;
        GPSData.nSats = g_SatsInView;

        GPSData.FixTime = m_fixtime;

        g_pi_manager->SendPositionFixToAllPlugIns( &GPSData );
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

        if( tooclose || toofar )
            AnchorAlertOn1 = true;
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

    if( (pAnchorWatchPoint1 || pAnchorWatchPoint2) && !bGPSValid )
        AnchorAlertOn1 = true;

//  Send current nav status data to log file on every half hour   // pjotrc 2010.02.09

    wxDateTime lognow = wxDateTime::Now();   // pjotrc 2010.02.09
    int hourLOC = lognow.GetHour();
    int minuteLOC = lognow.GetMinute();
    lognow.MakeGMT();
    int minuteUTC = lognow.GetMinute();
    int second = lognow.GetSecond();

    wxTimeSpan logspan = lognow.Subtract( g_loglast_time );
    if( ( logspan.IsLongerThan( wxTimeSpan( 0, 30, 0, 0 ) ) ) || ( minuteUTC == 0 )
            || ( minuteUTC == 30 ) ) {
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
                    sog.Printf( _T("SOG %6.2f ") + getUsrSpeedUnit(), toUsrSpeed( gSog ) );

                navmsg += cog;
                navmsg += sog;
            } else {
                wxString data;
                data.Printf( _T(" DR Lat %10.5f Lon %10.5f"), gLat, gLon );
                navmsg += data;
            }
            wxLogMessage( navmsg );
            g_loglast_time = lognow;

            int bells = ( hourLOC % 4 ) * 2;     // 2 bells each hour
            if( minuteLOC != 0 ) bells++;       // + 1 bell on 30 minutes
            if( !bells ) bells = 8;     // 0 is 8 bells

            if( g_bPlayShipsBells && ( ( minuteLOC == 0 ) || ( minuteLOC == 30 ) ) ) {
                m_BellsToPlay = bells;
                BellsTimer.Start(5, wxTIMER_ONE_SHOT);
            }
        }
    }
    
    if( ShouldRestartTrack() )
        TrackDailyRestart();

    if(g_bSleep){
        FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );
        return;
    }

//      Update the Toolbar Status windows and lower status bar the first time watchdog times out
    if( ( gGPS_Watchdog == 0 ) || ( gSAT_Watchdog == 0 ) ) {
        wxString sogcog( _T("SOG --- ") + getUsrSpeedUnit() + + _T("     ") + _T(" COG ---\u00B0") );
        if( GetStatusBar() ) SetStatusText( sogcog, STAT_FIELD_SOGCOG );

        gCog = 0.0;                                 // say speed is zero to kill ownship predictor
    }

#if !defined(__WXGTK__) && !defined(__WXQT__)
    {
        double cursor_lat, cursor_lon;
        cc1->GetCursorLatLon( &cursor_lat, &cursor_lon );
        cc1->SetCursorStatus(cursor_lat, cursor_lon);
    }
#endif
//      Update the chart database and displayed chart
    bool bnew_view = false;

//    Do the chart update based on the global update period currently set
//    If in COG UP mode, the chart update is handled by COG Update timer
    if( !g_bCourseUp && ( 0 == m_ChartUpdatePeriod-- ) ) {
        bnew_view = DoChartUpdate();
        m_ChartUpdatePeriod = g_ChartUpdatePeriod;
    }

    nBlinkerTick++;
    cc1->DrawBlinkObjects();

//      Update the active route, if any
    if( g_pRouteMan->UpdateProgress() ) {
        //    This RefreshRect will cause any active routepoint to blink
        if( g_pRouteMan->GetpActiveRoute() ) cc1->RefreshRect( g_blink_rect, false );
    }
#if 0 // too slow, my computer hiccups, this takes nearly a second on some machines.
//  Instead we should save the current configuration only when it needs to be saved.
    if( 0 == ( g_tick % ( g_nautosave_interval_seconds ) ) ) {
        pConfig->UpdateSettings();
        pConfig->UpdateNavObj();
    }
#endif

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

    if( !bGPSValid )
        cc1->SetOwnShipState( SHIP_INVALID );

    if( bGPSValid != m_last_bGPSValid ) {
        if(!g_bopengl)
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

        if( brq_dynamic )
            bnew_view = true;
    }

    FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

    //  Make sure we get a redraw and alert sound on AnchorWatch excursions.
    if(AnchorAlertOn1 || AnchorAlertOn2)
        bnew_view = true;
    
    if(g_bopengl) {
#ifdef ocpnUSE_GL
        if(m_fixtime - cc1->GetglCanvas()->m_last_render_time > 0)
            bnew_view = true;

        if( AnyAISTargetsOnscreen( cc1->GetVP() ) )
            bnew_view = true;

        if(bnew_view) /* full frame in opengl mode */
            cc1->Refresh(false);
#endif
    } else {
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

double MyFrame::GetTrueOrMag(double a)
{
    if( g_bShowMag ){
        if(!wxIsNaN(gVar)){
            if((a - gVar) >360.)
                return (a - gVar - 360.);
            else
                return ((a - gVar) >= 0.) ? (a - gVar) : (a - gVar + 360.);
        }
        else{
            if((a - g_UserVar) >360.)
                return (a - g_UserVar - 360.);
            else
                return ((a - g_UserVar) >= 0.) ? (a - g_UserVar) : (a - g_UserVar + 360.);
        }
    }
    else
        return a;
}

double MyFrame::GetTrueOrMag(double a, double lat, double lon)
{
    if( g_bShowMag ){
        if(g_pi_manager && g_pi_manager->IsPlugInAvailable(_T("WMM"))){
            
            // Request variation at a specific lat/lon
            
            // Note that the requested value is returned sometime later in the event stream,
            // so there may be invalid data returned on the first call to this method.
            // In the case of rollover windows, the value is requested continuously, so will be correct very soon.
            wxDateTime now = wxDateTime::Now();
            SendJSON_WMM_Var_Request(lat, lon, now);
            
            if((a - gQueryVar) >360.)
                return (a - gQueryVar - 360.);
            else
                return ((a - gQueryVar) >= 0.) ? (a - gQueryVar) : (a - gQueryVar + 360.);
        }
        else if(!wxIsNaN(gVar)){
            if((a - gVar) >360.)
                return (a - gVar - 360.);
            else
                return ((a - gVar) >= 0.) ? (a - gVar) : (a - gVar + 360.);
        }
        else{
            if((a - g_UserVar) >360.)
                return (a - g_UserVar - 360.);
            else
                return ((a - g_UserVar) >= 0.) ? (a - g_UserVar) : (a - g_UserVar + 360.);
        }
    }
    else
        return a;
}

bool MyFrame::SendJSON_WMM_Var_Request(double lat, double lon, wxDateTime date)
{
    if(g_pi_manager){
        wxJSONValue v;
        v[_T("Lat")] = lat;
        v[_T("Lon")] = lon;
        v[_T("Year")] = date.GetYear();
        v[_T("Month")] = date.GetMonth();
        v[_T("Day")] = date.GetDay();
    
        g_pi_manager->SendJSONMessageToAllPlugins(_T("WMM_VARIATION_REQUEST"), v);
        return true;
    }
    else
        return false;
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
            if( !g_bShowAIS ) iconName = _T("AIS_Disabled");

            if( m_lastAISiconName != iconName ) {
                if( g_toolbar) {
                    g_toolbar->SetToolNormalBitmapEx( m_pAISTool, iconName );
                    g_toolbar->Refresh();
                    m_lastAISiconName = iconName;
                }
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
        if( g_pAIS->IsAISSuppressed() )
            iconName = _T("AIS_Suppressed");
        if( g_pAIS->IsAISAlertGeneral() )
            iconName = _T("AIS_AlertGeneral");
        if( !g_bShowAIS )
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
                if( !g_bShowAIS )
                    iconName = _T("AIS_Disabled");
            }
        }

        if( ( m_lastAISiconName != iconName ) ) b_update = true;

        if( b_update && g_toolbar) {
            g_toolbar->SetToolNormalBitmapEx( m_pAISTool, iconName );
            g_toolbar->Refresh();
            m_lastAISiconName = iconName;
        }

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
    DoCOGSet();

    if(!g_bCourseUp)
        return;

    //    Restart the timer, max frequency is 10 hz.
    int period_ms = 100;
    if( g_COGAvgSec > 0 )
        period_ms = g_COGAvgSec * 1000;
    FrameCOGTimer.Start( period_ms, wxTIMER_CONTINUOUS );
}

void MyFrame::DoCOGSet( void )
{
    if( !g_bCourseUp )
        return;

    if(!cc1)
        return;

    double old_VPRotate = g_VPRotate;
    g_VPRotate = -g_COGAvg * PI / 180.;

    cc1->SetVPRotation( g_VPRotate );
    bool bnew_chart = DoChartUpdate();

    if( ( bnew_chart ) || ( old_VPRotate != g_VPRotate ) )
        cc1->ReloadVP();
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

void MyFrame::UpdateRotationState( double rotation )
{
    //  If rotated manually, we switch to NORTHUP
    g_bCourseUp = false;

    if(fabs(rotation) > .001){
        SetMenubarItemState( ID_MENU_CHART_COGUP, false );
        SetMenubarItemState( ID_MENU_CHART_NORTHUP, true );
        if(m_pMenuBar){
            m_pMenuBar->SetLabel( ID_MENU_CHART_NORTHUP, _("Rotated Mode") );
        }
    }
    else{
        SetMenubarItemState( ID_MENU_CHART_COGUP, g_bCourseUp );
        SetMenubarItemState( ID_MENU_CHART_NORTHUP, !g_bCourseUp );
        if(m_pMenuBar){
            m_pMenuBar->SetLabel( ID_MENU_CHART_NORTHUP, _("North Up Mode") );
        }
    }

    UpdateGPSCompassStatusBox( true );
    DoChartUpdate();
    cc1->ReloadVP();
}


void MyFrame::UpdateGPSCompassStatusBox( bool b_force_new )
{
   //    Look for change in overlap or positions
    bool b_update = false;
    int cc1_edge_comp = 2;

    if( g_FloatingToolbarDialog ) {
        wxRect rect = g_Compass->GetRect();
        wxSize parent_size = cc1->GetSize();

        // check to see if it would overlap if it was in its home position (upper right)
        wxPoint tentative_pt(parent_size.x - rect.width - cc1_edge_comp, g_StyleManager->GetCurrentStyle()->GetCompassYOffset());
        wxRect tentative_rect( tentative_pt, rect.GetSize() );

        //  If the toolbar location has changed, or the proposed compassDialog location has changed
        if( g_FloatingToolbarDialog->GetScreenRect() != g_last_tb_rect || b_force_new) {

            wxRect tb_rect = g_FloatingToolbarDialog->GetScreenRect();
            wxPoint tentative_pt_in_screen(cc1->ClientToScreen(tentative_pt));
            wxRect tentative_rect_in_screen(tentative_pt_in_screen.x, tentative_pt_in_screen.y,
                                            rect.width, rect.height);

            //    if they would not intersect, go ahead and move it to the upper right
            //      Else it has to be on lower right
            if( !tb_rect.Intersects( tentative_rect_in_screen ) )
                g_Compass->Move( tentative_pt );
            else
                g_Compass->Move( wxPoint( cc1->GetSize().x - rect.width - cc1_edge_comp,
                                          cc1->GetSize().y - ( rect.height + cc1_edge_comp ) ) );

            if(rect != g_Compass->GetRect()) {
                Refresh(true);
                cc1->m_brepaint_piano = true;
                b_update = true;
            }
            g_last_tb_rect = tb_rect;

        }
    }

    if( g_Compass && g_Compass->IsShown())
        g_Compass->UpdateStatus( b_force_new | b_update );
}

int MyFrame::GetnChartStack( void )
{
    if(pCurrentStack)
        return pCurrentStack->nEntry;
    else
        return 0;
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

    // stop movement or on slow computer we may get something like :
    // zoom out with the wheel (timer is set)
    // quickly click and display a chart, which may zoom in
    // but the delayed timer fires first and it zooms out again!
    cc1->StopMovement();

    if( !cc1->GetQuiltMode() ) {
        if( m_bpersistent_quilt/* && g_bQuiltEnable*/ ) {
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

        if( Current_Ch )
            cc1->GetVP().SetProjectionType(Current_Ch->GetChartProjectionType());
        
    } else {
        if( cc1->IsChartQuiltableRef( selected_dbIndex ) ){
            if( ChartData ) ChartData->PurgeCache();


            //  If the chart is a vector chart, and of very large scale,
            //  then we had better set the new scale directly to avoid excessive underzoom
            //  on, eg, Inland ENCs
            bool set_scale = false;
            if(ChartData){
                if( CHART_TYPE_S57 == ChartData->GetDBChartType( selected_dbIndex ) ){
                    if( ChartData->GetDBChartScale(selected_dbIndex) < 5000){
                        set_scale = true;
                    }
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
                    double proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / cc1->GetVPScale();

                    if(g_bPreserveScaleOnX){
                        proposed_scale_onscreen = wxMin(proposed_scale_onscreen,
                                                100 * pc->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));
                    }
                    else{
                        proposed_scale_onscreen = wxMin(proposed_scale_onscreen,
                                                        20 * pc->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));

                        proposed_scale_onscreen = wxMax(proposed_scale_onscreen,
                                                pc->GetNormalScaleMin(cc1->GetCanvasScaleFactor(), g_b_overzoom_x));
                    }

                    cc1->SetVPScale( cc1->GetCanvasScaleFactor() / proposed_scale_onscreen );
                }
            }
        }
        else {
            ToggleQuiltMode();
            SelectdbChart( selected_dbIndex );
            m_bpersistent_quilt = true;
        }
    }

    cc1->SetQuiltChartHiLiteIndex( -1 );
    UpdateGlobalMenuItems(); // update the state of the menu items (checkmarks etc)
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

    if(ChartData && ChartData->IsBusy())
        return;

    wxPoint key_location = g_Piano->GetKeyOrigin( selected_index );

    if( !cc1->GetQuiltMode() ) {
        SetChartThumbnail( selected_index );
        cc1->ShowChartInfoWindow( key_location.x, selected_dbIndex );
    } else {
        ArrayOfInts piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();

        if( ( pCurrentStack->nEntry > 1 ) || ( piano_chart_index_array.GetCount() > 1 ) ) {
            cc1->ShowChartInfoWindow( key_location.x, selected_dbIndex );
            cc1->SetQuiltChartHiLiteIndex( selected_dbIndex );

            cc1->ReloadVP( false );         // no VP adjustment allowed
        } else if( pCurrentStack->nEntry == 1 ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry(
                    pCurrentStack->GetDBIndex( 0 ) );
            if( CHART_TYPE_CM93COMP != cte.GetChartType() ) {
                cc1->ShowChartInfoWindow( key_location.x, selected_dbIndex );
                cc1->ReloadVP( false );
            } else if( ( -1 == selected_index ) && ( -1 == selected_dbIndex ) ) {
                cc1->ShowChartInfoWindow( key_location.x, selected_dbIndex );
            }
        }
        SetChartThumbnail( -1 );        // hide all thumbs in quilt mode
    }
}

double MyFrame::GetBestVPScale( ChartBase *pchart )
{
    if( pchart ) {
        double proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / cc1->GetVPScale();

        if( ( g_bPreserveScaleOnX ) || ( CHART_TYPE_CM93COMP == pchart->GetChartType() ) ) {
            double new_scale_ppm = cc1->GetVPScale();
            proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
        } else {
            //  This logic will bring the new chart onscreen at roughly twice the true paper scale equivalent.
            proposed_scale_onscreen = pchart->GetNativeScale() / 2;
            double equivalent_vp_scale = cc1->GetCanvasScaleFactor() / proposed_scale_onscreen;
            double new_scale_ppm = pchart->GetNearestPreferredScalePPM( equivalent_vp_scale );
            proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
        }

        // Do not allow excessive underzoom, even if the g_bPreserveScaleOnX flag is set.
        // Otherwise, we get severe performance problems on all platforms

        double max_underzoom_multiplier = 2.0;
        proposed_scale_onscreen =
               wxMin(proposed_scale_onscreen,
                     pchart->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()) *
                     max_underzoom_multiplier);

        //  And, do not allow excessive overzoom either
        proposed_scale_onscreen =
                wxMax(proposed_scale_onscreen, pchart->GetNormalScaleMin(cc1->GetCanvasScaleFactor(), false));

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

void MyFrame::SelectQuiltRefdbChart( int db_index, bool b_autoscale )
{
    if( pCurrentStack ) pCurrentStack->SetCurrentEntryFromdbIndex( db_index );

    cc1->SetQuiltRefChart( db_index );

    ChartBase *pc = ChartData->OpenChartFromDB( db_index, FULL_INIT );
    if( pc ) {
        if(b_autoscale) {
            double best_scale = GetBestVPScale( pc );
            cc1->SetVPScale( best_scale );
        }
    }
    else
        cc1->SetQuiltRefChart( -1 );


}

void MyFrame::SelectChartFromStack( int index, bool bDir, ChartTypeEnum New_Type,
        ChartFamilyEnum New_Family )
{
    if( !pCurrentStack )
        return;

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
        double rotation = cc1->GetVPRotation();
        double oldskew = cc1->GetVPSkew();
        double newskew = Current_Ch->GetChartSkew() * PI / 180.0;

        if (!g_bskew_comp && !g_bCourseUp) {
            if (fabs(oldskew) > 0.0001)
                rotation = 0.0;
            if (fabs(newskew) > 0.0001)
                rotation = newskew;
        }

        cc1->SetViewPoint( zLat, zLon, best_scale, newskew, rotation );

        SetChartUpdatePeriod( cc1->GetVP() );

        UpdateGPSCompassStatusBox();           // Pick up the rotation

    }

    //          Refresh the Piano Bar
    ArrayOfInts piano_active_chart_index_array;
    piano_active_chart_index_array.Add( pCurrentStack->GetCurrentEntrydbIndex() );
    g_Piano->SetActiveKeyArray( piano_active_chart_index_array );

    if( g_ChartBarWin )
        g_ChartBarWin->Refresh( true );
}

void MyFrame::SelectdbChart( int dbindex )
{
    if( !pCurrentStack )
        return;

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

        if( Current_Ch )
            cc1->SetViewPoint( zLat, zLon, best_scale, Current_Ch->GetChartSkew() * PI / 180.,
                cc1->GetVPRotation() );

        SetChartUpdatePeriod( cc1->GetVP() );

        UpdateGPSCompassStatusBox();           // Pick up the rotation

    }

    //          Refresh the Piano Bar
    ArrayOfInts piano_active_chart_index_array;
    piano_active_chart_index_array.Add( pCurrentStack->GetCurrentEntrydbIndex() );
    g_Piano->SetActiveKeyArray( piano_active_chart_index_array );
    if( g_ChartBarWin )
        g_ChartBarWin->Refresh( true );
}

void MyFrame::SetChartUpdatePeriod( ViewPort &vp )
{
    //    Set the chart update period based upon chart skew and skew compensator

    g_ChartUpdatePeriod = 1;            // General default

    if (!g_bopengl && !vp.b_quilt)
        if ( fabs(vp.skew) > 0.0001)
            g_ChartUpdatePeriod = g_SkewCompUpdatePeriod;

    m_ChartUpdatePeriod = g_ChartUpdatePeriod;
}

void MyFrame::SetChartThumbnail( int index )
{
    if( bDBUpdateInProgress ) return;

    if( NULL == pCurrentStack ) return;

    if( NULL == pthumbwin ) return;

    if( NULL == cc1 ) return;

    bool bneedmove = false;
    
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
                        bneedmove = true;
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
                        pthumbwin->Move( wxPoint( 4, 4 ) );
                        bneedmove = true;
                    } else
                        pthumbwin->Show( false );

                    cc1->Refresh( FALSE );
                }
            }
            
            if(bneedmove && pthumbwin){         // Adjust position to avoid bad overlap
                wxPoint pos = wxPoint(4,4);
                
                wxPoint tLocn = ClientToScreen(pos);
                wxRect tRect = wxRect(tLocn.x, tLocn.y, pthumbwin->GetSize().x, pthumbwin->GetSize().y);
                
                // Simplistic overlap avoidance works best when toolbar is horizontal near the top of screen.
                // Other difficult cases simply center the thumbwin on the canvas....
                if( g_FloatingToolbarDialog && !g_FloatingToolbarDialog->isSubmergedToGrabber()){
                    if( g_FloatingToolbarDialog->GetScreenRect().Intersects( tRect ) ) {
                        wxPoint tbpos = cc1->ScreenToClient(g_FloatingToolbarDialog->GetPosition());
                        pos = wxPoint(4, g_FloatingToolbarDialog->GetSize().y + tbpos.y + 4);
                        tLocn = ClientToScreen(pos);
                    }
                }
                
                //  We cannot let the thumbwin overlap the Piano
                if(g_Piano){
                    int piano_height = g_Piano->GetHeight() + 4;
                    wxPoint cbarLocn = ClientToScreen(wxPoint(0, cc1->GetCanvasHeight() - piano_height));
                    wxRect cbarRect = wxRect(cbarLocn.x, cbarLocn.y, cc1->GetCanvasWidth(), piano_height);
                    if( cbarRect.Intersects( wxRect(tLocn.x, tLocn.y, pthumbwin->GetSize().x, pthumbwin->GetSize().y))){
                        pos = wxPoint((cc1->GetCanvasWidth() - pthumbwin->GetSize().x)/2,
                                      (cc1->GetCanvasHeight() - pthumbwin->GetSize().y)/2 - piano_height);
                    }
                }
                pthumbwin->Move( pos );
                
            }
            
        }

}

void MyFrame::UpdateControlBar( void )
{
    if( !cc1 ) return;

    if( !pCurrentStack ) return;

    if ( !g_bShowChartBar ) return;

    int sel_type = -1;
    int sel_family = -1;

    ArrayOfInts piano_chart_index_array;
    ArrayOfInts empty_piano_chart_index_array;

    wxString old_hash = g_Piano->GetStoredHash();

    if( cc1->GetQuiltMode() ) {
        piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();
        g_Piano->SetKeyArray( piano_chart_index_array );

        ArrayOfInts piano_active_chart_index_array = cc1->GetQuiltCandidatedbIndexArray();
        g_Piano->SetActiveKeyArray( piano_active_chart_index_array );

        ArrayOfInts piano_eclipsed_chart_index_array = cc1->GetQuiltEclipsedStackdbIndexArray();
        g_Piano->SetEclipsedIndexArray( piano_eclipsed_chart_index_array );

        g_Piano->SetNoshowIndexArray( g_quilt_noshow_index_array );

        if(ChartData){
            sel_type = ChartData->GetDBChartType(cc1->GetQuiltReferenceChartIndex());
            sel_family = ChartData->GetDBChartFamily(cc1->GetQuiltReferenceChartIndex());
        }
    } else {
        piano_chart_index_array = ChartData->GetCSArray( pCurrentStack );
        g_Piano->SetKeyArray( piano_chart_index_array );

        ArrayOfInts piano_active_chart_index_array;
        piano_active_chart_index_array.Add( pCurrentStack->GetCurrentEntrydbIndex() );
        g_Piano->SetActiveKeyArray( piano_active_chart_index_array );

        if(Current_Ch){
            sel_type = Current_Ch->GetChartType();
            sel_family = Current_Ch->GetChartFamily();
        }

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

        if( ctei.GetChartProjectionType() == PROJECTION_TRANSVERSE_MERCATOR )
            piano_tmerc_chart_index_array.Add( piano_chart_index_array.Item( ino ) );

        //    Polyconic skewed charts should show as skewed
        else
            if( ctei.GetChartProjectionType() == PROJECTION_POLYCONIC ) {
                if( fabs( skew_norm ) > 1. )
                    piano_skew_chart_index_array.Add(piano_chart_index_array.Item( ino ) );
                else
                    piano_poly_chart_index_array.Add( piano_chart_index_array.Item( ino ) );
            } else
                if( fabs( skew_norm ) > 1. )
                    piano_skew_chart_index_array.Add(piano_chart_index_array.Item( ino ) );

    }
    g_Piano->SetSkewIndexArray( piano_skew_chart_index_array );
    g_Piano->SetTmercIndexArray( piano_tmerc_chart_index_array );
    g_Piano->SetPolyIndexArray( piano_poly_chart_index_array );
    g_Piano->FormatKeys();

    wxString new_hash = g_Piano->GenerateAndStoreNewHash();
    if(new_hash != old_hash) {
        SetChartThumbnail( -1 );
        cc1->HideChartInfoWindow();
        g_Piano->ResetRollover();
        cc1->SetQuiltChartHiLiteIndex( -1 );
        if( g_ChartBarWin )
            g_ChartBarWin->Refresh( false );
        cc1->m_brepaint_piano = true;
    }

    // Create a bitmask int that describes what Family/Type of charts are shown in the bar,
    // and notify the platform.
    int mask = 0;
    for( unsigned int ino = 0; ino < piano_chart_index_array.GetCount(); ino++ ) {
        const ChartTableEntry &ctei = ChartData->GetChartTableEntry( piano_chart_index_array.Item( ino ) );
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

void MyFrame::selectChartDisplay( int type, int family)
{
    double target_scale = cc1->GetVP().view_scale_ppm;

    if( !cc1->GetQuiltMode() ) {
        if(pCurrentStack){
            int stack_index = -1;
            for(int i = 0; i < pCurrentStack->nEntry ; i++){
                int check_dbIndex = pCurrentStack->GetDBIndex( i );
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
        ArrayOfInts piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();
        for(unsigned int i = 0; i < piano_chart_index_array.Count() ; i++){
            int check_dbIndex = piano_chart_index_array.Item( i );
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( check_dbIndex );
            if(type == cte.GetChartType()){
                if( cc1->IsChartQuiltableRef( check_dbIndex ) ) {
                    sel_dbIndex = check_dbIndex;
                    break;
                }
            }
            else if(family == cte.GetChartFamily()){
                if( cc1->IsChartQuiltableRef( check_dbIndex ) ) {
                    sel_dbIndex = check_dbIndex;
                    break;
                }
            }
        }

        if(sel_dbIndex >= 0){
            SelectQuiltRefdbChart( sel_dbIndex, false );  // no autoscale
            //  Re-qualify the quilt reference chart selection
            cc1->AdjustQuiltRefChart(  );
        }

        //  Now reset the scale to the target...
        cc1->SetVPScale(target_scale);




    }

    UpdateGlobalMenuItems(); // update the state of the menu items (checkmarks etc)
    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();
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

    if(ChartData->IsBusy())
        return false;

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
        if( g_bLookAhead ) {
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
        if( pCurrentStack )
            current_db_index = pCurrentStack->GetCurrentEntrydbIndex(); // capture the currently selected Ref chart dbIndex
        else
            pCurrentStack = new ChartStack;

        //  This logic added to enable opening a chart when there is no
        //  previous chart indication, either from inital startup, or from adding new chart directory
        if( b_autofind && (-1 == cc1->GetQuiltReferenceChartIndex()) && pCurrentStack ){
            if(pCurrentStack->nEntry){
                int new_dbIndex = pCurrentStack->GetDBIndex(pCurrentStack->nEntry-1);    // smallest scale
                SelectQuiltRefdbChart(new_dbIndex, true);
                b_autofind = false;
            }
        }

        ChartData->BuildChartStack( pCurrentStack, tLat, tLon );
        pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );

        if( bFirstAuto ) {
            double proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / cc1->GetVPScale(); // as set from config load

            int initial_db_index = g_restore_dbindex;
            if( initial_db_index < 0 ) {
                if( pCurrentStack->nEntry ) {
                    if( ( g_restore_stackindex < pCurrentStack->nEntry )
                            && ( g_restore_stackindex >= 0 ) )
                        initial_db_index = pCurrentStack->GetDBIndex( g_restore_stackindex );
                    else
                        initial_db_index = pCurrentStack->GetDBIndex( pCurrentStack->nEntry - 1 );
                } else
                    b_autofind = true; //initial_db_index = 0;
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

                if( ChartData ) {
                    ChartBase *pc = ChartData->OpenChartFromDB( initial_db_index, FULL_INIT );
                    if( pc ) {
                        cc1->SetQuiltRefChart( initial_db_index );
                        pCurrentStack->SetCurrentEntryFromdbIndex( initial_db_index );
                    }
                }

                // Try to bound the inital Viewport scale to something reasonable for the selected reference chart
                if( ChartData ) {
                    ChartBase *pc = ChartData->OpenChartFromDB( initial_db_index, FULL_INIT );
                    if( pc ) {
                        
                        // If the chart zoom modifier is greater than 1, allow corresponding underzoom (with a 10% fluff) on startup
                        double mod = ((double)g_chart_zoom_modifier + 5.)/5.;  // 0->2
                        mod = wxMax(mod, 1.0);
                        mod = wxMin(mod, 2.0);
                        
                        proposed_scale_onscreen =
                                wxMin(proposed_scale_onscreen, mod * 1.10 * pc->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));
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
    if( 0 == ChartData->BuildChartStack( &WorkStack, tLat, tLon, g_sticky_chart ) ) {      // Bogus Lat, Lon?
        if( NULL == pDummyChart ) {
            pDummyChart = new ChartDummy;
            bNewChart = true;
        }

        if( Current_Ch ) if( Current_Ch->GetChartType() != CHART_TYPE_DUMMY ) bNewChart = true;

        Current_Ch = pDummyChart;

//    If the current viewpoint is invalid, set the default scale to something reasonable.
        double set_scale = cc1->GetVPScale();
        if( !cc1->GetVP().IsValid() ) set_scale = 1. / 20000.;

        bNewView |= cc1->SetViewPoint( tLat, tLon, set_scale, 0, cc1->GetVPRotation() );

        //      If the chart stack has just changed, there is new status
        if(WorkStack.nEntry && pCurrentStack->nEntry){
            if( !ChartData->EqualStacks( &WorkStack, pCurrentStack ) ) {
                bNewPiano = true;
                bNewChart = true;
            }
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
                set_scale = 1. / 20000.;
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
    if( pthumbwin && pthumbwin->IsShown() ) {
        if( pthumbwin->pThumbChart ){
            if( pthumbwin->pThumbChart->UpdateThumbData( gLat, gLon ) )
                pthumbwin->Refresh( TRUE );
        }
    }

    bFirstAuto = false;                           // Auto open on program start

    //  If we need a Refresh(), do it here...
    //  But don't duplicate a Refresh() done by SetViewPoint()
    if( bNewChart && !bNewView ) cc1->Refresh( false );

#ifdef ocpnUSE_GL
    // If a new chart, need to invalidate gl viewport for refresh
    // so the fbo gets flushed
    if(g_bopengl & bNewChart)
        cc1->GetglCanvas()->Invalidate();
#endif

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
    if( !pCurrentStack )
        return;

    //    No context menu if quilting is disabled
    if( !cc1->GetQuiltMode() ) return;

    menu_selected_dbIndex = selected_dbIndex;
    menu_selected_index = selected_index;

    piano_ctx_menu = new wxMenu();

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
        piano_ctx_menu->Append( ID_PIANO_ENABLE_QUILT_CHART, _("Show This Chart") );
        Connect( ID_PIANO_ENABLE_QUILT_CHART, wxEVT_COMMAND_MENU_SELECTED,
                wxCommandEventHandler(MyFrame::OnPianoMenuEnableChart) );
    } else
        if( pCurrentStack->nEntry > 1 ) {
            piano_ctx_menu->Append( ID_PIANO_DISABLE_QUILT_CHART, _("Hide This Chart") );
            Connect( ID_PIANO_DISABLE_QUILT_CHART, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler(MyFrame::OnPianoMenuDisableChart) );
        }

    wxPoint pos;
    if(g_ChartBarWin) {
        int sx, sy;
        g_ChartBarWin->GetPosition( &sx, &sy );
        pos = g_ChartBarWin->GetParent()->ScreenToClient( wxPoint( sx, sy ) );
        wxPoint key_location = g_Piano->GetKeyOrigin( selected_index );
        pos.x += key_location.x;
    } else
        pos = wxPoint(x, y);

    pos.y -= 30;

//        Invoke the drop-down menu
    if( piano_ctx_menu->GetMenuItems().GetCount() ) PopupMenu( piano_ctx_menu, pos );

    delete piano_ctx_menu;
    piano_ctx_menu = NULL;

    cc1->HideChartInfoWindow();
    g_Piano->ResetRollover();

    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();

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
    if( !pCurrentStack )
        return;

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
#ifdef __OCPN__ANDROID__
    return androidGetMemoryStatus( mem_total, mem_used );
#endif

#ifdef __LINUX__

//      Use filesystem /proc/self/statm to determine memory status
//	Provides information about memory usage, measured in pages.  The columns are:
//	size       total program size (same as VmSize in /proc/[pid]/status)
//	resident   resident set size (same as VmRSS in /proc/[pid]/status)
//	share      shared pages (from shared mappings)
//	text       text (code)
//	lib        library (unused in Linux 2.6)
//	data       data + stack
//	dt         dirty pages (unused in Linux 2.6)

    wxTextFile file;
    wxString file_name;

    if(mem_used)
    {
        *mem_used = 0;
        file_name = _T("/proc/self/statm");
        if(file.Open(file_name))
        {
            wxString str = file.GetFirstLine();
            wxStringTokenizer tkm(str, _T(" "));
            wxString mem = tkm.GetNextToken();
            mem = tkm.GetNextToken();
            long mem_extract = 0;
            if (mem.Len()) {
                mem.ToLong(&mem_extract);
                *mem_used = mem_extract *4; // XXX assume 4K page
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

    MyPrintout printout( wxT("Chart Print") );

    //  In OperGL mode, make the bitmap capture of the screen before the print method starts,
    //  so as to be sure the "Abort..." dialog does not appear on the image
    if(g_bopengl)
        printout.GenerateGLbmp( );

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

    #ifdef __WXGTK__
    SurfaceToolbar();
    cc1->SetFocus();
    Raise();                      // I dunno why...
    #endif
}

void MyFrame::OnEvtPlugInMessage( OCPN_MsgEvent & event )
{
    wxString message_ID = event.GetID();
    wxString message_JSONText = event.GetJSONText();

    //  We are free to use or ignore any or all of the PlugIn messages flying thru this pipe tee.

    //  We can possibly use the estimated magnetic variation if WMM_pi is present and active
    //  and we have no other source of Variation
    if(!g_bVAR_Rx)
    {
        if(message_ID == _T("WMM_VARIATION_BOAT"))
        {

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
        }
    }
    
    if(message_ID == _T("WMM_VARIATION"))
    {
        
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
        
        gQueryVar = decl_val;
    }
    

    if(message_ID == _T("OCPN_TRACK_REQUEST"))
    {
        wxJSONValue  root;
        wxJSONReader reader;
        wxString trk_id = wxEmptyString;

        int numErrors = reader.Parse( message_JSONText, &root );
        if ( numErrors > 0 )
            return;

        if(root.HasMember(_T("Track_ID")))
            trk_id = root[_T("Track_ID")].AsString();

        for(RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); it++)
        {
            wxString name = wxEmptyString;
            if((*it)->IsTrack() && (*it)->m_GUID == trk_id)
            {
                name = (*it)->m_RouteNameString;
                if(name.IsEmpty())
                {
                    RoutePoint *rp = (*it)->GetPoint( 1 );
                    if( rp && rp->GetCreateTime().IsValid() )
                        name = rp->GetCreateTime().FormatISODate() + _T(" ") + rp->GetCreateTime().FormatISOTime();
                    else
                        name = _("(Unnamed Track)");
                }

/*                Tracks can be huge e.g merged tracks. On Compüters with small memory this can produce a crash by insufficient memory !!

                wxJSONValue v; unsigned long i = 0;
                for(RoutePointList::iterator itp = (*it)->pRoutePointList->begin(); itp != (*it)->pRoutePointList->end(); itp++)
                {
                    v[i][0] = (*itp)->m_lat;
                    v[i][1] = (*itp)->m_lon;
                    i++;
                }
                    wxString msg_id( _T("OCPN_TRACKPOINTS_COORDS") );
                    g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
            }
*/
/*                To avoid memory problems send a single trackpoint. It's up to the plugin to collect the data. */
                int i = 1;     wxJSONValue v;
                for(RoutePointList::iterator itp = (*it)->pRoutePointList->begin(); itp != (*it)->pRoutePointList->end(); itp++)
                {
                    v[_T("lat")] = (*itp)->m_lat;
                    v[_T("lon")] = (*itp)->m_lon;
                    v[_T("TotalNodes")] = (*it)->pRoutePointList->GetCount();
                    v[_T("NodeNr")] = i;
                    v[_T("error")] = false;
                    i++;
                    wxString msg_id( _T("OCPN_TRACKPOINTS_COORDS") );
                    g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
                }
            }
            else
            {
                wxJSONValue v;
                v[_T("error")] = true;

                wxString msg_id( _T("OCPN_TRACKPOINTS_COORDS") );
                g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
            }
        }
    }
    else if(message_ID == _T("OCPN_ROUTE_REQUEST"))
    {
        wxJSONValue  root;
        wxJSONReader reader;
        wxString route_id = wxEmptyString;

        int numErrors = reader.Parse( message_JSONText, &root );
        if ( numErrors > 0 )  {
            return;
        }

        if(root.HasMember(_T("Route_ID")))
            route_id = root[_T("Route_ID")].AsString();

        for(RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); it++)
        {
            wxString name = wxEmptyString;
            wxJSONValue v;

            if(!(*it)->IsTrack() && (*it)->m_GUID == route_id)
            {
                name = (*it)->m_RouteNameString;
                if(name.IsEmpty())
                    name = _("(Unnamed Route)");

                v[_T("Name")] = name;

                wxJSONValue v; int i = 0;
                for(RoutePointList::iterator itp = (*it)->pRoutePointList->begin(); itp != (*it)->pRoutePointList->end(); itp++)
                {
                    v[i][_T("error")] = false;
                    v[i][_T("lat")] = (*itp)->m_lat;
                    v[i][_T("lon")] = (*itp)->m_lon;
                    v[i][_T("WPName")] = (*itp)->GetName();
                    v[i][_T("WPDescription")] = (*itp)->GetDescription();
                    wxHyperlinkListNode *node = (*itp)->m_HyperlinkList->GetFirst();
                    if(node)
                    {
                        int n = 1;
                        while(node)
                        {
                            Hyperlink *httpLink = node->GetData();
                            v[i][_T("WPLink")+wxString::Format(_T("%d"),n)] = httpLink->Link;
                            v[i][_T("WPLinkDesciption")+wxString::Format(_T("%d"),n++)] = httpLink->DescrText;
                            node = node->GetNext();
                        }
                    }
                    i++;
                }
                wxString msg_id( _T("OCPN_ROUTE_RESPONSE") );
                g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
            }
            else
            {
                wxJSONValue v;
                v[0][_T("error")] = true;

                wxString msg_id( _T("OCPN_ROUTE_RESPONSE") );
                g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
            }
        }
    }
    else if(message_ID == _T("OCPN_ROUTELIST_REQUEST"))
    {
        wxJSONValue  root;
        wxJSONReader reader;
        bool mode = true, error = false;

        int numErrors = reader.Parse( message_JSONText, &root );
        if ( numErrors > 0 )
            return;

        if(root.HasMember(_T("mode")))
        {
            wxString str = root[_T("mode")].AsString();
            if( str == _T("Track")) mode = false;

            wxJSONValue v; int i = 1;
            for(RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); it++)
            {
                if((*it)->IsTrack())
                    if(mode == true) continue;
                if(!(*it)->IsTrack())
                    if(mode == false) continue;
                v[0][_T("isTrack")] = !mode;

                wxString name = (*it)->m_RouteNameString;
                if(name.IsEmpty() && !mode)
                {
                    RoutePoint *rp = (*it)->GetPoint( 1 );
                    if( rp && rp->GetCreateTime().IsValid() ) name = rp->GetCreateTime().FormatISODate() + _T(" ")
                        + rp->GetCreateTime().FormatISOTime();
                    else
                        name = _("(Unnamed Track)");
                }
                else if(name.IsEmpty() && mode)
                    name = _("(Unnamed Route)");


                v[i][_T("error")] = false;
                v[i][_T("name")] = name;
                v[i][_T("GUID")] = (*it)->m_GUID;
                bool l = (*it)->IsTrack();
                if(g_pActiveTrack == (*it) && !mode)
                    v[i][_T("active")] = true;
                else
                    v[i][_T("active")] = (*it)->IsActive();
                i++;
            }
            wxString msg_id( _T("OCPN_ROUTELIST_RESPONSE") );
            g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
        }
        else
        {
            wxJSONValue v;
            v[0][_T("error")] = true;
            wxString msg_id( _T("OCPN_ROUTELIST_RESPONSE") );
            g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
        }
    }
}

void MyFrame::OnEvtTHREADMSG( OCPN_ThreadMessageEvent & event )
{
    wxLogMessage( wxString(event.GetSString().c_str(), wxConvUTF8 ));
}


bool MyFrame::EvalPriority(const wxString & message, DataStream *pDS )
{
    bool bret = true;
    wxString msg_type = message.Mid(1, 5);

    wxString stream_name;
    int stream_priority = 0;
    if( pDS ){
        stream_priority = pDS->GetPriority();
        stream_name = pDS->GetPort();
    }

    //  If the message type has never been seen before...
    if( NMEA_Msg_Hash.find( msg_type ) == NMEA_Msg_Hash.end() ) {
        NMEA_Msg_Container *pcontainer = new NMEA_Msg_Container;
        pcontainer-> current_priority = -1;     //  guarantee to execute the next clause
        pcontainer->stream_name = stream_name;
        pcontainer->receipt_time = wxDateTime::Now();

        NMEA_Msg_Hash[msg_type] = pcontainer;
    }

    NMEA_Msg_Container *pcontainer = NMEA_Msg_Hash[msg_type];
    wxString old_port = pcontainer->stream_name;

    int old_priority = pcontainer->current_priority;

    //  If the message has been seen before, and the priority is greater than or equal to current priority,
    //  then simply update the record
    if( stream_priority >= pcontainer->current_priority )
    {
        pcontainer->receipt_time = wxDateTime::Now();
        pcontainer-> current_priority = stream_priority;
        pcontainer->stream_name = stream_name;

        bret = true;
    }

    //  If the message has been seen before, and the priority is less than the current priority,
    //  then if the time since the last recorded message is greater than GPS_TIMEOUT_SECONDS
    //  then update the record with the new priority and stream.
    //  Otherwise, ignore the message as too low a priority
    else
    {
        if( (wxDateTime::Now().GetTicks() - pcontainer->receipt_time.GetTicks()) > GPS_TIMEOUT_SECONDS )
        {
            pcontainer->receipt_time = wxDateTime::Now();
            pcontainer-> current_priority = stream_priority;
            pcontainer->stream_name = stream_name;

            bret = true;
        }
        else
            bret = false;
    }

    wxString new_port = pcontainer->stream_name;

    //  If the data source or priority has changed for this message type, emit a log entry
    if (pcontainer->current_priority != old_priority || new_port != old_port )
    {
         wxString logmsg = wxString::Format(_T("Changing NMEA Datasource for %s to %s (Priority: %i)"),
                                            msg_type.c_str(),
                                            new_port.c_str(),
                                            pcontainer->current_priority);
         wxLogMessage(logmsg );

         if (NMEALogWindow::Get().Active())
         {
             wxDateTime now = wxDateTime::Now();
             wxString ss = now.FormatISOTime();
             ss.Append( _T(" ") );
             ss.Append( logmsg );
             ss.Prepend( _T("<RED>") );

             NMEALogWindow::Get().Add(ss);
             NMEALogWindow::Get().Refresh(false);
         }
    }
    return bret;
}

void MyFrame::OnEvtOCPN_NMEA( OCPN_DataStreamEvent & event )
{
    wxString sfixtime;
    bool pos_valid = false;
    bool bis_recognized_sentence = true;
    bool ll_valid = true;

    wxString str_buf = wxString(event.GetNMEAString().c_str(), wxConvUTF8);

    if( g_nNMEADebug && ( g_total_NMEAerror_messages < g_nNMEADebug ) )
    {
        g_total_NMEAerror_messages++;
        wxString msg( _T("MEH.NMEA Sentence received...") );
        msg.Append( str_buf );
        wxLogMessage( msg );
    }

    //  The message must be at least reasonably formed...
    if( (str_buf[0] != '$')  &&  (str_buf[0] != '!') )
        return;

    if( event.GetStream() )
    {
        if(!event.GetStream()->ChecksumOK(event.GetNMEAString()) )
        {
            if( g_nNMEADebug && ( g_total_NMEAerror_messages < g_nNMEADebug ) )
            {
                g_total_NMEAerror_messages++;
                wxString msg( _T(">>>>>>NMEA Sentence Checksum Bad...") );
                msg.Append( str_buf );
                wxLogMessage( msg );
            }
            return;
        }
    }

    bool b_accept = EvalPriority( str_buf, event.GetStream() );
    if( b_accept ) {
        m_NMEA0183 << str_buf;
        if( m_NMEA0183.PreParse() )
        {
            if( m_NMEA0183.LastSentenceIDReceived == _T("RMC") )
            {
                if( m_NMEA0183.Parse() )
                {
                    if( m_NMEA0183.Rmc.IsDataValid == NTrue )
                    {
                        if( !wxIsNaN(m_NMEA0183.Rmc.Position.Latitude.Latitude) )
                        {
                            double llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
                            int lat_deg_int = (int) ( llt / 100 );
                            double lat_deg = lat_deg_int;
                            double lat_min = llt - ( lat_deg * 100 );
                            gLat = lat_deg + ( lat_min / 60. );
                            if( m_NMEA0183.Rmc.Position.Latitude.Northing == South ) gLat = -gLat;
                        }
                        else
                            ll_valid = false;

                        if( !wxIsNaN(m_NMEA0183.Rmc.Position.Longitude.Longitude) )
                        {
                            double lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                            int lon_deg_int = (int) ( lln / 100 );
                            double lon_deg = lon_deg_int;
                            double lon_min = lln - ( lon_deg * 100 );
                            gLon = lon_deg + ( lon_min / 60. );
                            if( m_NMEA0183.Rmc.Position.Longitude.Easting == West )
                                gLon = -gLon;
                        }
                        else
                            ll_valid = false;

                        gSog = m_NMEA0183.Rmc.SpeedOverGroundKnots;
                        gCog = m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue;

                        if( !wxIsNaN(m_NMEA0183.Rmc.MagneticVariation) )
                        {
                            if( m_NMEA0183.Rmc.MagneticVariationDirection == East )
                                gVar = m_NMEA0183.Rmc.MagneticVariation;
                            else
                                if( m_NMEA0183.Rmc.MagneticVariationDirection == West )
                                    gVar = -m_NMEA0183.Rmc.MagneticVariation;

                            g_bVAR_Rx = true;
                            gVAR_Watchdog = gps_watchdog_timeout_ticks;
                        }

                        sfixtime = m_NMEA0183.Rmc.UTCTime;

                        if(ll_valid )
                        {
                            gGPS_Watchdog = gps_watchdog_timeout_ticks;
                            wxDateTime now = wxDateTime::Now();
                            m_fixtime = now.GetTicks();
                        }
                        pos_valid = ll_valid;
                    }
                }
                else
                    if( g_nNMEADebug )
                    {
                        wxString msg( _T("   ") );
                        msg.Append( m_NMEA0183.ErrorMessage );
                        msg.Append( _T(" : ") );
                        msg.Append( str_buf );
                        wxLogMessage( msg );
                    }
            }

            else
                if( m_NMEA0183.LastSentenceIDReceived == _T("HDT") )
                {
                    if( m_NMEA0183.Parse() )
                    {
                        gHdt = m_NMEA0183.Hdt.DegreesTrue;
                        if( !wxIsNaN(m_NMEA0183.Hdt.DegreesTrue) )
                        {
                            g_bHDT_Rx = true;
                            gHDT_Watchdog = gps_watchdog_timeout_ticks;
                        }
                    }
                    else
                        if( g_nNMEADebug )
                        {
                            wxString msg( _T("   ") );
                            msg.Append( m_NMEA0183.ErrorMessage );
                            msg.Append( _T(" : ") );
                            msg.Append( str_buf );
                            wxLogMessage( msg );
                        }
                }
                else
                    if( m_NMEA0183.LastSentenceIDReceived == _T("HDG") )
                    {
                        if( m_NMEA0183.Parse() )
                        {
                            gHdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;
                            if( !wxIsNaN(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees) )
                                gHDx_Watchdog = gps_watchdog_timeout_ticks;

                            if( m_NMEA0183.Hdg.MagneticVariationDirection == East )
                                gVar = m_NMEA0183.Hdg.MagneticVariationDegrees;
                            else if( m_NMEA0183.Hdg.MagneticVariationDirection == West )
                                gVar = -m_NMEA0183.Hdg.MagneticVariationDegrees;

                            if( !wxIsNaN(m_NMEA0183.Hdg.MagneticVariationDegrees) )
                            {
                                g_bVAR_Rx = true;
                                gVAR_Watchdog = gps_watchdog_timeout_ticks;
                            }


                        } else
                            if( g_nNMEADebug )
                            {
                                wxString msg( _T("   ") );
                                msg.Append( m_NMEA0183.ErrorMessage );
                                msg.Append( _T(" : ") );
                                msg.Append( str_buf );
                                wxLogMessage( msg );
                            }

                    }
                    else
                        if( m_NMEA0183.LastSentenceIDReceived == _T("HDM") )
                        {
                            if( m_NMEA0183.Parse() )
                            {
                                gHdm = m_NMEA0183.Hdm.DegreesMagnetic;
                                if( !wxIsNaN(m_NMEA0183.Hdm.DegreesMagnetic) )
                                    gHDx_Watchdog = gps_watchdog_timeout_ticks;
                            }
                            else
                                if( g_nNMEADebug )
                                {
                                    wxString msg( _T("   ") );
                                    msg.Append( m_NMEA0183.ErrorMessage );
                                    msg.Append( _T(" : ") );
                                    msg.Append( str_buf );
                                    wxLogMessage( msg );
                                }
                        }
                        else
                            if( m_NMEA0183.LastSentenceIDReceived == _T("VTG") )
                            {
                                if( m_NMEA0183.Parse() )
                                {
                                    if( !wxIsNaN(m_NMEA0183.Vtg.SpeedKnots) )
                                        gSog = m_NMEA0183.Vtg.SpeedKnots;
                                    if( !wxIsNaN(m_NMEA0183.Vtg.TrackDegreesTrue) )
                                        gCog = m_NMEA0183.Vtg.TrackDegreesTrue;
                                    if( !wxIsNaN(m_NMEA0183.Vtg.SpeedKnots) && !wxIsNaN(m_NMEA0183.Vtg.TrackDegreesTrue) )
                                        gGPS_Watchdog = gps_watchdog_timeout_ticks;
                                }
                                else
                                    if( g_nNMEADebug )
                                    {
                                        wxString msg( _T("   ") );
                                        msg.Append( m_NMEA0183.ErrorMessage );
                                        msg.Append( _T(" : ") );
                                        msg.Append( str_buf );
                                        wxLogMessage( msg );
                                    }
                            }
                            else
                                if( m_NMEA0183.LastSentenceIDReceived == _T("GSV") )
                                {
                                    if( m_NMEA0183.Parse() )
                                    {
                                        g_SatsInView = m_NMEA0183.Gsv.SatsInView;
                                        gSAT_Watchdog = sat_watchdog_timeout_ticks;
                                        g_bSatValid = true;
                                    }
                                    else
                                        if( g_nNMEADebug )
                                        {
                                            wxString msg( _T("   ") );
                                            msg.Append( m_NMEA0183.ErrorMessage );
                                            msg.Append( _T(" : ") );
                                            msg.Append( str_buf );
                                            wxLogMessage( msg );
                                        }
                                }
                                else
                                    if( g_bUseGLL && m_NMEA0183.LastSentenceIDReceived == _T("GLL") )
                                    {
                                        if( m_NMEA0183.Parse() )
                                        {
                                            if( m_NMEA0183.Gll.IsDataValid == NTrue )
                                            {
                                                if( !wxIsNaN(m_NMEA0183.Gll.Position.Latitude.Latitude) )
                                                {
                                                    double llt = m_NMEA0183.Gll.Position.Latitude.Latitude;
                                                    int lat_deg_int = (int) ( llt / 100 );
                                                    double lat_deg = lat_deg_int;
                                                    double lat_min = llt - ( lat_deg * 100 );
                                                    gLat = lat_deg + ( lat_min / 60. );
                                                    if( m_NMEA0183.Gll.Position.Latitude.Northing
                                                            == South ) gLat = -gLat;
                                                }
                                                else
                                                    ll_valid = false;

                                                if( !wxIsNaN(m_NMEA0183.Gll.Position.Longitude.Longitude) )
                                                {
                                                    double lln = m_NMEA0183.Gll.Position.Longitude.Longitude;
                                                    int lon_deg_int = (int) ( lln / 100 );
                                                    double lon_deg = lon_deg_int;
                                                    double lon_min = lln - ( lon_deg * 100 );
                                                    gLon = lon_deg + ( lon_min / 60. );
                                                    if( m_NMEA0183.Gll.Position.Longitude.Easting == West )
                                                        gLon = -gLon;
                                                }
                                                else
                                                    ll_valid = false;

                                                sfixtime = m_NMEA0183.Gll.UTCTime;

                                                if(ll_valid)
                                                {
                                                    gGPS_Watchdog = gps_watchdog_timeout_ticks;
                                                    wxDateTime now = wxDateTime::Now();
                                                    m_fixtime = now.GetTicks();
                                                }
                                                pos_valid = ll_valid;
                                            }
                                        } else
                                            if( g_nNMEADebug )
                                            {
                                                wxString msg( _T("   ") );
                                                msg.Append( m_NMEA0183.ErrorMessage );
                                                msg.Append( _T(" : ") );
                                                msg.Append( str_buf );
                                                wxLogMessage( msg );
                                            }
                                    }
                                    else
                                        if( m_NMEA0183.LastSentenceIDReceived == _T("GGA") )
                                        {
                                            if( m_NMEA0183.Parse() )
                                            {
                                                if( m_NMEA0183.Gga.GPSQuality > 0 )
                                                {
                                                    if( !wxIsNaN(m_NMEA0183.Gga.Position.Latitude.Latitude) )
                                                    {
                                                        double llt = m_NMEA0183.Gga.Position.Latitude.Latitude;
                                                        int lat_deg_int = (int) ( llt / 100 );
                                                        double lat_deg = lat_deg_int;
                                                        double lat_min = llt - ( lat_deg * 100 );
                                                        gLat = lat_deg + ( lat_min / 60. );
                                                        if( m_NMEA0183.Gga.Position.Latitude.Northing == South )
                                                            gLat = -gLat;
                                                    }
                                                    else
                                                        ll_valid = false;

                                                    if( !wxIsNaN(m_NMEA0183.Gga.Position.Longitude.Longitude) )
                                                    {
                                                        double lln = m_NMEA0183.Gga.Position.Longitude.Longitude;
                                                        int lon_deg_int = (int) ( lln / 100 );
                                                        double lon_deg = lon_deg_int;
                                                        double lon_min = lln - ( lon_deg * 100 );
                                                        gLon = lon_deg + ( lon_min / 60. );
                                                        if( m_NMEA0183.Gga.Position.Longitude.Easting
                                                                == West ) gLon = -gLon;
                                                    }
                                                    else
                                                        ll_valid = false;

                                                    sfixtime = m_NMEA0183.Gga.UTCTime;

                                                    if(ll_valid)
                                                    {
                                                        gGPS_Watchdog = gps_watchdog_timeout_ticks;
                                                        wxDateTime now = wxDateTime::Now();
                                                        m_fixtime = now.GetTicks();
                                                    }
                                                    pos_valid = ll_valid;

                                                    g_SatsInView = m_NMEA0183.Gga.NumberOfSatellitesInUse;
                                                    gSAT_Watchdog = sat_watchdog_timeout_ticks;
                                                    g_bSatValid = true;

                                                }
                                            } else
                                                if( g_nNMEADebug )
                                                {
                                                    wxString msg( _T("   ") );
                                                    msg.Append( m_NMEA0183.ErrorMessage );
                                                    msg.Append( _T(" : ") );
                                                    msg.Append( str_buf );
                                                    wxLogMessage( msg );
                                                }
                                        }
        }
        //      Process ownship (AIVDO) messages from any source
        else if(str_buf.Mid( 1, 5 ).IsSameAs( _T("AIVDO") ) )
        {
            GenericPosDatEx gpd;
            AIS_Error nerr = AIS_GENERIC_ERROR;
            if(g_pAIS)
                nerr = g_pAIS->DecodeSingleVDO(str_buf, &gpd, &m_VDO_accumulator);

            if(nerr == AIS_NoError)
            {
                if( !wxIsNaN(gpd.kLat) )
                    gLat = gpd.kLat;
                if( !wxIsNaN(gpd.kLon) )
                    gLon = gpd.kLon;

                gCog = gpd.kCog;
                gSog = gpd.kSog;

                if( !wxIsNaN(gpd.kHdt) )
                {
                    gHdt = gpd.kHdt;
                    g_bHDT_Rx = true;
                    gHDT_Watchdog = gps_watchdog_timeout_ticks;
                }

                if( !wxIsNaN(gpd.kLat) && !wxIsNaN(gpd.kLon) )
                {
                    gGPS_Watchdog = gps_watchdog_timeout_ticks;
                    wxDateTime now = wxDateTime::Now();
                    m_fixtime = now.GetTicks();

                    pos_valid = true;
                }
            }
            else
            {
                if( g_nNMEADebug && ( g_total_NMEAerror_messages < g_nNMEADebug ) )
                {
                    g_total_NMEAerror_messages++;
                    wxString msg( _T("   Invalid AIVDO Sentence...") );
                    msg.Append( str_buf );
                    wxLogMessage( msg );
                }
            }
        }
        else
        {
            bis_recognized_sentence = false;
            if( g_nNMEADebug && ( g_total_NMEAerror_messages < g_nNMEADebug ) )
            {
                g_total_NMEAerror_messages++;
                wxString msg( _T("   Unrecognized NMEA Sentence...") );
                msg.Append( str_buf );
                wxLogMessage( msg );
            }
        }

        if( bis_recognized_sentence ) PostProcessNNEA( pos_valid, sfixtime );
    }
}

void MyFrame::PostProcessNNEA( bool pos_valid, const wxString &sfixtime )
{
    FilterCogSog();

    //    If gSog is greater than some threshold, we determine that we are "cruising"
    if( gSog > 3.0 ) g_bCruising = true;

    //    Here is the one place we try to create gHdt from gHdm and gVar,
    //    but only if NMEA HDT sentence is not being received

    if( !g_bHDT_Rx ) {
        if( !wxIsNaN(gVar) && !wxIsNaN(gHdm)) {
            gHdt = gHdm + gVar;
            gHDT_Watchdog = gps_watchdog_timeout_ticks;
        }
    }

    if( pos_valid ) {
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

        if(STAT_FIELD_TICK >= 0 )
            SetStatusText( s1, STAT_FIELD_TICK );

        wxString sogcog;
        if( wxIsNaN(gSog) ) sogcog.Printf( _T("SOG --- ") + getUsrSpeedUnit() + _T("     ") );
        else
            sogcog.Printf( _T("SOG %2.2f ") + getUsrSpeedUnit() + _T("  "), toUsrSpeed( gSog ) );

        wxString cogs;
        if( wxIsNaN(gCog) )
            cogs.Printf( wxString( "COG ---\u00B0", wxConvUTF8 ) );
        else {
            if( g_bShowMag )
                cogs << wxString::Format( wxString("COG %03d°(M)  ", wxConvUTF8 ), (int)gFrame->GetTrueOrMag( gCog ) );
            else
                cogs << wxString::Format( wxString("COG %03d°  ", wxConvUTF8 ), (int)gFrame->GetTrueOrMag( gCog ) );
        }

        sogcog.Append( cogs );
        SetStatusText( sogcog, STAT_FIELD_SOGCOG );
    }

//    Maintain average COG for Course Up Mode

    if( !wxIsNaN(gCog) ) {
        if( g_COGAvgSec > 0 ) {
            //    Make a hole
            for( int i = g_COGAvgSec - 1; i > 0; i-- )
                COGTable[i] = COGTable[i - 1];
            COGTable[0] = gCog;

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
        else
            g_COGAvg = gCog;
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

Route *pAISMOBRoute;

void MyFrame::ActivateAISMOBRoute( AIS_Target_Data *ptarget )
{
    if(!ptarget)
        return;

    //    The MOB point
    wxDateTime mob_time = wxDateTime::Now();
    wxString mob_label( _( "AIS MAN OVERBOARD" ) );
    mob_label += _T(" at ");
    mob_label += mob_time.FormatTime();

    RoutePoint *pWP_MOB = new RoutePoint( ptarget->Lat, ptarget->Lon, _T ( "mob" ), mob_label, GPX_EMPTY_STRING );
    pWP_MOB->m_bKeepXRoute = true;
    pWP_MOB->m_bIsolatedMark = true;
    pSelect->AddSelectableRoutePoint( ptarget->Lat, ptarget->Lon, pWP_MOB );
    pConfig->AddNewWayPoint( pWP_MOB, -1 );       // use auto next num


    if( bGPSValid && !wxIsNaN(gCog) && !wxIsNaN(gSog) ) {
        RoutePoint *pWP_src = new RoutePoint( gLat, gLon, g_default_wp_icon,
                                              wxString( _( "Ownship" ) ), GPX_EMPTY_STRING );
        pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_src );

        pAISMOBRoute = new Route();
        pRouteList->Append( pAISMOBRoute );

        pAISMOBRoute->AddPoint( pWP_src );
        pAISMOBRoute->AddPoint( pWP_MOB );

        pSelect->AddSelectableRouteSegment(ptarget->Lat, ptarget->Lon, gLat, gLon, pWP_src, pWP_MOB, pAISMOBRoute );

        pAISMOBRoute->m_RouteNameString = _("Temporary AISMOB Route");
        pAISMOBRoute->m_RouteStartString = _("Present Ownship");
        pAISMOBRoute->m_RouteEndString = mob_label;

        pAISMOBRoute->m_bDeleteOnArrival = false;

        pAISMOBRoute->SetRouteArrivalRadius( -1.0 );                    // never arrives

        pAISMOBRoute->RebuildGUIDList();         // ensure the GUID list is intact and good

        if( g_pRouteMan->GetpActiveRoute() )
            g_pRouteMan->DeactivateRoute();
        //       g_pRouteMan->ActivateRoute( pAISMOBRoute, pWP_MOB );

        wxJSONValue v;
        v[_T("GUID")] = pAISMOBRoute->m_GUID;
        wxString msg_id( _T("OCPN_MAN_OVERBOARD") );
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
    }

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
        pRouteManagerDialog->UpdateRouteListCtrl();
        pRouteManagerDialog->UpdateWptListCtrl();
    }

    cc1->Refresh( false );

    wxString mob_message( _( "AIS MAN OVERBOARD" ) );
    mob_message += _T(" Time: ");
    mob_message += mob_time.Format();
    mob_message += _T("  Ownship Position: ");
    mob_message += toSDMM( 1, gLat );
    mob_message += _T("   ");
    mob_message += toSDMM( 2, gLon );
    mob_message += _T("  MOB Position: ");
    mob_message += toSDMM( 1, ptarget->Lat );
    mob_message += _T("   ");
    mob_message += toSDMM( 2, ptarget->Lon );
    wxLogMessage( mob_message );

}

void MyFrame::UpdateAISMOBRoute( AIS_Target_Data *ptarget )
{
    if(pAISMOBRoute && ptarget)
    {
        //   Update Current Ownship point
        RoutePoint *OwnPoint = pAISMOBRoute->GetPoint( 1 );
        OwnPoint->m_lat = gLat;
        OwnPoint->m_lon = gLon;

        pSelect->DeleteSelectableRoutePoint( OwnPoint );
        pSelect->AddSelectableRoutePoint( gLat, gLon, OwnPoint );

        //   Update Current MOB point
        RoutePoint *MOB_Point = pAISMOBRoute->GetPoint( 2 );
        MOB_Point->m_lat = ptarget->Lat;
        MOB_Point->m_lon = ptarget->Lon;

        pSelect->DeleteSelectableRoutePoint( MOB_Point );
        pSelect->AddSelectableRoutePoint( ptarget->Lat, ptarget->Lon, MOB_Point );

        pSelect->UpdateSelectableRouteSegments( OwnPoint );
        pSelect->UpdateSelectableRouteSegments( MOB_Point );
    }

    cc1->Refresh( false );

    if( ptarget ){
        wxDateTime mob_time = wxDateTime::Now();

        wxString mob_message( _( "AIS MAN OVERBOARD UPDATE" ) );
        mob_message += _T(" Time: ");
        mob_message += mob_time.Format();
        mob_message += _T("  Ownship Position: ");
        mob_message += toSDMM( 1, gLat );
        mob_message += _T("   ");
        mob_message += toSDMM( 2, gLon );
        mob_message += _T("  MOB Position: ");
        mob_message += toSDMM( 1, ptarget->Lat );
        mob_message += _T("   ");
        mob_message += toSDMM( 2, ptarget->Lon );

        wxLogMessage( mob_message );
    }

}


void MyFrame::applySettingsString( wxString settings)
{
    //  Save some present values
    int last_UIScaleFactor = g_GUIScaleFactor;
    bool previous_expert = g_bUIexpert;
    int last_ChartScaleFactorExp = g_ChartScaleFactor;
    
    //  Parse the passed settings string
    bool bproc_InternalGPS = false;
    bool benable_InternalGPS = false;

    int rr = GENERIC_CHANGED;

    // extract chart directories
    ArrayOfCDI NewDirArray;

    if(ChartData){
        wxStringTokenizer tkd(settings, _T(";"));
        while ( tkd.HasMoreTokens() ){
            wxString token = tkd.GetNextToken();

            if(token.StartsWith( _T("ChartDir"))){
                wxString dir = token.AfterFirst(':');
                if(dir.Length()){
                    ChartDirInfo cdi;
                    cdi.fullpath = dir.Trim();
                    cdi.magic_number = ChartData->GetMagicNumberCached(dir.Trim());
                    NewDirArray.Add(cdi);
                }
            }
        }

        // Scan for changes
        if(!ChartData->CompareChartDirArray( NewDirArray )){
            rr |= VISIT_CHARTS;
            rr |= CHANGE_CHARTS;
            wxLogMessage(_T("Chart Dir List change detected"));
        }
    }


    wxStringTokenizer tk(settings, _T(";"));
    while ( tk.HasMoreTokens() )
    {
        wxString token = tk.GetNextToken();
        wxString val = token.AfterFirst(':');

        //  Binary switches

        if(token.StartsWith( _T("prefb_lookahead"))){
            g_bLookAhead = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_quilt"))){
            g_bQuiltEnable = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_lockwp"))){
            g_bWayPointPreventDragging = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showdepthunits"))){
            g_bShowDepthUnits = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_confirmdelete"))){
            g_bConfirmObjectDelete = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showgrid"))){
            g_bDisplayGrid = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showoutlines"))){
            g_bShowOutlines = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_expertmode"))){
            g_bUIexpert = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_internalGPS"))){
            bproc_InternalGPS = true;
            benable_InternalGPS = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefs_navmode"))){
            bool bPrevMode = g_bCourseUp;
            bool new_val = val.IsSameAs(_T("Course Up"));
            if(bPrevMode != new_val)
                ToggleCourseUp();
        }


        //  Strings, etc.

        else if(token.StartsWith( _T("prefs_UIScaleFactor"))){
            double a;
            if(val.ToDouble(&a))
                g_GUIScaleFactor = wxRound( (a / 10.) - 5.);
        }

        else if(token.StartsWith( _T("prefs_chartScaleFactor"))){
            double a;
            if(val.ToDouble(&a)){
                g_ChartScaleFactor = wxRound( (a / 10.) - 5.);
                g_ChartScaleFactorExp = g_Platform->getChartScaleFactorExp( g_ChartScaleFactor );
            }
        }

        else if(token.StartsWith( _T("prefs_chartInitDir"))){
            *pInit_Chart_Dir = val;
        }

#ifdef USE_S57
        if(ps52plib){
            float conv = 1;
            int depthUnit = ps52plib->m_nDepthUnitDisplay;
            if ( depthUnit == 0 ) // feet
                conv = 0.3048f; // international definiton of 1 foot is 0.3048 metres
            else if ( depthUnit == 2 ) // fathoms
                conv = 0.3048f * 6; // 1 fathom is 6 feet
            
            if(token.StartsWith( _T("prefb_showsound"))){
                bool old_val = ps52plib->m_bShowSoundg;
                ps52plib->m_bShowSoundg = val.IsSameAs(_T("1"));
                if(old_val != ps52plib->m_bShowSoundg)
                    rr |= S52_CHANGED;
            }
            else if(token.StartsWith( _T("prefb_showSCAMIN"))){
                bool old_val = ps52plib->m_bUseSCAMIN;
                ps52plib->m_bUseSCAMIN = val.IsSameAs(_T("1"));
                if(old_val != ps52plib->m_bUseSCAMIN)
                    rr |= S52_CHANGED;
            }
            else if(token.StartsWith( _T("prefb_showimptext"))){
                bool old_val = ps52plib->m_bShowS57ImportantTextOnly;
                ps52plib->m_bShowS57ImportantTextOnly = val.IsSameAs(_T("1"));
                if(old_val != ps52plib->m_bShowS57ImportantTextOnly)
                    rr |= S52_CHANGED;
            }
            else if(token.StartsWith( _T("prefb_showlightldesc"))){
                bool old_val = ps52plib->m_bShowLdisText;
                ps52plib->m_bShowLdisText = val.IsSameAs(_T("1"));
                if(old_val != ps52plib->m_bShowLdisText)
                    rr |= S52_CHANGED;
                }
            else if(token.StartsWith( _T("prefb_showATONLabels"))){
                bool old_val = ps52plib->m_bShowAtonText;
                ps52plib->m_bShowAtonText = val.IsSameAs(_T("1"));
                if(old_val != ps52plib->m_bShowAtonText)
                    rr |= S52_CHANGED;
            }
        
            else if(token.StartsWith( _T("prefs_displaycategory"))){
                _DisCat old_nset = ps52plib->GetDisplayCategory();

                _DisCat nset = DISPLAYBASE;
                if(wxNOT_FOUND != val.Lower().Find(_T("base")))
                    nset = DISPLAYBASE;
                else if(wxNOT_FOUND != val.Lower().Find(_T("mariner")))
                    nset = MARINERS_STANDARD;
                else if(wxNOT_FOUND != val.Lower().Find(_T("standard")))
                    nset = STANDARD;
                else if(wxNOT_FOUND != val.Lower().Find(_T("all")))
                    nset = OTHER;

                if(nset != old_nset){
                    rr |= S52_CHANGED;
                    ps52plib-> SetDisplayCategory( nset );
                }
            }

            else if(token.StartsWith( _T("prefs_shallowdepth"))){
                double old_dval = S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR );
                double dval;
                if(val.ToDouble(&dval)){
                    if(fabs(dval - old_dval) > .1){
                        S52_setMarinerParam( S52_MAR_SHALLOW_CONTOUR, dval * conv );
                        rr |= S52_CHANGED;
                    }
                }
            }

            else if(token.StartsWith( _T("prefs_safetydepth"))){
                double old_dval = S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR );
                double dval;
                if(val.ToDouble(&dval)){
                    if(fabs(dval - old_dval) > .1){
                        S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval * conv );
                        rr |= S52_CHANGED;
                    }
                }
            }

            else if(token.StartsWith( _T("prefs_deepdepth"))){
                double old_dval = S52_getMarinerParam( S52_MAR_DEEP_CONTOUR );
                double dval;
                if(val.ToDouble(&dval)){
                    if(fabs(dval - old_dval) > .1){
                        S52_setMarinerParam( S52_MAR_DEEP_CONTOUR, dval * conv );
                        rr |= S52_CHANGED;
                    }
                }
            }

            else if(token.StartsWith( _T("prefs_vectorgraphicsstyle"))){
                LUPname old_LUP = ps52plib->m_nSymbolStyle;

                if(wxNOT_FOUND != val.Lower().Find(_T("paper")))
                    ps52plib->m_nSymbolStyle = PAPER_CHART;
                else if(wxNOT_FOUND != val.Lower().Find(_T("simplified")))
                    ps52plib->m_nSymbolStyle = SIMPLIFIED;

                if(old_LUP != ps52plib->m_nSymbolStyle)
                    rr |= S52_CHANGED;

            }

            else if(token.StartsWith( _T("prefs_vectorboundarystyle"))){
                LUPname old_LUP = ps52plib->m_nBoundaryStyle;

                if(wxNOT_FOUND != val.Lower().Find(_T("plain")))
                    ps52plib->m_nBoundaryStyle = PLAIN_BOUNDARIES;
                else if(wxNOT_FOUND != val.Lower().Find(_T("symbolized")))
                    ps52plib->m_nBoundaryStyle = SYMBOLIZED_BOUNDARIES;

                if(old_LUP != ps52plib->m_nBoundaryStyle)
                    rr |= S52_CHANGED;

            }

            else if(token.StartsWith( _T("prefs_vectorchartcolors"))){
                double old_dval = S52_getMarinerParam( S52_MAR_TWO_SHADES );

                if(wxNOT_FOUND != val.Lower().Find(_T("2")))
                    S52_setMarinerParam( S52_MAR_TWO_SHADES, 1. );
                else if(wxNOT_FOUND != val.Lower().Find(_T("4")))
                    S52_setMarinerParam( S52_MAR_TWO_SHADES, 0. );

                double new_dval = S52_getMarinerParam( S52_MAR_TWO_SHADES );
                if(fabs(new_dval - old_dval) > .1){
                    rr |= S52_CHANGED;
                }
            }
        }
#endif        
    }

    // Process Connections
    if(g_pConnectionParams && bproc_InternalGPS){

        //  Does the connection already exist?
        ConnectionParams *pExistingParams = NULL;
        ConnectionParams *cp = NULL;

        for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
        {
            ConnectionParams *xcp = g_pConnectionParams->Item(i);
            if(INTERNAL_GPS == xcp->Type){
                pExistingParams = xcp;
                cp = xcp;
                break;
            }
        }

        bool b_action = true;
        if(pExistingParams){
            if(pExistingParams->bEnabled == benable_InternalGPS)
                b_action = false;                    // nothing to do...
            else
                cp->bEnabled = benable_InternalGPS;
        }
        else if(benable_InternalGPS){           //  Need a new Params
            // make a generic config string for InternalGPS.
            wxString sGPS = _T("2;3;;0;0;;0;1;0;0;;0;;1;0;0;0;0");          // 17 parms
            ConnectionParams *new_params = new ConnectionParams(sGPS);

            new_params->bEnabled = benable_InternalGPS;
            g_pConnectionParams->Add(new_params);
            cp = new_params;
        }


        if(b_action && cp){                               // something to do?

            // Terminate and remove any existing stream with the same port name
            DataStream *pds_existing = g_pMUX->FindStream( cp->GetDSPort() );
            if(pds_existing)
                g_pMUX->StopAndRemoveStream( pds_existing );


            if( cp->bEnabled ) {
                dsPortType port_type = cp->IOSelect;
                DataStream *dstr = new DataStream( g_pMUX,
                                                       cp->Type,
                                                       cp->GetDSPort(),
                                                       wxString::Format(wxT("%i"), cp->Baudrate),
                                                       port_type,
                                                       cp->Priority,
                                                       cp->Garmin);
                dstr->SetInputFilter(cp->InputSentenceList);
                dstr->SetInputFilterType(cp->InputSentenceListType);
                dstr->SetOutputFilter(cp->OutputSentenceList);
                dstr->SetOutputFilterType(cp->OutputSentenceListType);
                dstr->SetChecksumCheck(cp->ChecksumCheck);

                g_pMUX->AddStream(dstr);

                cp->b_IsSetup = true;
            }
        }
    }



    // And apply the changes
    pConfig->UpdateSettings();

    //  Might need to rebuild symbols
    if(last_ChartScaleFactorExp != g_ChartScaleFactor)
        rr |= S52_CHANGED;
    
#ifdef USE_S57
    if(rr & S52_CHANGED){
        if(ps52plib){
            ps52plib->FlushSymbolCaches();
            ps52plib->ClearCNSYLUPArray();      // some CNSY depends on renderer (e.g. CARC)
            ps52plib->GenerateStateHash();
        }
    }
#endif

    ProcessOptionsDialog( rr,  &NewDirArray );

    // Try to detect if the toolbar is changing, to avoid a rebuild if not necessary.

    bool b_newToolbar = false;

    if(g_GUIScaleFactor != last_UIScaleFactor)
        b_newToolbar = true;

    if(previous_expert != g_bUIexpert)
        b_newToolbar = true;


    if(b_newToolbar && g_FloatingToolbarDialog)
        g_FloatingToolbarDialog->DestroyToolBar();


    //  We do this is one case only to remove an orphan recovery window
#ifdef __OCPN__ANDROID__
     if(previous_expert && !g_bUIexpert){
         androidForceFullRepaint();
     }
#endif

    if(previous_expert != g_bUIexpert)
        g_Platform->applyExpertMode(g_bUIexpert);

    //  We set the compass size first, since that establishes the available space for the toolbar.
    SetGPSCompassScale();
    g_Compass->SetScaleFactor(g_compass_scalefactor);
    UpdateGPSCompassStatusBox( true );

    if(b_newToolbar){
        g_Platform->ShowBusySpinner();

        SetToolbarScale();
        RequestNewToolbar(true);    // Force rebuild, to pick up bGUIexpert and scale settings.

        g_Platform->HideBusySpinner();
    }

    SurfaceToolbar();

    gFrame->Raise();

    cc1->InvalidateGL();
    DoChartUpdate();
    UpdateControlBar();
    Refresh();


    ShowChartBarIfEnabled();


#if defined(__WXOSX__) || defined(__WXQT__)
    if( g_FloatingToolbarDialog )
        g_FloatingToolbarDialog->Raise();
#endif

    if(console)
        console->Raise();

    Refresh( false );

    if (NMEALogWindow::Get().Active())
        NMEALogWindow::Get().GetTTYWindow()->Raise();

}



#ifdef wxHAS_POWER_EVENTS
void MyFrame::OnSuspending(wxPowerEvent& event)
{
 //   wxDateTime now = wxDateTime::Now();
 //   printf("OnSuspending...%d\n", now.GetTicks());

    wxLogMessage(_T("System suspend starting..."));
}

void MyFrame::OnSuspended(wxPowerEvent& WXUNUSED(event))
{
//    wxDateTime now = wxDateTime::Now();
//    printf("OnSuspended...%d\n", now.GetTicks());
    wxLogMessage(_T("System is going to suspend."));

}

void MyFrame::OnSuspendCancel(wxPowerEvent& WXUNUSED(event))
{
//    wxDateTime now = wxDateTime::Now();
//    printf("OnSuspendCancel...%d\n", now.GetTicks());
    wxLogMessage(_T("System suspend was cancelled."));
}

int g_last_resume_ticks;
void MyFrame::OnResume(wxPowerEvent& WXUNUSED(event))
{
    wxDateTime now = wxDateTime::Now();
//    printf("OnResume...%d\n", now.GetTicks());
    wxLogMessage(_T("System resumed from suspend."));

    if((now.GetTicks() - g_last_resume_ticks) > 5){
        wxLogMessage(_T("Restarting streams."));
 //       printf("   Restarting streams\n");
        g_last_resume_ticks = now.GetTicks();
        if(g_pMUX){
            g_pMUX->ClearStreams();

            g_pMUX->StartAllStreams();
        }
    }

    //  If OpenGL is enabled, Windows Resume does not properly refresh the application GL context.
    //  We need to force a Resize event that actually does something.
    if(g_bopengl){
        if( IsMaximized() ){            // This is not real pretty on-screen, but works
            Maximize(false);
            wxYield();
            Maximize(true);
        }
        else {
            wxSize sz = GetSize();
            SetSize( wxSize(sz.x - 1, sz.y));
            wxYield();
            SetSize( sz );
        }
    }

}
#endif // wxHAS_POWER_EVENTS














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
#ifdef ocpnUSE_GL
        if(m_GLbmp.IsOk()){
            wxMemoryDC mdc;
            mdc.SelectObject( m_GLbmp );
            dc->Blit( 0, 0, m_GLbmp.GetWidth(), m_GLbmp.GetHeight(), &mdc, 0, 0 );
            mdc.SelectObject( wxNullBitmap );
        }
#endif
    }
    else {

//  And Blit/scale it onto the Printer DC
        wxMemoryDC mdc;
        mdc.SelectObject( *( cc1->pscratch_bm ) );

        dc->Blit( 0, 0, cc1->pscratch_bm->GetWidth(), cc1->pscratch_bm->GetHeight(), &mdc, 0, 0 );

        mdc.SelectObject( wxNullBitmap );
    }

}

void MyPrintout::GenerateGLbmp( )
{
    if(g_bopengl) {
#ifdef ocpnUSE_GL
        int gsx = cc1->GetglCanvas()->GetSize().x;
        int gsy = cc1->GetglCanvas()->GetSize().y;

        unsigned char *buffer = (unsigned char *)malloc( gsx * gsy * 4 );
        glReadPixels(0, 0, gsx, gsy, GL_RGBA, GL_UNSIGNED_BYTE, buffer );

        unsigned char *e = (unsigned char *)malloc( gsx * gsy * 3 );

        if(buffer && e){
            for( int p = 0; p < gsx*gsy; p++ ) {
                e[3*p+0] = buffer[4*p+0];
                e[3*p+1] = buffer[4*p+1];
                e[3*p+2] = buffer[4*p+2];
            }
        }
        free(buffer);

        wxImage image( gsx,gsy );
        image.SetData(e);
        wxImage mir_imag = image.Mirror( false );
        m_GLbmp = wxBitmap( mir_imag );
#endif
    }
}



//---------------------------------------------------------------------------------------
//
//        GPS Positioning Device Detection
//
//---------------------------------------------------------------------------------------

/*
 *     Enumerate all the serial ports on the system
 *
 *     wxArrayString *EnumerateSerialPorts(void)

 *     Very system specific, unavoidably.
 */

#if defined(__UNIX__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)
extern "C" int wait(int *);                     // POSIX wait() for process

#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#endif

// ****************************************
// Fulup devices selection with scandir
// ****************************************

// reserve 4 pattern for plugins
char* devPatern[] = {
  NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL, (char*)-1};


// This function allow external plugin to search for a special device name
// ------------------------------------------------------------------------
int paternAdd (const char* patern) {
  int ind;

  // snan table for a free slot inside devpatern table
  for (ind=0; devPatern[ind] != (char*)-1; ind++)
       if (devPatern[ind] == NULL) break;

  // table if full
  if  (devPatern [ind] == (char*) -1) return -1;

  // store a copy of the patern in case calling routine had it on its stack
  devPatern [ind]  = strdup (patern);
  return 0;
}


#if defined(__UNIX__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)
// This filter verify is device is withing searched patern and verify it is openable
// -----------------------------------------------------------------------------------
int paternFilter (const struct dirent * dir) {
 char* res = NULL;
 char  devname [255];
 int   fd, ind;

  // search if devname fits with searched paterns
  for (ind=0; devPatern [ind] != (char*)-1; ind++) {
     if (devPatern [ind] != NULL) res=(char*)strcasestr(dir->d_name,devPatern [ind]);
     if (res != NULL) break;
  }

  // File does not fit researched patern
  if (res == NULL) return 0;

  // Check if we may open this file
  snprintf (devname, sizeof (devname), "/dev/%s", dir->d_name);
  fd = open(devname, O_RDWR|O_NDELAY|O_NOCTTY);

  // device name is pointing to a real device
  if(fd > 0) {
    close (fd);
    return 1;
  }

  // file is not valid
  perror (devname);
  return 0;
}

int isTTYreal(const char *dev)
{
    struct serial_struct serinfo;
    int ret = 0;

    int fd = open(dev, O_RDWR | O_NONBLOCK | O_NOCTTY);

    // device name is pointing to a real device
    if(fd >= 0) {
        if (ioctl(fd, TIOCGSERIAL, &serinfo)==0) {
            // If device type is no PORT_UNKNOWN we accept the port
            if (serinfo.type != PORT_UNKNOWN)
                ret = 1;
        }
        close (fd);
    }

    return ret;
}


#endif

#ifdef __MINGW32__ // do I need this because of mingw, or because I am running mingw under wine?
# ifndef GUID_CLASS_COMPORT
DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
# endif
#endif

wxArrayString *EnumerateSerialPorts( void )
{
    wxArrayString *preturn = new wxArrayString;

#if defined(__UNIX__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)

    //Initialize the pattern table
    if( devPatern[0] == NULL ) {
        paternAdd ( "ttyUSB" );
        paternAdd ( "ttyACM" );
        paternAdd ( "ttyGPS" );
        paternAdd ( "refcom" );
    }

 //  Looking for user privilege openable devices in /dev
 //  Fulup use scandir to improve user experience and support new generation of AIS devices.

      wxString sdev;
      int ind, fcount;
      struct dirent **filelist = {0};

      // scan directory filter is applied automatically by this call
      fcount = scandir("/dev", &filelist, paternFilter, alphasort);

      for(ind = 0; ind < fcount; ind++)  {
       wxString sdev (filelist[ind]->d_name, wxConvUTF8);
       sdev.Prepend (_T("/dev/"));

       preturn->Add (sdev);
       free(filelist[ind]);
      }

      free(filelist);

//        We try to add a few more, arbitrarily, for those systems that have fixed, traditional COM ports

    if( isTTYreal("/dev/ttyS0") )
        preturn->Add( _T("/dev/ttyS0") );

    if( isTTYreal("/dev/ttyS1") )
        preturn->Add( _T("/dev/ttyS1") );


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
        preturn->Add( _T("/dev/ttyACM0"));
        preturn->Add( _T("/dev/ttyACM1"));
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
    //  This method will not find some Bluetooth SPP ports
    for( int i = 1; i < g_nCOMPortCheck; i++ ) {
        wxString s;
        s.Printf( _T("COM%d"), i );

        COMMCONFIG cc;
        DWORD dwSize = sizeof(COMMCONFIG);
        if( GetDefaultCommConfig( s.fn_str(), &cc, &dwSize ) )
            preturn->Add( wxString( s ) );
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

    // Method 3:  WDM-Setupapi
    //  This method may not find XPort virtual ports,
    //  but does find Bluetooth SPP ports

    GUID *guidDev = (GUID*) &GUID_CLASS_COMPORT;

    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

    hDevInfo = SetupDiGetClassDevs( guidDev,
                                     NULL,
                                     NULL,
                                     DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if(hDevInfo != INVALID_HANDLE_VALUE) {

        BOOL bOk = TRUE;
        SP_DEVICE_INTERFACE_DATA ifcData;

        ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        for (DWORD ii=0; bOk; ii++) {
            bOk = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guidDev, ii, &ifcData);
            if (bOk) {
            // Got a device. Get the details.

                SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};
                bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                                      &ifcData, NULL, 0, NULL, &devdata);

                //      We really only need devdata
                if( !bOk ) {
                    if( GetLastError() == 122)  //ERROR_INSUFFICIENT_BUFFER, OK in this case
                        bOk = true;
                }
//#if 0
                //      We could get friendly name and/or description here
                TCHAR fname[256] = {0};
                TCHAR desc[256] ={0};
                if (bOk) {
                    BOOL bSuccess = SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,
                        (PBYTE)fname, sizeof(fname), NULL);

                    bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
                        (PBYTE)desc, sizeof(desc), NULL);
                }
//#endif
                //  Get the "COMn string from the registry key
                if(bOk) {
                    bool bFoundCom = false;
                    TCHAR dname[256];
                    HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(hDevInfo, &devdata,
                                                                   DICS_FLAG_GLOBAL, 0,
                                                                   DIREG_DEV, KEY_QUERY_VALUE);
                    if(INVALID_HANDLE_VALUE != hDeviceRegistryKey) {
                            DWORD RegKeyType;
                            wchar_t    wport[80];
                            LPCWSTR cstr = wport;
                            MultiByteToWideChar( 0, 0, "PortName", -1, wport, 80);
                            DWORD len = sizeof(dname);

                            int result = RegQueryValueEx(hDeviceRegistryKey, cstr,
                                                        0, &RegKeyType, (PBYTE)dname, &len );
                            if( result == 0 )
                                bFoundCom = true;
                    }

                    if( bFoundCom ) {
                        wxString port( dname, wxConvUTF8 );

                        //      If the port has already been found, remove the prior entry
                        //      in favor of this entry, which will have descriptive information appended
                        for( unsigned int n=0 ; n < preturn->GetCount() ; n++ ) {
                            if((preturn->Item(n)).IsSameAs(port)){
                                preturn->RemoveAt( n );
                                break;
                            }
                        }
                        wxString desc_name( desc, wxConvUTF8 );         // append "description"
                        port += _T(" ");
                        port += desc_name;

                        preturn->Add( port );
                    }
                }
            }
        }//for
    }// if


//    Search for Garmin device driver on Windows platforms

    HDEVINFO hdeviceinfo = INVALID_HANDLE_VALUE;

    hdeviceinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_DETECT_GUID, NULL, NULL,
            DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

    if( hdeviceinfo != INVALID_HANDLE_VALUE ) {

        if(GarminProtocolHandler::IsGarminPlugged()){
            wxLogMessage( _T("EnumerateSerialPorts() Found Garmin USB Device.") );
            preturn->Add( _T("Garmin-USB") );         // Add generic Garmin selectable device
        }
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


bool CheckSerialAccess( void )
{
    bool bret = true;
#if defined(__UNIX__) && !defined(__OCPN__ANDROID__)

#if 0
    termios ttyset_old;
    termios ttyset;
    termios ttyset_check;

    // Get a list of the ports
    wxArrayString *ports = EnumerateSerialPorts();
    if( ports->GetCount() == 0 )
        bret = false;

    for(unsigned int i=0 ; i < ports->GetCount() ; i++){
        wxCharBuffer buf = ports->Item(i).ToUTF8();

        //      For the first real port found, try to open it, write some config, and
        //      be sure it reads back correctly.
        if( isTTYreal( buf.data() ) ){
            int fd = open(buf.data(), O_RDWR | O_NONBLOCK | O_NOCTTY);

            // device name is pointing to a real device
            if(fd > 0) {

                if (isatty(fd) != 0)
                {
                    /* Save original terminal parameters */
                    tcgetattr(fd,&ttyset_old);
                    // Write some data
                    memcpy(&ttyset, &ttyset_old, sizeof(termios));

                    ttyset.c_cflag &=~ CSIZE;
                    ttyset.c_cflag |= CSIZE & CS7;

                    tcsetattr(fd, TCSANOW, &ttyset);

                    // Read it back
                    tcgetattr(fd, &ttyset_check);
                    if(( ttyset_check.c_cflag & CSIZE) != CS7 ){
                        bret = false;
                    }
                    else {
                            // and again
                        ttyset.c_cflag &=~ CSIZE;
                        ttyset.c_cflag |= CSIZE & CS8;

                        tcsetattr(fd, TCSANOW, &ttyset);

                            // Read it back
                        tcgetattr(fd, &ttyset_check);
                        if(( ttyset_check.c_cflag & CSIZE) != CS8 ){
                            bret = false;
                        }
                    }

                    tcsetattr(fd, TCSANOW, &ttyset_old);
                }

                close (fd);
            }   // if open
        }
    }

#endif

    //  Who owns /dev/ttyS0?
    bret = false;

    wxArrayString result1;
    wxExecute(_T("stat -c %G /dev/ttyS0"), result1);
    if(!result1.size())
        wxExecute(_T("stat -c %G /dev/ttyUSB0"), result1);

    if(!result1.size())
        wxExecute(_T("stat -c %G /dev/ttyACM0"), result1);

    wxString msg1 = _("OpenCPN requires access to serial ports to use serial NMEA data.\n");
    if(!result1.size()) {
        wxString msg = msg1 + _("No Serial Ports can be found on this system.\n\
You must install a serial port (modprobe correct kernel module) or plug in a usb serial device.\n");

        OCPNMessageBox ( NULL, msg, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 30 );
        return false;
    }

    //  Is the current user in this group?
    wxString user = wxGetUserId(), group = result1[0];

    wxArrayString result2;
    wxExecute(_T("groups ") + user, result2);

    if(result2.size()) {
        wxString user_groups = result2[0];

        if(user_groups.Find(group) != wxNOT_FOUND)
            bret = true;
    }

    if(!bret){

        wxString msg = msg1 + _("\
You do currently not have permission to access the serial ports on this system.\n\n\
It is suggested that you exit OpenCPN now,\n\
and add yourself to the correct group to enable serial port access.\n\n\
You may do so by executing the following command from the linux command line:\n\n\
                sudo usermod -a -G ");

        msg += group;
        msg += _T(" ");
        msg += user;
        msg += _T("\n");

        OCPNMessageBox ( NULL, msg, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 30 );
    }



#endif

    return bret;
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

wxColour GetGlobalColor(wxString colorName)
{
    wxColour ret_color;

#ifdef USE_S57
    //    Use the S52 Presentation library if present
    if( ps52plib )
        ret_color = ps52plib->getwxColour( colorName );
#endif
    if( !ret_color.Ok() && pcurrent_user_color_hash )
        ret_color = ( *pcurrent_user_color_hash )[colorName];

    //    Default
    if( !ret_color.Ok() ) {
        ret_color.Set( 128, 128, 128 );  // Simple Grey
        wxLogMessage(_T("Warning: Color not found ") + colorName);
        // Avoid duplicate warnings:
        ( *pcurrent_user_color_hash )[colorName] = ret_color;
    }

    return ret_color;
}

static const char *usercolors[] = { "Table:DAY", "GREEN1;120;255;120;", "GREEN2; 45;150; 45;",
        "GREEN3;200;220;200;", "GREEN4;  0;255;  0;", "BLUE1; 170;170;255;", "BLUE2;  45; 45;170;",
        "BLUE3;   0;  0;255;", "GREY1; 200;200;200;", "GREY2; 230;230;230;", "RED1;  220;200;200;",
        "UBLCK;   0;  0;  0;", "UWHIT; 255;255;255;", "URED;  255;  0;  0;", "UGREN;   0;255;  0;",
        "YELO1; 243;229; 47;", "YELO2; 128; 80;  0;", "TEAL1;   0;128;128;", "GREEN5;170;254;  0;",
        "COMPT; 245;247;244",
#ifdef __WXOSX__
        "DILG0; 255;255;255;",              // Dialog Background white
#else
        "DILG0; 238;239;242;",              // Dialog Background white
#endif
        "DILG1; 212;208;200;",              // Dialog Background
        "DILG2; 255;255;255;",              // Control Background
        "DILG3;   0;  0;  0;",              // Text
        "UITX1;   0;  0;  0;",              // Menu Text, derived from UINFF

        "CHGRF; 163; 180; 183;",
        "UINFM; 197;  69; 195;",
        "UINFG; 104; 228;  86;",
        "UINFF; 125; 137; 140;",
        "UINFR; 241;  84; 105;",
        "SHIPS;   7;   7;   7;",
        "CHYLW; 244; 218;  72;",
        "CHWHT; 212; 234; 238;",

        "UDKRD; 124; 16;  0;",
        "UARTE; 200;  0;  0;",              // Active Route, Grey on Dusk/Night

        "NODTA; 163; 180; 183;",
        "CHBLK;   7;   7;   7;",
        "SNDG1; 125; 137; 140;",
        "SNDG2;   7;   7;   7;",
        "SCLBR; 235; 125;  54;",
        "UIBDR; 125; 137; 140;",
        "UINFB;  58; 120; 240;",
        "UINFD;   7;   7;   7;",
        "UINFO; 235; 125;  54;",
        "PLRTE; 220;  64;  37;",
        "CHMGD; 197; 69; 195;",
        "UIBCK; 212; 234; 238;",

        "DASHB; 255;255;255;",              // Dashboard Instr background
        "DASHL; 190;190;190;",              // Dashboard Instr Label
        "DASHF;  50; 50; 50;",              // Dashboard Foreground
        "DASHR; 200;  0;  0;",              // Dashboard Red
        "DASHG;   0;200;  0;",              // Dashboard Green
        "DASHN; 200;120;  0;",              // Dashboard Needle
        "DASH1; 204;204;255;",              // Dashboard Illustrations
        "DASH2; 122;131;172;",              // Dashboard Illustrations
        "COMP1; 211;211;211;",              // Compass Window Background
        
        "Table:DUSK", "GREEN1; 60;128; 60;", "GREEN2; 22; 75; 22;", "GREEN3; 80;100; 80;",
        "GREEN4;  0;128;  0;", "BLUE1;  80; 80;160;", "BLUE2;  30; 30;120;", "BLUE3;   0;  0;128;",
        "GREY1; 100;100;100;", "GREY2; 128;128;128;", "RED1;  150;100;100;", "UBLCK;   0;  0;  0;",
        "UWHIT; 255;255;255;", "URED;  120; 54; 11;", "UGREN;  35;110; 20;", "YELO1; 120;115; 24;",
        "YELO2;  64; 40;  0;", "TEAL1;   0; 64; 64;", "GREEN5; 85;128; 0;",
        "COMPT; 124;126;121",
        
        "CHGRF;  41; 46; 46;",
        "UINFM;  58; 20; 57;",
        "UINFG;  35; 76; 29;",
        "UINFF;  41; 46; 46;",
        "UINFR;  80; 28; 35;",
        "SHIPS;  71; 78; 79;",
        "CHYLW;  81; 73; 24;",
        "CHWHT;  71; 78; 79;",

        "DILG0; 110;110;110;",              // Dialog Background
        "DILG1; 110;110;110;",              // Dialog Background
        "DILG2;   0;  0;  0;",              // Control Background
        "DILG3; 130;130;130;",              // Text
        "UITX1;  41; 46; 46;",              // Menu Text, derived from UINFF
        "UDKRD;  80;  0;  0;",
        "UARTE;  64; 64; 64;",              // Active Route, Grey on Dusk/Night

        "NODTA;  41;  46;  46;",
        "CHBLK;  54;  60;  61;",
        "SNDG1;  41;  46;  46;",
        "SNDG2;  71;  78;  79;",
        "SCLBR;  75;  38;  19;",
        "UIBDR;  54;  60;  61;",
        "UINFB;  19;  40;  80;",
        "UINFD;  71;  78;  79;",
        "UINFO;  75;  38;  19;",
        "PLRTE;  73;  21;  12;",
        "CHMGD; 74; 58; 81;",
        "UIBCK; 7; 7; 7;",

        "DASHB;  77; 77; 77;",              // Dashboard Instr background
        "DASHL;  54; 54; 54;",              // Dashboard Instr Label
        "DASHF;   0;  0;  0;",              // Dashboard Foreground
        "DASHR;  58; 21; 21;",              // Dashboard Red
        "DASHG;  21; 58; 21;",              // Dashboard Green
        "DASHN; 100; 50;  0;",              // Dashboard Needle
        "DASH1;  76; 76;113;",              // Dashboard Illustrations
        "DASH2;  48; 52; 72;",              // Dashboard Illustrations
        "COMP1; 107;107;107;",              // Compass Window Background
        
        "Table:NIGHT", "GREEN1; 30; 80; 30;", "GREEN2; 15; 60; 15;", "GREEN3; 12; 23;  9;",
        "GREEN4;  0; 64;  0;", "BLUE1;  60; 60;100;", "BLUE2;  22; 22; 85;", "BLUE3;   0;  0; 40;",
        "GREY1;  48; 48; 48;", "GREY2;  32; 32; 32;", "RED1;  100; 50; 50;", "UWHIT; 255;255;255;",
        "UBLCK;   0;  0;  0;", "URED;   60; 27;  5;", "UGREN;  17; 55; 10;", "YELO1;  60; 65; 12;",
        "YELO2;  32; 20;  0;", "TEAL1;   0; 32; 32;", "GREEN5; 44; 64; 0;",
        "COMPT;  48; 49; 51",
        "DILG0;  80; 80; 80;",              // Dialog Background
        "DILG1;  80; 80; 80;",              // Dialog Background
        "DILG2;   0;  0;  0;",              // Control Background
        "DILG3;  65; 65; 65;",              // Text
        "UITX1;  31; 34; 35;",              // Menu Text, derived from UINFF
        "UDKRD;  50;  0;  0;",
        "UARTE;  64; 64; 64;",              // Active Route, Grey on Dusk/Night

        "CHGRF;  16; 18; 18;",
        "UINFM;  52; 18; 52;",
        "UINFG;  22; 24;  7;",
        "UINFF;  31; 34; 35;",
        "UINFR;  59; 17; 10;",
        "SHIPS;  37; 41; 41;",
        "CHYLW;  31; 33; 10;",
        "CHWHT;  37; 41; 41;",

        "NODTA;   7;   7;   7;",
        "CHBLK;  31;  34;  35;",
        "SNDG1;  31;  34;  35;",
        "SNDG2;  43;  48;  48;",
        "SCLBR;  52;  28;  12;",
        "UIBDR;  31;  34;  35;",
        "UINFB;  21;  29;  69;",
        "UINFD;  43;  48;  58;",
        "UINFO;  52;  28;  12;",
        "PLRTE;  66;  19;  11;",
        "CHMGD; 52; 18; 52;",
        "UIBCK; 7; 7; 7;",

        "DASHB;   0;  0;  0;",              // Dashboard Instr background
        "DASHL;  20; 20; 20;",              // Dashboard Instr Label
        "DASHF;  64; 64; 64;",              // Dashboard Foreground
        "DASHR;  70; 15; 15;",              // Dashboard Red
        "DASHG;  15; 70; 15;",              // Dashboard Green
        "DASHN;  17; 80; 56;",              // Dashboard Needle
        "DASH1;  48; 52; 72;",              // Dashboard Illustrations
        "DASH2;  36; 36; 53;",              // Dashboard Illustrations
        "COMP1;  24; 24; 24;",              // Compass Window Background
        
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
            char name[21];
            int j = 0;
            while( buf[j] != ';' && j < 20 ) {
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

class  OCPNMessageDialog: public wxDialog
{

public:
    OCPNMessageDialog(wxWindow *parent, const wxString& message,
                           const wxString& caption = wxMessageBoxCaptionStr,
                           long style = wxOK|wxCENTRE, const wxPoint& pos = wxDefaultPosition);

    void OnYes(wxCommandEvent& event);
    void OnNo(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnClose( wxCloseEvent& event );

private:
    int m_style;
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(OCPNMessageDialog, wxDialog)
EVT_BUTTON(wxID_YES, OCPNMessageDialog::OnYes)
EVT_BUTTON(wxID_NO, OCPNMessageDialog::OnNo)
EVT_BUTTON(wxID_CANCEL, OCPNMessageDialog::OnCancel)
EVT_CLOSE(OCPNMessageDialog::OnClose)
END_EVENT_TABLE()


OCPNMessageDialog::OCPNMessageDialog( wxWindow *parent,
                                                const wxString& message,
                                                const wxString& caption,
                                                long style,
                                                const wxPoint& pos)
: wxDialog( parent, wxID_ANY, caption, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP )
{
    m_style = style;
    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );

    #if wxUSE_STATBMP
    // 1) icon
    if (style & wxICON_MASK)
    {
        wxBitmap bitmap;
        switch ( style & wxICON_MASK )
        {
            default:
                wxFAIL_MSG(_T("incorrect log style"));
                // fall through

            case wxICON_ERROR:
                bitmap = wxArtProvider::GetIcon(wxART_ERROR, wxART_MESSAGE_BOX);
                break;

            case wxICON_INFORMATION:
                bitmap = wxArtProvider::GetIcon(wxART_INFORMATION, wxART_MESSAGE_BOX);
                break;

            case wxICON_WARNING:
                bitmap = wxArtProvider::GetIcon(wxART_WARNING, wxART_MESSAGE_BOX);
                break;

            case wxICON_QUESTION:
                bitmap = wxArtProvider::GetIcon(wxART_QUESTION, wxART_MESSAGE_BOX);
                break;
        }
        wxStaticBitmap *icon = new wxStaticBitmap(this, wxID_ANY, bitmap);
        icon_text->Add( icon, 0, wxCENTER );
    }
    #endif // wxUSE_STATBMP

    #if wxUSE_STATTEXT
    // 2) text
    icon_text->Add( CreateTextSizer( message ), 0, wxALIGN_CENTER | wxLEFT, 10 );

    topsizer->Add( icon_text, 1, wxCENTER | wxLEFT|wxRIGHT|wxTOP, 10 );
    #endif // wxUSE_STATTEXT

    // 3) buttons
    int AllButtonSizerFlags = wxOK|wxCANCEL|wxYES|wxNO|wxHELP|wxNO_DEFAULT;
    int center_flag = wxEXPAND;
    if (style & wxYES_NO)
        center_flag = wxALIGN_CENTRE;
    wxSizer *sizerBtn = CreateSeparatedButtonSizer(style & AllButtonSizerFlags);
    if ( sizerBtn )
        topsizer->Add(sizerBtn, 0, center_flag | wxALL, 10 );

    SetAutoLayout( true );
    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );
    wxSize size( GetSize() );
    if (size.x < size.y*3/2)
    {
        size.x = size.y*3/2;
        SetSize( size );
    }

    Centre( wxBOTH | wxCENTER_FRAME);
}

void OCPNMessageDialog::OnYes(wxCommandEvent& WXUNUSED(event))
{
    SetReturnCode(wxID_YES);
    EndModal( wxID_YES );
}

void OCPNMessageDialog::OnNo(wxCommandEvent& WXUNUSED(event))
{
    SetReturnCode(wxID_NO);
    EndModal( wxID_NO );
}

void OCPNMessageDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    // Allow cancellation via ESC/Close button except if
    // only YES and NO are specified.
    if ( (m_style & wxYES_NO) != wxYES_NO || (m_style & wxCANCEL) )
    {
        SetReturnCode(wxID_CANCEL);
        EndModal( wxID_CANCEL );
    }
}

void OCPNMessageDialog::OnClose( wxCloseEvent& event )
{
    SetReturnCode(wxID_CANCEL);
    EndModal( wxID_CANCEL );
}




class TimedMessageBox:wxEvtHandler
{
public:
    TimedMessageBox(wxWindow* parent, const wxString& message,
                    const wxString& caption = _T("Message box"), long style = wxOK | wxCANCEL,
                    int timeout_sec = -1, const wxPoint& pos = wxDefaultPosition );
    ~TimedMessageBox();
    int GetRetVal(void){ return ret_val; }
    void OnTimer(wxTimerEvent &evt);

    wxTimer     m_timer;
    OCPNMessageDialog *dlg;
    int         ret_val;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(TimedMessageBox, wxEvtHandler)
EVT_TIMER(-1, TimedMessageBox::OnTimer)
END_EVENT_TABLE()

TimedMessageBox::TimedMessageBox(wxWindow* parent, const wxString& message,
                                 const wxString& caption, long style, int timeout_sec, const wxPoint& pos )
{
    ret_val = 0;
    m_timer.SetOwner( this, -1 );

    if(timeout_sec > 0)
        m_timer.Start( timeout_sec * 1000, wxTIMER_ONE_SHOT );

    dlg = new OCPNMessageDialog( parent, message, caption, style, pos );
    dlg->ShowModal();

    int ret= dlg->GetReturnCode();

    //  Not sure why we need this, maybe on wx3?
    if( ((style & wxYES_NO) == wxYES_NO) && (ret == wxID_OK))
        ret = wxID_YES;

    delete dlg;
    dlg = NULL;

    ret_val = ret;
}


TimedMessageBox::~TimedMessageBox()
{
}

void TimedMessageBox::OnTimer(wxTimerEvent &evt)
{
    if( dlg )
        dlg->EndModal( wxID_CANCEL );
}






int OCPNMessageBox( wxWindow *parent, const wxString& message, const wxString& caption, int style,
                    int timeout_sec, int x, int y  )
{

#ifdef __WXOSX__
    long parent_style;
    bool b_toolviz = false;
    bool b_g_ChartBarWinviz = false;

    if(g_FloatingToolbarDialog && g_FloatingToolbarDialog->IsShown()){
        g_FloatingToolbarDialog->Hide();
        b_toolviz = true;
    }

    if( g_ChartBarWin && g_ChartBarWin->IsShown()) {
        g_ChartBarWin->Hide();
        b_g_ChartBarWinviz = true;
    }

    if(parent) {
        parent_style = parent->GetWindowStyle();
        parent->SetWindowStyle( parent_style & !wxSTAY_ON_TOP );
    }

#endif

      int ret =  wxID_OK;

      TimedMessageBox tbox(parent, message, caption, style, timeout_sec, wxPoint( x, y )  );
      ret = tbox.GetRetVal() ;

//    wxMessageDialog dlg( parent, message, caption, style | wxSTAY_ON_TOP, wxPoint( x, y ) );
//    ret = dlg.ShowModal();

#ifdef __WXOSX__
    if(gFrame && b_toolviz)
        gFrame->SurfaceToolbar();

    if( g_ChartBarWin && b_g_ChartBarWinviz)
        g_ChartBarWin->Show();

    if(parent){
        parent->Raise();
        parent->SetWindowStyle( parent_style );
    }
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

//      Auto timed popup Window implementation

BEGIN_EVENT_TABLE(TimedPopupWin, wxWindow) EVT_PAINT(TimedPopupWin::OnPaint)
EVT_TIMER(POPUP_TIMER, TimedPopupWin::OnTimer)

END_EVENT_TABLE()

// Define a constructor
TimedPopupWin::TimedPopupWin( wxWindow *parent, int timeout ) :
wxWindow( parent, wxID_ANY, wxPoint( 0, 0 ), wxSize( 1, 1 ), wxNO_BORDER )
{
    m_pbm = NULL;

    m_timer_timeout.SetOwner( this, POPUP_TIMER );
    m_timeout_sec = timeout;
    isActive = false;
    Hide();
}

TimedPopupWin::~TimedPopupWin()
{
    delete m_pbm;
}
void TimedPopupWin::OnTimer( wxTimerEvent& event )
{
    if( IsShown() )
        Hide();
}


void TimedPopupWin::SetBitmap( wxBitmap &bmp )
{
    delete m_pbm;
    m_pbm = new wxBitmap( bmp );

    // Retrigger the auto timeout
    if( m_timeout_sec > 0 )
        m_timer_timeout.Start( m_timeout_sec * 1000, wxTIMER_ONE_SHOT );
}

void TimedPopupWin::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    wxMemoryDC mdc;
    mdc.SelectObject( *m_pbm );
    dc.Blit( 0, 0, width, height, &mdc, 0, 0 );

}


//      Console supporting printf functionality for Windows GUI app

#ifdef __WXMSW__
static const WORD MAX_CONSOLE_LINES = 500;  // maximum mumber of lines the output console should have

//#ifdef _DEBUG

void RedirectIOToConsole()

{

    int hConHandle;

    wxIntPtr lStdHandle;

    CONSOLE_SCREEN_BUFFER_INFO coninfo;

    FILE *fp;

    // allocate a console for this app

    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);

    // redirect unbuffered STDOUT to the console

    lStdHandle = (wxIntPtr)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    setvbuf( stdout, NULL, _IONBF, 0 );


    // redirect unbuffered STDIN to the console

    lStdHandle = (wxIntPtr)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;
    setvbuf( stdin, NULL, _IONBF, 0 );

    // redirect unbuffered STDERR to the console

    lStdHandle = (wxIntPtr)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf( stderr, NULL, _IONBF, 0 );

    // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well

    //ios::sync_with_stdio();

}

//#endif
#endif


#ifdef __WXMSW__
bool TestGLCanvas(wxString prog_dir)
{
#ifdef __MSVC__
    wxString test_app = prog_dir;
    test_app += _T("ocpn_gltest1.exe");

    if(::wxFileExists(test_app)){
        long proc_return = ::wxExecute(test_app, wxEXEC_SYNC);
        printf("OpenGL Test Process returned %0X\n", proc_return);
        if(proc_return == 0)
            printf("GLCanvas OK\n");
        else
            printf("GLCanvas failed to start, disabling OpenGL.\n");

        return (proc_return == 0);
    }
    else
        return true;
#else
    /* until we can get the source to ocpn_gltest1 assume true for mingw */
    return true;
#endif
}
#endif



wxFont *GetOCPNScaledFont( wxString item, int default_size )
{
    wxFont *dFont = FontMgr::Get().GetFont( item, default_size );
    int req_size = dFont->GetPointSize();

    if( g_bresponsive ){
        //      Adjust font size to be no smaller than xx mm actual size
        double scaled_font_size = dFont->GetPointSize();

        {

            double points_per_mm  = g_Platform->getFontPointsperPixel() * g_Platform->GetDisplayDPmm();
            double min_scaled_font_size = 3 * points_per_mm;    // smaller than 3 mm is unreadable
            int nscaled_font_size = wxMax( wxRound(scaled_font_size), min_scaled_font_size );

            if(req_size >= nscaled_font_size)
                return dFont;
            else{
                wxFont *qFont = FontMgr::Get().FindOrCreateFont( nscaled_font_size,
                                                             dFont->GetFamily(),
                                                             dFont->GetStyle(),
                                                             dFont->GetWeight());
                return qFont;
            }
        }
    }
    return dFont;
}

wxFont GetOCPNGUIScaledFont( wxString item )
{
    wxFont *dFont = FontMgr::Get().GetFont( item, 0 );
    int req_size = dFont->GetPointSize();
    wxFont qFont = *dFont;

    if( g_bresponsive ){
       double postmult =  exp( g_GUIScaleFactor * (0.693 / 5.0) );       //  exp(2)
       double scaled_font_size = dFont->GetPointSize() * postmult;

       double points_per_mm  = g_Platform->getFontPointsperPixel() * g_Platform->GetDisplayDPmm();
       double min_scaled_font_size = 3 * points_per_mm;    // smaller than 3 mm is unreadable
       int nscaled_font_size = wxMax( wxRound(scaled_font_size), min_scaled_font_size );

//        wxFont *qFont = wxTheFontList->FindOrCreateFont( nscaled_font_size,
//                                                                  dFont->GetFamily(),
//                                                                  dFont->GetStyle(),
//                                                                  dFont->GetWeight());
       qFont.SetPointSize(nscaled_font_size);
    }

    return qFont;
}

OCPN_ThreadMessageEvent::OCPN_ThreadMessageEvent(wxEventType commandType, int id)
:wxEvent(id, commandType)
{
}

OCPN_ThreadMessageEvent::~OCPN_ThreadMessageEvent()
{
}

wxEvent* OCPN_ThreadMessageEvent::Clone() const
{
    OCPN_ThreadMessageEvent *newevent=new OCPN_ThreadMessageEvent(*this);
    newevent->m_string=this->m_string;
    return newevent;
}




#if 0
/*************************************************************************
 * Serial port enumeration routines
 *
 * The EnumSerialPort function will populate an array of SSerInfo structs,
 * each of which contains information about one serial port present in
 * the system. Note that this code must be linked with setupapi.lib,
 * which is included with the Win32 SDK.
 *
 * by Zach Gorman <gormanjz@hotmail.com>
 *
 * Copyright (c) 2002 Archetype Auction Software, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following condition is
 * met: Redistributions of source code must retain the above copyright
 * notice, this condition and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ARCHETYPE AUCTION SOFTWARE OR ITS
 * AFFILIATES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ************************************************************************/

// For MFC
#include <stdafx.h>

// The next 3 includes are needed for serial port enumeration
#include <objbase.h>
#include <initguid.h>
#include <Setupapi.h>

#include "EnumSerial.h"

// The following define is from ntddser.h in the DDK. It is also
// needed for serial port enumeration.
#ifndef GUID_CLASS_COMPORT
DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, \
0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif


struct SSerInfo {
    SSerInfo() : bUsbDevice(FALSE) {}
    CString strDevPath;          // Device path for use with CreateFile()
    CString strPortName;         // Simple name (i.e. COM1)
    CString strFriendlyName;     // Full name to be displayed to a user
    BOOL bUsbDevice;             // Provided through a USB connection?
    CString strPortDesc;         // friendly name without the COMx
};

//---------------------------------------------------------------
// Helpers for enumerating the available serial ports.
// These throw a CString on failure, describing the nature of
// the error that occurred.

void EnumPortsWdm(CArray<SSerInfo,SSerInfo&> &asi);
void EnumPortsWNt4(CArray<SSerInfo,SSerInfo&> &asi);
void EnumPortsW9x(CArray<SSerInfo,SSerInfo&> &asi);
void SearchPnpKeyW9x(HKEY hkPnp, BOOL bUsbDevice,
                     CArray<SSerInfo,SSerInfo&> &asi);


//---------------------------------------------------------------
// Routine for enumerating the available serial ports.
// Throws a CString on failure, describing the error that
// occurred. If bIgnoreBusyPorts is TRUE, ports that can't
// be opened for read/write access are not included.

void EnumSerialPorts(CArray<SSerInfo,SSerInfo&> &asi, BOOL bIgnoreBusyPorts)
{
    // Clear the output array
    asi.RemoveAll();

    // Use different techniques to enumerate the available serial
    // ports, depending on the OS we're using
    OSVERSIONINFO vi;
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (!::GetVersionEx(&vi)) {
        CString str;
        str.Format("Could not get OS version. (err=%lx)",
                   GetLastError());
        throw str;
    }
    // Handle windows 9x and NT4 specially
    if (vi.dwMajorVersion < 5) {
        if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
            EnumPortsWNt4(asi);
        else
            EnumPortsW9x(asi);
    }
    else {
        // Win2k and later support a standard API for
        // enumerating hardware devices.
        EnumPortsWdm(asi);
    }

    for (int ii=0; ii<asi.GetSize(); ii++)
    {
        SSerInfo& rsi = asi[ii];
        if (bIgnoreBusyPorts) {
            // Only display ports that can be opened for read/write
            HANDLE hCom = CreateFile(rsi.strDevPath,
                                     GENERIC_READ | GENERIC_WRITE,
                                     0,    /* comm devices must be opened w/exclusive-access */
                                     NULL, /* no security attrs */
                                     OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
                                     0,    /* not overlapped I/O */
                                     NULL  /* hTemplate must be NULL for comm devices */
            );
            if (hCom == INVALID_HANDLE_VALUE) {
                // It can't be opened; remove it.
                asi.RemoveAt(ii);
                ii--;
                continue;
            }
            else {
                // It can be opened! Close it and add it to the list
                ::CloseHandle(hCom);
            }
        }

        // Come up with a name for the device.
        // If there is no friendly name, use the port name.
        if (rsi.strFriendlyName.IsEmpty())
            rsi.strFriendlyName = rsi.strPortName;

        // If there is no description, try to make one up from
            // the friendly name.
            if (rsi.strPortDesc.IsEmpty()) {
                // If the port name is of the form "ACME Port (COM3)"
                // then strip off the " (COM3)"
                rsi.strPortDesc = rsi.strFriendlyName;
                int startdex = rsi.strPortDesc.Find(" (");
                int enddex = rsi.strPortDesc.Find(")");
                if (startdex > 0 && enddex ==
                    (rsi.strPortDesc.GetLength()-1))
                    rsi.strPortDesc = rsi.strPortDesc.Left(startdex);
            }
    }
}

// Helpers for EnumSerialPorts

void EnumPortsWdm(CArray<SSerInfo,SSerInfo&> &asi)
{
    CString strErr;
    // Create a device information set that will be the container for
    // the device interfaces.
    GUID *guidDev = (GUID*) &GUID_CLASS_COMPORT;

    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

    try {
        hDevInfo = SetupDiGetClassDevs( guidDev,
                                        NULL,
                                        NULL,
                                        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
        );

        if(hDevInfo == INVALID_HANDLE_VALUE)
        {
            strErr.Format("SetupDiGetClassDevs failed. (err=%lx)",
                          GetLastError());
            throw strErr;
        }

        // Enumerate the serial ports
        BOOL bOk = TRUE;
        SP_DEVICE_INTERFACE_DATA ifcData;
        DWORD dwDetDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
        pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[dwDetDataSize];
        // This is required, according to the documentation. Yes,
        // it's weird.
        ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        for (DWORD ii=0; bOk; ii++) {
            bOk = SetupDiEnumDeviceInterfaces(hDevInfo,
                                              NULL, guidDev, ii, &ifcData);
            if (bOk) {
                // Got a device. Get the details.
                SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};
                bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                                      &ifcData, pDetData, dwDetDataSize, NULL, &devdata);
                if (bOk) {
                    CString strDevPath(pDetData->DevicePath);
                    // Got a path to the device. Try to get some more info.
                    TCHAR fname[256];
                    TCHAR desc[256];
                    BOOL bSuccess = SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,
                        (PBYTE)fname, sizeof(fname), NULL);
                    bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
                        (PBYTE)desc, sizeof(desc), NULL);
                    BOOL bUsbDevice = FALSE;
                    TCHAR locinfo[256];
                    if (SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_LOCATION_INFORMATION, NULL,
                        (PBYTE)locinfo, sizeof(locinfo), NULL))
                    {
                        // Just check the first three characters to determine
                        // if the port is connected to the USB bus. This isn't
                        // an infallible method; it would be better to use the
                        // BUS GUID. Currently, Windows doesn't let you query
                        // that though (SPDRP_BUSTYPEGUID seems to exist in
                        // documentation only).
                        bUsbDevice = (strncmp(locinfo, "USB", 3)==0);
                    }
                    if (bSuccess) {
                        // Add an entry to the array
                        SSerInfo si;
                        si.strDevPath = strDevPath;
                        si.strFriendlyName = fname;
                        si.strPortDesc = desc;
                        si.bUsbDevice = bUsbDevice;
                        asi.Add(si);
                    }

                }
                else {
                    strErr.Format("SetupDiGetDeviceInterfaceDetail failed. (err=%lx)",
                                  GetLastError());
                    throw strErr;
                }
            }
            else {
                DWORD err = GetLastError();
                if (err != ERROR_NO_MORE_ITEMS) {
                    strErr.Format("SetupDiEnumDeviceInterfaces failed. (err=%lx)", err);
                    throw strErr;
                }
            }
        }
    }
    catch (CString strCatchErr) {
        strErr = strCatchErr;
    }

    if (pDetData != NULL)
        delete [] (char*)pDetData;
    if (hDevInfo != INVALID_HANDLE_VALUE)
        SetupDiDestroyDeviceInfoList(hDevInfo);

    if (!strErr.IsEmpty())
        throw strErr;
}

void EnumPortsWNt4(CArray<SSerInfo,SSerInfo&> &asi)
{
    // NT4's driver model is totally different, and not that
    // many people use NT4 anymore. Just try all the COM ports
    // between 1 and 16
    SSerInfo si;
    for (int ii=1; ii<=16; ii++) {
        CString strPort;
        strPort.Format("COM%d",ii);
        si.strDevPath = CString("\\\\.\\") + strPort;
        si.strPortName = strPort;
        asi.Add(si);
    }
}

void EnumPortsW9x(CArray<SSerInfo,SSerInfo&> &asi)
{
    // Look at all keys in HKLM\Enum, searching for subkeys named
    // *PNP0500 and *PNP0501. Within these subkeys, search for
    // sub-subkeys containing value entries with the name "PORTNAME"
    // Search all subkeys of HKLM\Enum\USBPORTS for PORTNAME entries.

    // First, open HKLM\Enum
    HKEY hkEnum = NULL;
    HKEY hkSubEnum = NULL;
    HKEY hkSubSubEnum = NULL;

    try {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Enum", 0, KEY_READ,
            &hkEnum) != ERROR_SUCCESS)
            throw CString("Could not read from HKLM\\Enum");

        // Enumerate the subkeys of HKLM\Enum
            char acSubEnum[128];
            DWORD dwSubEnumIndex = 0;
            DWORD dwSize = sizeof(acSubEnum);
            while (RegEnumKeyEx(hkEnum, dwSubEnumIndex++, acSubEnum, &dwSize,
                NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
                HKEY hkSubEnum = NULL;
                if (RegOpenKeyEx(hkEnum, acSubEnum, 0, KEY_READ,
                    &hkSubEnum) != ERROR_SUCCESS)
                    throw CString("Could not read from HKLM\\Enum\\")+acSubEnum;

                // Enumerate the subkeys of HKLM\Enum\*\, looking for keys
                    // named *PNP0500 and *PNP0501 (or anything in USBPORTS)
                    BOOL bUsbDevice = (strcmp(acSubEnum,"USBPORTS")==0);
                    char acSubSubEnum[128];
                    dwSize = sizeof(acSubSubEnum);  // set the buffer size
                    DWORD dwSubSubEnumIndex = 0;
                    while (RegEnumKeyEx(hkSubEnum, dwSubSubEnumIndex++, acSubSubEnum,
                        &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                    {
                        BOOL bMatch = (strcmp(acSubSubEnum,"*PNP0500")==0 ||
                        strcmp(acSubSubEnum,"*PNP0501")==0 ||
                        bUsbDevice);
                        if (bMatch) {
                            HKEY hkSubSubEnum = NULL;
                            if (RegOpenKeyEx(hkSubEnum, acSubSubEnum, 0, KEY_READ,
                                &hkSubSubEnum) != ERROR_SUCCESS)
                                throw CString("Could not read from HKLM\\Enum\\") +
                                acSubEnum + "\\" + acSubSubEnum;
                            SearchPnpKeyW9x(hkSubSubEnum, bUsbDevice, asi);
                            RegCloseKey(hkSubSubEnum);
                            hkSubSubEnum = NULL;
                        }

                        dwSize = sizeof(acSubSubEnum);  // restore the buffer size
                    }

                    RegCloseKey(hkSubEnum);
                    hkSubEnum = NULL;
                    dwSize = sizeof(acSubEnum); // restore the buffer size
            }
    }
    catch (CString strError) {
        if (hkEnum != NULL)
            RegCloseKey(hkEnum);
        if (hkSubEnum != NULL)
            RegCloseKey(hkSubEnum);
        if (hkSubSubEnum != NULL)
            RegCloseKey(hkSubSubEnum);
        throw strError;
    }

    RegCloseKey(hkEnum);
}

void SearchPnpKeyW9x(HKEY hkPnp, BOOL bUsbDevice,
                     CArray<SSerInfo,SSerInfo&> &asi)
{
    // Enumerate the subkeys of the given PNP key, looking for values with
    // the name "PORTNAME"
    // First, open HKLM\Enum
    HKEY hkSubPnp = NULL;

    try {
        // Enumerate the subkeys of HKLM\Enum\*\PNP050[01]
        char acSubPnp[128];
        DWORD dwSubPnpIndex = 0;
        DWORD dwSize = sizeof(acSubPnp);
        while (RegEnumKeyEx(hkPnp, dwSubPnpIndex++, acSubPnp, &dwSize,
            NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            HKEY hkSubPnp = NULL;
            if (RegOpenKeyEx(hkPnp, acSubPnp, 0, KEY_READ,
                &hkSubPnp) != ERROR_SUCCESS)
                throw CString("Could not read from HKLM\\Enum\\...\\")
                + acSubPnp;

            // Look for the PORTNAME value
                char acValue[128];
                dwSize = sizeof(acValue);
                if (RegQueryValueEx(hkSubPnp, "PORTNAME", NULL, NULL, (BYTE*)acValue,
                    &dwSize) == ERROR_SUCCESS)
                {
                    CString strPortName(acValue);

                    // Got the portname value. Look for a friendly name.
                    CString strFriendlyName;
                    dwSize = sizeof(acValue);
                    if (RegQueryValueEx(hkSubPnp, "FRIENDLYNAME", NULL, NULL, (BYTE*)acValue,
                        &dwSize) == ERROR_SUCCESS)
                        strFriendlyName = acValue;

                    // Prepare an entry for the output array.
                        SSerInfo si;
                        si.strDevPath = CString("\\\\.\\") + strPortName;
                        si.strPortName = strPortName;
                        si.strFriendlyName = strFriendlyName;
                        si.bUsbDevice = bUsbDevice;

                        // Overwrite duplicates.
                        BOOL bDup = FALSE;
                        for (int ii=0; ii<asi.GetSize() && !bDup; ii++)
                        {
                            if (asi[ii].strPortName == strPortName) {
                                bDup = TRUE;
                                asi[ii] = si;
                            }
                        }
                        if (!bDup) {
                            // Add an entry to the array
                            asi.Add(si);
                        }
                }

                RegCloseKey(hkSubPnp);
                hkSubPnp = NULL;
                dwSize = sizeof(acSubPnp);  // restore the buffer size
        }
    }
    catch (CString strError) {
        if (hkSubPnp != NULL)
            RegCloseKey(hkSubPnp);
        throw strError;
    }
}

#endif


bool ReloadLocale()
{
    bool ret = false;
#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)

    //  Old locale is done.
    delete plocale_def_lang;

    plocale_def_lang = new wxLocale;
    wxString loc_lang_canonical;

    const wxLanguageInfo *pli = wxLocale::FindLanguageInfo( g_locale );
    bool b_initok = false;

    if( pli ) {
        b_initok = plocale_def_lang->Init( pli->Language, 1 );
        // If the locale was not initialized OK, it may be that the wxstd.mo translations
        // of the wxWidgets strings is not present.
        // So try again, without attempting to load defaults wxstd.mo.
        if( !b_initok ){
            delete plocale_def_lang;
            plocale_def_lang = new wxLocale;
            b_initok = plocale_def_lang->Init( pli->Language, 0 );
        }
        loc_lang_canonical = pli->CanonicalName;
    }

    if( !pli || !b_initok ) {
        delete plocale_def_lang;
        plocale_def_lang = new wxLocale;
        b_initok = plocale_def_lang->Init( wxLANGUAGE_ENGLISH_US, 0 );
        loc_lang_canonical = wxLocale::GetLanguageInfo( wxLANGUAGE_ENGLISH_US )->CanonicalName;
    }

    if(b_initok){
        wxString imsg = _T("Opencpn language reload for:  ");
        imsg += loc_lang_canonical;
        wxLogMessage( imsg );

        //  wxWidgets assigneds precedence to message catalogs in reverse order of loading.
        //  That is, the last catalog containing a certain translatable item takes precedence.

        //  So, Load the catalogs saved in a global string array which is populated as PlugIns request a catalog load.
        //  We want to load the PlugIn catalogs first, so that core opencpn translations loaded later will become precedent.

//        wxLog::SetVerbose(true);            // log all messages for debugging language stuff

        for(unsigned int i=0 ; i < g_locale_catalog_array.GetCount() ; i++){
            wxString imsg = _T("Loading catalog for:  ");
            imsg += g_locale_catalog_array.Item(i);
            wxLogMessage( imsg );
            plocale_def_lang->AddCatalog( g_locale_catalog_array.Item(i) );
        }


    // Get core opencpn catalog translation (.mo) file
        wxLogMessage( _T("Loading catalog for opencpn core.") );
        plocale_def_lang->AddCatalog( _T("opencpn") );

//        wxLog::SetVerbose(false);

        ret = true;
    }

    //    Always use dot as decimal
    setlocale( LC_NUMERIC, "C" );

#endif
    return ret;

}


