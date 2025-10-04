/***************************************************************************
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 * Variables maintained by comm stack, read-only access for others
 */

#ifndef COMM_VARS_H_
#define COMM_VARS_H_

#include <wx/string.h>

extern wxString gRmcDate;
extern wxString gRmcTime;

extern int g_priSats;
extern int g_SatsInView;

extern bool g_bVAR_Rx;

/**
 * Indicates valid GNSS reception status based on satellite visibility
 * and successful parsing of NMEA0183, SignalK, or NMEA2000 data.
 * Reset to false if no valid signal is received within watchdog timeout period.
 */
extern bool g_bSatValid;
extern wxString g_ownshipMMSI_SK;
#endif  // COMM_VARS_H__
