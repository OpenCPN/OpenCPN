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
#include "config.h"


#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers
#ifdef __WXMSW__
//#include "c:\\Program Files\\visual leak detector\\include\\vld.h"
#endif

#include <algorithm>
#include <limits.h>
#include <memory>
#include <thread>

#ifdef __WXMSW__
#include <math.h>
#include <psapi.h>
#include <stdlib.h>
#include <time.h>
#endif

#ifndef __WXMSW__
#include <setjmp.h>
#include <signal.h>
#endif

#ifdef OCPN_HAVE_X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#endif

#if (defined(__clang_major__) && (__clang_major__ < 15))
// MacOS 1.13
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#else
#include <filesystem>
#include <utility>
namespace fs = std::filesystem;
#endif

using namespace std::literals::chrono_literals;

#include <wx/apptrait.h>
#include <wx/arrimpl.cpp>
#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/clrpicker.h>
#include <wx/cmdline.h>
#include <wx/dialog.h>
#include <wx/dialog.h>
#include <wx/dir.h>
#include <wx/display.h>
#include <wx/dynlib.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/ipc.h>
#include <wx/jsonreader.h>
#include <wx/listctrl.h>
#include <wx/power.h>
#include <wx/printdlg.h>
#include <wx/print.h>
#include <wx/progdlg.h>
#include <wx/settings.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>

#include "model/ais_decoder.h"
#include "model/ais_state_vars.h"
#include "model/certificates.h"
#include "model/cmdline.h"
#include "model/comm_bridge.h"
#include "model/comm_n0183_output.h"
#include "model/comm_vars.h"
#include "model/config_vars.h"
#include "model/instance_check.h"
#include "model/local_api.h"
#include "model/logger.h"
#include "model/mDNS_query.h"
#include "model/mDNS_service.h"
#include "model/multiplexer.h"
#include "model/nav_object_database.h"
#include "model/navutil_base.h"
#include "model/own_ship.h"
#include "model/plugin_handler.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/track.h"

#include "AboutFrameImpl.h"
#include "about.h"
#include "ais_info_gui.h"
#include "AISTargetAlertDialog.h"
#include "AISTargetListDialog.h"
#include "AISTargetQueryDialog.h"
#include "CanvasConfig.h"
#include "chartdb.h"
#include "chcanv.h"
#include "cm93.h"
#include "concanv.h"
#include "config.h"
#include "ConfigMgr.h"
#include "DetailSlider.h"
#include "dychart.h"
#include "FontMgr.h"
#include "gdal/cpl_csv.h"
#include "glTexCache.h"
#include "GoToPositionDialog.h"
#include "Layer.h"
#include "MarkInfo.h"
#include "navutil.h"
#include "NMEALogWindow.h"
#include "observable.h"
#include "ocpn_app.h"
#include "OCPN_AUIManager.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "options.h"
#include "rest_server_gui.h"
#include "route_ctx_factory.h"
#include "routemanagerdialog.h"
#include "routeman_gui.h"
#include "RoutePropDlgImpl.h"
#include "s52plib.h"
#include "s57chart.h"
#include "S57QueryDialog.h"
#include "safe_mode_gui.h"
#include "SoundFactory.h"
#include "styles.h"
#include "tcmgr.h"
#include "thumbwin.h"
#include "TrackPropDlg.h"
#include "udev_rule_mgr.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#ifdef __WXOSX__
#include "model/macutils.h"
#endif

#ifdef __WXMSW__
#include "model/garmin_protocol_mgr.h"  // Used for port probing on Windows
void RedirectIOToConsole();
#endif

#if defined(__WXMSW__) && defined (__MSVC__LEAK)
#include "Stackwalker.h"
#endif

#ifdef LINUX_CRASHRPT
#include "crashprint.h"
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#else
#include "serial/serial.h"
#endif
#include "wiz_ui.h"


const char* const kUsage =
R"""(Usage:
  opencpn -h | --help
  opencpn [-p] [-f] [-G] [-g] [-P] [-l <str>] [-u <num>] [-U] [-s] [GPX file ...]
  opencpn --remote [-R] | -q] | -e] |-o <str>]

Options for starting opencpn

  -c, --configdir=<dirpath>     Use alternative configuration directory.
  -p, --portable               	Run in portable mode.
  -f, --fullscreen             	Switch to full screen mode on start.
  -G, --no_opengl              	Disable OpenGL video acceleration. This setting will
                                be remembered.
  -g, --rebuild_gl_raster_cache	Rebuild OpenGL raster cache on start.
  -D, --rebuild_chart_db        Rescan chart directories and rebuild the chart database
  -P, --parse_all_enc          	Convert all S-57 charts to OpenCPN's internal format on start.
  -l, --loglevel=<str>         	Amount of logging: error, warning, message, info, debug or trace
  -u, --unit_test_1=<num>      	Display a slideshow of <num> charts and then exit.
                                Zero or negative <num> specifies no limit.
  -U, --unit_test_2
  -s, --safe_mode              	Run without plugins, opengl and other "dangerous" stuff
  -W, --config_wizard          	Start with initial configuration wizard

Options manipulating already started opencpn
  -r, --remote                 	Execute commands on already running instance
  -R, --raise                  	Make running OpenCPN visible if hidden
  -q, --quit                   	Terminate already running opencpn
  -e, --get_rest_endpoint      	Print rest server endpoint and exit.
  -o, --open=<GPX file>         Open file in running opencpn

Arguments:
  GPX  file                     GPX-formatted file with waypoints or routes.
)""";


//  comm event definitions
wxDEFINE_EVENT(EVT_N2K_129029, wxCommandEvent);
wxDEFINE_EVENT(EVT_N2K_129026, wxCommandEvent);

wxDEFINE_EVENT(EVT_N0183_RMC, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_HDT, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_HDG, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_HDM, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_VTG, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_GSV, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_GGA, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_GLL, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_AIVDO, wxCommandEvent);


//------------------------------------------------------------------------------
//      Fwd Declarations
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      Static variable definition
//------------------------------------------------------------------------------

WX_DEFINE_OBJARRAY(ArrayOfCDI);

OCPNPlatform *g_Platform;

bool g_bFirstRun;
bool g_bUpgradeInProcess;

bool g_bPauseTest;


// Files specified on the command line, if any.

MyFrame *gFrame;

ConsoleCanvas *console;

MyConfig *pConfig;
ChartDB *ChartData;
int g_restore_stackindex;
int g_restore_dbindex;
double g_ChartNotRenderScaleFactor;

LayerList *pLayerList;

Select *pSelectTC;

MarkInfoDlg *g_pMarkInfoDialog;
RoutePropDlgImpl *pRoutePropDialog;
TrackPropDlg *pTrackPropDialog;
RouteManagerDialog *pRouteManagerDialog;
GoToPositionDialog *pGoToPositionDialog;

double vLat, vLon;

int g_nbrightness = 100;

bool bDBUpdateInProgress;

ThumbWin *pthumbwin;
TCMgr *ptcmgr;

bool g_bshowToolbar = true;
bool g_bexpert = true;
bool g_bBasicMenus = false;

bool bDrawCurrentValues;

wxString ChartListFileName;
wxString gWorldMapLocation, gDefaultWorldMapLocation;
wxString gWorldShapefileLocation;
wxString *pInit_Chart_Dir;
wxString g_csv_locn;
wxString g_SENCPrefix;
wxString g_UserPresLibData;
wxString g_VisibleLayers;
wxString g_InvisibleLayers;
wxString g_VisiNameinLayers;
wxString g_InVisiNameinLayers;

bool g_bcompression_wait;
bool g_FlushNavobjChanges;
int g_FlushNavobjChangesTimeout;

wxString g_uploadConnection;

int user_user_id;
int file_user_id;

int quitflag;
int g_tick = 0;
int g_mem_total, g_mem_initial;

bool s_bSetSystemTime;

static unsigned int malloc_max;

wxDateTime g_start_time;
wxDateTime g_loglast_time;
static OcpnSound *_bells_sounds[] = {SoundFactory(), SoundFactory()};
std::vector<OcpnSound *> bells_sound(_bells_sounds, _bells_sounds + 2);

OcpnSound *g_anchorwatch_sound = SoundFactory();

double AnchorPointMinDist;
bool AnchorAlertOn1, AnchorAlertOn2;
bool g_bCruising;

ChartDummy *pDummyChart;

ocpnStyle::StyleManager *g_StyleManager;

