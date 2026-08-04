/**************************************************************************
 *   Copyright (C) 2010 by Thomas Rauch                                    *
 *   Copyright (C) 2010 by stedy                                           *
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
 * Dashboard barometer history instrument.
 */

#ifndef BARO_HISTORY_H__
#define BARO_HISTORY_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "instrument.h"
#include "dial.h"

// Warn: div by 0 if count == 1
#define BARO_RECORD_COUNT 2000

class DashboardInstrument_BaroHistory : public DashboardInstrument {
public:
  DashboardInstrument_BaroHistory(wxWindow* parent, wxWindowID id,
                                  wxString title,
                                  InstrumentProperties* Properties);

  ~DashboardInstrument_BaroHistory(void) {}

  void SetData(DASH_CAP, double, wxString);
  wxSize GetSize(int orient, wxSize hint);

private:
  int m_solo_in_pane;
  int m_spd_rec_cnt, m_dir_rec_cnt, m_spd_start_val, m_dir_start_val;
  int m_is_null;
  int m_wind_dir_shift;

protected:
  double alpha;
  double m_array_baro_history[BARO_RECORD_COUNT];
  double m_array_press_history[BARO_RECORD_COUNT];
  double m_exp_smooth_array_pressure[BARO_RECORD_COUNT];

  wxDateTime::Tm m_array_rec_time[BARO_RECORD_COUNT];

  double m_max_press;        //...in array
  double m_min_press;        //...in array
  double m_total_max_press;  // since O is started
  double m_total_min_press;
  double m_press;
  double m_max_press_scale;
  double m_ratio_w;

  bool m_is_running;
  int m_sample_count;
  int m_set_new_data;
  wxRect m_window_rect;
  wxRect m_draw_area_rect;  // the coordinates of the real darwing area
  int m_drawing_width, m_top_line_height, m_drawing_height;
  int m_width, m_height;
  int m_left_legend, m_right_legend;
  int m_curr_sec, m_last_sec, m_spd_cntper_sec;
  double m_cnt_spd, m_cnt_dir, m_avg_spd, m_avg_dir;

  void Draw(wxGCDC* dc);
  void DrawBackground(wxGCDC* dc);
  void DrawForeground(wxGCDC* dc);
  void SetMinMaxWindScale();

  void DrawWindSpeedScale(wxGCDC* dc);
  // wxString GetWindDirStr(wxString WindDir);
};

#endif  // BARO_HISTORY_H__
