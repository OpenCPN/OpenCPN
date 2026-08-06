/*************************************************************************
 *  Copyright (C) 2010 by Jean-Eudes Onfray                               *
 *  Copyright (C) 2010 by David S. Register                               *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program; if not, see <https://www.gnu.org/licenses/>. *
 *************************************************************************/

/**
 * \file
 *
 * Dashboard rudder angle instrument implementation.
 */

#include "rudder_angle.h"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

DashboardInstrument_RudderAngle::DashboardInstrument_RudderAngle(
    wxWindow* parent, wxWindowID id, wxString title,
    InstrumentProperties* Properties)
    : DashboardInstrument_Dial(parent, id, title, Properties, OCPN_DBP_STC_RSA,
                               100, 160, -40, +40) {
  // Default Rudder position is centered
  m_main_value = 0;

  // SetOptionMainValue("%3.0f Deg", DIAL_POSITION_BOTTOMLEFT);
  SetOptionMarker(5, DIAL_MARKER_REDGREEN, 2);
  // Labels are set static because we've no logic to display them this way
  wxString labels[] = {"40", "30", "20", "10", "0", "10", "20", "30", "40"};
  SetOptionLabel(10, DIAL_LABEL_HORIZONTAL, wxArrayString(9, labels));
  //      SetOptionExtraValue("%02.0f", DIAL_POSITION_INSIDE);
}

wxSize DashboardInstrument_RudderAngle::GetSize(int orient, wxSize hint) {
  InitTitleSize();

  int w;
  if (orient == wxHORIZONTAL) {
    w = wxMax(hint.y, (DefaultWidth - m_TitleHeight) / .7);
  } else {
    w = wxMax(hint.x, DefaultWidth);
  }
  return wxSize(w, m_TitleHeight + w * .7);
}

void DashboardInstrument_RudderAngle::SetData(DASH_CAP st, double data,
                                              wxString unit) {
  if (st == m_main_value_cap) {
    // Dial works clockwise but Rudder has negative values for left
    // and positive for right so we must inverse it.
    data = -data;

    if (data < m_main_value_min)
      m_main_value = m_main_value_min;
    else if (data > m_main_value_max)
      m_main_value = m_main_value_max;
    else
      m_main_value = data;
    m_main_value_unit = unit;
  } else if (st == m_extra_value_cap) {
    m_extra_value = data;
    m_extra_value_unit = unit;
  } else
    return;
}

void DashboardInstrument_RudderAngle::DrawFrame(wxGCDC* dc) {
  // We don't need the upper part
  // Move center up
  wxSize size = GetClientSize();
  wxColour cl;

  int drawHeight = GetDataBottom(size.y) - m_DataTop;
  InitTitleAndDataPosition(drawHeight);

  m_cx = size.x / 2;
  m_cy = m_DataTop + (drawHeight) * 0.38;
  m_radius = (drawHeight) * .6;

  dc->SetBrush(*wxTRANSPARENT_BRUSH);

  wxPen pen;
  pen.SetStyle(wxPENSTYLE_SOLID);
  pen.SetWidth(2);
  GetGlobalColor("DASHF", &cl);
  pen.SetColour(cl);
  dc->SetPen(pen);

  double angle1 = deg2rad(215);  // 305-ANGLE_OFFSET
  double angle2 = deg2rad(-35);  // 55-ANGLE_OFFSET
  wxCoord x1 = m_cx + (m_radius * cos(angle1));
  wxCoord y1 = m_cy + (m_radius * sin(angle1));
  wxCoord x2 = m_cx + (m_radius * cos(angle2));
  wxCoord y2 = m_cy + (m_radius * sin(angle2));
  dc->DrawArc(x1, y1, x2, y2, m_cx, m_cy);
  dc->DrawLine(x1, y1, x2, y2);
}

void DashboardInstrument_RudderAngle::DrawBackground(wxGCDC* dc) {
  wxCoord x = m_cx - (m_radius * 0.3);
  wxCoord y = m_cy - (m_radius * 1.1);
  wxColour cl;
  GetGlobalColor("DASH1", &cl);
  dc->SetBrush(cl);
  dc->DrawEllipticArc(x, y, m_radius * 0.6, m_radius * 1.4, 0, -180);
}
