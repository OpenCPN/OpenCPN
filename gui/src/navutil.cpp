/**************************************************************************
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

/**
 * \file
 *
 *  Implement nav_util.h -- Utility Functions
 */

#include "gl_headers.h"  // Must be included before anything using GL stuff

#include <wx/wxprec.h>

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <locale>
#include <list>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/bmpcbox.h>
#include <wx/dir.h>
#include "wx/dirctrl.h"
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

#include "o_sound/o_sound.h"

#include "model/ais_decoder.h"
#include "model/ais_state_vars.h"
#include "model/cmdline.h"
#include "model/config_vars.h"
#include "model/conn_params.h"
#include "model/cutil.h"
#include "model/geodesic.h"
#include "model/georef.h"
#include "model/gui_vars.h"
#include "model/idents.h"
#include "model/multiplexer.h"
#include "model/nav_object_database.h"
#include "model/navutil_base.h"
#include "model/navobj_db.h"
#include "model/own_ship.h"
#include "model/plugin_comm.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/track.h"

#include "ais.h"
#include "canvas_config.h"
#include "chartbase.h"
#include "chartdb.h"
#include "chcanv.h"
#include "cm93.h"
#include "config.h"
#include "config_mgr.h"
#include "dychart.h"
#include "font_mgr.h"
#include "layer.h"
#include "navutil.h"
#include "nmea0183.h"
#include "observable_globvar.h"
#include "ocpndc.h"
#include "ocpn_frame.h"
#include "ocpn_plugin.h"
#include "ocpn_platform.h"
#include "s52plib.h"
#include "s52utils.h"
#include "snd_config.h"
#include "styles.h"

#ifdef ocpnUSE_GL
#include "gl_chart_canvas.h"
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

MyConfig *pConfig;  ///< Global instance
static bool g_bLayersLoaded;

#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

namespace navutil {

wxArrayString *pMessageOnceArray;

void InitGlobals() { pMessageOnceArray = new wxArrayString(); }

void DeinitGlobals() {
  delete pMessageOnceArray;
  pMessageOnceArray = nullptr;
}

}  // namespace navutil

// Layer helper function

