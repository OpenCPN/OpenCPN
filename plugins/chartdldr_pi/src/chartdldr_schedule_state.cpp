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

wxString ChartDldrScheduledNeverRunDisplayText() { return _("-- : --"); }

wxString ChartDldrScheduledDisplaySeparator() { return " - "; }

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

bool ChartDldrBulkRunAllowsSameDayRetry(ChartDldrScheduledRunOutcome outcome,
                                        int catalog_refresh_failures) {
  return ChartDldrScheduledOutcomeAllowsSameDayRetry(outcome) ||
         catalog_refresh_failures > 0;
}

namespace {

wxString FormatCatalogRefreshInteractiveMessage(int catalog_refresh_failures) {
  return wxString::Format(
      _(ChartDldrScheduleMessages::kInteractiveCatalogRefreshFailedFormat),
      catalog_refresh_failures);
}

void AppendInteractiveParagraph(wxString& message, const wxString& paragraph) {
  if (!message.empty()) {
    message += wxT("\n\n");
  }
  message += paragraph;
}

/**
 * Single ordered decision for a bulk run: catalog-only refresh failure →
 * no-attempts → all-failed → partial → success. Catalog refresh failures that
 * accompany chart attempts keep the chart outcome, append status/interactive
 * text, and set allows_same_day_retry without renaming the outcome.
 */
struct BulkRunClassificationCore {
  ChartDldrScheduledRunOutcome outcome =
      ChartDldrScheduledRunOutcome::BulkNoAttempts;
  wxString schedule_status;
  wxString interactive_message;
  bool allows_same_day_retry = false;
};

BulkRunClassificationCore ClassifyBulkRunCore(
    const ChartDldrBulkRunStats& stats) {
  BulkRunClassificationCore core;

  if (stats.catalog_refresh_failures > 0 && stats.attempted == 0) {
    core.outcome = ChartDldrScheduledRunOutcome::CatalogRefreshFailed;
    core.schedule_status = wxString::Format(
        _(ChartDldrScheduleMessages::kCatalogRefreshFailedFormat),
        stats.catalog_refresh_failures);
    core.interactive_message =
        FormatCatalogRefreshInteractiveMessage(stats.catalog_refresh_failures);
    core.allows_same_day_retry = true;
    return core;
  }

  if (stats.attempted == 0) {
    core.outcome = ChartDldrScheduledRunOutcome::BulkNoAttempts;
    core.schedule_status = _(ChartDldrScheduleMessages::kNoChartsToUpdate);
    core.allows_same_day_retry =
        ChartDldrBulkRunAllowsSameDayRetry(core.outcome, 0);
    return core;
  }

  if (stats.downloaded_ok() <= 0) {
    core.outcome = ChartDldrScheduledRunOutcome::BulkAllFailed;
    core.schedule_status =
        wxString::Format(_(ChartDldrScheduleMessages::kFailedChartsFormat),
                         stats.failed, stats.attempted);
    core.interactive_message =
        ChartDldrBulkChartFailureMessage(stats.failed, stats.attempted);
  } else if (stats.failed > 0) {
    core.outcome = ChartDldrScheduledRunOutcome::BulkPartialSuccess;
    core.schedule_status = wxString::Format(
        _(ChartDldrScheduleMessages::kPartialUpdateFormat),
        stats.updated_downloads, stats.new_downloads, stats.failed);
    core.interactive_message =
        ChartDldrBulkChartFailureMessage(stats.failed, stats.attempted);
  } else {
    core.outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
    core.schedule_status =
        wxString::Format(_(ChartDldrScheduleMessages::kSuccessUpdateFormat),
                         stats.updated_downloads, stats.new_downloads);
  }

  // Catalog refresh failures alongside chart attempts: keep the chart outcome
  // and surface the refresh failure in both status and interactive text.
  if (stats.catalog_refresh_failures > 0) {
    core.schedule_status += wxString::Format(
        _(ChartDldrScheduleMessages::kCatalogRefreshSuffixFormat),
        stats.catalog_refresh_failures);
    AppendInteractiveParagraph(
        core.interactive_message,
        FormatCatalogRefreshInteractiveMessage(stats.catalog_refresh_failures));
  }

  core.allows_same_day_retry = ChartDldrBulkRunAllowsSameDayRetry(
      core.outcome, stats.catalog_refresh_failures);
  return core;
}

}  // namespace

ChartDldrBulkRunClassification ChartDldrClassifyBulkRun(
    const ChartDldrBulkRunStats& stats) {
  const BulkRunClassificationCore core = ClassifyBulkRunCore(stats);
  ChartDldrBulkRunClassification result;
  result.outcome = core.outcome;
  result.schedule_status = core.schedule_status;
  result.interactive_message = core.interactive_message;
  result.allows_same_day_retry = core.allows_same_day_retry;
  return result;
}

ChartDldrScheduledBulkResult ChartDldrScheduledBulkResultFromStats(
    const ChartDldrBulkRunStats& stats) {
  const ChartDldrBulkRunClassification classified =
      ChartDldrClassifyBulkRun(stats);
  ChartDldrScheduledBulkResult result;
  result.outcome = classified.outcome;
  result.status_detail = classified.schedule_status;
  result.allows_same_day_retry = classified.allows_same_day_retry;
  return result;
}

void ChartDldrRecordScheduledAttemptStart(ChartDldrScheduleConfig& schedule,
                                          const wxDateTime& run_time) {
  schedule.last_attempt_iso = ChartDldrFormatScheduleRunIso(run_time);
  schedule.last_outcome = ChartDldrScheduledRunOutcome::Pending;
  schedule.last_allows_same_day_retry = true;
}

ChartDldrScheduledBulkResult ChartDldrScheduledAbortedResult() {
  ChartDldrScheduledBulkResult result;
  result.outcome = ChartDldrScheduledRunOutcome::Aborted;
  result.status_detail = _("Cancelled");
  result.allows_same_day_retry = true;
  return result;
}

void ChartDldrApplyScheduledRunOutcome(
    ChartDldrScheduleConfig& schedule,
    const ChartDldrScheduledBulkResult& result, const wxDateTime* run_time) {
  schedule.last_status = result.status_detail;
  schedule.last_outcome = result.outcome;
  schedule.last_allows_same_day_retry = result.allows_same_day_retry;

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
  schedule.last_allows_same_day_retry = true;
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
  failed.allows_same_day_retry = true;
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
