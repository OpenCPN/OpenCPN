/******************************************************************************
 *
 * Project: OpenCPN
 * Purpose: Variables maintained by comm stack, read-only access for others
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef COMM_VARS_H__
#define COMM_VARS_H__

#include <wx/string.h>

extern wxString gRmcDate;
extern wxString gRmcTime;

extern int g_priSats;
extern int g_SatsInView;

extern bool g_bVAR_Rx;
extern bool g_bSatValid;
extern wxString g_ownshipMMSI_SK;
#endif  // COMM_VARS_H__
