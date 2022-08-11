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

#include "comm_decoder.h"
#include "comm_util.h"

bool CommDecoder::ParsePosition(const LATLONG &Position, double&lat, double& lon) {
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

bool CommDecoder::DecodeRMC(std::string s, std::shared_ptr<BasicNavDataMsg> msg) {

  wxString sentence(s.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);

  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse())
    return false;
  if (!m_NMEA0183.Parse())
    return false;

  if (m_NMEA0183.Rmc.IsDataValid == NTrue) {
    double tlat, tlon;
    if (ParsePosition(m_NMEA0183.Rmc.Position, tlat, tlon)) {
      msg->pos.lat = tlat;
      msg->pos.lon = tlon;
    }

    //FIXME (dave) if (!g_own_ship_sog_cog_calc )
    {
      if (!std::isnan(m_NMEA0183.Rmc.SpeedOverGroundKnots)){
        msg->sog = m_NMEA0183.Rmc.SpeedOverGroundKnots;
      }
      if(!std::isnan(msg->sog) && (msg->sog > 0)){
        msg->cog = m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue;
      }
      else{
        msg->cog = NAN;
      }
    }
    // Any device sending VAR=0.0 can be assumed to not really know
    // what the actual variation is, so in this case we use WMM if
    // available
    if ((!std::isnan(m_NMEA0183.Rmc.MagneticVariation)) &&
                0.0 != m_NMEA0183.Rmc.MagneticVariation) {
      if (m_NMEA0183.Rmc.MagneticVariationDirection == East)
        msg->var = m_NMEA0183.Rmc.MagneticVariation;
      else if (m_NMEA0183.Rmc.MagneticVariationDirection == West)
        msg->var = -m_NMEA0183.Rmc.MagneticVariation;

      //FIXME (dave) g_bVAR_Rx = true;
    }
  }

  return true;
}

