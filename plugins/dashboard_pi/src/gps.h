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
 * Dashboard GPS instrument
 */

#ifndef GpS_H_
#define GpS_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "instrument.h"

// Required for struct SAT_INFO
#include "SatInfo.h"

class DashboardInstrument_GPS : public DashboardInstrument {
public:
  DashboardInstrument_GPS(wxWindow* parent, wxWindowID id, wxString title,
                          InstrumentProperties* Properties);

  ~DashboardInstrument_GPS() override = default;

  wxSize GetSize(int orient, wxSize hint) override;
  void SetData(DASH_CAP, double, wxString) override {};
  void SetSatInfo(int cnt, int seq, wxString talk, SAT_INFO sats[4]);

private:
protected:
#define GNSS_SYSTEM 6
  int m_cx, m_cy, m_radius, m_ref_dim, m_scale_delta, m_scale_base;
  int m_sat_count;
  int m_max_sat_count;
  wxString talker_id;
  SAT_INFO m_sat_info[12];
  bool b_shift;
  wxDateTime m_last_shift;
  wxDateTime m_gtime[GNSS_SYSTEM];
  int m_master;
  wxString m_talker;
  void Draw(wxGCDC* dc) override;
  void DrawFrame(wxGCDC* dc);
  void DrawBackground(wxGCDC* dc);
  void DrawForeground(wxGCDC* dc);
};

#endif  // GpS_H_