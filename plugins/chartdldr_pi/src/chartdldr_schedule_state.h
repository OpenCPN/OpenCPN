/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_SCHEDULE_STATE_H_
#define CHARTDLDR_SCHEDULE_STATE_H_

#include <wx/datetime.h>
#include <wx/intl.h>
#include <wx/string.h>

class ChartDldrScheduleConfig;
struct ChartDldrBulkRunStats;

/** Result of a scheduled bulk attempt (timer or prefs "Run update now"). */
enum class ChartDldrScheduledRunOutcome {
  Pending,
  Skipped,
  BulkNoAttempts,
  BulkAllFailed,
  BulkPartialSuccess,
  BulkSuccess,
  /** Catalog refreshes failed with no chart attempts. */
  CatalogRefreshFailed,
  /** User cancel or shutdown after attempt start. */
  Aborted,
};

wxString ChartDldrScheduledNeverRunDisplayText();

/** Separator between timestamp and detail in last-run display (" - "). */
wxString ChartDldrScheduledDisplaySeparator();

wxString ChartDldrFormatScheduledRunTimestamp(const wxDateTime& run_time);

/** Persisted attempt/run timestamp (local time, seconds). */
wxString ChartDldrFormatScheduleRunIso(const wxDateTime& run_time);

bool ChartDldrParseScheduleRunIso(const wxString& iso, wxDateTime& out);

wxString ChartDldrFormatScheduledLastRunDisplay(
    const ChartDldrScheduleConfig& schedule);

/**
 * Outcome-only same-day retry defaults (Pending / all-failed / catalog-only /
 * Aborted). Mixed catalog failures with chart success keep a non-retryable
 * outcome and set allows_same_day_retry on the classified result instead.
 */
bool ChartDldrScheduledOutcomeAllowsSameDayRetry(
    ChartDldrScheduledRunOutcome outcome);

/** True when the outcome is retryable or any catalog refresh failed. */
bool ChartDldrBulkRunAllowsSameDayRetry(ChartDldrScheduledRunOutcome outcome,
                                        int catalog_refresh_failures);

bool ChartDldrParseScheduledRunOutcome(long value,
                                       ChartDldrScheduledRunOutcome& out);

struct ChartDldrScheduledBulkResult {
  ChartDldrScheduledRunOutcome outcome =
      ChartDldrScheduledRunOutcome::BulkNoAttempts;
  wxString status_detail;
  /** Persisted separately from outcome so catalog-mixed runs can retry. */
  bool allows_same_day_retry = false;
};

/** Outcome + status strings for a finished bulk run (scheduled or interactive).
 */
struct ChartDldrBulkRunClassification {
  ChartDldrScheduledRunOutcome outcome =
      ChartDldrScheduledRunOutcome::BulkNoAttempts;
  wxString schedule_status;
  wxString interactive_message;
  bool allows_same_day_retry = false;
};

ChartDldrBulkRunClassification ChartDldrClassifyBulkRun(
    const ChartDldrBulkRunStats& stats);

ChartDldrScheduledBulkResult ChartDldrScheduledBulkResultFromStats(
    const ChartDldrBulkRunStats& stats);

/** Persist attempt time before a scheduled bulk run starts (blocks retry
 * storm). */
void ChartDldrRecordScheduledAttemptStart(ChartDldrScheduleConfig& schedule,
                                          const wxDateTime& run_time);

/** Persist Aborted after cancel/shutdown of a started scheduled attempt. */
ChartDldrScheduledBulkResult ChartDldrScheduledAbortedResult();

void ChartDldrApplyScheduledRunOutcome(
    ChartDldrScheduleConfig& schedule,
    const ChartDldrScheduledBulkResult& result,
    const wxDateTime* run_time = nullptr);

ChartDldrScheduleConfig ChartDldrScheduleOutcomeCandidate(
    const ChartDldrScheduleConfig& current,
    const ChartDldrScheduledBulkResult& result);
ChartDldrScheduleConfig ChartDldrScheduleAttemptStartCandidate(
    const ChartDldrScheduleConfig& current, const wxDateTime& run_time);
/** Convert a persisted Pending from an interrupted process to Aborted. */
bool ChartDldrPromoteStalePending(ChartDldrScheduleConfig& schedule);

/**
 * Schedule persist policy (attempt start vs run outcome):
 *
 * - Attempt start: persist Pending + attempt stamp. On persist failure, record
 *   Aborted in live config so the timer backs off instead of retrying every
 *   minute with stale disk state.
 * - Run outcome: persist the classified result (outcome, status, and
 *   allows_same_day_retry). On persist failure, still apply that result in live
 *   config so a finished run does not leave stale Pending that
 *   PromoteStalePending would misread after restart.
 *
 * Both return false when disk persist fails; callers may log but need not roll
 * back in-memory state.
 */
bool ChartDldrCommitScheduleAttemptStart(
    ChartDldrScheduleConfig& live, const wxDateTime& run_time,
    bool (*persist)(const ChartDldrScheduleConfig& cfg, void* ctx), void* ctx);

bool ChartDldrCommitScheduleOutcome(
    ChartDldrScheduleConfig& live, const ChartDldrScheduledBulkResult& result,
    bool (*persist)(const ChartDldrScheduleConfig& cfg, void* ctx), void* ctx);

inline void ChartDldrApplyScheduledRunOutcome(
    ChartDldrScheduleConfig& schedule, ChartDldrScheduledRunOutcome outcome,
    const wxString& status_detail, const wxDateTime* run_time = nullptr) {
  ChartDldrScheduledBulkResult result;
  result.outcome = outcome;
  result.status_detail = status_detail;
  result.allows_same_day_retry =
      ChartDldrScheduledOutcomeAllowsSameDayRetry(outcome);
  ChartDldrApplyScheduledRunOutcome(schedule, result, run_time);
}

/** English msgids for persisted schedule status strings. */
namespace ChartDldrScheduleMessages {

inline constexpr const char* kNoChartsToUpdate = "No charts to update";

inline constexpr const char* kBulkDisabled =
    "bulk update disabled in preferences";
inline constexpr const char* kNoSources = "no chart sources configured";
inline constexpr const char* kPanelUnavailable = "downloader panel unavailable";
inline constexpr const char* kSessionActive =
    "an update is already in progress";

inline constexpr const char* kCatalogRefreshFailedFormat =
    "catalog refresh failed: %d sources";
inline constexpr const char* kFailedChartsFormat = "failed: %d of %d charts";
inline constexpr const char* kPartialUpdateFormat =
    "%d update %d new, %d failed";
inline constexpr const char* kSuccessUpdateFormat = "%d update %d new";
inline constexpr const char* kCatalogRefreshSuffixFormat =
    ", %d catalog refresh failed";

inline constexpr const char* kInteractiveCatalogRefreshFailedFormat =
    "Catalog refresh failed for %d source(s).\nVerify there is a working "
    "Internet connection and repeat the operation if needed.";

inline constexpr const char* kSkipStatusMsgids[] = {
    kBulkDisabled,
    kNoSources,
    kPanelUnavailable,
};

}  // namespace ChartDldrScheduleMessages

/** True when @a status equals @a msgid or its wx translation. */
inline bool ChartDldrScheduleStatusMatchesMsgid(const wxString& status,
                                                const char* msgid) {
  return status == msgid || status == wxGetTranslation(msgid);
}

#endif  // CHARTDLDR_SCHEDULE_STATE_H_
