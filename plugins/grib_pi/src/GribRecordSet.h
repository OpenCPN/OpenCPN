/***************************************************************************
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
 ***************************************************************************/
/**
 * \file
 * GRIB Record Set Management.
 *
 * Defines classes for managing collections of GRIB records that share
 * the same reference time. A record set combines multiple meteorological
 * (wind, pressure, waves, etc.) valid at a single forecast time.
 */
#include "GribRecord.h"

/**
 * Indices for different meteorological parameters in GribRecordSet's record
 * array.
 */
enum {
  Idx_WIND_VX,     //!< Surface wind velocity X component in m/s
  Idx_WIND_VX850,  //!< Wind velocity X component at 850 hPa in m/s
  Idx_WIND_VX700,  //!< Wind velocity X component at 700 hPa in m/s
  Idx_WIND_VX500,  //!< Wind velocity X component at 500 hPa in m/s
  Idx_WIND_VX300,  //!< Wind velocity X component at 300 hPa in m/s
  Idx_WIND_VY,     //!< Surface wind velocity Y component in m/s
  Idx_WIND_VY850,  //!< Wind velocity Y component at 850 hPa in m/s
  Idx_WIND_VY700,  //!< Wind velocity Y component at 700 hPa in m/s
  Idx_WIND_VY500,  //!< Wind velocity Y component at 500 hPa in m/s
  Idx_WIND_VY300,  //!< Wind velocity Y component at 300 hPa in m/s
  Idx_WIND_GUST,   //!< Wind gust speed at surface in m/s
  Idx_PRESSURE,    //!< Surface pressure in Pascal (Pa)
  /**
   * Significant wave height in meters
   *
   * Represents the average height of the highest one-third of all waves.
   * Values typically range:
   * 0-0.5m: Calm to smooth seas
   * 0.5-1.25m: Slight seas
   * 1.25-2.5m: Moderate seas
   * 2.5-4m: Rough seas
   * 4-6m: Very rough seas
   * 6-9m: High seas
   * 9-14m: Very high seas
   * >14m: Phenomenal seas (hurricane conditions)
   */
  Idx_HTSIGW,
  Idx_WVDIR,          //!< Wave direction
  Idx_WVPER,          //!< Wave period
  Idx_SEACURRENT_VX,  //!< Sea current velocity X component in m/s
  Idx_SEACURRENT_VY,  //!< Sea current velocity Y component in m/s
  /** Precipitation data in millimeters per hour. */
  Idx_PRECIP_TOT,
  Idx_CLOUD_TOT,    //!< Total cloud cover in  % (percent, range 0-100%)
  Idx_AIR_TEMP,     //!< Air temperature at 2m in Kelvin (K)
  Idx_AIR_TEMP850,  //!< Air temperature at 850 hPa in Kelvin (K)
  Idx_AIR_TEMP700,  //!< Air temperature at 700 hPa in Kelvin (K)
  Idx_AIR_TEMP500,  //!< Air temperature at 500 hPa in Kelvin (K)
  Idx_AIR_TEMP300,  //!< Air temperature at 300 hPa in Kelvin (K)
  /**  Sea surface temperature in Kelvin (K) */
  Idx_SEA_TEMP,
  /**
   * Convective Available Potential Energy in J/kg (Joules per kilogram)
   *
   * CAPE measures the amount of energy available for convection in the
   * atmosphere. Higher values (>1000 J/kg) indicate potential for severe
   * thunderstorms. Values >2500 J/kg often associated with tornadic
   * thunderstorms. Useful for forecasting severe weather conditions and
   * atmospheric instability.
   */
  Idx_CAPE,
  /**
   * Composite radar reflectivity in dBZ (decibel relative to Z)
   *
   * Represents the maximum radar reflectivity at any altitude in the
   * atmosphere. Values can be negative or positive: Negative values (-30 to 0
   * dBZ): Very light precipitation, drizzle, fog, or atmospheric dust Values of
   * 0-20 dBZ: Light precipitation or cloud droplets Values of 20-30 dBZ: Light
   * to moderate rainfall Values of 30-40 dBZ: Moderate rainfall Values of 40-50
   * dBZ: Heavy rainfall, possible small hail Values >50 dBZ: Intense rainfall,
   * large hail, severe thunderstorm activity
   */
  Idx_COMP_REFL,
  /** Surface relative humidity in % (percent, range 0-100%) */
  Idx_HUMID_RE,
  /** Relative humidity at 850 hPa in % (percent, range 0-100%) */
  Idx_HUMID_RE850,
  /** Relative humidity at 700 hPa in % (percent, range 0-100%) */
  Idx_HUMID_RE700,
  /** Relative humidity at 500 hPa in % (percent, range 0-100%) */
  Idx_HUMID_RE500,
  /** Relative humidity at 300 hPa in % (percent, range 0-100%) */
  Idx_HUMID_RE300,
  /**
   * Surface geopotential height in gpm (geopotential meters)
   *
   * Geopotential height represents the height of a pressure level above mean
   * sea level. It accounts for variations in gravity with latitude and
   * altitude. Lower values indicate lower pressure systems (cyclonic activity,
   * potential storms) Higher values indicate higher pressure systems
   * (anticyclonic, typically fair weather) Typical sea level values range from
   * 5000-5900 gpm, varying with weather systems Used to identify pressure
   * systems, fronts, and atmospheric waves
   */
  Idx_GEOP_HGT,
  /** Geopotential height at 850 hPa in gpm (geopotential meters) */
  Idx_GEOP_HGT850,
  /** Geopotential height at 700 hPa in gpm (geopotential meters) */
  Idx_GEOP_HGT700,
  /** Geopotential height at 500 hPa in gpm (geopotential meters) */
  Idx_GEOP_HGT500,
  /** Geopotential height at 300 hPa in gpm (geopotential meters) */
  Idx_GEOP_HGT300,
  /** Number of supported GRIB record types */
  Idx_COUNT
};

