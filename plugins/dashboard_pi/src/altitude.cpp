/******************************************************************************
 * $Id: altitude.cpp, v0.1 $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin, display altitude trace
 * Author:   derived from Jean-Eudes Onfray's depth.cpp by Andreas Merz
 *
 * Comment:  since not every vessel is always on sea level, I found it
 *           sometimes intersting to observe the GPS altitude information.
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

#include "altitude.h"
#include "wx28compat.h"
extern int g_iDashDepthUnit;   // use same unit for altitude

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

DashboardInstrument_Altitude::DashboardInstrument_Altitude(wxWindow* parent,
                                                     wxWindowID id,
                                                     wxString title)
    : DashboardInstrument(parent, id, title, OCPN_DBP_STC_ALTI) {
  m_cap_flag.set(OCPN_DBP_STC_TMP);
  m_MaxAltitude = 0;
  m_Altitude = 0;
  m_AltitudeUnit = getUsrDistanceUnit_Plugin(g_iDashDepthUnit);
  m_Temp = _T("--");
  for (int idx = 0; idx < ALTITUDE_RECORD_COUNT; idx++) {
    m_ArrayAltitude[idx] = 0.0;
  }
}

wxSize DashboardInstrument_Altitude::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
  if (orient == wxHORIZONTAL) {
    return wxSize(DefaultWidth, wxMax(m_TitleHeight + 140, hint.y));
  } else {
    return wxSize(wxMax(hint.x, DefaultWidth), m_TitleHeight + 140);
  }
}

void DashboardInstrument_Altitude::SetData(DASH_CAP st, double data,
                                        wxString unit) {
  if (st == OCPN_DBP_STC_ALTI) {
    m_Altitude = std::isnan(data) ? 0.0 : data;

    //printf("Altitude = %3.3f\n", m_Altitude); // debug output
    for (int idx = 1; idx < ALTITUDE_RECORD_COUNT; idx++) {
      m_ArrayAltitude[idx - 1] = m_ArrayAltitude[idx];      // shift FIFO
    }
    m_ArrayAltitude[ALTITUDE_RECORD_COUNT - 1] = m_Altitude;
    m_AltitudeUnit = unit;
  } else if (st == OCPN_DBP_STC_ATMP) {
    if (!std::isnan(data)) {
      m_Temp = wxString::Format(_T("%.1f"), data) + DEGREE_SIGN + unit;
    } else {
      m_Temp = "---";
    }
  }
}

void DashboardInstrument_Altitude::Draw(wxGCDC* dc) {
  DrawBackground(dc);
  DrawForeground(dc);
}

void DashboardInstrument_Altitude::DrawBackground(wxGCDC* dc) {
  wxSize size = GetClientSize();
  wxColour cl;

  GetGlobalColor(_T("DASHL"), &cl);
  dc->SetTextForeground(cl);

  wxPen pen;
  pen.SetStyle(wxPENSTYLE_SOLID);
  GetGlobalColor(_T("DASHF"), &cl);
  pen.SetColour(cl);
  pen.SetWidth(1);
  dc->SetPen(pen);

  //dc->DrawLine(3,  44, size.x - 3,  44);
  dc->DrawLine(3, 140, size.x - 3, 140);  // Base line


#ifdef __WXMSW__
  pen.SetStyle(wxPENSTYLE_SHORT_DASH);
#else
  pen.SetStyle(wxPENSTYLE_DOT);
  pen.SetWidth(1);
#endif

  // Grid lines
  dc->SetPen(pen);
  dc->DrawLine(3,  44, size.x - 3,  44);
  dc->DrawLine(3,  68, size.x - 3,  68);
  dc->DrawLine(3,  92, size.x - 3,  92);
  dc->DrawLine(3, 116, size.x - 3, 116);

  dc->SetFont(*g_pFontSmall);

  double MaxAltitude =   -9999.0;
  double MinAltitude = 9999999.0;
  for (int idx = 0; idx < ALTITUDE_RECORD_COUNT; idx++) {
    if (m_ArrayAltitude[idx] > MaxAltitude) MaxAltitude = m_ArrayAltitude[idx];
    if (m_ArrayAltitude[idx] < MinAltitude) MinAltitude = m_ArrayAltitude[idx];
  }
  // limit zooming, the 4 grid lines will be spaced 1m at least
  if( MaxAltitude-MinAltitude < 4.0 ) {
    MinAltitude -= 1.0;
    MaxAltitude = MinAltitude + 4.0;
  }
  // only update axes on major changes
  if( MaxAltitude - m_MaxAltitude >  0.6 || 
      MaxAltitude - m_MaxAltitude < -4.0 || 
      MinAltitude - m_MinAltitude < -1.0 ||
      MinAltitude - m_MinAltitude >  4.0 ) {
    m_MaxAltitude = round(MaxAltitude);
    m_MinAltitude = round(MinAltitude);
  }
  
  wxString label;
  label.Printf(_T("%.0f ") + m_AltitudeUnit, m_MaxAltitude);
  int width, height;
  dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
  dc->DrawText(label, size.x - width - 1, 40 - height);

  label.Printf(_T("%.0f ") + m_AltitudeUnit, m_MinAltitude);
  dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
  dc->DrawText(label, size.x - width - 1, size.y - height);
}

void DashboardInstrument_Altitude::DrawForeground(wxGCDC* dc) {
  wxSize size = GetClientSize();
  wxColour cl;

  GetGlobalColor(_T("DASH1"), &cl);
  wxBrush brush;
  brush.SetStyle(wxBRUSHSTYLE_SOLID);
  brush.SetColour(cl);
  dc->SetBrush(brush);
  dc->SetPen(*wxTRANSPARENT_PEN);

  double ratioH = m_MaxAltitude-m_MinAltitude < 1.0 ? 96.0 : 96.0 / (m_MaxAltitude-m_MinAltitude);  // 140-44=96
  double ratioW = double(size.x - 6) / (ALTITUDE_RECORD_COUNT - 1);
  wxPoint points[ALTITUDE_RECORD_COUNT + 2];
#ifdef __OCPN__ANDROID__
  int px = 3;
  points[0].x = px;
  points[0].y = 140;

  for (int idx = 0; idx < ALTITUDE_RECORD_COUNT - 1; idx++) {
    points[1].x = points[0].x;
    if (m_ArrayAltitude[idx])
      points[1].y = 140 - ( m_ArrayAltitude[idx] - m_MinAltitude) * ratioH ;
    else
      points[1].y = 140;

    points[2].x = points[1].x + ratioW;
    if (m_ArrayAltitude[idx + 1])
      points[2].y = 140 - ( m_ArrayAltitude[idx + 1]- m_MinAltitude) * ratioH;
    else
      points[2].y = 140;

    points[3].x = points[2].x;
    points[3].y = 140;
    dc->DrawPolygon(4, points);

    points[0].x = points[2].x;
    points[0].y = 140;
  }

#else
  for (int idx = 0; idx < ALTITUDE_RECORD_COUNT; idx++) {
    points[idx].x = idx * ratioW + 3;
    points[idx].y = 140 -(m_ArrayAltitude[idx]-m_MinAltitude) * ratioH;
  }
  points[ALTITUDE_RECORD_COUNT].x = size.x - 3;
  points[ALTITUDE_RECORD_COUNT].y = 140;
  points[ALTITUDE_RECORD_COUNT + 1].x = 3;
  points[ALTITUDE_RECORD_COUNT + 1].y = 140;
  dc->DrawPolygon(ALTITUDE_RECORD_COUNT + 2, points);
#endif

  GetGlobalColor(_T("DASHF"), &cl);
  dc->SetTextForeground(cl);
  dc->SetFont(*g_pFontData);
  if (m_AltitudeUnit != _T("-")) {  // Watchdog
    wxString s_alti = wxString::Format(_T("%.2f"), m_Altitude);
    // We want only one decimal but for security not rounded up.
    s_alti = s_alti.Mid(0, s_alti.length() - 1);
    dc->DrawText(s_alti + _T(" ") + m_AltitudeUnit, 10, m_TitleHeight);
  } else
    dc->DrawText(_T("---"), 10, m_TitleHeight);

  // TODO: test display air temperature ID_DBP_I_ATMP
  dc->SetFont(*g_pFontLabel);
  int width, height;
  dc->GetTextExtent(m_Temp, &width, &height, 0, 0, g_pFontLabel);
  dc->DrawText(m_Temp, 0, size.y - height);
}
