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

#include <wx/wxprec.h>

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <locale>
#include <list>


#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/bmpcbox.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/graphics.h>
#include <wx/image.h>
#include <wx/listbook.h>
#include <wx/listimpl.cpp>
#include <wx/progdlg.h>
#include <wx/sstream.h>
#include <wx/tglbtn.h>
#include <wx/timectrl.h>
#include <wx/tokenzr.h>

#include "model/ais_decoder.h"
#include "model/ais_state_vars.h"
#include "model/cmdline.h"
#include "model/config_vars.h"
#include "model/conn_params.h"
#include "model/cutil.h"
#include "model/geodesic.h"
#include "model/georef.h"
#include "model/idents.h"
#include "model/multiplexer.h"
#include "model/nav_object_database.h"
#include "model/navutil_base.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/track.h"

#include "ais.h"
#include "CanvasConfig.h"
#include "chartbase.h"
#include "chartdb.h"
#include "chcanv.h"
#include "cm93.h"
#include "config.h"
#include "ConfigMgr.h"
#include "dychart.h"
#include "FontMgr.h"
#include "Layer.h"
#include "navutil.h"
#include "nmea0183.h"
#include "NMEALogWindow.h"
#include "observable_globvar.h"
#include "ocpndc.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "OCPN_Sound.h"
#include "s52plib.h"
#include "s52utils.h"
#include "styles.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

//    Statics

extern OCPNPlatform *g_Platform;
extern MyFrame *gFrame;

extern double g_ChartNotRenderScaleFactor;
extern int g_restore_stackindex;
extern int g_restore_dbindex;
extern RouteList *pRouteList;
extern std::vector<Track*> g_TrackList;
extern LayerList *pLayerList;
extern MyConfig *pConfig;
extern double vLat, vLon;
extern double kLat, kLon;
extern ColorScheme global_color_scheme;
extern int g_nbrightness;
extern bool g_bShowStatusBar;
extern bool g_bUIexpert;
extern bool g_bFullscreen;
extern wxString g_winPluginDir;

extern wxString g_SENCPrefix;
extern wxString g_UserPresLibData;
extern wxString g_TalkerIdText;

extern wxString *pInit_Chart_Dir;
extern wxString gWorldMapLocation;
extern WayPointman *pWayPointMan;

extern bool s_bSetSystemTime;
extern bool g_bDisplayGrid;  // Flag indicating if grid is to be displayed
extern bool g_bPlayShipsBells;
extern int g_iSoundDeviceIndex;
extern bool g_bFullscreenToolbar;
extern bool g_bShowLayers;
extern bool g_bTransparentToolbar;
extern bool g_bPermanentMOBIcon;

extern bool g_bShowDepthUnits;
extern bool g_bAutoAnchorMark;
extern bool g_bskew_comp;
extern bool g_bopengl;
extern bool g_bSoftwareGL;
extern bool g_bShowFPS;
extern bool g_bsmoothpanzoom;
extern bool g_fog_overzoom;
extern double g_overzoom_emphasis_base;
extern bool g_oz_vector_scale;
extern double g_plus_minus_zoom_factor;
extern bool g_bShowOutlines;
extern bool g_bShowActiveRouteHighway;
extern bool g_bShowRouteTotal;
extern int g_nAWDefault;
extern int g_nAWMax;

extern int g_nframewin_x;
extern int g_nframewin_y;
extern int g_nframewin_posx;
extern int g_nframewin_posy;
extern bool g_bframemax;

extern wxString g_VisibleLayers;
extern wxString g_InvisibleLayers;
extern wxString g_VisiNameinLayers;
extern wxString g_InVisiNameinLayers;

extern wxArrayString *pMessageOnceArray;

// LIVE ETA OPTION
extern bool g_bShowLiveETA;
extern double g_defaultBoatSpeed;
extern double g_defaultBoatSpeedUserUnit;

extern wxString g_AisTargetList_perspective;
extern bool g_bUseOnlyConfirmedAISName;
extern int g_ScaledNumWeightSOG;
extern int g_ScaledSizeMinimal;

extern int g_S57_dialog_sx, g_S57_dialog_sy;
int g_S57_extradialog_sx, g_S57_extradialog_sy;

extern int g_iNavAidRadarRingsNumberVisible;
extern float g_fNavAidRadarRingsStep;
extern int g_pNavAidRadarRingsStepUnits;
extern bool g_bWayPointPreventDragging;
extern bool g_bConfirmObjectDelete;
extern wxColour g_colourOwnshipRangeRingsColour;

extern bool g_bEnableZoomToCursor;
extern wxString g_toolbarConfig;
extern double g_TrackIntervalSeconds;

extern int g_nCacheLimit;
extern int g_memCacheLimit;

extern bool g_bGDAL_Debug;
extern bool g_bDebugCM93;
extern bool g_bDebugS57;

extern double g_ownship_predictor_minutes;
extern double g_ownship_HDTpredictor_miles;

extern bool g_own_ship_sog_cog_calc;
extern int g_own_ship_sog_cog_calc_damp_sec;

extern bool g_bShowMenuBar;
extern bool g_bShowCompassWin;

extern s52plib *ps52plib;

extern int g_cm93_zoom_factor;
extern bool g_b_legacy_input_filter_behaviour;
extern bool g_bShowDetailSlider;
extern int g_detailslider_dialog_x, g_detailslider_dialog_y;

extern bool g_bUseGreenShip;

extern int g_OwnShipIconType;
extern double g_n_ownship_length_meters;
extern double g_n_ownship_beam_meters;
extern double g_n_gps_antenna_offset_y;
extern double g_n_gps_antenna_offset_x;
extern int g_n_ownship_min_mm;
extern int g_maxzoomin;

extern bool g_bShowShipToActive;
extern int g_shipToActiveStyle;
extern int g_shipToActiveColor;

extern bool g_bPreserveScaleOnX;
extern bool g_bsimplifiedScalebar;

extern bool g_bUseGLL;

extern wxString g_locale;
extern wxString g_localeOverride;

extern bool g_bCourseUp;
extern bool g_bLookAhead;
extern int g_COGAvgSec;
extern bool g_bShowChartBar;

extern int g_MemFootMB;

extern wxString g_AW1GUID;
extern wxString g_AW2GUID;
extern int g_BSBImgDebug;

extern int n_NavMessageShown;
extern wxString g_config_version_string;

extern wxString g_CmdSoundString;

extern bool g_bDebugGPSD;

extern bool g_bfilter_cogsog;
extern int g_COGFilterSec;
extern int g_SOGFilterSec;

int g_navobjbackups;

extern bool g_bQuiltEnable;
extern bool g_bFullScreenQuilt;
extern bool g_bQuiltStart;

extern int g_SkewCompUpdatePeriod;

extern int g_maintoolbar_x;
extern int g_maintoolbar_y;
extern long g_maintoolbar_orient;

extern int g_lastClientRectx;
extern int g_lastClientRecty;
extern int g_lastClientRectw;
extern int g_lastClientRecth;

extern bool g_bHighliteTracks;
extern int g_cog_predictor_width;
extern int g_ais_cog_predictor_width;

extern wxColour g_colourTrackLineColour;
extern wxString g_default_wp_icon;

extern ChartGroupArray *g_pGroupArray;

extern bool g_bDebugOGL;
extern int g_tcwin_scale;
extern wxString g_uploadConnection;

extern ocpnStyle::StyleManager *g_StyleManager;
extern std::vector<std::string> TideCurrentDataSet;
extern wxString g_TCData_Dir;

extern bool g_bresponsive;

extern bool g_bGLexpert;

extern int g_SENC_LOD_pixels;
extern ArrayOfMmsiProperties g_MMSI_Props_Array;

extern int g_chart_zoom_modifier_raster;
extern int g_chart_zoom_modifier_vector;

extern int g_NMEAAPBPrecision;

extern bool g_bShowTrackPointTime;

extern bool g_bAdvanceRouteWaypointOnArrivalOnly;
extern double g_display_size_mm;
extern double g_config_display_size_mm;
extern bool g_config_display_size_manual;

extern bool g_benable_rotate;
extern bool g_bEmailCrashReport;

extern int g_default_font_size;
extern wxString g_default_font_facename;

extern bool g_bAutoHideToolbar;
extern int g_nAutoHideToolbar;
extern int g_GUIScaleFactor;
extern int g_ChartScaleFactor;
extern float g_MarkScaleFactorExp;

extern int g_ShipScaleFactor;
extern float g_ShipScaleFactorExp;
extern int g_ENCSoundingScaleFactor;
extern int g_ENCTextScaleFactor;

extern bool g_bInlandEcdis;
extern int g_iENCToolbarPosX;
extern int g_iENCToolbarPosY;
extern bool g_bRollover;

extern bool g_bSpaceDropMark;

extern bool g_bShowTide;
extern bool g_bShowCurrent;

extern wxString g_uiStyle;
extern bool g_btrackContinuous;
extern bool g_useMUI;

int g_nCPUCount;

extern unsigned int g_canvasConfig;
extern wxString g_lastAppliedTemplateGUID;

extern int g_route_prop_x, g_route_prop_y;
extern int g_route_prop_sx, g_route_prop_sy;
extern int g_AndroidVersionCode;

extern wxString g_ObjQFileExt;

wxString g_gpx_path;
bool g_bLayersLoaded;
bool g_bShowMuiZoomButtons = true;

double g_mouse_zoom_sensitivity;
int g_mouse_zoom_sensitivity_ui;

#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

// Layer helper function

wxString GetLayerName(int id) {
  wxString name(_T("unknown layer"));
  if (id <= 0) return (name);
  LayerList::iterator it;
  int index = 0;
  for (it = (*pLayerList).begin(); it != (*pLayerList).end(); ++it, ++index) {
    Layer *lay = (Layer *)(*it);
    if (lay->m_LayerID == id) return (lay->m_LayerName);
  }
  return (name);
}

// Helper conditional file name dir slash
void appendOSDirSlash(wxString *pString);

//-----------------------------------------------------------------------------
//          MyConfig Implementation
//-----------------------------------------------------------------------------
//

MyConfig::MyConfig(const wxString &LocalFileName)
    : wxFileConfig(_T (""), _T (""), LocalFileName, _T (""),
                   wxCONFIG_USE_LOCAL_FILE) {
  //    Create the default NavObjectCollection FileName
  wxFileName config_file(LocalFileName);
  m_sNavObjSetFile =
      config_file.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
  m_sNavObjSetFile += _T ( "navobj.xml" );
  m_sNavObjSetChangesFile = m_sNavObjSetFile + _T ( ".changes" );

  m_pNavObjectInputSet = NULL;
  m_pNavObjectChangesSet = NULL;

}

MyConfig::~MyConfig() {
}

void MyConfig::CreateRotatingNavObjBackup() {
  // Avoid nonsense log errors...
#ifdef __OCPN__ANDROID__
  wxLogNull logNo;
#endif

  // Rotate navobj backups, but just in case there are some changes in the
  // current version to prevent the user trying to "fix" the problem by
  // continuously starting the application to overwrite all of his good
  // backups...
  if (g_navobjbackups > 0) {
    wxFile f;
    wxString oldname = m_sNavObjSetFile;
    wxString newname = wxString::Format(_T("%s.1"), m_sNavObjSetFile.c_str());

    wxFileOffset s_diff = 1;
    if (::wxFileExists(newname)) {
      if (f.Open(oldname)) {
        s_diff = f.Length();
        f.Close();
      }

      if (f.Open(newname)) {
        s_diff -= f.Length();
        f.Close();
      }
    }

    if (s_diff != 0) {
      for (int i = g_navobjbackups - 1; i >= 1; i--) {
        oldname = wxString::Format(_T("%s.%d"), m_sNavObjSetFile.c_str(), i);
        newname =
            wxString::Format(_T("%s.%d"), m_sNavObjSetFile.c_str(), i + 1);
        if (wxFile::Exists(oldname)) wxCopyFile(oldname, newname);
      }

      if (wxFile::Exists(m_sNavObjSetFile)) {
        newname = wxString::Format(_T("%s.1"), m_sNavObjSetFile.c_str());
        wxCopyFile(m_sNavObjSetFile, newname);
      }
    }
  }
  // try to clean the backups the user doesn't want - breaks if he deleted some
  // by hand as it tries to be effective...
  for (int i = g_navobjbackups + 1; i <= 99; i++)
    if (wxFile::Exists(
            wxString::Format(_T("%s.%d"), m_sNavObjSetFile.c_str(), i)))
      wxRemoveFile(wxString::Format(_T("%s.%d"), m_sNavObjSetFile.c_str(), i));
    else
      break;
}

int MyConfig::LoadMyConfig() {
  int display_width, display_height;
  wxDisplaySize(&display_width, &display_height);

  //  Set up any defaults not set elsewhere
  g_useMUI = true;
  g_TalkerIdText = _T("EC");
  g_maxWPNameLength = 6;
  g_NMEAAPBPrecision = 3;

#ifdef ocpnUSE_GL
  g_GLOptions.m_bUseAcceleratedPanning = true;
  g_GLOptions.m_GLPolygonSmoothing = true;
  g_GLOptions.m_GLLineSmoothing = true;
  g_GLOptions.m_iTextureDimension = 512;
  g_GLOptions.m_iTextureMemorySize = 128;
  if (!g_bGLexpert) {
    g_GLOptions.m_iTextureMemorySize =
        wxMax(128, g_GLOptions.m_iTextureMemorySize);
    g_GLOptions.m_bTextureCompressionCaching =
        g_GLOptions.m_bTextureCompression;
  }
#endif

  g_maintoolbar_orient = wxTB_HORIZONTAL;
  g_iENCToolbarPosX = -1;
  g_iENCToolbarPosY = -1;
  g_restore_dbindex = -1;
  g_ChartNotRenderScaleFactor = 1.5;
  g_detailslider_dialog_x = 200L;
  g_detailslider_dialog_y = 200L;
  g_SENC_LOD_pixels = 2;
  g_SkewCompUpdatePeriod = 10;

  g_bShowStatusBar = 1;
  g_bShowCompassWin = 1;
  g_iSoundDeviceIndex = -1;
  g_bFullscreenToolbar = 1;
  g_bTransparentToolbar = 0;
  g_bShowLayers = 1;
  g_bShowDepthUnits = 1;
  g_bShowActiveRouteHighway = 1;
  g_bShowChartBar = 1;
  g_defaultBoatSpeed = 6.0;
  g_ownship_predictor_minutes = 5;
  g_cog_predictor_width = 3;
  g_ownship_HDTpredictor_miles = 1;
  g_n_ownship_min_mm = 2;
  g_own_ship_sog_cog_calc_damp_sec = 1;
  g_bFullScreenQuilt = 1;
  g_track_rotate_time_type = TIME_TYPE_COMPUTER;
  g_bHighliteTracks = 1;
  g_bPreserveScaleOnX = 1;
  g_navobjbackups = 5;
  g_benableAISNameCache = true;
  g_n_arrival_circle_radius = 0.05;
  g_plus_minus_zoom_factor = 2.0;
  g_mouse_zoom_sensitivity = 1.5;

  g_AISShowTracks_Mins = 20;
  g_AISShowTracks_Limit = 300.0;
  g_ShowScaled_Num = 10;
  g_ScaledNumWeightSOG = 50;
  g_ScaledNumWeightCPA = 60;
  g_ScaledNumWeightTCPA = 25;
  g_ScaledSizeMinimal = 50;
  g_ScaledNumWeightRange = 75;
  g_ScaledNumWeightSizeOfT = 25;
  g_Show_Target_Name_Scale = 250000;
  g_bWplUsePosition = 0;
  g_WplAction = 0;
  g_ais_cog_predictor_width = 3;
  g_ais_alert_dialog_sx = 200;
  g_ais_alert_dialog_sy = 200;
  g_ais_alert_dialog_x = 200;
  g_ais_alert_dialog_y = 200;
  g_ais_query_dialog_x = 200;
  g_ais_query_dialog_y = 200;
  g_AisTargetList_range = 40;
  g_AisTargetList_sortColumn = 2;  // Column #2 is MMSI
  g_S57_dialog_sx = 400;
  g_S57_dialog_sy = 400;
  g_S57_extradialog_sx = 400;
  g_S57_extradialog_sy = 400;

  //    Reasonable starting point
  vLat = START_LAT;  // display viewpoint
  vLon = START_LON;
  gLat = START_LAT;  // GPS position, as default
  gLon = START_LON;
  g_maxzoomin = 800;

  g_iNavAidRadarRingsNumberVisible = 0;
  g_fNavAidRadarRingsStep = 1.0;
  g_pNavAidRadarRingsStepUnits = 0;
  g_colourOwnshipRangeRingsColour = *wxRED;
  g_iWaypointRangeRingsNumber = 0;
  g_fWaypointRangeRingsStep = 1.0;
  g_iWaypointRangeRingsStepUnits = 0;
  g_colourWaypointRangeRingsColour = wxColour(*wxRED);
  g_bConfirmObjectDelete = true;

  g_TrackIntervalSeconds = 60.0;
  g_TrackDeltaDistance = 0.10;
  g_route_line_width = 2;
  g_track_line_width = 2;
  g_colourTrackLineColour = wxColour(243, 229, 47);  // Yellow

  g_tcwin_scale = 100;
  g_default_wp_icon = _T("triangle");
  g_default_routepoint_icon = _T("diamond");

  g_nAWDefault = 50;
  g_nAWMax = 1852;
  g_ObjQFileExt = _T("txt,rtf,png,html,gif,tif");

  // Load the raw value, with no defaults, and no processing
  int ret_Val = LoadMyConfigRaw();

  //  Perform any required post processing and validation
  if (!ret_Val) {
    g_ChartScaleFactorExp =
        g_Platform->GetChartScaleFactorExp(g_ChartScaleFactor);
    g_ShipScaleFactorExp =
        g_Platform->GetChartScaleFactorExp(g_ShipScaleFactor);
    g_MarkScaleFactorExp =
        g_Platform->GetMarkScaleFactorExp(g_ChartScaleFactor);

    g_COGFilterSec = wxMin(g_COGFilterSec, MAX_COGSOG_FILTER_SECONDS);
    g_COGFilterSec = wxMax(g_COGFilterSec, 1);
    g_SOGFilterSec = g_COGFilterSec;

    if (!g_bShowTrue && !g_bShowMag) g_bShowTrue = true;
    g_COGAvgSec =
        wxMin(g_COGAvgSec, MAX_COG_AVERAGE_SECONDS);  // Bound the array size

    if (g_bInlandEcdis) g_bLookAhead = 1;

    if (g_bdisable_opengl) g_bopengl = false;

#ifdef ocpnUSE_GL
    if (!g_bGLexpert) {
      g_GLOptions.m_iTextureMemorySize =
          wxMax(128, g_GLOptions.m_iTextureMemorySize);
      g_GLOptions.m_bTextureCompressionCaching =
          g_GLOptions.m_bTextureCompression;
    }
#endif

    g_chart_zoom_modifier_raster = wxMin(g_chart_zoom_modifier_raster, 5);
    g_chart_zoom_modifier_raster = wxMax(g_chart_zoom_modifier_raster, -5);
    g_chart_zoom_modifier_vector = wxMin(g_chart_zoom_modifier_vector, 5);
    g_chart_zoom_modifier_vector = wxMax(g_chart_zoom_modifier_vector, -5);
    g_cm93_zoom_factor = wxMin(g_cm93_zoom_factor, CM93_ZOOM_FACTOR_MAX_RANGE);
    g_cm93_zoom_factor =
        wxMax(g_cm93_zoom_factor, (-CM93_ZOOM_FACTOR_MAX_RANGE));

    if ((g_detailslider_dialog_x < 0) ||
        (g_detailslider_dialog_x > display_width))
      g_detailslider_dialog_x = 5;
    if ((g_detailslider_dialog_y < 0) ||
        (g_detailslider_dialog_y > display_height))
      g_detailslider_dialog_y = 5;

    g_defaultBoatSpeedUserUnit = toUsrSpeed(g_defaultBoatSpeed, -1);
    g_n_ownship_min_mm = wxMax(g_n_ownship_min_mm, 2);

    if (g_navobjbackups > 99) g_navobjbackups = 99;
    if (g_navobjbackups < 0) g_navobjbackups = 0;
    g_n_arrival_circle_radius = wxClip(g_n_arrival_circle_radius, 0.001, 0.6);

    g_selection_radius_mm = wxMax(g_selection_radius_mm, 0.5);
    g_selection_radius_touch_mm = wxMax(g_selection_radius_touch_mm, 1.0);

    g_Show_Target_Name_Scale = wxMax(5000, g_Show_Target_Name_Scale);

    if ((g_ais_alert_dialog_x < 0) || (g_ais_alert_dialog_x > display_width))
      g_ais_alert_dialog_x = 5;
    if ((g_ais_alert_dialog_y < 0) || (g_ais_alert_dialog_y > display_height))
      g_ais_alert_dialog_y = 5;
    if ((g_ais_query_dialog_x < 0) || (g_ais_query_dialog_x > display_width))
      g_ais_query_dialog_x = 5;
    if ((g_ais_query_dialog_y < 0) || (g_ais_query_dialog_y > display_height))
      g_ais_query_dialog_y = 5;

    SwitchInlandEcdisMode(g_bInlandEcdis);
    if (g_bInlandEcdis)
      global_color_scheme =
          GLOBAL_COLOR_SCHEME_DUSK;  // startup in duskmode if inlandEcdis

    //    Multicanvas Settings
    LoadCanvasConfigs();
  }

  return ret_Val;
}

