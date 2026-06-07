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

const wxString kLegacyStatusNoChartsMsgid = wxT("No charts to update");

bool LegacyStatusIsNoCharts(const wxString& status) {
  return status == kLegacyStatusNoChartsMsgid ||
         status == wxGetTranslation(kLegacyStatusNoChartsMsgid);
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

bool ChartDldrParseScheduledRunOutcome(long value,
                                     ChartDldrScheduledRunOutcome& out) {
  if (value < static_cast<long>(ChartDldrScheduledRunOutcome::Pending) ||
      value > static_cast<long>(ChartDldrScheduledRunOutcome::BulkSuccess)) {
    return false;
  }
  out = static_cast<ChartDldrScheduledRunOutcome>(value);
  return true;
}

void ChartDldrMigrateLegacyScheduleStatus(ChartDldrScheduleConfig& schedule) {
  if (!schedule.last_attempt_iso.IsEmpty()) {
    return;
  }

  wxString attempt_iso;
  if (TrySplitLegacyCombinedStatus(schedule.last_status, attempt_iso)) {
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

bool ChartDldrScheduledOutcomeAllowsSameDayRetry(
    ChartDldrScheduledRunOutcome outcome) {
  switch (outcome) {
    case ChartDldrScheduledRunOutcome::Pending:
    case ChartDldrScheduledRunOutcome::BulkAllFailed:
      return true;
    case ChartDldrScheduledRunOutcome::Skipped:
    case ChartDldrScheduledRunOutcome::BulkNoAttempts:
    case ChartDldrScheduledRunOutcome::BulkPartialSuccess:
    case ChartDldrScheduledRunOutcome::BulkSuccess:
      break;
  }
  return false;
}

void ChartDldrInferScheduleOutcomeFromLegacy(ChartDldrScheduleConfig& schedule) {
  if (schedule.last_attempt_iso.IsEmpty()) {
    schedule.last_outcome = ChartDldrScheduledRunOutcome::Pending;
    return;
  }
  if (schedule.last_status.IsEmpty()) {
    schedule.last_outcome = ChartDldrScheduledRunOutcome::Pending;
    return;
  }
  if (LegacyStatusIsNoCharts(schedule.last_status)) {
    schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkNoAttempts;
    return;
  }
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
}

ChartDldrScheduledBulkResult ChartDldrScheduledBulkResultFromStats(
    int downloaded_ok, int attempted, int failed, int new_downloads,
    int updated_downloads) {
  ChartDldrScheduledBulkResult result;
  result.outcome =
      ChartDldrScheduledOutcomeFromBulkResult(downloaded_ok, attempted, failed);
  result.status_detail = ChartDldrScheduledStatusFromBulkResult(
      downloaded_ok, attempted, failed, new_downloads, updated_downloads);
  return result;
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

void ChartDldrRecordScheduledAttemptStart(ChartDldrScheduleConfig& schedule,
                                          const wxDateTime& run_time) {
  schedule.last_attempt_iso = ChartDldrFormatScheduleRunIso(run_time);
  schedule.last_outcome = ChartDldrScheduledRunOutcome::Pending;
  schedule.SyncLegacyLastRunMirror();
}

void ChartDldrApplyScheduledRunOutcome(
    ChartDldrScheduleConfig& schedule,
    const ChartDldrScheduledBulkResult& result, const wxDateTime* run_time) {
  wxDateTime when = wxDateTime::Now();
  if (run_time && run_time->IsValid()) {
    when = *run_time;
  }

  schedule.last_status = result.status_detail;
  schedule.last_attempt_iso = ChartDldrFormatScheduleRunIso(when);
  schedule.last_outcome = result.outcome;
  schedule.SyncLegacyLastRunMirror();
}