/**
 * Manages a collection of GribRecord objects representing multiple
 * meteorological parameters at a single point in time.
 *
 * While a GribRecord represents a single parameter's grid data (e.g.,
 * temperature or wind-x component), GribRecordSet bundles related records
 * together to provide a complete meteorological picture for one timestamp. For
 * example, it might contain:
 *
 * - Surface winds (both X and Y components)
 * - Wind components at different pressure levels (850hPa, 700hPa, etc.)
 * - Temperature, pressure, humidity
 * - Wave and current data
 *
 * Records are stored in a fixed-size array where each index corresponds to a
 * specific parameter as defined by the Idx_* enumeration. The set can either:
 * - Own records (usually interpolated records it created)
 * - Reference records (owned by the GRIB file reader)
 *
 * @note This class handles memory management for records it owns via
 * m_GribRecordUnref
 */
class GribRecordSet {
public:
  /**
   * Creates an empty record set.
   *
   * @param id Unique identifier for this record set.
   *           The id is used to uniquely identify this record set within a GRIB
   * file.
   */
  GribRecordSet(unsigned int id) : m_Reference_Time(-1), m_ID(id) {
    for (int i = 0; i < Idx_COUNT; i++) {
      m_GribRecordPtrArray[i] = 0;
      m_GribRecordUnref[i] = false;
    }
  }

  virtual ~GribRecordSet() { RemoveGribRecords(); }

  /**
   * Sets a GRIB record that this set owns and will be responsible for deleting.
   *
   * Typically used for interpolated records created between two time points.
   * The set takes ownership and will delete the record when appropriate.
   *
   * @param i Index in the record array where to store the record
   * @param pGR Pointer to the GRIB record to store
   */
  void SetUnRefGribRecord(int i, GribRecord *pGR) {
    assert(i >= 0 && i < Idx_COUNT);
    if (m_GribRecordUnref[i] == true) {
      delete m_GribRecordPtrArray[i];
    }
    m_GribRecordPtrArray[i] = pGR;
    m_GribRecordUnref[i] = true;
  }

  /**
   * Removes and deletes all GRIB records owned by this set.
   *
   * Records not owned by this set are left untouched.
   */
  void RemoveGribRecords() {
    for (int i = 0; i < Idx_COUNT; i++) {
      if (m_GribRecordUnref[i] == true) {
        delete m_GribRecordPtrArray[i];
      }
    }
  }

  /** Reference time for this set of records, as the number of seconds since the
   * epoch. */
  time_t m_Reference_Time;
  /** Unique identifier for this record set. */
  unsigned int m_ID;

  /**
   * Array of pointers to GRIB records representing different meteorological
   * parameters.
   *
   * Each index corresponds to a specific parameter (wind, gust, wave, etc).
   * Records may be owned by this set (tracked by m_GribRecordUnref) or
   * referenced from elsewhere.
   */
  GribRecord *m_GribRecordPtrArray[Idx_COUNT];

private:
  // grib records files are stored and owned by reader mapGribRecords
  // interpolated grib are not, keep track of them
  bool m_GribRecordUnref[Idx_COUNT];
};
