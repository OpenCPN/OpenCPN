/***************************************************************************
 *   Copyright (C) 2023 - 2025 Alec Leamas                                 *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Global variables stored in configuration file.
 *
 * Note: please keep variable definitions at one single line, extended
 * docs goes to the bottom. This is to make it possible to maintain the
 * sorted list.
 *
 * That is, please keep list sorted.
 */

#ifndef CONFIG_VARS_H__
#define CONFIG_VARS_H__

#include <limits>
#include <string>
#include <vector>

#include <wx/config.h>
#include <wx/string.h>

#undef max  // get rid of windows define blocking ::max

constexpr unsigned kUndefinedColor =
    std::numeric_limits<unsigned>::max();  // Undefined wxColour RGB

extern bool g_allow_arb_system_plugin;
extern bool g_always_send_rmb_rmc;  // See extended docs below
extern bool g_bAISRolloverShowClass;
extern bool g_bAISRolloverShowCOG;
extern bool g_bAISRolloverShowCPA;
extern bool g_bAllowShipToActive;
extern bool g_bAutoAnchorMar;
extern bool g_bAutoAnchorMark;
extern bool g_bAutoHideToolbar;
extern bool g_bBasicMenus;
extern bool g_bConfirmObjectDelete;
extern bool g_bCourseUp;
extern bool g_bDebugCM93;
extern bool g_bDebugGPSD;
extern bool g_bDebugOGL;
extern bool g_bDebugS57;
extern bool g_bDisplayGrid;  ///< Should  lat/lon grid be displayed ?
extern bool g_bEmailCrashReport;
extern bool g_benable_rotate;
extern bool g_benableUDPNullHeader;
extern bool g_bEnableZoomToCursor;
extern bool g_bfilter_cogsog;
extern bool g_bframemax;
extern bool g_bFullscreen;
extern bool g_bFullScreenQuilt;
extern bool g_bFullscreenToolbar;
extern bool g_bGarminHostUpload;
extern bool g_bGDAL_Debug;
extern bool g_bGLexpert;
extern bool g_bHighliteTracks;
extern bool g_bInlandEcdis;
extern bool g_bLookAhead;
extern bool g_bMagneticAPB;
extern bool g_bNavAidRadarRingsShown;
extern bool g_bopengl;
extern bool g_bOverruleScaMin;
extern bool g_bPermanentMOBIcon;
extern bool g_bPlayShipsBell;
extern bool g_bPlayShipsBells;
extern bool g_bPreserveScaleOnX;
extern bool g_bQuiltEnable;
extern bool g_bQuiltStart;  // Not used
extern bool g_bresponsive;
extern bool g_bRollover;  ///<  Enable/disable mouse rollover GUI effects
extern bool g_bShowActiveRouteHighway;
extern bool g_bShowChartBar;
extern bool g_bShowCompassWin;
extern bool g_bShowCurrent;
extern bool g_bShowDepthUnits;
extern bool g_bShowDetailSlider;
extern bool g_bShowLayers;
extern bool g_bShowLiveETA;
extern bool g_bShowMag;
extern bool g_bShowMenuBar;
extern bool g_bShowMuiZoomButtons;
extern bool g_bShowOutlines;
extern bool g_bShowRouteTotal;
extern bool g_bShowShipToActive;
extern bool g_bShowStatusBar;
extern bool g_bShowTide;  ///< not used
extern bool g_bShowTrackPointTime;
extern bool g_bShowTrue;
extern bool g_bShowWptName;
extern bool g_bsimplifiedScalebar;
extern bool g_bskew_comp;
extern bool g_bsmoothpanzoom;
extern bool g_bsmoothpanzoom;  // see extended docs below
extern bool g_bSoftwareGL;
extern bool g_bSpaceDropMark;
extern bool g_btenhertz;
extern bool g_btouch;
extern bool g_btrackContinuous;
extern bool g_bTrackDaily;
extern bool g_bTransparentToolbar;
extern bool g_bUIexpert;
extern bool g_bUseGLL;
extern bool g_bUseGreenShip;
extern bool g_bUseOnlyConfirmedAISName;
extern bool g_bUserIconsFirst;
extern bool g_bUseWptScaMin;
extern bool g_bWayPointPreventDragging;
extern bool g_bWplUsePosition;
extern bool g_config_display_size_manual;
extern bool g_declutter_anchorage;
extern bool g_enable_root_menu_debug;
extern bool g_fog_overzoom;
extern bool g_oz_vector_scale;
extern bool g_persist_active_route;
extern bool g_useMUI;
extern bool s_bSetSystemTime;
extern bool g_kiosk_startup;