// Global print data, to remember settings during the session
wxPrintData *g_printData = (wxPrintData *)NULL;

// Global page setup data
wxPageSetupData *g_pageSetupData = (wxPageSetupData *)NULL;

bool g_bShowOutlines;
bool g_bShowDepthUnits;
bool g_bDisplayGrid;  // Flag indicating weather the lat/lon grid should be
                      // displayed
bool g_bShowChartBar;
bool g_bShowActiveRouteHighway;
int g_nAWDefault;
int g_nAWMax;
bool g_bPlayShipsBells;
bool g_bFullscreenToolbar;
bool g_bShowLayers;
bool g_bTransparentToolbar;
bool g_bTransparentToolbarInOpenGLOK;
int g_nAutoHideToolbar;
bool g_bAutoHideToolbar;

bool g_bPermanentMOBIcon;
bool g_bTempShowMenuBar;

int g_iNavAidRadarRingsNumberVisible;
bool g_bNavAidRadarRingsShown;
float g_fNavAidRadarRingsStep;
int g_pNavAidRadarRingsStepUnits;
bool g_bWayPointPreventDragging;
bool g_bConfirmObjectDelete;
wxColour g_colourOwnshipRangeRingsColour;
int g_maxzoomin;

// Set default color scheme
ColorScheme global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;

wxArrayPtrVoid *UserColourHashTableArray;
wxColorHashMap *pcurrent_user_color_hash;

bool bVelocityValid;

int gHDx_Watchdog;

bool g_bDebugCM93;
bool g_bDebugS57;

bool g_bfilter_cogsog;
int g_COGFilterSec = 1;
int g_SOGFilterSec;

int g_ChartUpdatePeriod;
int g_SkewCompUpdatePeriod;

int g_lastClientRectx;
int g_lastClientRecty;
int g_lastClientRectw;
int g_lastClientRecth;
double g_display_size_mm;
std::vector<size_t> g_config_display_size_mm;
bool g_config_display_size_manual;

int g_GUIScaleFactor;
int g_ChartScaleFactor;
float g_MarkScaleFactorExp;
int g_last_ChartScaleFactor;
int g_ShipScaleFactor;
float g_ShipScaleFactorExp;
int g_ENCSoundingScaleFactor;
int g_ENCTextScaleFactor;

bool g_bShowTide;
bool g_bShowCurrent;

s52plib *ps52plib;
s57RegistrarMgr *m_pRegistrarMan;

CM93OffsetDialog *g_pCM93OffsetDialog;

#ifdef __WXOSX__
#include "model/macutils.h"
#endif

// begin rms
#ifdef __WXOSX__
#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL
// end rms
extern bool s_glu_dll_ready;
extern HINSTANCE s_hGLU_DLL;  // Handle to DLL
#endif
#endif
#endif
#endif

double g_ownship_predictor_minutes;
double g_ownship_HDTpredictor_miles;
int g_cog_predictor_style;
wxString g_cog_predictor_color;
int g_cog_predictor_endmarker;
int g_ownship_HDTpredictor_style;
wxString g_ownship_HDTpredictor_color;
int g_ownship_HDTpredictor_endmarker;
int g_ownship_HDTpredictor_width;

bool g_own_ship_sog_cog_calc;
int g_own_ship_sog_cog_calc_damp_sec;

AisInfoGui *g_pAISGUI;

AISTargetQueryDialog *g_pais_query_dialog_active;
int g_iSoundDeviceIndex;

int g_S57_dialog_sx, g_S57_dialog_sy;

int g_nframewin_x;
int g_nframewin_y;
int g_nframewin_posx;
int g_nframewin_posy;
bool g_bframemax;

bool g_bAutoAnchorMark;

int gpIDXn;
long gStart_LMT_Offset;

wxArrayString *pMessageOnceArray;

bool g_bUseGLL = true;

int g_nCacheLimit;
int g_memCacheLimit;
bool g_bGDAL_Debug;

bool g_bCourseUp;
int g_COGAvgSec = 15;  // COG average period (sec.) for Course Up Mode
double g_COGAvg;
bool g_bLookAhead;
bool g_bskew_comp;
bool g_bopengl;
bool g_bSoftwareGL;
bool g_bsmoothpanzoom;
bool g_fog_overzoom;
double g_overzoom_emphasis_base;
bool g_oz_vector_scale;
double g_plus_minus_zoom_factor;
bool g_bChartBarEx;

bool g_b_legacy_input_filter_behaviour;  // Support original input filter
                                         // process or new process

PlugInManager *g_pi_manager;

bool g_bDebugGPSD;

bool g_bFullScreenQuilt = true;
bool g_bQuiltEnable;
bool g_bQuiltStart;


ChartGroupArray *g_pGroupArray;

S57QueryDialog *g_pObjectQueryDialog;

std::vector<std::string> TideCurrentDataSet;
wxString g_TCData_Dir;

bool g_boptionsactive;
options *g_options;
bool g_bDeferredInitDone;
int options_lastPage = 0;
int options_subpage = 0;

wxPoint options_lastWindowPos(0, 0);
wxSize options_lastWindowSize(0, 0);

bool g_bSleep;
bool g_bsimplifiedScalebar;

int osMajor, osMinor;

bool GetMemoryStatus(int *mem_total, int *mem_used);
bool g_bHasHwClock;


int g_nAIS_activity_timer;

bool g_bEnableZoomToCursor;

bool g_bTrackActive;
bool g_bDeferredStartTrack;
bool g_bHighliteTracks;
wxColour g_colourTrackLineColour;
wxString g_default_wp_icon;

ActiveTrack *g_pActiveTrack;
double g_TrackIntervalSeconds;

int g_cm93_zoom_factor;
PopUpDSlide *pPopupDetailSlider;
bool g_bShowDetailSlider;
int g_detailslider_dialog_x, g_detailslider_dialog_y;

bool g_bUseGreenShip;

wxString g_AW1GUID;
wxString g_AW2GUID;

bool g_b_overzoom_x = true;  // Allow high overzoom

int g_OwnShipIconType;
double g_n_ownship_length_meters;
double g_n_ownship_beam_meters;
double g_n_gps_antenna_offset_y;
double g_n_gps_antenna_offset_x;
int g_n_ownship_min_mm;

int g_NeedDBUpdate; // 0 - No update needed, 1 - Update needed because there is no chart database, inform user, 2 - Start update right away
bool g_bPreserveScaleOnX;

AboutFrameImpl *g_pAboutDlg;
about *g_pAboutDlgLegacy;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
wxLocale *plocale_def_lang = 0;
#endif

wxString g_locale;
wxString g_localeOverride;
bool g_b_assume_azerty;

int g_click_stop;

int g_MemFootMB;

bool g_bShowStatusBar;

bool g_bquiting;
int g_BSBImgDebug;

AISTargetListDialog *g_pAISTargetList;
int g_AisTargetList_count;
bool g_bAisTargetList_autosort;

bool g_bFullscreen;

OCPN_AUIManager *g_pauimgr;
wxAuiDefaultDockArt *g_pauidockart;

wxString g_toolbarConfig = _T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

ocpnFloatingToolbarDialog *g_MainToolbar;
int g_maintoolbar_x;
int g_maintoolbar_y;
long g_maintoolbar_orient;
float g_toolbar_scalefactor;

float g_compass_scalefactor;
bool g_bShowMenuBar;
bool g_bShowCompassWin;

bool g_benable_rotate;

int g_GPU_MemSize;

wxString g_uiStyle;

// Values returned from WMM_PI for variation computation request.
// Initialize to invalid so we don't use it if WMM hasn't updated yet
double gQueryVar = 361.0;

char bells_sound_file_name[2][12] = {"1bells.wav", "2bells.wav"};

int portaudio_initialized;

char nmea_tick_chars[] = {'|', '/', '-', '\\', '|', '/', '-', '\\'};

int g_sticky_chart;
int g_sticky_projection;

int n_NavMessageShown;
wxString g_config_version_string;

wxString g_CmdSoundString;

bool g_bresponsive;
bool g_bRollover;

bool b_inCompressAllCharts;
bool g_bGLexpert;
bool g_bUIexpert;

int g_chart_zoom_modifier_raster;
int g_chart_zoom_modifier_vector;


bool g_bAdvanceRouteWaypointOnArrivalOnly;

bool g_bSpaceDropMark;

wxArrayString g_locale_catalog_array;
bool b_reloadForPlugins;
bool g_btrackContinuous;