int MyConfig::LoadMyConfigRaw(bool bAsTemplate) {
  int read_int;
  wxString val;

  int display_width, display_height;
  wxDisplaySize(&display_width, &display_height);

  //    Global options and settings
  SetPath(_T ( "/Settings" ));
  Read("ActiveRoute", &g_active_route);
  Read("PersistActiveRoute", &g_persist_active_route);
  Read(_T ( "LastAppliedTemplate" ), &g_lastAppliedTemplateGUID);
  Read(_T ( "CompatOS" ), &g_compatOS);
  Read(_T ( "CompatOsVersion" ), &g_compatOsVersion);

  // Some undocumented values
  Read(_T ( "ConfigVersionString" ), &g_config_version_string);
  Read(_T("CmdSoundString"), &g_CmdSoundString, wxString(OCPN_SOUND_CMD));
  if (wxIsEmpty(g_CmdSoundString))
    g_CmdSoundString = wxString(OCPN_SOUND_CMD);
  Read(_T ( "NavMessageShown" ), &n_NavMessageShown);
  Read(_T ( "DisableOpenGL" ), &g_bdisable_opengl);

  Read(_T ( "AndroidVersionCode" ), &g_AndroidVersionCode);

  Read(_T ( "UIexpert" ), &g_bUIexpert);

  Read(_T ( "UIStyle" ), &g_uiStyle);

  Read(_T ( "NCacheLimit" ), &g_nCacheLimit);

  Read(_T ( "InlandEcdis" ),
       &g_bInlandEcdis);  // First read if in iENC mode as this will override
                          // some config settings

  Read(_T( "SpaceDropMark" ), &g_bSpaceDropMark);

  int mem_limit = 0;
  Read(_T ( "MEMCacheLimit" ), &mem_limit);
  if (mem_limit > 0)
    g_memCacheLimit = mem_limit * 1024;  // convert from MBytes to kBytes

  Read(_T ( "UseModernUI5" ), &g_useMUI);

  Read(_T( "NCPUCount" ), &g_nCPUCount);

  Read(_T ( "DebugGDAL" ), &g_bGDAL_Debug);
  Read(_T ( "DebugNMEA" ), &g_nNMEADebug);
  Read(_T ( "AnchorWatchDefault" ), &g_nAWDefault);
  Read(_T ( "AnchorWatchMax" ), &g_nAWMax);
  Read(_T ( "GPSDogTimeout" ), &gps_watchdog_timeout_ticks);
  Read(_T ( "DebugCM93" ), &g_bDebugCM93);
  Read(_T ( "DebugS57" ),
       &g_bDebugS57);  // Show LUP and Feature info in object query
  Read(_T ( "DebugBSBImg" ), &g_BSBImgDebug);
  Read(_T ( "DebugGPSD" ), &g_bDebugGPSD);
  Read(_T ( "MaxZoomScale" ), &g_maxzoomin);
  g_maxzoomin = wxMax(g_maxzoomin, 50);

  Read(_T ( "DefaultFontSize"), &g_default_font_size);
  Read(_T ( "DefaultFontFacename"), &g_default_font_facename);

  Read(_T ( "UseGreenShipIcon" ), &g_bUseGreenShip);

  Read(_T ( "AutoHideToolbar" ), &g_bAutoHideToolbar);
  Read(_T ( "AutoHideToolbarSecs" ), &g_nAutoHideToolbar);

  Read(_T ( "UseSimplifiedScalebar" ), &g_bsimplifiedScalebar);
  Read(_T ( "ShowTide" ), &g_bShowTide);
  Read(_T ( "ShowCurrent" ), &g_bShowCurrent);

  int size_mm = -1;
  Read(_T ( "DisplaySizeMM" ), &size_mm);

  Read(_T ( "SelectionRadiusMM" ), &g_selection_radius_mm);
  Read(_T ( "SelectionRadiusTouchMM" ), &g_selection_radius_touch_mm);

  if (!bAsTemplate) {
    if (size_mm > 0) {
      g_config_display_size_mm = size_mm;
      if ((size_mm > 100) && (size_mm < 2000)) {
        g_display_size_mm = size_mm;
      }
    }
    Read(_T ( "DisplaySizeManual" ), &g_config_display_size_manual);
  }

  Read(_T ( "GUIScaleFactor" ), &g_GUIScaleFactor);

  Read(_T ( "ChartObjectScaleFactor" ), &g_ChartScaleFactor);
  Read(_T ( "ShipScaleFactor" ), &g_ShipScaleFactor);
  Read(_T ( "ENCSoundingScaleFactor" ), &g_ENCSoundingScaleFactor);
  Read(_T ( "ENCTextScaleFactor" ), &g_ENCTextScaleFactor);
  Read(_T ( "ObjQueryAppendFilesExt" ), &g_ObjQFileExt);

  // Plugin catalog handler persistent variables.
  Read("CatalogCustomURL", &g_catalog_custom_url);
  Read("CatalogChannel", &g_catalog_channel);

  //  NMEA connection options.
  if (!bAsTemplate) {
    Read(_T ( "FilterNMEA_Avg" ), &g_bfilter_cogsog);
    Read(_T ( "FilterNMEA_Sec" ), &g_COGFilterSec);
    Read(_T ( "GPSIdent" ), &g_GPS_Ident);
    Read(_T ( "UseGarminHostUpload" ), &g_bGarminHostUpload);
    Read(_T ( "UseNMEA_GLL" ), &g_bUseGLL);
    Read(_T ( "UseMagAPB" ), &g_bMagneticAPB);
    Read(_T ( "TrackContinuous" ), &g_btrackContinuous, false);
    Read(_T ( "FilterTrackDropLargeJump" ), &g_trackFilterMax, 1000);
  }

  Read(_T ( "ShowTrue" ), &g_bShowTrue);
  Read(_T ( "ShowMag" ), &g_bShowMag);

  wxString umv;
  Read(_T ( "UserMagVariation" ), &umv);
  if (umv.Len()) umv.ToDouble(&g_UserVar);

  Read(_T ( "ScreenBrightness" ), &g_nbrightness);

  Read(_T ( "MemFootprintTargetMB" ), &g_MemFootMB);

  Read(_T ( "WindowsComPortMax" ), &g_nCOMPortCheck);

  Read(_T ( "ChartQuilting" ), &g_bQuiltEnable);
  Read(_T ( "ChartQuiltingInitial" ), &g_bQuiltStart);

  Read(_T ( "CourseUpMode" ), &g_bCourseUp);
  Read(_T ( "COGUPAvgSeconds" ), &g_COGAvgSec);
  Read(_T ( "LookAheadMode" ), &g_bLookAhead);
  Read(_T ( "SkewToNorthUp" ), &g_bskew_comp);

  Read(_T ( "ShowFPS" ), &g_bShowFPS);

  Read(_T( "NMEAAPBPrecision" ), &g_NMEAAPBPrecision);

  Read(_T( "TalkerIdText" ), &g_TalkerIdText);
  Read(_T( "MaxWaypointNameLength" ), &g_maxWPNameLength);

  Read(_T( "ShowTrackPointTime" ), &g_bShowTrackPointTime, true);
  /* opengl options */
#ifdef ocpnUSE_GL
  if (!bAsTemplate) {
    Read(_T ( "OpenGLExpert" ), &g_bGLexpert, false);
    Read(_T ( "UseAcceleratedPanning" ), &g_GLOptions.m_bUseAcceleratedPanning,
         true);
    Read(_T ( "GPUTextureCompression" ), &g_GLOptions.m_bTextureCompression);
    Read(_T ( "GPUTextureCompressionCaching" ),
         &g_GLOptions.m_bTextureCompressionCaching);
    Read(_T ( "PolygonSmoothing" ), &g_GLOptions.m_GLPolygonSmoothing);
    Read(_T ( "LineSmoothing" ), &g_GLOptions.m_GLLineSmoothing);
    Read(_T ( "GPUTextureDimension" ), &g_GLOptions.m_iTextureDimension);
    Read(_T ( "GPUTextureMemSize" ), &g_GLOptions.m_iTextureMemorySize);
    Read(_T ( "DebugOpenGL" ), &g_bDebugOGL);
    Read(_T ( "OpenGL" ), &g_bopengl);
    Read(_T ( "SoftwareGL" ), &g_bSoftwareGL);
  }
#endif

  Read(_T ( "SmoothPanZoom" ), &g_bsmoothpanzoom);

  Read(_T ( "ToolbarX"), &g_maintoolbar_x);
  Read(_T ( "ToolbarY" ), &g_maintoolbar_y);
  Read(_T ( "ToolbarOrient" ), &g_maintoolbar_orient);
  Read(_T ( "GlobalToolbarConfig" ), &g_toolbarConfig);

  Read(_T ( "iENCToolbarX"), &g_iENCToolbarPosX);
  Read(_T ( "iENCToolbarY"), &g_iENCToolbarPosY);

  Read(_T ( "AnchorWatch1GUID" ), &g_AW1GUID);
  Read(_T ( "AnchorWatch2GUID" ), &g_AW2GUID);

  Read(_T ( "InitialStackIndex" ), &g_restore_stackindex);
  Read(_T ( "InitialdBIndex" ), &g_restore_dbindex);

  Read(_T ( "ChartNotRenderScaleFactor" ), &g_ChartNotRenderScaleFactor);

  Read(_T ( "MobileTouch" ), &g_btouch);

//  "Responsive graphics" option deprecated in O58+
//  Read(_T ( "ResponsiveGraphics" ), &g_bresponsive);
#ifdef __OCPN__ANDROID__
  g_bresponsive = true;
#else
  g_bresponsive = false;
#endif

  Read(_T ( "EnableRolloverBlock" ), &g_bRollover);

  Read(_T ( "ZoomDetailFactor" ), &g_chart_zoom_modifier_raster);
  Read(_T ( "ZoomDetailFactorVector" ), &g_chart_zoom_modifier_vector);
  Read(_T ( "PlusMinusZoomFactor" ), &g_plus_minus_zoom_factor, 2.0);
  Read("MouseZoomSensitivity", &g_mouse_zoom_sensitivity, 1.3);
  g_mouse_zoom_sensitivity_ui =
      MouseZoom::config_to_ui(g_mouse_zoom_sensitivity);
  Read(_T ( "CM93DetailFactor" ), &g_cm93_zoom_factor);

  Read(_T ( "CM93DetailZoomPosX" ), &g_detailslider_dialog_x);
  Read(_T ( "CM93DetailZoomPosY" ), &g_detailslider_dialog_y);
  Read(_T ( "ShowCM93DetailSlider" ), &g_bShowDetailSlider);

  Read(_T ( "SENC_LOD_Pixels" ), &g_SENC_LOD_pixels);

  Read(_T ( "SkewCompUpdatePeriod" ), &g_SkewCompUpdatePeriod);

  Read(_T ( "SetSystemTime" ), &s_bSetSystemTime);
  Read(_T ( "ShowStatusBar" ), &g_bShowStatusBar);
#ifndef __WXOSX__
  Read(_T ( "ShowMenuBar" ), &g_bShowMenuBar);
#endif
  Read(_T ( "Fullscreen" ), &g_bFullscreen);
  Read(_T ( "ShowCompassWindow" ), &g_bShowCompassWin);
  Read(_T ( "ShowGrid" ), &g_bDisplayGrid);
  Read(_T ( "PlayShipsBells" ), &g_bPlayShipsBells);
  Read(_T ( "SoundDeviceIndex" ), &g_iSoundDeviceIndex);
  Read(_T ( "FullscreenToolbar" ), &g_bFullscreenToolbar);
  Read(_T ( "PermanentMOBIcon" ), &g_bPermanentMOBIcon);
  Read(_T ( "ShowLayers" ), &g_bShowLayers);
  Read(_T ( "ShowDepthUnits" ), &g_bShowDepthUnits);
  Read(_T ( "AutoAnchorDrop" ), &g_bAutoAnchorMark);
  Read(_T ( "ShowChartOutlines" ), &g_bShowOutlines);
  Read(_T ( "ShowActiveRouteHighway" ), &g_bShowActiveRouteHighway);
  Read(_T ( "ShowActiveRouteTotal" ), &g_bShowRouteTotal);
  Read(_T ( "MostRecentGPSUploadConnection" ), &g_uploadConnection);
  Read(_T ( "ShowChartBar" ), &g_bShowChartBar);
  Read(_T ( "SDMMFormat" ),
       &g_iSDMMFormat);  // 0 = "Degrees, Decimal minutes"), 1 = "Decimal
                         // degrees", 2 = "Degrees,Minutes, Seconds"

  Read(_T ( "DistanceFormat" ),
       &g_iDistanceFormat);  // 0 = "Nautical miles"), 1 = "Statute miles", 2 =
                             // "Kilometers", 3 = "Meters"
  Read(_T ( "SpeedFormat" ),
       &g_iSpeedFormat);  // 0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"
  Read(_T ( "WindSpeedFormat" ),
       &g_iWindSpeedFormat);  // 0 = "knots"), 1 = "m/s", 2 = "Mph", 3 = "km/h"
  Read(_T ("TemperatureFormat"), &g_iTempFormat);  // 0 = C, 1 = F, 2 = K

  // LIVE ETA OPTION
  Read(_T ( "LiveETA" ), &g_bShowLiveETA);
  Read(_T ( "DefaultBoatSpeed" ), &g_defaultBoatSpeed);

  Read(_T ( "OwnshipCOGPredictorMinutes" ), &g_ownship_predictor_minutes);
  Read(_T ( "OwnshipCOGPredictorWidth" ), &g_cog_predictor_width);
  Read(_T ( "OwnshipHDTPredictorMiles" ), &g_ownship_HDTpredictor_miles);

  Read(_T ( "OwnShipIconType" ), &g_OwnShipIconType);
  Read(_T ( "OwnShipLength" ), &g_n_ownship_length_meters);
  Read(_T ( "OwnShipWidth" ), &g_n_ownship_beam_meters);
  Read(_T ( "OwnShipGPSOffsetX" ), &g_n_gps_antenna_offset_x);
  Read(_T ( "OwnShipGPSOffsetY" ), &g_n_gps_antenna_offset_y);
  Read(_T ( "OwnShipMinSize" ), &g_n_ownship_min_mm);
  Read(_T ( "OwnShipSogCogCalc" ), &g_own_ship_sog_cog_calc);
  Read(_T ( "OwnShipSogCogCalcDampSec"), &g_own_ship_sog_cog_calc_damp_sec);
  Read(_T ( "ShowDirectRouteLine"), &g_bShowShipToActive);
  Read(_T ( "DirectRouteLineStyle"), &g_shipToActiveStyle);
  Read(_T( "DirectRouteLineColor" ), &g_shipToActiveColor);

  wxString racr;
  Read(_T ( "RouteArrivalCircleRadius" ), &racr);
  if (racr.Len()) racr.ToDouble(&g_n_arrival_circle_radius);

  Read(_T ( "FullScreenQuilt" ), &g_bFullScreenQuilt);

  Read(_T ( "StartWithTrackActive" ), &g_bTrackCarryOver);
  Read(_T ( "AutomaticDailyTracks" ), &g_bTrackDaily);
  Read(_T ( "TrackRotateAt" ), &g_track_rotate_time);
  Read(_T ( "TrackRotateTimeType" ), &g_track_rotate_time_type);
  Read(_T ( "HighlightTracks" ), &g_bHighliteTracks);

  wxString stps;
  Read(_T ( "PlanSpeed" ), &stps);
  if (!stps.IsEmpty()) stps.ToDouble(&g_PlanSpeed);

  Read(_T ( "VisibleLayers" ), &g_VisibleLayers);
  Read(_T ( "InvisibleLayers" ), &g_InvisibleLayers);
  Read(_T ( "VisNameInLayers" ), &g_VisiNameinLayers);
  Read(_T ( "InvisNameInLayers" ), &g_InVisiNameinLayers);

  Read(_T ( "PreserveScaleOnX" ), &g_bPreserveScaleOnX);

  Read(_T ( "ShowMUIZoomButtons" ), &g_bShowMuiZoomButtons);

  Read(_T ( "Locale" ), &g_locale);
  Read(_T ( "LocaleOverride" ), &g_localeOverride);

  // We allow 0-99 backups ov navobj.xml
  Read(_T ( "KeepNavobjBackups" ), &g_navobjbackups);

  NMEALogWindow::Get().SetSize(Read(_T("NMEALogWindowSizeX"), 600L),
                               Read(_T("NMEALogWindowSizeY"), 400L));
  NMEALogWindow::Get().SetPos(Read(_T("NMEALogWindowPosX"), 10L),
                              Read(_T("NMEALogWindowPosY"), 10L));
  NMEALogWindow::Get().CheckPos(display_width, display_height);

  // Boolean to cater for legacy Input COM Port filer behaviour, i.e. show msg
  // filtered but put msg on bus.
  Read(_T ( "LegacyInputCOMPortFilterBehaviour" ),
       &g_b_legacy_input_filter_behaviour);

  // Boolean to cater for sailing when not approaching waypoint
  Read(_T( "AdvanceRouteWaypointOnArrivalOnly" ),
       &g_bAdvanceRouteWaypointOnArrivalOnly);

  Read(_T ( "EnableRotateKeys" ), &g_benable_rotate);
  Read(_T ( "EmailCrashReport" ), &g_bEmailCrashReport);

  g_benableAISNameCache = true;
  Read(_T ( "EnableAISNameCache" ), &g_benableAISNameCache);

  Read(_T ( "EnableUDPNullHeader" ), &g_benableUDPNullHeader);

  SetPath(_T ( "/Settings/GlobalState" ));

  Read(_T ( "FrameWinX" ), &g_nframewin_x);
  Read(_T ( "FrameWinY" ), &g_nframewin_y);
  Read(_T ( "FrameWinPosX" ), &g_nframewin_posx);
  Read(_T ( "FrameWinPosY" ), &g_nframewin_posy);
  Read(_T ( "FrameMax" ), &g_bframemax);

  Read(_T ( "ClientPosX" ), &g_lastClientRectx);
  Read(_T ( "ClientPosY" ), &g_lastClientRecty);
  Read(_T ( "ClientSzX" ), &g_lastClientRectw);
  Read(_T ( "ClientSzY" ), &g_lastClientRecth);

  Read(_T( "RoutePropSizeX" ), &g_route_prop_sx);
  Read(_T( "RoutePropSizeY" ), &g_route_prop_sy);
  Read(_T( "RoutePropPosX" ), &g_route_prop_x);
  Read(_T( "RoutePropPosY" ), &g_route_prop_y);

  read_int = -1;
  Read(_T ( "S52_DEPTH_UNIT_SHOW" ), &read_int);  // default is metres
  if (read_int >= 0) {
    read_int = wxMax(read_int, 0);  // qualify value
    read_int = wxMin(read_int, 2);
    g_nDepthUnitDisplay = read_int;
  }

  // Sounds
  SetPath(_T ( "/Settings/Audio" ));

  // Set reasonable defaults
  wxString sound_dir = g_Platform->GetSharedDataDir();
  sound_dir.Append(_T("sounds"));
  sound_dir.Append(wxFileName::GetPathSeparator());

  g_AIS_sound_file = sound_dir + _T("beep_ssl.wav");
  g_DSC_sound_file = sound_dir + _T("phonering1.wav");
  g_SART_sound_file = sound_dir + _T("beep3.wav");
  g_anchorwatch_sound_file = sound_dir + _T("beep1.wav");

  Read(_T ( "AISAlertSoundFile" ), &g_AIS_sound_file);
  Read(_T ( "DSCAlertSoundFile" ), &g_DSC_sound_file);
  Read(_T ( "SARTAlertSoundFile" ), &g_SART_sound_file);
  Read(_T ( "AnchorAlarmSoundFile" ), &g_anchorwatch_sound_file);

  Read(_T ( "bAIS_GCPA_AlertAudio" ), &g_bAIS_GCPA_Alert_Audio);
  Read(_T ( "bAIS_SART_AlertAudio" ), &g_bAIS_SART_Alert_Audio);
  Read(_T ( "bAIS_DSC_AlertAudio" ), &g_bAIS_DSC_Alert_Audio);
  Read(_T ( "bAnchorAlertAudio" ), &g_bAnchor_Alert_Audio);

  //    AIS
  wxString s;
  SetPath(_T ( "/Settings/AIS" ));

  g_bUseOnlyConfirmedAISName = false;
  Read(_T ( "UseOnlyConfirmedAISName" ), &g_bUseOnlyConfirmedAISName);

  Read(_T ( "bNoCPAMax" ), &g_bCPAMax);

  Read(_T ( "NoCPAMaxNMi" ), &s);
  s.ToDouble(&g_CPAMax_NM);

  Read(_T ( "bCPAWarn" ), &g_bCPAWarn);

  Read(_T ( "CPAWarnNMi" ), &s);
  s.ToDouble(&g_CPAWarn_NM);

  Read(_T ( "bTCPAMax" ), &g_bTCPA_Max);

  Read(_T ( "TCPAMaxMinutes" ), &s);
  s.ToDouble(&g_TCPA_Max);

  Read(_T ( "bMarkLostTargets" ), &g_bMarkLost);

  Read(_T ( "MarkLost_Minutes" ), &s);
  s.ToDouble(&g_MarkLost_Mins);

  Read(_T ( "bRemoveLostTargets" ), &g_bRemoveLost);

  Read(_T ( "RemoveLost_Minutes" ), &s);
  s.ToDouble(&g_RemoveLost_Mins);

  Read(_T ( "bShowCOGArrows" ), &g_bShowCOG);

  Read(_T ("bSyncCogPredictors"), &g_bSyncCogPredictors);

  Read(_T ( "CogArrowMinutes" ), &s);
  s.ToDouble(&g_ShowCOG_Mins);

  Read(_T ( "bShowTargetTracks" ), &g_bAISShowTracks);

  if (Read(_T ( "TargetTracksLimit" ), &s)) {
    s.ToDouble(&g_AISShowTracks_Limit);
    g_AISShowTracks_Limit = wxMax(300.0, g_AISShowTracks_Limit);
  }
  if (Read(_T ( "TargetTracksMinutes" ), &s)) {
    s.ToDouble(&g_AISShowTracks_Mins);
    g_AISShowTracks_Mins = wxMax(1.0, g_AISShowTracks_Mins);
    g_AISShowTracks_Mins = wxMin(g_AISShowTracks_Limit, g_AISShowTracks_Mins);
  }

  Read(_T ( "bHideMooredTargets" ), &g_bHideMoored);
  if (Read(_T ( "MooredTargetMaxSpeedKnots" ), &s))
    s.ToDouble(&g_ShowMoored_Kts);

  Read(_T ("bShowScaledTargets"), &g_bAllowShowScaled);
  Read(_T ( "AISScaledNumber" ), &g_ShowScaled_Num);
  Read(_T ( "AISScaledNumberWeightSOG" ), &g_ScaledNumWeightSOG);
  Read(_T ( "AISScaledNumberWeightCPA" ), &g_ScaledNumWeightCPA);
  Read(_T ( "AISScaledNumberWeightTCPA" ), &g_ScaledNumWeightTCPA);
  Read(_T ( "AISScaledNumberWeightRange" ), &g_ScaledNumWeightRange);
  Read(_T ( "AISScaledNumberWeightSizeOfTarget" ), &g_ScaledNumWeightSizeOfT);
  Read(_T ( "AISScaledSizeMinimal" ), &g_ScaledSizeMinimal);
  Read(_T("AISShowScaled"), &g_bShowScaled);

  Read(_T ( "bShowAreaNotices" ), &g_bShowAreaNotices);
  Read(_T ( "bDrawAISSize" ), &g_bDrawAISSize);
  Read(_T ( "bDrawAISRealtime" ), &g_bDrawAISRealtime);
  Read(_T ( "bShowAISName" ), &g_bShowAISName);
  Read(_T ( "AISRealtimeMinSpeedKnots" ), &g_AIS_RealtPred_Kts, 0.7);
  Read(_T ( "bAISAlertDialog" ), &g_bAIS_CPA_Alert);
  Read(_T ( "ShowAISTargetNameScale" ), &g_Show_Target_Name_Scale);
  Read(_T ( "bWplIsAprsPositionReport" ), &g_bWplUsePosition);
  Read(_T ( "WplSelAction"), &g_WplAction);
  Read(_T ( "AISCOGPredictorWidth" ), &g_ais_cog_predictor_width);

  Read(_T ( "bAISAlertAudio" ), &g_bAIS_CPA_Alert_Audio);
  Read(_T ( "AISAlertAudioFile" ), &g_sAIS_Alert_Sound_File);
  Read(_T ( "bAISAlertSuppressMoored" ), &g_bAIS_CPA_Alert_Suppress_Moored);

  Read(_T ( "bAISAlertAckTimeout" ), &g_bAIS_ACK_Timeout);
  if (Read(_T ( "AlertAckTimeoutMinutes" ), &s)) s.ToDouble(&g_AckTimeout_Mins);

  Read(_T ( "AlertDialogSizeX" ), &g_ais_alert_dialog_sx);
  Read(_T ( "AlertDialogSizeY" ), &g_ais_alert_dialog_sy);
  Read(_T ( "AlertDialogPosX" ), &g_ais_alert_dialog_x);
  Read(_T ( "AlertDialogPosY" ), &g_ais_alert_dialog_y);
  Read(_T ( "QueryDialogPosX" ), &g_ais_query_dialog_x);
  Read(_T ( "QueryDialogPosY" ), &g_ais_query_dialog_y);

  Read(_T ( "AISTargetListPerspective" ), &g_AisTargetList_perspective);
  Read(_T ( "AISTargetListRange" ), &g_AisTargetList_range);
  Read(_T ( "AISTargetListSortColumn" ), &g_AisTargetList_sortColumn);
  Read(_T ( "bAISTargetListSortReverse" ), &g_bAisTargetList_sortReverse);
  Read(_T ( "AISTargetListColumnSpec" ), &g_AisTargetList_column_spec);
  Read(_T ("AISTargetListColumnOrder"), &g_AisTargetList_column_order);

  Read(_T ( "bAISRolloverShowClass" ), &g_bAISRolloverShowClass);
  Read(_T ( "bAISRolloverShowCOG" ), &g_bAISRolloverShowCOG);
  Read(_T ( "bAISRolloverShowCPA" ), &g_bAISRolloverShowCPA);

  Read(_T ( "S57QueryDialogSizeX" ), &g_S57_dialog_sx);
  Read(_T ( "S57QueryDialogSizeY" ), &g_S57_dialog_sy);
  Read(_T ( "S57QueryExtraDialogSizeX" ), &g_S57_extradialog_sx);
  Read(_T ( "S57QueryExtraDialogSizeY" ), &g_S57_extradialog_sy);

  wxString strpres(_T ( "PresentationLibraryData" ));
  wxString valpres;
  SetPath(_T ( "/Directories" ));
  Read(strpres, &valpres);  // Get the File name
  if (!valpres.IsEmpty()) g_UserPresLibData = valpres;

  wxString strs(_T ( "SENCFileLocation" ));
  SetPath(_T ( "/Directories" ));
  wxString vals;
  Read(strs, &vals);  // Get the Directory name
  if (!vals.IsEmpty()) g_SENCPrefix = vals;

  SetPath(_T ( "/Directories" ));
  wxString vald;
  Read(_T ( "InitChartDir" ), &vald);  // Get the Directory name

  wxString dirnamed(vald);
  if (!dirnamed.IsEmpty()) {
    if (pInit_Chart_Dir->IsEmpty())  // on second pass, don't overwrite
    {
      pInit_Chart_Dir->Clear();
      pInit_Chart_Dir->Append(vald);
    }
  }

  Read(_T ( "GPXIODir" ), &g_gpx_path);     // Get the Directory name
  Read(_T ( "TCDataDir" ), &g_TCData_Dir);  // Get the Directory name
  Read(_T ( "BasemapDir"), &gWorldMapLocation);
  Read(_T ( "pluginInstallDir"), &g_winPluginDir);
  wxLogMessage("winPluginDir, read from ini file: %s",
               g_winPluginDir.mb_str().data());

  SetPath(_T ( "/Settings/GlobalState" ));

  if (Read(_T ( "nColorScheme" ), &read_int))
    global_color_scheme = (ColorScheme)read_int;

  if (!bAsTemplate) {
    SetPath(_T ( "/Settings/NMEADataSource" ));

    wxString connectionconfigs;
    Read(_T( "DataConnections" ), &connectionconfigs);
    if (!connectionconfigs.IsEmpty()) {
      wxArrayString confs = wxStringTokenize(connectionconfigs, _T("|"));
      TheConnectionParams()->Clear();
      for (size_t i = 0; i < confs.Count(); i++) {
        ConnectionParams *prm = new ConnectionParams(confs[i]);
        if (!prm->Valid) {
          wxLogMessage(_T( "Skipped invalid DataStream config"));
          delete prm;
          continue;
        }
        TheConnectionParams()->Add(prm);
      }
    }
  }

  SetPath(_T ( "/Settings/GlobalState" ));
  wxString st;

  double st_lat, st_lon;
  if (Read(_T ( "VPLatLon" ), &st)) {
    sscanf(st.mb_str(wxConvUTF8), "%lf,%lf", &st_lat, &st_lon);

    //    Sanity check the lat/lon...both have to be reasonable.
    if (fabs(st_lon) < 360.) {
      while (st_lon < -180.) st_lon += 360.;

      while (st_lon > 180.) st_lon -= 360.;

      vLon = st_lon;
    }

    if (fabs(st_lat) < 90.0) vLat = st_lat;

    s.Printf(_T ( "Setting Viewpoint Lat/Lon %g, %g" ), vLat, vLon);
    wxLogMessage(s);
  }

  double st_view_scale, st_rotation;
  if (Read(wxString(_T ( "VPScale" )), &st)) {
    sscanf(st.mb_str(wxConvUTF8), "%lf", &st_view_scale);
    //    Sanity check the scale
    st_view_scale = fmax(st_view_scale, .001 / 32);
    st_view_scale = fmin(st_view_scale, 4);
  }

  if (Read(wxString(_T ( "VPRotation" )), &st)) {
    sscanf(st.mb_str(wxConvUTF8), "%lf", &st_rotation);
    //    Sanity check the rotation
    st_rotation = fmin(st_rotation, 360);
    st_rotation = fmax(st_rotation, 0);
  }

  wxString sll;
  double lat, lon;
  if (Read(_T ( "OwnShipLatLon" ), &sll)) {
    sscanf(sll.mb_str(wxConvUTF8), "%lf,%lf", &lat, &lon);

    //    Sanity check the lat/lon...both have to be reasonable.
    if (fabs(lon) < 360.) {
      while (lon < -180.) lon += 360.;

      while (lon > 180.) lon -= 360.;

      gLon = lon;
    }

    if (fabs(lat) < 90.0) gLat = lat;

    s.Printf(_T ( "Setting Ownship Lat/Lon %g, %g" ), gLat, gLon);
    wxLogMessage(s);
  }

  //    Fonts

  //  Load the persistent Auxiliary Font descriptor Keys
  SetPath(_T ( "/Settings/AuxFontKeys" ));

  wxString strk;
  long dummyk;
  wxString kval;
  bool bContk = GetFirstEntry(strk, dummyk);
  bool bNewKey = false;
  while (bContk) {
    Read(strk, &kval);
    bNewKey = FontMgr::Get().AddAuxKey(kval);
    if (!bAsTemplate && !bNewKey) {
      DeleteEntry(strk);
      dummyk--;
    }
    bContk = GetNextEntry(strk, dummyk);
  }

#ifdef __WXX11__
  SetPath(_T ( "/Settings/X11Fonts" ));
#endif

#ifdef __WXGTK__
  SetPath(_T ( "/Settings/GTKFonts" ));
#endif

#ifdef __WXMSW__
  SetPath(_T ( "/Settings/MSWFonts" ));
#endif

#ifdef __WXMAC__
  SetPath(_T ( "/Settings/MacFonts" ));
#endif

#ifdef __WXQT__
  SetPath(_T ( "/Settings/QTFonts" ));
#endif

  wxString str;
  long dummy;
  wxString pval;
  wxArrayString deleteList;

  bool bCont = GetFirstEntry(str, dummy);
  while (bCont) {
    Read(str, pval);

    if (str.StartsWith(_T("Font"))) {
      // Convert pre 3.1 setting. Can't delete old entries from inside the
      // GetNextEntry() loop, so we need to save those and delete outside.
      deleteList.Add(str);
      wxString oldKey = pval.BeforeFirst(_T(':'));
      str = FontMgr::GetFontConfigKey(oldKey);
    }

    if (pval.IsEmpty() || pval.StartsWith(_T(":"))) {
      deleteList.Add(str);
    } else
      FontMgr::Get().LoadFontNative(&str, &pval);

    bCont = GetNextEntry(str, dummy);
  }

  for (unsigned int i = 0; i < deleteList.Count(); i++) {
    DeleteEntry(deleteList[i]);
  }
  deleteList.Clear();

  //  Tide/Current Data Sources
  SetPath(_T ( "/TideCurrentDataSources" ));
  if (GetNumberOfEntries()) {
    TideCurrentDataSet.clear();
    wxString str, val;
    long dummy;
    bool bCont = GetFirstEntry(str, dummy);
    while (bCont) {
      Read(str, &val);  // Get a file name and add it to the list just in case it is not repeated
      // We have seen duplication of dataset entries in https://github.com/OpenCPN/OpenCPN/issues/3042, this
      // effectively gets rid of them.
      if (std::find(TideCurrentDataSet.begin(), TideCurrentDataSet.end(), val.ToStdString()) == TideCurrentDataSet.end()) {
        TideCurrentDataSet.push_back(val.ToStdString());
      }
      bCont = GetNextEntry(str, dummy);
    }
  }

  //    Groups
  LoadConfigGroups(g_pGroupArray);

  //     //    Multicanvas Settings
  //     LoadCanvasConfigs();

  SetPath(_T ( "/Settings/Others" ));

  // Radar rings
  Read(_T ( "RadarRingsNumberVisible" ), &val);
  if (val.Length() > 0) g_iNavAidRadarRingsNumberVisible = atoi(val.mb_str());

  Read(_T ( "RadarRingsStep" ), &val);
  if (val.Length() > 0) g_fNavAidRadarRingsStep = atof(val.mb_str());

  Read(_T ( "RadarRingsStepUnits" ), &g_pNavAidRadarRingsStepUnits);

  wxString l_wxsOwnshipRangeRingsColour;
  Read(_T ( "RadarRingsColour" ), &l_wxsOwnshipRangeRingsColour);
  if (l_wxsOwnshipRangeRingsColour.Length())
    g_colourOwnshipRangeRingsColour.Set(l_wxsOwnshipRangeRingsColour);

  // Waypoint Radar rings
  Read(_T ( "WaypointRangeRingsNumber" ), &val);
  if (val.Length() > 0) g_iWaypointRangeRingsNumber = atoi(val.mb_str());

  Read(_T ( "WaypointRangeRingsStep" ), &val);
  if (val.Length() > 0) g_fWaypointRangeRingsStep = atof(val.mb_str());

  Read(_T ( "WaypointRangeRingsStepUnits" ), &g_iWaypointRangeRingsStepUnits);

  wxString l_wxsWaypointRangeRingsColour;
  Read(_T( "WaypointRangeRingsColour" ), &l_wxsWaypointRangeRingsColour);
  g_colourWaypointRangeRingsColour.Set(l_wxsWaypointRangeRingsColour);

  if (!Read(_T("WaypointUseScaMin"), &g_bUseWptScaMin)) g_bUseWptScaMin = false;
  if (!Read(_T("WaypointScaMinValue"), &g_iWpt_ScaMin))
    g_iWpt_ScaMin = 2147483646;
  if (!Read(_T("WaypointUseScaMinOverrule"), &g_bOverruleScaMin))
    g_bOverruleScaMin = false;
  if (!Read(_T("WaypointsShowName"), &g_bShowWptName)) g_bShowWptName = true;

  //  Support Version 3.0 and prior config setting for Radar Rings
  bool b300RadarRings = true;
  if (Read(_T ( "ShowRadarRings" ), &b300RadarRings)) {
    if (!b300RadarRings) g_iNavAidRadarRingsNumberVisible = 0;
  }

  Read(_T ( "ConfirmObjectDeletion" ), &g_bConfirmObjectDelete);

  // Waypoint dragging with mouse
  g_bWayPointPreventDragging = false;
  Read(_T ( "WaypointPreventDragging" ), &g_bWayPointPreventDragging);

  g_bEnableZoomToCursor = false;
  Read(_T ( "EnableZoomToCursor" ), &g_bEnableZoomToCursor);

  val.Clear();
  Read(_T ( "TrackIntervalSeconds" ), &val);
  if (val.Length() > 0) {
    double tval = atof(val.mb_str());
    if (tval >= 2.) g_TrackIntervalSeconds = tval;
  }

  val.Clear();
  Read(_T ( "TrackDeltaDistance" ), &val);
  if (val.Length() > 0) {
    double tval = atof(val.mb_str());
    if (tval >= 0.05) g_TrackDeltaDistance = tval;
  }

  Read(_T ( "TrackPrecision" ), &g_nTrackPrecision);

  Read(_T ( "NavObjectFileName" ), m_sNavObjSetFile);

  Read(_T ( "RouteLineWidth" ), &g_route_line_width);
  Read(_T ( "TrackLineWidth" ), &g_track_line_width);

  wxString l_wxsTrackLineColour;
  if (Read(_T( "TrackLineColour" ), &l_wxsTrackLineColour))
    g_colourTrackLineColour.Set(l_wxsTrackLineColour);

  Read(_T ( "TideCurrentWindowScale" ), &g_tcwin_scale);
  Read(_T ( "DefaultWPIcon" ), &g_default_wp_icon);
  Read(_T ( "DefaultRPIcon" ), &g_default_routepoint_icon);

  SetPath(_T ( "/MmsiProperties" ));
  int iPMax = GetNumberOfEntries();
  if (iPMax) {
    g_MMSI_Props_Array.Empty();
    wxString str, val;
    long dummy;
    bool bCont = pConfig->GetFirstEntry(str, dummy);
    while (bCont) {
      pConfig->Read(str, &val);  // Get an entry

      MmsiProperties *pProps = new MmsiProperties(val);
      g_MMSI_Props_Array.Add(pProps);

      bCont = pConfig->GetNextEntry(str, dummy);
    }
  }

  return (0);
}

