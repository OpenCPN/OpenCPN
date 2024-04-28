/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/log.h>
#include <wx/math.h>
#include <wx/string.h>

#include "rapidjson/document.h"

#include "model/comm_decoder.h"
#include "model/comm_util.h"
#include "model/comm_vars.h"
#include "model/geodesic.h"
#include "model/own_ship.h"


bool CommDecoder::ParsePosition(const LATLONG& Position, double& lat,
                                double& lon) {
  bool ll_valid = true;
  double llt = Position.Latitude.Latitude;

  if (!std::isnan(llt)) {
    int lat_deg_int = (int)(llt / 100);
    double lat_deg = lat_deg_int;
    double lat_min = llt - lat_deg * 100;

    lat = lat_deg + lat_min / 60.;
    if (Position.Latitude.Northing == South) lat = -lat;
  } else
    ll_valid = false;

  double lln = Position.Longitude.Longitude;
  if (!std::isnan(lln)) {
    int lon_deg_int = (int)(lln / 100);
    double lon_deg = lon_deg_int;
    double lon_min = lln - lon_deg * 100;

    lon = lon_deg + lon_min / 60.;
    if (Position.Longitude.Easting == West) lon = -lon;
  } else
    ll_valid = false;

  return ll_valid;
}

bool CommDecoder::DecodeRMC(std::string s, NavData& temp_data) {
  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse()) return false;
  if (!m_NMEA0183.Parse()) return false;

  if (m_NMEA0183.Rmc.IsDataValid == NTrue) {
    double tlat, tlon;
    if (ParsePosition(m_NMEA0183.Rmc.Position, tlat, tlon)) {
      temp_data.gLat = tlat;
      temp_data.gLon = tlon;
    } else
      return false;

    // FIXME (dave) if (!g_own_ship_sog_cog_calc )
    {
      if (!std::isnan(m_NMEA0183.Rmc.SpeedOverGroundKnots)) {
        temp_data.gSog = m_NMEA0183.Rmc.SpeedOverGroundKnots;
      }
      if (!std::isnan(temp_data.gSog) && temp_data.gSog > 0.05) {
        temp_data.gCog = m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue;
      } else {
        temp_data.gCog = NAN;
      }
    }
    // Any device sending VAR=0.0 can be assumed to not really know
    // what the actual variation is, so in this case we use WMM if
    // available
    if (!std::isnan(m_NMEA0183.Rmc.MagneticVariation) &&
        0.0 != m_NMEA0183.Rmc.MagneticVariation) {
      if (m_NMEA0183.Rmc.MagneticVariationDirection == East)
        temp_data.gVar = m_NMEA0183.Rmc.MagneticVariation;
      else if (m_NMEA0183.Rmc.MagneticVariationDirection == West)
        temp_data.gVar = -m_NMEA0183.Rmc.MagneticVariation;

      g_bVAR_Rx = true;
    }

    gRmcTime = m_NMEA0183.Rmc.UTCTime;
    gRmcDate = m_NMEA0183.Rmc.Date;
  } else
    return false;

  return true;
}

bool CommDecoder::DecodeHDM(std::string s, NavData& temp_data) {
  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse()) return false;
  if (!m_NMEA0183.Parse()) return false;

  temp_data.gHdm = m_NMEA0183.Hdm.DegreesMagnetic;

  return true;
}

bool CommDecoder::DecodeHDT(std::string s, NavData& temp_data) {
  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse()) return false;
  if (!m_NMEA0183.Parse()) return false;

  temp_data.gHdt = m_NMEA0183.Hdt.DegreesTrue;

  return true;
}

bool CommDecoder::DecodeHDG(std::string s, NavData& temp_data) {
  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse()) return false;
  if (!m_NMEA0183.Parse()) return false;

  temp_data.gHdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;

  // Any device sending VAR=0.0 can be assumed to not really know
  // what the actual variation is, so in this case we use WMM if
  // available
  if (!std::isnan(m_NMEA0183.Hdg.MagneticVariationDegrees) &&
      0.0 != m_NMEA0183.Hdg.MagneticVariationDegrees) {
    if (m_NMEA0183.Hdg.MagneticVariationDirection == East)
      temp_data.gVar = m_NMEA0183.Hdg.MagneticVariationDegrees;
    else if (m_NMEA0183.Hdg.MagneticVariationDirection == West)
      temp_data.gVar = -m_NMEA0183.Hdg.MagneticVariationDegrees;

    g_bVAR_Rx = true;
  }

  return true;
}