unsigned int g_canvasConfig;
bool g_useMUI;
bool g_bmasterToolbarFull = true;

int g_AndroidVersionCode;

int g_memUsed;
SENCThreadManager *g_SencThreadManager;

WX_DEFINE_ARRAY_PTR(ChartCanvas *, arrayofCanvasPtr);

arrayofCanvasPtr g_canvasArray;
wxString g_lastAppliedTemplateGUID;

ChartCanvas *g_focusCanvas;
ChartCanvas *g_overlayCanvas;

bool b_inCloseWindow;
extern int ShowNavWarning();

#ifdef LINUX_CRASHRPT
wxCrashPrint g_crashprint;
#endif

#ifndef __WXMSW__
sigjmp_buf env;  // the context saved by sigsetjmp();
#endif

// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
#ifdef __WXMSW__
DEFINE_GUID(GARMIN_DETECT_GUID, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81,
            0x6b, 0xba, 0xe7, 0x22, 0xc0);
#endif

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

//    Some static helpers
void appendOSDirSlash(wxString *pString);

void InitializeUserColors(void);
void DeInitializeUserColors(void);
void SetSystemColors(ColorScheme cs);

static bool LoadAllPlugIns(bool load_enabled) {
  g_Platform->ShowBusySpinner();
  bool b = PluginLoader::getInstance()->LoadAllPlugIns(load_enabled);
  g_Platform->HideBusySpinner();
  return b;
}

//------------------------------------------------------------------------------
//    PNG Icon resources
//------------------------------------------------------------------------------

#if defined(__WXGTK__) || defined(__WXQT__)
#include "bitmaps/opencpn.xpm"
#endif


wxString newPrivateFileName(wxString, const char *name,
                            [[maybe_unused]] const char *windowsName) {
  wxString fname = wxString::FromUTF8(name);
  wxString filePathAndName;

  filePathAndName = g_Platform->GetPrivateDataDir();
  if (filePathAndName.Last() != wxFileName::GetPathSeparator())
    filePathAndName.Append(wxFileName::GetPathSeparator());

#ifdef __WXMSW__
  wxString fwname = wxString::FromUTF8(windowsName);
  filePathAndName.Append(fwname);
#else
  filePathAndName.Append(fname);
#endif

  return filePathAndName;
}


// `Main program` equivalent, creating windows and returning main app frame
//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------
IMPLEMENT_APP(MyApp)

BEGIN_EVENT_TABLE(MyApp, wxApp)
EVT_ACTIVATE_APP(MyApp::OnActivateApp)
END_EVENT_TABLE()

static void ActivateRoute(const std::string &guid) {
  Route *route = g_pRouteMan->FindRouteByGUID(guid);
  if (!route) {
    wxLogMessage("Cannot activate guid: no such route");
    return;
  }
  if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();
  //  If this is an auto-created MOB route, always select the second point
  //  (the MOB)
  // as the destination.
  RoutePoint* point;
  if (wxNOT_FOUND == route->m_RouteNameString.Find("MOB")) {
    point = g_pRouteMan->FindBestActivatePoint(route, gLat, gLon, gCog, gSog);
  } else {
    point = route->GetPoint(2);
  }
  g_pRouteMan->ActivateRoute(route, point);
  if (g_pRouteMan) g_pRouteMan->on_routes_update.Notify();
  route->m_bRtIsSelected = false;
}

static void ReverseRoute(const std::string &guid) {
  Route *route = g_pRouteMan->FindRouteByGUID(guid);
  if (!route) {
    wxLogMessage("Cannot activate guid: no such route");
    return;
  }
  route->Reverse();
  if (g_pRouteMan) g_pRouteMan->on_routes_update.Notify();
}


void MyApp::InitRestListeners() {
  auto activate_route = [&](wxCommandEvent ev) {
    auto guid = ev.GetString().ToStdString();
    ActivateRoute(guid); };
  rest_activate_listener.Init(m_rest_server.activate_route, activate_route);
  auto reverse_route = [&](wxCommandEvent ev) {
    auto guid = ev.GetString().ToStdString();
    ReverseRoute(guid); };
  rest_reverse_listener.Init(m_rest_server.reverse_route, reverse_route);
}

  bool MyApp::OpenFile(const std::string& path) {
  NavObjectCollection1 nav_objects;
  auto result = nav_objects.load_file(path.c_str());
  if (!result)  {
    std::string s(_("Cannot load route or waypoint file: "));
    s += std::string("\"") + path + "\"";
    wxMessageBox(s, "OpenCPN", wxICON_WARNING | wxOK);
    return false;
  }

  int wpt_dups;
  // Import with full vizibility of names and objects
  nav_objects.LoadAllGPXObjects(!nav_objects.IsOpenCPN(), wpt_dups, true);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateLists();
  LLBBox box = nav_objects.GetBBox();
  if (box.GetValid()) {
    gFrame->CenterView(gFrame->GetPrimaryCanvas(), box);
  }
  return true;
}