void MyConfig::LoadS57Config() {
  if (!ps52plib) return;

  int read_int;
  double dval;
  SetPath(_T ( "/Settings/GlobalState" ));

  Read(_T ( "bShowS57Text" ), &read_int, 1);
  ps52plib->SetShowS57Text(!(read_int == 0));

  Read(_T ( "bShowS57ImportantTextOnly" ), &read_int, 0);
  ps52plib->SetShowS57ImportantTextOnly(!(read_int == 0));

  Read(_T ( "bShowLightDescription" ), &read_int, 0);
  ps52plib->SetShowLdisText(!(read_int == 0));

  Read(_T ( "bExtendLightSectors" ), &read_int, 0);
  ps52plib->SetExtendLightSectors(!(read_int == 0));

  Read(_T ( "nDisplayCategory" ), &read_int, (enum _DisCat)STANDARD);
  ps52plib->SetDisplayCategory((enum _DisCat)read_int);

  Read(_T ( "nSymbolStyle" ), &read_int, (enum _LUPname)PAPER_CHART);
  ps52plib->m_nSymbolStyle = (LUPname)read_int;

  Read(_T ( "nBoundaryStyle" ), &read_int, PLAIN_BOUNDARIES);
  ps52plib->m_nBoundaryStyle = (LUPname)read_int;

  Read(_T ( "bShowSoundg" ), &read_int, 1);
  ps52plib->m_bShowSoundg = !(read_int == 0);

  Read(_T ( "bShowMeta" ), &read_int, 0);
  ps52plib->m_bShowMeta = !(read_int == 0);

  Read(_T ( "bUseSCAMIN" ), &read_int, 1);
  ps52plib->m_bUseSCAMIN = !(read_int == 0);

  Read(_T ( "bUseSUPER_SCAMIN" ), &read_int, 0);
  ps52plib->m_bUseSUPER_SCAMIN = !(read_int == 0);

  Read(_T ( "bShowAtonText" ), &read_int, 1);
  ps52plib->m_bShowAtonText = !(read_int == 0);

  Read(_T ( "bDeClutterText" ), &read_int, 0);
  ps52plib->m_bDeClutterText = !(read_int == 0);

  Read(_T ( "bShowNationalText" ), &read_int, 0);
  ps52plib->m_bShowNationalTexts = !(read_int == 0);

  Read(_T ( "ENCSoundingScaleFactor" ), &read_int, 0);
  ps52plib->m_nSoundingFactor = read_int;

  Read(_T ( "ENCTextScaleFactor" ), &read_int, 0);
  ps52plib->m_nTextFactor = read_int;

  if (Read(_T ( "S52_MAR_SAFETY_CONTOUR" ), &dval, 3.0)) {
    S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, dval);
    S52_setMarinerParam(S52_MAR_SAFETY_DEPTH,
                        dval);  // Set safety_contour and safety_depth the same
  }

  if (Read(_T ( "S52_MAR_SHALLOW_CONTOUR" ), &dval, 2.0))
    S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval);

  if (Read(_T ( "S52_MAR_DEEP_CONTOUR" ), &dval, 6.0))
    S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval);

  if (Read(_T ( "S52_MAR_TWO_SHADES" ), &dval, 0.0))
    S52_setMarinerParam(S52_MAR_TWO_SHADES, dval);

  ps52plib->UpdateMarinerParams();

  SetPath(_T ( "/Settings/GlobalState" ));
  Read(_T ( "S52_DEPTH_UNIT_SHOW" ), &read_int, 1);  // default is metres
  read_int = wxMax(read_int, 0);                     // qualify value
  read_int = wxMin(read_int, 2);
  ps52plib->m_nDepthUnitDisplay = read_int;
  g_nDepthUnitDisplay = read_int;

  //    S57 Object Class Visibility

  OBJLElement *pOLE;

  SetPath(_T ( "/Settings/ObjectFilter" ));

  int iOBJMax = GetNumberOfEntries();
  if (iOBJMax) {
    wxString str;
    long val;
    long dummy;

    wxString sObj;

    bool bCont = pConfig->GetFirstEntry(str, dummy);
    while (bCont) {
      pConfig->Read(str, &val);  // Get an Object Viz

      bool bNeedNew = true;

      if (str.StartsWith(_T ( "viz" ), &sObj)) {
        for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount();
             iPtr++) {
          pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));
          if (!strncmp(pOLE->OBJLName, sObj.mb_str(), 6)) {
            pOLE->nViz = val;
            bNeedNew = false;
            break;
          }
        }

        if (bNeedNew) {
          pOLE = (OBJLElement *)calloc(sizeof(OBJLElement), 1);
          memcpy(pOLE->OBJLName, sObj.mb_str(), OBJL_NAME_LEN);
          pOLE->nViz = 1;

          ps52plib->pOBJLArray->Add((void *)pOLE);
        }
      }
      bCont = pConfig->GetNextEntry(str, dummy);
    }
  }
}

