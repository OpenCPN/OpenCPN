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


#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include "altitude.h"
extern int g_iDashDepthUnit;   // use same unit for altitude

#ifdef __BORLANDC__
#pragma hdrstop
#endif

int m_aDataHeight;
int x_alabel, y_alabel, a_plotdown, a_plotup, a_plotheight;

DashboardInstrument_Altitude::DashboardInstrument_Altitude(wxWindow* parent,
                                                     wxWindowID id,
                                                     wxString title,
                                                     InstrumentProperties* Properties)
    : DashboardInstrument(parent, id, title, OCPN_DBP_STC_ALTI, Properties) {
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
  wxFont f;
  if (m_Properties)
  {
      f = m_Properties->m_TitleFont.GetChosenFont();
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, &f);
      f = m_Properties->m_DataFont.GetChosenFont();
      dc.GetTextExtent("15.7 Feet", &w, &m_aDataHeight, 0, 0, &f);
      f = m_Properties->m_LabelFont.GetChosenFont();
      dc.GetTextExtent("20.8 C", &x_alabel, &y_alabel, 0, 0, &f);
  }
  else
  {
      f = g_pFontTitle->GetChosenFont();
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, &f);
      f = g_pFontData->GetChosenFont();
      dc.GetTextExtent("15.7 Feet", &w, &m_aDataHeight, 0, 0, &f);
      // Space for bottom(temp)text later.
      f = g_pFontLabel->GetChosenFont();
      dc.GetTextExtent("20.8 C", &x_alabel, &y_alabel, 0, 0, &f);
  }
  int y_total =
      //  Title         Alt. data       plot area     air-temp
      m_TitleHeight + m_aDataHeight + 4 * m_aDataHeight + y_alabel;
  if (orient == wxHORIZONTAL) {
    return wxSize(DefaultWidth, wxMax(y_total, hint.y));
  } else {
    return wxSize(wxMax(hint.x, DefaultWidth), y_total);
  }
}

