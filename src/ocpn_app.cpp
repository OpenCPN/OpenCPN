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

#include <wx/apptrait.h>
#include <wx/arrimpl.cpp>
#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/clrpicker.h>
#include <wx/dialog.h>
#include <wx/dialog.h>
#include <wx/dir.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/ipc.h>
#include <wx/jsonreader.h>
#include <wx/listctrl.h>
#include <wx/printdlg.h>
#include <wx/print.h>
#include <wx/progdlg.h>
#include <wx/settings.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/cmdline.h>

#include "ocpn_app.h"
#include "ocpn_frame.h"

#include "AboutFrameImpl.h"
#include "about.h"
#include "AISTargetAlertDialog.h"
#include "AISTargetQueryDialog.h"
#include "ais_info_gui.h"

#include "CanvasConfig.h"
#include "chartdb.h"
#include "chcanv.h"
#include "cm93.h"
#include "concanv.h"
#include "config.h"
#include "config_vars.h"
#include "ConfigMgr.h"
#include "DetailSlider.h"
#include "dychart.h"
#include "FontMgr.h"
#include "GoToPositionDialog.h"
#include "gdal/cpl_csv.h"
#include "glTexCache.h"
#include "Layer.h"
#include "logger.h"
#include "MarkInfo.h"
#include "multiplexer.h"
#include "nav_object_database.h"
#include "navutil.h"
#include "navutil_base.h"
#include "OCPN_AUIManager.h"
#include "OCPNPlatform.h"
#include "options.h"
#include "own_ship.h"
#include "plugin_handler.h"
#include "route.h"
#include "routemanagerdialog.h"
#include "routeman.h"
#include "RoutePropDlgImpl.h"
#include "s52plib.h"
#include "s57chart.h"
#include "S57QueryDialog.h"
#include "safe_mode.h"
#include "select.h"
#include "SoundFactory.h"
#include "styles.h"
#include "tcmgr.h"
#include "thumbwin.h"
#include "track.h"
#include "TrackPropDlg.h"
#include "AISTargetListDialog.h"
#include "comm_n0183_output.h"
#include "comm_bridge.h"
#include "certificates.h"
#include "mDNS_query.h"

//#include "usb_devices.h"
//#include "comm_drv_registry.h"
//#include "comm_navmsg_bus.h"
//#include "N2KParser.h"
//#include "comm_util.h"
#include "comm_vars.h"

#include "mDNS_service.h"

#ifdef __linux__
#include "udev_rule_mgr.h"
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#ifdef __WXOSX__
#include "macutils.h"
#endif

#ifdef __WXMSW__
#include "garmin_protocol_mgr.h"  // Used for port probing on Windows
void RedirectIOToConsole();
#endif

#if defined(__WXMSW__) && defined (__MSVC__LEAK)
#include "Stackwalker.h"
#endif

#ifdef LINUX_CRASHRPT
#include "crashprint.h"
#endif

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#else
#include "serial/serial.h"
#endif

static void UpdatePositionCalculatedSogCog();


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
BasePlatform *g_BasePlatform;   // points to g_platform, handles brain-dead MS linker.

wxString g_vs;
bool g_bFirstRun;
bool g_bUpgradeInProcess;

bool g_bPauseTest;

wxString g_compatOS;
wxString g_compatOsVersion;

int g_unit_test_1;
int g_unit_test_2;
bool g_start_fullscreen;
bool g_rebuild_gl_cache;
bool g_parse_all_enc;

// Files specified on the command line, if any.
wxVector<wxString> g_params;

MyFrame *gFrame;

ConsoleCanvas *console;

MyConfig *pConfig;
ChartBase *Current_Vector_Ch;
ChartDB *ChartData;
wxString *pdir_list[20];
int g_restore_stackindex;
int g_restore_dbindex;
double g_ChartNotRenderScaleFactor;
int g_nDepthUnitDisplay;

RouteList *pRouteList;
std::vector<Track*> g_TrackList;
LayerList *pLayerList;
bool g_bIsNewLayer;
int g_LayerIdx;
bool g_bLayerViz;

Select *pSelect;
Select *pSelectTC;
Select *pSelectAIS;

Routeman *g_pRouteMan;
WayPointman *pWayPointMan;
MarkInfoDlg *g_pMarkInfoDialog;
RoutePropDlgImpl *pRoutePropDialog;
TrackPropDlg *pTrackPropDialog;
RouteManagerDialog *pRouteManagerDialog;
GoToPositionDialog *pGoToPositionDialog;

