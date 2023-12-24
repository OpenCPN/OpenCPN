/******************************************************************************
 * $Id: baro_history.h, v1.0 2014/02/10 tom-r Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   stedy
 * Based on code from  Thomas Rauch
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

#ifndef __BARO_HISTORY_H__
#define __BARO_HISTORY_H__

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
#define BARO_RECORD_COUNT 2000

#include "instrument.h"
#include "dial.h"

class DashboardInstrument_BaroHistory : public DashboardInstrument {
public:
  DashboardInstrument_BaroHistory(wxWindow* parent, wxWindowID id,
                                  wxString title, InstrumentProperties* Properties);

  ~DashboardInstrument_BaroHistory(void) {}

  void SetData(DASH_CAP, double, wxString);
  wxSize GetSize(int orient, wxSize hint);

private:
  int m_soloInPane;
  int m_SpdRecCnt, m_DirRecCnt, m_SpdStartVal, m_DirStartVal;
  int m_isNULL;
  int m_WindDirShift;

protected:
  double alpha;
  double m_ArrayBaroHistory[BARO_RECORD_COUNT];
  double m_ArrayPressHistory[BARO_RECORD_COUNT];
  double m_ExpSmoothArrayPressure[BARO_RECORD_COUNT];

  wxDateTime::Tm m_ArrayRecTime[BARO_RECORD_COUNT];

  double m_MaxPress;       //...in array
  double m_MinPress;       //...in array
  double m_TotalMaxPress;  // since O is started
  double m_TotalMinPress;
  double m_Press;
  double m_MaxPressScale;
  double m_ratioW;

  bool m_IsRunning;
  int m_SampleCount;
  int m_SetNewData;
  wxRect m_WindowRect;
  wxRect m_DrawAreaRect;  // the coordinates of the real darwing area
  int m_DrawingWidth, m_TopLineHeight, m_DrawingHeight;
  int m_width, m_height;
  int m_LeftLegend, m_RightLegend;
  int m_currSec, m_lastSec, m_SpdCntperSec;
  double m_cntSpd, m_cntDir, m_avgSpd, m_avgDir;

  void Draw(wxGCDC* dc);
  void DrawBackground(wxGCDC* dc);
  void DrawForeground(wxGCDC* dc);
  void SetMinMaxWindScale();

  void DrawWindSpeedScale(wxGCDC* dc);
  // wxString GetWindDirStr(wxString WindDir);
};

#endif  // __BARO_HISTORY_H__