bool CommDecoder::DecodeVTG(std::string s, NavData& temp_data) {
  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse()) return false;
  if (!m_NMEA0183.Parse()) return false;

  // FIXME (dave)if (g_own_ship_sog_cog_calc) return false;

  if (!std::isnan(m_NMEA0183.Vtg.SpeedKnots)) temp_data.gSog = m_NMEA0183.Vtg.SpeedKnots;

  if (!std::isnan(m_NMEA0183.Vtg.SpeedKnots) &&
      !std::isnan(m_NMEA0183.Vtg.TrackDegreesTrue)) {
    temp_data.gCog = m_NMEA0183.Vtg.TrackDegreesTrue;
  }

  return true;
}

bool CommDecoder::DecodeGLL(std::string s, NavData& temp_data) {
  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse()) return false;
  if (!m_NMEA0183.Parse()) return false;

  if (m_NMEA0183.Gll.IsDataValid == NTrue) {
    double tlat, tlon;
    if (ParsePosition(m_NMEA0183.Gll.Position, tlat, tlon)) {
      temp_data.gLat = tlat;
      temp_data.gLon = tlon;
    } else
      return false;
  } else
    return false;

  return true;
}

bool CommDecoder::DecodeGSV(std::string s, NavData& temp_data) {
  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse()) return false;
  if (!m_NMEA0183.Parse()) return false;

  if (m_NMEA0183.Gsv.MessageNumber == 1)
    temp_data.n_satellites = m_NMEA0183.Gsv.SatsInView;

  return true;
}

bool CommDecoder::DecodeGGA(std::string s, NavData& temp_data) {
  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse()) return false;
  if (!m_NMEA0183.Parse()) return false;

  if (m_NMEA0183.Gga.GPSQuality > 0) {
    double tlat, tlon;
    if (ParsePosition(m_NMEA0183.Gga.Position, tlat, tlon)) {
      temp_data.gLat = tlat;
      temp_data.gLon = tlon;
    } else
      return false;

    temp_data.n_satellites = m_NMEA0183.Gga.NumberOfSatellitesInUse;

  } else
    return false;

  return true;
}


//---------------------------------------------------------------------
// NMEA2000 PGN Decode
//---------------------------------------------------------------------

bool CommDecoder::DecodePGN129026(std::vector<unsigned char> v,  NavData& temp_data) {

  unsigned char SID;
  tN2kHeadingReference ref;
  double COG, SOG;

  if (ParseN2kPGN129026(v, SID, ref, COG, SOG)) {
    temp_data.gCog = COG;
    temp_data.gSog = SOG;
    temp_data.SID = SID;
    return true;
  }

  return false;
}

bool CommDecoder::DecodePGN129029(std::vector<unsigned char> v,  NavData& temp_data) {
  unsigned char SID;
  uint16_t DaysSince1970;
  double SecondsSinceMidnight;
  double Latitude, Longitude, Altitude;
  tN2kGNSStype GNSStype;
  tN2kGNSSmethod GNSSmethod;
  unsigned char nSatellites;
  double HDOP, PDOP, GeoidalSeparation;
  unsigned char nReferenceStations;
  tN2kGNSStype ReferenceStationType;
  uint16_t ReferenceSationID;
  double AgeOfCorrection;

  if (ParseN2kPGN129029(v, SID, DaysSince1970, SecondsSinceMidnight,
                        Latitude, Longitude, Altitude,
                        GNSStype, GNSSmethod,
                        nSatellites, HDOP, PDOP, GeoidalSeparation,
                        nReferenceStations, ReferenceStationType, ReferenceSationID,
                        AgeOfCorrection
                        )) {
    temp_data.gLat = Latitude;
    temp_data.gLon = Longitude;
    temp_data.SID = SID;

    // Some devices produce "0" satelites for PGN 129029, even with a vaild fix
    //  One supposes that PGN 129540 should be used instead
    //  Here we decide that if a fix is valid, nSatellites must be > 0 to be
    //  reported in this PGN 129029
    if ( GNSSmethod == N2kGNSSm_GNSSfix ||
         GNSSmethod == N2kGNSSm_DGNSS ||
         GNSSmethod == N2kGNSSm_PreciseGNSS){
      if (nSatellites > 0)
        temp_data.n_satellites = nSatellites;
    }

    return true;
  }

  return false;
}

