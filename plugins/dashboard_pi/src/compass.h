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
 * Dashboard compass instrument
 *
 * Inspired by original work from Andreas Heiming
 */

#ifndef Compass_H_
#define Compass_H_

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "dial.h"

/** A compass style control */
class DashboardInstrument_Compass : public DashboardInstrument_Dial {
public:
  DashboardInstrument_Compass(wxWindow* parent, wxWindowID id, wxString title,
                              InstrumentProperties* Properties,
                              DASH_CAP cap_flag);

  ~DashboardInstrument_Compass() override = default;

  void SetData(DASH_CAP, double, wxString) override;

private:
protected:
  void DrawBackground(wxGCDC* dc) override;
  void DrawForeground(wxGCDC* dc) override;
};

#endif  // Compass_H_
