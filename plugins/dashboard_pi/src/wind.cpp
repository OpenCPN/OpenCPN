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


#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include "wind.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include <cmath>

#include "wx/tokenzr.h"

// Display the arrow for MainValue (wind angle)
// We also want the extra value (wind speed) displayed inside the dial

DashboardInstrument_Wind::DashboardInstrument_Wind(wxWindow* parent,
                                                   wxWindowID id,
                                                   wxString title,
                                                   InstrumentProperties* Properties,
                                                   DASH_CAP cap_flag)
    : DashboardInstrument_Dial(parent, id, title, Properties, cap_flag, 0, 360, 0, 360) {
  SetOptionMarker(10, DIAL_MARKER_REDGREENBAR, 3);
  // Labels are set static because we've no logic to display them this way
  wxString labels[] = {_T(""),    _T("30"),  _T("60"), _T("90"),
                       _T("120"), _T("150"), _T(""),   _T("150"),
                       _T("120"), _T("90"),  _T("60"), _T("30")};
  SetOptionLabel(30, DIAL_LABEL_HORIZONTAL, wxArrayString(12, labels));
}

void DashboardInstrument_Wind::DrawBackground(wxGCDC* dc) {
  DrawBoat(dc, m_cx, m_cy, m_radius);
}

DashboardInstrument_WindCompass::DashboardInstrument_WindCompass(
    wxWindow* parent, wxWindowID id, wxString title, InstrumentProperties* Properties, DASH_CAP cap_flag)
    : DashboardInstrument_Dial(parent, id, title, Properties, cap_flag, 0, 360, 0, 360) {
  SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
  wxString labels[] = {_("N"), _("NE"), _("E"), _("SE"),
                       _("S"), _("SW"), _("W"), _("NW")};
  SetOptionLabel(45, DIAL_LABEL_HORIZONTAL, wxArrayString(8, labels));
}

void DashboardInstrument_WindCompass::DrawBackground(wxGCDC* dc) {
  DrawCompassRose(dc, m_cx, m_cy, m_radius * 0.85, m_AngleStart, false, m_Properties);
}

// Display the arrow for MainValue (wind angle)
// We also want the extra value (wind speed) displayed inside the dial

DashboardInstrument_TrueWindAngle::DashboardInstrument_TrueWindAngle(
    wxWindow* parent, wxWindowID id, wxString title, InstrumentProperties* Properties, DASH_CAP cap_flag)
    : DashboardInstrument_Dial(parent, id, title, Properties, cap_flag, 0, 360, 0, 360) {
  SetOptionMarker(10, DIAL_MARKER_REDGREENBAR, 3);
  // Labels are set static because we've no logic to display them this way
  wxString labels[] = {_T(""),    _T("30"),  _T("60"), _T("90"),
                       _T("120"), _T("150"), _T(""),   _T("150"),
                       _T("120"), _T("90"),  _T("60"), _T("30")};
  SetOptionLabel(30, DIAL_LABEL_HORIZONTAL, wxArrayString(12, labels));
}

void DashboardInstrument_TrueWindAngle::DrawBackground(wxGCDC* dc) {
  DrawBoat(dc, m_cx, m_cy, m_radius);
}

/*****************************************************************************
  Apparent & True wind angle combined in one dial instrument
  Author: Thomas Rauch
******************************************************************************/
DashboardInstrument_AppTrueWindAngle::DashboardInstrument_AppTrueWindAngle(
    wxWindow* parent, wxWindowID id, wxString title, InstrumentProperties* Properties, DASH_CAP cap_flag)
    : DashboardInstrument_Dial(parent, id, title, Properties, cap_flag, 0, 360, 0, 360) {
  SetOptionMarker(10, DIAL_MARKER_REDGREENBAR, 3);
  // Labels are set static because we've no logic to display them this way
  wxString labels[] = {_T(""),    _T("30"),  _T("60"), _T("90"),
                       _T("120"), _T("150"), _T(""),   _T("150"),
                       _T("120"), _T("90"),  _T("60"), _T("30")};
  SetOptionLabel(30, DIAL_LABEL_HORIZONTAL, wxArrayString(12, labels));
}

