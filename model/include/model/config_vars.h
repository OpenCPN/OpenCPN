/******************************************************************************
 *
 * Project: OpenCPN
 * Purpose: Variables defined in config file, command line etc.
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
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

#ifndef CONFIG_VARS_H__
#define CONFIG_VARS_H__

#include <wx/config.h>
#include <wx/string.h>
extern bool g_bAISRolloverShowClass;
extern bool g_bAISRolloverShowCOG;
extern bool g_bAISRolloverShowCPA;
extern bool g_bAllowShipToActive;
extern bool g_benableUDPNullHeader;
extern bool g_bfilter_cogsog;
extern bool g_bGarminHostUpload;
extern bool g_bInlandEcdis;
extern bool g_bMagneticAPB;
extern bool g_bOverruleScaMin;
extern bool g_bShowMag;
extern bool g_bShowTrue;
extern bool g_bShowWptName;
extern bool g_btouch;
extern bool g_bTrackDaily;
extern bool g_bUserIconsFirst;
extern bool g_bUseWptScaMin;
extern bool g_bWplUsePosition;
extern bool g_enable_root_menu_debug;
extern bool g_persist_active_route;

/**
 * Always send RMB and RMC n0183 messages even if there is no active
 * route. One use case is the Nasa Marine instruments Clipper GPS
 */
extern bool g_always_send_rmb_rmc;

extern double g_n_arrival_circle_radius;
extern double g_PlanSpeed;
extern double g_TrackDeltaDistance;
extern double g_UserVar;

extern float g_fWaypointRangeRingsStep;
extern float g_GLMinSymbolLineWidth;
extern float g_selection_radius_mm;
extern float g_selection_radius_touch_mm;

extern int g_COGFilterSec;
extern int g_NMEAAPBPrecision;
extern int g_SOGFilterSec;
extern int g_WplAction;
extern int g_iDistanceFormat;
extern int g_iSDMMFormat;
extern int g_iSpeedFormat;
extern int g_iTempFormat;
extern int g_iWaypointRangeRingsNumber;
extern int g_iWaypointRangeRingsStepUnits;
extern int g_iWindSpeedFormat;
extern int g_iWpt_ScaMin;
extern int g_maxWPNameLength;
extern int g_mbtilesMaxLayers;
extern int g_nCOMPortCheck;
extern int g_nDepthUnitDisplay;
extern int g_netmask_bits;
extern int g_nNMEADebug;
extern int g_nTrackPrecision;
extern int g_route_line_width;
extern int g_trackFilterMax;
extern int g_track_line_width;
extern int gps_watchdog_timeout_ticks;
extern int sat_watchdog_timeout_ticks;

extern wxString g_active_route;
extern wxString g_android_Device_Model;
extern wxString g_catalog_channel;
extern wxString g_catalog_custom_url;
extern wxString g_compatOS;
extern wxString g_compatOsVersion;
extern wxString g_default_routepoint_icon;
extern wxString g_GPS_Ident;
extern wxString g_hostname;
extern wxString g_SART_sound_file;
extern wxString g_TalkerIdText;
extern wxString g_winPluginDir;  // Base plugin directory on Windows.
/**
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
extern wxString g_datetime_format;

wxConfigBase* TheBaseConfig();
void InitBaseConfig(wxConfigBase* cfg);

#endif  // CONFIG_VARS_H__
