 /**************************************************************************
 *   Copyright (C) 2023 Håkan Svensson
 *   Copyright (C) 2023 Alec Leamas
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

#ifndef _METEO_POINTS_H__
#define _METEO_POINTS_H__

/** Meteo points are Meteorological and Hydrographic data received
 ** by NMEA0183 (AIS) VDM message 8 dac:001 fi: 31 or Ais8_367_33.
 ** Structure and content as described in IMO SN.1/Circ.289 or
 ** DAC367_FI33_em_version_release_3-23mar15_0**/

 struct AisMeteoData {
   // Ais8_001_31, Ais8_367_33 Meteo data
  int original_mmsi;
  int month;            // UTC 0
  int day;              // UTC 0
  int hour;             // UTC 24
  int minute;           // UTC 60
  int pos_acc;          // low = 0 GNSS
  int wind_kn;          // NAN=127
  int wind_gust_kn;     // kn NAN=127/122
  int wind_dir;         // NAN=360
  int wind_gust_dir;    // NAN=360
  double air_temp;      // C NAN = -102.4
  int rel_humid;        // % NAN = 101
  double dew_point;     // NAN = 501(50.1)
  int airpress;         // value+799 hPa NAN = 511(1310)
  int airpress_tend;    // NAN = 3
  double hor_vis;       // NAN = 127(12.7)
  double water_lev_dev; // Water level deviation (incl.tide) NAN = 30
  double water_level;   // Water level NAN = -32,768
  int water_lev_trend;  // NAN = 3
  double current;       // kn NAN = 255(25.5)
  int curr_dir;         // NAN = 360
  double wave_height;   // m NAN=255(24.5)
  int wave_period;      // s NAN = 63
  int wave_dir;         // NAN = 360
  double swell_height;  // m NAN = 255 (25.5)
  int swell_per;        // s NAN = 63
  int swell_dir;        // NAN=360
  int seastate;         // Bf NAN=13
  double water_temp;    // C NAN = 501(50.1)
  int precipitation;    // type NAN=7
  double salinity;      // ‰ NAN=510(50.0)
  int ice;              // NAN=3
  int vertical_ref;     // NAN=14
 };

 /**
 * Add a new point to the list of Meteo stations
 */
class AisMeteoPoint {
public:
  const int mmsi;
  const wxString lat;
  const wxString lon;
  const int siteID;
  const int orig_mmsi;
  AisMeteoPoint(int mmsi, const wxString& lat, const wxString& lon, int siteID,
                int orig_mmsi)
      : mmsi(mmsi), lat(lat), lon(lon), siteID(siteID), orig_mmsi(orig_mmsi) {}
};

/**
 * List of Meteo stations, a singleton.
 * Since several nations have chose not to use individual mmsi ID
 * for each station but the same for all we need to separate them
 * by its position. Every station is allocated a unique Meteo ID.
 * This list collect them and is used to destine each data update to whom it belongs.
 */
class AisMeteoPoints {
public:
  static AisMeteoPoints& GetInstance() {
     static AisMeteoPoints me;
     return me;
  }
  std::vector<AisMeteoPoint>& GetPoints() { return points; }

  void operator=(const AisMeteoPoints&) = delete;
  AisMeteoPoints(AisMeteoPoints& other) = delete;

private:
  AisMeteoPoints() = default;
  std::vector<AisMeteoPoint> points;
};


#endif
