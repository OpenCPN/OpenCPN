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


extern bool g_bGarminHostUpload;
extern bool g_bWplUsePosition;

extern double g_UserVar;

extern int g_maxWPNameLength;
extern int g_nNMEADebug;
extern int gps_watchdog_timeout_ticks;
extern int sat_watchdog_timeout_ticks;

extern wxString g_GPS_Ident;
extern wxString g_hostname;

wxConfigBase* TheBaseConfig();
void InitBaseConfig(wxConfigBase* cfg);

#endif  // CONFIG_VARS_H__
