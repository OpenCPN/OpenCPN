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
 * Dashboard wind instrument implementation.
 */

#ifndef Wind_H_
#define Wind_H_

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "dial.h"

/** A wind style control. */
class DashboardInstrument_Wind : public DashboardInstrument_Dial {
public:
  DashboardInstrument_Wind(wxWindow* parent, wxWindowID id, wxString title,
                           InstrumentProperties* Properties, DASH_CAP cap_flag);

  ~DashboardInstrument_Wind() override = default;

private:
protected:
  void DrawBackground(wxGCDC* dc) override;
};

class DashboardInstrument_WindCompass : public DashboardInstrument_Dial {
public:
  DashboardInstrument_WindCompass(wxWindow* parent, wxWindowID id,
                                  wxString title,
                                  InstrumentProperties* Properties,
                                  DASH_CAP cap_flag);

  ~DashboardInstrument_WindCompass() override = default;

protected:
  void DrawBackground(wxGCDC* dc) override;
};

class DashboardInstrument_TrueWindAngle : public DashboardInstrument_Dial {
public:
  DashboardInstrument_TrueWindAngle(wxWindow* parent, wxWindowID id,
                                    wxString title,
                                    InstrumentProperties* Properties,
                                    DASH_CAP cap_flag);

  ~DashboardInstrument_TrueWindAngle() override = default;

protected:
  void DrawBackground(wxGCDC* dc) override;
};
/*****************************************************************************
Apparent & True wind angle combined in one dial instrument
Author: Thomas Rauch
******************************************************************************/
class DashboardInstrument_AppTrueWindAngle : public DashboardInstrument_Dial {
public:
  DashboardInstrument_AppTrueWindAngle(wxWindow* parent, wxWindowID id,
                                       wxString title,
                                       InstrumentProperties* Properties,
                                       DASH_CAP cap_flag);

  ~DashboardInstrument_AppTrueWindAngle() override = default;
  void SetData(DASH_CAP, double, wxString) override;

private:
protected:
  double m_MainValueApp, m_MainValueTrue;
  double m_ExtraValueApp, m_ExtraValueTrue;

  wxString m_ExtraValueAppUnit, m_ExtraValueTrueUnit, m_MainValueAppUnit,
      m_MainValueTrueUnit;
  DialPositionOption m_MainValueOption1, m_MainValueOption2,
      m_ExtraValueOption1, m_ExtraValueOption2;
  void DrawBackground(wxGCDC* dc) override;
  void Draw(wxGCDC* dc) override;
  void DrawForeground(wxGCDC* dc) override;
  void DrawData(wxGCDC* dc, double value, wxString unit, wxString format,
                DialPositionOption position) override;
};

#endif  // Wind_H_
