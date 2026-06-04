/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_schedule_config.h"

#include "chartdldr_schedule.h"

#include <wx/fileconf.h>
#include <wx/utils.h>

void ChartDldrScheduleConfig::SetTime(int h, int m) {
  hour = wxMin(23, wxMax(0, h));
  minute = wxMin(59, wxMax(0, m));
}

void ChartDldrScheduleConfig::Load(wxFileConfig* conf) {
  if (!conf) {
    return;
  }
  conf->Read(_T("ScheduledUpdateEnabled"), &enabled, false);
  conf->Read(_T("ScheduledUpdateHour"), &hour, 3);
  conf->Read(_T("ScheduledUpdateMinute"), &minute, 0);
  conf->Read(_T("ScheduledUpdateLastRun"), &last_run_iso, wxEmptyString);
  if (last_run_iso.empty()) {
    wxString legacy_date;
    conf->Read(_T("ScheduledUpdateLastRunDate"), &legacy_date, wxEmptyString);
    last_run_iso = legacy_date;
  }
  conf->Read(_T("ScheduledUpdateLastStatus"), &last_status, wxEmptyString);
  SetTime(hour, minute);
}

void ChartDldrScheduleConfig::Save(wxFileConfig* conf) const {
  if (!conf) {
    return;
  }
  conf->Write(_T("ScheduledUpdateEnabled"), enabled);
  conf->Write(_T("ScheduledUpdateHour"), hour);
  conf->Write(_T("ScheduledUpdateMinute"), minute);
  conf->Write(_T("ScheduledUpdateLastRun"), last_run_iso);
  conf->Write(_T("ScheduledUpdateLastStatus"), last_status);
}

bool ChartDldrScheduleConfig::ShouldRunNow(const wxDateTime& now) const {
  return ChartDldrScheduleShouldRun(now, enabled, hour, minute, last_run_iso);
}