#ifndef __ANDROID__
void MyApp::OnInitCmdLine(wxCmdLineParser &parser) {
  // Add OpenCPN specific command line options. Help message
  // is hardcoded in kUsage;
  parser.AddSwitch("h", "help", "", wxCMD_LINE_OPTION_HELP);
  parser.AddSwitch("p", "portable");
  parser.AddOption("c", "configdir",  "", wxCMD_LINE_VAL_STRING,
                   wxCMD_LINE_PARAM_OPTIONAL);
  parser.AddSwitch("f", "fullscreen");
  parser.AddSwitch("G", "no_opengl");
  parser.AddSwitch("W", "config_wizard");
  parser.AddSwitch("g", "rebuild_gl_raster_cache");
  parser.AddSwitch("D", "rebuild_chart_db");
  parser.AddSwitch("P", "parse_all_enc");
  parser.AddOption("l", "loglevel");
  parser.AddOption("u", "unit_test_1", "", wxCMD_LINE_VAL_NUMBER);
  parser.AddSwitch("U", "unit_test_2");
  parser.AddParam("import GPX files", wxCMD_LINE_VAL_STRING,
                  wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
  parser.AddSwitch("s", "safe_mode");
  parser.AddSwitch("r", "remote");
  parser.AddSwitch("R", "raise");
  parser.AddSwitch("q", "quit");
  parser.AddSwitch("e", "get_rest_endpoint");
  parser.AddOption("o", "open", "", wxCMD_LINE_VAL_STRING,
                   wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
}
#endif   // __ANDROID__

/** Parse --loglevel and set up logging, falling back to defaults. */
#ifdef __ANDROID__
static void ParseLoglevel(wxCmdLineParser &parser) {
  wxLog::SetLogLevel(wxLOG_Message);
}
#else
static void ParseLoglevel(wxCmdLineParser &parser) {
  const char *strLevel = std::getenv("OPENCPN_LOGLEVEL");
  strLevel = strLevel ? strLevel : "info";
  wxString wxLevel;
  if (parser.Found("l", &wxLevel)) {
    strLevel = wxLevel.c_str();
  }
  wxLogLevel level = OcpnLog::str2level(strLevel);
  if (level == OcpnLog::LOG_BADLEVEL) {
    fprintf(stderr, "Bad loglevel %s, using \"info\"", strLevel);
    level = wxLOG_Info;
  }
  wxLog::SetLogLevel(level);
}
#endif   // __ANDROID__

#ifndef __ANDROID__
bool MyApp::OnCmdLineHelp(wxCmdLineParser& parser) {
  std::cout << kUsage;
  return false;
}
#endif

#ifndef __ANDROID__
bool MyApp::OnCmdLineParsed(wxCmdLineParser &parser) {
  long number;
  wxString repo;
  wxString plugin;

  g_unit_test_2 = parser.Found("unit_test_2");
  g_bportable = parser.Found("p");
  g_start_fullscreen = parser.Found("fullscreen");
  g_bdisable_opengl = parser.Found("no_opengl");
  g_rebuild_gl_cache = parser.Found("rebuild_gl_raster_cache");
  g_NeedDBUpdate = parser.Found("rebuild_chart_db") ? 2 : 0;
  g_parse_all_enc = parser.Found("parse_all_enc");
  g_config_wizard = parser.Found("config_wizard");
  if (parser.Found("unit_test_1", &number)) {
    g_unit_test_1 = static_cast<int>(number);
    if (g_unit_test_1 == 0) g_unit_test_1 = -1;
  }
  safe_mode::set_mode(parser.Found("safe_mode"));
  ParseLoglevel(parser);
  wxString wxstr;
  if (parser.Found("configdir", &wxstr)) {
    g_configdir = wxstr.ToStdString();
    fs::path path(g_configdir);
    if (!fs::exists(path) || !fs::is_directory(path)) {
      std::cerr << g_configdir << " is not an existing directory.\n";
      return false;
    }
  }

  bool has_start_options = false;
  static const std::vector<std::string> kStartOptions = {
    "unit_test_2", "p", "fullscreen", "no_opengl", "rebuild_gl_raster_cache",
    "rebuild_chart_db", "parse_all_enc", "unit_test_1", "safe_mode", "loglevel" };
  for (const auto& opt : kStartOptions) {
    if (parser.Found(opt)) has_start_options = true;
  }
  if (has_start_options && parser.Found("remote")) {
    std::cerr << "this option is not compatible with --remote\n";
    return false;
  }

  bool has_remote_options = false;
  static const std::vector<std::string> kRemoteOptions = {
    "raise", "quit", "open", "get_rest_endpoint"};
  for (const auto& opt : kRemoteOptions) {
    if (parser.Found(opt)) has_remote_options = true;
  }
  if (has_remote_options && ! parser.Found("remote")) {
    std::cerr << "This option requires --remote\n";
    return false;
  }

  for (size_t paramNr = 0; paramNr < parser.GetParamCount(); ++paramNr)
    g_params.push_back(parser.GetParam(paramNr).ToStdString());

  wxString optarg;
  if (!parser.Found("remote"))
    m_parsed_cmdline = ParsedCmdline();
  else if (parser.Found("raise"))
    m_parsed_cmdline = ParsedCmdline(CmdlineAction::Raise);
  else if (parser.Found("quit"))
    m_parsed_cmdline = ParsedCmdline(CmdlineAction::Quit);
  else if (parser.Found("get_rest_endpoint"))
    m_parsed_cmdline = ParsedCmdline(CmdlineAction::GetRestEndpoint);
  else if (parser.Found("open", &optarg))
    m_parsed_cmdline = ParsedCmdline(CmdlineAction::Open,
                                     optarg.ToStdString());
  else if (parser.GetParamCount() == 1)
    m_parsed_cmdline = ParsedCmdline(CmdlineAction::Open,
                                     parser.GetParam(0).ToStdString());
  else if (!has_start_options && !has_remote_options) {
    // Neither arguments nor options
    m_parsed_cmdline = ParsedCmdline(CmdlineAction::Raise);
  }
  return true;
}
#endif  // __ANDROID__

#ifdef __WXMSW__
//  Handle any exception not handled by CrashRpt
//  Most probable:  Malloc/new failure

bool MyApp::OnExceptionInMainLoop() {
  wxLogWarning(_T("Caught MainLoopException, continuing..."));
  return true;
}
#endif

void MyApp::OnActivateApp(wxActivateEvent &event) {
  return;
}


static wxStopWatch init_sw;

int MyApp::OnRun() {
  if (m_exitcode != -2) return m_exitcode;
  return wxAppConsole::OnRun();
}

MyApp::MyApp()
    : m_checker(InstanceCheck::GetInstance()),
      m_rest_server(PINCreateDialog::GetDlgCtx(),
      RouteCtxFactory(),
      g_bportable),
      m_usb_watcher(UsbWatchDaemon::GetInstance()),
      m_exitcode(-2)
{
#ifdef __linux__
  // Handle e. g., wayland default display -- see #1166.
  if (wxGetEnv( "WAYLAND_DISPLAY", NULL)) {
    setenv("GDK_BACKEND", "x11", 1);
  }
  setenv("mesa_glthread", "false", 1); // Explicitly disable glthread. This may have some impact on OpenGL performance,
                                       // but we know it is problematic for us. See #2889
#endif   // __linux__
}

bool MyApp::OnInit() {
  if (!wxApp::OnInit()) return false;
#ifdef __ANDROID__
  androidEnableBackButton(false);
  androidEnableOptionItems(false);
#endif

  GpxDocument::SeedRandom();

#if defined(__WXGTK__) && defined(ocpnUSE_GLES) && defined(__ARM_ARCH)
  // There is a race condition between cairo which is used for text rendering
  // by gtk and EGL which without the below code causes a bus error and the
  // program aborts before startup
  // this hack forces cairo to load right now by rendering some text

  wxBitmap bmp(10, 10, -1);
  wxMemoryDC dc;
  dc.SelectObject(bmp);
  dc.DrawText(_T("X"), 0, 0);
#endif

  // Instantiate the global OCPNPlatform class
  g_Platform = new OCPNPlatform;
  g_BasePlatform = g_Platform;
#ifndef __ANDROID__
  //  We allow only one instance unless the portable option is used
  if (!g_bportable && wxDirExists(g_Platform->GetPrivateDataDir())) {
    m_checker.WaitUntilValid();
    if (m_checker.IsMainInstance()) {
      // Server is created on first call to GetInstance()
      if (m_parsed_cmdline.action == CmdlineAction::Skip) {
        // Server starts running when referenced.
        [[maybe_unused]] auto& server = LocalServerApi::GetInstance();
      } else {
        std::cerr << "No remote opencpn found. Giving up.\n";
        m_exitcode = 1;
        return true;
      }
    } else {
      std::unique_ptr<LocalClientApi> client;
      try {
        client = LocalClientApi::GetClient();
      } catch (LocalApiException& ie) {
        WARNING_LOG << "Ipc client exception: " << ie.str();
        // If we get here it means that the instance_chk found another
        // running instance. But that instance is for some reason not
        // reachable. The safe thing to do is delete the lockfile and exit.
        // Next start  will proceed normally. This may leave a zombie OpenCPN,
        // but at least O starts.
        m_checker.CleanUp();
        wxMessageBox(_("Sorry, an existing instance of OpenCPN may be too busy "
                       "to respond.\nPlease retry."),
                     "OpenCPN", wxICON_INFORMATION | wxOK);
        m_exitcode = 2;
        return true;  // main program quiet exit.
      }
      if (client) {
        auto result =
          client->HandleCmdline(m_parsed_cmdline.action, m_parsed_cmdline.arg);
        if (result.first) {
          m_exitcode = 0;
        } else {
          wxLogDebug("Error running remote command: %s", result.second.c_str());
          m_exitcode = 1;
        }
        return true;
      }
    }
  }
#endif  // __ANDROID__

  if (getenv("OPENCPN_FATAL_ERROR") != 0) {
    wxLogFatalError(getenv("OPENCPN_FATAL_ERROR"));
  }

#ifndef __ANDROID__
  // Check if last run failed, set up safe_mode.
  if (!safe_mode::get_mode()) {
    safe_mode::check_last_start();
  }
#endif

  //  Perform first stage initialization
  OCPNPlatform::Initialize_1();

  // Set the name of the app as displayed to the user.
  // This is necessary at least on OS X, for the capitalisation to be correct in
  // the system menus.
  MyApp::SetAppDisplayName("OpenCPN");

  //  Seed the random number generator
  wxDateTime x = wxDateTime::UNow();
  long seed = x.GetMillisecond();
  seed *= x.GetTicks();
  srand(seed);

  // Fulup: force floating point to use dot as separation.
  // This needs to be set early to catch numerics in config file.
  setlocale(LC_NUMERIC, "C");

  g_start_time = wxDateTime::Now();

  g_loglast_time = g_start_time;
  g_loglast_time.MakeGMT();
  g_loglast_time.Subtract(
      wxTimeSpan(0, 29, 0, 0));  // give 1 minute for GPS to get a fix

  AnchorPointMinDist = 5.0;

  //      Init the private memory manager
  malloc_max = 0;

  //      Record initial memory status
  GetMemoryStatus(&g_mem_total, &g_mem_initial);

  // Set up default FONT encoding, which should have been done by wxWidgets some
  // time before this......
  wxFont temp_font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                   wxFONTWEIGHT_NORMAL, FALSE, wxString(_T("")),
                   wxFONTENCODING_SYSTEM);
  temp_font.SetDefaultEncoding(wxFONTENCODING_SYSTEM);

  //      Establish Log File location
  if (!g_Platform->InitializeLogFile()) return false;

#ifdef __WXMSW__

    //  Un-comment the following to establish a separate console window as a
    //  target for printf() in Windows
    //     RedirectIOToConsole();

#endif

  //      Send init message
  wxLogMessage(_T("\n\n________\n"));

  wxDateTime now = wxDateTime::Now();
  LOG_INFO("------- OpenCPN version %s restarted at %s -------\n", VERSION_FULL,
           now.FormatISODate().mb_str().data());
  wxLogLevel level = wxLog::GetLogLevel();
  LOG_INFO("Using loglevel %s", OcpnLog::level2str(level).c_str());

  wxString wxver(wxVERSION_STRING);
  wxver.Prepend(_T("wxWidgets version: "));

  wxPlatformInfo platforminfo = wxPlatformInfo::Get();

  wxString os_name;
#ifndef __ANDROID__
  os_name = platforminfo.GetOperatingSystemIdName();
#else
  os_name = platforminfo.GetOperatingSystemFamilyName();
#endif

  wxString platform = os_name + _T(" ") + platforminfo.GetArchName() + _T(" ") +
                      platforminfo.GetPortIdName();

  wxLogMessage(wxver + _T(" ") + platform);

  ::wxGetOsVersion(&osMajor, &osMinor);
  wxString osVersionMsg;
  osVersionMsg.Printf(_T("OS Version reports as:  %d.%d"), osMajor, osMinor);
  wxLogMessage(osVersionMsg);

  wxLogMessage(_T("MemoryStatus:  mem_total: %d mb,  mem_initial: %d mb"),
               g_mem_total / 1024, g_mem_initial / 1024);

  OCPN_OSDetail *detail = g_Platform->GetOSDetail();
  wxString msgplat;
  wxString like0;
  if (!detail->osd_names_like.empty())
    like0 = detail->osd_names_like[0].c_str();
  msgplat.Printf("OCPN_OSDetail:  %s ; %s ; %s ; %s ; %s",
                 detail->osd_arch.c_str(), detail->osd_name.c_str(),
                 detail->osd_version.c_str(), detail->osd_ID.c_str(),
                 like0.mb_str());
  wxLogMessage(msgplat);

  wxString imsg = _T("SData_Locn is ");
  imsg += g_Platform->GetSharedDataDir();
  wxLogMessage(imsg);

  //    Initialize embedded PNG icon graphics
  ::wxInitAllImageHandlers();

#ifdef __WXQT__
  //  Now we can configure the Qt StyleSheets, if present
  prepareAndroidStyleSheets();
#endif

  //      Create some static strings
  pInit_Chart_Dir = new wxString();

  //  Establish an empty ChartCroupArray
  g_pGroupArray = new ChartGroupArray;

  imsg = _T("PrivateDataDir is ");
  imsg += g_Platform->GetPrivateDataDir();
  wxLogMessage(imsg);

  //      Create an array string to hold repeating messages, so they don't
  //      overwhelm the log
  pMessageOnceArray = new wxArrayString;

  //      Init the Route Manager

 g_pRouteMan = new Routeman(RoutePropDlg::GetDlgCtx(), RoutemanGui::GetDlgCtx(),
                   NMEALogWindow::GetInstance());

  //      Init the Selectable Route Items List
  pSelect = new Select();
  pSelect->SetSelectPixelRadius(12);

  //      Init the Selectable Tide/Current Items List
  pSelectTC = new Select();
  //  Increase the select radius for tide/current stations
  pSelectTC->SetSelectPixelRadius(25);

  //      Init the Selectable AIS Target List
  pSelectAIS = new Select();
  pSelectAIS->SetSelectPixelRadius(12);

  //      Initially AIS display is always on
  g_bShowAIS = true;
  g_pais_query_dialog_active = NULL;

  //      Who am I?
  g_hostname = ::wxGetHostName();
  if(g_hostname.IsEmpty())
     g_hostname = wxGetUserName();
#ifdef __ANDROID__
  androidGetDeviceInfo();
  g_hostname = wxString("Android-") + g_android_Device_Model;
  g_hostname.Replace(" ", "-", true);
#endif

  //      A Portabel need a unique mDNS data hostname to share routes.
  if (g_bportable) {
    wxString p("Portable-");
    g_hostname = p + g_hostname;
  }


  //      Initialize some lists
  //    Layers
  pLayerList = new LayerList;
  //  Routes
  pRouteList = new RouteList;

  //      (Optionally) Capture the user and file(effective) ids
  //  Some build environments may need root privileges for hardware
  //  port I/O, as in the NMEA data input class.  Set that up here.

#ifndef __WXMSW__
#ifdef PROBE_PORTS__WITH_HELPER
  user_user_id = getuid();
  file_user_id = geteuid();
#endif
#endif

  bool b_initial_load = false;

  wxFileName config_test_file_name(g_Platform->GetConfigFileName());
  if (config_test_file_name.FileExists())
    wxLogMessage(_T("Using existing Config_File: ") +
                 g_Platform->GetConfigFileName());
  else {
    {
      wxLogMessage(_T("Creating new Config_File: ") +
                   g_Platform->GetConfigFileName());

      b_initial_load = true;

      if (true !=
          config_test_file_name.DirExists(config_test_file_name.GetPath()))
        if (!config_test_file_name.Mkdir(config_test_file_name.GetPath()))
          wxLogMessage(_T("Cannot create config file directory for ") +
                       g_Platform->GetConfigFileName());
    }
  }

  //      Open/Create the Config Object
  pConfig = g_Platform->GetConfigObject();
  InitBaseConfig(pConfig);
  pConfig->LoadMyConfig();

  //  Override for some safe and nice default values if the config file was
  //  created from scratch
  if (b_initial_load) g_Platform->SetDefaultOptions();

  if (g_config_wizard || b_initial_load) {
    FirstUseWizImpl wiz(gFrame, pConfig);
    auto res = wiz.Run();
    if(res) {
      g_NeedDBUpdate = 2;
    }
  }

  g_Platform->applyExpertMode(g_bUIexpert);

  // Now initialize UI Style.
  g_StyleManager = new ocpnStyle::StyleManager();

  //     if(g_useMUI)
  //         g_uiStyle = _T("MUI_flat");

  g_StyleManager->SetStyle(_T("MUI_flat"));
  if (!g_StyleManager->IsOK()) {
    wxString msg = _("Failed to initialize the user interface. ");
    msg << _("OpenCPN cannot start. ");
    msg << _("The necessary configuration files were not found. ");
    msg << _("See the log file at ") << g_Platform->GetLogFileName()
        << _(" for details.") << _T("\n\n");
    msg << g_Platform->GetSharedDataDir();

    wxMessageDialog w(NULL, msg, _("Failed to initialize the user interface. "),
                      wxCANCEL | wxICON_ERROR);
    w.ShowModal();
    exit(EXIT_FAILURE);
  }

  if (g_useMUI) {
    ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
    if (style) style->chartStatusWindowTransparent = true;
  }

  //      Init the WayPoint Manager
  pWayPointMan = NULL;

  g_display_size_mm = wxMax(50, g_Platform->GetDisplaySizeMM());
  wxString msg;
  msg.Printf(_T("Detected display size (horizontal): %d mm"),
             (int)g_display_size_mm);
  wxLogMessage(msg);

  // User override....
  if (g_config_display_size_manual && g_config_display_size_mm.size() > g_current_monitor && g_config_display_size_mm[g_current_monitor] > 0) {
    g_display_size_mm = g_config_display_size_mm[g_current_monitor];
    wxString msg;
    msg.Printf(_T("Display size (horizontal) config override: %d mm"),
               (int)g_display_size_mm);
    wxLogMessage(msg);
    g_Platform->SetDisplaySizeMM(g_current_monitor, g_display_size_mm);
  }

  g_display_size_mm = wxMax(50, g_display_size_mm);

  if (g_btouch) {
    int SelectPixelRadius = 50;

    pSelect->SetSelectPixelRadius(SelectPixelRadius);
    pSelectTC->SetSelectPixelRadius(wxMax(25, SelectPixelRadius));
    pSelectAIS->SetSelectPixelRadius(SelectPixelRadius);
  }

  //  Is this the first run after a clean installation?
  if (!n_NavMessageShown) g_bFirstRun = true;

    //  Now we can set the locale
    //    using wxWidgets/gettext methodology....

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)

  //  Where are the opencpn.mo files?
  g_Platform->SetLocaleSearchPrefixes();

  wxString def_lang_canonical = g_Platform->GetDefaultSystemLocale();

  imsg = _T("System default Language:  ") + def_lang_canonical;
  wxLogMessage(imsg);

  wxString cflmsg = _T("Config file language:  ") + g_locale;
  wxLogMessage(cflmsg);

  if (g_locale.IsEmpty()) {
    g_locale = def_lang_canonical;
    cflmsg = _T("Config file language empty, using system default:  ") + g_locale;
    wxLogMessage(cflmsg);
  }

  //  Make any adjustments necessary
  g_locale = g_Platform->GetAdjustedAppLocale();
  cflmsg = _T("Adjusted App language:  ") + g_locale;
  wxLogMessage(cflmsg);

  // Set the desired locale
  g_Platform->ChangeLocale(g_locale, plocale_def_lang, &plocale_def_lang);

  imsg = _T("Opencpn language set to:  ");
  imsg += g_locale;
  wxLogMessage(imsg);

  //  French language locale is assumed to include the AZERTY keyboard
  //  This applies to either the system language, or to OpenCPN language
  //  selection
  if (g_locale == _T("fr_FR")) g_b_assume_azerty = true;
#else
  wxLogMessage(_T("wxLocale support not available"));
#endif

  // Instantiate and initialize the Config Manager
  ConfigMgr::Get();

  // Is this an upgrade?
  wxString vs = wxString("Version ") + VERSION_FULL + " Build " + VERSION_DATE;
  g_bUpgradeInProcess = (vs != g_config_version_string);

  g_Platform->SetUpgradeOptions(vs, g_config_version_string);

  //  log deferred log restart message, if it exists.
  if (!g_Platform->GetLargeLogMessage().IsEmpty())
    wxLogMessage(g_Platform->GetLargeLogMessage());

    //  Validate OpenGL functionality, if selected
#ifndef ocpnUSE_GL
  g_bdisable_opengl = true;
  ;
#endif

  if (g_bdisable_opengl) g_bopengl = false;

#if defined(__linux__) && !defined(__ANDROID__)
  if (g_bSoftwareGL) setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
#endif

  //FIXMW (dave) move to frame
  //g_bTransparentToolbarInOpenGLOK = isTransparentToolbarInOpenGLOK();

  // On Windows platforms, establish a default cache managment policy
  // as allowing OpenCPN a percentage of available physical memory,
  // not to exceed 1 GB
  // Note that this logic implies that Windows platforms always use
  // the memCacheLimit policy, and never use the fallback nCacheLimit policy
#ifdef __WXMSW__
  if (0 == g_memCacheLimit) g_memCacheLimit = (int)(g_mem_total * 0.5);
  g_memCacheLimit =
      wxMin(g_memCacheLimit, 1024 * 1024);  // math in kBytes, Max is 1 GB
#else
  // All other platforms will use the nCacheLimit policy
  // sinc on linux it is impossible to accurately measure the application memory
  // footprint without expensive methods such as malloc/free tracking, and such

  g_memCacheLimit = 0;
  if (0 == g_nCacheLimit)  // allow config file override
    g_nCacheLimit = CACHE_N_LIMIT_DEFAULT;
#endif

  //      Establish location and name of chart database
  ChartListFileName = newPrivateFileName(g_Platform->GetPrivateDataDir(),
                                         "chartlist.dat", "CHRTLIST.DAT");

  //      Establish location and name of AIS MMSI -> Target Name mapping
  AISTargetNameFileName = newPrivateFileName(g_Platform->GetPrivateDataDir(),
                                             "mmsitoname.csv", "MMSINAME.CSV");

  //      Establish guessed location of chart tree
  if (pInit_Chart_Dir->IsEmpty()) {
    wxStandardPaths &std_path = g_Platform->GetStdPaths();

    if (!g_bportable)
#ifndef __ANDROID__
      pInit_Chart_Dir->Append(std_path.GetDocumentsDir());
#else
      pInit_Chart_Dir->Append(androidGetExtStorageDir());
#endif
  }

  InitRestListeners();

  //      Establish the GSHHS Dataset location
  gDefaultWorldMapLocation = "gshhs";
  gDefaultWorldMapLocation.Prepend(g_Platform->GetSharedDataDir());
  gDefaultWorldMapLocation.Append(wxFileName::GetPathSeparator());
  if (gWorldMapLocation == wxEmptyString) {
    gWorldMapLocation = gDefaultWorldMapLocation;
  }

  //  Check the global Tide/Current data source array
  //  If empty, preset default (US + ROW) data sources
  wxString default_tcdata0 =
      (g_Platform->GetSharedDataDir() + _T("tcdata") +
       wxFileName::GetPathSeparator() + _T("harmonics-dwf-20210110-free.tcd"));
  wxString default_tcdata1 =
      (g_Platform->GetSharedDataDir() + _T("tcdata") +
       wxFileName::GetPathSeparator() + _T("HARMONICS_NO_US.IDX"));

  if (TideCurrentDataSet.empty()) {
    TideCurrentDataSet.push_back(g_Platform->NormalizePath(default_tcdata0).ToStdString());
    TideCurrentDataSet.push_back(g_Platform->NormalizePath(default_tcdata1).ToStdString());
  }

  //  Check the global AIS alarm sound file
  //  If empty, preset default
  if (g_sAIS_Alert_Sound_File.IsEmpty()) {
    wxString default_sound =
        (g_Platform->GetSharedDataDir() + _T("sounds") +
         wxFileName::GetPathSeparator() + _T("2bells.wav"));
    g_sAIS_Alert_Sound_File = g_Platform->NormalizePath(default_sound);
  }

  gpIDXn = 0;

  g_Platform->Initialize_2();

  //  Set up the frame initial visual parameters
  //      Default size, resized later
  wxSize new_frame_size(-1, -1);
  int cx, cy, cw, ch;
  ::wxClientDisplayRect(&cx, &cy, &cw, &ch);

  InitializeUserColors();

  auto style = g_StyleManager->GetCurrentStyle();
  auto bitmap = new wxBitmap(style->GetIcon("default_pi", 32, 32));
  if (bitmap->IsOk())
    PluginLoader::getInstance()->SetPluginDefaultIcon(bitmap);
  else
    wxLogWarning("Cannot initiate plugin default jigsaw icon.");


  if ((g_nframewin_x > 100) && (g_nframewin_y > 100) && (g_nframewin_x <= cw) &&
      (g_nframewin_y <= ch))
    new_frame_size.Set(g_nframewin_x, g_nframewin_y);
  else
    new_frame_size.Set(cw * 7 / 10, ch * 7 / 10);

  //  Try to detect any change in physical screen configuration
  //  This can happen when drivers are changed, for instance....
  //  and can confuse the WUI layout perspective stored in the config file.
  //  If detected, force a nominal window size and position....
  if ((g_lastClientRectx != cx) || (g_lastClientRecty != cy) ||
      (g_lastClientRectw != cw) || (g_lastClientRecth != ch)) {
    new_frame_size.Set(cw * 7 / 10, ch * 7 / 10);
    g_bframemax = false;
  }

  g_lastClientRectx = cx;
  g_lastClientRecty = cy;
  g_lastClientRectw = cw;
  g_lastClientRecth = ch;

  //  Validate config file position
  wxPoint position(0, 0);
  wxSize dsize = wxGetDisplaySize();

#ifdef __WXMAC__
  g_nframewin_posy = wxMax(g_nframewin_posy, 22);
#endif

  if ((g_nframewin_posx < dsize.x) && (g_nframewin_posy < dsize.y))
    position = wxPoint(g_nframewin_posx, g_nframewin_posy);

#ifdef __WXMSW__
  //  Support MultiMonitor setups which can allow negative window positions.
  RECT frame_rect;
  frame_rect.left = position.x;
  frame_rect.top = position.y;
  frame_rect.right = position.x + new_frame_size.x;
  frame_rect.bottom = position.y + new_frame_size.y;

  //  If the requested frame window does not intersect any installed monitor,
  //  then default to simple primary monitor positioning.
  if (NULL == MonitorFromRect(&frame_rect, MONITOR_DEFAULTTONULL))
    position = wxPoint(10, 10);
#endif

#ifdef __WXOSX__
  //  Support MultiMonitor setups which can allow negative window positions.
  const wxPoint ptScreen(position.x, position.y);
  const int displayIndex = wxDisplay::GetFromPoint(ptScreen);

  if (displayIndex == wxNOT_FOUND)
    position = wxPoint(10, 30);
#endif

  g_nframewin_posx = position.x;
  g_nframewin_posy = position.y;

#ifdef __ANDROID__
  wxSize asz = getAndroidDisplayDimensions();
  ch = asz.y;
  cw = asz.x;
  //    qDebug() << cw << ch;

  if ((cw > 200) && (ch > 200))
    new_frame_size.Set(cw, ch);
  else
    new_frame_size.Set(800, 400);
#endif

  //  For Windows and GTK, provide the expected application Minimize/Close bar
  long app_style = wxDEFAULT_FRAME_STYLE;
  app_style |= wxWANTS_CHARS;

  // Create the main frame window

  // Strip the commit SHA number from the string to be shown in frame title.
  wxString short_version_name = wxString(PACKAGE_VERSION).BeforeFirst('+');
  wxString myframe_window_title = wxString(wxT("OpenCPN ") + short_version_name);

  if (g_bportable) {
    myframe_window_title += _(" -- [Portable(-p) executing from ");
    myframe_window_title += g_Platform->GetHomeDir();
    myframe_window_title += _T("]");
  }

  wxString fmsg;
  fmsg.Printf(_T("Creating MyFrame...size(%d, %d)  position(%d, %d)"),
              new_frame_size.x, new_frame_size.y, position.x, position.y);
  wxLogMessage(fmsg);

  gFrame = new MyFrame(NULL, myframe_window_title, position, new_frame_size,
                       app_style);  // Gunther

  //  Do those platform specific initialization things that need gFrame
  g_Platform->Initialize_3();

  //  Initialize the Plugin Manager
  g_pi_manager = new PlugInManager(gFrame);

  // g_pauimgr = new wxAuiManager;
  g_pauimgr = new OCPN_AUIManager;
  g_pauidockart = new wxAuiDefaultDockArt;
  g_pauimgr->SetArtProvider(g_pauidockart);
  g_pauimgr->SetDockSizeConstraint(.9, .9);

  // g_pauimgr->SetFlags(g_pauimgr->GetFlags() | wxAUI_MGR_LIVE_RESIZE);

  // tell wxAuiManager to manage the frame
  g_pauimgr->SetManagedWindow(gFrame);

  gFrame->CreateCanvasLayout();

  // gFrame->RequestNewMasterToolbar( true );

  gFrame->SetChartUpdatePeriod();  // Reasonable default

  gFrame->Enable();

  gFrame->GetPrimaryCanvas()->SetFocus();

  pthumbwin = new ThumbWin(gFrame->GetPrimaryCanvas());

  gFrame->ApplyGlobalSettings(false);  // done once on init with resize

  gFrame->SetAllToolbarScale();

  // Show the frame
  gFrame->Show(TRUE);
  Yield();      // required for Gnome 45

  gFrame->SetAndApplyColorScheme(global_color_scheme);

  if (g_bframemax) gFrame->Maximize(true);

#ifdef __ANDROID__
  if (g_bresponsive && (gFrame->GetPrimaryCanvas()->GetPixPerMM() > 4.0))
    gFrame->Maximize(true);
#endif

  //  Yield to pick up the OnSize() calls that result from Maximize()
  Yield();

  //   Build the initial chart dir array
  ArrayOfCDI ChartDirArray;
  pConfig->LoadChartDirArray(ChartDirArray);

  //  Windows installer may have left hints regarding the initial chart dir
  //  selection
#ifdef __WXMSW__
  if (g_bFirstRun && (ChartDirArray.GetCount() == 0)) {
    int ndirs = 0;

    wxRegKey RegKey(wxString(_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN")));
    if (RegKey.Exists()) {
      wxLogMessage(
          _("Retrieving initial Chart Directory set from Windows Registry"));
      wxString dirs;
      RegKey.QueryValue(wxString(_T("ChartDirs")), dirs);

      wxStringTokenizer tkz(dirs, _T(";"));
      while (tkz.HasMoreTokens()) {
        wxString token = tkz.GetNextToken();

        ChartDirInfo cdi;
        cdi.fullpath = token.Trim();
        cdi.magic_number = _T("");

        ChartDirArray.Add(cdi);
        ndirs++;
      }
    }

    if (g_bportable) {
      ChartDirInfo cdi;
      cdi.fullpath = _T("charts");
      cdi.fullpath.Prepend(g_Platform->GetSharedDataDir());
      cdi.magic_number = _T("");
      ChartDirArray.Add(cdi);
      ndirs++;
    }

    if (ndirs) pConfig->UpdateChartDirs(ChartDirArray);
  }
#endif

  //    If the ChartDirArray is empty at this point, any existing chart database
  //    file must be declared invalid, So it is best to simply delete it if
  //    present.
  //    TODO  There is a possibility of recreating the dir list from the
  //    database itself......

  if (!ChartDirArray.GetCount())
    if (::wxFileExists(ChartListFileName)) ::wxRemoveFile(ChartListFileName);

  //      Try to load the current chart list Data file
  ChartData = new ChartDB();
  if (g_NeedDBUpdate == 0 && !ChartData->LoadBinary(ChartListFileName, ChartDirArray)) {
    g_NeedDBUpdate = 1;
  }

  //  Verify any saved chart database startup index
  if (g_restore_dbindex >= 0) {
    if (ChartData->GetChartTableEntries() == 0)
      g_restore_dbindex = -1;

    else if (g_restore_dbindex > (ChartData->GetChartTableEntries() - 1))
      g_restore_dbindex = 0;
  }

  //  Apply the inital Group Array structure to the chart database
  ChartData->ApplyGroupArray(g_pGroupArray);

  //      All set to go.....

  // Process command line option to rebuild cache
#ifdef ocpnUSE_GL
  extern ocpnGLOptions g_GLOptions;

  if (g_rebuild_gl_cache && g_bopengl && g_GLOptions.m_bTextureCompression &&
      g_GLOptions.m_bTextureCompressionCaching) {
    gFrame->ReloadAllVP();  //  Get a nice chart background loaded

    //      Turn off the toolbar as a clear signal that the system is busy right
    //      now.
    // Note: I commented this out because the toolbar never comes back for me
    // and is unusable until I restart opencpn without generating the cache
    //        if( g_MainToolbar )
    //            g_MainToolbar->Hide();

    if (g_glTextureManager) g_glTextureManager->BuildCompressedCache();
  }
#endif

  //FIXME (dave)
  // move method to frame
  //if (g_parse_all_enc) ParseAllENC(gFrame);

  //      establish GPS timeout value as multiple of frame timer
  //      This will override any nonsense or unset value from the config file
  if ((gps_watchdog_timeout_ticks > 60) || (gps_watchdog_timeout_ticks <= 0))
    gps_watchdog_timeout_ticks = (GPS_TIMEOUT_SECONDS * 1000) / TIMER_GFRAME_1;

  wxString dogmsg;
  dogmsg.Printf(_T("GPS Watchdog Timeout is: %d sec."),
                gps_watchdog_timeout_ticks);
  wxLogMessage(dogmsg);

  sat_watchdog_timeout_ticks = gps_watchdog_timeout_ticks;

  g_priSats = 99;

  //  Most likely installations have no ownship heading information
  g_bVAR_Rx = false;

  //  Start up a new track if enabled in config file
  if (g_bTrackCarryOver) g_bDeferredStartTrack = true;

  pAnchorWatchPoint1 = NULL;
  pAnchorWatchPoint2 = NULL;

  Yield();

  gFrame->DoChartUpdate();

  FontMgr::Get()
      .ScrubList();  // Clean the font list, removing nonsensical entries

  gFrame->ReloadAllVP();  // once more, and good to go

  gFrame->Refresh(false);
  gFrame->Raise();

  gFrame->GetPrimaryCanvas()->Enable();
  gFrame->GetPrimaryCanvas()->SetFocus();

  //  This little hack fixes a problem seen with some UniChrome OpenGL drivers
  //  We need a deferred resize to get glDrawPixels() to work right.
  //  So we set a trigger to generate a resize after 5 seconds....
  //  See the "UniChrome" hack elsewhere
#ifdef ocpnUSE_GL
  if (!g_bdisable_opengl) {
    glChartCanvas *pgl =
        (glChartCanvas *)gFrame->GetPrimaryCanvas()->GetglCanvas();
    if (pgl &&
        (pgl->GetRendererString().Find(_T("UniChrome")) != wxNOT_FOUND)) {
      gFrame->m_defer_size = gFrame->GetSize();
      gFrame->SetSize(gFrame->m_defer_size.x - 10, gFrame->m_defer_size.y);
      g_pauimgr->Update();
      gFrame->m_bdefer_resize = true;
    }
  }
#endif

  // Horrible Hack (tm): Make sure the RoutePoint destructor can invoke
  // glDeleteTextures. Truly awful.
#ifdef ocpnUSE_GL
  if (g_bopengl)
    RoutePoint::delete_gl_textures =
      [](unsigned n, const unsigned* texts) { glDeleteTextures(n, texts); };
#else
  RoutePoint::delete_gl_textures = [](unsigned n, const unsigned* texts) { };
#endif


  if (g_start_fullscreen) gFrame->ToggleFullScreen();

#ifdef __ANDROID__
  //  We need a resize to pick up height adjustment after building android
  //  ActionBar
  gFrame->SetSize(getAndroidDisplayDimensions());
  androidSetFollowTool(gFrame->GetPrimaryCanvas()->m_bFollow ? 1 : 0, true);
#endif

  gFrame->Raise();
  gFrame->GetPrimaryCanvas()->Enable();
  gFrame->GetPrimaryCanvas()->SetFocus();

  // Setup Tides/Currents to settings present at last shutdown
  // TODO
  //     gFrame->ShowTides( g_bShowTide );
  //     gFrame->ShowCurrents( g_bShowCurrent );

  //      Start up the ticker....
  gFrame->FrameTimer1.Start(TIMER_GFRAME_1, wxTIMER_CONTINUOUS);

  //      Start up the ViewPort Rotation angle Averaging Timer....
  gFrame->FrameCOGTimer.Start(2000, wxTIMER_CONTINUOUS);

  //    wxLogMessage( wxString::Format(_T("OpenCPN Initialized in %ld ms."),
  //    init_sw.Time() ) );

  OCPNPlatform::Initialize_4();

#ifdef __ANDROID__
  androidHideBusyIcon();
#endif
  wxLogMessage(
      wxString::Format(_("OpenCPN Initialized in %ld ms."), init_sw.Time()));

  wxMilliSleep(500);

#ifdef __ANDROID__
  //  We defer the startup message to here to allow the app frame to be
  //  contructed, thus avoiding a dialog with NULL parent which might not work
  //  on some devices.
  if (!n_NavMessageShown || (vs != g_config_version_string) ||
      (g_AndroidVersionCode != androidGetVersionCode())) {
    // qDebug() << "Showing NavWarning";
    wxMilliSleep(500);

    if (wxID_CANCEL == ShowNavWarning()) {
      qDebug() << "Closing due to NavWarning Cancel";
      gFrame->Close();
      androidTerminate();
      return true;
    }

    n_NavMessageShown = 1;
  }

  // Finished with upgrade checking, so persist the currect Version Code
  g_AndroidVersionCode = androidGetVersionCode();
  qDebug() << "Persisting Version Code: " << g_AndroidVersionCode;
#else
  //  Send the Welcome/warning message if it has never been sent before,
  //  or if the version string has changed at all
  //  We defer until here to allow for localization of the message
  if (!n_NavMessageShown || (vs != g_config_version_string)) {
    if (wxID_CANCEL == ShowNavWarning()) return false;
    n_NavMessageShown = 1;
  }
#endif

  // As an a.e. Raspberry does not have a hardwareclock we will have some
  // problems with date/time setting
  g_bHasHwClock = true;  // by default most computers do have a hwClock
#if defined(__UNIX__) && !defined(__ANDROID__)
  struct stat buffer;
  g_bHasHwClock =
      ((stat("/dev/rtc", &buffer) == 0) || (stat("/dev/rtc0", &buffer) == 0) ||
       (stat("/dev/misc/rtc", &buffer) == 0));
#endif

  g_config_version_string = vs;

  // The user accepted the "not for navigation" nag, so persist it here...
  pConfig->UpdateSettings();

  // Start delayed initialization chain after some milliseconds
  gFrame->InitTimer.Start(5, wxTIMER_CONTINUOUS);

  g_pauimgr->Update();

  for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
    ConnectionParams *cp = TheConnectionParams()->Item(i);
    if (cp->bEnabled) {
      if (cp->GetDSPort().Contains("Serial")) {
        std::string port(cp->Port.ToStdString());
        CheckSerialAccess(gFrame, port);
      }
    }
  }
  CheckDongleAccess(gFrame);

  // Initialize the CommBridge
  m_comm_bridge.Initialize();

  std::vector<std::string> ipv4_addrs = get_local_ipv4_addresses();

  //If network connection is available, start the server and mDNS client
  if (ipv4_addrs.size()) {
    std::string ipAddr = ipv4_addrs[0];

    wxString data_dir = g_Platform->GetPrivateDataDir();
    if (data_dir.Last() != wxFileName::GetPathSeparator())
      data_dir.Append(wxFileName::GetPathSeparator());

    make_certificate(ipAddr, data_dir.ToStdString());

    m_rest_server.StartServer(fs::path(data_dir.ToStdString()));
    StartMDNSService(g_hostname.ToStdString(),
                     "opencpn-object-control-service", 8000);
  }
  return TRUE;
}

int MyApp::OnExit() {

  wxLogMessage(_T("opencpn::MyApp starting exit."));
  m_checker.OnExit();
  m_usb_watcher.Stop();
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

  if (bGPSValid) {
    wxString data;
    data.Printf(_T("OFF: Lat %10.5f Lon %10.5f "), gLat, gLon);
    navmsg += data;

    wxString cog;
    if (std::isnan(gCog))
      cog.Printf(_T("COG ----- "));
    else
      cog.Printf(_T("COG %10.5f "), gCog);

    wxString sog;
    if (std::isnan(gSog))
      sog.Printf(_T("SOG -----  "));
    else
      sog.Printf(_T("SOG %6.2f ") + getUsrSpeedUnit(), toUsrSpeed(gSog));

    navmsg += cog;
    navmsg += sog;

  } else {
    wxString data;
    data.Printf(_T("OFF: Lat %10.5f Lon %10.5f"), gLat, gLon);
    navmsg += data;
  }
  wxLogMessage(navmsg);
  g_loglast_time = lognow;

  if (ptcmgr) delete ptcmgr;

  delete pConfig;
  delete pSelect;
  delete pSelectTC;
  delete pSelectAIS;

  delete ps52plib;
  delete g_SencThreadManager;

  if (g_pGroupArray) {
    for (unsigned int igroup = 0; igroup < g_pGroupArray->GetCount();
         igroup++) {
      delete g_pGroupArray->Item(igroup);
    }

    g_pGroupArray->Clear();
    delete g_pGroupArray;
  }

  wxLogMessage(_T("opencpn::MyApp exiting cleanly...\n"));
  wxLog::FlushActive();

  g_Platform->CloseLogFile();

  delete pInit_Chart_Dir;

  for (Track* track : g_TrackList) {
    delete track;
  }
  g_TrackList.clear();

  delete g_pRouteMan;
  delete pWayPointMan;

  delete pMessageOnceArray;

  DeInitializeUserColors();

  delete pLayerList;

  delete m_pRegistrarMan;
  CSVDeaccess(NULL);

  delete g_StyleManager;

#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL
  if (s_glu_dll_ready) FreeLibrary(s_hGLU_DLL);  // free the glu32.dll
#endif
#endif
#endif

    //      Restore any changed system colors


#ifdef __WXMSW__
void RestoreSystemColors(void);
  RestoreSystemColors();
#endif

#ifdef __MSVC__LEAK
  DeInitAllocCheck();
#endif

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
  if (plocale_def_lang) delete plocale_def_lang;
#endif

  FontMgr::Shutdown();

  g_Platform->OnExit_2();
  safe_mode::clear_check();
  delete g_Platform;

  return TRUE;
}

#ifdef LINUX_CRASHRPT
void MyApp::OnFatalException() { g_crashprint.Report(); }
#endif

//----------------------------------------------------------------------------------------------------------
//      Application-wide CPL Error handler
//----------------------------------------------------------------------------------------------------------
void MyCPLErrorHandler(CPLErr eErrClass, int nError, const char *pszErrorMsg)

{
  char msg[256];

  if (eErrClass == CE_Debug)
    snprintf(msg, 255, "CPL: %s", pszErrorMsg);
  else if (eErrClass == CE_Warning)
    snprintf(msg, 255, "CPL Warning %d: %s", nError, pszErrorMsg);
  else
    snprintf(msg, 255, "CPL ERROR %d: %s", nError, pszErrorMsg);

  wxString str(msg, wxConvUTF8);
  wxLogMessage(str);
}
