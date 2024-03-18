/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions without GUI deps
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

/**************************************************************************/
/*          Formats the coordinates to string                             */
/**************************************************************************/


#include <wx/datetime.h>
#include <wx/math.h>
#include <wx/string.h>
#include <wx/translation.h>
#include <wx/utils.h>

#include "model/navutil_base.h"
#include "model/own_ship.h"
#include "vector2D.h"

wxString toSDMM(int NEflag, double a, bool hi_precision) {
  wxString s;
  double mpy;
  short neg = 0;
  int d;
  long m;
  double ang = a;
  char c = 'N';

  if (a < 0.0) {
    a = -a;
    neg = 1;
  }
  d = (int)a;
  if (neg) d = -d;
  if (NEflag) {
    if (NEflag == 1) {
      c = 'N';

      if (neg) {
        d = -d;
        c = 'S';
      }
    } else if (NEflag == 2) {
      c = 'E';

      if (neg) {
        d = -d;
        c = 'W';
      }
    }
  }

  switch (g_iSDMMFormat) {
    case 0:
      mpy = 600.0;
      if (hi_precision) mpy = mpy * 1000;

      m = (long)wxRound((a - (double)d) * mpy);

      if (!NEflag || NEflag < 1 || NEflag > 2)  // Does it EVER happen?
      {
        if (hi_precision)
          s.Printf(_T ( "%d%c %02ld.%04ld'" ), d, 0x00B0, m / 10000, m % 10000);
        else
          s.Printf(_T ( "%d%c %02ld.%01ld'" ), d, 0x00B0, m / 10, m % 10);
      } else {
        if (hi_precision)
          if (NEflag == 1)
            s.Printf(_T ( "%02d%c %02ld.%04ld' %c" ), d, 0x00B0, m / 10000,
                     (m % 10000), c);
          else
            s.Printf(_T ( "%03d%c %02ld.%04ld' %c" ), d, 0x00B0, m / 10000,
                     (m % 10000), c);
        else if (NEflag == 1)
          s.Printf(_T ( "%02d%c %02ld.%01ld' %c" ), d, 0x00B0, m / 10, (m % 10), c);
        else
          s.Printf(_T ( "%03d%c %02ld.%01ld' %c" ), d, 0x00B0, m / 10, (m % 10), c);
      }
      break;
    case 1:
      if (hi_precision)
        s.Printf(_T ( "%03.6f" ),
                 ang);  // cca 11 cm - the GPX precision is higher, but as we
                        // use hi_precision almost everywhere it would be a
                        // little too much....
      else
        s.Printf(_T ( "%03.4f" ), ang);  // cca 11m
      break;
    case 2:
      m = (long)((a - (double)d) * 60);
      mpy = 10.0;
      if (hi_precision) mpy = mpy * 100;
      long sec = (long)((a - (double)d - (((double)m) / 60)) * 3600 * mpy);

      if (!NEflag || NEflag < 1 || NEflag > 2)  // Does it EVER happen?
      {
        if (hi_precision)
          s.Printf(_T ( "%d%c %ld'%ld.%ld\"" ), d, 0x00B0, m, sec / 1000,
                   sec % 1000);
        else
          s.Printf(_T ( "%d%c %ld'%ld.%ld\"" ), d, 0x00B0, m, sec / 10, sec % 10);
      } else {
        if (hi_precision)
          if (NEflag == 1)
            s.Printf(_T ( "%02d%c %02ld' %02ld.%03ld\" %c" ), d, 0x00B0, m,
                     sec / 1000, sec % 1000, c);
          else
            s.Printf(_T ( "%03d%c %02ld' %02ld.%03ld\" %c" ), d, 0x00B0, m,
                     sec / 1000, sec % 1000, c);
        else if (NEflag == 1)
          s.Printf(_T ( "%02d%c %02ld' %02ld.%ld\" %c" ), d, 0x00B0, m, sec / 10,
                   sec % 10, c);
        else
          s.Printf(_T ( "%03d%c %02ld' %02ld.%ld\" %c" ), d, 0x00B0, m, sec / 10,
                   sec % 10, c);
      }
      break;
  }
  return s;
}