void DashboardInstrument_Altitude::SetData(DASH_CAP st, double data,
                                        wxString unit) {
  if (st == OCPN_DBP_STC_ALTI) {
    m_Altitude = std::isnan(data) ? 0.0 : data;
    //m_Altitude = m_Altitude*10.0 +1000;       // inject fake testdata
    //printf("Altitude = %3.3f\n", m_Altitude); // debug output

    // save FLOPS by just accumulating the FIFO changes
    m_meanAltitude += (m_Altitude - m_ArrayAltitude[0]) / ALTITUDE_RECORD_COUNT;
    m_sum2Altitude += (m_Altitude*m_Altitude - m_ArrayAltitude[0]*m_ArrayAltitude[0]);
    
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

void   DashboardInstrument_Altitude::setAttenuation(int steps) {
  // fast int stuff
  // increase the attenuation in 1 2 5 10 20 50 steps
  if (steps > 0)
    while (steps-- ) {
      switch (m_Attenuation) {
        case 1:
          m_Attenuation=2; break;
        case 2:
          m_Attenuation=5; break;
        default:
          m_Attenuation=1;
          m_Decade*=10;
      }
    }
  // decrease the attenuation in 1 2 5 10 20 50 steps
  else if (steps<0)
    while ( steps++ ) {
      switch (m_Attenuation) {
        case 5:
          m_Attenuation=2; break;
        case 2:
          m_Attenuation=1; break;
        default:
          m_Attenuation=5;
          m_Decade/=10;
      }
    }
   // bottom limit: unity
   if (m_Decade < 1) {
     m_Decade = 1;
     m_Attenuation=1;
   }
}

int DashboardInstrument_Altitude::getAttenuation() {
  return m_Attenuation*m_Decade;
}


void DashboardInstrument_Altitude::Draw(wxGCDC* dc) {
  DrawBackground(dc);
  DrawForeground(dc);
}

void DashboardInstrument_Altitude::DrawBackground(wxGCDC* dc) {
  wxSize size = GetClientSize();
  wxColour cl;
  if (m_Properties)
  {
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_LabelFont.GetColour()));
  }
  else
  {
      if (GetColourSchemeFont(g_pFontSmall->GetColour()) == GetColourSchemeFont(g_pFontLabel->GetColour()))
      {
          GetGlobalColor(_T("DASHL"), &cl);
          dc->SetTextForeground(cl);
      }
      else
          dc->SetTextForeground(GetColourSchemeFont(g_pFontLabel->GetColour()));
  }

  wxPen pen;
  pen.SetStyle(wxPENSTYLE_SOLID);
  if (m_Properties)
      cl = GetColourSchemeFont(m_Properties->m_SmallFont.GetColour());
  else  
      cl = GetColourSchemeFont(g_pFontSmall->GetColour());
  //GetGlobalColor(_T("DASHF"), &cl);
  pen.SetColour(cl);
  pen.SetWidth(1);
  dc->SetPen(pen);

  a_plotup = m_TitleHeight + m_aDataHeight;
  a_plotdown = size.y - y_alabel;
  a_plotheight = a_plotdown - a_plotup;

  // dc->DrawLine(3,  44, size.x - 3,  44);
  dc->DrawLine(3, a_plotdown, size.x - 3, a_plotdown);

#ifdef __WXMSW__
  pen.SetStyle(wxPENSTYLE_SHORT_DASH);
#else
  pen.SetStyle(wxPENSTYLE_DOT);
  pen.SetWidth(1);
#endif

  // Grid lines
  dc->SetPen(pen);
  dc->DrawLine(3, a_plotup, size.x - 3, a_plotup);
  dc->DrawLine(3, a_plotup + a_plotheight / 4, size.x - 3,
               a_plotup + a_plotheight / 4);
  dc->DrawLine(3, a_plotup + a_plotheight * 2 / 4, size.x - 3,
               a_plotup + a_plotheight * 2 / 4);
  dc->DrawLine(3, a_plotup + a_plotheight * 3 / 4, size.x - 3,
               a_plotup + a_plotheight * 3 / 4);
  if (m_Properties)
  {
      dc->SetFont(m_Properties->m_SmallFont.GetChosenFont());
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_SmallFont.GetColour()));
  }
  else
  {
      dc->SetFont(g_pFontSmall->GetChosenFont());
      dc->SetTextForeground(GetColourSchemeFont(g_pFontSmall->GetColour()));
  }
  double MaxAltitude =   -9999.0;
  double MinAltitude = 9999999.0;
  // evaluate buffered data
  for (int idx = 0; idx < ALTITUDE_RECORD_COUNT; idx++) {
    if      (m_ArrayAltitude[idx] > MaxAltitude) MaxAltitude = m_ArrayAltitude[idx];
    else if (m_ArrayAltitude[idx] < MinAltitude) MinAltitude = m_ArrayAltitude[idx];
  }

  // calculate 1st and 2nd Moments
  double varAltitude = m_sum2Altitude / ALTITUDE_RECORD_COUNT;  // biased estimator, avoid / N-1
  varAltitude  -= m_meanAltitude*m_meanAltitude;
  
  // do AGC to adjust scaling
  double range  = MaxAltitude-MinAltitude;
  if (range > 1.1*m_Range )
    setAttenuation(+1);
  if (range < 0.3*m_Range )  // some hysteresis
    setAttenuation(-1);
  double grid = getAttenuation();
  m_Range = grid*c_GridLines;
  //printf("m_Range = %5.1f  range = %5.1f  att=%d , mean=%3.2f, std=%3.2f\n", 
  //  m_Range, range, getAttenuation(), meanAltitude, sqrt(varAltitude));  // debug output

  // only update axes on major corridor changes
  if( (MaxAltitude - m_MaxAltitude)/grid >  0.25 || 
      (MaxAltitude - m_MaxAltitude)/grid < -0.75*c_GridLines ) {
    m_MaxAltitude = (round(MaxAltitude/grid) + 1)*grid;
    m_MinAltitude = m_MaxAltitude - m_Range;
  }
  if( (MinAltitude - m_MinAltitude)/grid < -0.25 ||
      (MinAltitude - m_MinAltitude)/grid >  0.75*c_GridLines ) {
    m_MinAltitude = (round(MinAltitude/grid) - 1)*grid;
    m_MaxAltitude = m_MinAltitude + m_Range;
  }
 
  wxString label;
  label.Printf(_T("+/-%.1f %8.0f ") + m_AltitudeUnit, sqrt(varAltitude), m_MaxAltitude);
  int width, height;
  wxFont f;
  if (m_Properties)
      f = m_Properties->m_SmallFont.GetChosenFont();
  else
      f = g_pFontSmall->GetChosenFont();
  dc->GetTextExtent(label, &width, &height, 0, 0, &f);
  dc->DrawText(label, size.x - width - 1, a_plotup - height);

  label.Printf(_T("%.1f/ %8.0f ") + m_AltitudeUnit, m_Range/c_GridLines, m_MinAltitude);
  if (m_Properties)
      f = m_Properties->m_SmallFont.GetChosenFont();
  else
      f = g_pFontSmall->GetChosenFont();
  dc->GetTextExtent(label, &width, &height, 0, 0, &f);
  dc->DrawText(label, size.x - width - 1, a_plotdown);
}