/** Load changes from a pending changes file path. */
static bool ReloadPendingChanges(const wxString& changes_path) {
  wxULongLong size = wxFileName::GetSize(changes_path);

  // We crashed last time :(
  // That's why this file still exists...
  // Let's reconstruct the unsaved changes
  auto pNavObjectChangesSet = NavObjectChanges::getTempInstance();
  pNavObjectChangesSet->Init(changes_path);
  pNavObjectChangesSet->load_file(changes_path.fn_str());

  //  Remove the file before applying the changes,
  //  just in case the changes file itself causes a fault.
  //  If it does fault, at least the next restart will proceed without fault.
  if (::wxFileExists(changes_path))
    ::wxRemoveFile(changes_path);

  if (size == 0) return false;

  wxLogMessage(_T("Applying NavObjChanges"));
  pNavObjectChangesSet->ApplyChanges();
  return  true;
}

void MyConfig::LoadNavObjects() {
  //      next thing to do is read tracks, etc from the NavObject XML file,
  wxLogMessage(_T("Loading navobjects from navobj.xml"));
  CreateRotatingNavObjBackup();

  if (NULL == m_pNavObjectInputSet)
    m_pNavObjectInputSet = new NavObjectCollection1();

  int wpt_dups = 0;
  if (::wxFileExists(m_sNavObjSetFile) &&
      m_pNavObjectInputSet->load_file(m_sNavObjSetFile.fn_str()))
    m_pNavObjectInputSet->LoadAllGPXObjects(false, wpt_dups);

  wxLogMessage(_T("Done loading navobjects, %d duplicate waypoints ignored"),
               wpt_dups);
  delete m_pNavObjectInputSet;

  m_pNavObjectChangesSet = NavObjectChanges::getInstance();

  if (::wxFileExists(m_sNavObjSetChangesFile)) {
    if (ReloadPendingChanges(m_sNavObjSetChangesFile)) {
      UpdateNavObj();
    }
  }
  m_pNavObjectChangesSet->Init(m_sNavObjSetChangesFile);
  // Signal to listeners to g_active_route that it's possible to look up guid.
  GlobalVar<wxString> active_route(&g_active_route);
  active_route.Notify();
}

bool MyConfig::LoadLayers(wxString &path) {
  wxArrayString file_array;
  wxDir dir;
  Layer *l;
  dir.Open(path);
  if (dir.IsOpened()) {
    wxString filename;
    bool cont = dir.GetFirst(&filename);
    while (cont) {
      file_array.Clear();
      filename.Prepend(wxFileName::GetPathSeparator());
      filename.Prepend(path);
      wxFileName f(filename);
      size_t nfiles = 0;
      if (f.GetExt().IsSameAs(wxT("gpx")))
        file_array.Add(filename);  // single-gpx-file layer
      else {
        if (wxDir::Exists(filename)) {
          wxDir dir(filename);
          if (dir.IsOpened()) {
            nfiles = dir.GetAllFiles(filename, &file_array,
                                     wxT("*.gpx"));  // layers subdirectory set
          }
        }
      }

      if (file_array.GetCount()) {
        l = new Layer();
        l->m_LayerID = ++g_LayerIdx;
        l->m_LayerFileName = file_array[0];
        if (file_array.GetCount() <= 1)
          wxFileName::SplitPath(file_array[0], NULL, NULL, &(l->m_LayerName),
                                NULL, NULL);
        else
          wxFileName::SplitPath(filename, NULL, NULL, &(l->m_LayerName), NULL,
                                NULL);

        bool bLayerViz = g_bShowLayers;

        if (g_VisibleLayers.Contains(l->m_LayerName)) bLayerViz = true;
        if (g_InvisibleLayers.Contains(l->m_LayerName)) bLayerViz = false;

        l->m_bHasVisibleNames = wxCHK_UNDETERMINED;
        if (g_VisiNameinLayers.Contains(l->m_LayerName))
          l->m_bHasVisibleNames = wxCHK_CHECKED;
        if (g_InVisiNameinLayers.Contains(l->m_LayerName))
          l->m_bHasVisibleNames = wxCHK_UNCHECKED;

        l->m_bIsVisibleOnChart = bLayerViz;

        wxString laymsg;
        laymsg.Printf(wxT("New layer %d: %s"), l->m_LayerID,
                      l->m_LayerName.c_str());
        wxLogMessage(laymsg);

        pLayerList->Insert(l);

        //  Load the entire file array as a single layer

        for (unsigned int i = 0; i < file_array.GetCount(); i++) {
          wxString file_path = file_array[i];

          if (::wxFileExists(file_path)) {
            NavObjectCollection1 *pSet = new NavObjectCollection1;
            pSet->load_file(file_path.fn_str());
            long nItems = pSet->LoadAllGPXObjectsAsLayer(
                l->m_LayerID, bLayerViz, l->m_bHasVisibleNames);
            l->m_NoOfItems += nItems;
            l->m_LayerType = _("Persistent");

            wxString objmsg;
            objmsg.Printf(wxT("Loaded GPX file %s with %ld items."),
                          file_path.c_str(), nItems);
            wxLogMessage(objmsg);

            delete pSet;
          }
        }
      }

      cont = dir.GetNext(&filename);
    }
  }
  g_bLayersLoaded = true;

  return true;
}

bool MyConfig::LoadChartDirArray(ArrayOfCDI &ChartDirArray) {
  //    Chart Directories
  SetPath(_T ( "/ChartDirectories" ));
  int iDirMax = GetNumberOfEntries();
  if (iDirMax) {
    ChartDirArray.Empty();
    wxString str, val;
    long dummy;
    int nAdjustChartDirs = 0;
    int iDir = 0;
    bool bCont = pConfig->GetFirstEntry(str, dummy);
    while (bCont) {
      pConfig->Read(str, &val);  // Get a Directory name

      wxString dirname(val);
      if (!dirname.IsEmpty()) {
        /*     Special case for first time run after Windows install with sample
         chart data... We desire that the sample configuration file opencpn.ini
         should not contain any installation dependencies, so... Detect and
         update the sample [ChartDirectories] entries to point to the Shared
         Data directory For instance, if the (sample) opencpn.ini file should
         contain shortcut coded entries like:

         [ChartDirectories]
         ChartDir1=SampleCharts\\MaptechRegion7

         then this entry will be updated to be something like:
         ChartDir1=c:\Program Files\opencpn\SampleCharts\\MaptechRegion7

         */
        if (dirname.Find(_T ( "SampleCharts" )) ==
            0)  // only update entries starting with "SampleCharts"
        {
          nAdjustChartDirs++;

          pConfig->DeleteEntry(str);
          wxString new_dir = dirname.Mid(dirname.Find(_T ( "SampleCharts" )));
          new_dir.Prepend(g_Platform->GetSharedDataDir());
          dirname = new_dir;
        }

        ChartDirInfo cdi;
        cdi.fullpath = dirname.BeforeFirst('^');
        cdi.magic_number = dirname.AfterFirst('^');

        ChartDirArray.Add(cdi);
        iDir++;
      }

      bCont = pConfig->GetNextEntry(str, dummy);
    }

    if (nAdjustChartDirs) pConfig->UpdateChartDirs(ChartDirArray);
  }

  return true;
}

void MyConfig::AddNewRoute(Route *r) {
  m_pNavObjectChangesSet->AddNewRoute(r);
}

void MyConfig::UpdateRoute(Route *r) {
  m_pNavObjectChangesSet->UpdateRoute(r);
}

void MyConfig::DeleteConfigRoute(Route *pr) {
  m_pNavObjectChangesSet->DeleteConfigRoute(pr);
}

void MyConfig::AddNewTrack(Track *pt) {
    m_pNavObjectChangesSet->AddNewTrack(pt);
}

void MyConfig::UpdateTrack(Track *pt) {
    m_pNavObjectChangesSet->UpdateTrack(pt);
}

void MyConfig::DeleteConfigTrack(Track *pt) {
    m_pNavObjectChangesSet->DeleteConfigTrack(pt);
}

void MyConfig::AddNewWayPoint(RoutePoint *pWP, int crm) {
    m_pNavObjectChangesSet->AddNewWayPoint(pWP);
}

void MyConfig::UpdateWayPoint(RoutePoint *pWP) {
    m_pNavObjectChangesSet->UpdateWayPoint(pWP);
}

void MyConfig::DeleteWayPoint(RoutePoint *pWP) {
    m_pNavObjectChangesSet->DeleteWayPoint(pWP);
}

void MyConfig::AddNewTrackPoint(TrackPoint *pWP,
                                const wxString &parent_GUID) {
    m_pNavObjectChangesSet->AddNewTrackPoint(pWP, parent_GUID);
}

