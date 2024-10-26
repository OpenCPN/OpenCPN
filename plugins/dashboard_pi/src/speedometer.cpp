/******************************************************************************
 * $Id: speedometer.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 */

#include "speedometer.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

// Not much to do here most of the default dial values are fine.
// Note the default AngleStart = 225 and AngleRange = 270 set here.

DashboardInstrument_Speedometer::DashboardInstrument_Speedometer(
    wxWindow* parent, wxWindowID id, wxString title,
    InstrumentProperties* Properties, DASH_CAP cap_flag, int s_value,
    int e_value)
    : DashboardInstrument_Dial(parent, id, title, Properties, cap_flag, 225,
                               270, s_value, e_value) {
  // We want the main value displayed inside the dial as well
  // as the default arrow
  SetOptionMainValue(_T("%.1f"), DIAL_POSITION_INSIDE);
}
