/**************************************************************************
 *   Copyright (C) 2010 by Andreas Merz                                    *
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
 * Dashboard altitude instrument.
 */

#ifndef ALTITUDE_H_
#define ALTITUDE_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "instrument.h"

// Warn: div by 0 if count == 1
#define ALTITUDE_RECORD_COUNT 30

class DashboardInstrument_Altitude : public DashboardInstrument {
public:
  DashboardInstrument_Altitude(wxWindow* parent, wxWindowID id, wxString title,
                               InstrumentProperties* Properties);

  ~DashboardInstrument_Altitude() override = default;

  wxSize GetSize(int orient, wxSize hint) override;
  void SetData(DASH_CAP, double, wxString) override;

private:
protected:
  const int c_grid_lines = 4;
  double m_array_altitude[ALTITUDE_RECORD_COUNT];  // FIFO
  double m_min_altitude;
  double m_max_altitude;
  double m_range = c_grid_lines;  // will change in 1 2 5 steps
  double m_altitude;              // the actual measurement value
  double m_mean_altitude = 0.0;   // moving average
  double m_sum2_altitude = 0.0;   // squared sum moving average
  int m_cnt_valid = 0;            // number of valid FIFO entries
  int m_attenuation = 1;          // 1 2 5
  int m_decade = 1;               // 1 10 100 1000 ..
  wxString m_altitude_unit;
  wxString m_temp;

  void Draw(wxGCDC* dc) override;
  void DrawBackground(wxGCDC* dc);
  void DrawForeground(wxGCDC* dc);

  // plot scaling utilities
  void SetAttenuation(int steps);
  int GetAttenuation() const;
};

#endif  // ALTITUDE_H_
