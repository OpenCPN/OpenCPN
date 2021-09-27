/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
 * Author:   David Register
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/
// Originally by balp on 2018-07-28.

#ifndef OPENCPN_SIGNALKEVENTHANDLER_H
#define OPENCPN_SIGNALKEVENTHANDLER_H

static const double ms_to_knot_factor = 1.9438444924406;

#include <wx/jsonval.h>

class MyFrame;
class OCPN_SignalKEvent;

class SignalKEventHandler {
public:
  explicit SignalKEventHandler(MyFrame *frame) : m_frame(frame), m_self("") {}
  void OnEvtOCPN_SignalK(OCPN_SignalKEvent &event);

private:
  MyFrame *m_frame;
  wxString m_self;

  void handleUpdate(wxJSONValue &update) const;

  void updateItem(wxJSONValue &item, wxString &sfixtime) const;

  void updateNavigationPosition(wxJSONValue &value,
                                const wxString &sfixtime) const;
  void updateNavigationSpeedOverGround(wxJSONValue &value,
                                       const wxString &sfixtime) const;
  void updateNavigationCourseOverGround(wxJSONValue &value,
                                        const wxString &sfixtime) const;
  void updateGnssSatellites(wxJSONValue &value, const wxString &sfixtime) const;
  void updateHeadingTrue(wxJSONValue &value, const wxString &sfixtime) const;
  void updateHeadingMagnetic(wxJSONValue &value,
                             const wxString &sfixtime) const;
  void updateMagneticVariance(wxJSONValue &value,
                              const wxString &sfixtime) const;
};

#endif  // OPENCPN_SIGNALKEVENTHANDLER_H
