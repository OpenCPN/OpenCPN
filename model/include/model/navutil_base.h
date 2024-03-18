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


enum { SPEED_KTS = 0, SPEED_MPH, SPEED_KMH, SPEED_MS };
enum { WSPEED_KTS = 0, WSPEED_MS, WSPEED_MPH, WSPEED_KMH };
enum { DEPTH_FT = 0, DEPTH_M, DEPTH_FA };
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

class GpxDocument {
public:
  static wxString GetUUID(void);
  static void SeedRandom();

private:
  static int GetRandomNumber(int min, int max);
};


extern wxString toSDMM(int NEflag, double a, bool hi_precision = true);
extern double toUsrSpeed(double kts_speed, int unit = -1);
extern double toUsrWindSpeed(double kts_speed, int unit = -1);
extern wxString getUsrSpeedUnit(int unit = -1);
extern wxString getUsrWindSpeedUnit(int unit = -1);
extern wxString getUsrTempUnit(int unit = -1);
extern wxString FormatDistanceAdaptive(double distance);
extern double toUsrTemp(double cel_temp, int unit = -1);

extern double toUsrDistance(double nm_distance, int unit = -1);
extern wxString getUsrDistanceUnit(int unit = -1);
extern double fromUsrDistance(double usr_distance, int unit, int default_val);
extern double fromUsrSpeed(double usr_speed, int unit, int default_val);

extern double toUsrDepth(double cel_depth, int unit = -1);
extern double fromUsrDepth(double usr_depth, int unit = -1);
extern wxString getUsrDepthUnit(int unit = -1);

const wxChar *ParseGPXDateTime(wxDateTime &dt, const wxChar *datetime);

extern wxString formatTimeDelta(wxTimeSpan span);
extern wxString formatTimeDelta(wxDateTime startTime, wxDateTime endTime);
extern wxString formatTimeDelta(wxLongLong secs);

extern double fromDMM(wxString sdms);

extern double toMagnetic(double deg_true);
extern double toMagnetic(double deg_true, double variation);
#endif   // _NAVUTIL_BASE__