bool CommDecoder::DecodePGN127250(std::vector<unsigned char> v,  NavData& temp_data) {

  unsigned char SID;
  double Heading, Deviation, Variation;
  tN2kHeadingReference ref;

  if (ParseN2kPGN127250(v, SID, Heading, Deviation, Variation, ref)){
    temp_data.gHdt = N2kDoubleNA;
    temp_data.gHdm = N2kDoubleNA;
    if (ref == tN2kHeadingReference::N2khr_true)
      temp_data.gHdt = Heading;
    else if (ref == tN2kHeadingReference::N2khr_magnetic)
      temp_data.gHdm = Heading;

    temp_data.gVar = Variation;
    temp_data.SID = SID;
    return true;
  }

  return false;
}

bool CommDecoder::DecodePGN129025(std::vector<unsigned char> v,  NavData& temp_data) {

  double Latitude, Longitude;

  if (ParseN2kPGN129025(v, Latitude, Longitude)){

    temp_data.gLat = Latitude;
    temp_data.gLon = Longitude;
    return true;
  }

  return false;
}

bool CommDecoder::DecodePGN129540(std::vector<unsigned char> v,  NavData& temp_data) {

  unsigned char SID;
  uint8_t NumberOfSVs;;
  tN2kRangeResidualMode Mode;

  if (ParseN2kPGN129540(v, SID, Mode, NumberOfSVs)) {
    temp_data.n_satellites = NumberOfSVs;
    temp_data.SID = SID;
    return true;
  }

  return false;
}

bool CommDecoder::DecodeSignalK(std::string s, NavData& temp_data){
  rapidjson::Document root;

  root.Parse(s);
  if (root.HasParseError())
    return false;

  if (root.HasMember("updates") && root["updates"].IsArray()) {
    for (rapidjson::Value::ConstValueIterator itr = root["updates"].Begin(); itr != root["updates"].End(); ++itr) {
      handleUpdate(*itr, temp_data);
    }
  }

  return true;
}

void CommDecoder::handleUpdate(const rapidjson::Value &update, NavData& temp_data) {
  wxString sfixtime = "";

  if (update.HasMember("timestamp")) {
    sfixtime = update["timestamp"].GetString();
  }
  if (update.HasMember("source") && update["source"].HasMember("src")) {
    src_string = update["source"]["src"].GetString();
  }

  if (update.HasMember("values") && update["values"].IsArray()) {
    for (rapidjson::Value::ConstValueIterator itr = update["values"].Begin(); itr != update["values"].End(); ++itr) {
      updateItem(*itr, sfixtime, temp_data);
    }
  }
}

