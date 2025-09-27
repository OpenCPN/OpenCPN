/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions without GUI dependencies.
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

#ifndef _NAVUTIL_BASE__
#define _NAVUTIL_BASE__

#include <wx/datetime.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "model/config_vars.h"
#include "model/gpx_document.h"

enum { SPEED_KTS = 0, SPEED_MPH, SPEED_KMH, SPEED_MS };
enum { WSPEED_KTS = 0, WSPEED_MS, WSPEED_MPH, WSPEED_KMH };
enum { DEPTH_FT = 0, DEPTH_M, DEPTH_FA };
/** Height unit format constants for vertical measurements above reference datum
 */
enum { HEIGHT_M = 0, HEIGHT_FT };
enum { TEMPERATURE_C = 0, TEMPERATURE_F = 1, TEMPERATURE_K = 2 };

enum {
  DISTANCE_NMI = 0,
  DISTANCE_MI,
  DISTANCE_KM,
  DISTANCE_M,
  DISTANCE_FT,
  DISTANCE_FA,
  DISTANCE_IN,
  DISTANCE_CM
};

extern wxString toSDMM(int NEflag, double a, bool hi_precision = true);
extern double toUsrSpeed(double kts_speed, int unit = -1);
extern double toUsrWindSpeed(double kts_speed, int unit = -1);
extern wxString getUsrSpeedUnit(int unit = -1);
extern wxString getUsrWindSpeedUnit(int unit = -1);
extern wxString getUsrTempUnit(int unit = -1);
/**
 * Format a distance (given in nautical miles) using the current distance
 * preference, adapting unit and precision for readability when appropriate.
 *
 * - Interprets `distance` as nautical miles (NMi).
 * - Applies the user's distance preference when converting and formatting.
 * - May adjust the displayed unit and numeric precision based on magnitude to
 *   improve readability.
 *
 * @param distance Distance in nautical miles to format.
 * @return Localized string containing the numeric value and unit.
 */
extern wxString FormatDistanceAdaptive(double distance);
/**
 * Convert a temperature from Celsius to user display units.
 *
 * @param cel_temp Temperature in Celsius to convert.
 * @param unit Output unit or -1 to use the global temperature format.
 * @return Converted temperature in the selected unit, or NaN on error.
 */
extern double toUsrTemp(double cel_temp, int unit = -1);

/**
 * Convert a distance from nautical miles (NMi) to user display units.
 *
 * @param nm_distance Distance in nautical miles to convert.
 * @param unit Output unit or -1 to use the global distance format.
 * @return Converted distance in the selected unit, or NaN on error.
 */
extern double toUsrDistance(double nm_distance, int unit = -1);
extern wxString getUsrDistanceUnit(int unit = -1);
/**
 * Convert distance from user units to nautical miles.
 *
 * @param usr_distance Distance in user units
 * @param unit Unit to convert from, or -1 to use default_val
 * @param default_val Default unit when unit=-1, or -1 to use the global default
 */
extern double fromUsrDistance(double usr_distance, int unit,
                              int default_val = -1);
extern double fromUsrSpeed(double usr_speed, int unit, int default_val);

/**
 * Convert a depth from meters to user display units.
 *
 * @param m_depth Depth in meters to convert.
 * @param unit Output unit or -1 to use the global depth format.
 * @return Converted depth in the selected unit, or NaN on error.
 */
extern double toUsrDepth(double m_depth, int unit = -1);
extern double fromUsrDepth(double usr_depth, int unit = -1);
extern wxString getUsrDepthUnit(int unit = -1);

/** Convert height from meters to preferred height units */
extern double toUsrHeight(double m_height, int unit = -1);
/** Convert height from preferred height units to meters */
extern double fromUsrHeight(double usr_height, int unit = -1);
/** Get the abbreviation for the preferred height unit */
extern wxString getUsrHeightUnit(int unit = -1);

/**
 * This function parses a string containing a GPX time representation
 * and returns a wxDateTime containing the UTC corresponding to the
 * input. The function return value is a pointer past the last valid
 * character parsed (if successful) or NULL (if the string is invalid).
 *
 * Valid GPX time strings are in ISO 8601 format as follows:
 *
 *   [-]<YYYY>-<MM>-<DD>T<hh>:<mm>:<ss>Z|(+|-<hh>:<mm>)
 *
 * For example, 2010-10-30T14:34:56Z and 2010-10-30T14:34:56-04:00
 * are the same time. The first is UTC and the second is EDT.
 */
const wxChar *ParseGPXDateTime(wxDateTime &dt, const wxChar *datetime);

extern wxString formatTimeDelta(wxTimeSpan span);
extern wxString formatTimeDelta(wxDateTime startTime, wxDateTime endTime);
extern wxString formatTimeDelta(wxLongLong secs);

extern double fromDMM(wxString sdms);

extern double toMagnetic(double deg_true);
extern double toMagnetic(double deg_true, double variation);

wxString SanitizeFileName(const wxString &input);
#endif  // _NAVUTIL_BASE__
