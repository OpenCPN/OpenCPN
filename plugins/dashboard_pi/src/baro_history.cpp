/******************************************************************************
 * $Id: baro_history.cpp, v1.0 2014/02/10 tom-r Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   stedy
 * Based on code from  Thomas Rauch
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

#include "baro_history.h"
#include "wx28compat.h"

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

//************************************************************************************************************************
// History of barometic pressure
//************************************************************************************************************************

DashboardInstrument_BaroHistory::DashboardInstrument_BaroHistory(
    wxWindow* parent, wxWindowID id, wxString title)
    : DashboardInstrument(parent, id, title, OCPN_DBP_STC_MDA) {
  SetDrawSoloInPane(true);

  m_MaxPress = 0;
  m_MinPress = (double)1200;
  m_TotalMaxPress = 0;
  m_TotalMinPress = 1200;
  m_Press = 0;
  // Set top line height to leave space for pressure data
  wxClientDC dc(this);
  int w, h;
  dc.GetTextExtent("hPa----", &w, &h, 0, 0, g_pFontData);
  m_TopLineHeight = wxMax(30, h);
  m_SpdRecCnt = 0;
  m_SpdStartVal = -1;
  m_IsRunning = false;
  m_SampleCount = 0;
  m_LeftLegend = 3;
  m_RightLegend = 3;
  for (int idx = 0; idx < BARO_RECORD_COUNT; idx++) {
    m_ArrayPressHistory[idx] = -1;
    m_ExpSmoothArrayPressure[idx] = -1;
    m_ArrayRecTime[idx] = wxDateTime::Now().GetTm();
    m_ArrayRecTime[idx].year = 999;
  }
  alpha = 0.01;  // smoothing constant
  m_WindowRect = GetClientRect();
  m_DrawAreaRect = GetClientRect();
  m_DrawAreaRect.SetHeight(m_WindowRect.height - m_TopLineHeight -
                           m_TitleHeight);
}

wxSize DashboardInstrument_BaroHistory::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
  if (orient == wxHORIZONTAL) {
    return wxSize(DefaultWidth, wxMax(m_TitleHeight + 140, hint.y));
  } else {
    return wxSize(wxMax(hint.x, DefaultWidth),
                  wxMax(m_TitleHeight + 140, hint.y));
  }
}
void DashboardInstrument_BaroHistory::SetData(DASH_CAP st, double data,
                                              wxString unit) {
  if (st == OCPN_DBP_STC_MDA) {
    m_Press = data;
    if (m_SpdRecCnt++ <= 5) m_SpdStartVal += data;
  }
  if (m_SpdRecCnt == 5) {
    m_Press = m_SpdStartVal / 5;
  }
  // start working after we collected 5 records each, as start values for the
  // smoothed curves
  if (m_SpdRecCnt > 5) {
    m_IsRunning = true;
    m_SampleCount = m_SampleCount < BARO_RECORD_COUNT ? m_SampleCount + 1
                                                      : BARO_RECORD_COUNT;
    m_MaxPress = 0;
    ;
    // data shifting
    for (int idx = 1; idx < BARO_RECORD_COUNT; idx++) {
      if (BARO_RECORD_COUNT - m_SampleCount <= idx)
        m_MaxPress = wxMax(m_ArrayPressHistory[idx - 1], m_MaxPress);
      m_MinPress = wxMin(m_ArrayPressHistory[idx - 1], m_MinPress);
      m_ArrayPressHistory[idx - 1] = m_ArrayPressHistory[idx];
      m_ExpSmoothArrayPressure[idx - 1] = m_ExpSmoothArrayPressure[idx];
      m_ArrayRecTime[idx - 1] = m_ArrayRecTime[idx];
    }
    m_ArrayPressHistory[BARO_RECORD_COUNT - 1] = m_Press;
    if (m_SampleCount < 2) {
      m_ArrayPressHistory[BARO_RECORD_COUNT - 2] = m_Press;
      m_ExpSmoothArrayPressure[BARO_RECORD_COUNT - 2] = m_Press;
    }
    m_ExpSmoothArrayPressure[BARO_RECORD_COUNT - 1] =
        alpha * m_ArrayPressHistory[BARO_RECORD_COUNT - 2] +
        (1 - alpha) * m_ExpSmoothArrayPressure[BARO_RECORD_COUNT - 2];
    m_ArrayRecTime[BARO_RECORD_COUNT - 1] = wxDateTime::Now().GetTm();
    m_MaxPress = wxMax(m_Press, m_MaxPress);

    m_MinPress = wxMin(m_MinPress, m_Press);
    if (wxMin(m_Press, m_MinPress) == -1) {
      m_MinPress = wxMin(m_Press, 1200);  // to make a OK inital value
    }
    // get the overall max min pressure
    m_TotalMaxPress = wxMax(m_Press, m_TotalMaxPress);
    m_TotalMinPress = wxMin(m_Press, m_TotalMinPress);
  }
}

void DashboardInstrument_BaroHistory::Draw(wxGCDC* dc) {
  m_WindowRect = GetClientRect();
  m_DrawAreaRect = GetClientRect();
  m_DrawAreaRect.SetHeight(m_WindowRect.height - m_TopLineHeight -
                           m_TitleHeight);
  m_DrawAreaRect.SetX(m_LeftLegend + 3);
  DrawBackground(dc);
  DrawForeground(dc);
}

//*********************************************************************************
// draw pressure scale
//*********************************************************************************
void DashboardInstrument_BaroHistory::DrawWindSpeedScale(wxGCDC* dc) {
  wxString label1, label2, label3, label4, label5;
  wxColour cl;
  int width, height;
  cl = wxColour(61, 61, 204, 255);
  dc->SetTextForeground(cl);
  dc->SetFont(*g_pFontSmall);
  // round m_MaxPress up to the next hpa ...
  if (m_MaxPress > 1100) m_MaxPress = 1100;

  if (m_TotalMinPress < 930) m_TotalMinPress = 930;

  m_MaxPressScale = (int)((m_MaxPress + 15) - (m_TotalMinPress - 15));

  if (!m_IsRunning) {
    label1 = _T("-- hPa");
    label2 = _T("-- hPa");
    label3 = _T("-- hPa");
    label4 = _T("-- hPa");
    label5 = _T("-- hPa");
  } else {
    /*
     The goal is to draw the legend with decimals only, if we really have them !
    */
    // top legend for max press
    label1.Printf(_T("%.0f hPa"), m_MaxPressScale + (m_TotalMinPress - 18));

    // 3/4 legend

    label2.Printf(_T("%.0f hPa"),
                  m_MaxPressScale * 3. / 4 + (m_TotalMinPress - 18));

    // center legend

    label3.Printf(_T("%.0f hPa"), m_MaxPressScale / 2 + (m_TotalMinPress - 18));

    // 1/4 legend

    label4.Printf(_T("%.0f hPa"), m_MaxPressScale / 4 + (m_TotalMinPress - 18));

    // bottom legend for min wind
    label5.Printf(_T("%.0f hPa"), (m_TotalMinPress - 18));
  }
  dc->GetTextExtent(label1, &m_LeftLegend, &height, 0, 0, g_pFontSmall);
  dc->DrawText(label1, 4, (int)(m_TopLineHeight - height / 2));
  dc->GetTextExtent(label2, &width, &height, 0, 0, g_pFontSmall);
  dc->DrawText(label2, 4,
               (int)(m_TopLineHeight + m_DrawAreaRect.height / 4 - height / 2));
  m_LeftLegend = wxMax(width, m_LeftLegend);
  dc->GetTextExtent(label3, &width, &height, 0, 0, g_pFontSmall);
  dc->DrawText(label3, 4,
               (int)(m_TopLineHeight + m_DrawAreaRect.height / 2 - height / 2));
  m_LeftLegend = wxMax(width, m_LeftLegend);
  dc->GetTextExtent(label4, &width, &height, 0, 0, g_pFontSmall);
  dc->DrawText(
      label4, 4,
      (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.75 - height / 2));
  m_LeftLegend = wxMax(width, m_LeftLegend);
  dc->GetTextExtent(label5, &width, &height, 0, 0, g_pFontSmall);
  dc->DrawText(label5, 4,
               (int)(m_TopLineHeight + m_DrawAreaRect.height - height / 2));
  m_LeftLegend = wxMax(width, m_LeftLegend);
  m_LeftLegend += 4;
}