double vLat, vLon;
double initial_scale_ppm, initial_rotation;

int g_nbrightness = 100;

bool bDBUpdateInProgress;

ThumbWin *pthumbwin;
TCMgr *ptcmgr;

bool g_bshowToolbar = true;
bool g_bexpert = true;
bool g_bBasicMenus = false;

bool bDrawCurrentValues;

wxString ChartListFileName;
wxString AISTargetNameFileName;
wxString gWorldMapLocation, gDefaultWorldMapLocation;
wxString *pInit_Chart_Dir;
wxString g_winPluginDir;  // Base plugin directory on Windows.
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
int g_mem_total, g_mem_used, g_mem_initial;

bool s_bSetSystemTime;

static unsigned int malloc_max;

wxDateTime g_start_time;
wxDateTime g_loglast_time;
static OcpnSound *_bells_sounds[] = {SoundFactory(), SoundFactory()};
std::vector<OcpnSound *> bells_sound(_bells_sounds, _bells_sounds + 2);

OcpnSound *g_anchorwatch_sound = SoundFactory();
wxString g_anchorwatch_sound_file;
wxString g_DSC_sound_file;
wxString g_SART_sound_file;
wxString g_AIS_sound_file;

RoutePoint *pAnchorWatchPoint1;
RoutePoint *pAnchorWatchPoint2;
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

int g_iSDMMFormat;
int g_iDistanceFormat;
int g_iSpeedFormat;
int g_iTempFormat;

int g_iNavAidRadarRingsNumberVisible;
float g_fNavAidRadarRingsStep;
int g_pNavAidRadarRingsStepUnits;
int g_iWaypointRangeRingsNumber;
float g_fWaypointRangeRingsStep;
int g_iWaypointRangeRingsStepUnits;
wxColour g_colourWaypointRangeRingsColour;
bool g_bWayPointPreventDragging;
bool g_bConfirmObjectDelete;
wxColour g_colourOwnshipRangeRingsColour;
int g_iWpt_ScaMin;
bool g_bUseWptScaMin;
bool g_bShowWptName;
int g_maxzoomin;

// Set default color scheme
ColorScheme global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;

int Usercolortable_index;
wxArrayPtrVoid *UserColorTableArray;
wxArrayPtrVoid *UserColourHashTableArray;
wxColorHashMap *pcurrent_user_color_hash;

int gGPS_Watchdog;
bool bGPSValid;
bool bVelocityValid;

int gHDx_Watchdog;
int gHDT_Watchdog;
int gVAR_Watchdog;

int gSAT_Watchdog;

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
double g_config_display_size_mm;
bool g_config_display_size_manual;
float g_selection_radius_mm = 2.0;
float g_selection_radius_touch_mm = 10.0;

int g_GUIScaleFactor;
int g_ChartScaleFactor;
float g_ChartScaleFactorExp;
float g_MarkScaleFactorExp;
int g_last_ChartScaleFactor;
int g_ShipScaleFactor;
float g_ShipScaleFactorExp;
int g_ENCSoundingScaleFactor;
int g_ENCTextScaleFactor;

bool g_bShowTide;
bool g_bShowCurrent;

s52plib *ps52plib;
S57ClassRegistrar *g_poRegistrar;
s57RegistrarMgr *m_pRegistrarMan;

CM93OffsetDialog *g_pCM93OffsetDialog;

#ifdef __WXOSX__
#include "macutils.h"
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

bool g_own_ship_sog_cog_calc;
int g_own_ship_sog_cog_calc_damp_sec;
wxDateTime last_own_ship_sog_cog_calc_ts;
double last_own_ship_sog_cog_calc_lat, last_own_ship_sog_cog_calc_lon;

Multiplexer *g_pMUX;

AisDecoder *g_pAIS;
AisInfoGui *g_pAISGUI;

bool g_bAIS_CPA_Alert;
bool g_bAIS_CPA_Alert_Audio;
AISTargetAlertDialog *g_pais_alert_dialog_active;
AISTargetQueryDialog *g_pais_query_dialog_active;
int g_iSoundDeviceIndex;

int g_ais_alert_dialog_x, g_ais_alert_dialog_y;
int g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
int g_ais_query_dialog_x, g_ais_query_dialog_y;

int g_S57_dialog_sx, g_S57_dialog_sy;

int g_nframewin_x;
int g_nframewin_y;
int g_nframewin_posx;
int g_nframewin_posy;
bool g_bframemax;

bool g_bAutoAnchorMark;

