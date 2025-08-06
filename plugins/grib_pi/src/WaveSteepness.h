/***************************************************************************
 *   Copyright (C) 2024 by OpenCPN development team                        *
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
 * Wave Steepness Calculator for GRIB Wave Data
 *
 * Provides functionality to calculate wave steepness (H/L ratio) from
 * wave height and period data. Wave steepness is a critical parameter
 * for determining:
 * - Wave breaking probability
 * - Sea state severity
 * - Navigation safety
 * - Vessel motion characteristics
 *
 * Key formulas used:
 * - Deep water wavelength: L = g * T² / (2π)
 * - Wave steepness: S = H / L
 * - Breaking criterion: S > 1/7 (≈0.143)
 */

#ifndef __WAVESTEEPNESS_H__
#define __WAVESTEEPNESS_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GribRecord.h"

//----------------------------------------------------------------------------------------------------------
//    Wave Steepness Calculator
//----------------------------------------------------------------------------------------------------------

/**
 * WaveSteepnessCalculator
 *
 * Utility class for calculating wave steepness from height and period data.
 * All methods are static as this is a pure computational utility.
 */
class WaveSteepnessCalculator {
public:
  /**
   * Calculate wave steepness from height and period
   *
   * @param height Wave height in meters
   * @param period Wave period in seconds
   * @return Steepness ratio (H/L), or GRIB_NOTDEF if invalid input
   */
  static double CalculateSteepness(double height, double period);

  /**
   * Get human-readable steepness category
   *
   * @param steepness Wave steepness ratio
   * @return Descriptive category string (Low, Moderate, Steep, Very Steep,
   * Dangerous, Breaking)
   */
  static wxString GetSteepnessCategory(double steepness);

  /**
   * Calculate steepness from interpolated GRIB records
   *
   * @param heightRecord GRIB record containing wave height data
   * @param periodRecord GRIB record containing wave period data
   * @param lon Longitude for interpolation
   * @param lat Latitude for interpolation
   * @return Calculated steepness or GRIB_NOTDEF if data unavailable
   */
  static double CalculateSteepnessFromRecords(GribRecord* heightRecord,
                                              GribRecord* periodRecord,
                                              double lon, double lat);

private:
  // Physical constants
  static constexpr double GRAVITY =
      9.81;  ///< Gravitational acceleration (m/s²)
  static constexpr double TWO_PI = 2.0 * M_PI;  ///< 2π constant
  static constexpr double BREAKING_THRESHOLD =
      1.0 / 7.0;  ///< H/L = 1/7 ≈ 0.143

  // Steepness classification thresholds
  static constexpr double LOW_STEEPNESS = 0.02;  ///< Low steepness threshold
  static constexpr double MODERATE_STEEPNESS =
      0.05;  ///< Moderate steepness threshold
  static constexpr double HIGH_STEEPNESS = 0.10;  ///< High steepness threshold
  static constexpr double CAUTION_THRESHOLD =
      0.12;  ///< Caution threshold (85% of breaking)
};

#endif  // __WAVESTEEPNESS_H__