bool MyConfig::UpdateChartDirs(ArrayOfCDI &dir_array) {
  wxString key, dir;
  wxString str_buf;

  SetPath(_T ( "/ChartDirectories" ));
  int iDirMax = GetNumberOfEntries();
  if (iDirMax) {
    long dummy;

    for (int i = 0; i < iDirMax; i++) {
      GetFirstEntry(key, dummy);
      DeleteEntry(key, false);
    }
  }

  iDirMax = dir_array.GetCount();

  for (int iDir = 0; iDir < iDirMax; iDir++) {
    ChartDirInfo cdi = dir_array[iDir];

    wxString dirn = cdi.fullpath;
    dirn.Append(_T("^"));
    dirn.Append(cdi.magic_number);

    str_buf.Printf(_T ( "ChartDir%d" ), iDir + 1);

    Write(str_buf, dirn);
  }

// Avoid nonsense log errors...
#ifdef __OCPN__ANDROID__
  wxLogNull logNo;
#endif

  Flush();
  return true;
}

void MyConfig::CreateConfigGroups(ChartGroupArray *pGroupArray) {
  if (!pGroupArray) return;

  SetPath(_T ( "/Groups" ));
  Write(_T ( "GroupCount" ), (int)pGroupArray->GetCount());

  for (unsigned int i = 0; i < pGroupArray->GetCount(); i++) {
    ChartGroup *pGroup = pGroupArray->Item(i);
    wxString s;
    s.Printf(_T("Group%d"), i + 1);
    s.Prepend(_T ( "/Groups/" ));
    SetPath(s);

    Write(_T ( "GroupName" ), pGroup->m_group_name);
    Write(_T ( "GroupItemCount" ), (int)pGroup->m_element_array.size());

    for (unsigned int j = 0; j < pGroup->m_element_array.size(); j++) {
      wxString sg;
      sg.Printf(_T("Group%d/Item%d"), i + 1, j);
      sg.Prepend(_T ( "/Groups/" ));
      SetPath(sg);
      Write(_T ( "IncludeItem" ), pGroup->m_element_array[j].m_element_name);

      wxString t;
      wxArrayString u = pGroup->m_element_array[j].m_missing_name_array;
      if (u.GetCount()) {
        for (unsigned int k = 0; k < u.GetCount(); k++) {
          t += u[k];
          t += _T(";");
        }
        Write(_T ( "ExcludeItems" ), t);
      }
    }
  }
}

void MyConfig::DestroyConfigGroups(void) {
  DeleteGroup(_T ( "/Groups" ));  // zap
}

void MyConfig::LoadConfigGroups(ChartGroupArray *pGroupArray) {
  SetPath(_T ( "/Groups" ));
  unsigned int group_count;
  Read(_T ( "GroupCount" ), (int *)&group_count, 0);

  for (unsigned int i = 0; i < group_count; i++) {
    ChartGroup *pGroup = new ChartGroup;
    wxString s;
    s.Printf(_T("Group%d"), i + 1);
    s.Prepend(_T ( "/Groups/" ));
    SetPath(s);

    wxString t;
    Read(_T ( "GroupName" ), &t);
    pGroup->m_group_name = t;

    unsigned int item_count;
    Read(_T ( "GroupItemCount" ), (int *)&item_count);
    for (unsigned int j = 0; j < item_count; j++) {
      wxString sg;
      sg.Printf(_T("Group%d/Item%d"), i + 1, j);
      sg.Prepend(_T ( "/Groups/" ));
      SetPath(sg);

      wxString v;
      Read(_T ( "IncludeItem" ), &v);

      ChartGroupElement pelement{v};
      wxString u;
      if (Read(_T ( "ExcludeItems" ), &u)) {
        if (!u.IsEmpty()) {
          wxStringTokenizer tk(u, _T(";"));
          while (tk.HasMoreTokens()) {
            wxString token = tk.GetNextToken();
            pelement.m_missing_name_array.Add(token);
          }
        }
      }
      pGroup->m_element_array.push_back(std::move(pelement));
    }
    pGroupArray->Add(pGroup);
  }
}

void MyConfig::LoadCanvasConfigs(bool bApplyAsTemplate) {
  wxString s;
  canvasConfig *pcc;
  auto &config_array = ConfigMgr::Get().GetCanvasConfigArray();

  SetPath(_T ( "/Canvas" ));

  //  If the canvas config has never been set/persisted, use the global settings
  if (!HasEntry(_T ( "CanvasConfig" ))) {
    pcc = new canvasConfig(0);
    pcc->LoadFromLegacyConfig(this);
    config_array.Add(pcc);

    return;
  }

  Read(_T ( "CanvasConfig" ), (int *)&g_canvasConfig, 0);


  // Do not recreate canvasConfigs when applying config dynamically
  if (config_array.GetCount() == 0) {  // This is initial load from startup
    s.Printf(_T("/Canvas/CanvasConfig%d"), 1);
    SetPath(s);
    canvasConfig *pcca = new canvasConfig(0);
    LoadConfigCanvas(pcca, bApplyAsTemplate);
    config_array.Add(pcca);

    s.Printf(_T("/Canvas/CanvasConfig%d"), 2);
    SetPath(s);
    pcca = new canvasConfig(1);
    LoadConfigCanvas(pcca, bApplyAsTemplate);
    config_array.Add(pcca);
  } else {  // This is a dynamic (i.e. Template) load
    canvasConfig *pcca = config_array[0];
    s.Printf(_T("/Canvas/CanvasConfig%d"), 1);
    SetPath(s);
    LoadConfigCanvas(pcca, bApplyAsTemplate);

    if (config_array.GetCount() > 1) {
      canvasConfig *pcca = config_array[1];
      s.Printf(_T("/Canvas/CanvasConfig%d"), 2);
      SetPath(s);
      LoadConfigCanvas(pcca, bApplyAsTemplate);
    } else {
      s.Printf(_T("/Canvas/CanvasConfig%d"), 2);
      SetPath(s);
      pcca = new canvasConfig(1);
      LoadConfigCanvas(pcca, bApplyAsTemplate);
      config_array.Add(pcca);
    }
  }
}

void MyConfig::LoadConfigCanvas(canvasConfig *cConfig, bool bApplyAsTemplate) {
  wxString st;
  double st_lat, st_lon;

  if (!bApplyAsTemplate) {
    //    Reasonable starting point
    cConfig->iLat = START_LAT;  // display viewpoint
    cConfig->iLon = START_LON;

    if (Read(_T ( "canvasVPLatLon" ), &st)) {
      sscanf(st.mb_str(wxConvUTF8), "%lf,%lf", &st_lat, &st_lon);

      //    Sanity check the lat/lon...both have to be reasonable.
      if (fabs(st_lon) < 360.) {
        while (st_lon < -180.) st_lon += 360.;

        while (st_lon > 180.) st_lon -= 360.;

        cConfig->iLon = st_lon;
      }

      if (fabs(st_lat) < 90.0) cConfig->iLat = st_lat;
    }

    cConfig->iScale = .0003;  // decent initial value
    cConfig->iRotation = 0;

    double st_view_scale;
    if (Read(wxString(_T ( "canvasVPScale" )), &st)) {
      sscanf(st.mb_str(wxConvUTF8), "%lf", &st_view_scale);
      //    Sanity check the scale
      st_view_scale = fmax(st_view_scale, .001 / 32);
      st_view_scale = fmin(st_view_scale, 4);
      cConfig->iScale = st_view_scale;
    }

    double st_rotation;
    if (Read(wxString(_T ( "canvasVPRotation" )), &st)) {
      sscanf(st.mb_str(wxConvUTF8), "%lf", &st_rotation);
      //    Sanity check the rotation
      st_rotation = fmin(st_rotation, 360);
      st_rotation = fmax(st_rotation, 0);
      cConfig->iRotation = st_rotation * PI / 180.;
    }

    Read(_T ( "canvasInitialdBIndex" ), &cConfig->DBindex, 0);
    Read(_T ( "canvasbFollow" ), &cConfig->bFollow, 0);

    Read(_T ( "canvasCourseUp" ), &cConfig->bCourseUp, 0);
    Read(_T ( "canvasHeadUp" ), &cConfig->bHeadUp, 0);
    Read(_T ( "canvasLookahead" ), &cConfig->bLookahead, 0);
  }

  Read(_T ( "ActiveChartGroup" ), &cConfig->GroupID, 0);

  // Special check for group selection when applied as template
  if (cConfig->GroupID && bApplyAsTemplate) {
    if (cConfig->GroupID > (int)g_pGroupArray->GetCount()) cConfig->GroupID = 0;
  }

  Read(_T ( "canvasShowTides" ), &cConfig->bShowTides, 0);
  Read(_T ( "canvasShowCurrents" ), &cConfig->bShowCurrents, 0);

  Read(_T ( "canvasQuilt" ), &cConfig->bQuilt, 1);
  Read(_T ( "canvasShowGrid" ), &cConfig->bShowGrid, 0);
  Read(_T ( "canvasShowOutlines" ), &cConfig->bShowOutlines, 0);
  Read(_T ( "canvasShowDepthUnits" ), &cConfig->bShowDepthUnits, 0);

  Read(_T ( "canvasShowAIS" ), &cConfig->bShowAIS, 1);
  Read(_T ( "canvasAttenAIS" ), &cConfig->bAttenAIS, 0);

  // ENC options
  Read(_T ( "canvasShowENCText" ), &cConfig->bShowENCText, 1);
  Read(_T ( "canvasENCDisplayCategory" ), &cConfig->nENCDisplayCategory,
       STANDARD);
  Read(_T ( "canvasENCShowDepths" ), &cConfig->bShowENCDepths, 1);
  Read(_T ( "canvasENCShowBuoyLabels" ), &cConfig->bShowENCBuoyLabels, 1);
  Read(_T ( "canvasENCShowLightDescriptions" ),
       &cConfig->bShowENCLightDescriptions, 1);
  Read(_T ( "canvasENCShowLights" ), &cConfig->bShowENCLights, 1);
  Read(_T ( "canvasENCShowVisibleSectorLights" ),
       &cConfig->bShowENCVisibleSectorLights, 0);
  Read(_T ( "canvasENCShowAnchorInfo" ), &cConfig->bShowENCAnchorInfo, 0);
  Read(_T ( "canvasENCShowDataQuality" ), &cConfig->bShowENCDataQuality, 0);

  int sx, sy;
  Read(_T ( "canvasSizeX" ), &sx, 0);
  Read(_T ( "canvasSizeY" ), &sy, 0);
  cConfig->canvasSize = wxSize(sx, sy);
}

void MyConfig::SaveCanvasConfigs() {
  auto &config_array = ConfigMgr::Get().GetCanvasConfigArray();

  SetPath(_T ( "/Canvas" ));
  Write(_T ( "CanvasConfig" ), (int)g_canvasConfig);

  wxString s;
  canvasConfig *pcc;

  switch (g_canvasConfig) {
    case 0:
    default:

      s.Printf(_T("/Canvas/CanvasConfig%d"), 1);
      SetPath(s);

      if (config_array.GetCount() > 0) {
        pcc = config_array.Item(0);
        if (pcc) {
          SaveConfigCanvas(pcc);
        }
      }
      break;

    case 1:

      if (config_array.GetCount() > 1) {
        s.Printf(_T("/Canvas/CanvasConfig%d"), 1);
        SetPath(s);
        pcc = config_array.Item(0);
        if (pcc) {
          SaveConfigCanvas(pcc);
        }

        s.Printf(_T("/Canvas/CanvasConfig%d"), 2);
        SetPath(s);
        pcc = config_array.Item(1);
        if (pcc) {
          SaveConfigCanvas(pcc);
        }
      }
      break;
  }
}

void MyConfig::SaveConfigCanvas(canvasConfig *cConfig) {
  wxString st1;

  if (cConfig->canvas) {
    ViewPort vp = cConfig->canvas->GetVP();

    if (vp.IsValid()) {
      st1.Printf(_T ( "%10.4f,%10.4f" ), vp.clat, vp.clon);
      Write(_T ( "canvasVPLatLon" ), st1);
      st1.Printf(_T ( "%g" ), vp.view_scale_ppm);
      Write(_T ( "canvasVPScale" ), st1);
      st1.Printf(_T ( "%i" ), ((int)(vp.rotation * 180 / PI)) % 360);
      Write(_T ( "canvasVPRotation" ), st1);
    }

    int restore_dbindex = 0;
    ChartStack *pcs = cConfig->canvas->GetpCurrentStack();
    if (pcs) restore_dbindex = pcs->GetCurrentEntrydbIndex();
    if (cConfig->canvas->GetQuiltMode())
      restore_dbindex = cConfig->canvas->GetQuiltReferenceChartIndex();
    Write(_T ( "canvasInitialdBIndex" ), restore_dbindex);

    Write(_T ( "canvasbFollow" ), cConfig->canvas->m_bFollow);
    Write(_T ( "ActiveChartGroup" ), cConfig->canvas->m_groupIndex);

    Write(_T ( "canvasToolbarConfig" ),
          cConfig->canvas->GetToolbarConfigString());
    Write(_T ( "canvasShowToolbar" ),
          0);  // cConfig->canvas->GetToolbarEnable() );

    Write(_T ( "canvasQuilt" ), cConfig->canvas->GetQuiltMode());
    Write(_T ( "canvasShowGrid" ), cConfig->canvas->GetShowGrid());
    Write(_T ( "canvasShowOutlines" ), cConfig->canvas->GetShowOutlines());
    Write(_T ( "canvasShowDepthUnits" ), cConfig->canvas->GetShowDepthUnits());

    Write(_T ( "canvasShowAIS" ), cConfig->canvas->GetShowAIS());
    Write(_T ( "canvasAttenAIS" ), cConfig->canvas->GetAttenAIS());

    Write(_T ( "canvasShowTides" ), cConfig->canvas->GetbShowTide());
    Write(_T ( "canvasShowCurrents" ), cConfig->canvas->GetbShowCurrent());

    // ENC options
    Write(_T ( "canvasShowENCText" ), cConfig->canvas->GetShowENCText());
    Write(_T ( "canvasENCDisplayCategory" ),
          cConfig->canvas->GetENCDisplayCategory());
    Write(_T ( "canvasENCShowDepths" ), cConfig->canvas->GetShowENCDepth());
    Write(_T ( "canvasENCShowBuoyLabels" ),
          cConfig->canvas->GetShowENCBuoyLabels());
    Write(_T ( "canvasENCShowLightDescriptions" ),
          cConfig->canvas->GetShowENCLightDesc());
    Write(_T ( "canvasENCShowLights" ), cConfig->canvas->GetShowENCLights());
    Write(_T ( "canvasENCShowVisibleSectorLights" ),
          cConfig->canvas->GetShowVisibleSectors());
    Write(_T ( "canvasENCShowAnchorInfo" ),
          cConfig->canvas->GetShowENCAnchor());
    Write(_T ( "canvasENCShowDataQuality" ),
          cConfig->canvas->GetShowENCDataQual());
    Write(_T ( "canvasCourseUp" ),
          cConfig->canvas->GetUpMode() == COURSE_UP_MODE);
    Write(_T ( "canvasHeadUp" ), cConfig->canvas->GetUpMode() == HEAD_UP_MODE);
    Write(_T ( "canvasLookahead" ), cConfig->canvas->GetLookahead());

    int width = cConfig->canvas->GetSize().x;
    //         if(cConfig->canvas->IsPrimaryCanvas()){
    //             width = wxMax(width, gFrame->GetClientSize().x / 10);
    //         }
    //         else{
    //             width = wxMin(width, gFrame->GetClientSize().x  * 9 / 10);
    //         }

    Write(_T ( "canvasSizeX" ), width);
    Write(_T ( "canvasSizeY" ), cConfig->canvas->GetSize().y);
  }
}

