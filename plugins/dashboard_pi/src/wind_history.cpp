/******************************************************************************
 * $Id: wind_history.cpp, v1.0 2010/08/30 tom-r Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Thomas Rauch
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

#include "wind_history.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

//************************************************************************************************************************
// History of wind direction
//************************************************************************************************************************

DashboardInstrument_WindDirHistory::DashboardInstrument_WindDirHistory(
    wxWindow* parent, wxWindowID id, wxString title, InstrumentProperties* Properties)
    : DashboardInstrument(parent, id, title, OCPN_DBP_STC_TWD, Properties) {
  m_cap_flag.set(OCPN_DBP_STC_TWS);
  SetDrawSoloInPane(true);
  m_MaxWindDir = -1;
  m_WindDir = -1;
  m_WindDirRange = 90;
  m_MaxWindSpd = 0;
  m_WindSpeedUnit = _("-");
  m_TotalMaxWindSpd = 0;
  m_WindSpd = 0;
  // Set top line height to leave space for wind data
  wxClientDC dc(this);
  int w, h;
  wxFont f;
  if (m_Properties)
      f = m_Properties->m_DataFont.GetChosenFont();
  else
      f = g_pFontData->GetChosenFont();
  dc.GetTextExtent("TWS----", &w, &h, 0, 0, &f);
  m_TopLineHeight = wxMax(30, h);
  m_SpdRecCnt = 0;
  m_DirRecCnt = 0;
  m_SpdStartVal = -1;
  m_DirStartVal = -1;
  m_IsRunning = false;
  m_SetNewData = 0;
  m_SampleCount = 0;
  m_LeftLegend = 3;
  m_RightLegend = 3;
  for (int idx = 0; idx < WIND_RECORD_COUNT; idx++) {
    m_ArrayWindDirHistory[idx] = -1;
    m_ArrayWindSpdHistory[idx] = -1;
    m_ExpSmoothArrayWindSpd[idx] = -1;
    m_ExpSmoothArrayWindDir[idx] = -1;
    m_ArrayRecTime[idx] = wxDateTime::Now().GetTm();
    m_ArrayRecTime[idx].year = 999;
  }
  alpha = 0.01;  // smoothing constant
  m_WindowRect = GetClientRect();
  m_DrawAreaRect = GetClientRect();
  m_DrawAreaRect.SetHeight(m_WindowRect.height - m_TopLineHeight -
                           m_TitleHeight);
}

wxSize DashboardInstrument_WindDirHistory::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  wxFont f;
  if (m_Properties)
      f = m_Properties->m_TitelFont.GetChosenFont();
  else
      f = g_pFontTitle->GetChosenFont();
  // Use a dummy for default min width instead of m_title
  wxString widthdummy = "Left Space TWS 25.5 kn TWD 320 right s";
  dc.GetTextExtent(widthdummy, &w, &m_TitleHeight, 0, 0, &f);
  if (orient == wxHORIZONTAL) {
    return wxSize(DefaultWidth, wxMax(m_TitleHeight + 140, hint.y));
  } else {
    return wxSize(wxMax(hint.x, w), wxMax(m_TitleHeight + 140, hint.y));
  }
}

void DashboardInstrument_WindDirHistory::SetData(DASH_CAP st, double data,
                                                 wxString unit) {
  if (st == OCPN_DBP_STC_TWD || st == OCPN_DBP_STC_TWS) {
    if (m_SetNewData < 1) {
      if (st == OCPN_DBP_STC_TWD) {
        if (std::isnan(data)) {
          // This NAN is from the one Watchdog used to reset wind history graph
          ResetData();
          m_WindSpd = m_WindDir = NAN;
        }
        else {
          m_WindDir = data;
          if (m_DirRecCnt <= 5) {
            m_DirStartVal += data;
            m_DirRecCnt++;
          }
        }
      }
      if (st == OCPN_DBP_STC_TWS && !std::isnan(data) && data < 200.0) {
        m_WindSpd = data;
        // if unit changes, reset everything ...
        if (unit != m_WindSpeedUnit && m_WindSpeedUnit != _("-")) {
          ResetData();
        }
        m_WindSpeedUnit = unit;
        if (m_SpdRecCnt <= 5) {
          m_SpdStartVal += data;
          m_SpdRecCnt++;
        }
      }
      if (m_SpdRecCnt == 5 && m_DirRecCnt == 5) {
        m_WindSpd = m_SpdStartVal / 5;
        m_WindDir = m_DirStartVal / 5;
        m_oldDirVal = m_WindDir;  // make sure we don't get a diff > or <180 in
        // the initial run
      }
      // start working after we collected 5 records each, as start values for the
      // smoothed curves
      if (m_SpdRecCnt > 5 && m_DirRecCnt > 5) {
        m_IsRunning = true;
        m_SampleCount = m_SampleCount < WIND_RECORD_COUNT ? m_SampleCount + 1
          : WIND_RECORD_COUNT;
        m_MaxWindDir = 0;
        m_MinWindDir = 360;
        m_MaxWindSpd = 0;
        // data shifting
        for (int idx = 1; idx < WIND_RECORD_COUNT; idx++) {
          if (WIND_RECORD_COUNT - m_SampleCount <= idx)
            m_MinWindDir = wxMin(m_ArrayWindDirHistory[idx], m_MinWindDir);
          m_MaxWindDir = wxMax(m_ArrayWindDirHistory[idx - 1], m_MaxWindDir);
          m_MaxWindSpd = wxMax(m_ArrayWindSpdHistory[idx - 1], m_MaxWindSpd);
          m_ArrayWindDirHistory[idx - 1] = m_ArrayWindDirHistory[idx];
          m_ArrayWindSpdHistory[idx - 1] = m_ArrayWindSpdHistory[idx];
          m_ExpSmoothArrayWindSpd[idx - 1] = m_ExpSmoothArrayWindSpd[idx];
          m_ExpSmoothArrayWindDir[idx - 1] = m_ExpSmoothArrayWindDir[idx];
          m_ArrayRecTime[idx - 1] = m_ArrayRecTime[idx];
        }
        double diff = m_WindDir - m_oldDirVal;
        if (diff < -270) {
          m_WindDir += 360;
        }
        else if (diff > 270) {
          m_WindDir -= 360;
        }
        m_ArrayWindDirHistory[WIND_RECORD_COUNT - 1] = m_WindDir;
        m_ArrayWindSpdHistory[WIND_RECORD_COUNT - 1] = m_WindSpd;
        if (m_SampleCount < 2) {
          m_ArrayWindSpdHistory[WIND_RECORD_COUNT - 2] = m_WindSpd;
          m_ExpSmoothArrayWindSpd[WIND_RECORD_COUNT - 2] = m_WindSpd;
          m_ArrayWindDirHistory[WIND_RECORD_COUNT - 2] = m_WindDir;
          m_ExpSmoothArrayWindDir[WIND_RECORD_COUNT - 2] = m_WindDir;
        }
        m_ExpSmoothArrayWindSpd[WIND_RECORD_COUNT - 1] =
          alpha * m_ArrayWindSpdHistory[WIND_RECORD_COUNT - 2] +
          (1 - alpha) * m_ExpSmoothArrayWindSpd[WIND_RECORD_COUNT - 2];
        m_ExpSmoothArrayWindDir[WIND_RECORD_COUNT - 1] =
          alpha * m_ArrayWindDirHistory[WIND_RECORD_COUNT - 2] +
          (1 - alpha) * m_ExpSmoothArrayWindDir[WIND_RECORD_COUNT - 2];
        m_ArrayRecTime[WIND_RECORD_COUNT - 1] = wxDateTime::Now().GetTm();
        m_oldDirVal = m_ExpSmoothArrayWindDir[WIND_RECORD_COUNT - 1];
        // include the new/latest value in the max/min value test too
        m_MaxWindDir = wxMax(m_WindDir, m_MaxWindDir);
        m_MinWindDir = wxMin(m_WindDir, m_MinWindDir);
        m_MaxWindSpd = wxMax(m_WindSpd, m_MaxWindSpd);
        // get the overall max Wind Speed
        m_TotalMaxWindSpd = wxMax(m_WindSpd, m_TotalMaxWindSpd);

        // set wind angle scale to full +/- 90 degr depending on the real max/min
        // value recorded
        SetMinMaxWindScale();
        // Wait two times until new data.
        m_SetNewData = 2;
      }
    }
    else m_SetNewData--;
  }
}

void DashboardInstrument_WindDirHistory::ResetData() {
  m_MaxWindDir = -1;
  m_WindDir = -1;
  m_WindDirRange = 90;
  m_MaxWindSpd = 0;
  m_TotalMaxWindSpd = 0;
  m_WindSpd = 0;
  m_SpdRecCnt = 0;
  m_DirRecCnt = 0;
  m_SpdStartVal = -1;
  m_DirStartVal = -1;
  m_IsRunning = false;
  m_SetNewData = 0;
  m_SampleCount = 0;
  m_LeftLegend = 3;
  m_RightLegend = 3;
  for (int idx = 0; idx < WIND_RECORD_COUNT; idx++) {
    m_ArrayWindDirHistory[idx] = -1;
    m_ArrayWindSpdHistory[idx] = -1;
    m_ExpSmoothArrayWindSpd[idx] = -1;
    m_ExpSmoothArrayWindDir[idx] = -1;
    m_ArrayRecTime[idx] = wxDateTime::Now().GetTm();
    m_ArrayRecTime[idx].year = 999;
  }
}

void DashboardInstrument_WindDirHistory::Draw(wxGCDC* dc) {
  m_WindowRect = GetClientRect();
  m_DrawAreaRect = GetClientRect();
  m_DrawAreaRect.SetHeight(m_WindowRect.height - m_TopLineHeight -
                           m_TitleHeight);
  m_DrawAreaRect.SetX(m_LeftLegend + 3);
  DrawBackground(dc);
  DrawForeground(dc);
}

//*********************************************************************************
// determine and set  min and max values for the direction
//*********************************************************************************
void DashboardInstrument_WindDirHistory::SetMinMaxWindScale() {
  // set wind direction legend to full +/- 90 degr depending on the real max/min
  // value recorded example : max wind dir. = 45 degr  ==> max = 90 degr
  //           min wind dir. = 45 degr  ==> min = 0 degr
  // first calculate the max wind direction
  int fulldeg = m_MaxWindDir / 90;  // we explicitly chop off the decimals by
                                    // type conversion from double to int !
  if (fulldeg == 0)
    fulldeg = m_MaxWindDir < 0 ? 0 : 1;
  else if (m_MaxWindDir > 0)
    fulldeg += 1;
  m_MaxWindDir = fulldeg * 90;
  // now calculate the min wind direction
  fulldeg = m_MinWindDir / 90;
  if (fulldeg == 0)
    fulldeg = m_MinWindDir < 0 ? -1 : 0;
  else
    fulldeg = m_MinWindDir > 0 ? fulldeg : (fulldeg - 1);
  m_MinWindDir = fulldeg * 90;

  // limit the visible wind dir range to 360  degr remove the extra range on the
  // opposite side of the current wind dir value
  m_WindDirRange = m_MaxWindDir - m_MinWindDir;
  if (m_WindDirRange > 360) {
    int diff2min =
        m_WindDir - m_MinWindDir;  // diff between min value and current value
    int diff2max =
        m_MaxWindDir - m_WindDir;  // diff between max value and current value
    if (diff2min > diff2max) {
      while (m_WindDirRange > 360) {
        m_MinWindDir += 90;
        m_WindDirRange = m_MaxWindDir - m_MinWindDir;
      }
    }
    if (diff2min < diff2max) {
      while (m_WindDirRange > 360) {
        m_MaxWindDir -= 90;
        m_WindDirRange = m_MaxWindDir - m_MinWindDir;
      }
    }
  }
}
//*********************************************************************************
// wind direction legend
//*********************************************************************************
void DashboardInstrument_WindDirHistory::DrawWindDirScale(wxGCDC* dc) {
  wxString label1, label2, label3, label4, label5;
  wxColour cl;
  wxPen pen;
  int width, height;
  cl = wxColour(204, 41, 41, 255);  // red, opague
  if (m_Properties)
  {
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_SmallFont.GetColour()));
      dc->SetFont(m_Properties->m_SmallFont.GetChosenFont());
  }
  else
  {
      dc->SetTextForeground(GetColourSchemeFont(g_pFontSmall->GetColour()));
      dc->SetFont(g_pFontSmall->GetChosenFont());
  }
  if (!m_IsRunning) {
    label1 = _T("---");
    label2 = _T("---");
    label3 = _T("---");
    label4 = _T("---");
    label5 = _T("---");
  } else {
    // label 1 : legend for bottom line. By definition always w/o decimals
    double tempdir = m_MinWindDir;
    while (tempdir < 0) tempdir += 360;
    while (tempdir >= 360) tempdir -= 360;
    label1 = GetWindDirStr(wxString::Format(_T("%.1f"), tempdir));
    // label 2 : 1/4
    tempdir = m_MinWindDir + m_WindDirRange / 4.;
    while (tempdir < 0) tempdir += 360;
    while (tempdir >= 360) tempdir -= 360;
    label2 = GetWindDirStr(wxString::Format(_T("%.1f"), tempdir));
    // label 3 : legend for center line
    tempdir = m_MinWindDir + m_WindDirRange / 2;
    while (tempdir < 0) tempdir += 360;
    while (tempdir >= 360) tempdir -= 360;
    label3 = GetWindDirStr(wxString::Format(_T("%.1f"), tempdir));
    // label 4 :  3/4
    tempdir = m_MinWindDir + m_WindDirRange * 0.75;
    while (tempdir < 0) tempdir += 360;
    while (tempdir >= 360) tempdir -= 360;
    label4 = GetWindDirStr(wxString::Format(_T("%.1f"), tempdir));
    // label 5 : legend for top line
    tempdir = m_MaxWindDir;
    while (tempdir < 0) tempdir += 360;
    while (tempdir >= 360) tempdir -= 360;
    label5 = GetWindDirStr(wxString::Format(_T("%.1f"), tempdir));
  }
  // draw the legend with the labels; find the widest string and store it in
  // m_RightLegend.
  // m_RightLegend is the basis for the horizontal lines !
  wxFont f;
  if (m_Properties)
  {
      f = m_Properties->m_SmallFont.GetChosenFont();
      dc->GetTextExtent(label5, &width, &height, 0, 0, &f);
      m_RightLegend = width;
      dc->GetTextExtent(label4, &width, &height, 0, 0, &f);
      m_RightLegend = wxMax(width, m_RightLegend);
      dc->GetTextExtent(label3, &width, &height, 0, 0, &f);
      m_RightLegend = wxMax(width, m_RightLegend);
      dc->GetTextExtent(label2, &width, &height, 0, 0, &f);
      m_RightLegend = wxMax(width, m_RightLegend);
      dc->GetTextExtent(label1, &width, &height, 0, 0, &f);
      m_RightLegend = wxMax(width, m_RightLegend);
  }
  else
  {
      f = g_pFontSmall->GetChosenFont();
      dc->GetTextExtent(label5, &width, &height, 0, 0, &f);
      m_RightLegend = width;
      dc->GetTextExtent(label4, &width, &height, 0, 0, &f);
      m_RightLegend = wxMax(width, m_RightLegend);
      dc->GetTextExtent(label3, &width, &height, 0, 0, &f);
      m_RightLegend = wxMax(width, m_RightLegend);
      dc->GetTextExtent(label2, &width, &height, 0, 0, &f);
      m_RightLegend = wxMax(width, m_RightLegend);
      dc->GetTextExtent(label1, &width, &height, 0, 0, &f);
      m_RightLegend = wxMax(width, m_RightLegend);
  }
  m_RightLegend += 4;  // leave some space to the edge
  dc->DrawText(label5, m_WindowRect.width - m_RightLegend,
               m_TopLineHeight - height / 2);
  dc->DrawText(label4, m_WindowRect.width - m_RightLegend,
               (int)(m_TopLineHeight + m_DrawAreaRect.height / 4 - height / 2));
  dc->DrawText(label3, m_WindowRect.width - m_RightLegend,
               (int)(m_TopLineHeight + m_DrawAreaRect.height / 2 - height / 2));
  dc->DrawText(
      label2, m_WindowRect.width - m_RightLegend,
      (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.75 - height / 2));
  dc->DrawText(label1, m_WindowRect.width - m_RightLegend,
               (int)(m_TopLineHeight + m_DrawAreaRect.height - height / 2));
}

//*********************************************************************************
// draw wind speed scale
//*********************************************************************************
void DashboardInstrument_WindDirHistory::DrawWindSpeedScale(wxGCDC* dc) {
  wxString label1, label2, label3, label4, label5;
  wxColour cl;
  int width, height;
  double val1;
  double WindSpdScale;

  //cl = wxColour(61, 61, 204, 255);
  if (m_Properties)
  {
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_SmallFont.GetColour()));
      dc->SetFont(m_Properties->m_SmallFont.GetChosenFont());
  }
  else
  {      
      dc->SetTextForeground(GetColourSchemeFont(g_pFontSmall->GetColour()));
      dc->SetFont(g_pFontSmall->GetChosenFont());
  }
  // round maxWindSpd up to the next full knot; nicer view ...
  m_MaxWindSpdScale = (int)m_MaxWindSpd + 1;
  if (!m_IsRunning) {
    label1.Printf(_("--- %s"), m_WindSpeedUnit.c_str());
    label2 = label1;
    label3 = label1;
    label4 = label1;
    label5 = label1;
  } else {
    /*we round the speed up to the next full knot ==> the top and bottom line
     have full numbers as legend (e.g. 23 kn -- 0 kn) but the intermediate lines
     may have decimal values (e.g. center line : 23/2=11.5 or quarter line
     23/4=5.75), so in worst case we end up with 23 - 17.25 - 11.5 - 5.75 - 0
     The goal is to draw the legend with decimals only, if we really have them !
    */
    // top legend for max wind
    label1.Printf(_T("%.0f %s"), m_MaxWindSpdScale, m_WindSpeedUnit.c_str());
    // 3/4 legend
    WindSpdScale = m_MaxWindSpdScale * 3. / 4.;
    // do we need a decimal ?
    val1 = (int)((WindSpdScale - (int)WindSpdScale) * 100);
    if (val1 == 25 || val1 == 75)  // it's a .25 or a .75
      label2.Printf(_T("%.2f %s"), WindSpdScale, m_WindSpeedUnit.c_str());
    else if (val1 == 50)
      label2.Printf(_T("%.1f %s"), WindSpdScale, m_WindSpeedUnit.c_str());
    else
      label2.Printf(_T("%.0f %s"), WindSpdScale, m_WindSpeedUnit.c_str());
    // center legend
    WindSpdScale = m_MaxWindSpdScale / 2.;
    // center line can either have a .0 or .5 value !
    if ((int)(WindSpdScale * 10) % 10 == 5)
      label3.Printf(_T("%.1f %s"), WindSpdScale, m_WindSpeedUnit.c_str());
    else
      label3.Printf(_T("%.0f %s"), WindSpdScale, m_WindSpeedUnit.c_str());

    // 1/4 legend
    WindSpdScale = m_MaxWindSpdScale / 4.;
    // do we need a decimal ?
    val1 = (int)((WindSpdScale - (int)WindSpdScale) * 100);
    if (val1 == 25 || val1 == 75)
      label4.Printf(_T("%.2f %s"), WindSpdScale, m_WindSpeedUnit.c_str());
    else if (val1 == 50)
      label4.Printf(_T("%.1f %s"), WindSpdScale, m_WindSpeedUnit.c_str());
    else
      label4.Printf(_T("%.0f %s"), WindSpdScale, m_WindSpeedUnit.c_str());

    // bottom legend for min wind, always 0
    label5.Printf(_T("%.0f %s"), 0.0, m_WindSpeedUnit.c_str());
  }
  wxFont f;
  if (m_Properties)
      f = m_Properties->m_SmallFont.GetChosenFont();
  else
      f = g_pFontSmall->GetChosenFont();
  dc->GetTextExtent(label1, &m_LeftLegend, &height, 0, 0, &f);
  dc->DrawText(label1, 4, (int)(m_TopLineHeight - height / 2));
  if (m_Properties)
      f = m_Properties->m_SmallFont.GetChosenFont();
  else
      f = g_pFontSmall->GetChosenFont();
  dc->GetTextExtent(label2, &width, &height, 0, 0, &f);
  dc->DrawText(label2, 4,
               (int)(m_TopLineHeight + m_DrawAreaRect.height / 4 - height / 2));
  m_LeftLegend = wxMax(width, m_LeftLegend);
  if (m_Properties)
      f = m_Properties->m_SmallFont.GetChosenFont();
  else
      f = g_pFontSmall->GetChosenFont();
  dc->GetTextExtent(label3, &width, &height, 0, 0, &f);
  dc->DrawText(label3, 4,
               (int)(m_TopLineHeight + m_DrawAreaRect.height / 2 - height / 2));
  m_LeftLegend = wxMax(width, m_LeftLegend);
  if (m_Properties)
      f = m_Properties->m_SmallFont.GetChosenFont();
  else
      f = g_pFontSmall->GetChosenFont();
  dc->GetTextExtent(label4, &width, &height, 0, 0, &f);
  dc->DrawText(
      label4, 4,
      (int)(m_TopLineHeight + m_DrawAreaRect.height * 0.75 - height / 2));
  m_LeftLegend = wxMax(width, m_LeftLegend);
  if (m_Properties)
      f = m_Properties->m_SmallFont.GetChosenFont();
  else
      f = g_pFontSmall->GetChosenFont();
  dc->GetTextExtent(label5, &width, &height, 0, 0, &f);
  dc->DrawText(label5, 4,
               (int)(m_TopLineHeight + m_DrawAreaRect.height - height / 2));
  m_LeftLegend = wxMax(width, m_LeftLegend);
  m_LeftLegend += 4;
}

