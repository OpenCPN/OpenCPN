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
 * Dashboard rudder angle instrument
 */

#ifndef __RudderAngle_H__
#define __RudderAngle_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "dial.h"

class DashboardInstrument_RudderAngle : public DashboardInstrument_Dial {
public:
  DashboardInstrument_RudderAngle(wxWindow* parent, wxWindowID id,
                                  wxString title,
                                  InstrumentProperties* Properties);
  ~DashboardInstrument_RudderAngle(void) {}

  wxSize GetSize(int orient, wxSize hint);
  void SetData(DASH_CAP, double, wxString);

private:
protected:
  void DrawFrame(wxGCDC* dc);
  void DrawBackground(wxGCDC* dc);
};

#endif  // __RudderAngle_H__
