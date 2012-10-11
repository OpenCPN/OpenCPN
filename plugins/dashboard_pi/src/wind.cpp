/******************************************************************************
 * $Id: wind.cpp, v1.0 2010/08/05 SethDart Exp $
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

#include "wind.h"

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

// Display the arrow for MainValue (wind angle)
// We also want the extra value (wind speed) displayed inside the dial

DashboardInstrument_Wind::DashboardInstrument_Wind( wxWindow *parent, wxWindowID id, wxString title, int cap_flag) :
      DashboardInstrument_Dial( parent, id, title, cap_flag, 0, 360, 0, 360)
{
      SetOptionMarker(10, DIAL_MARKER_REDGREENBAR, 3);
      // Labels are set static because we've no logic to display them this way
      wxString labels[] = {_T(""), _T("30"), _T("60"), _T("90"), _T("120"), _T("150"), _T(""), _T("150"), _T("120"), _T("90"), _T("60"), _T("30")};
      SetOptionLabel(30, DIAL_LABEL_HORIZONTAL, wxArrayString(12, labels));
}

void DashboardInstrument_Wind::DrawBackground(wxGCDC* dc)
{
    DrawBoat( dc, m_cx, m_cy, m_radius );
}

DashboardInstrument_WindCompass::DashboardInstrument_WindCompass( wxWindow *parent, wxWindowID id, wxString title, int cap_flag ) :
      DashboardInstrument_Dial( parent, id, title, cap_flag, 0, 360, 0, 360 )
{
      SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
      wxString labels[] = {_("N"), _("NE"), _("E"), _("SE"), _("S"), _("SW"), _("W"), _("NW")};
      SetOptionLabel(45, DIAL_LABEL_HORIZONTAL, wxArrayString(8, labels));
}

void DashboardInstrument_WindCompass::DrawBackground(wxGCDC* dc)
{
      DrawCompassRose(dc, m_cx, m_cy, m_radius * 0.85, m_AngleStart, false);
}

// Display the arrow for MainValue (wind angle)
// We also want the extra value (wind speed) displayed inside the dial

DashboardInstrument_TrueWindAngle::DashboardInstrument_TrueWindAngle( wxWindow *parent, wxWindowID id, wxString title, int cap_flag) :
      DashboardInstrument_Dial( parent, id, title, cap_flag, 0, 360, 0, 360)
{
      SetOptionMarker(10, DIAL_MARKER_REDGREENBAR, 3);
      // Labels are set static because we've no logic to display them this way
      wxString labels[] = {_T(""), _T("30"), _T("60"), _T("90"), _T("120"), _T("150"), _T(""), _T("150"), _T("120"), _T("90"), _T("60"), _T("30")};
      SetOptionLabel(30, DIAL_LABEL_HORIZONTAL, wxArrayString(12, labels));
}

void DashboardInstrument_TrueWindAngle::DrawBackground(wxGCDC* dc)
{
    DrawBoat( dc, m_cx, m_cy, m_radius );
}

