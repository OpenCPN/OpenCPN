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

bool g_bAllowShipToActive = false;
bool g_benableUDPNullHeader = false;
bool g_bGarminHostUpload = false;
bool g_bTrackDaily = false;
bool g_bWplUsePosition = false;
bool g_persist_active_route = false;
bool g_btouch = false;

double g_n_arrival_circle_radius = 0.0;
double g_PlanSpeed = 0.0;
double g_UserVar = 0.0;

float g_GLMinSymbolLineWidth;
float g_selection_radius_mm = 2.0;
float g_selection_radius_touch_mm = 10.0;

int g_iDistanceFormat = 0;
int g_iSDMMFormat = 0;
int g_iSpeedFormat = 0;
int g_iWindSpeedFormat = 0;
int g_iTempFormat = 0;
int g_maxWPNameLength;
int g_NMEAAPBPrecision = 3;
int g_nCOMPortCheck = 32;
int g_nDepthUnitDisplay = 0;
int g_nNMEADebug = 0;
int g_route_line_width = 0;
int g_track_line_width = 0;
int g_WplAction = 0;
int gps_watchdog_timeout_ticks = 0;
int sat_watchdog_timeout_ticks = 12;

wxString g_active_route;
wxString g_catalog_channel;
wxString g_catalog_custom_url;
wxString g_compatOS = PKG_TARGET;
wxString g_compatOsVersion = PKG_TARGET_VERSION;
wxString g_default_routepoint_icon;
wxString g_GPS_Ident;
wxString g_hostname;
wxString g_SART_sound_file;
wxString g_TalkerIdText;
wxString g_winPluginDir;

static wxConfigBase* the_base_config = 0;

wxConfigBase* TheBaseConfig() {
  wxASSERT_MSG(the_base_config != 0, "Uninitialized the_base_config");
  return the_base_config;
}
void InitBaseConfig(wxConfigBase* cfg) { the_base_config = cfg; }
