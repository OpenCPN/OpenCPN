/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_schedule_state.h"

#include "chartdldr_bulk.h"
#include "chartdldr_schedule_config.h"

#include <wx/datetime.h>
#include <wx/intl.h>

namespace {

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
      value > static_cast<long>(ChartDldrScheduledRunOutcome::Aborted)) {
    return false;
  }
  out = static_cast<ChartDldrScheduledRunOutcome>(value);
  return true;
}

void ChartDldrMigrateLegacyScheduleStatus(ChartDldrScheduleConfig& schedule,
                                          const wxString& legacy_last_run) {
  if (!schedule.last_attempt_iso.IsEmpty()) {
    return;
  }

  wxString attempt_iso;
  if (TrySplitLegacyCombinedStatus(schedule.last_status, attempt_iso)) {
    schedule.last_attempt_iso = attempt_iso;
    return;
  }

  if (!legacy_last_run.IsEmpty() && !schedule.last_status.IsEmpty()) {
    schedule.last_attempt_iso = legacy_last_run;
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
    case ChartDldrScheduledRunOutcome::CatalogRefreshFailed:
    case ChartDldrScheduledRunOutcome::Aborted:
      return true;
    case ChartDldrScheduledRunOutcome::Skipped:
    case ChartDldrScheduledRunOutcome::BulkNoAttempts:
    case ChartDldrScheduledRunOutcome::BulkPartialSuccess:
    case ChartDldrScheduledRunOutcome::BulkSuccess:
      break;
  }
  return false;
}

namespace {

wxString ClassifyScheduleStatus(const ChartDldrBulkRunStats& stats) {
  if (stats.catalog_refresh_failures > 0 && stats.attempted == 0) {
    return wxString::Format(
        _(ChartDldrScheduleMessages::kCatalogRefreshFailedFormat),
        stats.catalog_refresh_failures);
  }
  wxString status;
  if (stats.attempted == 0) {
    status = _(ChartDldrScheduleMessages::kNoChartsToUpdate);
  } else if (stats.downloaded_ok() > 0 && stats.failed > 0) {
    status = wxString::Format(
        _(ChartDldrScheduleMessages::kPartialUpdateFormat),
        stats.updated_downloads, stats.new_downloads, stats.failed);
  } else if (stats.downloaded_ok() > 0) {
    status =
        wxString::Format(_(ChartDldrScheduleMessages::kSuccessUpdateFormat),
                         stats.updated_downloads, stats.new_downloads);
  } else {
    status = wxString::Format(_(ChartDldrScheduleMessages::kFailedChartsFormat),
                              stats.failed, stats.attempted);
  }
  if (stats.catalog_refresh_failures > 0 && stats.attempted > 0) {
    status += wxString::Format(
        _(ChartDldrScheduleMessages::kCatalogRefreshSuffixFormat),
        stats.catalog_refresh_failures);
  }
  return status;
}

wxString ClassifyInteractiveMessage(const ChartDldrBulkRunStats& stats) {
  wxString message;
  if (stats.failed > 0) {
    message = ChartDldrBulkChartFailureMessage(stats.failed, stats.attempted);
  }
  if (stats.catalog_refresh_failures > 0) {
    const wxString catalog_message = wxString::Format(
        _(ChartDldrScheduleMessages::kInteractiveCatalogRefreshFailedFormat),
        stats.catalog_refresh_failures);
    if (!message.empty()) {
      message += wxT("\n\n");
    }
    message += catalog_message;
  }
  return message;
}

ChartDldrScheduledRunOutcome ClassifyOutcome(
    const ChartDldrBulkRunStats& stats) {
  if (stats.catalog_refresh_failures > 0) {
    return ChartDldrScheduledRunOutcome::CatalogRefreshFailed;
  }
  if (stats.attempted == 0) {
    return ChartDldrScheduledRunOutcome::BulkNoAttempts;
  }
  if (stats.downloaded_ok() <= 0) {
    return ChartDldrScheduledRunOutcome::BulkAllFailed;
  }
  if (stats.failed > 0) {
    return ChartDldrScheduledRunOutcome::BulkPartialSuccess;
  }
  return ChartDldrScheduledRunOutcome::BulkSuccess;
}

}  // namespace

ChartDldrBulkRunClassification ChartDldrClassifyBulkRun(
    const ChartDldrBulkRunStats& stats) {
  ChartDldrBulkRunClassification result;
  result.outcome = ClassifyOutcome(stats);
  result.schedule_status = ClassifyScheduleStatus(stats);
  result.interactive_message = ClassifyInteractiveMessage(stats);
  return result;
}