/**************************************************************************/
/*          Converts the speed to the units selected by user              */
/**************************************************************************/
double toUsrSpeed(double kts_speed, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_iSpeedFormat;
  switch (unit) {
    case SPEED_KTS:  // kts
      ret = kts_speed;
      break;
    case SPEED_MPH:  // mph
      ret = kts_speed * 1.15078;
      break;
    case SPEED_KMH:  // km/h
      ret = kts_speed * 1.852;
      break;
    case SPEED_MS:  // m/s
      ret = kts_speed * 0.514444444;
      break;
  }
  return ret;
}

/**************************************************************************/
/*          Converts the wind speed to the units selected by user         */
/**************************************************************************/
double toUsrWindSpeed(double kts_wspeed, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_iWindSpeedFormat;
  switch (unit) {
    case WSPEED_KTS:  // kts
      ret = kts_wspeed;
      break;
    case WSPEED_MS:  // m/s
      ret = kts_wspeed * 0.514444444;
      break;
    case WSPEED_MPH:  // mph
      ret = kts_wspeed * 1.15078;
      break;
    case WSPEED_KMH:  // km/h
      ret = kts_wspeed * 1.852;
      break;
  }
  return ret;
}

/**************************************************************************/
/*          Converts the distance to the units selected by user           */
/**************************************************************************/
double toUsrDistance(double nm_distance, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_iDistanceFormat;
  switch (unit) {
    case DISTANCE_NMI:  // Nautical miles
      ret = nm_distance;
      break;
    case DISTANCE_MI:  // Statute miles
      ret = nm_distance * 1.15078;
      break;
    case DISTANCE_KM:
      ret = nm_distance * 1.852;
      break;
    case DISTANCE_M:
      ret = nm_distance * 1852;
      break;
    case DISTANCE_FT:
      ret = nm_distance * 6076.12;
      break;
    case DISTANCE_FA:
      ret = nm_distance * 1012.68591;
      break;
    case DISTANCE_IN:
      ret = nm_distance * 72913.4;
      break;
    case DISTANCE_CM:
      ret = nm_distance * 185200;
      break;
  }
  return ret;
}

/**************************************************************************/
/*    Converts the temperature to the units selected by user              */
/**************************************************************************/
double toUsrTemp(double cel_temp, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_iTempFormat;
  switch (unit) {
    case TEMPERATURE_C:  // Celsius
      ret = cel_temp;
      break;
    case TEMPERATURE_F:  // Fahrenheit
      ret = (cel_temp * 9.0 / 5.0) + 32;
      break;
    case TEMPERATURE_K:
      ret = cel_temp + 273.15;
      break;
  }
  return ret;
}

/**************************************************************************/
/*          Returns the abbreviation of user selected temperature unit */
/**************************************************************************/
wxString getUsrTempUnit(int unit) {
  wxString ret;
  if (unit == -1) unit = g_iTempFormat;
  switch (unit) {
    case TEMPERATURE_C:  // Celsius
      ret = _("C");
      break;
    case TEMPERATURE_F:  // Fahrenheit
      ret = _("F");
      break;
    case TEMPERATURE_K:  // Kelvin
      ret = _("K");
      break;
  }
  return ret;
}

/**************************************************************************/
/*          Returns the abbreviation of user selected distance unit       */
/**************************************************************************/
wxString getUsrDistanceUnit(int unit) {
  wxString ret;
  if (unit == -1) unit = g_iDistanceFormat;
  switch (unit) {
    case DISTANCE_NMI:  // Nautical miles
      ret = _("NMi");
      break;
    case DISTANCE_MI:  // Statute miles
      ret = _("mi");
      break;
    case DISTANCE_KM:
      ret = _("km");
      break;
    case DISTANCE_M:
      ret = _("m");
      break;
    case DISTANCE_FT:
      ret = _("ft");
      break;
    case DISTANCE_FA:
      ret = _("fa");
      break;
    case DISTANCE_IN:
      ret = _("in");
      break;
    case DISTANCE_CM:
      ret = _("cm");
      break;
  }
  return ret;
}


/**************************************************************************/
/*          Returns the abbreviation of user selected speed unit          */
/**************************************************************************/
wxString getUsrSpeedUnit(int unit) {
  wxString ret;
  if (unit == -1) unit = g_iSpeedFormat;
  switch (unit) {
    case SPEED_KTS:  // kts
      ret = _("kts");
      break;
    case SPEED_MPH:  // mph
      ret = _("mph");
      break;
    case SPEED_KMH:
      ret = _("km/h");
      break;
    case SPEED_MS:
      ret = _("m/s");
      break;
  }
  return ret;
}

