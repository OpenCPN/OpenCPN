/**************************************************************************
 *   Copyright (C) 2010 by Jean-Eudes Onfray                               *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Dashboard dial instrument.
 *
 * Inspired by original work from Andreas Heiming
 */

#ifndef Dial_H_
#define Dial_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "instrument.h"

#define ANGLE_OFFSET 90  // 0 degrees are at 12 o clock

typedef enum {
  DIAL_LABEL_NONE,
  DIAL_LABEL_HORIZONTAL,
  DIAL_LABEL_ROTATED
} DialLabelOption;

typedef enum {
  DIAL_MARKER_NONE,
  DIAL_MARKER_SIMPLE,
  DIAL_MARKER_REDGREEN,
  DIAL_MARKER_REDGREENBAR
} DialMarkerOption;

typedef enum {
  DIAL_POSITION_NONE,
  DIAL_POSITION_INSIDE,
  DIAL_POSITION_TOPLEFT,
  DIAL_POSITION_TOPRIGHT,
  DIAL_POSITION_BOTTOMLEFT,
  DIAL_POSITION_BOTTOMRIGHT,
  DIAL_POSITION_BOTTOMMIDDLE
} DialPositionOption;

extern double rad2deg(double angle);
extern double deg2rad(double angle);

//+------------------------------------------------------------------------------
//|
//| CLASS:
//|    DashboardInstrument_Dial
//|
//| DESCRIPTION:
//|    This class creates a speedometer style control
//|
//+------------------------------------------------------------------------------
class DashboardInstrument_Dial : public DashboardInstrument {
public:
  DashboardInstrument_Dial(wxWindow* parent, wxWindowID id, wxString title,
                           InstrumentProperties* Properties, DASH_CAP cap_flag,
                           int s_angle, int r_angle, int s_value, int e_value);

  ~DashboardInstrument_Dial() override = default;

  wxSize GetSize(int orient, wxSize hint) override;
  void SetData(DASH_CAP, double, wxString) override;
  void SetOptionMarker(double step, DialMarkerOption option, int offset) {
    m_marker_step = step;
    m_marker_option = option;
    m_marker_offset = offset;
  }
  void SetOptionLabel(double step, DialLabelOption option,
                      wxArrayString labels = wxArrayString()) {
    m_label_step = step;
    m_label_option = option;
    m_label_array = labels;
  }
  void SetOptionMainValue(wxString format, DialPositionOption option) {
    m_main_value_format = format;
    m_main_value_option = option;
  }
  void SetOptionExtraValue(DASH_CAP cap, wxString format,
                           DialPositionOption option) {
    m_extra_value_cap = cap;
    m_cap_flag.set(cap);
    m_extra_value_format = format;
    m_extra_value_option = option;
  }

private:
protected:
  int m_cx, m_cy, m_radius;
  int m_angle_start, m_angle_range;
  bool m_gps_wd;
  double m_main_value;
  DASH_CAP m_main_value_cap;
  double m_main_value_min, m_main_value_max;
  wxString m_main_value_format;
  wxString m_main_value_unit;
  DialPositionOption m_main_value_option;
  double m_extra_value;
  DASH_CAP m_extra_value_cap;
  wxString m_extra_value_format;
  wxString m_extra_value_unit;
  DialPositionOption m_extra_value_option;
  DialMarkerOption m_marker_option;
  int m_marker_offset;
  double m_marker_step, m_label_step;
  DialLabelOption m_label_option;
  wxArrayString m_label_array;

  void Draw(wxGCDC* dc) override;
  virtual void DrawFrame(wxGCDC* dc);
  virtual void DrawMarkers(wxGCDC* dc);
  virtual void DrawLabels(wxGCDC* dc);
  virtual void DrawBackground(wxGCDC* dc);
  virtual void DrawData(wxGCDC* dc, double value, wxString unit,
                        wxString format, DialPositionOption position);
  virtual void DrawForeground(wxGCDC* dc);
};

/* Shared functions */
void DrawCompassRose(wxGCDC* dc, int cx, int cy, int radius, int startangle,
                     bool showlabels, InstrumentProperties* Properties);
void DrawBoat(wxGCDC* dc, int cx, int cy, int radius);

#endif  // Dial_H_