void DashboardInstrument_AppTrueWindAngle::DrawBackground(wxGCDC* dc) {
  DrawBoat(dc, m_cx, m_cy, m_radius);
}

void DashboardInstrument_AppTrueWindAngle::SetData(DASH_CAP st, double data,
                                                   wxString unit) {
  if (st == OCPN_DBP_STC_TWA) {
    m_MainValueTrue = data;
    m_MainValueTrueUnit = unit;
    m_MainValueOption2 = DIAL_POSITION_BOTTOMLEFT;
  } else if (st == OCPN_DBP_STC_AWA) {
    m_MainValueApp = data;
    m_MainValueAppUnit = unit;
    m_MainValueOption1 = DIAL_POSITION_TOPLEFT;
  } else if (st == OCPN_DBP_STC_AWS) {
    m_ExtraValueApp = data;
    m_ExtraValueAppUnit = unit;
    m_ExtraValueOption1 = DIAL_POSITION_TOPRIGHT;
  } else if (st == OCPN_DBP_STC_TWS) {
    m_ExtraValueTrue = data;
    m_ExtraValueTrueUnit = unit;
    m_ExtraValueOption2 = DIAL_POSITION_BOTTOMRIGHT;
  }
  Refresh();
}
void DashboardInstrument_AppTrueWindAngle::Draw(wxGCDC* bdc) {


  if (m_Properties)
  {
      wxBrush b1(GetColourSchemeBackgroundColour(m_Properties->m_DataBackgroundColour));
      bdc->SetBackground(GetColourSchemeBackgroundColour(m_Properties->m_DataBackgroundColour));
  }
  else
  {
      wxColour c1;
      GetGlobalColor(_T("DASHB"), &c1);
      wxBrush b1(c1);
      bdc->SetBackground(b1);
  }
  bdc->Clear();

  wxSize size = GetClientSize();
  int width, height;
  wxFont f;
  if (m_Properties)
    f = m_Properties->m_LabelFont.GetChosenFont();
  else
    f = g_pFontLabel->GetChosenFont();
  bdc->GetTextExtent(_T("000"), &width, &height, 0, 0, &f);
  m_cx = size.x / 2;
  int availableHeight = size.y - m_TitleHeight - height;
  m_cy = m_TitleHeight + height / 2;
  m_cy += availableHeight / 2;
  m_radius = availableHeight / 2.0 * 0.95;

  DrawLabels(bdc);
  DrawFrame(bdc);
  DrawMarkers(bdc);
  DrawBackground(bdc);
  DrawData(bdc, m_MainValueApp, m_MainValueAppUnit, m_MainValueFormat,
           m_MainValueOption1);
  DrawData(bdc, m_MainValueTrue, m_MainValueTrueUnit, m_MainValueFormat,
           m_MainValueOption2);
  DrawData(bdc, m_ExtraValueApp, m_ExtraValueAppUnit, m_ExtraValueFormat,
           m_ExtraValueOption1);
  DrawData(bdc, m_ExtraValueTrue, m_ExtraValueTrueUnit, m_ExtraValueFormat,
           m_ExtraValueOption2);
  DrawForeground(bdc);
}
void DashboardInstrument_AppTrueWindAngle::DrawForeground(wxGCDC* dc) {
  wxPoint points[4];
  double data;
  double val;
  double value;
  // The default foreground is the arrow used in most dials
  wxColour cl;
  GetGlobalColor(_T("DASH2"), &cl);
  wxPen pen1;
  pen1.SetStyle(wxPENSTYLE_SOLID);
  pen1.SetColour(cl);
  pen1.SetWidth(2);
  dc->SetPen(pen1);
  GetGlobalColor(_T("DASH1"), &cl);
  wxBrush brush1;
  brush1.SetStyle(wxBRUSHSTYLE_SOLID);
  brush1.SetColour(cl);
  dc->SetBrush(brush1);
  dc->DrawCircle(m_cx, m_cy, m_radius / 8);

  /*True Wind*/
  dc->SetPen(*wxTRANSPARENT_PEN);
  if (m_Properties)
      cl = GetColourSchemeFont(m_Properties->m_Arrow_Second_Colour);
  else
      GetGlobalColor(_T("BLUE3"), &cl);
  wxBrush brush2;
  brush2.SetStyle(wxBRUSHSTYLE_SOLID);
  brush2.SetColour(cl);
  dc->SetBrush(brush2);

  /* this is fix for a +/-180? round instrument, when m_MainValue is supplied as
   * <0..180><L | R> for example TWA & AWA */
  if (m_MainValueTrueUnit == _T("\u00B0L"))
    data = 360 - m_MainValueTrue;
  else
    data = m_MainValueTrue;

  // The arrow should stay inside fixed limits
  if (data < m_MainValueMin)
    val = m_MainValueMin;
  else if (data > m_MainValueMax)
    val = m_MainValueMax;
  else
    val = data;

  value = deg2rad((val - m_MainValueMin) * m_AngleRange /
                  (m_MainValueMax - m_MainValueMin)) +
          deg2rad(m_AngleStart - ANGLE_OFFSET);

  points[0].x = m_cx + (m_radius * 0.95 * cos(value - .010));
  points[0].y = m_cy + (m_radius * 0.95 * sin(value - .010));
  points[1].x = m_cx + (m_radius * 0.95 * cos(value + .015));
  points[1].y = m_cy + (m_radius * 0.95 * sin(value + .015));
  points[2].x = m_cx + (m_radius * 0.22 * cos(value + 2.8));
  points[2].y = m_cy + (m_radius * 0.22 * sin(value + 2.8));
  points[3].x = m_cx + (m_radius * 0.22 * cos(value - 2.8));
  points[3].y = m_cy + (m_radius * 0.22 * sin(value - 2.8));
  dc->DrawPolygon(4, points, 0, 0);

  /* Apparent Wind*/
  dc->SetPen(*wxTRANSPARENT_PEN);
  if (m_Properties)
      cl = GetColourSchemeFont(m_Properties->m_Arrow_First_Colour);
  else
      GetGlobalColor(_T("DASHN"), &cl);
  wxBrush brush;
  brush.SetStyle(wxBRUSHSTYLE_SOLID);
  brush.SetColour(cl);
  dc->SetBrush(brush);

  /* this is fix for a +/-180? round instrument, when m_MainValue is supplied as
   * <0..180><L | R> for example TWA & AWA */
  if (m_MainValueAppUnit == _T("\u00B0L"))
    data = 360 - m_MainValueApp;
  else
    data = m_MainValueApp;

  // The arrow should stay inside fixed limits
  if (data < m_MainValueMin)
    val = m_MainValueMin;
  else if (data > m_MainValueMax)
    val = m_MainValueMax;
  else
    val = data;

  value = deg2rad((val - m_MainValueMin) * m_AngleRange /
                  (m_MainValueMax - m_MainValueMin)) +
          deg2rad(m_AngleStart - ANGLE_OFFSET);

  points[0].x = m_cx + (m_radius * 0.95 * cos(value - .010));
  points[0].y = m_cy + (m_radius * 0.95 * sin(value - .010));
  points[1].x = m_cx + (m_radius * 0.95 * cos(value + .015));
  points[1].y = m_cy + (m_radius * 0.95 * sin(value + .015));
  points[2].x = m_cx + (m_radius * 0.22 * cos(value + 2.8));
  points[2].y = m_cy + (m_radius * 0.22 * sin(value + 2.8));
  points[3].x = m_cx + (m_radius * 0.22 * cos(value - 2.8));
  points[3].y = m_cy + (m_radius * 0.22 * sin(value - 2.8));
  dc->DrawPolygon(4, points, 0, 0);
}
void DashboardInstrument_AppTrueWindAngle::DrawData(
    wxGCDC* dc, double value, wxString unit, wxString format,
    DialPositionOption position) {
  if (position == DIAL_POSITION_NONE) return;

  wxColour cl;
  if (m_Properties)
  {
      dc->SetFont(m_Properties->m_LabelFont.GetChosenFont());
      cl = GetColourSchemeFont(m_Properties->m_LabelFont.GetColour());
  }
  else
  {
      dc->SetFont(g_pFontLabel->GetChosenFont());
      cl = GetColourSchemeFont(g_pFontLabel->GetColour());
  }
  //GetGlobalColor(_T("DASHF"), &cl);
  dc->SetTextForeground(cl);

  wxSize size = GetClientSize();

  wxString text;
  if (!std::isnan(value)) {
    if (unit == _T("\u00B0"))
      text = wxString::Format(format, value) + DEGREE_SIGN;
    else if (unit == _T("\u00B0L"))  // No special display for now, might be
                                     // XX?< (as in text-only instrument)
      text = wxString::Format(format, value) + DEGREE_SIGN;
    else if (unit ==
             _T("\u00B0R"))  // No special display for now, might be >XX?
      text = wxString::Format(format, value) + DEGREE_SIGN;
    else if (unit == _T("\u00B0T"))
      text = wxString::Format(format, value) + DEGREE_SIGN + _T("T");
    else if (unit == _T("\u00B0M"))
      text = wxString::Format(format, value) + DEGREE_SIGN + _T("M");
    else if (unit == _T("N"))  // Knots
      text = wxString::Format(format, value) + _T(" Kts");
    else
      text = wxString::Format(format, value) + _T(" ") + unit;
  } else
    text = _T("---");

  int width, height;
  wxFont f;
  if (m_Properties)
    f = m_Properties->m_LabelFont.GetChosenFont();
  else
    f = g_pFontLabel->GetChosenFont();
  dc->GetMultiLineTextExtent(text, &width, &height, NULL, &f);

  wxRect TextPoint;
  TextPoint.width = width;
  TextPoint.height = height;
  wxColour c3;

  switch (position) {
    case DIAL_POSITION_NONE:
      // This case was already handled before, it's here just
      // to avoid compiler warning.
      return;
    case DIAL_POSITION_INSIDE: {
      TextPoint.x = m_cx - (width / 2) - 1;
      TextPoint.y = (size.y * .75) - height;
      GetGlobalColor(_T("DASHL"), &cl);
      int penwidth = size.x / 100;
      wxPen* pen =
          wxThePenList->FindOrCreatePen(cl, penwidth, wxPENSTYLE_SOLID);
      dc->SetPen(*pen);
      GetGlobalColor(_T("DASHB"), &cl);
      dc->SetBrush(cl);
      // There might be a background drawn below
      // so we must clear it first.
      dc->DrawRoundedRectangle(TextPoint.x - 2, TextPoint.y - 2, width + 4,
                               height + 4, 3);
      break;
    }
    case DIAL_POSITION_TOPLEFT:
      GetGlobalColor(_T("DASHN"), &c3);
      TextPoint.x = 0;
      TextPoint.y = m_TitleHeight;
      text = _T("A:") + text;
      break;
    case DIAL_POSITION_TOPRIGHT:
      GetGlobalColor(_T("DASHN"), &c3);
      TextPoint.x = size.x - width - 1;
      TextPoint.y = m_TitleHeight;
      break;
    case DIAL_POSITION_BOTTOMLEFT:
      GetGlobalColor(_T("BLUE3"), &c3);
      text = _T("T:") + text;
      TextPoint.x = 0;
      TextPoint.y = size.y - height;
      break;
    case DIAL_POSITION_BOTTOMRIGHT:
      GetGlobalColor(_T("BLUE3"), &c3);
      TextPoint.x = size.x - width - 1;
      TextPoint.y = size.y - height;
      break;
  }
  wxColour c2;
  GetGlobalColor(_T("DASHB"), &c2);
  wxStringTokenizer tkz(text, _T("\n"));
  wxString token;

  token = tkz.GetNextToken();
  while (token.Length()) {
    if (m_Properties) {
        f = m_Properties->m_LabelFont.GetChosenFont();
        dc->GetTextExtent(token, &width, &height, NULL, NULL, &f);
    } else {
        f = g_pFontLabel->GetChosenFont();
        dc->GetTextExtent(token, &width, &height, NULL, NULL, &f);
    }
    dc->DrawText(token, TextPoint.x, TextPoint.y);
    TextPoint.y += height;
    token = tkz.GetNextToken();
  }
}