/**************************************************************************/
/*       Returns the abbreviation of user selected wind speed unit        */
/**************************************************************************/
wxString getUsrWindSpeedUnit(int unit) {
  wxString ret;
  if (unit == -1) unit = g_iWindSpeedFormat;
  switch (unit) {
    case WSPEED_KTS:  // kts
      ret = _("kts");
      break;
    case WSPEED_MS:
      ret = _("m/s");
      break;
    case WSPEED_MPH:  // mph
      ret = _("mph");
      break;
    case WSPEED_KMH:
      ret = _("km/h");
      break;

  }
  return ret;
}

wxString FormatDistanceAdaptive(double distance) {
  wxString result;
  int unit = g_iDistanceFormat;
  double usrDistance = toUsrDistance(distance, unit);
  if (usrDistance < 0.1 &&
      (unit == DISTANCE_KM || unit == DISTANCE_MI || unit == DISTANCE_NMI)) {
    unit = (unit == DISTANCE_MI) ? DISTANCE_FT : DISTANCE_M;
    usrDistance = toUsrDistance(distance, unit);
  }
  wxString format;
  if (usrDistance < 5.0) {
    format = _T("%1.2f ");
  } else if (usrDistance < 100.0) {
    format = _T("%2.1f ");
  } else if (usrDistance < 1000.0) {
    format = _T("%3.0f ");
  } else {
    format = _T("%4.0f ");
  }
  result << wxString::Format(format, usrDistance) << getUsrDistanceUnit(unit);
  return result;
}

/**************************************************************************/
/*          Converts the speed from the units selected by user to knots   */
/**************************************************************************/
double fromUsrSpeed(double usr_speed, int unit, int default_val) {
  double ret = NAN;

  if (unit == -1) unit = default_val;
  switch (unit) {
    case SPEED_KTS:  // kts
      ret = usr_speed;
      break;
    case SPEED_MPH:  // mph
      ret = usr_speed / 1.15078;
      break;
    case SPEED_KMH:  // km/h
      ret = usr_speed / 1.852;
      break;
    case SPEED_MS:  // m/s
      ret = usr_speed / 0.514444444;
      break;
  }
  return ret;
}

/**************************************************************************/
/*          Converts the distance from the units selected by user to NMi  */
/**************************************************************************/
double fromUsrDistance(double usr_distance, int unit, int default_val) {
  double ret = NAN;
  if (unit == -1) unit = default_val;
  switch (unit) {
    case DISTANCE_NMI:  // Nautical miles
      ret = usr_distance;
      break;
    case DISTANCE_MI:  // Statute miles
      ret = usr_distance / 1.15078;
      break;
    case DISTANCE_KM:
      ret = usr_distance / 1.852;
      break;
    case DISTANCE_M:
      ret = usr_distance / 1852;
      break;
    case DISTANCE_FT:
      ret = usr_distance / 6076.12;
      break;
  }
  return ret;
}

/**************************************************************************/
/*    Converts the depth in meters to the units selected by user          */
/**************************************************************************/
double toUsrDepth(double cel_depth, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_nDepthUnitDisplay;
  switch (unit) {
    case DEPTH_FT:  // Feet
      ret = cel_depth / 0.3048;
      break;
    case DEPTH_M:  // Meters
      ret = cel_depth ;
      break;
    case DEPTH_FA:
      ret = cel_depth / 0.3048 / 6;
      break;
  }
  return ret;
}

/**************************************************************************/
/*  Converts the depth from the units selected by user to Meters   */
/**************************************************************************/
double fromUsrDepth(double usr_depth, int unit) {
  double ret = NAN;
  if (unit == -1) unit = g_nDepthUnitDisplay;
  switch (unit) {
    case DEPTH_FT:  // Feet
      ret = usr_depth * 0.3048;
      break;
    case DEPTH_M:  // Feet
      ret = usr_depth;
      break;
    case DEPTH_FA:  // Fathoms
      ret = usr_depth * 0.3048 * 6;
      break;
  }
  return ret;
}