wxRect g_blink_rect;
double g_PlanSpeed;
wxDateTime g_StartTime;
int g_StartTimeTZ;
IDX_entry *gpIDX;
int gpIDXn;
long gStart_LMT_Offset;

wxArrayString *pMessageOnceArray;

FILE *s_fpdebug;
bool bAutoOpen;

bool g_bUseGLL = true;

int g_nCacheLimit;
int g_memCacheLimit;
bool g_bGDAL_Debug;

double g_VPRotate;  // Viewport rotation angle, used on "Course Up" mode
bool g_bCourseUp;
int g_COGAvgSec = 15;  // COG average period (sec.) for Course Up Mode
double g_COGAvg;
bool g_bLookAhead;
bool g_bskew_comp;
bool g_bopengl;
bool g_bSoftwareGL;
bool g_bShowFPS;
bool g_bsmoothpanzoom;
bool g_fog_overzoom;
double g_overzoom_emphasis_base;
bool g_oz_vector_scale;
double g_plus_minus_zoom_factor;

int g_nCOMPortCheck = 32;

bool g_b_legacy_input_filter_behaviour;  // Support original input filter
                                         // process or new process

bool g_bbigred;

PlugInManager *g_pi_manager;

bool g_bAISRolloverShowClass;
bool g_bAISRolloverShowCOG;
bool g_bAISRolloverShowCPA;

bool g_bDebugGPSD;

bool g_bFullScreenQuilt = true;
bool g_bQuiltEnable;
bool g_bQuiltStart;

bool g_bportable;

bool g_bdisable_opengl;

ChartGroupArray *g_pGroupArray;

S57QueryDialog *g_pObjectQueryDialog;

wxArrayString TideCurrentDataSet;
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

int g_grad_default;
wxColour g_border_color_default;
int g_border_size_default;
int g_sash_size_default;
wxColour g_caption_color_default;
wxColour g_sash_color_default;
wxColour g_background_color_default;

int osMajor, osMinor;

bool GetMemoryStatus(int *mem_total, int *mem_used);
bool g_bHasHwClock;


// AIS Global configuration
bool g_bShowAIS;
bool g_bCPAMax;
double g_CPAMax_NM;
bool g_bCPAWarn;
double g_CPAWarn_NM;
bool g_bTCPA_Max;
double g_TCPA_Max;
bool g_bMarkLost;
double g_MarkLost_Mins;
bool g_bRemoveLost;
double g_RemoveLost_Mins;
bool g_bShowCOG;
bool g_bSyncCogPredictors;
double g_ShowCOG_Mins;
bool g_bAISShowTracks;
double g_AISShowTracks_Mins;
double g_AISShowTracks_Limit;
bool g_bHideMoored;
bool g_bAllowShowScaled;
double g_ShowMoored_Kts;
wxString g_sAIS_Alert_Sound_File;
bool g_bAIS_CPA_Alert_Suppress_Moored;
bool g_bAIS_ACK_Timeout;
double g_AckTimeout_Mins;
bool g_bShowScaled;
bool g_bShowAreaNotices;
bool g_bDrawAISSize;
bool g_bDrawAISRealtime;
double g_AIS_RealtPred_Kts;
bool g_bShowAISName;
int g_Show_Target_Name_Scale;
int g_WplAction;

int g_nAIS_activity_timer;

bool g_bEnableZoomToCursor;

bool g_bTrackActive;
bool g_bTrackCarryOver;
bool g_bDeferredStartTrack;
bool g_bTrackDaily;
int g_track_rotate_time;
int g_track_rotate_time_type;
bool g_bHighliteTracks;
int g_route_line_width;
int g_track_line_width;
wxColour g_colourTrackLineColour;
wxString g_default_wp_icon;
wxString g_default_routepoint_icon;

ActiveTrack *g_pActiveTrack;
double g_TrackIntervalSeconds;
double g_TrackDeltaDistance;
int g_nTrackPrecision;

int g_total_NMEAerror_messages;

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

double g_n_arrival_circle_radius;

bool g_bNeedDBUpdate;
bool g_bPreserveScaleOnX;

AboutFrameImpl *g_pAboutDlg;
about *g_pAboutDlgLegacy;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
wxLocale *plocale_def_lang;
#endif

wxString g_locale;
wxString g_localeOverride;
bool g_b_assume_azerty;

bool g_bUseRaster;
bool g_bUseVector;
bool g_bUseCM93;

int g_click_stop;

int g_MemFootSec;
int g_MemFootMB;

wxStaticBitmap *g_pStatBoxTool;
bool g_bShowStatusBar;