void DashboardInstrument_Altitude::DrawForeground(wxGCDC* dc) {
  wxSize size = GetClientSize();
  wxColour cl;
  if (m_Properties)
  {
      cl = GetColourSchemeFont(m_Properties->m_LabelFont.GetColour());
  }
  else
  {
      if (GetColourSchemeFont(g_pFontSmall->GetColour()) == GetColourSchemeFont(g_pFontLabel->GetColour()))
          GetGlobalColor(_T("DASH1"), &cl);
      else
          cl = GetColourSchemeFont(g_pFontLabel->GetColour());
  }
  //GetGlobalColor(_T("DASH1"), &cl);
  wxBrush brush;
  brush.SetStyle(wxBRUSHSTYLE_SOLID);
  brush.SetColour(cl);
  dc->SetBrush(brush);
  dc->SetPen(*wxTRANSPARENT_PEN);

  double ratioH = double(a_plotheight) / m_Range;
  double ratioW = double(size.x - 6) / (ALTITUDE_RECORD_COUNT - 1);
  wxPoint points[ALTITUDE_RECORD_COUNT + 2];
#ifdef __OCPN__ANDROID__
  int px = 3;
  points[0].x = px;
  points[0].y = a_plotdown;

  for (int idx = 0; idx < ALTITUDE_RECORD_COUNT - 1; idx++) {
    points[1].x = points[0].x;
    if (m_ArrayAltitude[idx])
      points[1].y =
          a_plotdown - (m_ArrayAltitude[idx] - m_MinAltitude) * ratioH;
    else
      points[1].y = a_plotdown;

    points[2].x = points[1].x + ratioW;
    if (m_ArrayAltitude[idx + 1])
      points[2].y =
          a_plotdown - (m_ArrayAltitude[idx + 1] - m_MinAltitude) * ratioH;
    else
      points[2].y = a_plotdown;

    points[3].x = points[2].x;
    points[3].y = a_plotdown;
    dc->DrawPolygon(4, points);

    points[0].x = points[2].x;
    points[0].y = a_plotdown;
  }

#else
  for (int idx = 0; idx < ALTITUDE_RECORD_COUNT; idx++) {
    points[idx].x = idx * ratioW + 3;
    points[idx].y =
        a_plotdown - (m_ArrayAltitude[idx] - m_MinAltitude) * ratioH;
  }
  points[ALTITUDE_RECORD_COUNT].x = size.x - 3;
  points[ALTITUDE_RECORD_COUNT].y = a_plotdown;
  points[ALTITUDE_RECORD_COUNT + 1].x = 3;
  points[ALTITUDE_RECORD_COUNT + 1].y = a_plotdown;
  dc->DrawPolygon(ALTITUDE_RECORD_COUNT + 2, points);
#endif
  if (m_Properties)
  {
      dc->SetFont(m_Properties->m_DataFont.GetChosenFont());
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_DataFont.GetColour()));
  }
  else
  {
      //GetGlobalColor(_T("DASHF"), &cl); 
      dc->SetTextForeground(GetColourSchemeFont(g_pFontData->GetColour()));
      dc->SetFont(g_pFontData->GetChosenFont());
  } 
  if (m_AltitudeUnit != _T("-")) {  // Watchdog
    wxString s_alti = wxString::Format(_T("%.1f"), m_meanAltitude);
    dc->DrawText(s_alti + _T(" ") + m_AltitudeUnit, 10, m_TitleHeight);
  } else
    dc->DrawText(_T("---"), 10, m_TitleHeight);

  // TODO: test display air temperature ID_DBP_I_ATMP
  if (m_Properties)
      dc->SetFont(m_Properties->m_LabelFont.GetChosenFont());
  else
      dc->SetFont(g_pFontLabel->GetChosenFont());
  int width, height;
  wxFont f;
  if(m_Properties) {
      f = m_Properties->m_LabelFont.GetChosenFont();
      dc->GetTextExtent(m_Temp, &width, &height, 0, 0, &f);
  } else {
      f = g_pFontLabel->GetChosenFont();
      dc->GetTextExtent(m_Temp, &width, &height, 0, 0, &f);
  }
  dc->DrawText(m_Temp, 3, a_plotdown);
}