//*********************************************************************************
// draw background
//*********************************************************************************
void DashboardInstrument_WindDirHistory::DrawBackground(wxGCDC* dc) {
  wxString label, label1, label2, label3, label4, label5;
  wxColour cl;
  wxPen pen;
  //---------------------------------------------------------------------------------
  // draw legends for speed and direction
  //---------------------------------------------------------------------------------
  DrawWindDirScale(dc);
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

//***********************************************************************************
// convert numerical wind direction values to text, used in the wind direction
// legend
//***********************************************************************************
wxString DashboardInstrument_WindDirHistory::GetWindDirStr(wxString WindDir) {
  if (WindDir == _T("0.0") || WindDir == _T("360.0"))
    return _("N");
  else if (WindDir == _T("22.5"))
    return _("NNE");
  else if (WindDir == _T("45.0"))
    return _("NE");
  else if (WindDir == _T("67.5"))
    return _("ENE");
  else if (WindDir == _T("90.0"))
    return _("E");
  else if (WindDir == _T("112.5"))
    return _("ESE");
  else if (WindDir == _T("135.0"))
    return _("SE");
  else if (WindDir == _T("157.5"))
    return _("SSE");
  else if (WindDir == _T("180.0"))
    return _("S");
  else if (WindDir == _T("202.5"))
    return _("SSW");
  else if (WindDir == _T("225.0"))
    return _("SW");
  else if (WindDir == _T("247.5"))
    return _("WSW");
  else if (WindDir == _T("270.0"))
    return _("W");
  else if (WindDir == _T("292.5"))
    return _("WNW");
  else if (WindDir == _T("315.0"))
    return _("NW");
  else if (WindDir == _T("337.5"))
    return _("NNW");
  else
    return WindDir;
}

//*********************************************************************************
// draw foreground
//*********************************************************************************
void DashboardInstrument_WindDirHistory::DrawForeground(wxGCDC* dc) {
  wxColour col;
  double ratioH;
  int width, height, min, hour;
  double dir;
  wxString WindAngle, WindSpeed;
  wxPen pen;
  wxString label;

  //---------------------------------------------------------------------------------
  // wind direction
  //---------------------------------------------------------------------------------
  dc->SetFont((g_pFontSmall->GetChosenFont()));
  col = wxColour(204, 41, 41, 255);  // red, opaque Set TWD to DataFonf + Color, TWS to LabelFOnt + Color
  if (m_Properties)
  {
      dc->SetFont(m_Properties->m_SmallFont.GetChosenFont());
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_SmallFont.GetColour()));
  }
  else
  {
      dc->SetFont(g_pFontSmall->GetChosenFont());
      if (GetColourSchemeFont(g_pFontLabel->GetColour()) == GetColourSchemeFont(g_pFontLabel->GetColour()))
          dc->SetTextForeground(col);
      else
        dc->SetTextForeground(GetColourSchemeFont(g_pFontSmall->GetColour()));
  }
  if (!m_IsRunning)
    WindAngle = _T("TWD --- ");
  else {
    dir = m_WindDir;
    while (dir > 360) dir -= 360;
    while (dir < 0) dir += 360;
    if (!std::isnan(dir))
      WindAngle = wxString::Format(_T("TWD %3.0f"), dir) + DEGREE_SIGN;
    else
      WindAngle = wxString::Format(_T("TWD ---")) + DEGREE_SIGN;
  }
  wxFont f;
  if (m_Properties)
      f = m_Properties->m_SmallFont.GetChosenFont();
  else
      f = g_pFontLabel->GetChosenFont();

  dc->GetTextExtent(WindAngle, &degw, &degh, 0, 0, &f);
  dc->DrawText(WindAngle, m_WindowRect.width - m_RightLegend - degw, 3);

  pen.SetStyle(wxPENSTYLE_SOLID);
  if (m_Properties) {
      #if wxCHECK_VERSION(3, 1, 6)
      unsigned int r = GetColourSchemeFont(m_Properties->m_DataFont.GetColour()).GetRed();
      unsigned int g = GetColourSchemeFont(m_Properties->m_DataFont.GetColour()).GetGreen();
      unsigned int b = GetColourSchemeFont(m_Properties->m_DataFont.GetColour()).GetBlue();
      #else
      unsigned int r = GetColourSchemeFont(m_Properties->m_DataFont.GetColour()).Red();
      unsigned int g = GetColourSchemeFont(m_Properties->m_DataFont.GetColour()).Green();
      unsigned int b = GetColourSchemeFont(m_Properties->m_DataFont.GetColour()).Blue();
      #endif
      pen.SetColour(wxColour(r, g, b, 96)); // transparent
  } else {
      if (GetColourSchemeFont(g_pFontData->GetColour()) == GetColourSchemeFont(g_pFontLabel->GetColour()))
          pen.SetColour(wxColour(204, 41, 41, 96));
      else {
          #if wxCHECK_VERSION(3, 1, 6)
          unsigned int r = GetColourSchemeFont(g_pFontData->GetColour()).GetRed();
          unsigned int g = GetColourSchemeFont(g_pFontData->GetColour()).GetGreen();
          unsigned int b = GetColourSchemeFont(g_pFontData->GetColour()).GetBlue();
          #else
          unsigned int r = GetColourSchemeFont(g_pFontData->GetColour()).Red();
          unsigned int g = GetColourSchemeFont(g_pFontData->GetColour()).Green();
          unsigned int b = GetColourSchemeFont(g_pFontData->GetColour()).Blue();
          #endif
          pen.SetColour(wxColour(r, g, b, 96)); // transparent
      }
  }
    //pen.SetColour(wxColour(204, 41, 41, 96));  // red, transparent
  pen.SetWidth(1);
  dc->SetPen(pen);
  ratioH = (double)m_DrawAreaRect.height / m_WindDirRange;
  m_DrawAreaRect.SetWidth(m_WindowRect.width - 6 - m_LeftLegend -
                          m_RightLegend);
  m_ratioW = double(m_DrawAreaRect.width) / (WIND_RECORD_COUNT - 1);

  //---------------------------------------------------------------------------------
  // live direction data
  //---------------------------------------------------------------------------------
  wxPoint points[WIND_RECORD_COUNT + 2];
  wxPoint wdDraw[WIND_RECORD_COUNT + 2];
  int ld = 0;
  wdDraw[ld].x = m_ratioW + 3 + m_LeftLegend;
  wdDraw[ld].y = m_TopLineHeight + m_DrawAreaRect.height -
    (m_ArrayWindDirHistory[1] - m_MinWindDir) * ratioH;

  for (int idx = 1; idx < WIND_RECORD_COUNT; idx++) {
    points[idx].x = idx * m_ratioW + 3 + m_LeftLegend;
    points[idx].y = m_TopLineHeight + m_DrawAreaRect.height -
                    (m_ArrayWindDirHistory[idx] - m_MinWindDir) * ratioH;
    if (WIND_RECORD_COUNT - m_SampleCount <= idx &&
                 points[idx].y > m_TopLineHeight &&
                 points[idx].y <= m_TopLineHeight + m_DrawAreaRect.height) {
      wdDraw[ld] = points[idx];
      ld++;
    }
  }
  if (ld > 1)
    dc->DrawLines(ld, wdDraw);

  //---------------------------------------------------------------------------------
  // exponential smoothing of direction
  //---------------------------------------------------------------------------------
  pen.SetStyle(wxPENSTYLE_SOLID);
  //pen.SetColour(wxColour(204, 41, 41, 255));
  if (m_Properties)
      pen.SetColour(GetColourSchemeFont(m_Properties->m_DataFont.GetColour()));
  else
  {
      if (GetColourSchemeFont(g_pFontData->GetColour()) == GetColourSchemeFont(g_pFontLabel->GetColour()))
          pen.SetColour(wxColour(204, 41, 41, 255));
      else
          pen.SetColour(GetColourSchemeFont(g_pFontData->GetColour()));
  }
  pen.SetWidth(2);
  dc->SetPen(pen);

  ld = 0;
  wdDraw[ld].x = m_ratioW + 3 + m_LeftLegend;
  wdDraw[ld].y = m_TopLineHeight + m_DrawAreaRect.height -
    (m_ExpSmoothArrayWindDir[ld] - m_MinWindDir) * ratioH;

  for (int idx = 1; idx < WIND_RECORD_COUNT; idx++) {
    points[idx].x = idx * m_ratioW + 3 + m_LeftLegend;
    points[idx].y = m_TopLineHeight + m_DrawAreaRect.height -
                    (m_ExpSmoothArrayWindDir[idx] - m_MinWindDir) * ratioH;
    if (WIND_RECORD_COUNT - m_SampleCount <= idx &&
      points[idx].y > m_TopLineHeight &&
      points[idx].y <= m_TopLineHeight + m_DrawAreaRect.height) {
      wdDraw[ld] = points[idx];
      ld++;
    }
  }
  if (ld > 1)
    dc->DrawLines(ld, wdDraw);

  //---------------------------------------------------------------------------------
  // wind speed
  //---------------------------------------------------------------------------------
  col = wxColour(61, 61, 204, 255);  // blue, opaque
  if (m_Properties)
  {
      dc->SetFont(m_Properties->m_SmallFont.GetChosenFont());
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_SmallFont.GetColour()));
  }
  else
  {
    dc->SetFont(g_pFontSmall->GetChosenFont());
    if (GetColourSchemeFont(g_pFontSmall->GetColour()) ==
        GetColourSchemeFont(g_pFontSmall->GetColour()))
          dc->SetTextForeground(col);
      else
      dc->SetTextForeground(GetColourSchemeFont(g_pFontSmall->GetColour()));
  }
  if (!std::isnan(m_WindSpd))
    WindSpeed = wxString::Format(_T("TWS %3.1f %s "), m_WindSpd,
                                 m_WindSpeedUnit.c_str());
  else
    WindSpeed = wxString::Format(_T("TWS --- %s "), m_WindSpeedUnit.c_str());
  if (m_Properties) {
      f =m_Properties->m_LabelFont.GetChosenFont();
    dc->GetTextExtent(WindSpeed, &speedw, &degh, 0, 0, &f);
  } else {
    f = g_pFontSmall->GetChosenFont();
    dc->GetTextExtent(WindSpeed, &speedw, &degh, 0, 0, &f);
  }
  dc->DrawText(WindSpeed, m_LeftLegend, 3);

  dc->SetFont((g_pFontSmall->GetChosenFont()));
  int labelw, labelh;
  if (m_Properties)
  {
    f = m_Properties->m_SmallFont.GetChosenFont();
      dc->GetTextExtent(WindSpeed, &labelw, &labelh, 0, 0, &f);
    dc->SetFont(m_Properties->m_SmallFont.GetChosenFont());
      dc->SetTextForeground(
          GetColourSchemeFont(m_Properties->m_SmallFont.GetColour()));
  }
  else
  {
    f = g_pFontSmall->GetChosenFont();
      dc->GetTextExtent(WindSpeed, &labelw, &labelh, 0, 0, &f);
    dc->SetFont(g_pFontSmall->GetChosenFont());
      dc->SetTextForeground(GetColourSchemeFont(g_pFontSmall->GetColour()));
  }
  // determine the time range of the available data (=oldest data value)
  int i = 0;
  while (m_ArrayRecTime[i].year == 999 && i < WIND_RECORD_COUNT - 1) i++;
  if (i == WIND_RECORD_COUNT - 1) {
    min = 0;
    hour = 0;
  } else {
    wxDateTime localTime(m_ArrayRecTime[i]);
    min = localTime.GetMinute();
    hour = localTime.GetHour();
  }

  wxString statistics = wxString::Format(_(" Max %.1f %s since %02d:%02d  Overall %.1f"),
                       m_MaxWindSpd, m_WindSpeedUnit.c_str(), hour, min,
                       m_TotalMaxWindSpd, m_WindSpeedUnit.c_str());
  int statw, stath;
  dc->GetTextExtent(statistics, &statw, &stath, 0, 0, &f);
  if (statw <
      (m_WindowRect.width - m_LeftLegend - speedw - degw - m_RightLegend)) {
    dc->DrawText(statistics, speedw + m_LeftLegend, 3);
  }

  pen.SetStyle(wxPENSTYLE_SOLID);
  if (m_Properties) {
      #if wxCHECK_VERSION(3, 1, 6)
      unsigned int r = GetColourSchemeFont(m_Properties->m_LabelFont.GetColour()).GetRed();
      unsigned int g = GetColourSchemeFont(m_Properties->m_LabelFont.GetColour()).GetGreen();
      unsigned int b = GetColourSchemeFont(m_Properties->m_LabelFont.GetColour()).GetBlue();
      #else
      unsigned int r = GetColourSchemeFont(m_Properties->m_LabelFont.GetColour()).Red();
      unsigned int g = GetColourSchemeFont(m_Properties->m_LabelFont.GetColour()).Green();
      unsigned int b = GetColourSchemeFont(m_Properties->m_LabelFont.GetColour()).Blue();
      #endif
      pen.SetColour(wxColour(r, g, b, 96)); // transparent
  } else {
      if (GetColourSchemeFont(g_pFontData->GetColour()) == GetColourSchemeFont(g_pFontLabel->GetColour()))
          pen.SetColour(wxColour(61, 61, 204, 96)); // blue, transparent
      else {
          #if wxCHECK_VERSION(3, 1, 6)
          unsigned int r = GetColourSchemeFont(g_pFontLabel->GetColour()).GetRed();
          unsigned int g = GetColourSchemeFont(g_pFontLabel->GetColour()).GetGreen();
          unsigned int b = GetColourSchemeFont(g_pFontLabel->GetColour()).GetBlue();
          #else
          unsigned int r = GetColourSchemeFont(g_pFontLabel->GetColour()).Red();
          unsigned int g = GetColourSchemeFont(g_pFontLabel->GetColour()).Green();
          unsigned int b = GetColourSchemeFont(g_pFontLabel->GetColour()).Blue();
          #endif
          pen.SetColour(wxColour(r, g, b, 96)); // transparent
      }
  }
  //pen.SetColour(wxColour(61, 61, 204, 96));  // blue, transparent
  pen.SetWidth(1);
  dc->SetPen(pen);
  ratioH = (double)m_DrawAreaRect.height / m_MaxWindSpdScale;
  wxPoint pointsSpd[WIND_RECORD_COUNT + 2];
  wxPoint spdDraw[WIND_RECORD_COUNT + 2];

  //---------------------------------------------------------------------------------
  // live speed data
  //---------------------------------------------------------------------------------

  int ls = 0;
  spdDraw[ls].x = 1 * m_ratioW + 3 + m_LeftLegend;
  spdDraw[ls].y = m_TopLineHeight + m_DrawAreaRect.height -
                      m_ArrayWindSpdHistory[1] * ratioH;

  for (int idx = 1; idx < WIND_RECORD_COUNT; idx++) {
    pointsSpd[idx].x = idx * m_ratioW + 3 + m_LeftLegend;
    pointsSpd[idx].y = m_TopLineHeight + m_DrawAreaRect.height -
                       m_ArrayWindSpdHistory[idx] * ratioH;
    if (WIND_RECORD_COUNT - m_SampleCount <= idx &&
      pointsSpd[idx].y > m_TopLineHeight &&
      pointsSpd[idx].y <= m_TopLineHeight + m_DrawAreaRect.height) {
      spdDraw[ls] = pointsSpd[idx];
      ls++;
    }
  }
  if (ls > 1)
    dc->DrawLines(ls, spdDraw);

  //---------------------------------------------------------------------------------
  // exponential smoothing of speed
  //---------------------------------------------------------------------------------
  pen.SetStyle(wxPENSTYLE_SOLID);
  if (m_Properties)
      pen.SetColour(GetColourSchemeFont(m_Properties->m_LabelFont.GetColour()));
  else
  {
      if (GetColourSchemeFont(g_pFontData->GetColour()) == GetColourSchemeFont(g_pFontLabel->GetColour()))
          pen.SetColour(wxColour(61, 61, 204, 255));  // blue, opaque
      else
          pen.SetColour(GetColourSchemeFont(g_pFontLabel->GetColour()));
  }
  //pen.SetColour(wxColour(61, 61, 204, 255));  // blue, opaque
  pen.SetWidth(2);
  dc->SetPen(pen);
  ls = 0;
  spdDraw[ls].x = 1 * m_ratioW + 3 + m_LeftLegend;
  spdDraw[ls].y = m_TopLineHeight + m_DrawAreaRect.height -
                 m_ExpSmoothArrayWindSpd[1] * ratioH;

  for (int idx = 1; idx < WIND_RECORD_COUNT; idx++) {
    pointsSpd[idx].x = idx * m_ratioW + 3 + m_LeftLegend;
    pointsSpd[idx].y = m_TopLineHeight + m_DrawAreaRect.height -
                       m_ExpSmoothArrayWindSpd[idx] * ratioH;
    if (WIND_RECORD_COUNT - m_SampleCount <= idx &&
      pointsSpd[idx].y > m_TopLineHeight &&
      pointsSpd[idx].y <= m_TopLineHeight + m_DrawAreaRect.height) {
      spdDraw[ls] = pointsSpd[idx];
      ls++;
    }
  }
  if (ls > 1)
    dc->DrawLines(ls, spdDraw);

  //---------------------------------------------------------------------------------
  // draw vertical timelines every 10 minutes
  //---------------------------------------------------------------------------------
  GetGlobalColor(_T("DASHL"), &col);
  pen.SetColour(col);
  pen.SetStyle(wxPENSTYLE_DOT);
  dc->SetPen(pen);
  dc->SetTextForeground(col);
  dc->SetFont((g_pFontSmall->GetChosenFont()));
  int done = -1;
  wxPoint pointTime;
  for (int idx = 0; idx < WIND_RECORD_COUNT; idx++) {
    if (m_ArrayRecTime[idx].year != 999) {
      wxDateTime localTime(m_ArrayRecTime[idx]);
      hour = localTime.GetHour();
      min = localTime.GetMinute();
      if ((hour * 100 + min) != done && (min % 15 == 0)) {
        pointTime.x = idx * m_ratioW + 3 + m_LeftLegend;
        dc->DrawLine(pointTime.x, m_TopLineHeight + 1, pointTime.x,
                     (m_TopLineHeight + m_DrawAreaRect.height + 1));
        label.Printf(_T("%02d:%02d"), hour, min);
        f = g_pFontSmall->GetChosenFont();
        dc->GetTextExtent(label, &width, &height, 0, 0, &f);
        dc->DrawText(label, pointTime.x - width / 2,
                     m_WindowRect.height - height);
        done = hour * 100 + min;
      }
    }
  }
}
