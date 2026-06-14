/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_prefs_time.h"

#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>
#include <wx/wx.h>
#include <wx/window.h>

#ifndef UNIT_TESTS
#include "ocpn_plugin.h"
#if wxCHECK_VERSION(3, 1, 6)
#include <wx/uilocale.h>
#endif
#endif

namespace {

#ifndef UNIT_TESTS
bool ChartDldrLocaleUsesTwelveHourClock() {
#if wxCHECK_VERSION(3, 1, 6)
  const wxString time_fmt = wxUILocale::GetCurrent().GetInfo(wxLOCALE_TIME_FMT);
  if (time_fmt.Contains("%I") || time_fmt.Contains("%p")) {
    return true;
  }
#endif
  const wxDateTime probe(1, wxDateTime::Jan, 2020, 13, 30, 0);
  DateTimeFormatOptions opts;
  opts.SetFormatString("$hour_minutes_seconds");
  opts.SetTimezone("Local Time");
  opts.SetShowTimezone(false);
  const wxString sample = toUsrDateTimeFormat_Plugin(probe, opts);
  return sample.Contains("AM") || sample.Contains("PM") ||
         sample.Contains("am") || sample.Contains("pm");
}

wxString ChartDldrLocaleTimeFormatMinutesOnly() {
#if wxCHECK_VERSION(3, 1, 6)
  wxString fmt = wxUILocale::GetCurrent().GetInfo(wxLOCALE_TIME_FMT);
  fmt.Replace(":%S", wxEmptyString);
  fmt.Replace("%S", wxEmptyString);
  fmt.Replace(wxString(wxUniChar(0x202F)), " ");
  return fmt;
#else
  return "%X";
#endif
}
#endif

}  // namespace

bool ChartDldrParseScheduledTimeEntry(const wxString& text, int& hour,
                                      int& minute) {
  wxString trimmed = text;
  trimmed.Trim(true).Trim(false);

  long h = 0;
  long m = 0;
  if (wxSscanf(trimmed, "%ld:%ld", &h, &m) != 2) {
    return false;
  }
  if (h < 0 || h > 23 || m < 0 || m > 59) {
    return false;
  }

  hour = static_cast<int>(h);
  minute = static_cast<int>(m);
  return ChartDldrFormatScheduledTimeEntry(hour, minute) == trimmed;
}

wxString ChartDldrFormatScheduledTimeEntry(int hour, int minute) {
  return wxString::Format("%02d:%02d", wxMin(23, wxMax(0, hour)),
                          wxMin(59, wxMax(0, minute)));
}

wxString ChartDldrFormatScheduledTimePreview(int hour, int minute) {
#ifndef UNIT_TESTS
  if (!ChartDldrLocaleUsesTwelveHourClock()) {
    return wxEmptyString;
  }
  wxDateTime dt(1, wxDateTime::Jan, 2020, hour, minute, 0);
  return dt.Format(ChartDldrLocaleTimeFormatMinutesOnly(), wxDateTime::Local);
#else
  (void)hour;
  (void)minute;
  return wxEmptyString;
#endif
}

bool ChartDldrShowsScheduledTimePreview() {
#ifndef UNIT_TESTS
  return ChartDldrLocaleUsesTwelveHourClock();
#else
  return false;
#endif
}

void ChartDldrUpdateScheduledTimePreviewWidgets(wxTextCtrl* entry,
                                                wxStaticText* preview) {
  if (!entry || !preview) {
    return;
  }

  auto relayout = [&]() {
    if (wxWindow* row = entry->GetParent()) {
      row->Layout();
    }
  };

  if (!ChartDldrShowsScheduledTimePreview()) {
    preview->SetLabel(wxEmptyString);
    preview->Hide();
    relayout();
    return;
  }

  preview->Show();

  int hour = 0;
  int minute = 0;
  if (!ChartDldrParseScheduledTimeEntry(entry->GetValue(), hour, minute)) {
    preview->SetLabel(wxEmptyString);
    relayout();
    return;
  }

  preview->SetLabel(ChartDldrFormatScheduledTimePreview(hour, minute));
  relayout();
}
