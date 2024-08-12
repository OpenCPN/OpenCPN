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

#ifndef DATETIME_API_H__
#define DATETIME_API_H__

#include <wx/wx.h>

extern wxString g_timezone;

namespace ocpn {

/**
 * Format a date/time to a localized string representation, conforming to the
 * OpenCPN timezone settings. This function should be used instead of
 * wxDateTime.Format() to ensure consistent date/time formatting.
 *
 * @param date_time The date/time to format.
 * @param format_string The format string for date/time.
 *        The default is "$weekday_short_date $hour_minutes_seconds".
 *        Substitutions can be combined, for example
 * "$short_date\n$hour_minutes". To ensure consistent date/time formatting, use
 * the predefined format strings. Custom wxDateTime.Format descriptors are
 * supported but not recommended to ensure consistency.
 * @param desired_time_zone The timezone to use when formatting the date/time.
 * Options include:
 *        - Empty string: the date/time is formatted according to the OpenCPN
 * timezone settings.
 *        - "UTC": the date/time is formatted in UTC.
 *        - "Local Time": the date/time is formatted in the local time.
 *        - "LMT": the date/time is formatted in local mean time. In this case,
 * longitude is required.
 *        - Valid timezone name: the date/time is formatted in that timezone
 * (TODO).
 * @param longitude The longitude to use when formatting the date/time in local
 * mean time (LMT).
 * @return wxString The formatted date/time string.
 */
wxString TToString(const wxDateTime date_time,
                   const wxString format_string = wxEmptyString,
                   const wxString desired_time_zone = wxEmptyString,
                   const double longitude = NAN);
}  // namespace ocpn

#endif  // DATETIME_API_H__
