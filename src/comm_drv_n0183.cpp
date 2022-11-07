/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement the comm_drv_n0183.h -- Nmea0183 driver base
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "comm_drv_n0183.h"

CommDriverN0183::CommDriverN0183() : AbstractCommDriver(NavAddr::Bus::N0183) {}

CommDriverN0183::CommDriverN0183(NavAddr::Bus b, const std::string& s)
    : AbstractCommDriver(NavAddr::Bus::N0183, s) {}

CommDriverN0183::~CommDriverN0183() {}

void CommDriverN0183::Activate() {}