bool g_bquiting;
int g_BSBImgDebug;

AISTargetListDialog *g_pAISTargetList;
wxString g_AisTargetList_perspective;
int g_AisTargetList_range;
int g_AisTargetList_sortColumn;
bool g_bAisTargetList_sortReverse;
wxString g_AisTargetList_column_spec;
wxString g_AisTargetList_column_order;
int g_AisTargetList_count;
bool g_bAisTargetList_autosort;

bool g_bFullscreen;

OCPN_AUIManager *g_pauimgr;
wxAuiDefaultDockArt *g_pauidockart;

wxString g_toolbarConfig = _T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
wxString g_toolbarConfigSecondary = _T("....XX..X........XXXXXXXXXXXX");

ocpnFloatingToolbarDialog *g_MainToolbar;
int g_maintoolbar_x;
int g_maintoolbar_y;
long g_maintoolbar_orient;
float g_toolbar_scalefactor;

float g_compass_scalefactor;
bool g_bShowMenuBar;
bool g_bShowCompassWin;

bool g_benable_rotate;

bool g_bShowTrue = true;
bool g_bShowMag;

bool g_bMagneticAPB;

bool g_bInlandEcdis;

int g_GPU_MemSize;

wxString g_uiStyle;

//      Values returned from WMM_PI for variation computation request
//      Initialize to invalid value so we don't use if if WMM hasn't updated yet
double gQueryVar = 361.0;

char bells_sound_file_name[2][12] = {"1bells.wav", "2bells.wav"};

int portaudio_initialized;

bool g_bAIS_GCPA_Alert_Audio;
bool g_bAIS_SART_Alert_Audio;
bool g_bAIS_DSC_Alert_Audio;
bool g_bAnchor_Alert_Audio;

char nmea_tick_chars[] = {'|', '/', '-', '\\', '|', '/', '-', '\\'};

int g_sticky_chart;
int g_sticky_projection;

bool g_benableUDPNullHeader;

int n_NavMessageShown;
wxString g_config_version_string;

wxString g_CmdSoundString;

bool g_btouch;
bool g_bresponsive;
bool g_bRollover;

bool b_inCompressAllCharts;
bool g_bGLexpert;
bool g_bUIexpert;

int g_chart_zoom_modifier_raster;
int g_chart_zoom_modifier_vector;

int g_NMEAAPBPrecision;

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
arrayofCanvasConfigPtr g_canvasConfigArray;
wxString g_lastAppliedTemplateGUID;

ChartCanvas *g_focusCanvas;
ChartCanvas *g_overlayCanvas;

bool b_inCloseWindow;

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




#ifndef __OCPN__ANDROID__
// Connection class, for use by both communicating instances
class stConnection : public wxConnection {
public:
  stConnection() {}
  ~stConnection() {}
  bool OnExec(const wxString &topic, const wxString &data);
};

// Opens a file passed from another instance
bool stConnection::OnExec(const wxString &topic, const wxString &data) {
  // not setup yet
  if (!gFrame) return false;

  wxString path(data);
  if (path.IsEmpty()) {
    gFrame->InvalidateAllGL();
    gFrame->RefreshAllCanvas(false);
    gFrame->Raise();
  } else {
    NavObjectCollection1 *pSet = new NavObjectCollection1;
    pSet->load_file(path.fn_str());
    int wpt_dups;
    pSet->LoadAllGPXObjects(
        !pSet->IsOpenCPN(), wpt_dups,
        true);  // Import with full vizibility of names and objects
    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateLists();

    LLBBox box = pSet->GetBBox();
    if (box.GetValid()) {
      gFrame->CenterView(gFrame->GetPrimaryCanvas(), box);
    }
    delete pSet;
    return true;
  }
  return true;
}

// Server class, for listening to connection requests
class stServer : public wxServer {
public:
  wxConnectionBase *OnAcceptConnection(const wxString &topic);
};

// Accepts a connection from another instance
wxConnectionBase *stServer::OnAcceptConnection(const wxString &topic) {
  if (topic.Lower() == wxT("opencpn")) {
    // Check that there are no modal dialogs active
    wxWindowList::Node *node = wxTopLevelWindows.GetFirst();
    while (node) {
      wxDialog *dialog = wxDynamicCast(node->GetData(), wxDialog);
      if (dialog && dialog->IsModal()) {
        return 0;
      }
      node = node->GetNext();
    }
    return new stConnection();
  }
  return 0;
}