extern double g_COGAvg;  ///< Debug only usage
extern double g_defaultBoatSpeed;
extern double g_display_size_mm;  ///< Physical display width (mm)
extern double g_mouse_zoom_sensitivity;
extern double g_n_arrival_circle_radius;
extern double g_n_gps_antenna_offset_x;
extern double g_n_gps_antenna_offset_y;
extern double g_n_ownship_beam_meters;
extern double g_n_ownship_length_meters;
extern double g_overzoom_emphasis_base;
extern double g_ownship_HDTpredictor_miles;
extern double g_ownship_predictor_minutes;
extern double g_PlanSpeed;
extern double g_plus_minus_zoom_factor;
extern double g_TrackDeltaDistance;
extern double g_TrackIntervalSeconds;
extern double g_UserVar;

extern float g_fNavAidRadarRingsStep;
extern float g_fWaypointRangeRingsStep;
extern float g_GLMinSymbolLineWidth;
extern float g_MarkScaleFactorExp;
extern float g_selection_radius_mm;
extern float g_selection_radius_touch_mm;
extern float g_ShipScaleFactorExp;

extern int g_ais_cog_predictor_width;
extern int g_AndroidVersionCode;
extern int g_BSBImgDebug;
extern int g_ChartScaleFactor;
extern int g_chart_zoom_modifier_raster;
extern int g_chart_zoom_modifier_vector;
extern int g_cm93_zoom_factor;
extern int g_COGAvgSec;  ///< COG average period for Course Up Mode (sec)
extern int g_COGFilterSec;
extern int g_cog_predictor_endmarker;
extern int g_cog_predictor_style;
extern int g_cog_predictor_width;
extern int g_default_font_size;
extern int g_detailslider_dialog_x;
extern int g_detailslider_dialog_y;
extern int g_ENCSoundingScaleFactor;
extern int g_ENCTextScaleFactor;
extern int g_GUIScaleFactor;

extern int g_iDistanceFormat;  // see extended docs below
extern int g_iENCToolbarPosX;
extern int g_iENCToolbarPosY;
extern int g_iHeightFormat;  // see extended docs below
extern int g_iNavAidRadarRingsNumberVisible;
extern int g_iSDMMFormat;
extern int g_iSoundDeviceIndex;