/**************************************************************************/
/*          Returns the abbreviation of user selected depth unit */
/**************************************************************************/
wxString getUsrDepthUnit(int unit) {
  wxString ret;
  if (unit == -1) unit = g_nDepthUnitDisplay;
  switch (unit) {
    case DEPTH_FT:  // Feet
      ret = _("ft");
      break;
    case DEPTH_M:// Meters
      ret = _("m");
      break;
    case DEPTH_FA:  // Fathoms
      ret = _("fa");
      break;
  }
  return ret;
}

//---------------------------------------------------------------------------------
//          Vector Stuff for Hit Test Algorithm
//---------------------------------------------------------------------------------
double vGetLengthOfNormal(pVector2D a, pVector2D b, pVector2D n) {
  vector2D c, vNormal;
  vNormal.x = 0;
  vNormal.y = 0;
  //
  // Obtain projection vector.
  //
  // c = ((a * b)/(|b|^2))*b
  //
  c.x = b->x * (vDotProduct(a, b) / vDotProduct(b, b));
  c.y = b->y * (vDotProduct(a, b) / vDotProduct(b, b));
  //
  // Obtain perpendicular projection : e = a - c
  //
  vSubtractVectors(a, &c, &vNormal);
  //
  // Fill PROJECTION structure with appropriate values.
  //
  *n = vNormal;

  return (vVectorMagnitude(&vNormal));
}

double vDotProduct(pVector2D v0, pVector2D v1) {
  double dotprod;

  dotprod =
      (v0 == NULL || v1 == NULL) ? 0.0 : (v0->x * v1->x) + (v0->y * v1->y);

  return (dotprod);
}

pVector2D vAddVectors(pVector2D v0, pVector2D v1, pVector2D v) {
  if (v0 == NULL || v1 == NULL)
    v = (pVector2D)NULL;
  else {
    v->x = v0->x + v1->x;
    v->y = v0->y + v1->y;
  }
  return (v);
}

pVector2D vSubtractVectors(pVector2D v0, pVector2D v1, pVector2D v) {
  if (v0 == NULL || v1 == NULL)
    v = (pVector2D)NULL;
  else {
    v->x = v0->x - v1->x;
    v->y = v0->y - v1->y;
  }
  return (v);
}

double vVectorSquared(pVector2D v0) {
  double dS;

  if (v0 == NULL)
    dS = 0.0;
  else
    dS = ((v0->x * v0->x) + (v0->y * v0->y));
  return (dS);
}

double vVectorMagnitude(pVector2D v0) {
  double dMagnitude;

  if (v0 == NULL)
    dMagnitude = 0.0;
  else
    dMagnitude = sqrt(vVectorSquared(v0));
  return (dMagnitude);
}


// This function parses a string containing a GPX time representation
// and returns a wxDateTime containing the UTC corresponding to the
// input. The function return value is a pointer past the last valid
// character parsed (if successful) or NULL (if the string is invalid).
//
// Valid GPX time strings are in ISO 8601 format as follows:
//
//   [-]<YYYY>-<MM>-<DD>T<hh>:<mm>:<ss>Z|(+|-<hh>:<mm>)
//
// For example, 2010-10-30T14:34:56Z and 2010-10-30T14:34:56-04:00
// are the same time. The first is UTC and the second is EDT.

