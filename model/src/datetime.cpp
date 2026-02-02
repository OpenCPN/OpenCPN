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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement datetime.h -- date and time utilities.
 */

#include <iomanip>
#include <sstream>

#include <wx/tokenzr.h>
#if wxCHECK_VERSION(3, 1, 6)
#include <wx/uilocale.h>
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#include "model/datetime.h"
#include "ocpn_plugin.h"

namespace ocpn {

wxString getUsrDateTimeFormat() { return ::g_datetime_format; }

// date/time in the desired time zone format.
wxString toUsrDateTimeFormat(const wxDateTime date_time,
                             const DateTimeFormatOptions& options
#if wxCHECK_VERSION(3, 1, 6)
                             ,
                             const wxUILocale& locale
#endif
) {
  wxDateTime t(date_time);
  wxString effective_time_zone = options.time_zone;
  if (effective_time_zone == "") {
    effective_time_zone = ::g_datetime_format;
  }
  if (effective_time_zone == "") {
    effective_time_zone = "UTC";
  }
#ifdef __ANDROID__
  wxString aform = androidGetLocalizedDateTime(options, date_time);
  if (!aform.IsEmpty()) return aform;
#endif

  // Define a map for custom format specifiers.
  std::vector<std::pair<wxString, wxString>> formatMap = {
#if wxCHECK_VERSION(3, 1, 6)
      // Note: the GetInfo() method may return special unicode characters, such
      // as narrow no-break space (U+202F).
      {"$long_date_time", locale.GetInfo(wxLOCALE_LONG_DATE_FMT) + " " +
                              locale.GetInfo(wxLOCALE_TIME_FMT)},
      {"$long_date", locale.GetInfo(wxLOCALE_LONG_DATE_FMT)},
      {"$weekday_short_date_time", "%a " +
                                       locale.GetInfo(wxLOCALE_SHORT_DATE_FMT) +
                                       " " + locale.GetInfo(wxLOCALE_TIME_FMT)},
      {"$weekday_short_date", "%a " + locale.GetInfo(wxLOCALE_SHORT_DATE_FMT)},
      {"$short_date_time", locale.GetInfo(wxLOCALE_SHORT_DATE_FMT) + " " +
                               locale.GetInfo(wxLOCALE_TIME_FMT)},
      {"$short_date", locale.GetInfo(wxLOCALE_SHORT_DATE_FMT)},
      {"$hour_minutes_seconds", locale.GetInfo(wxLOCALE_TIME_FMT)},
#else
      {"$long_date_time", "%x %X"},
      {"$long_date", "%x"},  // There is no descriptor for localized long date.
                             // Fallback to short date.
      {"$weekday_short_date_time", "%a %x %X"},
      {"$weekday_short_date", "%a %x"},
      {"$short_date_time", "%x %X"},
      {"$short_date", "%x"},
      {"$hour_minutes_seconds", "%X"},
#endif
      {"$hour_minutes", "%H:%M"},
      {"$24_hour_minutes_seconds", "%H:%M:%S"},
  };
  wxString format = options.format_string;
  if (format == "") {
    format = "$weekday_short_date_time";
  }
  // Iterate through the formatMap and replace each key with its value in
  // the format string.
  for (const auto& pair : formatMap) {
    if (format.Contains(pair.first)) {
      format.Replace(pair.first, pair.second);
    }
  }
  // wxDateTime::Format() does not work when the format string contains 0x202F
  // (narrow no-break space). Replace it with a regular space.
  format.Replace(wxString(wxUniChar(0x202F)), " ");
  wxString ret;
  wxString tzName;
  if (effective_time_zone == "Local Time") {
    wxDateTime now = wxDateTime::Now();
    if (now == now.ToGMT() && t.IsDST()) {
      // bug in wxWingets 3.0 for UTC meridien ?
      t.Add(wxTimeSpan(1, 0, 0, 0));
    }
    if (options.show_timezone) {
#ifdef __WXMSW__
      tzName = _("LOC");
#else
      // Get the name of the timezone configured in the operating system.
      // Formatting with the actual timezone (rather than "LOC") makes the
      // labels unambiguous, even if the user changes the timezone settings in
      // the operating system. For example "2021-10-31 01:30:00 EDT" is
      // unambiguous, while "2021-10-31 01:30:00 LOC" is not.
      tzName = t.Format("%Z");
#endif
    }
  } else if (effective_time_zone == "LMT") {
    // Local mean solar time at the current location.
    t.MakeUTC();
    tzName = _("LMT");
    if (std::isnan(options.longitude)) {
      t = wxInvalidDateTime;
    } else {
      t.Add(wxTimeSpan(0, 0, wxLongLong(options.longitude * 3600. / 15.)));
    }
  } else {
    // UTC, or fallback to UTC if the timezone is not recognized.
    tzName = _("UTC");
  }
  wxDateTime::TimeZone zone =
      tzName == _("UTC") ? wxDateTime::GMT0 : wxDateTime::Local;
  wxString formattedDate = t.Format(format, zone);
  if (options.show_timezone) {
    return formattedDate + " " + tzName;
  } else {
    return formattedDate;
  }
}

}  // namespace ocpn
