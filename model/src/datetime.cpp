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

wxString getUsrDateTimeFormat() { return ::g_datetime_format; }

// date/time in the desired time zone format.
wxString toUsrDateTimeFormat(const wxDateTime date_time,
                             const DateTimeFormatOptions& options) {
  wxDateTime t(date_time);
  wxString effective_time_zone = options.time_zone;
  if (effective_time_zone == wxEmptyString) {
    effective_time_zone = ::g_datetime_format;
  }
  if (effective_time_zone == wxEmptyString) {
    effective_time_zone = "UTC";
  }
  // Define a map for custom format specifiers.
  std::vector<std::pair<wxString, wxString>> formatMap = {
#if wxCHECK_VERSION(3, 1, 6)
      // Note: the GetInfo() method may return special unicode characters, such
      // as
      // narrow no-break space (U+202F).
      {"$long_date_time",
       wxUILocale::GetCurrent().GetInfo(wxLOCALE_LONG_DATE_FMT) + " " +
           wxUILocale::GetCurrent().GetInfo(wxLOCALE_TIME_FMT)},
      {"$long_date", wxUILocale::GetCurrent().GetInfo(wxLOCALE_LONG_DATE_FMT)},
      {"$weekday_short_date_time",
       "%a " + wxUILocale::GetCurrent().GetInfo(wxLOCALE_SHORT_DATE_FMT) + " " +
           wxUILocale::GetCurrent().GetInfo(wxLOCALE_TIME_FMT)},
      {"$weekday_short_date",
       "%a " + wxUILocale::GetCurrent().GetInfo(wxLOCALE_SHORT_DATE_FMT)},
      {"short_date_time",
       wxUILocale::GetCurrent().GetInfo(wxLOCALE_SHORT_DATE_FMT) + " " +
           wxUILocale::GetCurrent().GetInfo(wxLOCALE_TIME_FMT)},
      {"$short_date",
       wxUILocale::GetCurrent().GetInfo(wxLOCALE_SHORT_DATE_FMT)},
      {"$hour_minutes_seconds",
       wxUILocale::GetCurrent().GetInfo(wxLOCALE_TIME_FMT)},
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
  };
  wxString format = options.format_string;
  if (format == wxEmptyString) {
    format = "$weekday_short_date_time";
  }
  // Replace custom specifiers with actual format strings
  for (const auto& pair : formatMap) {
    format.Replace(pair.first, pair.second);
  }
  wxString ret;
  if (effective_time_zone == "Local Time") {
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
    ret = t.Format(format) + " " + tzName;
  } else if (effective_time_zone == "UTC") {
    // Convert to UTC and format date/time.
    ret = t.ToUTC().Format(format) + " " + _("UTC");
  } else if (effective_time_zone == "LMT") {
    // Local mean solar time at the current location.
    t = t.ToUTC();
    if (std::isnan(options.longitude)) {
      t = wxInvalidDateTime;
    } else {
      t.Add(wxTimeSpan(0, 0, wxLongLong(options.longitude * 3600. / 15.)));
    }
    ret = t.Format(format) + " " + _("LMT");
  } else {
    // Fallback to UTC if the timezone is not recognized.
    ret = t.ToUTC().Format(format) + " " + _("UTC");
  }
  return ret;
}

}  // namespace ocpn