void MyConfig::UpdateSettings() {
  //  Temporarily suppress logging of trivial non-fatal wxLogSysError() messages
  //  provoked by Android security...
#ifdef __OCPN__ANDROID__
  wxLogNull logNo;
#endif

  //    Global options and settings
  SetPath(_T ( "/Settings" ));

  Write(_T ( "LastAppliedTemplate" ), g_lastAppliedTemplateGUID);
  Write(_T ( "CompatOS" ), g_compatOS);
  Write(_T ( "CompatOsVersion" ), g_compatOsVersion);
  Write(_T ( "ConfigVersionString" ), g_config_version_string);
  if (wxIsEmpty(g_CmdSoundString))
    g_CmdSoundString = wxString(OCPN_SOUND_CMD);
  Write(_T( "CmdSoundString" ), g_CmdSoundString);
  Write(_T ( "NavMessageShown" ), n_NavMessageShown);
  Write(_T ( "InlandEcdis" ), g_bInlandEcdis);

  Write(_T ( "AndroidVersionCode" ), g_AndroidVersionCode);

  Write(_T ( "UIexpert" ), g_bUIexpert);
  Write(_T( "SpaceDropMark" ), g_bSpaceDropMark);
  //    Write( _T ( "UIStyle" ), g_StyleManager->GetStyleNextInvocation() );
  //    //Not desired for O5 MUI

  Write(_T ( "ShowStatusBar" ), g_bShowStatusBar);
#ifndef __WXOSX__
  Write(_T ( "ShowMenuBar" ), g_bShowMenuBar);
#endif
  Write(_T ( "DefaultFontSize" ), g_default_font_size);
  Write(_T ( "DefaultFontFacename" ), g_default_font_facename);

  Write(_T ( "Fullscreen" ), g_bFullscreen);
  Write(_T ( "ShowCompassWindow" ), g_bShowCompassWin);
  Write(_T ( "SetSystemTime" ), s_bSetSystemTime);
  Write(_T ( "ShowGrid" ), g_bDisplayGrid);
  Write(_T ( "PlayShipsBells" ), g_bPlayShipsBells);
  Write(_T ( "SoundDeviceIndex" ), g_iSoundDeviceIndex);
  Write(_T ( "FullscreenToolbar" ), g_bFullscreenToolbar);
  Write(_T ( "TransparentToolbar" ), g_bTransparentToolbar);
  Write(_T ( "PermanentMOBIcon" ), g_bPermanentMOBIcon);
  Write(_T ( "ShowLayers" ), g_bShowLayers);
  Write(_T ( "AutoAnchorDrop" ), g_bAutoAnchorMark);
  Write(_T ( "ShowChartOutlines" ), g_bShowOutlines);
  Write(_T ( "ShowActiveRouteTotal" ), g_bShowRouteTotal);
  Write(_T ( "ShowActiveRouteHighway" ), g_bShowActiveRouteHighway);
  Write(_T ( "SDMMFormat" ), g_iSDMMFormat);
  Write(_T ( "MostRecentGPSUploadConnection" ), g_uploadConnection);
  Write(_T ( "ShowChartBar" ), g_bShowChartBar);

  Write(_T ( "GUIScaleFactor" ), g_GUIScaleFactor);
  Write(_T ( "ChartObjectScaleFactor" ), g_ChartScaleFactor);
  Write(_T ( "ShipScaleFactor" ), g_ShipScaleFactor);
  Write(_T ( "ENCSoundingScaleFactor" ), g_ENCSoundingScaleFactor);
  Write(_T ( "ENCTextScaleFactor" ), g_ENCTextScaleFactor);
  Write(_T ( "ObjQueryAppendFilesExt" ), g_ObjQFileExt);

  // Plugin catalog persistent values.
  Write(_T( "CatalogCustomURL"), g_catalog_custom_url);
  Write(_T( "CatalogChannel"), g_catalog_channel);

  Write(_T ( "FilterNMEA_Avg" ), g_bfilter_cogsog);
  Write(_T ( "FilterNMEA_Sec" ), g_COGFilterSec);

  Write(_T ( "TrackContinuous" ), g_btrackContinuous);

  Write(_T ( "ShowTrue" ), g_bShowTrue);
  Write(_T ( "ShowMag" ), g_bShowMag);
  Write(_T ( "UserMagVariation" ), wxString::Format(_T("%.2f"), g_UserVar));

  Write(_T ( "CM93DetailFactor" ), g_cm93_zoom_factor);
  Write(_T ( "CM93DetailZoomPosX" ), g_detailslider_dialog_x);
  Write(_T ( "CM93DetailZoomPosY" ), g_detailslider_dialog_y);
  Write(_T ( "ShowCM93DetailSlider" ), g_bShowDetailSlider);

  Write(_T ( "SkewToNorthUp" ), g_bskew_comp);
  Write(_T ( "OpenGL" ), g_bopengl);
  Write(_T ( "DisableOpenGL" ), g_bdisable_opengl);
  Write(_T ( "SoftwareGL" ), g_bSoftwareGL);
  Write(_T ( "ShowFPS" ), g_bShowFPS);

  Write(_T ( "ZoomDetailFactor" ), g_chart_zoom_modifier_raster);
  Write(_T ( "ZoomDetailFactorVector" ), g_chart_zoom_modifier_vector);

  Write(_T ( "FogOnOverzoom" ), g_fog_overzoom);
  Write(_T ( "OverzoomVectorScale" ), g_oz_vector_scale);
  Write(_T ( "OverzoomEmphasisBase" ), g_overzoom_emphasis_base);
  Write(_T ( "PlusMinusZoomFactor" ), g_plus_minus_zoom_factor);
  Write("MouseZoomSensitivity",
        MouseZoom::ui_to_config(g_mouse_zoom_sensitivity_ui));
  Write(_T ( "ShowMUIZoomButtons" ), g_bShowMuiZoomButtons);

#ifdef ocpnUSE_GL
  /* opengl options */
  Write(_T ( "UseAcceleratedPanning" ), g_GLOptions.m_bUseAcceleratedPanning);

  Write(_T ( "GPUTextureCompression" ), g_GLOptions.m_bTextureCompression);
  Write(_T ( "GPUTextureCompressionCaching" ),
        g_GLOptions.m_bTextureCompressionCaching);
  Write(_T ( "GPUTextureDimension" ), g_GLOptions.m_iTextureDimension);
  Write(_T ( "GPUTextureMemSize" ), g_GLOptions.m_iTextureMemorySize);
  Write(_T ( "PolygonSmoothing" ), g_GLOptions.m_GLPolygonSmoothing);
  Write(_T ( "LineSmoothing" ), g_GLOptions.m_GLLineSmoothing);
#endif
  Write(_T ( "SmoothPanZoom" ), g_bsmoothpanzoom);

  Write(_T ( "CourseUpMode" ), g_bCourseUp);
  if (!g_bInlandEcdis) Write(_T ( "LookAheadMode" ), g_bLookAhead);
  Write(_T ( "COGUPAvgSeconds" ), g_COGAvgSec);
  Write(_T ( "UseMagAPB" ), g_bMagneticAPB);

  Write(_T ( "OwnshipCOGPredictorMinutes" ), g_ownship_predictor_minutes);
  Write(_T ( "OwnshipCOGPredictorWidth" ), g_cog_predictor_width);
  Write(_T ( "OwnshipHDTPredictorMiles" ), g_ownship_HDTpredictor_miles);
  Write(_T ( "OwnShipIconType" ), g_OwnShipIconType);
  Write(_T ( "OwnShipLength" ), g_n_ownship_length_meters);
  Write(_T ( "OwnShipWidth" ), g_n_ownship_beam_meters);
  Write(_T ( "OwnShipGPSOffsetX" ), g_n_gps_antenna_offset_x);
  Write(_T ( "OwnShipGPSOffsetY" ), g_n_gps_antenna_offset_y);
  Write(_T ( "OwnShipMinSize" ), g_n_ownship_min_mm);
  Write(_T ( "OwnShipSogCogCalc" ), g_own_ship_sog_cog_calc);
  Write(_T ( "OwnShipSogCogCalcDampSec"), g_own_ship_sog_cog_calc_damp_sec);
  Write(_T ( "ShowDirectRouteLine"), g_bShowShipToActive);
  Write(_T ( "DirectRouteLineStyle"), g_shipToActiveStyle);
  Write(_T ( "DirectRouteLineColor" ), g_shipToActiveColor);

  wxString racr;
  //   racr.Printf( _T ( "%g" ), g_n_arrival_circle_radius );
  //   Write( _T ( "RouteArrivalCircleRadius" ), racr );
  Write(_T ( "RouteArrivalCircleRadius" ),
        wxString::Format(_T("%.2f"), g_n_arrival_circle_radius));

  Write(_T ( "ChartQuilting" ), g_bQuiltEnable);

  Write(_T ( "NMEALogWindowSizeX" ), NMEALogWindow::Get().GetSizeW());
  Write(_T ( "NMEALogWindowSizeY" ), NMEALogWindow::Get().GetSizeH());
  Write(_T ( "NMEALogWindowPosX" ), NMEALogWindow::Get().GetPosX());
  Write(_T ( "NMEALogWindowPosY" ), NMEALogWindow::Get().GetPosY());

  Write(_T ( "PreserveScaleOnX" ), g_bPreserveScaleOnX);

  Write(_T ( "StartWithTrackActive" ), g_bTrackCarryOver);
  Write(_T ( "AutomaticDailyTracks" ), g_bTrackDaily);
  Write(_T ( "TrackRotateAt" ), g_track_rotate_time);
  Write(_T ( "TrackRotateTimeType" ), g_track_rotate_time_type);
  Write(_T ( "HighlightTracks" ), g_bHighliteTracks);

  Write(_T ( "InitialStackIndex" ), g_restore_stackindex);
  Write(_T ( "InitialdBIndex" ), g_restore_dbindex);

  Write(_T( "NMEAAPBPrecision" ), g_NMEAAPBPrecision);

  Write(_T("TalkerIdText"), g_TalkerIdText);
  Write(_T("ShowTrackPointTime"), g_bShowTrackPointTime);

  Write(_T ( "AnchorWatch1GUID" ), g_AW1GUID);
  Write(_T ( "AnchorWatch2GUID" ), g_AW2GUID);

  Write(_T ( "ToolbarX" ), g_maintoolbar_x);
  Write(_T ( "ToolbarY" ), g_maintoolbar_y);
  // Write( _T ( "ToolbarOrient" ), g_maintoolbar_orient );

  Write(_T ( "iENCToolbarX" ), g_iENCToolbarPosX);
  Write(_T ( "iENCToolbarY" ), g_iENCToolbarPosY);

  if (!g_bInlandEcdis) {
    Write(_T ( "GlobalToolbarConfig" ), g_toolbarConfig);
    Write(_T ( "DistanceFormat" ), g_iDistanceFormat);
    Write(_T ( "SpeedFormat" ), g_iSpeedFormat);
    Write(_T ( "WindSpeedFormat" ), g_iWindSpeedFormat);
    Write(_T ( "ShowDepthUnits" ), g_bShowDepthUnits);
    Write(_T ( "TemperatureFormat" ), g_iTempFormat);
  }
  Write(_T ( "GPSIdent" ), g_GPS_Ident);
  Write("ActiveRoute" , g_active_route);
  Write("PersistActiveRoute", g_persist_active_route);
  Write(_T ( "UseGarminHostUpload" ), g_bGarminHostUpload);

  Write(_T ( "MobileTouch" ), g_btouch);
  Write(_T ( "ResponsiveGraphics" ), g_bresponsive);
  Write(_T ( "EnableRolloverBlock" ), g_bRollover);

  Write(_T ( "AutoHideToolbar" ), g_bAutoHideToolbar);
  Write(_T ( "AutoHideToolbarSecs" ), g_nAutoHideToolbar);

  Write(_T ( "DisplaySizeMM" ), g_config_display_size_mm);
  Write(_T ( "DisplaySizeManual" ), g_config_display_size_manual);

  Write(_T ( "SelectionRadiusMM" ), g_selection_radius_mm);
  Write(_T ( "SelectionRadiusTouchMM" ), g_selection_radius_touch_mm);

  wxString st0;
  st0.Printf(_T ( "%g" ), g_PlanSpeed);
  Write(_T ( "PlanSpeed" ), st0);

  if (g_bLayersLoaded) {
    wxString vis, invis, visnames, invisnames;
    LayerList::iterator it;
    int index = 0;
    for (it = (*pLayerList).begin(); it != (*pLayerList).end(); ++it, ++index) {
      Layer *lay = (Layer *)(*it);
      if (lay->IsVisibleOnChart())
        vis += (lay->m_LayerName) + _T(";");
      else
        invis += (lay->m_LayerName) + _T(";");

      if (lay->HasVisibleNames() == wxCHK_CHECKED) {
        visnames += (lay->m_LayerName) + _T(";");
      } else if (lay->HasVisibleNames() == wxCHK_UNCHECKED) {
        invisnames += (lay->m_LayerName) + _T(";");
      }
    }
    Write(_T ( "VisibleLayers" ), vis);
    Write(_T ( "InvisibleLayers" ), invis);
    Write(_T ( "VisNameInLayers" ), visnames);
    Write(_T ( "InvisNameInLayers" ), invisnames);
  }
  Write(_T ( "Locale" ), g_locale);
  Write(_T ( "LocaleOverride" ), g_localeOverride);

  Write(_T ( "KeepNavobjBackups" ), g_navobjbackups);
  Write(_T ( "LegacyInputCOMPortFilterBehaviour" ),
        g_b_legacy_input_filter_behaviour);
  Write(_T( "AdvanceRouteWaypointOnArrivalOnly" ),
        g_bAdvanceRouteWaypointOnArrivalOnly);

  // LIVE ETA OPTION
  Write(_T( "LiveETA" ), g_bShowLiveETA);
  Write(_T( "DefaultBoatSpeed" ), g_defaultBoatSpeed);

  //    S57 Object Filter Settings

  SetPath(_T ( "/Settings/ObjectFilter" ));

  if (ps52plib) {
    for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount();
         iPtr++) {
      OBJLElement *pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));

      wxString st1(_T ( "viz" ));
      char name[7];
      strncpy(name, pOLE->OBJLName, 6);
      name[6] = 0;
      st1.Append(wxString(name, wxConvUTF8));
      Write(st1, pOLE->nViz);
    }
  }

  //    Global State

  SetPath(_T ( "/Settings/GlobalState" ));

  wxString st1;

  //     if( cc1 ) {
  //         ViewPort vp = cc1->GetVP();
  //
  //         if( vp.IsValid() ) {
  //             st1.Printf( _T ( "%10.4f,%10.4f" ), vp.clat, vp.clon );
  //             Write( _T ( "VPLatLon" ), st1 );
  //             st1.Printf( _T ( "%g" ), vp.view_scale_ppm );
  //             Write( _T ( "VPScale" ), st1 );
  //             st1.Printf( _T ( "%i" ), ((int)(vp.rotation * 180 / PI)) % 360
  //             ); Write( _T ( "VPRotation" ), st1 );
  //         }
  //     }

  st1.Printf(_T ( "%10.4f, %10.4f" ), gLat, gLon);
  Write(_T ( "OwnShipLatLon" ), st1);

  //    Various Options
  SetPath(_T ( "/Settings/GlobalState" ));
  if (!g_bInlandEcdis)
    Write(_T ( "nColorScheme" ), (int)gFrame->GetColorScheme());

  Write(_T ( "FrameWinX" ), g_nframewin_x);
  Write(_T ( "FrameWinY" ), g_nframewin_y);
  Write(_T ( "FrameWinPosX" ), g_nframewin_posx);
  Write(_T ( "FrameWinPosY" ), g_nframewin_posy);
  Write(_T ( "FrameMax" ), g_bframemax);

  Write(_T ( "ClientPosX" ), g_lastClientRectx);
  Write(_T ( "ClientPosY" ), g_lastClientRecty);
  Write(_T ( "ClientSzX" ), g_lastClientRectw);
  Write(_T ( "ClientSzY" ), g_lastClientRecth);

  Write(_T ( "S52_DEPTH_UNIT_SHOW" ), g_nDepthUnitDisplay);

  Write(_T( "RoutePropSizeX" ), g_route_prop_sx);
  Write(_T( "RoutePropSizeY" ), g_route_prop_sy);
  Write(_T( "RoutePropPosX" ), g_route_prop_x);
  Write(_T( "RoutePropPosY" ), g_route_prop_y);

  // Sounds
  SetPath(_T ( "/Settings/Audio" ));
  Write(_T ( "AISAlertSoundFile" ), g_AIS_sound_file);
  Write(_T ( "DSCAlertSoundFile" ), g_DSC_sound_file);
  Write(_T ( "SARTAlertSoundFile" ), g_SART_sound_file);
  Write(_T ( "AnchorAlarmSoundFile" ), g_anchorwatch_sound_file);

  Write(_T ( "bAIS_GCPA_AlertAudio" ), g_bAIS_GCPA_Alert_Audio);
  Write(_T ( "bAIS_SART_AlertAudio" ), g_bAIS_SART_Alert_Audio);
  Write(_T ( "bAIS_DSC_AlertAudio" ), g_bAIS_DSC_Alert_Audio);
  Write(_T ( "bAnchorAlertAudio" ), g_bAnchor_Alert_Audio);

  //    AIS
  SetPath(_T ( "/Settings/AIS" ));

  Write(_T ( "bNoCPAMax" ), g_bCPAMax);
  Write(_T ( "NoCPAMaxNMi" ), g_CPAMax_NM);
  Write(_T ( "bCPAWarn" ), g_bCPAWarn);
  Write(_T ( "CPAWarnNMi" ), g_CPAWarn_NM);
  Write(_T ( "bTCPAMax" ), g_bTCPA_Max);
  Write(_T ( "TCPAMaxMinutes" ), g_TCPA_Max);
  Write(_T ( "bMarkLostTargets" ), g_bMarkLost);
  Write(_T ( "MarkLost_Minutes" ), g_MarkLost_Mins);
  Write(_T ( "bRemoveLostTargets" ), g_bRemoveLost);
  Write(_T ( "RemoveLost_Minutes" ), g_RemoveLost_Mins);
  Write(_T ( "bShowCOGArrows" ), g_bShowCOG);
  Write(_T ( "bSyncCogPredictors" ), g_bSyncCogPredictors);
  Write(_T ( "CogArrowMinutes" ), g_ShowCOG_Mins);
  Write(_T ( "bShowTargetTracks" ), g_bAISShowTracks);
  Write(_T ( "TargetTracksMinutes" ), g_AISShowTracks_Mins);

  Write(_T ( "bHideMooredTargets" ), g_bHideMoored);
  Write(_T ( "MooredTargetMaxSpeedKnots" ), g_ShowMoored_Kts);

  Write(_T ( "bAISAlertDialog" ), g_bAIS_CPA_Alert);
  Write(_T ( "bAISAlertAudio" ), g_bAIS_CPA_Alert_Audio);

  Write(_T ( "AISAlertAudioFile" ), g_sAIS_Alert_Sound_File);
  Write(_T ( "bAISAlertSuppressMoored" ), g_bAIS_CPA_Alert_Suppress_Moored);
  Write(_T ( "bShowAreaNotices" ), g_bShowAreaNotices);
  Write(_T ( "bDrawAISSize" ), g_bDrawAISSize);
  Write(_T ( "bDrawAISRealtime" ), g_bDrawAISRealtime);
  Write(_T ( "AISRealtimeMinSpeedKnots" ), g_AIS_RealtPred_Kts);
  Write(_T ( "bShowAISName" ), g_bShowAISName);
  Write(_T ( "ShowAISTargetNameScale" ), g_Show_Target_Name_Scale);
  Write(_T ( "bWplIsAprsPositionReport" ), g_bWplUsePosition);
  Write(_T ( "WplSelAction" ), g_WplAction);
  Write(_T ( "AISCOGPredictorWidth" ), g_ais_cog_predictor_width);
  Write(_T ( "bShowScaledTargets" ), g_bAllowShowScaled);
  Write(_T ( "AISScaledNumber" ), g_ShowScaled_Num);
  Write(_T ( "AISScaledNumberWeightSOG" ), g_ScaledNumWeightSOG);
  Write(_T ( "AISScaledNumberWeightCPA" ), g_ScaledNumWeightCPA);
  Write(_T ( "AISScaledNumberWeightTCPA" ), g_ScaledNumWeightTCPA);
  Write(_T ( "AISScaledNumberWeightRange" ), g_ScaledNumWeightRange);
  Write(_T ( "AISScaledNumberWeightSizeOfTarget" ), g_ScaledNumWeightSizeOfT);
  Write(_T ( "AISScaledSizeMinimal" ), g_ScaledSizeMinimal);
  Write(_T ( "AISShowScaled"), g_bShowScaled);

  Write(_T ( "AlertDialogSizeX" ), g_ais_alert_dialog_sx);
  Write(_T ( "AlertDialogSizeY" ), g_ais_alert_dialog_sy);
  Write(_T ( "AlertDialogPosX" ), g_ais_alert_dialog_x);
  Write(_T ( "AlertDialogPosY" ), g_ais_alert_dialog_y);
  Write(_T ( "QueryDialogPosX" ), g_ais_query_dialog_x);
  Write(_T ( "QueryDialogPosY" ), g_ais_query_dialog_y);
  Write(_T ( "AISTargetListPerspective" ), g_AisTargetList_perspective);
  Write(_T ( "AISTargetListRange" ), g_AisTargetList_range);
  Write(_T ( "AISTargetListSortColumn" ), g_AisTargetList_sortColumn);
  Write(_T ( "bAISTargetListSortReverse" ), g_bAisTargetList_sortReverse);
  Write(_T ( "AISTargetListColumnSpec" ), g_AisTargetList_column_spec);
  Write(_T ("AISTargetListColumnOrder"), g_AisTargetList_column_order);

  Write(_T ( "S57QueryDialogSizeX" ), g_S57_dialog_sx);
  Write(_T ( "S57QueryDialogSizeY" ), g_S57_dialog_sy);
  Write(_T ( "S57QueryExtraDialogSizeX" ), g_S57_extradialog_sx);
  Write(_T ( "S57QueryExtraDialogSizeY" ), g_S57_extradialog_sy);

  Write(_T ( "bAISRolloverShowClass" ), g_bAISRolloverShowClass);
  Write(_T ( "bAISRolloverShowCOG" ), g_bAISRolloverShowCOG);
  Write(_T ( "bAISRolloverShowCPA" ), g_bAISRolloverShowCPA);

  Write(_T ( "bAISAlertAckTimeout" ), g_bAIS_ACK_Timeout);
  Write(_T ( "AlertAckTimeoutMinutes" ), g_AckTimeout_Mins);

  SetPath(_T ( "/Settings/GlobalState" ));
  if (ps52plib) {
    Write(_T ( "bShowS57Text" ), ps52plib->GetShowS57Text());
    Write(_T ( "bShowS57ImportantTextOnly" ),
          ps52plib->GetShowS57ImportantTextOnly());
    if (!g_bInlandEcdis)
      Write(_T ( "nDisplayCategory" ), (long)ps52plib->GetDisplayCategory());
    Write(_T ( "nSymbolStyle" ), (int)ps52plib->m_nSymbolStyle);
    Write(_T ( "nBoundaryStyle" ), (int)ps52plib->m_nBoundaryStyle);

    Write(_T ( "bShowSoundg" ), ps52plib->m_bShowSoundg);
    Write(_T ( "bShowMeta" ), ps52plib->m_bShowMeta);
    Write(_T ( "bUseSCAMIN" ), ps52plib->m_bUseSCAMIN);
    Write(_T ( "bUseSUPER_SCAMIN" ), ps52plib->m_bUseSUPER_SCAMIN);
    Write(_T ( "bShowAtonText" ), ps52plib->m_bShowAtonText);
    Write(_T ( "bShowLightDescription" ), ps52plib->m_bShowLdisText);
    Write(_T ( "bExtendLightSectors" ), ps52plib->m_bExtendLightSectors);
    Write(_T ( "bDeClutterText" ), ps52plib->m_bDeClutterText);
    Write(_T ( "bShowNationalText" ), ps52plib->m_bShowNationalTexts);

    Write(_T ( "S52_MAR_SAFETY_CONTOUR" ),
          S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR));
    Write(_T ( "S52_MAR_SHALLOW_CONTOUR" ),
          S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR));
    Write(_T ( "S52_MAR_DEEP_CONTOUR" ),
          S52_getMarinerParam(S52_MAR_DEEP_CONTOUR));
    Write(_T ( "S52_MAR_TWO_SHADES" ), S52_getMarinerParam(S52_MAR_TWO_SHADES));
    Write(_T ( "S52_DEPTH_UNIT_SHOW" ), ps52plib->m_nDepthUnitDisplay);
    Write(_T ( "ENCSoundingScaleFactor" ), g_ENCSoundingScaleFactor);
    Write(_T ( "ENCTextScaleFactor" ), g_ENCTextScaleFactor);
  }
  SetPath(_T ( "/Directories" ));
  Write(_T ( "S57DataLocation" ), _T(""));
  //    Write( _T ( "SENCFileLocation" ), _T("") );

  SetPath(_T ( "/Directories" ));
  Write(_T ( "InitChartDir" ), *pInit_Chart_Dir);
  Write(_T ( "GPXIODir" ), g_gpx_path);
  Write(_T ( "TCDataDir" ), g_TCData_Dir);
  Write(_T ( "BasemapDir" ), g_Platform->NormalizePath(gWorldMapLocation));
  Write(_T ( "pluginInstallDir" ), g_Platform->NormalizePath(g_winPluginDir));

  SetPath(_T ( "/Settings/NMEADataSource" ));
  wxString connectionconfigs;
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
    if (i > 0) connectionconfigs.Append(_T("|"));
    connectionconfigs.Append(TheConnectionParams()->Item(i)->Serialize());
  }
  Write(_T ( "DataConnections" ), connectionconfigs);

  //    Fonts

  //  Store the persistent Auxiliary Font descriptor Keys
  SetPath(_T ( "/Settings/AuxFontKeys" ));

  wxArrayString keyArray = FontMgr::Get().GetAuxKeyArray();
  for (unsigned int i = 0; i < keyArray.GetCount(); i++) {
    wxString key;
    key.Printf(_T("Key%i"), i);
    wxString keyval = keyArray[i];
    Write(key, keyval);
  }

  wxString font_path;
