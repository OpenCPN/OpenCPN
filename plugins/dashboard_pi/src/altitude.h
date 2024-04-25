/******************************************************************************
 * $Id: altitude.h, v0.1 $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin, display altitude trace
 * Author:   derived from Jean-Eudes Onfray's depth.h by Andreas Merz
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

#ifndef __ALTITUDE_H__
#define __ALTITUDE_H__

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

// Warn: div by 0 if count == 1
#define ALTITUDE_RECORD_COUNT 30

#include "instrument.h"

class DashboardInstrument_Altitude : public DashboardInstrument {
public:
  DashboardInstrument_Altitude(wxWindow* parent, wxWindowID id, wxString title, InstrumentProperties* Properties);

  ~DashboardInstrument_Altitude(void) {}

  wxSize GetSize(int orient, wxSize hint);
  void SetData(DASH_CAP, double, wxString);

private:

protected:
  const int    c_GridLines = 4;
  double m_ArrayAltitude[ALTITUDE_RECORD_COUNT];   // FIFO
  double m_MinAltitude;
  double m_MaxAltitude;
  double m_Range = c_GridLines;    // will change in 1 2 5 steps
  double m_Altitude;               // the actual measurement value
  double m_meanAltitude = 0.0;     // moving average
  double m_sum2Altitude = 0.0;     // squared sum moving average
  int    m_Attenuation = 1;        // 1 2 5
  int    m_Decade = 1;             // 1 10 100 1000 ..
  wxString m_AltitudeUnit;
  wxString m_Temp;

  void Draw(wxGCDC* dc);
  void DrawBackground(wxGCDC* dc);
  void DrawForeground(wxGCDC* dc);
  
  // plot scaling utilities
  void setAttenuation(int steps);
  int  getAttenuation();

};

#endif  // __ALTITUDE_H__