//*********************************************************************************
// draw background
//*********************************************************************************
void DashboardInstrument_BaroHistory::DrawBackground(wxGCDC* dc) {
  wxString label, label1, label2, label3, label4, label5;
  wxColour cl;
  wxPen pen;
  //---------------------------------------------------------------------------------
  // draw legends for spressure
  //---------------------------------------------------------------------------------

  DrawWindSpeedScale(dc);

  //---------------------------------------------------------------------------------
  // horizontal lines
  //---------------------------------------------------------------------------------
  GetGlobalColor(_T("UBLCK"), &cl);
  pen.SetColour(cl);
  dc->SetPen(pen);
  dc->DrawLine(m_LeftLegend + 3, m_TopLineHeight,
               m_WindowRect.width - 3 - m_RightLegend,
               m_TopLineHeight);  // the upper line
  dc->DrawLine(m_LeftLegend + 3, (int)(m_TopLineHeight + m_DrawAreaRect.height),
               m_WindowRect.width - 3 - m_RightLegend,
               (int)(m_TopLineHeight + m_DrawAreaRect.height));
  pen.SetStyle(wxPENSTYLE_DOT);
  dc->SetPen(pen);
  dc->DrawLine(m_LeftLegend + 3,
               (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.25),
               m_WindowRect.width - 3 - m_RightLegend,
               (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.25));
  dc->DrawLine(m_LeftLegend + 3,
               (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.75),
               m_WindowRect.width - 3 - m_RightLegend,
               (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.75));
#ifdef __WXMSW__
  pen.SetStyle(wxPENSTYLE_SHORT_DASH);
  dc->SetPen(pen);
#endif
  dc->DrawLine(m_LeftLegend + 3,
               (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.5),
               m_WindowRect.width - 3 - m_RightLegend,
               (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.5));
}