// Client class, to be used by subsequent instances in OnInit
class stClient : public wxClient {
public:
  stClient(){};
  wxConnectionBase *OnMakeConnection() { return new stConnection; }
};

#endif

//------------------------------------------------------------------------------
//    PNG Icon resources
//------------------------------------------------------------------------------

#if defined(__WXGTK__) || defined(__WXQT__)
#include "bitmaps/opencpn.xpm"
#endif



wxString newPrivateFileName(wxString home_locn, const char *name,
                            const char *windowsName) {
  wxString fname = wxString::FromUTF8(name);
  wxString fwname = wxString::FromUTF8(windowsName);
  wxString filePathAndName;

  filePathAndName = g_Platform->GetPrivateDataDir();
  if (filePathAndName.Last() != wxFileName::GetPathSeparator())
    filePathAndName.Append(wxFileName::GetPathSeparator());

#ifdef __WXMSW__
  filePathAndName.Append(fwname);
#else
  filePathAndName.Append(fname);
#endif

  return filePathAndName;
}


// `Main program' equivalent, creating windows and returning main app frame
//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------
IMPLEMENT_APP(MyApp)

BEGIN_EVENT_TABLE(MyApp, wxApp)
EVT_ACTIVATE_APP(MyApp::OnActivateApp)
END_EVENT_TABLE()

#include <wx/dynlib.h>

#if wxUSE_CMDLINE_PARSER
void MyApp::OnInitCmdLine(wxCmdLineParser &parser) {
  //    Add some OpenCPN specific command line options
  parser.AddSwitch(_T("h"), _T("help"), _("Show usage syntax."),
                   wxCMD_LINE_OPTION_HELP);
  parser.AddSwitch(_T("p"), wxEmptyString, _("Run in portable mode."));
  parser.AddSwitch(_T("fullscreen"), wxEmptyString,
                   _("Switch to full screen mode on start."));
  parser.AddSwitch(
      _T("no_opengl"), wxEmptyString,
      _("Disable OpenGL video acceleration. This setting will be remembered."));
  parser.AddSwitch(_T("rebuild_gl_raster_cache"), wxEmptyString,
                   _T("Rebuild OpenGL raster cache on start."));
  parser.AddSwitch(
      _T("parse_all_enc"), wxEmptyString,
      _T("Convert all S-57 charts to OpenCPN's internal format on start."));
  parser.AddOption(
      _T("l"), _T("loglevel"),
      _("Amount of logging: error, warning, message, info, debug or trace"));
  parser.AddOption(_T("unit_test_1"), wxEmptyString,
                   _("Display a slideshow of <num> charts and then exit. Zero "
                     "or negative <num> specifies no limit."),
                   wxCMD_LINE_VAL_NUMBER);
  parser.AddSwitch(_T("unit_test_2"));
  parser.AddParam("import GPX files", wxCMD_LINE_VAL_STRING,
                  wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
  parser.AddLongSwitch("unit_test_2");
  parser.AddSwitch("safe_mode");
}

/** Parse --loglevel and set up logging, falling back to defaults. */
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
    strLevel = "info";
    level = wxLOG_Info;
  }
  wxLog::SetLogLevel(level);
}

bool MyApp::OnCmdLineParsed(wxCmdLineParser &parser) {
  long number;
  wxString repo;
  wxString plugin;

  g_unit_test_2 = parser.Found(_T("unit_test_2"));
  g_bportable = parser.Found(_T("p"));
  g_start_fullscreen = parser.Found(_T("fullscreen"));
  g_bdisable_opengl = parser.Found(_T("no_opengl"));
  g_rebuild_gl_cache = parser.Found(_T("rebuild_gl_raster_cache"));
  g_parse_all_enc = parser.Found(_T("parse_all_enc"));
  if (parser.Found(_T("unit_test_1"), &number)) {
    g_unit_test_1 = static_cast<int>(number);
    if (g_unit_test_1 == 0) g_unit_test_1 = -1;
  }
  safe_mode::set_mode(parser.Found("safe_mode"));
  ParseLoglevel(parser);

  for (size_t paramNr = 0; paramNr < parser.GetParamCount(); ++paramNr)
    g_params.push_back(parser.GetParam(paramNr));

  return true;
}
#endif

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
  //    Code carefully in this method.
  //    It is called in some unexpected places,
  //    such as on closure of dialogs, etc.

  if (!event.GetActive()) {
    //  Remove a temporary Menubar when the application goes inactive
    //  This is one way to handle properly ALT-TAB navigation on the Windows
    //  desktop without accidentally leaving an unwanted Menubar shown.
#ifdef __WXMSW__
    if (g_bTempShowMenuBar) {
      g_bTempShowMenuBar = false;
      if (gFrame) gFrame->ApplyGlobalSettings(false);
    }
#endif
  }
  event.Skip();
}



