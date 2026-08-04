/**************************************************************************
 *   Copyright (C) 2010 by Pavel Kalian                                    *
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
 * Dashboard clock instrument
 */

#ifndef CLOCK_H_
#define CLOCK_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "instrument.h"

extern int g_iUTCOffset;  // get offset from dashboard_pi.cpp

/**
 * A dashboard instrument that displays the GNSS clock time, if available.
 *
 * A dashboard instrument that shows the current time from GNSS source.
 * Can display in either UTC or local time based on user preference.
 * Time format is configurable through the format string parameter.
 */
class DashboardInstrument_Clock : public DashboardInstrument_Single {
public:
  DashboardInstrument_Clock(wxWindow *parent, wxWindowID id, wxString title,
                            InstrumentProperties *Properties,
                            DASH_CAP cap_flag = OCPN_DBP_STC_CLK,
                            wxString format = "%02i:%02i:%02i UTC");

  ~DashboardInstrument_Clock() override = default;

  void SetData(DASH_CAP, double, wxString) override;
  virtual void SetUtcTime(wxDateTime value);
  wxString GetDisplayTime(wxDateTime UTCtime);
  [[nodiscard]] bool GetUtc() const { return bUTC; }
  void SetUtc(bool flag) { bUTC = flag; }
  InstrumentProperties *m_Properties;

private:
  bool bUTC;
};

/**
 * A dashboard instrument that displays current moon phase information.
 *
 * Calculates and displays:
 * - Visual moon phase representation
 * - Current lunar phase
 * - Hemisphere information
 */
class DashboardInstrument_Moon : public DashboardInstrument_Clock {
public:
  DashboardInstrument_Moon(wxWindow *parent, wxWindowID id, wxString title,
                           InstrumentProperties *Properties);
  ~DashboardInstrument_Moon() override = default;

  wxSize GetSize(int orient, wxSize hint) override;
  void SetData(DASH_CAP, double, wxString) override;
  void SetUtcTime(wxDateTime value) override;

protected:
  void Draw(wxGCDC *dc) override;

private:
  int moon_phase(int y, int m, int d);
  int m_phase;
  int m_radius;
  wxString m_hemisphere;
};

/**
 * A dashboard instrument that displays sunrise and sunset times.
 */
class DashboardInstrument_Sun : public DashboardInstrument_Clock {
public:
  DashboardInstrument_Sun(wxWindow *parent, wxWindowID id, wxString title,
                          InstrumentProperties *Properties,
                          wxString format = "%02i:%02i:%02i UTC");

  ~DashboardInstrument_Sun() override = default;

  wxSize GetSize(int orient, wxSize hint) override;
  void SetData(DASH_CAP st, double data, wxString unit) override;
  void SetUtcTime(wxDateTime value) override;

protected:
  void Draw(wxGCDC *dc) override;

private:
  wxString m_sunrise;
  wxString m_sunset;
  double m_lat;
  double m_lon;
  wxDateTime m_dt;

  void calculateSun(double latit, double longit, wxDateTime &sunrise,
                    wxDateTime &sunset);
};

/**
 * A dashboard instrument that displays the current computer time.
 */
class DashboardInstrument_CPUClock : public DashboardInstrument_Clock {
public:
  DashboardInstrument_CPUClock(wxWindow *parent, wxWindowID id, wxString title,
                               InstrumentProperties *Properties,
                               wxString format = "%02i:%02i:%02i UTC");

  ~DashboardInstrument_CPUClock() override = default;

  void SetData(DASH_CAP, double, wxString) override;

  void SetUtcTime(wxDateTime value) override;
};
#endif  // CLOCK_H_