//*********************************************************************************
// draw foreground
//*********************************************************************************
void DashboardInstrument_BaroHistory::DrawForeground(wxGCDC* dc) {
  wxColour col;
  double ratioH;
  int degw, degh;
  int width, height, sec, min, hour;
  wxString WindAngle, WindSpeed;
  wxPen pen;
  wxString label;

  //---------------------------------------------------------------------------------
  // Pressure
  //---------------------------------------------------------------------------------
  col = wxColour(61, 61, 204, 255);  // blue, opaque
  dc->SetFont(*g_pFontData);
  dc->SetTextForeground(col);
  if (!std::isnan(m_Press))
    WindSpeed = wxString::Format(_T("hPa %3.1f  "), m_Press);
  else
    WindSpeed = wxString::Format(_T("hPa ---  "));
  dc->GetTextExtent(WindSpeed, &degw, &degh, 0, 0, g_pFontData);
  dc->DrawText(WindSpeed, m_LeftLegend + 3, 1);
  dc->SetFont(*g_pFontLabel);
  int labelw, labelh;
  dc->GetTextExtent(WindSpeed, &labelw, &labelh, 0, 0, g_pFontLabel);
  // determine the time range of the available data (=oldest data value)
  int i = 0;
  while (m_ArrayRecTime[i].year == 999 && i < BARO_RECORD_COUNT - 1) i++;
  if (i == BARO_RECORD_COUNT - 1) {
    min = 0;
    hour = 0;

  } else {
    wxDateTime localTime(m_ArrayRecTime[i]);
    min = localTime.GetMinute();
    hour = localTime.GetHour();
  }
  m_ratioW = double(m_DrawAreaRect.width) / (BARO_RECORD_COUNT - 1);

  dc->DrawText(wxString::Format(
                   _(" Max %.1f since %02d:%02d  Overall Max %.1f Min %.1f "),
                   m_MaxPress, hour, min, m_TotalMaxPress, m_TotalMinPress),
                   m_LeftLegend + 2 + degw, m_TopLineHeight - 1 - labelh);
  pen.SetStyle(wxPENSTYLE_SOLID);
  pen.SetColour(wxColour(61, 61, 204, 96));  // blue, transparent
  pen.SetWidth(1);
  dc->SetPen(pen);
  ratioH = (double)m_DrawAreaRect.height / (double)m_MaxPressScale;

  wxPoint pointsSpd[BARO_RECORD_COUNT + 2], pointSpeed_old;
  pointSpeed_old.x = m_LeftLegend + 3;
  pointSpeed_old.y =
      m_TopLineHeight + m_DrawAreaRect.height - m_ArrayPressHistory[0] * ratioH;

  //---------------------------------------------------------------------------------
  // live pressure data
  //---------------------------------------------------------------------------------

  for (int idx = 1; idx < BARO_RECORD_COUNT; idx++) {
    // pointsSpd[idx].x = idx  + 3 + m_LeftLegend;
    // pointsSpd[idx].y = m_TopLineHeight+m_DrawAreaRect.height -
    // m_ArrayPressHistory[idx] * ratioH;
    pointsSpd[idx].y =
        m_TopLineHeight + m_DrawAreaRect.height -
        ((m_ArrayPressHistory[idx] - (double)m_TotalMinPress + 18) * ratioH);
    pointsSpd[idx].x = idx * m_ratioW - 3;  //- 30 + m_LeftLegend;
    // pointsSpd[idx].x = idx + m_DrawAreaRect.x;
    // pointsSpd[idx].y= m_ArrayPressHistory[idx] * ratioH;
    if (BARO_RECORD_COUNT - m_SampleCount <= idx &&
        pointsSpd[idx].y > m_TopLineHeight &&
        pointSpeed_old.y > m_TopLineHeight &&
        pointsSpd[idx].y <= m_TopLineHeight + m_DrawAreaRect.height &&
        pointSpeed_old.y <= m_TopLineHeight + m_DrawAreaRect.height)
      dc->DrawLine(pointSpeed_old.x, pointSpeed_old.y, pointsSpd[idx].x,
                   pointsSpd[idx].y);
    // dc->DrawLine( pointSpeed_old.x, pointSpeed_old.y,
    // pointsSpd[idx].x,pointsSpd[idx].y );
    pointSpeed_old.x = pointsSpd[idx].x;
    pointSpeed_old.y = pointsSpd[idx].y;
  }

  //---------------------------------------------------------------------------------
  // exponential smoothing of barometric pressure
  //---------------------------------------------------------------------------------
  /*
   // For now i cant see the reason for implementing smoothing for barometric
   pressure. pen.SetStyle(wxSOLID); pen.SetColour(wxColour(61,61,204,255));
   //blue, opaque pen.SetWidth(2); dc->SetPen( pen );
    pointSpeed_old.x=m_LeftLegend+3;
    pointSpeed_old.y = m_TopLineHeight+m_DrawAreaRect.height -
   m_ExpSmoothArrayWindSpd[0] * ratioH;

    for (int idx = 1; idx < BARO_RECORD_COUNT; idx++) {
      pointsSpd[idx].x = idx * m_ratioW + 3 + m_LeftLegend;
      pointsSpd[idx].y = m_ExpSmoothArrayWindSpd[idx] * ratioH;
      if(BARO_RECORD_COUNT-m_SampleCount <= idx && pointsSpd[idx].y >
   m_TopLineHeight && pointSpeed_old.y > m_TopLineHeight && pointsSpd[idx].y
   <=m_TopLineHeight+m_DrawAreaRect.height &&
   pointSpeed_old.y<=m_TopLineHeight+m_DrawAreaRect.height) dc->DrawLine(
   pointSpeed_old.x, pointSpeed_old.y, pointsSpd[idx].x,pointsSpd[idx].y );
      pointSpeed_old.x=pointsSpd[idx].x;
      pointSpeed_old.y=pointsSpd[idx].y;
    }

  */
  //---------------------------------------------------------------------------------
  // draw vertical timelines every 5 minutes
  //---------------------------------------------------------------------------------
  GetGlobalColor(_T("UBLCK"), &col);
  pen.SetColour(col);
  pen.SetStyle(wxPENSTYLE_DOT);
  dc->SetPen(pen);
  dc->SetTextForeground(col);
  dc->SetFont(*g_pFontSmall);
  int done = -1;
  wxPoint pointTime;
  for (int idx = 0; idx < BARO_RECORD_COUNT; idx++) {
    if (m_ArrayRecTime[idx].year != 999) {
      wxDateTime localTime(m_ArrayRecTime[i]);
      hour = localTime.GetHour();
      sec = localTime.GetSecond();
      min = localTime.GetMinute();
      if ((hour * 100 + min) != done && (min % 5 == 0) &&
          (sec == 0 || sec == 1)) {
        pointTime.x = idx * m_ratioW + 3 + m_LeftLegend;
        dc->DrawLine(pointTime.x, m_TopLineHeight + 1, pointTime.x,
                     (m_TopLineHeight + m_DrawAreaRect.height + 1));
        label.Printf(_T("%02d:%02d"), hour, min);
        dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
        dc->DrawText(label, pointTime.x - width / 2,
                     m_WindowRect.height - height);
        done = hour * 100 + min;
      }
    }
  }
}