static wxStopWatch init_sw;

MyApp::MyApp() {
#ifdef __linux__
// Handle e. g., wayland default display -- see #1166.

  if (wxGetEnv( "WAYLAND_DISPLAY", NULL))
    setenv("GDK_BACKEND", "x11", 1);

#endif   // __linux__
}

bool MyApp::OnInit() {
  if (!wxApp::OnInit()) return false;
#ifdef __OCPN__ANDROID__
  androidEnableBackButton(false);
  androidEnableOptionItems(false);
#endif

  GpxDocument::SeedRandom();
  safe_mode::set_mode(false);

  last_own_ship_sog_cog_calc_ts = wxInvalidDateTime;

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
  m_checker = 0;

  // Instantiate the global OCPNPlatform class
  g_Platform = new OCPNPlatform;
  g_BasePlatform = g_Platform;

#ifndef __OCPN__ANDROID__
  //  On Windows
  //  We allow only one instance unless the portable option is used
  if (!g_bportable && wxDirExists(g_Platform->GetPrivateDataDir())) {
    wxChar separator = wxFileName::GetPathSeparator();
    wxString service_name =
        g_Platform->GetPrivateDataDir() + separator + _T("opencpn-ipc");

    m_checker = new wxSingleInstanceChecker(_T("_OpenCPN_SILock"),
                                            g_Platform->GetPrivateDataDir());
    if (!m_checker->IsAnotherRunning()) {
      stServer *m_server = new stServer;
      if (!m_server->Create(service_name)) {
        wxLogDebug(wxT("Failed to create an IPC service."));
      }
    } else {
      wxLogNull logNull;
      stClient *client = new stClient;
      // ignored under DDE, host name in TCP/IP based classes
      wxString hostName = wxT("localhost");
      // Create the connection service, topic
      wxConnectionBase *connection =
          client->MakeConnection(hostName, service_name, _T("OpenCPN"));
      if (connection) {
        // Ask the other instance to open a file or raise itself
        if (!g_params.empty()) {
          for (size_t n = 0; n < g_params.size(); n++) {
            wxString path = g_params[n];
            if (::wxFileExists(path)) {
              connection->Execute(path);
            }
          }
        }
        connection->Execute(wxT(""));
        connection->Disconnect();
        delete connection;
      } else {
        //  If we get here, it means that the wxWidgets single-instance-detect
        //  logic found the lock file, And so thinks another instance is
        //  running. But that instance is not reachable, for some reason. So,
        //  the safe thing to do is delete the lockfile, and exit.  Next start
        //  will proceed normally. This may leave a zombie OpenCPN, but at least
        //  O starts.
        wxString lockFile = wxString(g_Platform->GetPrivateDataDir() +
                                     separator + _T("_OpenCPN_SILock"));
        if (wxFileExists(lockFile)) wxRemoveFile(lockFile);

        wxMessageBox(_("Sorry, an existing instance of OpenCPN may be too busy "
                       "too respond.\nPlease retry."),
                     wxT("OpenCPN"), wxICON_INFORMATION | wxOK);
      }
      delete client;
      return false;  // exit quietly
    }
  }
#endif  // __OCPN__ANDROID__

  if (getenv("OPENCPN_FATAL_ERROR") != 0) {
    wxLogFatalError(getenv("OPENCPN_FATAL_ERROR"));
  }
  // Check if last run failed, set up safe_mode.
  if (!safe_mode::get_mode()) {
    safe_mode::check_last_start();
  }

  //  Perform first stage initialization
  OCPNPlatform::Initialize_1();

#if wxCHECK_VERSION(3, 0, 0)
  // Set the name of the app as displayed to the user.
  // This is necessary at least on OS X, for the capitalisation to be correct in
  // the system menus.
  MyApp::SetAppDisplayName("OpenCPN");
#endif

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

  g_vs = wxString(VERSION_FULL).Trim(true).Trim(false);
  wxDateTime now = wxDateTime::Now();
  LOG_INFO("------- OpenCPN version %s restarted at %s -------\n", VERSION_FULL,
           now.FormatISODate().mb_str().data());
  wxLogLevel level = wxLog::GetLogLevel();
  LOG_INFO("Using loglevel %s", OcpnLog::level2str(level).c_str());

  wxString wxver(wxVERSION_STRING);
  wxver.Prepend(_T("wxWidgets version: "));

  wxPlatformInfo platforminfo = wxPlatformInfo::Get();

  wxString os_name;
#ifndef __OCPN__ANDROID__
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

  //    Initialize embedded PNG icon graphics
  ::wxInitAllImageHandlers();

  wxString imsg = _T("SData_Locn is ");
  imsg += g_Platform->GetSharedDataDir();
  wxLogMessage(imsg);

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

  struct RoutePropDlgCtx ctx;
  ctx.SetRouteAndUpdate = [&](Route* r) {
    if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
      pRoutePropDialog->SetRouteAndUpdate(r, true);
    }
  };
  ctx.SetEnroutePoint = [&](Route* r, RoutePoint* rt) {
    if (pRoutePropDialog && pRoutePropDialog->IsShown()) {
      if (pRoutePropDialog->GetRoute() == r) {
        pRoutePropDialog->SetEnroutePoint(rt);
      }
    }
  };
  ctx.Hide = [&](Route* r) {
    if (pRoutePropDialog && (pRoutePropDialog->IsShown()) &&
        (r == pRoutePropDialog->GetRoute())) {
      pRoutePropDialog->Hide();
    }
  };
  auto RouteMgrDlgUpdateListCtrl = [&]() {
    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateRouteListCtrl();
  };

  g_pRouteMan = new Routeman(ctx, RouteMgrDlgUpdateListCtrl);

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

  g_display_size_mm = wxMax(100, g_Platform->GetDisplaySizeMM());
  wxString msg;
  msg.Printf(_T("Detected display size (horizontal): %d mm"),
             (int)g_display_size_mm);
  wxLogMessage(msg);

  // User override....
  if ((g_config_display_size_mm > 0) && (g_config_display_size_manual)) {
    g_display_size_mm = g_config_display_size_mm;
    wxString msg;
    msg.Printf(_T("Display size (horizontal) config override: %d mm"),
               (int)g_display_size_mm);
    wxLogMessage(msg);
    g_Platform->SetDisplaySizeMM(g_display_size_mm);
  }

  g_display_size_mm = wxMax(80, g_display_size_mm);

  if (g_btouch) {
    int SelectPixelRadius = 50;

    pSelect->SetSelectPixelRadius(SelectPixelRadius);
    pSelectTC->SetSelectPixelRadius(wxMax(25, SelectPixelRadius));
    pSelectAIS->SetSelectPixelRadius(SelectPixelRadius);
  }

  //        Is this the first run after a clean install?
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
#ifdef ocpnUSE_GL

