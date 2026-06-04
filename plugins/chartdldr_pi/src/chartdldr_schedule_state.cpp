/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_schedule_state.h"

#include "chartdldr_schedule_config.h"

#include <wx/datetime.h>
#include <wx/intl.h>

namespace {

constexpr int kScheduledStampChars = 16;  // "%Y-%m-%d %H:%M"

bool TrySplitCombinedStatusAtSeparator(const wxString& separator,
                                       wxString& status,
                                       wxString& attempt_iso) {
  if (status.length() <= kScheduledStampChars + separator.length()) {
    return false;
  }

  const wxString stamp = status.Left(kScheduledStampChars);
  wxDateTime parsed;
  if (!parsed.ParseFormat(stamp, _("%Y-%m-%d %H:%M"))) {
    return false;
  }

  if (!status.Mid(kScheduledStampChars).StartsWith(separator)) {
    return false;
  }

  attempt_iso = ChartDldrFormatScheduleRunIso(parsed);
  status = status.Mid(kScheduledStampChars + separator.length());
  status.Trim(false);
  return true;
}

bool TrySplitLegacyCombinedStatus(wxString& status, wxString& attempt_iso) {
  if (status.length() < 18) {
    return false;
  }

  const wxString stamp = status.BeforeFirst(' ');
  const wxString rest = status.AfterFirst(' ');
  if (stamp.length() != 10 || rest.length() < 6) {
    return false;
  }

  const wxString combined = stamp + " " + rest.BeforeFirst(' ');
  wxDateTime parsed;
  if (!parsed.ParseFormat(combined, _("%Y-%m-%d %H:%M"))) {
    return false;
  }

  attempt_iso = ChartDldrFormatScheduleRunIso(parsed);
  status = rest.AfterFirst(' ');
  status.Trim(false);
  return true;
}

}  // namespace

wxString ChartDldrScheduledNeverRunDisplayText() { return _("-- : --"); }

wxString ChartDldrScheduledDisplaySeparator() {
  // ASCII hyphen; not translated so migration can split reliably.
  return " - ";
}

wxString ChartDldrFormatScheduledRunTimestamp(const wxDateTime& run_time) {
  return run_time.Format(_("%Y-%m-%d %H:%M"));
}

wxString ChartDldrFormatScheduleRunIso(const wxDateTime& run_time) {
  return run_time.Format("%Y-%m-%d %H:%M:%S");
}

bool ChartDldrParseScheduleRunIso(const wxString& iso, wxDateTime& out) {
  if (iso.empty()) {
    return false;
  }
  if (out.ParseISOCombined(iso)) {
    return true;
  }
  if (out.ParseFormat(iso, "%Y-%m-%d %H:%M:%S")) {
    return true;
  }
  return out.ParseFormat(iso, "%Y-%m-%d %H:%M");
}

void ChartDldrMigrateLegacyScheduleStatus(ChartDldrScheduleConfig& schedule) {
  if (!schedule.last_attempt_iso.IsEmpty()) {
    return;
  }

  wxString attempt_iso;
  const wxString hyphen_sep = ChartDldrScheduledDisplaySeparator();
  const wxString em_dash_sep = wxString::FromUTF8(" \xe2\x80\x94 ");
  if (TrySplitCombinedStatusAtSeparator(hyphen_sep, schedule.last_status,
                                      attempt_iso) ||
      TrySplitCombinedStatusAtSeparator(em_dash_sep, schedule.last_status,
                                      attempt_iso) ||
      TrySplitLegacyCombinedStatus(schedule.last_status, attempt_iso)) {
    schedule.last_attempt_iso = attempt_iso;
    if (schedule.last_run_iso.IsEmpty()) {
      schedule.last_run_iso = attempt_iso;
    }
    return;
  }

  if (!schedule.last_run_iso.IsEmpty() && !schedule.last_status.IsEmpty()) {
    schedule.last_attempt_iso = schedule.last_run_iso;
  }
}

wxString ChartDldrFormatScheduledLastRunDisplay(
    const ChartDldrScheduleConfig& schedule) {
  if (schedule.last_attempt_iso.IsEmpty() && schedule.last_status.IsEmpty()) {
    return ChartDldrScheduledNeverRunDisplayText();
  }

  wxDateTime attempt_time;
  if (!schedule.last_attempt_iso.IsEmpty() &&
      ChartDldrParseScheduleRunIso(schedule.last_attempt_iso, attempt_time)) {
    const wxString stamp = ChartDldrFormatScheduledRunTimestamp(attempt_time);
    if (schedule.last_status.IsEmpty()) {
      return stamp;
    }
    return stamp + ChartDldrScheduledDisplaySeparator() + schedule.last_status;
  }

  if (!schedule.last_status.IsEmpty()) {
    return schedule.last_status;
  }

  return ChartDldrScheduledNeverRunDisplayText();
}

bool ChartDldrScheduledOutcomeAdvancesLastRun(
    ChartDldrScheduledRunOutcome outcome) {
  switch (outcome) {
    case ChartDldrScheduledRunOutcome::Skipped:
    case ChartDldrScheduledRunOutcome::BulkNoAttempts:
    case ChartDldrScheduledRunOutcome::BulkPartialSuccess:
    case ChartDldrScheduledRunOutcome::BulkSuccess:
      return true;
    case ChartDldrScheduledRunOutcome::BulkAllFailed:
      break;
  }
  return false;
}

ChartDldrScheduledRunOutcome ChartDldrScheduledOutcomeFromBulkResult(
    int downloaded_ok, int attempted, int failed) {
  if (attempted == 0) {
    return ChartDldrScheduledRunOutcome::BulkNoAttempts;
  }
  if (downloaded_ok <= 0) {
    return ChartDldrScheduledRunOutcome::BulkAllFailed;
  }
  if (failed > 0) {
    return ChartDldrScheduledRunOutcome::BulkPartialSuccess;
  }
  return ChartDldrScheduledRunOutcome::BulkSuccess;
}

wxString ChartDldrScheduledStatusFromBulkResult(int downloaded_ok,
                                                int attempted, int failed,
                                                int new_downloads,
                                                int updated_downloads) {
  if (attempted == 0) {
    return _("No charts to update");
  }
  if (downloaded_ok > 0 && failed > 0) {
    return wxString::Format(_("%d update %d new, %d failed"), updated_downloads,
                            new_downloads, failed);
  }
  if (downloaded_ok > 0) {
    return wxString::Format(_("%d update %d new"), updated_downloads,
                            new_downloads);
  }
  return wxString::Format(_("failed: %d of %d charts"), failed, attempted);
}

void ChartDldrApplyScheduledRunOutcome(ChartDldrScheduleConfig& schedule,
                                       ChartDldrScheduledRunOutcome outcome,
                                       const wxString& status_detail,
                                       const wxDateTime* run_time) {
  wxDateTime when = wxDateTime::Now();
  if (run_time && run_time->IsValid()) {
    when = *run_time;
  }

  schedule.last_status = status_detail;
  schedule.last_attempt_iso = ChartDldrFormatScheduleRunIso(when);
  if (ChartDldrScheduledOutcomeAdvancesLastRun(outcome)) {
    schedule.last_run_iso = schedule.last_attempt_iso;
  }
}
