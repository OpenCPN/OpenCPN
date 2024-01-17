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

bool g_persist_active_route;
bool g_bGarminHostUpload;
bool g_bWplUsePosition;

double g_UserVar = 0.0;
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
int gps_watchdog_timeout_ticks = 0;
int sat_watchdog_timeout_ticks = 12;

int g_iCOMPort = 0;
int g_iBaudrate = 0;
int g_iParity = 0;
int g_iData_bits = 0;
int g_iStopbits = 0;

wxString g_active_route;

wxString g_GPS_Ident;
wxString g_hostname;
wxString g_TalkerIdText;
wxString g_winPluginDir;

static wxConfigBase* the_base_config = 0;

wxConfigBase* TheBaseConfig() {
  wxASSERT_MSG(the_base_config != 0, "Uninitialized the_base_config");
  return the_base_config;
}
void InitBaseConfig(wxConfigBase* cfg) { the_base_config = cfg; }