#ifdef __WXMSW__
#if !wxCHECK_VERSION( \
    2, 9, 0)  // The OpenGL test app only runs on wx 2.8, unavailable on wx3.x

  if (/*g_bopengl &&*/ !g_bdisable_opengl) {
    wxFileName fn(g_Platform->GetExePath());
    bool b_test_result = TestGLCanvas(fn.GetPathWithSep());

    if (!b_test_result)
      wxLogMessage(_T("OpenGL disabled due to test app failure."));

    g_bdisable_opengl = !b_test_result;
  }
#endif
#endif

#else
  g_bdisable_opengl = true;
  ;
#endif

  if (g_bdisable_opengl) g_bopengl = false;

#if defined(__UNIX__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)
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
#ifndef __OCPN__ANDROID__
      pInit_Chart_Dir->Append(std_path.GetDocumentsDir());
#else
      pInit_Chart_Dir->Append(androidGetExtStorageDir());
#endif
  }

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

  if (!TideCurrentDataSet.GetCount()) {
    TideCurrentDataSet.Add(g_Platform->NormalizePath(default_tcdata0));
    TideCurrentDataSet.Add(g_Platform->NormalizePath(default_tcdata1));
  } else {
    wxString first_tide = TideCurrentDataSet[0];
    wxFileName ft(first_tide);
    if (!ft.FileExists()) {
      TideCurrentDataSet.RemoveAt(0);
      TideCurrentDataSet.Insert(g_Platform->NormalizePath(default_tcdata0), 0);
      TideCurrentDataSet.Add(g_Platform->NormalizePath(default_tcdata1));
    }
  }

  //  Check the global AIS alarm sound file
  //  If empty, preset default
  if (g_sAIS_Alert_Sound_File.IsEmpty()) {
    wxString default_sound =
        (g_Platform->GetSharedDataDir() + _T("sounds") +
         wxFileName::GetPathSeparator() + _T("2bells.wav"));
    g_sAIS_Alert_Sound_File = g_Platform->NormalizePath(default_sound);
  }

  gpIDX = NULL;
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
  //  Support MultiMonitor setups which an allow negative window positions.
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

