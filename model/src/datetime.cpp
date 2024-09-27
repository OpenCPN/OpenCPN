/***************************************************************************
 *   Copyright (C) 2023 by David Register                                  *
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

#include "model/datetime.h"

#include "ocpn_plugin.h"

#if wxCHECK_VERSION(3, 1, 6)
#include <wx/uilocale.h>
#endif

namespace ocpn {

// date/time in the desired time zone format.
wxString TToString(const wxDateTime date_time, const wxString format_string,
                   const wxString desired_time_zone, const double longitude) {
  wxDateTime t(date_time);
  wxString effective_time_zone = desired_time_zone;
  if (effective_time_zone == wxEmptyString) {
    effective_time_zone = ::g_timezone;
  }
  if (effective_time_zone == wxEmptyString) {
    effective_time_zone = DT_TZ_UTC;
  }
  // Define a map for custom format specifiers.
  std::vector<std::pair<wxString, wxString>> formatMap = {
#if wxCHECK_VERSION(3, 1, 6)
      // Note: the GetInfo() method may return special unicode characters, such
      // as
      // narrow no-break space (U+202F).
      {DT_LONG_DATE, wxUILocale::GetCurrent().GetInfo(wxLOCALE_LONG_DATE_FMT)},
      {DT_WEEKDAY_SHORT_DATE,
       "%a " + wxUILocale::GetCurrent().GetInfo(wxLOCALE_SHORT_DATE_FMT)},
      {DT_SHORT_DATE,
       wxUILocale::GetCurrent().GetInfo(wxLOCALE_SHORT_DATE_FMT)},
      // $hour_minutes_seconds needs to be substituted before $hour_minutes.
      {DT_HOUR_MINUTES_SECONDS,
       wxUILocale::GetCurrent().GetInfo(wxLOCALE_TIME_FMT)},
#else
      {DT_LONG_DATE, "%x"},  // There is no descriptor for localized long date.
                             // Fallback to short date.
      {DT_WEEKDAY_SHORT_DATE, "%a %x"},
      {DT_SHORT_DATE, "%x"},
      // $hour_minutes_seconds needs to be substituted before $hour_minutes.
      {DT_HOUR_MINUTES_SECONDS, "%X"},
#endif
      {DT_HOUR_MINUTES, "%H:%M"},
  };
  wxString format = format_string;
  if (format == wxEmptyString) {
    format = DT_LONG_DATE_TIME;
  }
  // Replace custom specifiers with actual format strings
  for (const auto& pair : formatMap) {
    format.Replace(pair.first, pair.second);
  }
  if (format == wxEmptyString) {
    format = DT_LONG_DATE_TIME;
  }
  wxString ret;
  if (effective_time_zone == DT_TZ_LOCAL_TIME) {
    wxDateTime now = wxDateTime::Now();
    if ((now == (now.ToGMT())) &&
        t.IsDST())  // bug in wxWingets 3.0 for UTC meridien ?
      t.Add(wxTimeSpan(1, 0, 0, 0));
    // Get the abbreviated name of the timezone configured in the operating
    // system. Formatting with the actual timezone (rather than "LOC") makes the
    // labels unambiguous, even if the user changes the timezone settings in the
    // operating system. For example "2021-10-31 01:30:00 EDT" is unambiguous,
    // while "2021-10-31 01:30:00 LOC" is not.
    wxString tzName = t.Format("%Z");
    ret = t.Format(format, wxDateTime::Local) + " " + tzName;
  } else if (effective_time_zone == DT_TZ_UTC) {
    ret = t.Format(format, wxDateTime::UTC) + " " + _("UTC");
  } else if (effective_time_zone == DT_TZ_LMT) {
    // Local mean solar time at the current location.
    if (std::isnan(longitude)) {
      t = wxInvalidDateTime;
    } else {
      t.Add(wxTimeSpan(0, 0, wxLongLong(longitude * 3600. / 15.)));
    }
    ret = t.Format(format, wxDateTime::UTC) + " " + _("LMT");
  } else {
    // TODO: after OpenCPN migrates to C++ 20, use timezone library to format
    // the date/time in the specified time zone.
    ret = t.Format(format, wxDateTime::UTC) + " " + _("UTC");
  }
  return ret;
}

}  // namespace ocpn