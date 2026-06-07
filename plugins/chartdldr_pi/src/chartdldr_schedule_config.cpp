/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_schedule_config.h"

#include "chartdldr_schedule_state.h"

#include <wx/fileconf.h>
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

wxDateTime ParseScheduleStamp(const wxString& iso, int hour, int minute) {
  if (iso.empty()) {
    return wxDateTime();
  }

  wxDateTime parsed;
  if (ChartDldrParseScheduleRunIso(iso, parsed)) {
    return parsed;
  }

  if (parsed.ParseFormat(iso, wxT("%Y-%m-%d"))) {
    return ScheduledMomentOnDay(parsed, hour, minute);
  }

  return wxDateTime();
}

bool IsPersistedStampValid(const wxString& iso) {
  if (iso.empty()) {
    return true;
  }
  wxDateTime parsed;
  if (ChartDldrParseScheduleRunIso(iso, parsed)) {
    return true;
  }
  return parsed.ParseFormat(iso, wxT("%Y-%m-%d"));
}

}  // namespace

int ChartDldrScheduleConfig::MinRetryMinutes() { return 240; }

void ChartDldrScheduleConfig::SetTime(int h, int m) {
  hour = wxMin(23, wxMax(0, h));
  minute = wxMin(59, wxMax(0, m));
}

void ChartDldrScheduleConfig::SanitizeTimestamps() {
  if (!IsPersistedStampValid(last_run_iso)) {
    last_run_iso.clear();
  }
  if (!IsPersistedStampValid(last_attempt_iso)) {
    last_attempt_iso.clear();
  }
}

void ChartDldrScheduleConfig::SyncLegacyLastRunMirror() {
  if (!last_attempt_iso.IsEmpty()) {
    last_run_iso = last_attempt_iso;
  }
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
  conf->Read(_T("ScheduledUpdateLastAttempt"), &last_attempt_iso,
             wxEmptyString);
  long outcome_value = static_cast<long>(ChartDldrScheduledRunOutcome::Pending);
  const bool has_outcome =
      conf->Read(_T("ScheduledUpdateLastOutcome"), &outcome_value);
  const bool parsed_outcome = has_outcome && ChartDldrParseScheduledRunOutcome(
                                                 outcome_value, last_outcome);
  SetTime(hour, minute);
  SanitizeTimestamps();
  ChartDldrMigrateLegacyScheduleStatus(*this);
  if (!parsed_outcome) {
    ChartDldrInferScheduleOutcomeFromLegacy(*this);
  }
  SanitizeTimestamps();
}

void ChartDldrScheduleConfig::Save(wxFileConfig* conf) const {
  if (!conf) {
    return;
  }
  conf->Write(_T("ScheduledUpdateEnabled"), enabled);
  conf->Write(_T("ScheduledUpdateHour"), hour);
  conf->Write(_T("ScheduledUpdateMinute"), minute);
  conf->Write(_T("ScheduledUpdateLastRun"),
              last_attempt_iso.IsEmpty() ? last_run_iso : last_attempt_iso);
  conf->Write(_T("ScheduledUpdateLastStatus"), last_status);
  conf->Write(_T("ScheduledUpdateLastAttempt"), last_attempt_iso);
  conf->Write(_T("ScheduledUpdateLastOutcome"),
              static_cast<long>(last_outcome));
}

ChartDldrScheduleEligibility ChartDldrScheduleConfig::EligibilityAt(
    const wxDateTime& now) const {
  if (!enabled) {
    return ChartDldrScheduleEligibility::Disabled;
  }

  const wxDateTime scheduled_today = ScheduledMomentOnDay(now, hour, minute);
  if (now < scheduled_today) {
    return ChartDldrScheduleEligibility::NotYet;
  }

  const wxDateTime last_attempt =
      ParseScheduleStamp(last_attempt_iso, hour, minute);
  if (!last_attempt.IsValid() || last_attempt < scheduled_today) {
    return ChartDldrScheduleEligibility::Ready;
  }

  if (ChartDldrScheduledOutcomeAllowsSameDayRetry(last_outcome)) {
    const wxTimeSpan elapsed = now - last_attempt;
    if (elapsed.GetMinutes() < MinRetryMinutes()) {
      return ChartDldrScheduleEligibility::BlockedToday;
    }
    return ChartDldrScheduleEligibility::Ready;
  }

  return ChartDldrScheduleEligibility::BlockedToday;
}

bool ChartDldrScheduleConfig::ShouldRunNow(const wxDateTime& now) const {
  return EligibilityAt(now) == ChartDldrScheduleEligibility::Ready;
}