#ifdef __OCPN__ANDROID__
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
  wxString myframe_window_title = wxString::Format(wxT("OpenCPN %s"),
                                                   VERSION_FULL);  // Gunther

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

  g_grad_default = g_pauidockart->GetMetric(wxAUI_DOCKART_GRADIENT_TYPE);
  g_border_color_default =
      g_pauidockart->GetColour(wxAUI_DOCKART_BORDER_COLOUR);
  g_border_size_default =
      g_pauidockart->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE);
  g_sash_size_default = g_pauidockart->GetMetric(wxAUI_DOCKART_SASH_SIZE);
  g_caption_color_default =
      g_pauidockart->GetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR);
  g_sash_color_default = g_pauidockart->GetColour(wxAUI_DOCKART_SASH_COLOUR);
  g_background_color_default =
      g_pauidockart->GetColour(wxAUI_DOCKART_BACKGROUND_COLOUR);

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

  gFrame->SetAndApplyColorScheme(global_color_scheme);

  if (g_bframemax) gFrame->Maximize(true);

#ifdef __OCPN__ANDROID__
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
  if (!ChartData->LoadBinary(ChartListFileName, ChartDirArray)) {
    g_bNeedDBUpdate = true;
  }

  //  Verify any saved chart database startup index
  if (g_restore_dbindex >= 0) {
    if (ChartData->GetChartTableEntries() == 0)
      g_restore_dbindex = -1;

    else if (g_restore_dbindex > (ChartData->GetChartTableEntries() - 1))
      g_restore_dbindex = 0;
  }

  //  Apply the inital Group Array structure to the chart data base
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

  if (g_start_fullscreen) gFrame->ToggleFullScreen();

#ifdef __OCPN__ANDROID__
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

#ifdef __OCPN__ANDROID__
  androidHideBusyIcon();
#endif
  wxLogMessage(
      wxString::Format(_("OpenCPN Initialized in %ld ms."), init_sw.Time()));

  wxMilliSleep(500);

#ifdef __OCPN__ANDROID__
  //  We defer the startup message to here to allow the app frame to be
  //  contructed, thus avoiding a dialog with NULL parent which might not work
  //  on some devices.
  if (!n_NavMessageShown || (vs != g_config_version_string) ||
      (g_AndroidVersionCode != androidGetVersionCode())) {
    // qDebug() << "Showing NavWarning";
    wxMilliSleep(500);
    //FIXME (dave)  Move to frame
/*
    if (wxID_CANCEL == ShowNavWarning()) {
      qDebug() << "Closing due to NavWarning Cancel";
      gFrame->Close();
      androidTerminate();
      return true;
    }
*/
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
    //FIXME (dave)  Move to frame
    //if (wxID_CANCEL == ShowNavWarning()) return false;
    n_NavMessageShown = 1;
  }
#endif

  // As an a.e. Raspberry does not have a hardwareclock we will have some
  // problems with date/time setting
  g_bHasHwClock = true;  // by default most computers do have a hwClock
#if defined(__UNIX__) && !defined(__OCPN__ANDROID__)
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

#if defined(__linux__) && !defined(__OCPN__ANDROID__)
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
    ConnectionParams *cp = TheConnectionParams()->Item(i);
    if (cp->bEnabled) {
      if (cp->GetDSPort().Contains(_T("Serial"))) {
        std::string port(cp->Port.ToStdString());
        CheckSerialAccess(gFrame, port);
      }
    }
  }
  CheckDongleAccess(gFrame);
#endif

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

    m_RESTserver.StartServer(data_dir.ToStdString());

    StartMDNSService(g_hostname.ToStdString(), "opencpn-object-control-service", 8000);
  }

  return TRUE;
}

int MyApp::OnExit() {
  wxLogMessage(_T("opencpn::MyApp starting exit."));

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
  delete plocale_def_lang;
#endif

  FontMgr::Shutdown();

  delete m_checker;

  g_Platform->OnExit_2();
  safe_mode::clear_check();

  return TRUE;
}

#ifdef LINUX_CRASHRPT
void MyApp::OnFatalException() { g_crashprint.Report(); }
#endif

Track* MyApp::TrackOff(void) {
  if (gFrame)
    return gFrame->TrackOff();
  else
    return nullptr;
}

#include <wx/power.h>











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


