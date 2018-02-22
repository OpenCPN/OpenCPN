/******************************************************************************
 * $Id: compass.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 *           (Inspired by original work from Andreas Heiming)
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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

#include "compass.h"

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

DashboardInstrument_Compass::DashboardInstrument_Compass( wxWindow *parent, wxWindowID id, wxString title, int cap_flag) :
      DashboardInstrument_Dial( parent, id, title, cap_flag, 0, 360, 0, 360)
{
      SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
      SetOptionLabel(20, DIAL_LABEL_ROTATED);
      SetOptionMainValue(_T("%5.0f Deg"), DIAL_POSITION_TOPRIGHT);

      SetInstrumentWidth(200);
}

void DashboardInstrument_Compass::SetData(int st, double data, wxString unit)
{
      if (st == m_MainValueCap)
      {
            // Rotate the rose
            m_AngleStart = -data;
            // Required to display data
            m_MainValue = data;

            Refresh(false);
      }
      else if (st == m_ExtraValueCap)
      {
            m_ExtraValue = data;
            Refresh(false);
      }
}

void DashboardInstrument_Compass::DrawBackground(wxBufferedDC* dc)
{
      wxPen pen;

//      wxBrush brushHatch(*wxLIGHT_GREY, wxTRANSPARENT);

      // Now draw the boat
      wxColour cl;
      GetGlobalColor(_T("GREY1"), &cl);
      pen.SetStyle(wxSOLID);
      pen.SetColour(cl);
      dc->SetPen(pen);
      GetGlobalColor(_T("GREY2"), &cl);
      dc->SetBrush(cl);

      wxPoint points[7];

/*
 *           0
 *          /\
 *         /  \
 *        /    \
 *     6 /      \ 1
 *      |   X    |
 *      |        |
 *    5 |        | 2
 *       \      /
 *        \__ _/
 *        4    3
 */
      points[0].x = m_cx;
      points[0].y = m_cy - m_radius * .50; // a little bit longer than compass rose
      points[1].x = m_cx + m_radius * .15;
      points[1].y = m_cy;
      points[2].x = m_cx + m_radius * .15;
      points[2].y = m_cy + m_radius * .20;
      points[3].x = m_cx + m_radius * .10;
      points[3].y = m_cy + m_radius * .40;
      points[4].x = m_cx - m_radius * .10;
      points[4].y = m_cy + m_radius * .40;
      points[5].x = m_cx - m_radius * .15;
      points[5].y = m_cy + m_radius * .20;
      points[6].x = m_cx - m_radius * .15;
      points[6].y = m_cy;

      dc->DrawPolygon(7, points, 0, 0);

      DrawCompassRose(dc);
}

void DashboardInstrument_Compass::DrawCompassRose(wxBufferedDC* dc)
{
      wxPoint TextPoint, points[3];
      wxString Value;
      int width, height;
      wxString CompassArray[] = {_("N"),_("NE"),_("E"),_("SE"),_("S"),_("SW"),_("W"),_("NW"),_("N")};

      int tmpradius = m_radius * 0.75;

      dc->SetFont(*g_pFontSmall);

      wxColour cl;
      wxPen pen;
      pen.SetStyle(wxSOLID);
      GetGlobalColor(_T("BLUE1"), &cl);
      pen.SetColour(cl);
      dc->SetPen(pen);
      dc->SetTextForeground(cl);
      //dc->SetPen(*wxTRANSPARENT_PEN);

      int offset = 0;
      for(double tmpangle = m_AngleStart - ANGLE_OFFSET;
                        tmpangle <= m_AngleStart + 360 - ANGLE_OFFSET; tmpangle+=45)
      {
            Value = CompassArray[offset];
            dc->GetTextExtent(Value, &width, &height, 0, 0, g_pFontSmall);
            double x = width/2;
            long double anglefortext = asin((x/tmpradius));
            anglefortext = tmpangle - rad2deg(anglefortext);
            TextPoint.x = m_cx + tmpradius * cos(deg2rad(anglefortext));
            TextPoint.y = m_cy + tmpradius * sin(deg2rad(anglefortext));
            dc->DrawRotatedText(Value, TextPoint.x,
                                                TextPoint.y, -90 - tmpangle);

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

void DashboardInstrument_Compass::DrawForeground(wxBufferedDC* dc)
{
      // We dont want the default foreground (arrow) drawn
}
