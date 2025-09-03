/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
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
 */
#include "model/config_vars.h"
#include "config.h"

bool g_always_send_rmb_rmc = false;
bool g_bAISRolloverShowClass = false;
bool g_bAISRolloverShowCOG = false;
bool g_bAISRolloverShowCPA = false;
bool g_bAllowShipToActive = false;
bool g_bBasicMenus = false;
bool g_bConfirmObjectDelete = false;
bool g_bDebugCM93 = false;
bool g_benableUDPNullHeader = false;
bool g_bfilter_cogsog = false;
bool g_bGarminHostUpload = false;
bool g_bInlandEcdis = false;
bool g_bMagneticAPB = false;
bool g_bOverruleScaMin = false;
bool g_bresponsive = false;
bool g_bShowMag = false;
bool g_bShowTrue = false;
bool g_bShowWptName = false;
bool g_bskew_comp = false;
bool g_btouch = false;
bool g_bTrackDaily = false;
bool g_bUserIconsFirst = true;
bool g_bUseWptScaMin = false;
bool g_bWplUsePosition = false;
bool g_enable_root_menu_debug = false;
bool g_persist_active_route = false;
bool g_bsimplifiedScalebar = false;
bool g_bShowMenuBar = false;
bool g_bShowCompassWin = false;
bool g_bWayPointPreventDragging = false;
bool g_bEnableZoomToCursor = false;
bool g_bShowChartBar = false;
bool g_bShowShipToActive = false;
bool g_bopengl = false;
bool g_bFullScreenQuilt = true;
bool g_bsmoothpanzoom = false;
bool g_bDebugOGL = false;
bool g_benable_rotate = false;
bool g_bRollover = false;  ///<  enable/disable mouse rollover GUI effects
bool g_bSpaceDropMark = false;
bool g_bAutoHideToolbar = false;
bool g_bShowTrackPointTime = false;
bool g_bShowLiveETA = false;
bool g_btenhertz = false;
bool g_useMUI = false;
bool g_bPreserveScaleOnX = false;

double g_mouse_zoom_sensitivity = 0.0;
double g_n_arrival_circle_radius = 0.0;
double g_PlanSpeed = 0.0;
double g_TrackDeltaDistance = 0.0;
double g_UserVar = 0.0;
double g_plus_minus_zoom_factor = 0.0;
double g_n_ownship_length_meters = 0.0;
double g_n_ownship_beam_meters = 0.0;
double g_n_gps_antenna_offset_y = 0.0;
double g_n_gps_antenna_offset_x = 0.0;
double g_ownship_predictor_minutes = 0.0;
double g_ownship_HDTpredictor_miles = 0.0;
double g_COGAvg = 0.0;
double g_display_size_mm = 0.0;
double g_defaultBoatSpeed = 0.0;

float g_fWaypointRangeRingsStep = 0.0;
float g_GLMinSymbolLineWidth = 0.0;
float g_selection_radius_mm = 2.0;
float g_selection_radius_touch_mm = 10.0;
float g_ShipScaleFactorExp = 0.0;
float g_fNavAidRadarRingsStep = 0.0;

int g_detailslider_dialog_x = 0;
int g_detailslider_dialog_y = 0;
int g_cm93_zoom_factor = 0;
int g_COGFilterSec = 1;
int g_iDistanceFormat = 0;
int g_iSDMMFormat = 0;
int g_iSpeedFormat = 0;
int g_iTempFormat = 0;
int g_iWaypointRangeRingsNumber = 0;
int g_iWaypointRangeRingsStepUnits = 0;
int g_iWindSpeedFormat = 0;
int g_iWpt_ScaMin = 0;
int g_maxWPNameLength;
int g_mbtilesMaxLayers = 2;
int g_nAWDefault = 0;
int g_nAWMax = 0;
int g_nCOMPortCheck = 32;
int g_nDepthUnitDisplay = 0;
int g_netmask_bits = 24;
int g_NMEAAPBPrecision = 3;
int g_nbrightness = 100;
int g_nNMEADebug = 0;
int g_nTrackPrecision = 0;
int gps_watchdog_timeout_ticks = 0;
int g_route_line_width = 0;
int g_SOGFilterSec = 0;
int g_trackFilterMax = 0;
int g_track_line_width = 0;
int g_WplAction = 0;
int sat_watchdog_timeout_ticks = 12;
int g_nCacheLimit = 0;
int g_memCacheLimit = 0;
int g_iNavAidRadarRingsNumberVisible = 0;
int g_pNavAidRadarRingsStepUnits = 0;
int g_ENCSoundingScaleFactor = 0;
int g_ENCTextScaleFactor = 0;
int g_maxzoomin = 0;
int g_shipToActiveStyle = 0;
int g_shipToActiveColor = 0;
int g_S57_dialog_sx = 0;
int g_S57_dialog_sy = 0;
int g_OwnShipIconType = 0;
int g_n_ownship_min_mm = 0;
int g_cog_predictor_style = 0;
int g_cog_predictor_endmarker = 0;
int g_ownship_HDTpredictor_style = 0;
int g_ownship_HDTpredictor_endmarker = 0;
int g_ownship_HDTpredictor_width = 0;
int g_chart_zoom_modifier_raster = 0;
int g_chart_zoom_modifier_vector = 0;
int g_ChartScaleFactor = 0;
int g_cog_predictor_width = 0;
int g_COGAvgSec = 15;
int g_GUIScaleFactor = 1;

unsigned g_canvasConfig = 0;

wxString g_active_route;
wxString g_AW1GUID;
wxString g_AW2GUID;
wxString g_catalog_channel;
wxString g_catalog_custom_url;
wxString g_compatOS = PKG_TARGET;
wxString g_compatOsVersion = PKG_TARGET_VERSION;
wxString g_default_routepoint_icon;
wxString g_default_wp_icon;
wxString g_GPS_Ident;
wxString g_hostname;
wxString g_SART_sound_file;
wxString g_TalkerIdText;
wxString g_winPluginDir;
wxString g_datetime_format;
wxString gWorldMapLocation;
wxString gWorldShapefileLocation;
wxString g_uploadConnection;
wxString g_cog_predictor_color;
wxString g_ownship_HDTpredictor_color;
wxString g_CmdSoundString;
wxString g_ObjQFileExt;

static wxConfigBase* the_base_config = 0;

wxConfigBase* TheBaseConfig() {
  wxASSERT_MSG(the_base_config != 0, "Uninitialized the_base_config");
  return the_base_config;
}
void InitBaseConfig(wxConfigBase* cfg) { the_base_config = cfg; }
