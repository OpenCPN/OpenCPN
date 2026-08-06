/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2010 by Jean-Eudes Onfray                               *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Dashboard compass
 */

#include "compass.h"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

DashboardInstrument_Compass::DashboardInstrument_Compass(
    wxWindow* parent, wxWindowID id, wxString title,
    InstrumentProperties* Properties, DASH_CAP cap_flag)
    : DashboardInstrument_Dial(parent, id, title, Properties, cap_flag, 0, 360,
                               0, 360) {
  SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
  SetOptionLabel(20, DIAL_LABEL_ROTATED);
  SetOptionMainValue("%.0f", DIAL_POSITION_INSIDE);
}

void DashboardInstrument_Compass::SetData(DASH_CAP st, double data,
                                          wxString unit) {
  m_gps_wd = false;
  if (std::isnan(data)) m_gps_wd = true;

  if (st == m_main_value_cap) {
    // Rotate the rose
    m_angle_start = static_cast<int>(-data);
    // Required to display data
    m_main_value = data;
    m_main_value_unit = unit;
  } else if (st == m_extra_value_cap) {
    m_extra_value = data;
    m_extra_value_unit = unit;
  }
  Refresh();
}

void DashboardInstrument_Compass::DrawBackground(wxGCDC* dc) {
  DrawBoat(dc, m_cx, m_cy, m_radius);
  if (!m_gps_wd)  // Don't draw if no GPS
    DrawCompassRose(dc, m_cx, m_cy, static_cast<int>(0.7 * m_radius),
                    m_angle_start, true, m_Properties);
}

void DashboardInstrument_Compass::DrawForeground(wxGCDC* dc) {
  // We dont want the default foreground (arrow) drawn
}
