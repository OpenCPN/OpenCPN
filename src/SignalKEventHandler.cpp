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

#include <cstddef>

#include "wx/jsonval.h"
#include "wx/jsonwriter.h"
#include "wx/jsonreader.h"

#include "geodesic.h"
#include "SignalKEventHandler.h"
#include "OCPN_SignalKEvent.h"
#include "pluginmanager.h"
#include "logger.h"
#include "ocpn_frame.h"

extern PlugInManager *g_pi_manager;
//wxString g_ownshipMMSI_SK;
bool bGPSValid_SK;
extern int g_priSats;

void SignalKEventHandler::OnEvtOCPN_SignalK(OCPN_SignalKEvent &event) {
  wxJSONReader jsonReader;
  wxJSONValue root;

  LOG_DEBUG("%s\n", event.GetString().c_str());

  std::string msgTerminated = event.GetString();
  msgTerminated.append("\r\n");

  int errors = jsonReader.Parse(msgTerminated, &root);
  if (errors > 0) {
    wxLogMessage(wxString::Format(
        _T("SignalKDataStream ERROR: the JSON document is not well-formed:%d"),
        errors));
    return;
  }

  if (root.HasMember(_T("version"))) {
    wxString msg = _T("Connected to Signal K server version: ");
    msg << (root[_T("version")].AsString());
    wxLogMessage(msg);
  }

  if (root.HasMember("self")) {
    if (root["self"].AsString().StartsWith(_T("vessels.")))
      m_self = (root["self"].AsString());  // for java server, and OpenPlotter
                                           // node.js server 1.20
    else
      m_self =
          _T("vessels.") + (root["self"].AsString());  // for Node.js server
  }

  if (root.HasMember("context") && root["context"].IsString()) {
    auto context = root["context"].AsString();
    if (context != m_self) {
#if 0
            wxLogMessage(_T("** Ignore context of other ships.."));
#endif
      return;
    }
  }

  if (root.HasMember("updates") && root["updates"].IsArray()) {
    wxJSONValue &updates = root["updates"];
    for (int i = 0; i < updates.Size(); ++i) {
      handleUpdate(updates[i]);
    }
  }
}

void SignalKEventHandler::handleUpdate(wxJSONValue &update) const {
  wxString sfixtime = "";

  if (update.HasMember("timestamp")) {
    sfixtime = update["timestamp"].AsString();
  }
  if (update.HasMember("values") && update["values"].IsArray()) {
    for (int j = 0; j < update["values"].Size(); ++j) {
      wxJSONValue &item = update["values"][j];
      updateItem(item, sfixtime);
    }
  }
}

void SignalKEventHandler::updateItem(wxJSONValue &item,
                                     wxString &sfixtime) const {
  if (item.HasMember("path") && item.HasMember("value")) {
    const wxString &update_path = item["path"].AsString();
    wxJSONValue &value = item["value"];
    if (update_path == _T("navigation.position") && !value.IsNull()) {
      updateNavigationPosition(value, sfixtime);
    } else if (update_path == _T("navigation.speedOverGround") &&
               bGPSValid_SK && !value.IsNull()) {
      updateNavigationSpeedOverGround(value, sfixtime);
    } else if (update_path == _T("navigation.courseOverGroundTrue") &&
               bGPSValid_SK && !value.IsNull()) {
      updateNavigationCourseOverGround(value, sfixtime);
    } else if (update_path == _T("navigation.courseOverGroundMagnetic")) {
    }  // Ignore magnetic COG as OpenCPN don't handle yet.
    else if (update_path ==
             _T("navigation.gnss.satellites"))  // From GGA sats in use
    {
      if (g_priSats >= 2) updateGnssSatellites(value, sfixtime);
    } else if (update_path ==
               _T("navigation.gnss.satellitesInView"))  // From GSV sats in view
    {
      if (g_priSats >= 3) updateGnssSatellites(value, sfixtime);
    } else if (update_path == _T("navigation.headingTrue")) {
      if(!value.IsNull())
        updateHeadingTrue(value, sfixtime);
    } else if (update_path == _T("navigation.headingMagnetic")) {
      if(!value.IsNull())
        updateHeadingMagnetic(value, sfixtime);
    } else if (update_path == _T("navigation.magneticVariation")) {
      if(!value.IsNull())
        updateMagneticVariance(value, sfixtime);
    } else {
      // wxLogMessage(wxString::Format(_T("** Signal K unhandled update: %s"),
      // update_path));
#if 0
            wxString dbg;
            wxJSONWriter writer;
            writer.Write(item, dbg);
            wxString msg( _T("update: ") );
            msg.append(dbg);
            wxLogMessage(msg);
#endif
    }
  }
}

void SignalKEventHandler::updateNavigationPosition(
    wxJSONValue &value, const wxString &sfixtime) const
    {
#if 0
  if ((value.HasMember("latitude" && value["latitude"].IsDouble())) &&
      (value.HasMember("longitude") && value["longitude"].IsDouble())) {
    // wxLogMessage(_T(" ***** Position Update"));
    m_frame->setPosition(value["latitude"].AsDouble(),
                         value["longitude"].AsDouble());
    m_frame->PostProcessNMEA(true, false, false, sfixtime);
    bGPSValid_SK = true;
  } else {
    bGPSValid_SK = false;
  }
#endif
}

void SignalKEventHandler::updateNavigationSpeedOverGround(
    wxJSONValue &value, const wxString &sfixtime) const
    {
#if 0
  double sog_ms = value.AsDouble();
  double sog_knot = sog_ms * ms_to_knot_factor;
  // wxLogMessage(wxString::Format(_T(" ***** SOG: %f, %f"), sog_ms, sog_knot));
  m_frame->setSpeedOverGround(sog_knot);
  m_frame->PostProcessNMEA(false, true, false, sfixtime);
#endif
}

void SignalKEventHandler::updateNavigationCourseOverGround(
    wxJSONValue &value, const wxString &sfixtime) const
    {
#if 0
  double cog_rad = value.AsDouble();
  double cog_deg = GEODESIC_RAD2DEG(cog_rad);
  // wxLogMessage(wxString::Format(_T(" ***** COG: %f, %f"), cog_rad, cog_deg));
  m_frame->setCourseOverGround(cog_deg);
  m_frame->PostProcessNMEA(false, false, true, sfixtime);
#endif
}

void SignalKEventHandler::updateGnssSatellites(wxJSONValue &value,
                                               const wxString &sfixtime) const {
  if (value.IsInt()) {
    if (value.AsInt() > 0) {
      m_frame->setSatelitesInView(value.AsInt());
      g_priSats = 2;
    }
  } else if ((value.HasMember("count") && value["count"].IsInt())) {
    m_frame->setSatelitesInView(value["count"].AsInt());
    g_priSats = 3;
  }
}

void SignalKEventHandler::updateHeadingTrue(wxJSONValue &value,
                                            const wxString &sfixtime) const {
  m_frame->setHeadingTrue(GEODESIC_RAD2DEG(value.AsDouble()));
}

void SignalKEventHandler::updateHeadingMagnetic(
    wxJSONValue &value, const wxString &sfixtime) const {
  m_frame->setHeadingMagnetic(GEODESIC_RAD2DEG(value.AsDouble()));
}

void SignalKEventHandler::updateMagneticVariance(
    wxJSONValue &value, const wxString &sfixtime) const {
  m_frame->setMagneticVariation(GEODESIC_RAD2DEG(value.AsDouble()));
}
