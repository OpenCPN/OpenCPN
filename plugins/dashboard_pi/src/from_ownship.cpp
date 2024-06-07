/******************************************************************************
 * $Id: from_ownship.cpp
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#include "from_ownship.h"

extern int g_iDashDistanceUnit;

//----------------------------------------------------------------
//
//    DashboardInstrument_FromOwnship Implementation
//
//----------------------------------------------------------------
DashboardInstrument_FromOwnship::DashboardInstrument_FromOwnship(
    wxWindow* pparent, wxWindowID id, wxString title, InstrumentProperties* Properties, DASH_CAP cap_flag1,
    DASH_CAP cap_flag2, DASH_CAP cap_flag3, DASH_CAP cap_flag4)
    : DashboardInstrument(pparent, id, title, cap_flag1, Properties) {
  m_cap_flag.set(cap_flag2);
  m_cap_flag.set(cap_flag3);
  m_cap_flag.set(cap_flag4);
  m_data1 = _T("---");
  m_data2 = _T("---");
  m_cap_flag1 = cap_flag1;
  m_cap_flag2 = cap_flag2;
  m_cap_flag3 = cap_flag3;
  m_cap_flag4 = cap_flag4;
  s_lat = 99999999;
  s_lon = 99999999;
}

void DashboardInstrument_FromOwnship::Draw(wxGCDC* dc) {
  wxColour cl;
  wxFont f;
  int dataMargin=g_iDataMargin;
  if (m_Properties)
  {
      f = m_Properties->m_DataFont.GetChosenFont();
      dc->SetFont((f));
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_DataFont.GetColour()));
      if ( m_Properties->m_DataMargin>=0 ) dataMargin = m_Properties->m_DataMargin;
  }
  else
  {
      f = g_pFontData->GetChosenFont();
      dc->SetFont((f));
      dc->SetTextForeground(GetColourSchemeFont(g_pFontData->GetColour()));
  }

  int x1,x2;
  x1=x2=dataMargin;

  if ( m_DataRightAlign ) {
    int w,h;
    dc->GetTextExtent(m_data1, &w, &h, 0, 0, &f);
    x1=GetClientSize().GetWidth() - w - dataMargin;
    dc->GetTextExtent(m_data2, &w, &h, 0, 0, &f);
    x2=GetClientSize().GetWidth() - w - dataMargin;
  }

  dc->DrawText(m_data1, x1, m_DataTop);
  dc->DrawText(m_data2, x2, m_DataTop + m_DataHeight);
}

void DashboardInstrument_FromOwnship::SetData(DASH_CAP st, double data,
                                              wxString unit) {
  if (st == m_cap_flag1) {
    c_lat = data;
  } else if (st == m_cap_flag2) {
    c_lon = data;
  } else if (st == m_cap_flag3) {
    s_lat = data;
  } else if (st == m_cap_flag4) {
    s_lon = data;
  } else
    return;
  if (s_lat < 99999999 && s_lon < 99999999) {
    double brg, dist;
    bool showUnit = (m_Properties ? (m_Properties->m_ShowUnit==1) : g_bShowUnit);
    DistanceBearingMercator_Plugin(c_lat, c_lon, s_lat, s_lon, &brg, &dist);
    if (showUnit) {
      m_data1.Printf(_T("%03d ") + DEGREE_SIGN , (int)brg);
      m_data2.Printf(_T("%3.2f %s"),
                     toUsrDistance_Plugin(dist, g_iDashDistanceUnit),
                     getUsrDistanceUnit_Plugin(g_iDashDistanceUnit).c_str());
    } else {
      m_data1.Printf(_T("%03d") , (int)brg);
      m_data2.Printf(_T("%3.2f"),
                     toUsrDistance_Plugin(dist, g_iDashDistanceUnit));
    }
  }

  Refresh(false);
}

wxSize DashboardInstrument_FromOwnship::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  wxFont f;
  wxString sampleText;
  int dataMargin=g_iDataMargin;
  m_DataRightAlign = (g_DataAlignment & wxALIGN_RIGHT) != 0;
  int InstrumentSpacing = g_iInstrumentSpacing;

  if (m_Properties ? (m_Properties->m_ShowUnit == 1) : g_bShowUnit) {
    sampleText=_T("000.00 NMi");
  } else {
    sampleText=_T("000.00");
  }
  if (m_Properties)
  {
      f = m_Properties->m_TitleFont.GetChosenFont();
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, &f);
      f = m_Properties->m_DataFont.GetChosenFont();
      dc.GetTextExtent(sampleText, &w, &m_DataHeight, 0, 0, &f);

      if ( m_Properties->m_DataMargin>=0 ) dataMargin = m_Properties->m_DataMargin;
      if ( m_Properties->m_DataAlignment!=wxALIGN_INVALID ) m_DataRightAlign = (m_Properties->m_DataAlignment & wxALIGN_RIGHT) != 0;
      if ( m_Properties->m_InstrumentSpacing >= 0 ) InstrumentSpacing = m_Properties->m_InstrumentSpacing;
  }
  else
  {
      f = g_pFontTitle->GetChosenFont();
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, &f);
      f = g_pFontData->GetChosenFont();
      dc.GetTextExtent(sampleText, &w, &m_DataHeight, 0, 0, &f);
  }

  m_TitleRightAlign = (g_TitleAlignment & wxALIGN_RIGHT) != 0;
  m_TitleTop = m_DataHeight*g_TitleVerticalOffset;
  int h=m_TitleTop + m_TitleHeight + m_DataHeight * 2 + InstrumentSpacing;
  m_DataTop = m_TitleHeight;
  if ( (g_TitleAlignment & wxALIGN_BOTTOM) != 0 ) {
    m_TitleTop = m_DataHeight * 2 + m_DataHeight * g_TitleVerticalOffset;
    h=m_TitleTop + m_TitleHeight + InstrumentSpacing;
    m_DataTop = 0;
  }

  if (orient == wxHORIZONTAL) {
    return wxSize(w + dataMargin, wxMax(hint.y, h));
  } else {
    return wxSize(wxMax(hint.x, w + dataMargin), h);
  }
}
