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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "wx/math.h"

#include "comm_decoder.h"
#include "comm_util.h"

extern int gps_watchdog_timeout_ticks;
extern wxString gRmcDate, gRmcTime;
extern bool g_bHDT_Rx, g_bVAR_Rx;

bool CommDecoder::ParsePosition(const LATLONG& Position, double& lat,
                                double& lon) {
  bool ll_valid = true;
  double llt = Position.Latitude.Latitude;

  if (!std::isnan(llt)) {
    int lat_deg_int = (int)(llt / 100);
    double lat_deg = lat_deg_int;
    double lat_min = llt - (lat_deg * 100);

    lat = lat_deg + (lat_min / 60.);
    if (Position.Latitude.Northing == South) lat = -lat;
  } else
    ll_valid = false;

  double lln = Position.Longitude.Longitude;
  if (!std::isnan(lln)) {
    int lon_deg_int = (int)(lln / 100);
    double lon_deg = lon_deg_int;
    double lon_min = lln - (lon_deg * 100);

    lon = lon_deg + (lon_min / 60.);
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
      if (!std::isnan(temp_data.gSog) && (temp_data.gSog > 0)) {
        temp_data.gCog = m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue;
      } else {
        temp_data.gCog = NAN;
      }
    }
    // Any device sending VAR=0.0 can be assumed to not really know
    // what the actual variation is, so in this case we use WMM if
    // available
    if ((!std::isnan(m_NMEA0183.Rmc.MagneticVariation)) &&
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
  if (!std::isnan(m_NMEA0183.Hdt.DegreesTrue)) {
    g_bHDT_Rx = true;
  }

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
  if ((!std::isnan(m_NMEA0183.Hdg.MagneticVariationDegrees)) &&
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

  // Fixme (dave)
  //   if (g_priSats >= 4) {
  //     if (m_NMEA0183.Gsv.MessageNumber == 1) {
  //       // Some GNSS print SatsInView in message #1 only
  //       setSatelitesInView(m_NMEA0183.Gsv.SatsInView);
  //       g_priSats = 4;
  //     }
  //   }

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
    temp_data.gCog = COG *180. / M_PI;
    temp_data.gSog = SOG;
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
    temp_data.n_satellites = nSatellites;

    return true;
  }

  return false;
}

bool CommDecoder::DecodePGN127250(std::vector<unsigned char> v,  NavData& temp_data) {

  unsigned char SID;
  double Heading, Deviation, Variation;
  tN2kHeadingReference ref;

  if (ParseN2kPGN127250(v, SID, Heading, Deviation, Variation, ref)){
    temp_data.gHdt = Heading * 180. / M_PI;
    temp_data.gVar = Variation * 180. / M_PI;
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
