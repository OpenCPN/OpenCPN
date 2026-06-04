/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_schedule.h"

#include "chartdldr_schedule_state.h"

#include <wx/utils.h>

namespace {

wxDateTime ScheduledMomentOnDay(const wxDateTime& day, int hour, int minute) {
  wxDateTime t = day;
  t.SetHour(hour);
  t.SetMinute(minute);
  t.SetSecond(0);
  t.SetMillisecond(0);
  return t;
}

wxDateTime ParseLastRun(const wxString& last_run_iso, int hour, int minute) {
  if (last_run_iso.empty()) {
    return wxDateTime();
  }

  wxDateTime parsed;
  if (ChartDldrParseScheduleRunIso(last_run_iso, parsed)) {
    return parsed;
  }

  if (parsed.ParseFormat(last_run_iso, wxT("%Y-%m-%d"))) {
    return ScheduledMomentOnDay(parsed, hour, minute);
  }

  // Unparseable values are treated as "never run" (see schedule tests).
  return wxDateTime();
}

}  // namespace

bool ChartDldrScheduleShouldRun(const wxDateTime& now, bool enabled, int hour,
                                int minute, const wxString& last_run_iso) {
  if (!enabled) {
    return false;
  }

  const int clamped_hour = wxMin(23, wxMax(0, hour));
  const int clamped_minute = wxMin(59, wxMax(0, minute));
  const wxDateTime scheduled_today =
      ScheduledMomentOnDay(now, clamped_hour, clamped_minute);

  if (now < scheduled_today) {
    return false;
  }

  const wxDateTime last_run =
      ParseLastRun(last_run_iso, clamped_hour, clamped_minute);
  if (!last_run.IsValid()) {
    return true;
  }

  return last_run < scheduled_today;
}
