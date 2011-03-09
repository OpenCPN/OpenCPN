/******************************************************************************
 * $Id: wind.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
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

DashboardInstrument_Wind::DashboardInstrument_Wind( wxWindow *parent, wxWindowID id, wxString title, int cap_flag) :
      DashboardInstrument_Dial( parent, id, title, cap_flag, 0, 360, 0, 360)
{
      //SetOptionMainValue(_T("%3.0f Deg"), DIAL_POSITION_BOTTOMLEFT);
      SetOptionMarker(10, DIAL_MARKER_REDGREENBAR, 3);
      // Labels are set static because we've no logic to display them this way
      wxString labels[] = {_T(""), _T("30"), _T("60"), _T("90"), _T("120"), _T("150"), _T(""), _T("150"), _T("120"), _T("90"), _T("60"), _T("30")};
      SetOptionLabel(30, DIAL_LABEL_HORIZONTAL, wxArrayString(12, labels));

      SetInstrumentWidth(200);
}

void DashboardInstrument_Wind::DrawBackground(wxBufferedDC* dc)
{
/*
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
*/
      wxCoord x = m_cx - (m_radius * 0.3);
      wxCoord y = m_cy - (m_radius * 0.6);
      dc->DrawEllipticArc(x, y, m_radius * 0.6, m_radius * 1.4, 0, 180);
}

DashboardInstrument_WindCompass::DashboardInstrument_WindCompass( wxWindow *parent, wxWindowID id, wxString title, int cap_flag ) :
      DashboardInstrument_Dial( parent, id, title, cap_flag, 0, 360, 0, 360 )
{
      SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
      wxString labels[] = {_("N"), _("NE"), _("E"), _("SE"), _("S"), _("SW"), _("W"), _("NW")};
      SetOptionLabel(45, DIAL_LABEL_HORIZONTAL, wxArrayString(8, labels));

      SetInstrumentWidth(200);
}

void DashboardInstrument_WindCompass::DrawBackground(wxBufferedDC* dc)
{
      wxPoint points[3];
      int tmpradius = m_radius * 0.85;

      wxColour cl;
      wxPen pen;
      pen.SetStyle(wxSOLID);
      GetGlobalColor(_T("BLUE1"), &cl);
      pen.SetColour(cl);
      dc->SetPen(pen);
      dc->SetTextForeground(cl);

      int offset = 0;
      for(double tmpangle = m_AngleStart - ANGLE_OFFSET;
                        tmpangle <= m_AngleStart + 360 - ANGLE_OFFSET; tmpangle+=45)
      {
            dc->SetBrush(*wxTRANSPARENT_BRUSH);
            points[0].x = m_cx;
            points[0].y = m_cy;
            points[1].x = m_cx + tmpradius * 0.1 * cos(deg2rad(tmpangle-45));
            points[1].y = m_cy + tmpradius * 0.1 * sin(deg2rad(tmpangle-45));
            double size = (offset % 2 ? 0.50 : 0.80);
            points[2].x = m_cx + tmpradius * size * cos(deg2rad(tmpangle));
            points[2].y = m_cy + tmpradius * size * sin(deg2rad(tmpangle));
            dc->DrawPolygon(3, points, 0, 0);

            points[1].x = m_cx + tmpradius * 0.1 * cos(deg2rad(tmpangle+45));
            points[1].y = m_cy + tmpradius * 0.1 * sin(deg2rad(tmpangle+45));
            dc->SetBrush(cl);
            dc->DrawPolygon(3, points, 0, 0);
            offset++;
      }
}

