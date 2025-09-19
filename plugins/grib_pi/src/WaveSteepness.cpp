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
 * \implements \ref WaveSteepness.h
 */

#include "WaveSteepness.h"
#include <cmath>

double WaveSteepnessCalculator::CalculateSteepness(double height,
                                                   double period) {
  // Validate input parameters
  if (height == GRIB_NOTDEF || period == GRIB_NOTDEF || period <= 0.0 ||
      height < 0.0) {
    return GRIB_NOTDEF;
  }

  // Handle edge case of zero height (flat sea)
  if (height == 0.0) {
    return 0.0;
  }

  // Calculate deep water wavelength using dispersion relation
  // L = g * T² / (2π)
  double wavelength = GRAVITY * period * period / TWO_PI;

  // Calculate steepness ratio H/L
  double steepness = height / wavelength;

  return steepness;
}

wxString WaveSteepnessCalculator::GetSteepnessCategory(double steepness) {
  if (steepness == GRIB_NOTDEF) {
    return _("N/A");
  }

  if (steepness >= BREAKING_THRESHOLD) {
    return _("Breaking");
  } else if (steepness >= CAUTION_THRESHOLD) {
    return _("Dangerous");
  } else if (steepness >= HIGH_STEEPNESS) {
    return _("Very Steep");
  } else if (steepness >= MODERATE_STEEPNESS) {
    return _("Steep");
  } else if (steepness >= LOW_STEEPNESS) {
    return _("Moderate");
  } else {
    return _("Low");
  }
}

double WaveSteepnessCalculator::CalculateSteepnessFromRecords(
    GribRecord* heightRecord, GribRecord* periodRecord, double lon,
    double lat) {
  // Validate input records
  if (!heightRecord || !periodRecord) {
    return GRIB_NOTDEF;
  }

  // Get interpolated values at the specified location
  double height = heightRecord->getInterpolatedValue(lon, lat, true);
  double period = periodRecord->getInterpolatedValue(lon, lat, true);

  // Calculate steepness using the interpolated values
  return CalculateSteepness(height, period);
}