ChartDldrScheduledBulkResult ChartDldrScheduledBulkResultFromStats(
    const ChartDldrBulkRunStats& stats) {
  const ChartDldrBulkRunClassification classified =
      ChartDldrClassifyBulkRun(stats);
  ChartDldrScheduledBulkResult result;
  result.outcome = classified.outcome;
  result.status_detail = classified.schedule_status;
  return result;
}

void ChartDldrRecordScheduledAttemptStart(ChartDldrScheduleConfig& schedule,
                                          const wxDateTime& run_time) {
  schedule.last_attempt_iso = ChartDldrFormatScheduleRunIso(run_time);
  schedule.last_outcome = ChartDldrScheduledRunOutcome::Pending;
}

ChartDldrScheduledBulkResult ChartDldrScheduledAbortedResult() {
  ChartDldrScheduledBulkResult result;
  result.outcome = ChartDldrScheduledRunOutcome::Aborted;
  result.status_detail = _("Cancelled");
  return result;
}

void ChartDldrApplyScheduledRunOutcome(
    ChartDldrScheduleConfig& schedule,
    const ChartDldrScheduledBulkResult& result, const wxDateTime* run_time) {
  schedule.last_status = result.status_detail;
  schedule.last_outcome = result.outcome;

  // last_attempt_iso records the *attempt start*, not completion: eligibility
  // keys the once-per-day gate off the attempt day. An explicit run_time stamps
  // it (attempt-start persist, or a caller that owns the timestamp). With no
  // run_time this is a completion apply, so preserve the attempt-start stamp
  // that ChartDldrRecordScheduledAttemptStart wrote; otherwise a run that
  // crosses midnight would re-stamp itself into the next day and consume that
  // day's slot. Only fall back to now when no attempt start was ever recorded.
  if (run_time && run_time->IsValid()) {
    schedule.last_attempt_iso = ChartDldrFormatScheduleRunIso(*run_time);
  } else if (schedule.last_attempt_iso.IsEmpty()) {
    schedule.last_attempt_iso =
        ChartDldrFormatScheduleRunIso(wxDateTime::Now());
  }
}

ChartDldrScheduleConfig ChartDldrScheduleOutcomeCandidate(
    const ChartDldrScheduleConfig& current,
    const ChartDldrScheduledBulkResult& result) {
  ChartDldrScheduleConfig candidate = current;
  ChartDldrApplyScheduledRunOutcome(candidate, result);
  return candidate;
}

ChartDldrScheduleConfig ChartDldrScheduleAttemptStartCandidate(
    const ChartDldrScheduleConfig& current, const wxDateTime& run_time) {
  ChartDldrScheduleConfig candidate = current;
  ChartDldrRecordScheduledAttemptStart(candidate, run_time);
  return candidate;
}

bool ChartDldrPromoteStalePending(ChartDldrScheduleConfig& schedule) {
  if (schedule.last_outcome != ChartDldrScheduledRunOutcome::Pending ||
      schedule.last_attempt_iso.IsEmpty()) {
    return false;
  }
  schedule.last_outcome = ChartDldrScheduledRunOutcome::Aborted;
  schedule.last_status = _("Previous scheduled update was interrupted");
  return true;
}

bool ChartDldrCommitScheduleAttemptStart(
    ChartDldrScheduleConfig& live, const wxDateTime& run_time,
    bool (*persist)(const ChartDldrScheduleConfig& cfg, void* ctx), void* ctx) {
  const ChartDldrScheduleConfig candidate =
      ChartDldrScheduleAttemptStartCandidate(live, run_time);
  if (persist && persist(candidate, ctx)) {
    live = candidate;
    return true;
  }
  ChartDldrScheduledBulkResult failed;
  failed.outcome = ChartDldrScheduledRunOutcome::Aborted;
  failed.status_detail = _("Could not save schedule state");
  ChartDldrApplyScheduledRunOutcome(live, failed, &run_time);
  return false;
}

bool ChartDldrCommitScheduleOutcome(
    ChartDldrScheduleConfig& live, const ChartDldrScheduledBulkResult& result,
    bool (*persist)(const ChartDldrScheduleConfig& cfg, void* ctx), void* ctx) {
  const ChartDldrScheduleConfig candidate =
      ChartDldrScheduleOutcomeCandidate(live, result);
  if (persist && persist(candidate, ctx)) {
    live = candidate;
    return true;
  }
  ChartDldrApplyScheduledRunOutcome(live, result);
  return false;
}