#ifdef __WXX11__
  font_path = (_T ( "/Settings/X11Fonts" ));
#endif

#ifdef __WXGTK__
  font_path = (_T ( "/Settings/GTKFonts" ));
#endif

#ifdef __WXMSW__
  font_path = (_T ( "/Settings/MSWFonts" ));
#endif

#ifdef __WXMAC__
  font_path = (_T ( "/Settings/MacFonts" ));
#endif

#ifdef __WXQT__
  font_path = (_T ( "/Settings/QTFonts" ));
#endif

  DeleteGroup(font_path);

  SetPath(font_path);

  int nFonts = FontMgr::Get().GetNumFonts();

  for (int i = 0; i < nFonts; i++) {
    wxString cfstring(FontMgr::Get().GetConfigString(i));
    wxString valstring = FontMgr::Get().GetFullConfigDesc(i);
    Write(cfstring, valstring);
  }

  //  Tide/Current Data Sources
  DeleteGroup(_T ( "/TideCurrentDataSources" ));
  SetPath(_T ( "/TideCurrentDataSources" ));
  unsigned int id = 0;
  for (auto val : TideCurrentDataSet) {
    wxString key;
    key.Printf(_T("tcds%d"), id);
    Write(key, wxString(val));
    ++id;
  }

  SetPath(_T ( "/Settings/Others" ));

  // Radar rings
  Write(_T ( "ShowRadarRings" ),
        (bool)(g_iNavAidRadarRingsNumberVisible > 0));  // 3.0.0 config support
  Write(_T ( "RadarRingsNumberVisible" ), g_iNavAidRadarRingsNumberVisible);
  Write(_T ( "RadarRingsStep" ), g_fNavAidRadarRingsStep);
  Write(_T ( "RadarRingsStepUnits" ), g_pNavAidRadarRingsStepUnits);
  Write(_T ( "RadarRingsColour" ),
        g_colourOwnshipRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX));
  Write(_T( "WaypointUseScaMin" ), g_bUseWptScaMin);
  Write(_T( "WaypointScaMinValue" ), g_iWpt_ScaMin);
  Write(_T( "WaypointUseScaMinOverrule" ), g_bOverruleScaMin);
  Write(_T("WaypointsShowName"), g_bShowWptName);

  // Waypoint Radar rings
  Write(_T ( "WaypointRangeRingsNumber" ), g_iWaypointRangeRingsNumber);
  Write(_T ( "WaypointRangeRingsStep" ), g_fWaypointRangeRingsStep);
  Write(_T ( "WaypointRangeRingsStepUnits" ), g_iWaypointRangeRingsStepUnits);
  Write(_T ( "WaypointRangeRingsColour" ),
        g_colourWaypointRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX));

  Write(_T ( "ConfirmObjectDeletion" ), g_bConfirmObjectDelete);

  // Waypoint dragging with mouse; toh, 2009.02.24
  Write(_T ( "WaypointPreventDragging" ), g_bWayPointPreventDragging);

  Write(_T ( "EnableZoomToCursor" ), g_bEnableZoomToCursor);

  Write(_T ( "TrackIntervalSeconds" ), g_TrackIntervalSeconds);
  Write(_T ( "TrackDeltaDistance" ), g_TrackDeltaDistance);
  Write(_T ( "TrackPrecision" ), g_nTrackPrecision);

  Write(_T ( "RouteLineWidth" ), g_route_line_width);
  Write(_T ( "TrackLineWidth" ), g_track_line_width);
  Write(_T ( "TrackLineColour" ),
        g_colourTrackLineColour.GetAsString(wxC2S_HTML_SYNTAX));
  Write(_T ( "DefaultWPIcon" ), g_default_wp_icon);
  Write(_T ( "DefaultRPIcon" ), g_default_routepoint_icon);

  DeleteGroup(_T ( "/MmsiProperties" ));
  SetPath(_T ( "/MmsiProperties" ));
  for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
    wxString p;
    p.Printf(_T("Props%d"), i);
    Write(p, g_MMSI_Props_Array[i]->Serialize());
  }

  SaveCanvasConfigs();

  Flush();
}

void MyConfig::UpdateNavObjOnly() {
  //   Create the NavObjectCollection, and save to specified file
  NavObjectCollection1 *pNavObjectSet = new NavObjectCollection1();

  pNavObjectSet->CreateAllGPXObjects();
  pNavObjectSet->SaveFile(m_sNavObjSetFile);

  delete pNavObjectSet;
}

void MyConfig::UpdateNavObj(bool bRecreate) {
  //   Create the NavObjectCollection, and save to specified file
  NavObjectCollection1 *pNavObjectSet = new NavObjectCollection1();

  pNavObjectSet->CreateAllGPXObjects();
  pNavObjectSet->SaveFile(m_sNavObjSetFile);

  delete pNavObjectSet;

  if (m_pNavObjectChangesSet->m_changes_file)
    fclose(m_pNavObjectChangesSet->m_changes_file);

  if (::wxFileExists(m_sNavObjSetChangesFile)) {
    wxLogNull logNo;  // avoid silly log error message.
    wxRemoveFile(m_sNavObjSetChangesFile);
  }

  if (bRecreate) {
    m_pNavObjectChangesSet->Init(m_sNavObjSetChangesFile);

    m_pNavObjectChangesSet->reset();
    m_pNavObjectChangesSet->load_file(m_sNavObjSetChangesFile.fn_str());
  }
}

static wxFileName exportFileName(wxWindow *parent,
                                 const wxString suggestedName) {
  wxFileName ret;
  wxString path;
  wxString validName{suggestedName};
  // replace common date characters invalid in filename
  // MS-DOS file systems have many more
  validName.Replace(_T("/"), _T("-"));
  validName.Replace(_T(":"), _T("_"));
  int response = g_Platform->DoFileSelectorDialog(
      parent, &path, _("Export GPX file"), g_gpx_path, validName, wxT("*.gpx"));

  if (response == wxID_OK) {
    wxFileName fn(path);
    g_gpx_path = fn.GetPath();
    fn.SetExt(_T("gpx"));

#ifndef __WXMAC__
    if (wxFileExists(fn.GetFullPath())) {
      int answer =
          OCPNMessageBox(NULL, _("Overwrite existing file?"), _T("Confirm"),
                         wxICON_QUESTION | wxYES_NO | wxCANCEL);
      if (answer != wxID_YES) return ret;
    }
#endif
    ret = fn;
  }
  return ret;
}

bool MyConfig::IsChangesFileDirty() {
  if (m_pNavObjectChangesSet) {
    return m_pNavObjectChangesSet->IsDirty();
  } else {
    return true;
  }
}

bool ExportGPXRoutes(wxWindow *parent, RouteList *pRoutes,
                     const wxString suggestedName) {
  wxFileName fn = exportFileName(parent, suggestedName);
  if (fn.IsOk()) {
    NavObjectCollection1 *pgpx = new NavObjectCollection1;
    pgpx->AddGPXRoutesList(pRoutes);

#ifdef __OCPN__ANDROID__
    wxString fns = androidGetCacheDir() + wxFileName::GetPathSeparator() +
                   fn.GetFullName();
    pgpx->SaveFile(fns);
    AndroidSecureCopyFile(fns, fn.GetFullPath());
#else
    pgpx->SaveFile(fn.GetFullPath());

#endif

    delete pgpx;

    return true;
  }
  return false;
}

bool ExportGPXTracks(wxWindow *parent, std::vector<Track*> *pTracks,
                     const wxString suggestedName) {
  wxFileName fn = exportFileName(parent, suggestedName);
  if (fn.IsOk()) {
    NavObjectCollection1 *pgpx = new NavObjectCollection1;
    pgpx->AddGPXTracksList(pTracks);
#ifdef __OCPN__ANDROID__
    wxString fns = androidGetCacheDir() + wxFileName::GetPathSeparator() +
                   fn.GetFullName();
    pgpx->SaveFile(fns);
    AndroidSecureCopyFile(fns, fn.GetFullPath());
#else
    pgpx->SaveFile(fn.GetFullPath());
#endif
    delete pgpx;

    return true;
  }
  return false;
}

bool ExportGPXWaypoints(wxWindow *parent, RoutePointList *pRoutePoints,
                        const wxString suggestedName) {
  wxFileName fn = exportFileName(parent, suggestedName);
  if (fn.IsOk()) {
    NavObjectCollection1 *pgpx = new NavObjectCollection1;
    pgpx->AddGPXPointsList(pRoutePoints);

#ifdef __OCPN__ANDROID__
    wxString fns = androidGetCacheDir() + wxFileName::GetPathSeparator() +
                   fn.GetFullName();
    pgpx->SaveFile(fns);
    AndroidSecureCopyFile(fns, fn.GetFullPath());
#else
    pgpx->SaveFile(fn.GetFullPath());
#endif

    delete pgpx;

    return true;
  }
  return false;
}

