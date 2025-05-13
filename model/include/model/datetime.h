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
#if wxCHECK_VERSION(3, 1, 6)
#include <wx/uilocale.h>
#endif

#include "ocpn_plugin.h"

extern wxString g_datetime_format;

namespace ocpn {

/**
 * Return the date/time format to use when formatting date/time strings.
 * This is a global setting that affects all date/time formatting in OpenCPN.
 *
 * @details Supported values are:
 * - "UTC": Format date/time in Coordinated Universal Time (UTC).
 * - "Local Time": Format date/time using the operating system timezone
 *   configuration.
 */
wxString getUsrDateTimeFormat();

/**
 * Format a date/time to a localized string representation, conforming to
 * the formatting options.
 *
 * @param date_time The date/time to format, must be local time.
 * @param options The date/time format options.
 * @param locale The locale to use for formatting. If not provided, the
 *              current locale is used.
 * @return wxString The formatted date/time string with appropriate timezone
 * indicator.
 *
 * @note This function should be used instead of wxDateTime.Format() to ensure
 * consistent date/time formatting across the entire application, including
 * plugins.
 */
wxString toUsrDateTimeFormat(
    const wxDateTime date_time,
    const ::DateTimeFormatOptions& options = ::DateTimeFormatOptions()
#if wxCHECK_VERSION(3, 1, 6)
        ,
    const wxUILocale& locale = wxUILocale::GetCurrent()
#endif
);
}  // namespace ocpn

#endif  // DATETIME_API_H__