extern int g_iSpeedFormat;  // See extended docs below
extern int g_iTempFormat;   // See extended docs below
extern int g_iWaypointRangeRingsNumber;
extern int g_iWaypointRangeRingsStepUnits;
extern int g_iWindSpeedFormat;
extern int g_iWpt_ScaMin;
extern int g_iWpt_ScaMax;
extern int g_lastClientRecth;
extern int g_lastClientRectw;
extern int g_lastClientRectx;
extern int g_lastClientRecty;
extern int g_maintoolbar_x;
extern int g_maintoolbar_y;
extern int g_maxWPNameLength;
extern int g_maxzoomin;
extern int g_mbtilesMaxLayers;
extern int g_memCacheLimit;
extern int g_MemFootMB;
extern int g_mouse_zoom_sensitivity_ui;
extern int g_nAutoHideToolbar;
extern int g_navobjbackups;
extern int g_nAWDefault;
extern int g_nAWMax;
extern int g_nbrightness;
extern int g_nCacheLimit;
extern int g_nCOMPortCheck;
extern int g_nCPUCount;
extern int g_nDepthUnitDisplay;  // see extended docs below
extern int g_netmask_bits;
extern int g_nframewin_posx;
extern int g_nframewin_posy;
extern int g_nframewin_x;
extern int g_nframewin_y;
extern int g_NMEAAPBPrecision;
extern int g_nNMEADebug;
extern int g_n_ownship_min_mm;
extern int g_nTrackPrecision;
extern int g_ownship_HDTpredictor_endmarker;
extern int g_ownship_HDTpredictor_style;
extern int g_ownship_HDTpredictor_width;
extern int g_OwnShipIconType;
extern int g_pNavAidRadarRingsStepUnits;
extern int gps_watchdog_timeout_ticks;
extern int g_restore_dbindex;
extern int g_restore_stackindex;
extern int g_route_line_width;
extern int g_route_prop_sx;
extern int g_route_prop_sy;
extern int g_route_prop_x;
extern int g_route_prop_y;
extern int g_S57_dialog_sx;
extern int g_S57_dialog_sy;
extern int g_S57_extradialog_sx;
extern int g_S57_extradialog_sy;
extern int g_SENC_LOD_pixels;
extern int g_ShipScaleFactor;
extern int g_shipToActiveColor;
extern int g_shipToActiveStyle;
extern int g_SkewCompUpdatePeriod;
extern int g_SOGFilterSec;
extern int g_tcwin_scale;
extern int g_trackFilterMax;
extern int g_track_line_width;
extern int g_WplAction;
extern int n_NavMessageShown;
extern int sat_watchdog_timeout_ticks;
extern int g_AIS_alert_delay;

extern long g_maintoolbar_orient;

extern std::vector<std::string> TideCurrentDataSet;

extern unsigned g_canvasConfig;
extern unsigned g_dm_dropped;
extern unsigned g_dm_filtered;
extern unsigned g_dm_input;
extern unsigned g_dm_not_ok;
extern unsigned g_dm_ok;
extern unsigned g_dm_output;

extern wxString g_active_route;
extern wxString g_android_Device_Model;
extern wxString g_AW1GUID;
extern wxString g_AW2GUID;
extern wxString g_catalog_channel;
extern wxString g_catalog_custom_url;
extern wxString g_CmdSoundString;
extern wxString g_cog_predictor_color;
extern wxString g_compatOS;
extern wxString g_compatOsVersion;
extern wxString g_config_version_string;
extern wxString g_datetime_format;
extern wxString g_datetime_format;        // See extended docs below
extern wxString g_default_font_facename;  // See extended docs below
extern wxString g_default_routepoint_icon;
extern wxString g_default_wp_icon;
extern wxString g_dm_logfile;  ///< Last Data Monitor log file
extern wxString g_GPS_Ident;
extern wxString g_gpx_path;
extern wxString g_hostname;
extern wxString g_InvisibleLayer;
extern wxString g_InvisibleLayers;
extern wxString g_InVisiNameinLayers;
extern wxString g_lastAppliedTemplateGUID;
extern wxString g_locale;
extern wxString g_localeOverride;
extern wxString g_ObjQFileExt;
extern wxString g_ownship_HDTpredictor_color;
extern wxString g_SART_sound_file;
extern wxString g_TalkerIdText;
extern wxString g_TCData_Dir;
extern wxString g_toolbarConfig;
extern wxString g_uiStyle;  // Not used
extern wxString g_uploadConnection;
extern wxString g_UserPresLibData;
extern wxString g_VisibleLayers;
extern wxString g_VisiNameinLayers;
extern wxString g_winPluginDir;  ///< Base plugin directory on Windows.
extern wxString gWorldMapLocation;
extern wxString gWorldShapefileLocation;

/** Size of pysical screen in millimeters. */
extern std::vector<size_t> g_config_display_size_mm;

wxConfigBase* TheBaseConfig();
void InitBaseConfig(wxConfigBase* cfg);