void CommDecoder::updateItem(const rapidjson::Value &item,
                             wxString &sfixtime, NavData& temp_data) {
  bool bposValid = false;
  if (item.HasMember("path") && item.HasMember("value")) {
    const wxString &update_path = item["path"].GetString();

    if (update_path == _T("navigation.gnss.methodQuality")) {
      // Record statically the GNSS status for this source in a hashmap
      if (src_string.size()) {
        if (item["value"] == "no GPS") {  // no GPS GNSS Fix
          GNSS_quality_map[src_string] = 0;
        } else {
          GNSS_quality_map[src_string] = 1;
        }
      }
    }

    if (update_path == _T("navigation.position") && !item["value"].IsNull()) {
      bposValid = updateNavigationPosition(item["value"], sfixtime, temp_data);

      // if "gnss.methodQuality" is reported as "no GPS", then invalidate gLat
      // This will flow upstream, eventually triggering the GPS watchdog
      if (GNSS_quality_map.find(src_string) != GNSS_quality_map.end()) {
        if (GNSS_quality_map[src_string] == 0) {
          temp_data.gLat = NAN;
        }
      }

    } else if (update_path == _T("navigation.speedOverGround") &&
               /*bposValid &&*/ !item["value"].IsNull()) {
      updateNavigationSpeedOverGround(item["value"], sfixtime, temp_data);

      // If the tracked "methodQuality" exists for this source,
      // and state was recorded as "no GPS", set SOG = 0
      if (GNSS_quality_map.find(src_string) != GNSS_quality_map.end()) {
        if (GNSS_quality_map[src_string] == 0) {
          temp_data.gSog = 0;
        }
      }

    } else if (update_path == _T("navigation.courseOverGroundTrue") &&
               /*bposValid &&*/ !item["value"].IsNull()) {
      updateNavigationCourseOverGround(item["value"], sfixtime, temp_data);
    } else if (update_path == _T("navigation.courseOverGroundMagnetic")) {
    }
    else if (update_path ==
             _T("navigation.gnss.satellites"))  // From GGA sats in use
    {
      updateGnssSatellites(item["value"], sfixtime, temp_data);
    } else if (update_path ==
               _T("navigation.gnss.satellitesInView"))  // From GSV sats in view
    {
      updateGnssSatellites(item["value"], sfixtime, temp_data);
    } else if (update_path == _T("navigation.headingTrue")) {
      if(!item["value"].IsNull())
        updateHeadingTrue(item["value"], sfixtime, temp_data);
    } else if (update_path == _T("navigation.headingMagnetic")) {
      if(!item["value"].IsNull())
        updateHeadingMagnetic(item["value"], sfixtime, temp_data);
    } else if (update_path == _T("navigation.magneticVariation")) {
      if(!item["value"].IsNull())
        updateMagneticVariance(item["value"], sfixtime, temp_data);
    } else {
      // wxLogMessage(wxString::Format(_T("** Signal K unhandled update: %s"),
      // update_path));
    }
  }
}

bool CommDecoder::updateNavigationPosition(
    const rapidjson::Value &value, const wxString &sfixtime, NavData& temp_data) {
  if (value.HasMember("latitude") && value["latitude"].IsDouble() &&
      (value.HasMember("longitude") && value["longitude"].IsDouble())) {
    // wxLogMessage(_T(" ***** Position Update"));
    temp_data.gLat = value["latitude"].GetDouble();
    temp_data.gLon = value["longitude"].GetDouble();
    return true;
  } else {
    return false;
  }
}


void CommDecoder::updateNavigationSpeedOverGround(
    const rapidjson::Value &value, const wxString &sfixtime, NavData& temp_data){
  double sog_ms = value.GetDouble();
  double sog_knot = sog_ms * 1.9438444924406;   // m/s to knots
  // wxLogMessage(wxString::Format(_T(" ***** SOG: %f, %f"), sog_ms, sog_knot));
  temp_data.gSog = sog_knot;
}

void CommDecoder::updateNavigationCourseOverGround(
  const rapidjson::Value &value, const wxString &sfixtime, NavData& temp_data) {
  double cog_rad = value.GetDouble();
  double cog_deg = GEODESIC_RAD2DEG(cog_rad);
  // wxLogMessage(wxString::Format(_T(" ***** COG: %f, %f"), cog_rad, cog_deg));
  temp_data.gCog = cog_deg;
}

void CommDecoder::updateGnssSatellites(const rapidjson::Value &value,
                                       const wxString &sfixtime,
                                       NavData& temp_data) {

  if (value.IsInt()) {
    if (value.GetInt() > 0) {
      temp_data.n_satellites = value.GetInt();
    }
  } else if (value.HasMember("count") && value["count"].IsInt()) {
    temp_data.n_satellites = value["count"].GetInt();
  }
  //  If "gnss.methodQuality" is "no GPS", then clear the satellite count
  if (GNSS_quality_map.find(src_string) != GNSS_quality_map.end()) {
    if (GNSS_quality_map[src_string] == 0) {
      temp_data.n_satellites = 0;
    }
  }
}

void CommDecoder::updateHeadingTrue(const rapidjson::Value &value,
                                    const wxString &sfixtime,
                                    NavData& temp_data) {
  temp_data.gHdt = GEODESIC_RAD2DEG(value.GetDouble());
}

void CommDecoder::updateHeadingMagnetic(
    const rapidjson::Value &value, const wxString &sfixtime,
    NavData& temp_data) {
  temp_data.gHdm = GEODESIC_RAD2DEG(value.GetDouble());
}

void CommDecoder::updateMagneticVariance(
    const rapidjson::Value &value, const wxString &sfixtime,
    NavData& temp_data) {
  temp_data.gVar = GEODESIC_RAD2DEG(value.GetDouble());
}
