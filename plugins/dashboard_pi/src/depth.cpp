/******************************************************************************
 * $Id: depth.cpp, v1.0 2010/08/30 SethDart Exp $
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


#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include "depth.h"
extern int g_iDashDepthUnit;

#ifdef __BORLANDC__
#pragma hdrstop
#endif

int m_DataHeight;
int w_label, h_label, m_plotdown, m_plotup, m_plotheight;

DashboardInstrument_Depth::DashboardInstrument_Depth(wxWindow* parent,
                                                     wxWindowID id,
                                                     wxString title)
    : DashboardInstrument(parent, id, title, OCPN_DBP_STC_DPT) {
  m_cap_flag.set(OCPN_DBP_STC_TMP);
  m_MaxDepth = 0;
  m_Depth = 0;
  m_DepthUnit = getUsrDistanceUnit_Plugin(g_iDashDepthUnit);
  m_Temp = _T("--");
  for (int idx = 0; idx < DEPTH_RECORD_COUNT; idx++) {
    m_ArrayDepth[idx] = 0;
  }
}

wxSize DashboardInstrument_Depth::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
  dc.GetTextExtent("15.7 Feet", &w, &m_DataHeight, 0, 0, g_pFontData);
  // Space for bottom(temp)text later.
  dc.GetTextExtent("20.8 C", &w_label, &h_label, 0, 0, g_pFontLabel);

  int y_total =
      //  Title         Depth data       plot area       w-temp
      m_TitleHeight + m_DataHeight + 4 * m_DataHeight + h_label;
  if (orient == wxHORIZONTAL) {
    return wxSize( DefaultWidth, wxMax(y_total, hint.y));
  } else {
    return wxSize(wxMax(hint.x, DefaultWidth), y_total);
  }
}

void DashboardInstrument_Depth::SetData(DASH_CAP st, double data,
                                        wxString unit) {
  if (st == OCPN_DBP_STC_DPT) {
    m_Depth = std::isnan(data) ? 0.0 : data;

    for (int idx = 1; idx < DEPTH_RECORD_COUNT; idx++) {
      m_ArrayDepth[idx - 1] = m_ArrayDepth[idx];
    }
    m_ArrayDepth[DEPTH_RECORD_COUNT - 1] = m_Depth;
    m_DepthUnit = unit;
  } else if (st == OCPN_DBP_STC_TMP) {
    if (!std::isnan(data)) {
      m_Temp = wxString::Format(_T("%.1f"), data) + DEGREE_SIGN + unit;
    } else {
      m_Temp = "---";
    }
  }
}

void DashboardInstrument_Depth::Draw(wxGCDC* dc) {
  DrawBackground(dc);
  DrawForeground(dc);
}

void DashboardInstrument_Depth::DrawBackground(wxGCDC* dc) {
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

  m_plotup = m_TitleHeight + m_DataHeight;
  m_plotdown = size.y - h_label;
  m_plotheight = m_plotdown - m_plotup;

  dc->DrawLine(3, m_plotup, size.x - 3, m_plotup);
  dc->DrawLine(3, m_plotdown, size.x - 3, m_plotdown);

#ifdef __WXMSW__
  pen.SetStyle(wxPENSTYLE_SHORT_DASH);
#else
  pen.SetStyle(wxPENSTYLE_DOT);
  pen.SetWidth(1);
#endif

  dc->SetPen(pen);
  dc->DrawLine(3, m_plotup + m_plotheight / 4,
               size.x - 3, m_plotup + m_plotheight / 4);
  dc->DrawLine(3, m_plotup + m_plotheight * 2 / 4,
               size.x - 3, m_plotup + m_plotheight * 2 / 4);
  dc->DrawLine(3, m_plotup + m_plotheight * 3 / 4,
               size.x - 3, m_plotup + m_plotheight * 3 / 4);

  dc->SetFont(*g_pFontSmall);

  m_MaxDepth = 0;
  for (int idx = 0; idx < DEPTH_RECORD_COUNT; idx++) {
    if (m_ArrayDepth[idx] > m_MaxDepth) m_MaxDepth = m_ArrayDepth[idx];
  }
  // Increase MaxDepth slightly for nicer display
  m_MaxDepth *= 1.2;

  wxString label;
  label.Printf(_T("%.0f ") + m_DepthUnit, 0.0);
  int width, height;
  dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
  dc->DrawText(label, size.x - width - 1, m_plotup - height);

  label.Printf(_T("%.0f ") + m_DepthUnit, m_MaxDepth);
  dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
  dc->DrawText(label, size.x - width - 1, m_plotdown);
}

void DashboardInstrument_Depth::DrawForeground(wxGCDC* dc) {
  wxSize size = GetClientSize();
  wxColour cl;

  GetGlobalColor(_T("DASHL"), &cl);
  wxBrush brush;
  brush.SetStyle(wxBRUSHSTYLE_SOLID);
  brush.SetColour(cl);
  dc->SetBrush(brush);
  dc->SetPen(*wxTRANSPARENT_PEN);

  double ratioH = double(m_plotheight) / m_MaxDepth;
  double ratioW = double(size.x - 6) / (DEPTH_RECORD_COUNT - 1);
  wxPoint points[DEPTH_RECORD_COUNT + 2];

#ifdef __OCPN__ANDROID__
  int px = 3;
  points[0].x = px;
  points[0].y = m_plotdown;

  for (int idx = 0; idx < DEPTH_RECORD_COUNT - 1; idx++) {
    points[1].x = points[0].x;
    if (m_ArrayDepth[idx])
      points[1].y = m_plotup + m_ArrayDepth[idx] * ratioH;
    else
      points[1].y = m_plotdown;

    points[2].x = points[1].x + ratioW;
    if (m_ArrayDepth[idx + 1])
      points[2].y = m_plotup + m_ArrayDepth[idx + 1] * ratioH;
    else
      points[2].y = m_plotdown;

    points[3].x = points[2].x;
    points[3].y = m_plotdown;
    dc->DrawPolygon(4, points);

    points[0].x = points[2].x;
    points[0].y = m_plotdown;
  }

#else
  for (int idx = 0; idx < DEPTH_RECORD_COUNT; idx++) {
    points[idx].x = idx * ratioW + 3;
    if (m_ArrayDepth[idx])
      points[idx].y = m_plotup + m_ArrayDepth[idx] * ratioH;
    else
      points[idx].y = m_plotdown;
  }
  points[DEPTH_RECORD_COUNT].x = size.x - 3;
  points[DEPTH_RECORD_COUNT].y = m_plotdown;
  points[DEPTH_RECORD_COUNT + 1].x = 3;
  points[DEPTH_RECORD_COUNT + 1].y = m_plotdown;
  dc->DrawPolygon(DEPTH_RECORD_COUNT + 2, points);
#endif

  GetGlobalColor(_T("DASHF"), &cl);
  dc->SetTextForeground(cl);
  dc->SetFont(*g_pFontData);
  if (m_DepthUnit != _T("-")) {  // Watchdog
    wxString s_depth = wxString::Format(_T("%.2f"), m_Depth);
    // We want only one decimal but for security not rounded up.
    s_depth = s_depth.Mid(0, s_depth.length() - 1);
    dc->DrawText(s_depth + _T(" ") + m_DepthUnit, 10, m_TitleHeight);
  } else
    dc->DrawText(_T("---"), 10, m_TitleHeight);

  dc->SetFont(*g_pFontLabel);  
  dc->DrawText(m_Temp, 5, m_plotdown);
}