/**
 * \var g_iHeightFormat
 *
 * User-selected height (vertical, above reference datum) unit format for
 * display and input. Values correspond to HEIGHT_* enum (e.g., meters, feet).
 * Used for tide levels, bridge clearances, and other height displays.
 */

/**
 * \var g_iTempFormat
 *
 * User-selected temperature unit format for display and input.
 * Values correspond to TEMPERATURE_* enum (e.g., Celsius, Fahrenheit, Kelvin).
 * Used for weather overlays, tide info, and other temperature displays.
 */

/**
 * \var g_iDistanceFormat
 *
 * User-selected distance (horizontal) unit format for display and input.
 * Values correspond to DISTANCE_* enum (e.g., NMi, mi, km, m, ft, yd, etc.).
 */

/**
 * \var g_iSpeedFormat
 *
 * User-selected speed unit format for display and input.
 * Values correspond to SPEED_* enum (e.g., knots, mph, km/h, m/s).
 * Used for ownship speed, route planning, and other speed displays.
 */

/**
 * \var g_nDepthUnitDisplay
 *
 * User-selected depth (below surface) unit format for display and input.
 * Values correspond to DEPTH_* enum (e.g., meters, feet, fathoms).
 * Used for chart soundings, depth contours, and other depth displays.
 */

/**
 * \var g_bsmoothpanzoom
 *
 * Controls how the chart panning and zooming smoothing is done during user
 * interactions.
 *
 * When enabled (true):
 * - Chart panning has inertia, with smooth acceleration and deceleration
 * - Mouse wheel zooming is smoothly animated between zoom levels
 * - Chart overscaled rendering is optimized for smooth transitions
 * - Chart quilting transitions may be smoother
 *
 * When disabled (false):
 * - Chart panning stops immediately when mouse is released
 * - Mouse wheel zooming jumps directly between zoom levels without animation
 * - Rendering may be slightly faster but less visually polished
 */

/**
 * \var g_default_font_facename
 *
 * Default font size for user interface elements such as menus, dialogs, etc.
 *
 * @note Defaults to 0, which falls back to the system default font size.
 * @note Currently only set on the Android platform.
 * @note Can be manually overridden in opencpn.ini (DefaultFontSize parameter).
 */

/**
 * \var g_bsmoothpanzoom
 *
 * Controls how the chart panning and zooming smoothing is done during user
 * interactions.
 *
 * When enabled (true):
 * - Chart panning has inertia, with smooth acceleration and deceleration
 * - Mouse wheel zooming is smoothly animated between zoom levels
 * - Chart overscaled rendering is optimized for smooth transitions
 * - Chart quilting transitions may be smoother
 *
 * When disabled (false):
 * - Chart panning stops immediately when mouse is released
 * - Mouse wheel zooming jumps directly between zoom levels without animation
 * - Rendering may be slightly faster but less visually polished
 */

/**
 * \var g_always_send_rmb_rmc
 *
 * Always send RMB and RMC n0183 messages even if there is no active
 * route. One use case is the Nasa Marine instruments Clipper GPS
 */

/**
 * \var g_datetime_format
 *
 * Date/time format to use when formatting date/time strings.
 * This is a global setting that affects all date/time formatting in OpenCPN.
 *
 * @details Supported values are:
 * - "UTC": Format date/time in Coordinated Universal Time (UTC).
 * - "Local Time": Format date/time using the operating system timezone
 * configuration.
 *
 * @note Future support could potentially include:
 * - "LMT": Format date/time using the solar mean time at a given location.
 * - Valid IANA TZ name: Format date/time using the specified timezone.
 *   This could be useful when planning a route in a timezone other than
 *   what is configured in the operating system.
 *   See https://en.wikipedia.org/wiki/List_of_tz_database_time_zones for valid
 *   names.
 * - Custom date/time format: Allow the user to specify a custom date/time
 * format string.
 *
 * @note This configuration parameter stores the English name without
 * translation. Widgets may provide a localized version of this value when
 * displaying it to the user.
 */

#endif  // CONFIG_VARS_H__
