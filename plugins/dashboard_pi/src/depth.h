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
 * Dashboard depth instrument
 */

#ifndef DEPTH_H_
#define DEPTH_H_

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "instrument.h"

// Warn: div by 0 if count == 1
#define DEPTH_RECORD_COUNT 30

class DashboardInstrument_Depth : public DashboardInstrument {
public:
  DashboardInstrument_Depth(wxWindow* parent, wxWindowID id, wxString title,
                            InstrumentProperties* Properties);

  ~DashboardInstrument_Depth() override = default;

  wxSize GetSize(int orient, wxSize hint) override;
  void SetData(DASH_CAP, double, wxString) override;

private:
  int w_label, h_label, m_plotdown, m_plotup, m_plotheight;

protected:
  double m_array_depth[DEPTH_RECORD_COUNT];
  double m_max_depth;
  double m_depth;
  wxString m_depth_unit;
  wxString m_temp;

  void Draw(wxGCDC* dc) override;
  void DrawBackground(wxGCDC* dc);
  void DrawForeground(wxGCDC* dc);
};

#endif  // DEPTH_H_