const wxChar *ParseGPXDateTime(wxDateTime &dt, const wxChar *datetime) {
  long sign, hrs_west, mins_west;
  const wxChar *end;

  // Skip any leading whitespace
  while (isspace(*datetime)) datetime++;

  // Skip (and ignore) leading hyphen
  if (*datetime == wxT('-')) datetime++;

  // Parse and validate ISO 8601 date/time string
  if ((end = dt.ParseFormat(datetime, wxT("%Y-%m-%dT%T"))) != NULL) {
    // Invalid date/time
    if (*end == 0) return NULL;

    // ParseFormat outputs in UTC if the controlling
    // wxDateTime class instance has not been initialized.

    // Date/time followed by UTC time zone flag, so we are done
    else if (*end == wxT('Z')) {
      end++;
      return end;
    }

    // Date/time followed by given number of hrs/mins west of UTC
    else if (*end == wxT('+') || *end == wxT('-')) {
      // Save direction from UTC
      if (*end == wxT('+'))
        sign = 1;
      else
        sign = -1;
      end++;

      // Parse hrs west of UTC
      if (isdigit(*end) && isdigit(*(end + 1)) && *(end + 2) == wxT(':')) {
        // Extract and validate hrs west of UTC
        wxString(end).ToLong(&hrs_west);
        if (hrs_west > 12) return NULL;
        end += 3;

        // Parse mins west of UTC
        if (isdigit(*end) && isdigit(*(end + 1))) {
          // Extract and validate mins west of UTC
          wxChar mins[3];
          mins[0] = *end;
          mins[1] = *(end + 1);
          mins[2] = 0;
          wxString(mins).ToLong(&mins_west);
          if (mins_west > 59) return NULL;

          // Apply correction
          dt -= sign * wxTimeSpan(hrs_west, mins_west, 0, 0);
          return end + 2;
        } else
          // Missing mins digits
          return NULL;
      } else
        // Missing hrs digits or colon
        return NULL;
    } else
      // Unknown field after date/time (not UTC, not hrs/mins
      //  west of UTC)
      return NULL;
  } else
    // Invalid ISO 8601 date/time
    return NULL;
}

wxString formatTimeDelta(wxTimeSpan span) {
  wxString timeStr;
  int days = span.GetDays();
  span -= wxTimeSpan::Days(days);
  int hours = span.GetHours();
  span -= wxTimeSpan::Hours(hours);
  double minutes = (double)span.GetSeconds().ToLong() / 60.0;
  span -= wxTimeSpan::Minutes(span.GetMinutes());
  int seconds = (double)span.GetSeconds().ToLong();

  timeStr =
      (days ? wxString::Format(_("%dd "), days) : _T("")) +
      (hours || days
           ? wxString::Format(_("%2dH %2dM"), hours, (int)round(minutes))
           : wxString::Format(_("%2dM %2dS"), (int)floor(minutes), seconds));

  return timeStr;
}

wxString formatTimeDelta(wxDateTime startTime, wxDateTime endTime) {
  wxString timeStr;
  if (startTime.IsValid() && endTime.IsValid()) {
    wxTimeSpan span = endTime - startTime;
    return formatTimeDelta(span);
  } else {
    return _("N/A");
  }
}

wxString formatTimeDelta(wxLongLong secs) {
  wxString timeStr;

  wxTimeSpan span(0, 0, secs);
  return formatTimeDelta(span);
}

// RFC4122 version 4 compliant random UUIDs generator.
wxString GpxDocument::GetUUID(void) {
  wxString str;
  struct {
    int time_low;
    int time_mid;
    int time_hi_and_version;
    int clock_seq_hi_and_rsv;
    int clock_seq_low;
    int node_hi;
    int node_low;
  } uuid;

  uuid.time_low = GetRandomNumber(
      0, 2147483647);  // FIXME: the max should be set to something like
                       // MAXINT32, but it doesn't compile un gcc...
  uuid.time_mid = GetRandomNumber(0, 65535);
  uuid.time_hi_and_version = GetRandomNumber(0, 65535);
  uuid.clock_seq_hi_and_rsv = GetRandomNumber(0, 255);
  uuid.clock_seq_low = GetRandomNumber(0, 255);
  uuid.node_hi = GetRandomNumber(0, 65535);
  uuid.node_low = GetRandomNumber(0, 2147483647);

  /* Set the two most significant bits (bits 6 and 7) of the
   * clock_seq_hi_and_rsv to zero and one, respectively. */
  uuid.clock_seq_hi_and_rsv = (uuid.clock_seq_hi_and_rsv & 0x3F) | 0x80;

  /* Set the four most significant bits (bits 12 through 15) of the
   * time_hi_and_version field to 4 */
  uuid.time_hi_and_version = (uuid.time_hi_and_version & 0x0fff) | 0x4000;

  str.Printf(_T("%08x-%04x-%04x-%02x%02x-%04x%08x"), uuid.time_low,
             uuid.time_mid, uuid.time_hi_and_version, uuid.clock_seq_hi_and_rsv,
             uuid.clock_seq_low, uuid.node_hi, uuid.node_low);

  return str;
}

int GpxDocument::GetRandomNumber(int range_min, int range_max) {
  long u = (long)wxRound(
      ((double)rand() / ((double)(RAND_MAX) + 1) * (range_max - range_min)) +
      range_min);
  return (int)u;
}

