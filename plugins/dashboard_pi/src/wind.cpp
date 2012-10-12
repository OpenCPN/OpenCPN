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

// Display the arrow for MainValue (wind angle)
// We also want the extra value (wind speed) displayed inside the dial

DashboardInstrument_TrueWindAngle::DashboardInstrument_TrueWindAngle( wxWindow *parent, wxWindowID id, wxString title, int cap_flag) :
      DashboardInstrument_Dial( parent, id, title, cap_flag, 0, 360, 0, 360)
{
      m_unit=_T("");
      SetOptionMarker(10, DIAL_MARKER_REDGREENBAR, 3);
      // Labels are set static because we've no logic to display them this way
      wxString labels[] = {_T(""), _T("30"), _T("60"), _T("90"), _T("120"), _T("150"), _T(""), _T("150"), _T("120"), _T("90"), _T("60"), _T("30")};
      SetOptionLabel(30, DIAL_LABEL_HORIZONTAL, wxArrayString(12, labels));

      SetInstrumentWidth(200);
}

void DashboardInstrument_TrueWindAngle::DrawBackground(wxBufferedDC* dc)
{
      wxCoord x = m_cx - (m_radius * 0.3);
      wxCoord y = m_cy - (m_radius * 0.6);
      dc->DrawEllipticArc(x, y, m_radius * 0.6, m_radius * 1.4, 0, 180);
}
void DashboardInstrument_TrueWindAngle::DrawForeground(wxBufferedDC* dc)
{
      // The default foreground is the arrow used in most dials
      wxColour cl;
      GetGlobalColor(_T("GREY1"), &cl);
      wxPen pen1;
      pen1.SetStyle(wxSOLID);
      pen1.SetColour(cl);
      pen1.SetWidth(2);
      dc->SetPen(pen1);
      GetGlobalColor(_T("GREY2"), &cl);
      wxBrush brush1;
      brush1.SetStyle(wxSOLID);
      brush1.SetColour(cl);
      dc->SetBrush(brush1);
      dc->DrawCircle(m_cx, m_cy, m_radius / 8);

      dc->SetPen(*wxTRANSPARENT_PEN);

      //GetGlobalColor(_T("BLUE1"), &cl);
      wxBrush brush;
      brush.SetStyle(wxSOLID);
      brush.SetColour(wxColour(255,145,0));
      dc->SetBrush(brush);
	  //this is fix for a +/-180° round instrument, when m_MainValue is supplied as <0..180><L | R>, in this case the "True wind angle"
	  //do it here, because otherwise m_MainValueOption is incorrect !!!
	  double data;
	  if(m_unit == _T("DegL"))	//specially for instrument OCPN_DBP_STC_VWT
		  data=360-m_MainValue;
	  else
		  data=m_MainValue;
	        // The arrow should stay inside fixed limits
      double val;
      if (data < m_MainValueMin) val = m_MainValueMin;
      else if (data > m_MainValueMax) val = m_MainValueMax;
      else val = data;

      double value = deg2rad((val - m_MainValueMin) * m_AngleRange / (m_MainValueMax - m_MainValueMin)) + deg2rad(m_AngleStart - ANGLE_OFFSET);

      wxPoint points[3];
      points[0].x = m_cx + (m_radius * 0.95 * cos(value));
      points[0].y = m_cy + (m_radius * 0.95 * sin(value));
      points[1].x = m_cx + (m_radius * 0.22 * cos(value + 160));
      points[1].y = m_cy + (m_radius * 0.22 * sin(value + 160));
      points[2].x = m_cx + (m_radius * 0.22 * cos(value - 160));
      points[2].y = m_cy + (m_radius * 0.22 * sin(value - 160));
      dc->DrawPolygon(3, points, 0, 0);
}
void DashboardInstrument_TrueWindAngle::SetData(int st, double data, wxString unit)
{
      if (st == m_MainValueCap)
            m_MainValue = data;
      else if (st == m_ExtraValueCap)
            m_ExtraValue = data;
      m_unit=unit;

      Refresh(false);
}