void ExportGPX(wxWindow *parent, bool bviz_only, bool blayer) {
  wxFileName fn = exportFileName(parent, _T("userobjects.gpx"));
  if (fn.IsOk()) {
    ::wxBeginBusyCursor();

    NavObjectCollection1 *pgpx = new NavObjectCollection1;

    wxGenericProgressDialog *pprog = nullptr;
    int count = pWayPointMan->GetWaypointList()->GetCount();
    int progStep = count / 32;
    if (count > 200) {
      pprog = new wxGenericProgressDialog(
          _("Export GPX file"), _T("0/0"), count, NULL,
          wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_ELAPSED_TIME |
              wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
      pprog->SetSize(400, wxDefaultCoord);
      pprog->Centre();
    }

    // WPTs
    int ic = 1;

    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
    RoutePoint *pr;
    while (node) {
      if (pprog && !(ic % progStep)) {
        wxString msg;
        msg.Printf(_T("%d/%d"), ic, count);
        pprog->Update(ic, msg);
      }
      ic++;

      pr = node->GetData();

      bool b_add = true;

      if (bviz_only && !pr->m_bIsVisible) b_add = false;

      if (pr->m_bIsInLayer && !blayer) b_add = false;
      if (b_add) {
        if (pr->IsShared() || !WptIsInRouteList(pr)) pgpx->AddGPXWaypoint(pr);
      }

      node = node->GetNext();
    }
    // RTEs and TRKs
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while (node1) {
      Route *pRoute = node1->GetData();

      bool b_add = true;

      if (bviz_only && !pRoute->IsVisible()) b_add = false;

      if (pRoute->m_bIsInLayer && !blayer) b_add = false;

      if (b_add) pgpx->AddGPXRoute(pRoute);

      node1 = node1->GetNext();
    }

    for (Track* pTrack : g_TrackList) {
      bool b_add = true;

      if (bviz_only && !pTrack->IsVisible()) b_add = false;

      if (pTrack->m_bIsInLayer && !blayer) b_add = false;

      if (b_add) pgpx->AddGPXTrack(pTrack);
    }

    // Android 5+ requires special handling to support native app file writes to
    // SDCard We need to use a two step copy process using a guaranteed
    // accessible location for the first step.
#ifdef __OCPN__ANDROID__
    wxString fns = androidGetCacheDir() + wxFileName::GetPathSeparator() +
                   fn.GetFullName();
    pgpx->SaveFile(fns);
    AndroidSecureCopyFile(fns, fn.GetFullPath());
#else
    pgpx->SaveFile(fn.GetFullPath());
#endif

    delete pgpx;
    ::wxEndBusyCursor();

    delete pprog;
  }
}

void UI_ImportGPX(wxWindow *parent, bool islayer, wxString dirpath,
                  bool isdirectory, bool isPersistent) {
  int response = wxID_CANCEL;
  wxArrayString file_array;
  Layer *l = NULL;

  if (!islayer || dirpath.IsSameAs(_T(""))) {
    //  Platform DoFileSelectorDialog method does not properly handle multiple
    //  selections So use native method if not Android, which means Android gets
    //  single selection only.
#ifndef __OCPN__ANDROID__
    wxFileDialog *popenDialog =
        new wxFileDialog(NULL, _("Import GPX file"), g_gpx_path, wxT(""),
                         wxT("GPX files (*.gpx)|*.gpx|All files (*.*)|*.*"),
                         wxFD_OPEN | wxFD_MULTIPLE);

    if (g_bresponsive && parent)
      popenDialog = g_Platform->AdjustFileDialogFont(parent, popenDialog);

    popenDialog->Centre();

#ifdef __WXOSX__
    if (parent) parent->HideWithEffect(wxSHOW_EFFECT_BLEND);
#endif

    response = popenDialog->ShowModal();

#ifdef __WXOSX__
    if (parent) parent->ShowWithEffect(wxSHOW_EFFECT_BLEND);
#endif

    if (response == wxID_OK) {
      popenDialog->GetPaths(file_array);

      //    Record the currently selected directory for later use
      if (file_array.GetCount()) {
        wxFileName fn(file_array[0]);
        g_gpx_path = fn.GetPath();
      }
    }
    delete popenDialog;
#else
    wxString path;
    response = g_Platform->DoFileSelectorDialog(
        NULL, &path, _("Import GPX file"), g_gpx_path, _T(""), wxT("*.gpx"));

    file_array.Add(path);
    wxFileName fn(path);
    g_gpx_path = fn.GetPath();

#endif

  } else {
    if (isdirectory) {
      if (wxDir::GetAllFiles(dirpath, &file_array, wxT("*.gpx")))
        response = wxID_OK;
    } else {
      file_array.Add(dirpath);
      response = wxID_OK;
    }
  }

  if (response == wxID_OK) {
    if (islayer) {
      l = new Layer();
      l->m_LayerID = ++g_LayerIdx;
      l->m_LayerFileName = file_array[0];
      if (file_array.GetCount() <= 1)
        wxFileName::SplitPath(file_array[0], NULL, NULL, &(l->m_LayerName),
                              NULL, NULL);
      else {
        if (dirpath.IsSameAs(_T("")))
          wxFileName::SplitPath(g_gpx_path, NULL, NULL, &(l->m_LayerName), NULL,
                                NULL);
        else
          wxFileName::SplitPath(dirpath, NULL, NULL, &(l->m_LayerName), NULL,
                                NULL);
      }

      bool bLayerViz = g_bShowLayers;
      if (g_VisibleLayers.Contains(l->m_LayerName)) bLayerViz = true;
      if (g_InvisibleLayers.Contains(l->m_LayerName)) bLayerViz = false;
      l->m_bIsVisibleOnChart = bLayerViz;

      // Default for new layers is "Names visible"
      l->m_bHasVisibleNames = wxCHK_CHECKED;

      wxString laymsg;
      laymsg.Printf(wxT("New layer %d: %s"), l->m_LayerID,
                    l->m_LayerName.c_str());
      wxLogMessage(laymsg);

      pLayerList->Insert(l);
    }

    for (unsigned int i = 0; i < file_array.GetCount(); i++) {
      wxString path = file_array[i];

      if (::wxFileExists(path)) {
        NavObjectCollection1 *pSet = new NavObjectCollection1;
        pSet->load_file(path.fn_str());

        if (islayer) {
          l->m_NoOfItems = pSet->LoadAllGPXObjectsAsLayer(
              l->m_LayerID, l->m_bIsVisibleOnChart, l->m_bHasVisibleNames);
          l->m_LayerType = isPersistent ? _("Persistent") : _("Temporary");

          if (isPersistent) {
            // If this is a persistent layer also copy the file to config file
            // dir /layers
            wxString destf, f, name, ext;
            f = l->m_LayerFileName;
            wxFileName::SplitPath(f, NULL, NULL, &name, &ext);
            destf = g_Platform->GetPrivateDataDir();
            appendOSDirSlash(&destf);
            destf.Append(_T("layers"));
            appendOSDirSlash(&destf);
            if (!wxDirExists(destf)) {
              if (!wxMkdir(destf, wxS_DIR_DEFAULT))
                wxLogMessage(_T("Error creating layer directory"));
            }

            destf << name << _T(".") << ext;
            wxString msg;
            if (wxCopyFile(f, destf, true))
              msg.Printf(_T("File: %s.%s also added to persistent layers"),
                         name, ext);
            else
              msg.Printf(_T("Failed adding %s.%s to persistent layers"), name,
                         ext);
            wxLogMessage(msg);
          }
        } else {
          int wpt_dups;
          pSet->LoadAllGPXObjects(
              !pSet->IsOpenCPN(),
              wpt_dups);  // Import with full visibility of names and objects
          if (wpt_dups > 0) {
            OCPNMessageBox(
                parent,
                wxString::Format(_T("%d ") + _("duplicate waypoints detected "
                                               "during import and ignored."),
                                 wpt_dups),
                _("OpenCPN Info"), wxICON_INFORMATION | wxOK, 10);
          }
        }
        delete pSet;
      }
    }
  }
}

//-------------------------------------------------------------------------
//           Static Routine Switch to Inland Ecdis Mode
//-------------------------------------------------------------------------
void SwitchInlandEcdisMode(bool Switch) {
  if (Switch) {
    wxLogMessage(_T("Switch InlandEcdis mode On"));
    LoadS57();
    // Overule some sewttings to comply with InlandEcdis
    // g_toolbarConfig = _T ( ".....XXXX.X...XX.XXXXXXXXXXXX" );
    g_iDistanceFormat = 2;  // 0 = "Nautical miles"), 1 = "Statute miles", 2 =
                            // "Kilometers", 3 = "Meters"
    g_iSpeedFormat = 2;     // 0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"
    if (ps52plib) ps52plib->SetDisplayCategory(STANDARD);
    g_bDrawAISSize = false;
    if (gFrame) gFrame->RequestNewToolbars(true);
  } else {
    wxLogMessage(_T("Switch InlandEcdis mode Off"));
    // reread the settings overruled by inlandEcdis
    if (pConfig) {
      pConfig->SetPath(_T ( "/Settings" ));
      pConfig->Read(_T ( "GlobalToolbarConfig" ), &g_toolbarConfig);
      pConfig->Read(_T ( "DistanceFormat" ), &g_iDistanceFormat);
      pConfig->Read(_T ( "SpeedFormat" ), &g_iSpeedFormat);
      pConfig->Read(_T ( "ShowDepthUnits" ), &g_bShowDepthUnits, 1);
      int read_int;
      pConfig->Read(_T ( "nDisplayCategory" ), &read_int,
                    (enum _DisCat)STANDARD);
      if (ps52plib) ps52plib->SetDisplayCategory((enum _DisCat)read_int);
      pConfig->SetPath(_T ( "/Settings/AIS" ));
      pConfig->Read(_T ( "bDrawAISSize" ), &g_bDrawAISSize);
      pConfig->Read(_T ( "bDrawAISRealtime" ), &g_bDrawAISRealtime);
    }
    if (gFrame) gFrame->RequestNewToolbars(true);
  }
}

//-------------------------------------------------------------------------
//
//          Static GPX Support Routines
//
//-------------------------------------------------------------------------
// This function formats the input date/time into a valid GPX ISO 8601
// time string specified in the UTC time zone.

wxString FormatGPXDateTime(wxDateTime dt) {
  //      return dt.Format(wxT("%Y-%m-%dT%TZ"), wxDateTime::GMT0);
  return dt.Format(wxT("%Y-%m-%dT%H:%M:%SZ"));
}

/**************************************************************************/
/*          LogMessageOnce                                                */
/**************************************************************************/

bool LogMessageOnce(const wxString &msg) {
  //    Search the array for a match

  for (unsigned int i = 0; i < pMessageOnceArray->GetCount(); i++) {
    if (msg.IsSameAs(pMessageOnceArray->Item(i))) return false;
  }

  // Not found, so add to the array
  pMessageOnceArray->Add(msg);

  //    And print it
  wxLogMessage(msg);
  return true;
}

/**************************************************************************/
/*          Some assorted utilities                                       */
/**************************************************************************/

/**************************************************************************/
/*          Converts the distance from the units selected by user to NMi  */
/**************************************************************************/
double fromUsrDistance(double usr_distance, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_iDistanceFormat;
  switch (unit) {
    case DISTANCE_NMI:  // Nautical miles
      ret = usr_distance;
      break;
    case DISTANCE_MI:  // Statute miles
      ret = usr_distance / 1.15078;
      break;
    case DISTANCE_KM:
      ret = usr_distance / 1.852;
      break;
    case DISTANCE_M:
      ret = usr_distance / 1852;
      break;
    case DISTANCE_FT:
      ret = usr_distance / 6076.12;
      break;
  }
  return ret;
}
/**************************************************************************/
/*          Converts the speed from the units selected by user to knots   */
/**************************************************************************/
double fromUsrSpeed(double usr_speed, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_iSpeedFormat;
  switch (unit) {
    case SPEED_KTS:  // kts
      ret = usr_speed;
      break;
    case SPEED_MPH:  // mph
      ret = usr_speed / 1.15078;
      break;
    case SPEED_KMH:  // km/h
      ret = usr_speed / 1.852;
      break;
    case SPEED_MS:  // m/s
      ret = usr_speed / 0.514444444;
      break;
  }
  return ret;
}
/**************************************************************************/
/*     Converts the wind speed from the units selected by user to knots   */
/**************************************************************************/
double fromUsrWindSpeed(double usr_wspeed, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_iWindSpeedFormat;
  switch (unit) {
    case WSPEED_KTS:  // kts
      ret = usr_wspeed;
      break;
    case WSPEED_MS:  // m/s
      ret = usr_wspeed / 0.514444444;
      break;
    case WSPEED_MPH:  // mph
      ret = usr_wspeed / 1.15078;
      break;
    case WSPEED_KMH:  // km/h
      ret = usr_wspeed / 1.852;
      break;
  }
  return ret;
}


/**************************************************************************/
/*  Converts the temperature from the units selected by user to Celsius   */
/**************************************************************************/
double fromUsrTemp(double usr_temp, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_iTempFormat;
  switch (unit) {
    case TEMPERATURE_C:  // C
      ret = usr_temp;
      break;
    case TEMPERATURE_F:  // F
      ret = (usr_temp - 32) * 5.0 / 9.0;
      break;
    case TEMPERATURE_K:  // K
      ret = usr_temp - 273.15;
      break;
  }
  return ret;
}

wxString formatAngle(double angle) {
  wxString out;
  if (g_bShowMag && g_bShowTrue) {
    out.Printf(wxT("%03.0f %cT (%.0f %cM)"), angle, 0x00B0,
               gFrame->GetMag(angle), 0x00B0);
  } else if (g_bShowTrue) {
    out.Printf(wxT("%03.0f %cT"), angle, 0x00B0);
  } else {
    out.Printf(wxT("%03.0f %cM"), gFrame->GetMag(angle), 0x00B0);
  }
  return out;
}

/* render a rectangle at a given color and transparency */
void AlphaBlending(ocpnDC &dc, int x, int y, int size_x, int size_y,
                   float radius, wxColour color, unsigned char transparency) {
  wxDC *pdc = dc.GetDC();
  if (pdc) {
    //    Get wxImage of area of interest
    wxBitmap obm(size_x, size_y);
    wxMemoryDC mdc1;
    mdc1.SelectObject(obm);
    mdc1.Blit(0, 0, size_x, size_y, pdc, x, y);
    mdc1.SelectObject(wxNullBitmap);
    wxImage oim = obm.ConvertToImage();

    //    Create destination image
    wxBitmap olbm(size_x, size_y);
    wxMemoryDC oldc(olbm);
    if (!oldc.IsOk()) return;

    oldc.SetBackground(*wxBLACK_BRUSH);
    oldc.SetBrush(*wxWHITE_BRUSH);
    oldc.Clear();

    if (radius > 0.0) oldc.DrawRoundedRectangle(0, 0, size_x, size_y, radius);

    wxImage dest = olbm.ConvertToImage();
    unsigned char *dest_data =
        (unsigned char *)malloc(size_x * size_y * 3 * sizeof(unsigned char));
    unsigned char *bg = oim.GetData();
    unsigned char *box = dest.GetData();
    unsigned char *d = dest_data;

    //  Sometimes, on Windows, the destination image is corrupt...
    if (NULL == box) {
      free(d);
      return;
    }
    float alpha = 1.0 - (float)transparency / 255.0;
    int sb = size_x * size_y;
    for (int i = 0; i < sb; i++) {
      float a = alpha;
      if (*box == 0 && radius > 0.0) a = 1.0;
      int r = ((*bg++) * a) + (1.0 - a) * color.Red();
      *d++ = r;
      box++;
      int g = ((*bg++) * a) + (1.0 - a) * color.Green();
      *d++ = g;
      box++;
      int b = ((*bg++) * a) + (1.0 - a) * color.Blue();
      *d++ = b;
      box++;
    }

    dest.SetData(dest_data);

    //    Convert destination to bitmap and draw it
    wxBitmap dbm(dest);
    dc.DrawBitmap(dbm, x, y, false);

    // on MSW, the dc Bounding box is not updated on DrawBitmap() method.
    // Do it explicitely here for all platforms.
    dc.CalcBoundingBox(x, y);
    dc.CalcBoundingBox(x + size_x, y + size_y);
  } else {
#ifdef ocpnUSE_GL
    glEnable(GL_BLEND);

    float radMod = wxMax(radius, 2.0);
    wxColour c(color.Red(), color.Green(), color.Blue(), transparency);
    dc.SetBrush(wxBrush(c));
    dc.SetPen(wxPen(c, 1));
    dc.DrawRoundedRectangle(x, y, size_x, size_y, radMod);

    glDisable(GL_BLEND);

#endif
  }
}

void GpxDocument::SeedRandom() {
  /* Fill with random. Miliseconds hopefully good enough for our usage, reading
   * /dev/random would be much better on linux and system guid function on
   * Windows as well */
  wxDateTime x = wxDateTime::UNow();
  long seed = x.GetMillisecond();
  seed *= x.GetTicks();
  srand(seed);
}

void DimeControl(wxWindow *ctrl) {
#ifdef __WXOSX__
  // On macOS 10.14+, we use the native colours in both light mode and dark
  // mode, and do not need to do anything else. Dark mode is toggled at the
  // application level in `SetAndApplyColorScheme`, and is also respected if it
  // is enabled system-wide.
  if (wxPlatformInfo::Get().CheckOSVersion(10, 14)) {
    return;
  }
#endif
#ifdef __WXQT__
  return;  // this is seriously broken on wxqt
#endif

  if(wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOW).Red() < 128) {
    // Dark system color themes usually do better job than we do on diming UI controls, do not fight with them
    return;
  }

  if (NULL == ctrl) return;

  wxColour col, window_back_color, gridline, uitext, udkrd, ctrl_back_color,
      text_color;
  col = GetGlobalColor(_T("DILG0"));                // Dialog Background white
  window_back_color = GetGlobalColor(_T("DILG1"));  // Dialog Background
  ctrl_back_color = GetGlobalColor(_T("DILG1"));    // Control Background
  text_color = GetGlobalColor(_T("DILG3"));         // Text
  uitext = GetGlobalColor(_T("UITX1"));  // Menu Text, derived from UINFF
  udkrd = GetGlobalColor(_T("UDKRD"));
  gridline = GetGlobalColor(_T("GREY2"));

  DimeControl(ctrl, col, window_back_color, ctrl_back_color, text_color, uitext,
              udkrd, gridline);
}

void DimeControl(wxWindow *ctrl, wxColour col, wxColour window_back_color,
                 wxColour ctrl_back_color, wxColour text_color, wxColour uitext,
                 wxColour udkrd, wxColour gridline) {
#ifdef __WXOSX__
  // On macOS 10.14+, we use the native colours in both light mode and dark
  // mode, and do not need to do anything else. Dark mode is toggled at the
  // application level in `SetAndApplyColorScheme`, and is also respected if it
  // is enabled system-wide.
  if (wxPlatformInfo::Get().CheckOSVersion(10, 14)) {
    return;
  }
#endif

  ColorScheme cs = global_color_scheme;

  // Are we in dusk or night mode? (Used below in several places.)
  bool darkMode =
      (cs == GLOBAL_COLOR_SCHEME_DUSK || cs == GLOBAL_COLOR_SCHEME_NIGHT);

  static int depth = 0;  // recursion count
  if (depth == 0) {      // only for the window root, not for every child
    // If the color scheme is DAY or RGB, use the default platform native colour
    // for backgrounds
    if (!darkMode) {
#ifdef _WIN32
      window_back_color = wxNullColour;
#else
      window_back_color = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
#endif
      col = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);
      uitext = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    }

    ctrl->SetBackgroundColour(window_back_color);
    if (darkMode) ctrl->SetForegroundColour(text_color);
  }

  wxWindowList kids = ctrl->GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode *node = kids.Item(i);
    wxWindow *win = node->GetData();

    if (win->IsKindOf(CLASSINFO(wxListBox)) ||
        win->IsKindOf(CLASSINFO(wxListCtrl)) ||
        win->IsKindOf(CLASSINFO(wxTextCtrl))
        || win->IsKindOf(CLASSINFO(wxTimePickerCtrl))
    ) {
      win->SetBackgroundColour(col);
    } else if (win->IsKindOf(CLASSINFO(wxStaticText)) ||
               win->IsKindOf(CLASSINFO(wxCheckBox)) ||
               win->IsKindOf(CLASSINFO(wxRadioButton))) {
      win->SetForegroundColour(uitext);
    }
#ifndef __WXOSX__
    // On macOS most controls can't be styled, and trying to do so only creates
    // weird coloured boxes around them. Fortunately, however, many of them
    // inherit a colour or tint from the background of their parent.

    else if (win->IsKindOf(CLASSINFO(wxBitmapComboBox)) ||
             win->IsKindOf(CLASSINFO(wxChoice)) ||
             win->IsKindOf(CLASSINFO(wxComboBox)) ||
             win->IsKindOf(CLASSINFO(wxTreeCtrl))) {
      win->SetBackgroundColour(col);
    }

    else if (win->IsKindOf(CLASSINFO(wxScrolledWindow)) ||
             win->IsKindOf(CLASSINFO(wxGenericDirCtrl)) ||
             win->IsKindOf(CLASSINFO(wxListbook)) ||
             win->IsKindOf(CLASSINFO(wxButton)) ||
             win->IsKindOf(CLASSINFO(wxToggleButton))) {
      win->SetBackgroundColour(window_back_color);
    }

    else if (win->IsKindOf(CLASSINFO(wxNotebook))) {
      ((wxNotebook *)win)->SetBackgroundColour(window_back_color);
      ((wxNotebook *)win)->SetForegroundColour(text_color);
    }
#endif

    else if (win->IsKindOf(CLASSINFO(wxHtmlWindow))) {
      if (cs != GLOBAL_COLOR_SCHEME_DAY && cs != GLOBAL_COLOR_SCHEME_RGB)
        ((wxPanel *)win)->SetBackgroundColour(ctrl_back_color);
      else
        ((wxPanel *)win)->SetBackgroundColour(wxNullColour);
    }

    else if (win->IsKindOf(CLASSINFO(wxGrid))) {
      ((wxGrid *)win)->SetDefaultCellBackgroundColour(window_back_color);
      ((wxGrid *)win)->SetDefaultCellTextColour(uitext);
      ((wxGrid *)win)->SetLabelBackgroundColour(col);
      ((wxGrid *)win)->SetLabelTextColour(uitext);
#if !wxCHECK_VERSION(3, 0, 0)
      ((wxGrid *)win)->SetDividerPen(wxPen(col));
#endif
      ((wxGrid *)win)->SetGridLineColour(gridline);
    }

    if (win->GetChildren().GetCount() > 0) {
      depth++;
      wxWindow *w = win;
      DimeControl(w, col, window_back_color, ctrl_back_color, text_color,
                  uitext, udkrd, gridline);
      depth--;
    }
  }
}
