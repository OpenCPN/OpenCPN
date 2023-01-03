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

#include "config_vars.h"

bool g_bGarminHostUpload;
bool g_bWplUsePosition;

double g_UserVar = 0.0;

int g_maxWPNameLength;
int g_nNMEADebug = 0;
int gps_watchdog_timeout_ticks = 0;
int sat_watchdog_timeout_ticks = 12;

wxString g_GPS_Ident;
wxString g_hostname;

static wxConfigBase* the_base_config = 0;

wxConfigBase* TheBaseConfig() {
  wxASSERT_MSG(the_base_config != 0, "Uninitialized the_base_config");
  return the_base_config;
}
void InitConfigBase(wxConfigBase* cfg) { the_base_config = cfg; }
