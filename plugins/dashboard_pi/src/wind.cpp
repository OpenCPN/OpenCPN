/******************************************************************************
 * $Id: wind.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  DashBoard Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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

DashboardInstrument_Wind::DashboardInstrument_Wind( wxWindow *parent, wxWindowID id, wxString title) :
      DashboardInstrument_Dial( parent, id, title, 0, 360, 0, 360)
{
      //SetOptionMainValue(_T("%3.0f Deg"), DIAL_POSITION_BOTTOMLEFT);
      SetOptionMarker(10, DIAL_MARKER_REDGREEN, 3);
      // Labels are set static because we've no logic to display them this way
      wxString labels[] = {_T(""), _("30"), _("60"), _("90"), _("120"), _("150"), _T(""), _("150"), _("120"), _("90"), _("60"), _("30")};
      SetOptionLabel(30, DIAL_LABEL_HORIZONTAL, wxArrayString(12, labels));
      SetOptionExtraValue(_T("%5.2f Kts"), DIAL_POSITION_INSIDE);
}

void DashboardInstrument_Wind::DrawBackground(wxPaintDC* dc)
{
      wxPoint points[5];

      points[0].x = m_cx + m_radius * .25;
      points[0].y = m_cy + m_radius * .2;
      points[1].x = m_cx + m_radius * .25;
      points[1].y = m_cy - m_radius * .3;
      points[2].x = m_cx;
      points[2].y = m_cy - m_radius;
      points[3].x = m_cx - m_radius * .25;
      points[3].y = m_cy - m_radius * .3;
      points[4].x = m_cx - m_radius * .25;
      points[4].y = m_cy + m_radius * .2;
      dc->DrawLines(5, points);
}