wxString GetLayerName(int id) {
  wxString name("unknown layer");
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
    : wxFileConfig("", "", LocalFileName, "", wxCONFIG_USE_LOCAL_FILE) {}

MyConfig::~MyConfig() {}

int MyConfig::LoadMyConfig() {
  int display_width, display_height;
  display_width = g_monitor_info[g_current_monitor].width;
  display_height = g_monitor_info[g_current_monitor].height;

  //  Set up any defaults not set elsewhere
  g_useMUI = true;
  g_TalkerIdText = "EC";
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
  g_cog_predictor_style = 105;
  g_cog_predictor_color = "rgb(255,0,0)";
  g_cog_predictor_endmarker = 1;
  g_ownship_HDTpredictor_style = 105;
  g_ownship_HDTpredictor_color = "rgb(255,0,0)";
  g_ownship_HDTpredictor_endmarker = 1;
  g_ownship_HDTpredictor_width = 0;
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
  g_datetime_format = "UTC";

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
  g_bNavAidRadarRingsShown = false;
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
  g_default_wp_icon = "triangle";
  g_default_routepoint_icon = "diamond";

  g_nAWDefault = 50;
  g_nAWMax = 1852;
  g_ObjQFileExt = "txt,rtf,png,html,gif,tif,jpg";

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
  display_width = g_monitor_info[g_current_monitor].width;
  display_height = g_monitor_info[g_current_monitor].height;

  //    Global options and settings
  SetPath("/Settings");
  Read("ActiveRoute", &g_active_route);
  Read("PersistActiveRoute", &g_persist_active_route);
  Read("AlwaysSendRmbRmc", &g_always_send_rmb_rmc);
  Read("LastAppliedTemplate", &g_lastAppliedTemplateGUID);
  Read("CompatOS", &g_compatOS);
  Read("CompatOsVersion", &g_compatOsVersion);

  // Some undocumented values
  Read("ConfigVersionString", &g_config_version_string);
  Read("CmdSoundString", &g_CmdSoundString, wxString(OCPN_SOUND_CMD));
  if (wxIsEmpty(g_CmdSoundString)) g_CmdSoundString = wxString(OCPN_SOUND_CMD);
  Read("NavMessageShown", &n_NavMessageShown);

  Read("AndroidVersionCode", &g_AndroidVersionCode);

  Read("UIexpert", &g_bUIexpert);

  Read("UIStyle", &g_uiStyle);

  Read("NCacheLimit", &g_nCacheLimit);

  Read("InlandEcdis",
       &g_bInlandEcdis);  // First read if in iENC mode as this will override
                          // some config settings

  Read("SpaceDropMark", &g_bSpaceDropMark);

  int mem_limit = 0;
  Read("MEMCacheLimit", &mem_limit);
  if (mem_limit > 0)
    g_memCacheLimit = mem_limit * 1024;  // convert from MBytes to kBytes

  Read("UseModernUI5", &g_useMUI);

  Read("NCPUCount", &g_nCPUCount);

  Read("DebugGDAL", &g_bGDAL_Debug);
  Read("DebugNMEA", &g_nNMEADebug);
  Read("AnchorWatchDefault", &g_nAWDefault);
  Read("AnchorWatchMax", &g_nAWMax);
  Read("GPSDogTimeout", &gps_watchdog_timeout_ticks);
  Read("DebugCM93", &g_bDebugCM93);
  Read("DebugS57",
       &g_bDebugS57);  // Show LUP and Feature info in object query
  Read("DebugBSBImg", &g_BSBImgDebug);
  Read("DebugGPSD", &g_bDebugGPSD);
  Read("MaxZoomScale", &g_maxzoomin);
  g_maxzoomin = wxMax(g_maxzoomin, 50);

  Read("DefaultFontSize", &g_default_font_size);
  Read("DefaultFontFacename", &g_default_font_facename);

  Read("UseGreenShipIcon", &g_bUseGreenShip);

  Read("AutoHideToolbar", &g_bAutoHideToolbar);
  Read("AutoHideToolbarSecs", &g_nAutoHideToolbar);

  Read("UseSimplifiedScalebar", &g_bsimplifiedScalebar);
  Read("ShowTide", &g_bShowTide);
  Read("ShowCurrent", &g_bShowCurrent);

  wxString size_mm;
  Read("DisplaySizeMM", &size_mm);

  Read("SelectionRadiusMM", &g_selection_radius_mm);
  Read("SelectionRadiusTouchMM", &g_selection_radius_touch_mm);

  if (!bAsTemplate) {
    g_config_display_size_mm.clear();
    wxStringTokenizer tokenizer(size_mm, ",");
    while (tokenizer.HasMoreTokens()) {
      wxString token = tokenizer.GetNextToken();
      int size;
      try {
        size = std::stoi(token.ToStdString());
      } catch (std::invalid_argument &e) {
        size = 0;
      }
      if (size > 100 && size < 2000) {
        g_config_display_size_mm.push_back(size);
      } else {
        g_config_display_size_mm.push_back(0);
      }
    }
    Read("DisplaySizeManual", &g_config_display_size_manual);
  }

  Read("GUIScaleFactor", &g_GUIScaleFactor);

  Read("ChartObjectScaleFactor", &g_ChartScaleFactor);
  Read("ShipScaleFactor", &g_ShipScaleFactor);
  Read("ENCSoundingScaleFactor", &g_ENCSoundingScaleFactor);
  Read("ENCTextScaleFactor", &g_ENCTextScaleFactor);
  Read("ObjQueryAppendFilesExt", &g_ObjQFileExt);

  // Plugin catalog handler persistent variables.
  Read("CatalogCustomURL", &g_catalog_custom_url);
  Read("CatalogChannel", &g_catalog_channel);

  Read("NetmaskBits", &g_netmask_bits);

  //  NMEA connection options.
  if (!bAsTemplate) {
    Read("FilterNMEA_Avg", &g_bfilter_cogsog);
    Read("FilterNMEA_Sec", &g_COGFilterSec);
    Read("GPSIdent", &g_GPS_Ident);
    Read("UseGarminHostUpload", &g_bGarminHostUpload);
    Read("UseNMEA_GLL", &g_bUseGLL);
    Read("UseMagAPB", &g_bMagneticAPB);
    Read("TrackContinuous", &g_btrackContinuous, false);
    Read("FilterTrackDropLargeJump", &g_trackFilterMax, 1000);
  }

  Read("ShowTrue", &g_bShowTrue);
  Read("ShowMag", &g_bShowMag);

  wxString umv;
  Read("UserMagVariation", &umv);
  if (umv.Len()) umv.ToDouble(&g_UserVar);

  Read("ScreenBrightness", &g_nbrightness);

  Read("MemFootprintTargetMB", &g_MemFootMB);

  Read("WindowsComPortMax", &g_nCOMPortCheck);

  Read("ChartQuilting", &g_bQuiltEnable);
  Read("ChartQuiltingInitial", &g_bQuiltStart);

  Read("CourseUpMode", &g_bCourseUp);
  Read("COGUPAvgSeconds", &g_COGAvgSec);
  Read("LookAheadMode", &g_bLookAhead);
  Read("SkewToNorthUp", &g_bskew_comp);
  Read("TenHzUpdate", &g_btenhertz, 0);
  Read("DeclutterAnchorage", &g_declutter_anchorage, 0);

  Read("NMEAAPBPrecision", &g_NMEAAPBPrecision);

  Read("TalkerIdText", &g_TalkerIdText);
  Read("MaxWaypointNameLength", &g_maxWPNameLength);
  Read("MbtilesMaxLayers", &g_mbtilesMaxLayers);

  Read("ShowTrackPointTime", &g_bShowTrackPointTime, true);
  /* opengl options */
#ifdef ocpnUSE_GL
  if (!bAsTemplate) {
    Read("OpenGLExpert", &g_bGLexpert, false);
    Read("UseAcceleratedPanning", &g_GLOptions.m_bUseAcceleratedPanning, true);
    Read("GPUTextureCompression", &g_GLOptions.m_bTextureCompression);
    Read("GPUTextureCompressionCaching",
         &g_GLOptions.m_bTextureCompressionCaching);
    Read("PolygonSmoothing", &g_GLOptions.m_GLPolygonSmoothing);
    Read("LineSmoothing", &g_GLOptions.m_GLLineSmoothing);
    Read("GPUTextureDimension", &g_GLOptions.m_iTextureDimension);
    Read("GPUTextureMemSize", &g_GLOptions.m_iTextureMemorySize);
    Read("DebugOpenGL", &g_bDebugOGL);
    Read("OpenGL", &g_bopengl);
    Read("SoftwareGL", &g_bSoftwareGL);
  }
#endif

  Read("SmoothPanZoom", &g_bsmoothpanzoom);

  Read("ToolbarX", &g_maintoolbar_x);
  Read("ToolbarY", &g_maintoolbar_y);
  Read("ToolbarOrient", &g_maintoolbar_orient);
  Read("GlobalToolbarConfig", &g_toolbarConfig);

  Read("iENCToolbarX", &g_iENCToolbarPosX);
  Read("iENCToolbarY", &g_iENCToolbarPosY);

  Read("AnchorWatch1GUID", &g_AW1GUID);
  Read("AnchorWatch2GUID", &g_AW2GUID);

  Read("InitialStackIndex", &g_restore_stackindex);
  Read("InitialdBIndex", &g_restore_dbindex);

  Read("ChartNotRenderScaleFactor", &g_ChartNotRenderScaleFactor);

  Read("MobileTouch", &g_btouch);

//  "Responsive graphics" option deprecated in O58+
//  Read("ResponsiveGraphics", &g_bresponsive);
#ifdef __ANDROID__
  g_bresponsive = true;
#else
  g_bresponsive = false;
#endif

  Read("EnableRolloverBlock", &g_bRollover);

  Read("ZoomDetailFactor", &g_chart_zoom_modifier_raster);
  Read("ZoomDetailFactorVector", &g_chart_zoom_modifier_vector);
  Read("PlusMinusZoomFactor", &g_plus_minus_zoom_factor, 2.0);
  Read("MouseZoomSensitivity", &g_mouse_zoom_sensitivity, 1.3);
  g_mouse_zoom_sensitivity_ui =
      MouseZoom::config_to_ui(g_mouse_zoom_sensitivity);
  Read("CM93DetailFactor", &g_cm93_zoom_factor);

  Read("CM93DetailZoomPosX", &g_detailslider_dialog_x);
  Read("CM93DetailZoomPosY", &g_detailslider_dialog_y);
  Read("ShowCM93DetailSlider", &g_bShowDetailSlider);

  Read("SENC_LOD_Pixels", &g_SENC_LOD_pixels);

  Read("SkewCompUpdatePeriod", &g_SkewCompUpdatePeriod);

  Read("SetSystemTime", &s_bSetSystemTime);
  Read("ShowStatusBar", &g_bShowStatusBar);
#ifndef __WXOSX__
  Read("ShowMenuBar", &g_bShowMenuBar);
#endif
  Read("Fullscreen", &g_bFullscreen);
  Read("ShowCompassWindow", &g_bShowCompassWin);
  Read("ShowGrid", &g_bDisplayGrid);
  Read("PlayShipsBells", &g_bPlayShipsBells);
  Read("SoundDeviceIndex", &g_iSoundDeviceIndex);
  Read("FullscreenToolbar", &g_bFullscreenToolbar);
  Read("PermanentMOBIcon", &g_bPermanentMOBIcon);
  Read("ShowLayers", &g_bShowLayers);
  Read("ShowDepthUnits", &g_bShowDepthUnits);
  Read("AutoAnchorDrop", &g_bAutoAnchorMark);
  Read("ShowChartOutlines", &g_bShowOutlines);
  Read("ShowActiveRouteHighway", &g_bShowActiveRouteHighway);
  Read("ShowActiveRouteTotal", &g_bShowRouteTotal);
  Read("MostRecentGPSUploadConnection", &g_uploadConnection);
  Read("ShowChartBar", &g_bShowChartBar);
  Read("SDMMFormat",
       &g_iSDMMFormat);  // 0 = "Degrees, Decimal minutes"), 1 = "Decimal
                         // degrees", 2 = "Degrees,Minutes, Seconds"

  Read("DistanceFormat",
       &g_iDistanceFormat);  // 0 = "Nautical miles"), 1 = "Statute miles", 2 =
                             // "Kilometers", 3 = "Meters"
  Read("SpeedFormat",
       &g_iSpeedFormat);  // 0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"
  Read("WindSpeedFormat",
       &g_iWindSpeedFormat);  // 0 = "knots"), 1 = "m/s", 2 = "Mph", 3 = "km/h"
  Read("TemperatureFormat", &g_iTempFormat);  // 0 = C, 1 = F, 2 = K
  Read("HeightFormat", &g_iHeightFormat);     // 0 = M, 1 = FT

  // LIVE ETA OPTION
  Read("LiveETA", &g_bShowLiveETA);
  Read("DefaultBoatSpeed", &g_defaultBoatSpeed);

  Read("OwnshipCOGPredictorMinutes", &g_ownship_predictor_minutes);
  Read("OwnshipCOGPredictorStyle", &g_cog_predictor_style);
  Read("OwnshipCOGPredictorColor", &g_cog_predictor_color);
  Read("OwnshipCOGPredictorEndmarker", &g_cog_predictor_endmarker);
  Read("OwnshipCOGPredictorWidth", &g_cog_predictor_width);
  Read("OwnshipHDTPredictorStyle", &g_ownship_HDTpredictor_style);
  Read("OwnshipHDTPredictorColor", &g_ownship_HDTpredictor_color);
  Read("OwnshipHDTPredictorEndmarker", &g_ownship_HDTpredictor_endmarker);
  Read("OwnshipHDTPredictorWidth", &g_ownship_HDTpredictor_width);
  Read("OwnshipHDTPredictorMiles", &g_ownship_HDTpredictor_miles);
  int mmsi;
  Read("OwnShipMMSINumber", &mmsi);
  g_OwnShipmmsi = mmsi >= 0 ? static_cast<unsigned>(mmsi) : 0;
  Read("OwnShipIconType", &g_OwnShipIconType);
  Read("OwnShipLength", &g_n_ownship_length_meters);
  Read("OwnShipWidth", &g_n_ownship_beam_meters);
  Read("OwnShipGPSOffsetX", &g_n_gps_antenna_offset_x);
  Read("OwnShipGPSOffsetY", &g_n_gps_antenna_offset_y);
  Read("OwnShipMinSize", &g_n_ownship_min_mm);
  Read("OwnShipSogCogCalc", &g_own_ship_sog_cog_calc);
  Read("OwnShipSogCogCalcDampSec", &g_own_ship_sog_cog_calc_damp_sec);
  Read("ShowDirectRouteLine", &g_bShowShipToActive);
  Read("DirectRouteLineStyle", &g_shipToActiveStyle);
  Read("DirectRouteLineColor", &g_shipToActiveColor);

  wxString racr;
  Read("RouteArrivalCircleRadius", &racr);
  if (racr.Len()) racr.ToDouble(&g_n_arrival_circle_radius);

  Read("FullScreenQuilt", &g_bFullScreenQuilt);

  Read("StartWithTrackActive", &g_bTrackCarryOver);
  Read("AutomaticDailyTracks", &g_bTrackDaily);
  Read("TrackRotateAt", &g_track_rotate_time);
  Read("TrackRotateTimeType", &g_track_rotate_time_type);
  Read("HighlightTracks", &g_bHighliteTracks);

  Read("DateTimeFormat", &g_datetime_format);

  wxString stps;
  Read("PlanSpeed", &stps);
  if (!stps.IsEmpty()) stps.ToDouble(&g_PlanSpeed);

  Read("VisibleLayers", &g_VisibleLayers);
  Read("InvisibleLayers", &g_InvisibleLayers);
  Read("VisNameInLayers", &g_VisiNameinLayers);
  Read("InvisNameInLayers", &g_InVisiNameinLayers);

  Read("PreserveScaleOnX", &g_bPreserveScaleOnX);

  Read("ShowMUIZoomButtons", &g_bShowMuiZoomButtons);

  Read("Locale", &g_locale);
  Read("LocaleOverride", &g_localeOverride);

  // We allow 0-99 backups ov navobj.xml
  Read("KeepNavobjBackups", &g_navobjbackups);

  // Boolean to cater for legacy Input COM Port filer behaviour, i.e. show msg
  // filtered but put msg on bus.
  Read("LegacyInputCOMPortFilterBehaviour", &g_b_legacy_input_filter_behaviour);

  // Boolean to cater for sailing when not approaching waypoint
  Read("AdvanceRouteWaypointOnArrivalOnly",
       &g_bAdvanceRouteWaypointOnArrivalOnly);
  Read("EnableRootMenuDebug", &g_enable_root_menu_debug);

  Read("EnableRotateKeys", &g_benable_rotate);
  Read("EmailCrashReport", &g_bEmailCrashReport);

  g_benableAISNameCache = true;
  Read("EnableAISNameCache", &g_benableAISNameCache);

  Read("EnableUDPNullHeader", &g_benableUDPNullHeader);

  SetPath("/Settings/GlobalState");

  Read("FrameWinX", &g_nframewin_x);
  Read("FrameWinY", &g_nframewin_y);
  Read("FrameWinPosX", &g_nframewin_posx);
  Read("FrameWinPosY", &g_nframewin_posy);
  Read("FrameMax", &g_bframemax);

  Read("ClientPosX", &g_lastClientRectx);
  Read("ClientPosY", &g_lastClientRecty);
  Read("ClientSzX", &g_lastClientRectw);
  Read("ClientSzY", &g_lastClientRecth);

  Read("RoutePropSizeX", &g_route_prop_sx);
  Read("RoutePropSizeY", &g_route_prop_sy);
  Read("RoutePropPosX", &g_route_prop_x);
  Read("RoutePropPosY", &g_route_prop_y);

  Read("AllowArbitrarySystemPlugins", &g_allow_arb_system_plugin);

  read_int = -1;
  Read("S52_DEPTH_UNIT_SHOW", &read_int);  // default is metres
  if (read_int >= 0) {
    read_int = wxMax(read_int, 0);  // qualify value
    read_int = wxMin(read_int, 2);
    g_nDepthUnitDisplay = read_int;
  }

  // Sounds
  SetPath("/Settings/Audio");

  // Set reasonable defaults
  wxString sound_dir = g_Platform->GetSharedDataDir();
  sound_dir.Append("sounds");
  sound_dir.Append(wxFileName::GetPathSeparator());

  g_AIS_sound_file = sound_dir + "beep_ssl.wav";
  g_DSC_sound_file = sound_dir + "phonering1.wav";
  g_SART_sound_file = sound_dir + "beep3.wav";
  g_anchorwatch_sound_file = sound_dir + "beep1.wav";

  Read("AISAlertSoundFile", &g_AIS_sound_file);
  Read("DSCAlertSoundFile", &g_DSC_sound_file);
  Read("SARTAlertSoundFile", &g_SART_sound_file);
  Read("AnchorAlarmSoundFile", &g_anchorwatch_sound_file);

  Read("bAIS_GCPA_AlertAudio", &g_bAIS_GCPA_Alert_Audio);
  Read("bAIS_SART_AlertAudio", &g_bAIS_SART_Alert_Audio);
  Read("bAIS_DSC_AlertAudio", &g_bAIS_DSC_Alert_Audio);
  Read("bAnchorAlertAudio", &g_bAnchor_Alert_Audio);

  //    AIS
  wxString s;
  SetPath("/Settings/AIS");

  g_bUseOnlyConfirmedAISName = false;
  Read("UseOnlyConfirmedAISName", &g_bUseOnlyConfirmedAISName);

  Read("bNoCPAMax", &g_bCPAMax);

  Read("NoCPAMaxNMi", &s);
  s.ToDouble(&g_CPAMax_NM);

  Read("bCPAWarn", &g_bCPAWarn);

  Read("CPAWarnNMi", &s);
  s.ToDouble(&g_CPAWarn_NM);

  Read("bTCPAMax", &g_bTCPA_Max);

  Read("TCPAMaxMinutes", &s);
  s.ToDouble(&g_TCPA_Max);

  Read("bMarkLostTargets", &g_bMarkLost);

  Read("MarkLost_Minutes", &s);
  s.ToDouble(&g_MarkLost_Mins);

  Read("bRemoveLostTargets", &g_bRemoveLost);

  Read("RemoveLost_Minutes", &s);
  s.ToDouble(&g_RemoveLost_Mins);

  Read("bShowCOGArrows", &g_bShowCOG);

  Read("bSyncCogPredictors", &g_bSyncCogPredictors);

  Read("CogArrowMinutes", &s);
  s.ToDouble(&g_ShowCOG_Mins);

  Read("bShowTargetTracks", &g_bAISShowTracks);

  if (Read("TargetTracksLimit", &s)) {
    s.ToDouble(&g_AISShowTracks_Limit);
    g_AISShowTracks_Limit = wxMax(300.0, g_AISShowTracks_Limit);
  }
  if (Read("TargetTracksMinutes", &s)) {
    s.ToDouble(&g_AISShowTracks_Mins);
    g_AISShowTracks_Mins = wxMax(1.0, g_AISShowTracks_Mins);
    g_AISShowTracks_Mins = wxMin(g_AISShowTracks_Limit, g_AISShowTracks_Mins);
  }

  Read("bHideMooredTargets", &g_bHideMoored);
  if (Read("MooredTargetMaxSpeedKnots", &s)) s.ToDouble(&g_ShowMoored_Kts);

  g_SOGminCOG_kts = 0.2;
  if (Read("SOGMinimumForCOGDisplay", &s)) s.ToDouble(&g_SOGminCOG_kts);

  Read("bShowScaledTargets", &g_bAllowShowScaled);
  Read("AISScaledNumber", &g_ShowScaled_Num);
  Read("AISScaledNumberWeightSOG", &g_ScaledNumWeightSOG);
  Read("AISScaledNumberWeightCPA", &g_ScaledNumWeightCPA);
  Read("AISScaledNumberWeightTCPA", &g_ScaledNumWeightTCPA);
  Read("AISScaledNumberWeightRange", &g_ScaledNumWeightRange);
  Read("AISScaledNumberWeightSizeOfTarget", &g_ScaledNumWeightSizeOfT);
  Read("AISScaledSizeMinimal", &g_ScaledSizeMinimal);
  Read("AISShowScaled", &g_bShowScaled);

  Read("bShowAreaNotices", &g_bShowAreaNotices);
  Read("bDrawAISSize", &g_bDrawAISSize);
  Read("bDrawAISRealtime", &g_bDrawAISRealtime);
  Read("bShowAISName", &g_bShowAISName);
  Read("AISRealtimeMinSpeedKnots", &g_AIS_RealtPred_Kts, 0.7);
  Read("bAISAlertDialog", &g_bAIS_CPA_Alert);
  Read("ShowAISTargetNameScale", &g_Show_Target_Name_Scale);
  Read("bWplIsAprsPositionReport", &g_bWplUsePosition);
  Read("WplSelAction", &g_WplAction);
  Read("AISCOGPredictorWidth", &g_ais_cog_predictor_width);

  Read("bAISAlertAudio", &g_bAIS_CPA_Alert_Audio);
  Read("AISAlertAudioFile", &g_sAIS_Alert_Sound_File);
  Read("bAISAlertSuppressMoored", &g_bAIS_CPA_Alert_Suppress_Moored);

  Read("bAISAlertAckTimeout", &g_bAIS_ACK_Timeout);
  if (Read("AlertAckTimeoutMinutes", &s)) s.ToDouble(&g_AckTimeout_Mins);

  Read("AlertDialogSizeX", &g_ais_alert_dialog_sx);
  Read("AlertDialogSizeY", &g_ais_alert_dialog_sy);
  Read("AlertDialogPosX", &g_ais_alert_dialog_x);
  Read("AlertDialogPosY", &g_ais_alert_dialog_y);
  Read("QueryDialogPosX", &g_ais_query_dialog_x);
  Read("QueryDialogPosY", &g_ais_query_dialog_y);

  Read("AISTargetListPerspective", &g_AisTargetList_perspective);
  Read("AISTargetListRange", &g_AisTargetList_range);
  Read("AISTargetListSortColumn", &g_AisTargetList_sortColumn);
  Read("bAISTargetListSortReverse", &g_bAisTargetList_sortReverse);
  Read("AISTargetListColumnSpec", &g_AisTargetList_column_spec);
  Read("AISTargetListColumnOrder", &g_AisTargetList_column_order);

  Read("bAISRolloverShowClass", &g_bAISRolloverShowClass);
  Read("bAISRolloverShowCOG", &g_bAISRolloverShowCOG);
  Read("bAISRolloverShowCPA", &g_bAISRolloverShowCPA);

  Read("S57QueryDialogSizeX", &g_S57_dialog_sx);
  Read("S57QueryDialogSizeY", &g_S57_dialog_sy);
  Read("S57QueryExtraDialogSizeX", &g_S57_extradialog_sx);
  Read("S57QueryExtraDialogSizeY", &g_S57_extradialog_sy);

  wxString strpres("PresentationLibraryData");
  wxString valpres;
  SetPath("/Directories");
  Read(strpres, &valpres);  // Get the File name
  if (!valpres.IsEmpty()) g_UserPresLibData = valpres;

  wxString strs("SENCFileLocation");
  SetPath("/Directories");
  wxString vals;
  Read(strs, &vals);  // Get the Directory name
  if (!vals.IsEmpty()) g_SENCPrefix = vals;

  SetPath("/Directories");
  wxString vald;
  Read("InitChartDir", &vald);  // Get the Directory name

  wxString dirnamed(vald);
  if (!dirnamed.IsEmpty()) {
    if (pInit_Chart_Dir->IsEmpty())  // on second pass, don't overwrite
    {
      pInit_Chart_Dir->Clear();
      pInit_Chart_Dir->Append(vald);
    }
  }

  Read("GPXIODir", &g_gpx_path);     // Get the Directory name
  Read("TCDataDir", &g_TCData_Dir);  // Get the Directory name
  Read("BasemapDir", &gWorldMapLocation);
  Read("BaseShapefileDir", &gWorldShapefileLocation);
  Read("pluginInstallDir", &g_winPluginDir);
  wxLogMessage("winPluginDir, read from ini file: %s",
               g_winPluginDir.mb_str().data());

  SetPath("/Settings/GlobalState");

  if (Read("nColorScheme", &read_int))
    global_color_scheme = (ColorScheme)read_int;

  if (!bAsTemplate) {
    SetPath("/Settings/NMEADataSource");

    TheConnectionParams().clear();
    wxString connectionconfigs;
    Read("DataConnections", &connectionconfigs);
    if (!connectionconfigs.IsEmpty()) {
      wxArrayString confs = wxStringTokenize(connectionconfigs, "|");
      for (size_t i = 0; i < confs.Count(); i++) {
        ConnectionParams *prm = new ConnectionParams(confs[i]);
        if (!prm->Valid) {
          wxLogMessage("Skipped invalid DataStream config");
          delete prm;
          continue;
        }
        TheConnectionParams().push_back(prm);
      }
    }
  }

  SetPath("/Settings/GlobalState");
  wxString st;

  double st_lat, st_lon;
  if (Read("VPLatLon", &st)) {
    sscanf(st.mb_str(wxConvUTF8), "%lf,%lf", &st_lat, &st_lon);

    //    Sanity check the lat/lon...both have to be reasonable.
    if (fabs(st_lon) < 360.) {
      while (st_lon < -180.) st_lon += 360.;

      while (st_lon > 180.) st_lon -= 360.;

      vLon = st_lon;
    }

    if (fabs(st_lat) < 90.0) vLat = st_lat;

    s.Printf("Setting Viewpoint Lat/Lon %g, %g", vLat, vLon);
    wxLogMessage(s);
  }

  double st_view_scale, st_rotation;
  if (Read(wxString("VPScale"), &st)) {
    sscanf(st.mb_str(wxConvUTF8), "%lf", &st_view_scale);
    //    Sanity check the scale
    st_view_scale = fmax(st_view_scale, .001 / 32);
    st_view_scale = fmin(st_view_scale, 4);
  }

  if (Read(wxString("VPRotation"), &st)) {
    sscanf(st.mb_str(wxConvUTF8), "%lf", &st_rotation);
    //    Sanity check the rotation
    st_rotation = fmin(st_rotation, 360);
    st_rotation = fmax(st_rotation, 0);
  }

  wxString sll;
  double lat, lon;
  if (Read("OwnShipLatLon", &sll)) {
    sscanf(sll.mb_str(wxConvUTF8), "%lf,%lf", &lat, &lon);

    //    Sanity check the lat/lon...both have to be reasonable.
    if (fabs(lon) < 360.) {
      while (lon < -180.) lon += 360.;

      while (lon > 180.) lon -= 360.;

      gLon = lon;
    }

    if (fabs(lat) < 90.0) gLat = lat;

    s.Printf("Setting Ownship Lat/Lon %g, %g", gLat, gLon);
    wxLogMessage(s);
  }

  //    Fonts

  //  Load the persistent Auxiliary Font descriptor Keys
  SetPath("/Settings/AuxFontKeys");

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
  SetPath("/Settings/X11Fonts");
#endif

#ifdef __WXGTK__
  SetPath("/Settings/GTKFonts");
#endif

#ifdef __WXMSW__
  SetPath("/Settings/MSWFonts");
#endif

#ifdef __WXMAC__
  SetPath("/Settings/MacFonts");
#endif

#ifdef __WXQT__
  SetPath("/Settings/QTFonts");
#endif

  wxString str;
  long dummy;
  wxString pval;
  wxArrayString deleteList;

  bool bCont = GetFirstEntry(str, dummy);
  while (bCont) {
    pval = Read(str);

    if (str.StartsWith("Font")) {
      // Convert pre 3.1 setting. Can't delete old entries from inside the
      // GetNextEntry() loop, so we need to save those and delete outside.
      deleteList.Add(str);
      wxString oldKey = pval.BeforeFirst(_T(':'));
      str = FontMgr::GetFontConfigKey(oldKey);
    }

    if (pval.IsEmpty() || pval.StartsWith(":")) {
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
  SetPath("/TideCurrentDataSources");
  if (GetNumberOfEntries()) {
    TideCurrentDataSet.clear();
    wxString str, val;
    long dummy;
    bool bCont = GetFirstEntry(str, dummy);
    while (bCont) {
      Read(str, &val);  // Get a file name and add it to the list just in case
                        // it is not repeated
      // We have seen duplication of dataset entries in
      // https://github.com/OpenCPN/OpenCPN/issues/3042, this effectively gets
      // rid of them.
      if (std::find(TideCurrentDataSet.begin(), TideCurrentDataSet.end(),
                    val.ToStdString()) == TideCurrentDataSet.end()) {
        TideCurrentDataSet.push_back(val.ToStdString());
      }
      bCont = GetNextEntry(str, dummy);
    }
  }

  //    Groups
  LoadConfigGroups(g_pGroupArray);

  //     //    Multicanvas Settings
  //     LoadCanvasConfigs();

  SetPath("/Settings/Others");

  // Radar rings
  Read("RadarRingsNumberVisible", &val);
  if (val.Length() > 0) g_iNavAidRadarRingsNumberVisible = atoi(val.mb_str());
  g_bNavAidRadarRingsShown = g_iNavAidRadarRingsNumberVisible > 0;

  Read("RadarRingsStep", &val);
  if (val.Length() > 0) g_fNavAidRadarRingsStep = atof(val.mb_str());

  Read("RadarRingsStepUnits", &g_pNavAidRadarRingsStepUnits);

  wxString l_wxsOwnshipRangeRingsColour;
  Read("RadarRingsColour", &l_wxsOwnshipRangeRingsColour);
  if (l_wxsOwnshipRangeRingsColour.Length())
    g_colourOwnshipRangeRingsColour.Set(l_wxsOwnshipRangeRingsColour);

  // Waypoint Radar rings
  Read("WaypointRangeRingsNumber", &val);
  if (val.Length() > 0) g_iWaypointRangeRingsNumber = atoi(val.mb_str());

  Read("WaypointRangeRingsStep", &val);
  if (val.Length() > 0) g_fWaypointRangeRingsStep = atof(val.mb_str());

  Read("WaypointRangeRingsStepUnits", &g_iWaypointRangeRingsStepUnits);

  wxString l_wxsWaypointRangeRingsColour;
  Read("WaypointRangeRingsColour", &l_wxsWaypointRangeRingsColour);
  g_colourWaypointRangeRingsColour.Set(l_wxsWaypointRangeRingsColour);

  if (!Read("WaypointUseScaMin", &g_bUseWptScaMin)) g_bUseWptScaMin = false;
  if (!Read("WaypointScaMinValue", &g_iWpt_ScaMin)) g_iWpt_ScaMin = 2147483646;
  if (!Read("WaypointUseScaMinOverrule", &g_bOverruleScaMin))
    g_bOverruleScaMin = false;
  if (!Read("WaypointsShowName", &g_bShowWptName)) g_bShowWptName = true;
  if (!Read("UserIconsFirst", &g_bUserIconsFirst)) g_bUserIconsFirst = true;

  //  Support Version 3.0 and prior config setting for Radar Rings
  bool b300RadarRings = true;
  if (Read("ShowRadarRings", &b300RadarRings)) {
    if (!b300RadarRings) g_iNavAidRadarRingsNumberVisible = 0;
  }

  Read("ConfirmObjectDeletion", &g_bConfirmObjectDelete);

  // Waypoint dragging with mouse
  g_bWayPointPreventDragging = false;
  Read("WaypointPreventDragging", &g_bWayPointPreventDragging);

  g_bEnableZoomToCursor = false;
  Read("EnableZoomToCursor", &g_bEnableZoomToCursor);

  val.Clear();
  Read("TrackIntervalSeconds", &val);
  if (val.Length() > 0) {
    double tval = atof(val.mb_str());
    if (tval >= 2.) g_TrackIntervalSeconds = tval;
  }

  val.Clear();
  Read("TrackDeltaDistance", &val);
  if (val.Length() > 0) {
    double tval = atof(val.mb_str());
    if (tval >= 0.05) g_TrackDeltaDistance = tval;
  }

  Read("TrackPrecision", &g_nTrackPrecision);

  Read("RouteLineWidth", &g_route_line_width);
  Read("TrackLineWidth", &g_track_line_width);

  wxString l_wxsTrackLineColour;
  if (Read("TrackLineColour", &l_wxsTrackLineColour))
    g_colourTrackLineColour.Set(l_wxsTrackLineColour);

  Read("TideCurrentWindowScale", &g_tcwin_scale);
  Read("DefaultWPIcon", &g_default_wp_icon);
  Read("DefaultRPIcon", &g_default_routepoint_icon);

  SetPath("/MmsiProperties");
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
  SetPath("/Settings/GlobalState");

  Read("bShowS57Text", &read_int, 1);
  ps52plib->SetShowS57Text(!(read_int == 0));

  Read("bShowS57ImportantTextOnly", &read_int, 0);
  ps52plib->SetShowS57ImportantTextOnly(!(read_int == 0));

  Read("bShowLightDescription", &read_int, 0);
  ps52plib->SetShowLdisText(!(read_int == 0));

  Read("bExtendLightSectors", &read_int, 0);
  ps52plib->SetExtendLightSectors(!(read_int == 0));

  Read("nDisplayCategory", &read_int, (enum _DisCat)STANDARD);
  ps52plib->SetDisplayCategory((enum _DisCat)read_int);

  Read("nSymbolStyle", &read_int, (enum _LUPname)PAPER_CHART);
  ps52plib->m_nSymbolStyle = (LUPname)read_int;

  Read("nBoundaryStyle", &read_int, PLAIN_BOUNDARIES);
  ps52plib->m_nBoundaryStyle = (LUPname)read_int;

  Read("bShowSoundg", &read_int, 1);
  ps52plib->m_bShowSoundg = !(read_int == 0);

  Read("bShowMeta", &read_int, 0);
  ps52plib->m_bShowMeta = !(read_int == 0);

  Read("bUseSCAMIN", &read_int, 1);
  ps52plib->m_bUseSCAMIN = !(read_int == 0);

  Read("bUseSUPER_SCAMIN", &read_int, 0);
  ps52plib->m_bUseSUPER_SCAMIN = !(read_int == 0);

  Read("bShowAtonText", &read_int, 1);
  ps52plib->m_bShowAtonText = !(read_int == 0);

  Read("bDeClutterText", &read_int, 0);
  ps52plib->m_bDeClutterText = !(read_int == 0);

  Read("bShowNationalText", &read_int, 0);
  ps52plib->m_bShowNationalTexts = !(read_int == 0);

  Read("ENCSoundingScaleFactor", &read_int, 0);
  ps52plib->m_nSoundingFactor = read_int;

  Read("ENCTextScaleFactor", &read_int, 0);
  ps52plib->m_nTextFactor = read_int;

  if (Read("S52_MAR_SAFETY_CONTOUR", &dval, 3.0)) {
    S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, dval);
    S52_setMarinerParam(S52_MAR_SAFETY_DEPTH,
                        dval);  // Set safety_contour and safety_depth the same
  }

  if (Read("S52_MAR_SHALLOW_CONTOUR", &dval, 2.0))
    S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval);

  if (Read("S52_MAR_DEEP_CONTOUR", &dval, 6.0))
    S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval);

  if (Read("S52_MAR_TWO_SHADES", &dval, 0.0))
    S52_setMarinerParam(S52_MAR_TWO_SHADES, dval);

  ps52plib->UpdateMarinerParams();

  SetPath("/Settings/GlobalState");
  Read("S52_DEPTH_UNIT_SHOW", &read_int, 1);  // default is metres
  read_int = wxMax(read_int, 0);              // qualify value
  read_int = wxMin(read_int, 2);
  ps52plib->m_nDepthUnitDisplay = read_int;
  g_nDepthUnitDisplay = read_int;

  //    S57 Object Class Visibility

  OBJLElement *pOLE;

  SetPath("/Settings/ObjectFilter");

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

      if (str.StartsWith("viz", &sObj)) {
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
      if (f.GetExt().IsSameAs("gpx"))
        file_array.Add(filename);  // single-gpx-file layer
      else {
        if (wxDir::Exists(filename)) {
          wxDir dir(filename);
          if (dir.IsOpened()) {
            nfiles = dir.GetAllFiles(filename, &file_array,
                                     "*.gpx");  // layers subdirectory set
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
        laymsg.Printf("New layer %d: %s", l->m_LayerID, l->m_LayerName.c_str());
        wxLogMessage(laymsg);

        pLayerList->insert(pLayerList->begin(), l);

        //  Load the entire file array as a single layer

        for (unsigned int i = 0; i < file_array.GetCount(); i++) {
          wxString file_path = file_array[i];

          if (::wxFileExists(file_path)) {
            NavObjectCollection1 *pSet = new NavObjectCollection1;
            if (pSet->load_file(file_path.fn_str()).status !=
                pugi::xml_parse_status::status_ok) {
              wxLogMessage("Error loading GPX file " + file_path);
              pSet->reset();
            }
            long nItems = pSet->LoadAllGPXObjectsAsLayer(
                l->m_LayerID, bLayerViz, l->m_bHasVisibleNames);
            l->m_NoOfItems += nItems;
            l->m_LayerType = _("Persistent");

            wxString objmsg;
            objmsg.Printf("Loaded GPX file %s with %ld items.",
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
  SetPath("/ChartDirectories");
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
        if (dirname.Find("SampleCharts") ==
            0)  // only update entries starting with "SampleCharts"
        {
          nAdjustChartDirs++;

          pConfig->DeleteEntry(str);
          wxString new_dir = dirname.Mid(dirname.Find("SampleCharts"));
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

bool MyConfig::UpdateChartDirs(ArrayOfCDI &dir_array) {
  wxString key, dir;
  wxString str_buf;

  SetPath("/ChartDirectories");
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
    dirn.Append("^");
    dirn.Append(cdi.magic_number);

    str_buf.Printf("ChartDir%d", iDir + 1);

    Write(str_buf, dirn);
  }

// Avoid nonsense log errors...
#ifdef __ANDROID__
  wxLogNull logNo;
#endif

  Flush();
  return true;
}

void MyConfig::CreateConfigGroups(ChartGroupArray *pGroupArray) {
  if (!pGroupArray) return;

  SetPath("/Groups");
  Write("GroupCount", (int)pGroupArray->GetCount());

  for (unsigned int i = 0; i < pGroupArray->GetCount(); i++) {
    ChartGroup *pGroup = pGroupArray->Item(i);
    wxString s;
    s.Printf("Group%d", i + 1);
    s.Prepend("/Groups/");
    SetPath(s);

    Write("GroupName", pGroup->m_group_name);
    Write("GroupItemCount", (int)pGroup->m_element_array.size());

    for (unsigned int j = 0; j < pGroup->m_element_array.size(); j++) {
      wxString sg;
      sg.Printf("Group%d/Item%d", i + 1, j);
      sg.Prepend("/Groups/");
      SetPath(sg);
      Write("IncludeItem", pGroup->m_element_array[j].m_element_name);

      wxString t;
      wxArrayString u = pGroup->m_element_array[j].m_missing_name_array;
      if (u.GetCount()) {
        for (unsigned int k = 0; k < u.GetCount(); k++) {
          t += u[k];
          t += ";";
        }
        Write("ExcludeItems", t);
      }
    }
  }
}

void MyConfig::DestroyConfigGroups() {
  DeleteGroup("/Groups");  // zap
}

void MyConfig::LoadConfigGroups(ChartGroupArray *pGroupArray) {
  SetPath("/Groups");
  unsigned int group_count;
  Read("GroupCount", (int *)&group_count, 0);

  for (unsigned int i = 0; i < group_count; i++) {
    ChartGroup *pGroup = new ChartGroup;
    wxString s;
    s.Printf("Group%d", i + 1);
    s.Prepend("/Groups/");
    SetPath(s);

    wxString t;
    Read("GroupName", &t);
    pGroup->m_group_name = t;

    unsigned int item_count;
    Read("GroupItemCount", (int *)&item_count);
    for (unsigned int j = 0; j < item_count; j++) {
      wxString sg;
      sg.Printf("Group%d/Item%d", i + 1, j);
      sg.Prepend("/Groups/");
      SetPath(sg);

      wxString v;
      Read("IncludeItem", &v);

      ChartGroupElement pelement{v};
      wxString u;
      if (Read("ExcludeItems", &u)) {
        if (!u.IsEmpty()) {
          wxStringTokenizer tk(u, ";");
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

  SetPath("/Canvas");

  //  If the canvas config has never been set/persisted, use the global settings
  if (!HasEntry("CanvasConfig")) {
    pcc = new canvasConfig(0);
    pcc->LoadFromLegacyConfig(this);
    config_array.Add(pcc);

    return;
  }

  Read("CanvasConfig", (int *)&g_canvasConfig, 0);

  // Do not recreate canvasConfigs when applying config dynamically
  if (config_array.GetCount() == 0) {  // This is initial load from startup
    s.Printf("/Canvas/CanvasConfig%d", 1);
    SetPath(s);
    canvasConfig *pcca = new canvasConfig(0);
    LoadConfigCanvas(pcca, bApplyAsTemplate);
    config_array.Add(pcca);

    s.Printf("/Canvas/CanvasConfig%d", 2);
    SetPath(s);
    pcca = new canvasConfig(1);
    LoadConfigCanvas(pcca, bApplyAsTemplate);
    config_array.Add(pcca);
  } else {  // This is a dynamic (i.e. Template) load
    canvasConfig *pcca = config_array[0];
    s.Printf("/Canvas/CanvasConfig%d", 1);
    SetPath(s);
    LoadConfigCanvas(pcca, bApplyAsTemplate);

    if (config_array.GetCount() > 1) {
      canvasConfig *pcca = config_array[1];
      s.Printf("/Canvas/CanvasConfig%d", 2);
      SetPath(s);
      LoadConfigCanvas(pcca, bApplyAsTemplate);
    } else {
      s.Printf("/Canvas/CanvasConfig%d", 2);
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

    if (Read("canvasVPLatLon", &st)) {
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
    if (Read(wxString("canvasVPScale"), &st)) {
      sscanf(st.mb_str(wxConvUTF8), "%lf", &st_view_scale);
      //    Sanity check the scale
      st_view_scale = fmax(st_view_scale, .001 / 32);
      st_view_scale = fmin(st_view_scale, 4);
      cConfig->iScale = st_view_scale;
    }

    double st_rotation;
    if (Read(wxString("canvasVPRotation"), &st)) {
      sscanf(st.mb_str(wxConvUTF8), "%lf", &st_rotation);
      //    Sanity check the rotation
      st_rotation = fmin(st_rotation, 360);
      st_rotation = fmax(st_rotation, 0);
      cConfig->iRotation = st_rotation * PI / 180.;
    }

    Read("canvasInitialdBIndex", &cConfig->DBindex, 0);
    Read("canvasbFollow", &cConfig->bFollow, 0);

    Read("canvasCourseUp", &cConfig->bCourseUp, 0);
    Read("canvasHeadUp", &cConfig->bHeadUp, 0);
    Read("canvasLookahead", &cConfig->bLookahead, 0);
  }

  Read("ActiveChartGroup", &cConfig->GroupID, 0);

  // Special check for group selection when applied as template
  if (cConfig->GroupID && bApplyAsTemplate) {
    if (cConfig->GroupID > (int)g_pGroupArray->GetCount()) cConfig->GroupID = 0;
  }

  Read("canvasShowTides", &cConfig->bShowTides, 0);
  Read("canvasShowCurrents", &cConfig->bShowCurrents, 0);

  Read("canvasQuilt", &cConfig->bQuilt, 1);
  Read("canvasShowGrid", &cConfig->bShowGrid, 0);
  Read("canvasShowOutlines", &cConfig->bShowOutlines, 0);
  Read("canvasShowDepthUnits", &cConfig->bShowDepthUnits, 0);

  Read("canvasShowAIS", &cConfig->bShowAIS, 1);
  Read("canvasAttenAIS", &cConfig->bAttenAIS, 0);

  // ENC options
  Read("canvasShowENCText", &cConfig->bShowENCText, 1);
  Read("canvasENCDisplayCategory", &cConfig->nENCDisplayCategory, STANDARD);
  Read("canvasENCShowDepths", &cConfig->bShowENCDepths, 1);
  Read("canvasENCShowBuoyLabels", &cConfig->bShowENCBuoyLabels, 1);
  Read("canvasENCShowLightDescriptions", &cConfig->bShowENCLightDescriptions,
       1);
  Read("canvasENCShowLights", &cConfig->bShowENCLights, 1);
  Read("canvasENCShowVisibleSectorLights",
       &cConfig->bShowENCVisibleSectorLights, 0);
  Read("canvasENCShowAnchorInfo", &cConfig->bShowENCAnchorInfo, 0);
  Read("canvasENCShowDataQuality", &cConfig->bShowENCDataQuality, 0);

  int sx, sy;
  Read("canvasSizeX", &sx, 0);
  Read("canvasSizeY", &sy, 0);
  cConfig->canvasSize = wxSize(sx, sy);
}

void MyConfig::SaveCanvasConfigs() {
  auto &config_array = ConfigMgr::Get().GetCanvasConfigArray();

  SetPath("/Canvas");
  Write("CanvasConfig", (int)g_canvasConfig);

  wxString s;
  canvasConfig *pcc;

  switch (g_canvasConfig) {
    case 0:
    default:

      s.Printf("/Canvas/CanvasConfig%d", 1);
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
        s.Printf("/Canvas/CanvasConfig%d", 1);
        SetPath(s);
        pcc = config_array.Item(0);
        if (pcc) {
          SaveConfigCanvas(pcc);
        }

        s.Printf("/Canvas/CanvasConfig%d", 2);
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
      st1.Printf("%10.4f,%10.4f", vp.clat, vp.clon);
      Write("canvasVPLatLon", st1);
      st1.Printf("%g", vp.view_scale_ppm);
      Write("canvasVPScale", st1);
      st1.Printf("%i", ((int)(vp.rotation * 180 / PI)) % 360);
      Write("canvasVPRotation", st1);
    }

    int restore_dbindex = 0;
    ChartStack *pcs = cConfig->canvas->GetpCurrentStack();
    if (pcs) restore_dbindex = pcs->GetCurrentEntrydbIndex();
    if (cConfig->canvas->GetQuiltMode())
      restore_dbindex = cConfig->canvas->GetQuiltReferenceChartIndex();
    Write("canvasInitialdBIndex", restore_dbindex);

    Write("canvasbFollow", cConfig->canvas->m_bFollow);
    Write("ActiveChartGroup", cConfig->canvas->m_groupIndex);

    Write("canvasQuilt", cConfig->canvas->GetQuiltMode());
    Write("canvasShowGrid", cConfig->canvas->GetShowGrid());
    Write("canvasShowOutlines", cConfig->canvas->GetShowOutlines());
    Write("canvasShowDepthUnits", cConfig->canvas->GetShowDepthUnits());

    Write("canvasShowAIS", cConfig->canvas->GetShowAIS());
    Write("canvasAttenAIS", cConfig->canvas->GetAttenAIS());

    Write("canvasShowTides", cConfig->canvas->GetbShowTide());
    Write("canvasShowCurrents", cConfig->canvas->GetbShowCurrent());

    // ENC options
    Write("canvasShowENCText", cConfig->canvas->GetShowENCText());
    Write("canvasENCDisplayCategory", cConfig->canvas->GetENCDisplayCategory());
    Write("canvasENCShowDepths", cConfig->canvas->GetShowENCDepth());
    Write("canvasENCShowBuoyLabels", cConfig->canvas->GetShowENCBuoyLabels());
    Write("canvasENCShowLightDescriptions",
          cConfig->canvas->GetShowENCLightDesc());
    Write("canvasENCShowLights", cConfig->canvas->GetShowENCLights());
    Write("canvasENCShowVisibleSectorLights",
          cConfig->canvas->GetShowVisibleSectors());
    Write("canvasENCShowAnchorInfo", cConfig->canvas->GetShowENCAnchor());
    Write("canvasENCShowDataQuality", cConfig->canvas->GetShowENCDataQual());
    Write("canvasCourseUp", cConfig->canvas->GetUpMode() == COURSE_UP_MODE);
    Write("canvasHeadUp", cConfig->canvas->GetUpMode() == HEAD_UP_MODE);
    Write("canvasLookahead", cConfig->canvas->GetLookahead());

    int width = cConfig->canvas->GetSize().x;
    //         if(cConfig->canvas->IsPrimaryCanvas()){
    //             width = wxMax(width, gFrame->GetClientSize().x / 10);
    //         }
    //         else{
    //             width = wxMin(width, gFrame->GetClientSize().x  * 9 / 10);
    //         }

    Write("canvasSizeX", width);
    Write("canvasSizeY", cConfig->canvas->GetSize().y);
  }
}

void MyConfig::UpdateSettings() {
  //  Temporarily suppress logging of trivial non-fatal wxLogSysError() messages
  //  provoked by Android security...
#ifdef __ANDROID__
  wxLogNull logNo;
#endif

  //    Global options and settings
  SetPath("/Settings");

  Write("LastAppliedTemplate", g_lastAppliedTemplateGUID);
  Write("CompatOS", g_compatOS);
  Write("CompatOsVersion", g_compatOsVersion);
  Write("ConfigVersionString", g_config_version_string);
  if (wxIsEmpty(g_CmdSoundString)) g_CmdSoundString = wxString(OCPN_SOUND_CMD);
  Write("CmdSoundString", g_CmdSoundString);
  Write("NavMessageShown", n_NavMessageShown);
  Write("InlandEcdis", g_bInlandEcdis);

  Write("AndroidVersionCode", g_AndroidVersionCode);

  Write("UIexpert", g_bUIexpert);
  Write("SpaceDropMark", g_bSpaceDropMark);
  //    Write( "UIStyle", g_StyleManager->GetStyleNextInvocation() );
  //    //Not desired for O5 MUI

  Write("ShowStatusBar", g_bShowStatusBar);
#ifndef __WXOSX__
  Write("ShowMenuBar", g_bShowMenuBar);
#endif
  Write("DefaultFontSize", g_default_font_size);
  Write("DefaultFontFacename", g_default_font_facename);

  Write("Fullscreen", g_bFullscreen);
  Write("ShowCompassWindow", g_bShowCompassWin);
  Write("SetSystemTime", s_bSetSystemTime);
  Write("ShowGrid", g_bDisplayGrid);
  Write("PlayShipsBells", g_bPlayShipsBells);
  Write("SoundDeviceIndex", g_iSoundDeviceIndex);
  Write("FullscreenToolbar", g_bFullscreenToolbar);
  Write("TransparentToolbar", g_bTransparentToolbar);
  Write("PermanentMOBIcon", g_bPermanentMOBIcon);
  Write("ShowLayers", g_bShowLayers);
  Write("AutoAnchorDrop", g_bAutoAnchorMark);
  Write("ShowChartOutlines", g_bShowOutlines);
  Write("ShowActiveRouteTotal", g_bShowRouteTotal);
  Write("ShowActiveRouteHighway", g_bShowActiveRouteHighway);
  Write("SDMMFormat", g_iSDMMFormat);
  Write("MostRecentGPSUploadConnection", g_uploadConnection);
  Write("ShowChartBar", g_bShowChartBar);

  Write("GUIScaleFactor", g_GUIScaleFactor);
  Write("ChartObjectScaleFactor", g_ChartScaleFactor);
  Write("ShipScaleFactor", g_ShipScaleFactor);
  Write("ENCSoundingScaleFactor", g_ENCSoundingScaleFactor);
  Write("ENCTextScaleFactor", g_ENCTextScaleFactor);
  Write("ObjQueryAppendFilesExt", g_ObjQFileExt);

  // Plugin catalog persistent values.
  Write("CatalogCustomURL", g_catalog_custom_url);
  Write("CatalogChannel", g_catalog_channel);

  Write("NetmaskBits", g_netmask_bits);
  Write("FilterNMEA_Avg", g_bfilter_cogsog);
  Write("FilterNMEA_Sec", g_COGFilterSec);

  Write("TrackContinuous", g_btrackContinuous);

  Write("ShowTrue", g_bShowTrue);
  Write("ShowMag", g_bShowMag);
  Write("UserMagVariation", wxString::Format("%.2f", g_UserVar));

  Write("CM93DetailFactor", g_cm93_zoom_factor);
  Write("CM93DetailZoomPosX", g_detailslider_dialog_x);
  Write("CM93DetailZoomPosY", g_detailslider_dialog_y);
  Write("ShowCM93DetailSlider", g_bShowDetailSlider);

  Write("SkewToNorthUp", g_bskew_comp);
  if (!g_bdisable_opengl) {  // Only modify the saved value if OpenGL is not
                             // force-disabled from the command line
    Write("OpenGL", g_bopengl);
  }
  Write("SoftwareGL", g_bSoftwareGL);

  Write("ZoomDetailFactor", g_chart_zoom_modifier_raster);
  Write("ZoomDetailFactorVector", g_chart_zoom_modifier_vector);

  Write("FogOnOverzoom", g_fog_overzoom);
  Write("OverzoomVectorScale", g_oz_vector_scale);
  Write("OverzoomEmphasisBase", g_overzoom_emphasis_base);
  Write("PlusMinusZoomFactor", g_plus_minus_zoom_factor);
  Write("MouseZoomSensitivity",
        MouseZoom::ui_to_config(g_mouse_zoom_sensitivity_ui));
  Write("ShowMUIZoomButtons", g_bShowMuiZoomButtons);

#ifdef ocpnUSE_GL
  /* opengl options */
  Write("UseAcceleratedPanning", g_GLOptions.m_bUseAcceleratedPanning);

  Write("GPUTextureCompression", g_GLOptions.m_bTextureCompression);
  Write("GPUTextureCompressionCaching",
        g_GLOptions.m_bTextureCompressionCaching);
  Write("GPUTextureDimension", g_GLOptions.m_iTextureDimension);
  Write("GPUTextureMemSize", g_GLOptions.m_iTextureMemorySize);
  Write("PolygonSmoothing", g_GLOptions.m_GLPolygonSmoothing);
  Write("LineSmoothing", g_GLOptions.m_GLLineSmoothing);
#endif
  Write("SmoothPanZoom", g_bsmoothpanzoom);

  Write("CourseUpMode", g_bCourseUp);
  if (!g_bInlandEcdis) Write("LookAheadMode", g_bLookAhead);
  Write("TenHzUpdate", g_btenhertz);

  Write("COGUPAvgSeconds", g_COGAvgSec);
  Write("UseMagAPB", g_bMagneticAPB);

  Write("OwnshipCOGPredictorMinutes", g_ownship_predictor_minutes);
  Write("OwnshipCOGPredictorStyle", g_cog_predictor_style);
  Write("OwnshipCOGPredictorColor", g_cog_predictor_color);
  Write("OwnshipCOGPredictorEndmarker", g_cog_predictor_endmarker);
  Write("OwnshipCOGPredictorWidth", g_cog_predictor_width);
  Write("OwnshipHDTPredictorStyle", g_ownship_HDTpredictor_style);
  Write("OwnshipHDTPredictorColor", g_ownship_HDTpredictor_color);
  Write("OwnshipHDTPredictorEndmarker", g_ownship_HDTpredictor_endmarker);
  Write("OwnShipMMSINumber", g_OwnShipmmsi);
  Write("OwnshipHDTPredictorWidth", g_ownship_HDTpredictor_width);
  Write("OwnshipHDTPredictorMiles", g_ownship_HDTpredictor_miles);

  Write("OwnShipIconType", g_OwnShipIconType);
  Write("OwnShipLength", g_n_ownship_length_meters);
  Write("OwnShipWidth", g_n_ownship_beam_meters);
  Write("OwnShipGPSOffsetX", g_n_gps_antenna_offset_x);
  Write("OwnShipGPSOffsetY", g_n_gps_antenna_offset_y);
  Write("OwnShipMinSize", g_n_ownship_min_mm);
  Write("OwnShipSogCogCalc", g_own_ship_sog_cog_calc);
  Write("OwnShipSogCogCalcDampSec", g_own_ship_sog_cog_calc_damp_sec);
  Write("ShowDirectRouteLine", g_bShowShipToActive);
  Write("DirectRouteLineStyle", g_shipToActiveStyle);
  Write("DirectRouteLineColor", g_shipToActiveColor);

  wxString racr;
  //   racr.Printf( "%g", g_n_arrival_circle_radius );
  //   Write( "RouteArrivalCircleRadius", racr );
  Write("RouteArrivalCircleRadius",
        wxString::Format("%.2f", g_n_arrival_circle_radius));

  Write("ChartQuilting", g_bQuiltEnable);

  Write("PreserveScaleOnX", g_bPreserveScaleOnX);

  Write("StartWithTrackActive", g_bTrackCarryOver);
  Write("AutomaticDailyTracks", g_bTrackDaily);
  Write("TrackRotateAt", g_track_rotate_time);
  Write("TrackRotateTimeType", g_track_rotate_time_type);
  Write("HighlightTracks", g_bHighliteTracks);

  Write("DateTimeFormat", g_datetime_format);
  Write("InitialStackIndex", g_restore_stackindex);
  Write("InitialdBIndex", g_restore_dbindex);

  Write("NMEAAPBPrecision", g_NMEAAPBPrecision);

  Write("TalkerIdText", g_TalkerIdText);
  Write("ShowTrackPointTime", g_bShowTrackPointTime);

  Write("AnchorWatch1GUID", g_AW1GUID);
  Write("AnchorWatch2GUID", g_AW2GUID);

  Write("ToolbarX", g_maintoolbar_x);
  Write("ToolbarY", g_maintoolbar_y);
  // Write( "ToolbarOrient", g_maintoolbar_orient );

  Write("iENCToolbarX", g_iENCToolbarPosX);
  Write("iENCToolbarY", g_iENCToolbarPosY);

  if (!g_bInlandEcdis) {
    Write("GlobalToolbarConfig", g_toolbarConfig);
    Write("DistanceFormat", g_iDistanceFormat);
    Write("SpeedFormat", g_iSpeedFormat);
    Write("WindSpeedFormat", g_iWindSpeedFormat);
    Write("ShowDepthUnits", g_bShowDepthUnits);
    Write("TemperatureFormat", g_iTempFormat);
    Write("HeightFormat", g_iHeightFormat);
  }
  Write("GPSIdent", g_GPS_Ident);
  Write("ActiveRoute", g_active_route);
  Write("PersistActiveRoute", g_persist_active_route);
  Write("AlwaysSendRmbRmc", g_always_send_rmb_rmc);

  Write("UseGarminHostUpload", g_bGarminHostUpload);

  Write("MobileTouch", g_btouch);
  Write("ResponsiveGraphics", g_bresponsive);
  Write("EnableRolloverBlock", g_bRollover);

  Write("AutoHideToolbar", g_bAutoHideToolbar);
  Write("AutoHideToolbarSecs", g_nAutoHideToolbar);

  wxString st0;
  for (const auto &mm : g_config_display_size_mm) {
    st0.Append(wxString::Format("%zu,", mm));
  }
  st0.RemoveLast();  // Strip last comma
  Write("DisplaySizeMM", st0);
  Write("DisplaySizeManual", g_config_display_size_manual);

  Write("SelectionRadiusMM", g_selection_radius_mm);
  Write("SelectionRadiusTouchMM", g_selection_radius_touch_mm);

  st0.Printf("%g", g_PlanSpeed);
  Write("PlanSpeed", st0);

  if (g_bLayersLoaded) {
    wxString vis, invis, visnames, invisnames;
    LayerList::iterator it;
    int index = 0;
    for (it = (*pLayerList).begin(); it != (*pLayerList).end(); ++it, ++index) {
      Layer *lay = (Layer *)(*it);
      if (lay->IsVisibleOnChart())
        vis += (lay->m_LayerName) + ";";
      else
        invis += (lay->m_LayerName) + ";";

      if (lay->HasVisibleNames() == wxCHK_CHECKED) {
        visnames += (lay->m_LayerName) + ";";
      } else if (lay->HasVisibleNames() == wxCHK_UNCHECKED) {
        invisnames += (lay->m_LayerName) + ";";
      }
    }
    Write("VisibleLayers", vis);
    Write("InvisibleLayers", invis);
    Write("VisNameInLayers", visnames);
    Write("InvisNameInLayers", invisnames);
  }
  Write("Locale", g_locale);
  Write("LocaleOverride", g_localeOverride);

  Write("KeepNavobjBackups", g_navobjbackups);
  Write("LegacyInputCOMPortFilterBehaviour", g_b_legacy_input_filter_behaviour);
  Write("AdvanceRouteWaypointOnArrivalOnly",
        g_bAdvanceRouteWaypointOnArrivalOnly);
  Write("EnableRootMenuDebug", g_enable_root_menu_debug);

  // LIVE ETA OPTION
  Write("LiveETA", g_bShowLiveETA);
  Write("DefaultBoatSpeed", g_defaultBoatSpeed);

  //    S57 Object Filter Settings

  SetPath("/Settings/ObjectFilter");

  if (ps52plib) {
    for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount();
         iPtr++) {
      OBJLElement *pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));

      wxString st1("viz");
      char name[7];
      strncpy(name, pOLE->OBJLName, 6);
      name[6] = 0;
      st1.Append(wxString(name, wxConvUTF8));
      Write(st1, pOLE->nViz);
    }
  }

  //    Global State

  SetPath("/Settings/GlobalState");

  wxString st1;

  //     if( cc1 ) {
  //         ViewPort vp = cc1->GetVP();
  //
  //         if( vp.IsValid() ) {
  //             st1.Printf( "%10.4f,%10.4f", vp.clat, vp.clon );
  //             Write( "VPLatLon", st1 );
  //             st1.Printf( "%g", vp.view_scale_ppm );
  //             Write( "VPScale", st1 );
  //             st1.Printf( "%i", ((int)(vp.rotation * 180 / PI)) % 360
  //             ); Write( "VPRotation", st1 );
  //         }
  //     }

  st1.Printf("%10.4f, %10.4f", gLat, gLon);
  Write("OwnShipLatLon", st1);

  //    Various Options
  SetPath("/Settings/GlobalState");
  if (!g_bInlandEcdis) Write("nColorScheme", (int)gFrame->GetColorScheme());

  Write("FrameWinX", g_nframewin_x);
  Write("FrameWinY", g_nframewin_y);
  Write("FrameWinPosX", g_nframewin_posx);
  Write("FrameWinPosY", g_nframewin_posy);
  Write("FrameMax", g_bframemax);

  Write("ClientPosX", g_lastClientRectx);
  Write("ClientPosY", g_lastClientRecty);
  Write("ClientSzX", g_lastClientRectw);
  Write("ClientSzY", g_lastClientRecth);

  Write("S52_DEPTH_UNIT_SHOW", g_nDepthUnitDisplay);

  Write("RoutePropSizeX", g_route_prop_sx);
  Write("RoutePropSizeY", g_route_prop_sy);
  Write("RoutePropPosX", g_route_prop_x);
  Write("RoutePropPosY", g_route_prop_y);

  // Sounds
  SetPath("/Settings/Audio");
  Write("AISAlertSoundFile", g_AIS_sound_file);
  Write("DSCAlertSoundFile", g_DSC_sound_file);
  Write("SARTAlertSoundFile", g_SART_sound_file);
  Write("AnchorAlarmSoundFile", g_anchorwatch_sound_file);

  Write("bAIS_GCPA_AlertAudio", g_bAIS_GCPA_Alert_Audio);
  Write("bAIS_SART_AlertAudio", g_bAIS_SART_Alert_Audio);
  Write("bAIS_DSC_AlertAudio", g_bAIS_DSC_Alert_Audio);
  Write("bAnchorAlertAudio", g_bAnchor_Alert_Audio);

  //    AIS
  SetPath("/Settings/AIS");

  Write("bNoCPAMax", g_bCPAMax);
  Write("NoCPAMaxNMi", g_CPAMax_NM);
  Write("bCPAWarn", g_bCPAWarn);
  Write("CPAWarnNMi", g_CPAWarn_NM);
  Write("bTCPAMax", g_bTCPA_Max);
  Write("TCPAMaxMinutes", g_TCPA_Max);
  Write("bMarkLostTargets", g_bMarkLost);
  Write("MarkLost_Minutes", g_MarkLost_Mins);
  Write("bRemoveLostTargets", g_bRemoveLost);
  Write("RemoveLost_Minutes", g_RemoveLost_Mins);
  Write("bShowCOGArrows", g_bShowCOG);
  Write("bSyncCogPredictors", g_bSyncCogPredictors);
  Write("CogArrowMinutes", g_ShowCOG_Mins);
  Write("bShowTargetTracks", g_bAISShowTracks);
  Write("TargetTracksMinutes", g_AISShowTracks_Mins);

  Write("bHideMooredTargets", g_bHideMoored);
  Write("MooredTargetMaxSpeedKnots", g_ShowMoored_Kts);

  Write("bAISAlertDialog", g_bAIS_CPA_Alert);
  Write("bAISAlertAudio", g_bAIS_CPA_Alert_Audio);

  Write("AISAlertAudioFile", g_sAIS_Alert_Sound_File);
  Write("bAISAlertSuppressMoored", g_bAIS_CPA_Alert_Suppress_Moored);
  Write("bShowAreaNotices", g_bShowAreaNotices);
  Write("bDrawAISSize", g_bDrawAISSize);
  Write("bDrawAISRealtime", g_bDrawAISRealtime);
  Write("AISRealtimeMinSpeedKnots", g_AIS_RealtPred_Kts);
  Write("bShowAISName", g_bShowAISName);
  Write("ShowAISTargetNameScale", g_Show_Target_Name_Scale);
  Write("bWplIsAprsPositionReport", g_bWplUsePosition);
  Write("WplSelAction", g_WplAction);
  Write("AISCOGPredictorWidth", g_ais_cog_predictor_width);
  Write("bShowScaledTargets", g_bAllowShowScaled);
  Write("AISScaledNumber", g_ShowScaled_Num);
  Write("AISScaledNumberWeightSOG", g_ScaledNumWeightSOG);
  Write("AISScaledNumberWeightCPA", g_ScaledNumWeightCPA);
  Write("AISScaledNumberWeightTCPA", g_ScaledNumWeightTCPA);
  Write("AISScaledNumberWeightRange", g_ScaledNumWeightRange);
  Write("AISScaledNumberWeightSizeOfTarget", g_ScaledNumWeightSizeOfT);
  Write("AISScaledSizeMinimal", g_ScaledSizeMinimal);
  Write("AISShowScaled", g_bShowScaled);

  Write("AlertDialogSizeX", g_ais_alert_dialog_sx);
  Write("AlertDialogSizeY", g_ais_alert_dialog_sy);
  Write("AlertDialogPosX", g_ais_alert_dialog_x);
  Write("AlertDialogPosY", g_ais_alert_dialog_y);
  Write("QueryDialogPosX", g_ais_query_dialog_x);
  Write("QueryDialogPosY", g_ais_query_dialog_y);
  Write("AISTargetListPerspective", g_AisTargetList_perspective);
  Write("AISTargetListRange", g_AisTargetList_range);
  Write("AISTargetListSortColumn", g_AisTargetList_sortColumn);
  Write("bAISTargetListSortReverse", g_bAisTargetList_sortReverse);
  Write("AISTargetListColumnSpec", g_AisTargetList_column_spec);
  Write("AISTargetListColumnOrder", g_AisTargetList_column_order);

  Write("S57QueryDialogSizeX", g_S57_dialog_sx);
  Write("S57QueryDialogSizeY", g_S57_dialog_sy);
  Write("S57QueryExtraDialogSizeX", g_S57_extradialog_sx);
  Write("S57QueryExtraDialogSizeY", g_S57_extradialog_sy);

  Write("bAISRolloverShowClass", g_bAISRolloverShowClass);
  Write("bAISRolloverShowCOG", g_bAISRolloverShowCOG);
  Write("bAISRolloverShowCPA", g_bAISRolloverShowCPA);

  Write("bAISAlertAckTimeout", g_bAIS_ACK_Timeout);
  Write("AlertAckTimeoutMinutes", g_AckTimeout_Mins);

  SetPath("/Settings/GlobalState");
  if (ps52plib) {
    Write("bShowS57Text", ps52plib->GetShowS57Text());
    Write("bShowS57ImportantTextOnly", ps52plib->GetShowS57ImportantTextOnly());
    if (!g_bInlandEcdis)
      Write("nDisplayCategory", (long)ps52plib->GetDisplayCategory());
    Write("nSymbolStyle", (int)ps52plib->m_nSymbolStyle);
    Write("nBoundaryStyle", (int)ps52plib->m_nBoundaryStyle);

    Write("bShowSoundg", ps52plib->m_bShowSoundg);
    Write("bShowMeta", ps52plib->m_bShowMeta);
    Write("bUseSCAMIN", ps52plib->m_bUseSCAMIN);
    Write("bUseSUPER_SCAMIN", ps52plib->m_bUseSUPER_SCAMIN);
    Write("bShowAtonText", ps52plib->m_bShowAtonText);
    Write("bShowLightDescription", ps52plib->m_bShowLdisText);
    Write("bExtendLightSectors", ps52plib->m_bExtendLightSectors);
    Write("bDeClutterText", ps52plib->m_bDeClutterText);
    Write("bShowNationalText", ps52plib->m_bShowNationalTexts);

    Write("S52_MAR_SAFETY_CONTOUR",
          S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR));
    Write("S52_MAR_SHALLOW_CONTOUR",
          S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR));
    Write("S52_MAR_DEEP_CONTOUR", S52_getMarinerParam(S52_MAR_DEEP_CONTOUR));
    Write("S52_MAR_TWO_SHADES", S52_getMarinerParam(S52_MAR_TWO_SHADES));
    Write("S52_DEPTH_UNIT_SHOW", ps52plib->m_nDepthUnitDisplay);
    Write("ENCSoundingScaleFactor", g_ENCSoundingScaleFactor);
    Write("ENCTextScaleFactor", g_ENCTextScaleFactor);
  }
  SetPath("/Directories");
  Write("S57DataLocation", "");
  //    Write( "SENCFileLocation", "" );

  SetPath("/Directories");
  Write("InitChartDir", *pInit_Chart_Dir);
  Write("GPXIODir", g_gpx_path);
  Write("TCDataDir", g_TCData_Dir);
  Write("BasemapDir", g_Platform->NormalizePath(gWorldMapLocation));
  Write("BaseShapefileDir", g_Platform->NormalizePath(gWorldShapefileLocation));
  Write("pluginInstallDir", g_Platform->NormalizePath(g_winPluginDir));

  SetPath("/Settings/NMEADataSource");
  wxString connectionconfigs;
  for (size_t i = 0; i < TheConnectionParams().size(); i++) {
    if (i > 0) connectionconfigs.Append("|");
    connectionconfigs.Append(TheConnectionParams()[i]->Serialize());
  }
  Write("DataConnections", connectionconfigs);

  //    Fonts

  //  Store the persistent Auxiliary Font descriptor Keys
  SetPath("/Settings/AuxFontKeys");

  wxArrayString keyArray = FontMgr::Get().GetAuxKeyArray();
  for (unsigned int i = 0; i < keyArray.GetCount(); i++) {
    wxString key;
    key.Printf("Key%i", i);
    wxString keyval = keyArray[i];
    Write(key, keyval);
  }

  wxString font_path;
#ifdef __WXX11__
  font_path = ("/Settings/X11Fonts");
#endif

#ifdef __WXGTK__
  font_path = ("/Settings/GTKFonts");
#endif

#ifdef __WXMSW__
  font_path = ("/Settings/MSWFonts");
#endif

#ifdef __WXMAC__
  font_path = ("/Settings/MacFonts");
#endif

#ifdef __WXQT__
  font_path = ("/Settings/QTFonts");
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
  DeleteGroup("/TideCurrentDataSources");
  SetPath("/TideCurrentDataSources");
  unsigned int id = 0;
  for (auto val : TideCurrentDataSet) {
    wxString key;
    key.Printf("tcds%d", id);
    Write(key, wxString(val));
    ++id;
  }

  SetPath("/Settings/Others");

  // Radar rings
  Write("ShowRadarRings",
        (bool)(g_iNavAidRadarRingsNumberVisible > 0));  // 3.0.0 config support
  Write("RadarRingsNumberVisible", g_iNavAidRadarRingsNumberVisible);
  Write("RadarRingsStep", g_fNavAidRadarRingsStep);
  Write("RadarRingsStepUnits", g_pNavAidRadarRingsStepUnits);
  Write("RadarRingsColour",
        g_colourOwnshipRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX));
  Write("WaypointUseScaMin", g_bUseWptScaMin);
  Write("WaypointScaMinValue", g_iWpt_ScaMin);
  Write("WaypointUseScaMinOverrule", g_bOverruleScaMin);
  Write("WaypointsShowName", g_bShowWptName);
  Write("UserIconsFirst", g_bUserIconsFirst);

  // Waypoint Radar rings
  Write("WaypointRangeRingsNumber", g_iWaypointRangeRingsNumber);
  Write("WaypointRangeRingsStep", g_fWaypointRangeRingsStep);
  Write("WaypointRangeRingsStepUnits", g_iWaypointRangeRingsStepUnits);
  Write("WaypointRangeRingsColour",
        g_colourWaypointRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX));

  Write("ConfirmObjectDeletion", g_bConfirmObjectDelete);

  // Waypoint dragging with mouse; toh, 2009.02.24
  Write("WaypointPreventDragging", g_bWayPointPreventDragging);

  Write("EnableZoomToCursor", g_bEnableZoomToCursor);

  Write("TrackIntervalSeconds", g_TrackIntervalSeconds);
  Write("TrackDeltaDistance", g_TrackDeltaDistance);
  Write("TrackPrecision", g_nTrackPrecision);

  Write("RouteLineWidth", g_route_line_width);
  Write("TrackLineWidth", g_track_line_width);
  Write("TrackLineColour",
        g_colourTrackLineColour.GetAsString(wxC2S_HTML_SYNTAX));
  Write("DefaultWPIcon", g_default_wp_icon);
  Write("DefaultRPIcon", g_default_routepoint_icon);

  DeleteGroup("/MmsiProperties");
  SetPath("/MmsiProperties");
  for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
    wxString p;
    p.Printf("Props%d", i);
    Write(p, g_MMSI_Props_Array[i]->Serialize());
  }

  SaveCanvasConfigs();

  Flush();
  SendMessageToAllPlugins("GLOBAL_SETTINGS_UPDATED", "{\"updated\":\"1\"}");
}

static wxFileName exportFileName(wxWindow *parent,
                                 const wxString suggestedName) {
  wxFileName ret;
  wxString path;
  wxString valid_name = SanitizeFileName(suggestedName);

#ifdef __ANDROID__
  if (!valid_name.EndsWith(".gpx")) {
    wxFileName fn(valid_name);
    fn.ClearExt();
    fn.SetExt("gpx");
    valid_name = fn.GetFullName();
  }
#endif
  int response = g_Platform->DoFileSelectorDialog(
      parent, &path, _("Export GPX file"), g_gpx_path, valid_name, "*.gpx");

  if (response == wxID_OK) {
    wxFileName fn(path);
    g_gpx_path = fn.GetPath();
    if (!fn.GetExt().StartsWith("gpx")) fn.SetExt("gpx");

#if defined(__WXMSW__) || defined(__WXGTK__)
    if (wxFileExists(fn.GetFullPath())) {
      int answer = OCPNMessageBox(NULL, _("Overwrite existing file?"),
                                  "Confirm", wxICON_QUESTION | wxYES_NO);
      if (answer != wxID_YES) return ret;
    }
#endif
    ret = fn;
  }
  return ret;
}

int BackupDatabase(wxWindow *parent) {
  bool backupResult = false;
  wxDateTime tm = wxDateTime::Now();
  wxString proposedName = tm.Format("navobj-%Y-%m-%d_%H_%M");
  wxString acceptedName;

  if (wxID_OK ==
      g_Platform->DoFileSelectorDialog(parent, &acceptedName, _("Backup"),
                                       wxStandardPaths::Get().GetDocumentsDir(),
                                       proposedName, "*.bkp")) {
    wxFileName fileName(acceptedName);
    if (fileName.IsOk()) {
#if defined(__WXMSW__) || defined(__WXGTK__)
      if (fileName.FileExists()) {
        if (wxID_YES != OCPNMessageBox(NULL, _("Overwrite existing file?"),
                                       "Confirm", wxICON_QUESTION | wxYES_NO)) {
          return wxID_ABORT;  // We've decided not to overwrite a file, aborting
        }
      }
#endif

#ifdef __ANDROID__
      wxString secureFileName = androidGetCacheDir() +
                                wxFileName::GetPathSeparator() +
                                fileName.GetFullName();
      backupResult = NavObj_dB::GetInstance().Backup(secureFileName);
      AndroidSecureCopyFile(secureFileName, fileName.GetFullPath());
#else
      backupResult = NavObj_dB::GetInstance().Backup(fileName.GetFullPath());
#endif
    }
    return backupResult ? wxID_YES : wxID_NO;
  }
  return wxID_ABORT;  // Cancelled the file open dialog, aborting
}

bool ExportGPXRoutes(wxWindow *parent, RouteList *pRoutes,
                     const wxString suggestedName) {
#ifndef __ANDROID__
  wxFileName fn = exportFileName(parent, suggestedName);
  if (fn.IsOk()) {
    NavObjectCollection1 *pgpx = new NavObjectCollection1;
    pgpx->AddGPXRoutesList(pRoutes);
    pgpx->SaveFile(fn.GetFullPath());
    delete pgpx;
    return true;
  }
#else
  // Create the .GPX file, saving it in the OCPN Android cache directory
  wxString fns = androidGetCacheDir() + wxFileName::GetPathSeparator() +
                 suggestedName + ".gpx";
  NavObjectCollection1 *pgpx = new NavObjectCollection1;
  pgpx->AddGPXRoutesList(pRoutes);
  pgpx->SaveFile(fns);
  delete pgpx;

  // Kick off the Android file chooser activity
  wxString path;
  int response = g_Platform->DoFileSelectorDialog(
      parent, &path, _("Export GPX file"), g_gpx_path, suggestedName + ".gpx",
      "*.gpx");

  if (path.IsEmpty())  // relocation handled by SAF logic in Java
    return true;

  wxCopyFile(fns, path);  // known to be safe paths, since SAF is not involved.
  return true;

#endif

  return false;
}

bool ExportGPXTracks(wxWindow *parent, std::vector<Track *> *pTracks,
                     const wxString suggestedName) {
#ifndef __ANDROID__
  wxFileName fn = exportFileName(parent, suggestedName);
  if (fn.IsOk()) {
    NavObjectCollection1 *pgpx = new NavObjectCollection1;
    pgpx->AddGPXTracksList(pTracks);
    pgpx->SaveFile(fn.GetFullPath());
    delete pgpx;
    return true;
  }
#else
  // Create the .GPX file, saving it in the OCPN Android cache directory
  wxString fns = androidGetCacheDir() + wxFileName::GetPathSeparator() +
                 suggestedName + ".gpx";
  NavObjectCollection1 *pgpx = new NavObjectCollection1;
  pgpx->AddGPXTracksList(pTracks);
  pgpx->SaveFile(fns);
  delete pgpx;

  // Kick off the Android file chooser activity
  wxString path;
  int response = g_Platform->DoFileSelectorDialog(
      parent, &path, _("Export GPX file"), g_gpx_path, suggestedName + ".gpx",
      "*.gpx");

  if (path.IsEmpty())  // relocation handled by SAF logic in Java
    return true;

  wxCopyFile(fns, path);  // known to be safe paths, since SAF is not involved.
  return true;
#endif

  return false;
}

bool ExportGPXWaypoints(wxWindow *parent, RoutePointList *pRoutePoints,
                        const wxString suggestedName) {
#ifndef __ANDROID__
  wxFileName fn = exportFileName(parent, suggestedName);
  if (fn.IsOk()) {
    NavObjectCollection1 *pgpx = new NavObjectCollection1;
    pgpx->AddGPXPointsList(pRoutePoints);
    pgpx->SaveFile(fn.GetFullPath());
    delete pgpx;
    return true;
  }
#else
  // Create the .GPX file, saving it in the OCPN Android cache directory
  wxString fns = androidGetCacheDir() + wxFileName::GetPathSeparator() +
                 suggestedName + ".gpx";
  NavObjectCollection1 *pgpx = new NavObjectCollection1;
  pgpx->AddGPXPointsList(pRoutePoints);
  pgpx->SaveFile(fns);
  delete pgpx;

  // Kick off the Android file chooser activity
  wxString path;
  int response = g_Platform->DoFileSelectorDialog(
      parent, &path, _("Export GPX file"), g_gpx_path, suggestedName + ".gpx",
      "*.gpx");

  if (path.IsEmpty())  // relocation handled by SAF logic in Java
    return true;

  wxCopyFile(fns, path);  // known to be safe paths, since SAF is not involved.
  return true;

#endif

  return false;
}

void ExportGPX(wxWindow *parent, bool bviz_only, bool blayer) {
  NavObjectCollection1 *pgpx = new NavObjectCollection1;
  wxString fns;

#ifndef __ANDROID__
  wxFileName fn = exportFileName(parent, "userobjects.gpx");
  if (!fn.IsOk()) return;
  fns = fn.GetFullPath();
#else
  // Create the .GPX file, saving it in the OCPN Android cache directory
  fns =
      androidGetCacheDir() + wxFileName::GetPathSeparator() + "userobjects.gpx";

#endif
  ::wxBeginBusyCursor();

  wxGenericProgressDialog *pprog = nullptr;
  int count = pWayPointMan->GetWaypointList()->size();
  int progStep = count / 32;
  if (count > 200) {
    pprog = new wxGenericProgressDialog(
        _("Export GPX file"), "0/0", count, NULL,
        wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME |
            wxPD_REMAINING_TIME);
    pprog->SetSize(400, wxDefaultCoord);
    pprog->Centre();
  }

  // WPTs
  int ic = 1;

  for (RoutePoint *pr : *pWayPointMan->GetWaypointList()) {
    if (pprog && !(ic % progStep)) {
      wxString msg;
      msg.Printf("%d/%d", ic, count);
      pprog->Update(ic, msg);
    }
    ic++;

    bool b_add = true;
    if (bviz_only && !pr->m_bIsVisible) b_add = false;

    if (pr->m_bIsInLayer && !blayer) b_add = false;
    if (b_add) {
      if (pr->IsShared() || !WptIsInRouteList(pr)) pgpx->AddGPXWaypoint(pr);
    }
  }
  // RTEs and TRKs
  for (Route *pRoute : *pRouteList) {
    bool b_add = true;
    if (bviz_only && !pRoute->IsVisible()) b_add = false;
    if (pRoute->m_bIsInLayer && !blayer) b_add = false;

    if (b_add) pgpx->AddGPXRoute(pRoute);
  }

  for (Track *pTrack : g_TrackList) {
    bool b_add = true;

    if (bviz_only && !pTrack->IsVisible()) b_add = false;

    if (pTrack->m_bIsInLayer && !blayer) b_add = false;

    if (b_add) pgpx->AddGPXTrack(pTrack);
  }

  pgpx->SaveFile(fns);

#ifdef __ANDROID__
  // Kick off the Android file chooser activity
  wxString path;
  int response =
      g_Platform->DoFileSelectorDialog(parent, &path, _("Export GPX file"),
                                       g_gpx_path, "userobjects.gpx", "*.gpx");
  if (path.IsEmpty())  // relocation handled by SAF logic in Java
    return;

  wxCopyFile(fns, path);  // known to be safe paths, since SAF is not involved.
  return;
#endif
  delete pgpx;
  ::wxEndBusyCursor();
  delete pprog;
}

void UI_ImportGPX(wxWindow *parent, bool islayer, wxString dirpath,
                  bool isdirectory, bool isPersistent) {
  int response = wxID_CANCEL;
  wxArrayString file_array;

  if (!islayer || dirpath.IsSameAs("")) {
    //  Platform DoFileSelectorDialog method does not properly handle multiple
    //  selections So use native method if not Android, which means Android gets
    //  single selection only.
#ifndef __ANDROID__
    wxFileDialog *popenDialog =
        new wxFileDialog(NULL, _("Import GPX file"), g_gpx_path, "",
                         "GPX files (*.gpx)|*.gpx|All files (*.*)|*.*",
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
#else  // Android
    wxString path;
    response = g_Platform->DoFileSelectorDialog(
        NULL, &path, _("Import GPX file"), g_gpx_path, "", "*.gpx");

    wxFileName fn(path);
    g_gpx_path = fn.GetPath();
    if (path.IsEmpty()) {  // Return from SAF processing, expecting callback
      PrepareImportAndroid(islayer, isPersistent);
      return;
    } else
      file_array.Add(path);  // Return from safe app arena access

#endif
  } else {
    if (isdirectory) {
      if (wxDir::GetAllFiles(dirpath, &file_array, "*.gpx")) response = wxID_OK;
    } else {
      file_array.Add(dirpath);
      response = wxID_OK;
    }
  }

  if (response == wxID_OK) {
    ImportFileArray(file_array, islayer, isPersistent, dirpath);
  }
}

void ImportFileArray(const wxArrayString &file_array, bool islayer,
                     bool isPersistent, wxString dirpath) {
  Layer *l = NULL;

  if (islayer) {
    l = new Layer();
    l->m_LayerID = ++g_LayerIdx;
    l->m_LayerFileName = file_array[0];
    if (file_array.GetCount() <= 1)
      wxFileName::SplitPath(file_array[0], NULL, NULL, &(l->m_LayerName), NULL,
                            NULL);
    else {
      if (dirpath.IsSameAs(""))
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
    laymsg.Printf("New layer %d: %s", l->m_LayerID, l->m_LayerName.c_str());
    wxLogMessage(laymsg);

    pLayerList->insert(pLayerList->begin(), l);
  }

  for (unsigned int i = 0; i < file_array.GetCount(); i++) {
    wxString path = file_array[i];

    if (::wxFileExists(path)) {
      NavObjectCollection1 *pSet = new NavObjectCollection1;
      if (pSet->load_file(path.fn_str()).status !=
          pugi::xml_parse_status::status_ok) {
        wxLogMessage("Error loading GPX file " + path);
        pSet->reset();
        delete pSet;
        continue;
      }

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
          destf.Append("layers");
          appendOSDirSlash(&destf);
          if (!wxDirExists(destf)) {
            if (!wxMkdir(destf, wxS_DIR_DEFAULT))
              wxLogMessage("Error creating layer directory");
          }

          destf << name << "." << ext;
          wxString msg;
          if (wxCopyFile(f, destf, true))
            msg.Printf("File: %s.%s also added to persistent layers", name,
                       ext);
          else
            msg.Printf("Failed adding %s.%s to persistent layers", name, ext);
          wxLogMessage(msg);
        }
      } else {
        int wpt_dups;
        pSet->LoadAllGPXObjects(
            !pSet->IsOpenCPN(),
            wpt_dups);  // Import with full visibility of names and objects
#ifndef __ANDROID__
        if (wpt_dups > 0) {
          OCPNMessageBox(
              NULL,
              wxString::Format("%d " + _("duplicate waypoints detected "
                                         "during import and ignored."),
                               wpt_dups),
              _("OpenCPN Info"), wxICON_INFORMATION | wxOK, 10);
        }
#endif
      }
      delete pSet;
    }
  }
}

//-------------------------------------------------------------------------
//           Static Routine Switch to Inland Ecdis Mode
//-------------------------------------------------------------------------
void SwitchInlandEcdisMode(bool Switch) {
  if (Switch) {
    wxLogMessage("Switch InlandEcdis mode On");
    LoadS57();
    // Overrule some settings to comply with InlandEcdis
    // g_toolbarConfig = ".....XXXX.X...XX.XXXXXXXXXXXX";
    g_iDistanceFormat = 2;  // 0 = "Nautical miles"), 1 = "Statute miles", 2 =
                            // "Kilometers", 3 = "Meters"
    g_iSpeedFormat = 2;     // 0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"
    if (ps52plib) ps52plib->SetDisplayCategory(STANDARD);
    g_bDrawAISSize = false;
    if (gFrame) gFrame->RequestNewToolbars(true);
  } else {
    wxLogMessage("Switch InlandEcdis mode Off");
    // reread the settings overruled by inlandEcdis
    if (pConfig) {
      pConfig->SetPath("/Settings");
      pConfig->Read("GlobalToolbarConfig", &g_toolbarConfig);
      pConfig->Read("DistanceFormat", &g_iDistanceFormat);
      pConfig->Read("SpeedFormat", &g_iSpeedFormat);
      pConfig->Read("ShowDepthUnits", &g_bShowDepthUnits, 1);
      pConfig->Read("HeightFormat", &g_iHeightFormat);
      int read_int;
      pConfig->Read("nDisplayCategory", &read_int, (enum _DisCat)STANDARD);
      if (ps52plib) ps52plib->SetDisplayCategory((enum _DisCat)read_int);
      pConfig->SetPath("/Settings/AIS");
      pConfig->Read("bDrawAISSize", &g_bDrawAISSize);
      pConfig->Read("bDrawAISRealtime", &g_bDrawAISRealtime);
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
  //      return dt.Format("%Y-%m-%dT%TZ", wxDateTime::GMT0);
  return dt.Format("%Y-%m-%dT%H:%M:%SZ");
}

/**************************************************************************/
/*          LogMessageOnce                                                */
/**************************************************************************/

bool LogMessageOnce(const wxString &msg) {
  //    Search the array for a match

  for (unsigned int i = 0; i < navutil::pMessageOnceArray->GetCount(); i++) {
    if (msg.IsSameAs(navutil::pMessageOnceArray->Item(i))) return false;
  }

  // Not found, so add to the array
  navutil::pMessageOnceArray->Add(msg);

  //    And print it
  wxLogMessage(msg);
  return true;
}

/**************************************************************************/
/*          Some assorted utilities                                       */
/**************************************************************************/

wxDateTime toUsrDateTime(const wxDateTime ts, const int format,
                         const double lon) {
  if (!ts.IsValid()) {
    return ts;
  }
  int effective_format = format;
  if (effective_format == GLOBAL_SETTINGS_INPUT) {
    if (::g_datetime_format == "UTC") {
      effective_format = UTCINPUT;
    } else if (::g_datetime_format == "LMT") {
      effective_format = LMTINPUT;
    } else if (::g_datetime_format == "Local Time") {
      effective_format = LTINPUT;
    } else {
      // Default to UTC
      effective_format = UTCINPUT;
    }
  }
  wxDateTime dt;
  switch (effective_format) {
    case LMTINPUT:  // LMT@Location
      if (std::isnan(lon)) {
        dt = wxInvalidDateTime;
      } else {
        dt =
            ts.Add(wxTimeSpan(wxTimeSpan(0, 0, wxLongLong(lon * 3600. / 15.))));
      }
      break;
    case LTINPUT:  // Local@PC
      // Convert date/time from UTC to local time.
      dt = ts.FromUTC();
      break;
    case UTCINPUT:  // UTC
      // The date/time is already in UTC.
      dt = ts;
      break;
  }
  return dt;
}

wxDateTime fromUsrDateTime(const wxDateTime ts, const int format,
                           const double lon) {
  if (!ts.IsValid()) {
    return ts;
  }
  int effective_format = format;
  if (effective_format == GLOBAL_SETTINGS_INPUT) {
    if (::g_datetime_format == "UTC") {
      effective_format = UTCINPUT;
    } else if (::g_datetime_format == "LMT") {
      effective_format = LMTINPUT;
    } else if (::g_datetime_format == "Local Time") {
      effective_format = LTINPUT;
    } else {
      // Default to UTC
      effective_format = UTCINPUT;
    }
  }
  wxDateTime dt;
  switch (effective_format) {
    case LMTINPUT:  // LMT@Location
      if (std::isnan(lon)) {
        dt = wxInvalidDateTime;
      } else {
        dt = ts.Subtract(wxTimeSpan(0, 0, wxLongLong(lon * 3600. / 15.)));
      }
      break;
    case LTINPUT:  // Local@PC
      // The input date/time is in local time, so convert it to UTC.
      dt = ts.ToUTC();
      break;
    case UTCINPUT:  // UTC
      dt = ts;
      break;
  }
  return dt;
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
    out.Printf("%03.0f %cT (%.0f %cM)", angle, 0x00B0, toMagnetic(angle),
               0x00B0);
  } else if (g_bShowTrue) {
    out.Printf("%03.0f %cT", angle, 0x00B0);
  } else {
    out.Printf("%03.0f %cM", toMagnetic(angle), 0x00B0);
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

  if (wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOW).Red() <
      128) {
    // Dark system color themes usually do better job than we do on diming UI
    // controls, do not fight with them
    return;
  }

  if (NULL == ctrl) return;

  wxColour col, window_back_color, gridline, uitext, udkrd, ctrl_back_color,
      text_color;
  col = GetGlobalColor("DILG0");                // Dialog Background white
  window_back_color = GetGlobalColor("DILG1");  // Dialog Background
  ctrl_back_color = GetGlobalColor("DILG1");    // Control Background
  text_color = GetGlobalColor("DILG3");         // Text
  uitext = GetGlobalColor("UITX1");             // Menu Text, derived from UINFF
  udkrd = GetGlobalColor("UDKRD");
  gridline = GetGlobalColor("GREY2");

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

    if (dynamic_cast<wxListBox *>(win) || dynamic_cast<wxListCtrl *>(win) ||
        dynamic_cast<wxTextCtrl *>(win) ||
        dynamic_cast<wxTimePickerCtrl *>(win)) {
      win->SetBackgroundColour(col);
    } else if (dynamic_cast<wxStaticText *>(win) ||
               dynamic_cast<wxCheckBox *>(win) ||
               dynamic_cast<wxRadioButton *>(win)) {
      win->SetForegroundColour(uitext);
    }
#ifndef __WXOSX__
    // On macOS most controls can't be styled, and trying to do so only creates
    // weird coloured boxes around them. Fortunately, however, many of them
    // inherit a colour or tint from the background of their parent.

    else if (dynamic_cast<wxBitmapComboBox *>(win) ||
             dynamic_cast<wxChoice *>(win) || dynamic_cast<wxComboBox *>(win) ||
             dynamic_cast<wxTreeCtrl *>(win)) {
      win->SetBackgroundColour(col);
    }

    else if (dynamic_cast<wxScrolledWindow *>(win) ||
             dynamic_cast<wxGenericDirCtrl *>(win) ||
             dynamic_cast<wxListbook *>(win) || dynamic_cast<wxButton *>(win) ||
             dynamic_cast<wxToggleButton *>(win)) {
      win->SetBackgroundColour(window_back_color);
    }

    else if (dynamic_cast<wxNotebook *>(win)) {
      win->SetBackgroundColour(window_back_color);
      win->SetForegroundColour(text_color);
    }
#endif

    else if (dynamic_cast<wxHtmlWindow *>(win)) {
      if (cs != GLOBAL_COLOR_SCHEME_DAY && cs != GLOBAL_COLOR_SCHEME_RGB)
        win->SetBackgroundColour(ctrl_back_color);
      else
        win->SetBackgroundColour(wxNullColour);
    }

    else if (dynamic_cast<wxGrid *>(win)) {
      dynamic_cast<wxGrid *>(win)->SetDefaultCellBackgroundColour(
          window_back_color);
      dynamic_cast<wxGrid *>(win)->SetDefaultCellTextColour(uitext);
      dynamic_cast<wxGrid *>(win)->SetLabelBackgroundColour(col);
      dynamic_cast<wxGrid *>(win)->SetLabelTextColour(uitext);
      dynamic_cast<wxGrid *>(win)->SetGridLineColour(gridline);
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