/****************************************************************************/
// Modified from the code posted by Andy Ross at
//     http://www.mail-archive.com/flightgear-devel@flightgear.org/msg06702.html
// Basically, it looks for a list of decimal numbers embedded in the
// string and uses the first three as degree, minutes and seconds.  The
// presence of a "S" or "W character indicates that the result is in a
// hemisphere where the final answer must be negated.  Non-number
// characters are treated as whitespace separating numbers.
//
// So there are lots of bogus strings you can feed it to get a bogus
// answer, but that's not surprising.  It does, however, correctly parse
// all the well-formed strings I can thing of to feed it.  I've tried all
// the following:
//
// 37°54.204' N
// N37 54 12
// 37°54'12"
// 37.9034
// 122°18.621' W
// 122w 18 37
// -122.31035
/****************************************************************************/
double fromDMM(wxString sdms) {
  wchar_t buf[64];
  char narrowbuf[64];
  int i, len, top = 0;
  double stk[32], sign = 1;

  // First round of string modifications to accomodate some known strange
  // formats
  wxString replhelper;
  replhelper = wxString::FromUTF8("´·");  // UKHO PDFs
  sdms.Replace(replhelper, _T("."));
  replhelper =
      wxString::FromUTF8("\"·");  // Don't know if used, but to make sure
  sdms.Replace(replhelper, _T("."));
  replhelper = wxString::FromUTF8("·");
  sdms.Replace(replhelper, _T("."));

  replhelper =
      wxString::FromUTF8("s. š.");  // Another example: cs.wikipedia.org
                                    // (someone was too active translating...)
  sdms.Replace(replhelper, _T("N"));
  replhelper = wxString::FromUTF8("j. š.");
  sdms.Replace(replhelper, _T("S"));
  sdms.Replace(_T("v. d."), _T("E"));
  sdms.Replace(_T("z. d."), _T("W"));

  // If the string contains hemisphere specified by a letter, then '-' is for
  // sure a separator...
  sdms.UpperCase();
  if (sdms.Contains(_T("N")) || sdms.Contains(_T("S")) ||
      sdms.Contains(_T("E")) || sdms.Contains(_T("W")))
    sdms.Replace(_T("-"), _T(" "));

  wcsncpy(buf, sdms.wc_str(wxConvUTF8), 63);
  buf[63] = 0;
  len = wxMin(wcslen(buf), sizeof(narrowbuf) - 1);
  ;

  for (i = 0; i < len; i++) {
    wchar_t c = buf[i];
    if ((c >= '0' && c <= '9') || c == '-' || c == '.' || c == '+') {
      narrowbuf[i] = c;
      continue; /* Digit characters are cool as is */
    }
    if (c == ',') {
      narrowbuf[i] = '.'; /* convert to decimal dot */
      continue;
    }
    if ((c | 32) == 'w' || (c | 32) == 's')
      sign = -1;      /* These mean "negate" (note case insensitivity) */
    narrowbuf[i] = 0; /* Replace everything else with nuls */
  }

  /* Build a stack of doubles */
  stk[0] = stk[1] = stk[2] = 0;
  for (i = 0; i < len; i++) {
    while (i < len && narrowbuf[i] == 0) i++;
    if (i != len) {
      stk[top++] = atof(narrowbuf + i);
      i += strlen(narrowbuf + i);
    }
  }

  return sign * (stk[0] + (stk[1] + stk[2] / 60) / 60);
}

double toMagnetic(double deg_true) {
  if (!std::isnan(gVar)) {
    if ((deg_true - gVar) > 360.)
      return (deg_true - gVar - 360.);
    else
      return ((deg_true - gVar) >= 0.) ? (deg_true - gVar) : (deg_true - gVar + 360.);
  } else {
    if ((deg_true - g_UserVar) > 360.)
      return (deg_true - g_UserVar - 360.);
    else
      return ((deg_true - g_UserVar) >= 0.) ? (deg_true - g_UserVar) : (deg_true - g_UserVar + 360.);
  }
}

double toMagnetic(double deg_true, double variation) {
  double degm = deg_true - variation;
  if (degm >= 360.)
    return degm - 360.;
  else
    return degm >= 0. ? degm : degm + 360.;
}