/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"

namespace {

wxDateTime FixedRunTime() {
  wxDateTime dt;
  dt.Set(2, wxDateTime::Jun, 2026, 4, 0, 0);
  return dt;
}

ChartDldrBulkRunStats MakeStats(int attempted, int failed,
                                int new_downloads = 0,
                                int updated_downloads = 0,
                                int catalog_refresh_failures = 0) {
  ChartDldrBulkRunStats stats;
  stats.attempted = attempted;
  stats.failed = failed;
  stats.new_downloads = new_downloads;
  stats.updated_downloads = updated_downloads;
  stats.catalog_refresh_failures = catalog_refresh_failures;
  return stats;
}

}  // namespace

TEST(ChartDldrBulkSchedule, CallbackGateInvalidatesDeferredWork) {
  ChartDldrScheduleCallbackGate gate;
  const uint64_t captured = gate.Capture();
  EXPECT_NE(captured, 0u);
  EXPECT_TRUE(gate.IsValid(captured));
  EXPECT_FALSE(gate.IsValid(0));
  gate.Invalidate();
  EXPECT_FALSE(gate.IsValid(captured));
  const uint64_t refreshed = gate.Capture();
  EXPECT_NE(refreshed, 0u);
  EXPECT_TRUE(gate.IsValid(refreshed));
  EXPECT_NE(captured, refreshed);
}

TEST(ChartDldrBulkSchedule, CallbackGateRejectsStaleZeroToken) {
  ChartDldrScheduleCallbackGate first;
  EXPECT_FALSE(first.IsValid(0));
  ChartDldrScheduleCallbackGate second;
  EXPECT_FALSE(second.IsValid(0));
  EXPECT_NE(first.Capture(), second.Capture());
}

TEST(ChartDldrBulkSchedule, SkipAndSuccessBlockSameDayRetry) {
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::Skipped));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrClassifyBulkRun(MakeStats(3, 0)).outcome));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrClassifyBulkRun(MakeStats(3, 3)).outcome));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrClassifyBulkRun(MakeStats(0, 0)).outcome));
}

TEST(ChartDldrBulkSchedule, PartialFailureIsDistinctOutcome) {
  const ChartDldrBulkRunStats stats = MakeStats(5, 3, 1, 1);
  EXPECT_EQ(ChartDldrClassifyBulkRun(stats).outcome,
            ChartDldrScheduledRunOutcome::BulkPartialSuccess);
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrClassifyBulkRun(stats).outcome));
  EXPECT_EQ(ChartDldrClassifyBulkRun(stats).schedule_status,
            "1 update 1 new, 3 failed");
}

TEST(ChartDldrBulkSchedule, PendingAttemptAllowsThrottledRetry) {
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::Pending));
}

TEST(ChartDldrBulkSchedule, CatalogRefreshFailuresUseCatalogStatus) {
  const ChartDldrBulkRunStats stats = MakeStats(0, 0, 0, 0, 2);
  EXPECT_EQ(ChartDldrClassifyBulkRun(stats).outcome,
            ChartDldrScheduledRunOutcome::CatalogRefreshFailed);
  EXPECT_EQ(ChartDldrClassifyBulkRun(stats).schedule_status,
            "catalog refresh failed: 2 sources");
}

TEST(ChartDldrBulkSchedule, MixedCatalogRefreshFailureIsVisible) {
  const ChartDldrBulkRunStats stats = MakeStats(5, 0, 2, 1, 1);
  EXPECT_EQ(ChartDldrClassifyBulkRun(stats).outcome,
            ChartDldrScheduledRunOutcome::CatalogRefreshFailed);
  EXPECT_EQ(ChartDldrClassifyBulkRun(stats).schedule_status,
            "1 update 2 new, 1 catalog refresh failed");
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledBulkResultFromStats(stats), &run_time);
  EXPECT_EQ(schedule.last_outcome,
            ChartDldrScheduledRunOutcome::CatalogRefreshFailed);
}

TEST(ChartDldrBulkSchedule, AttemptStartCandidateDoesNotMutateLiveConfig) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  schedule.last_status = wxT("previous-ok");
  schedule.last_attempt_iso = wxT("2026-01-01T03:00:00Z");

  wxDateTime when;
  when.Set(2, wxDateTime::Jun, 2026, 4, 0, 0);
  const ChartDldrScheduleConfig candidate =
      ChartDldrScheduleAttemptStartCandidate(schedule, when);

  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::BulkSuccess);
  EXPECT_EQ(schedule.last_status, wxT("previous-ok"));
  EXPECT_EQ(schedule.last_attempt_iso, wxT("2026-01-01T03:00:00Z"));
  EXPECT_EQ(candidate.last_outcome, ChartDldrScheduledRunOutcome::Pending);
  EXPECT_EQ(candidate.last_attempt_iso, ChartDldrFormatScheduleRunIso(when));
}

TEST(ChartDldrBulkSchedule, OutcomeCandidateDoesNotMutateLiveConfig) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::Pending;
  schedule.last_status = wxT("in-progress");
  schedule.last_attempt_iso = wxT("2026-01-01T03:00:00Z");

  ChartDldrScheduledBulkResult result;
  result.outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  result.status_detail = wxT("2 updates");
  const ChartDldrScheduleConfig candidate =
      ChartDldrScheduleOutcomeCandidate(schedule, result);

  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::Pending);
  EXPECT_EQ(schedule.last_status, wxT("in-progress"));
  EXPECT_EQ(candidate.last_outcome, ChartDldrScheduledRunOutcome::BulkSuccess);
  EXPECT_EQ(candidate.last_status, wxT("2 updates"));
}

namespace {

bool PersistAlwaysFails(const ChartDldrScheduleConfig&, void*) { return false; }

bool PersistAlwaysOk(const ChartDldrScheduleConfig&, void*) { return true; }

}  // namespace

TEST(ChartDldrBulkSchedule, CommitAttemptStartBackoffWhenPersistFails) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  schedule.last_status = wxT("previous-ok");
  schedule.last_attempt_iso = wxT("2026-01-01T03:00:00Z");

  wxDateTime when;
  when.Set(2, wxDateTime::Jun, 2026, 4, 0, 0);
  EXPECT_FALSE(ChartDldrCommitScheduleAttemptStart(
      schedule, when, &PersistAlwaysFails, nullptr));
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::Aborted);
  EXPECT_FALSE(schedule.last_status.empty());
  EXPECT_EQ(schedule.last_attempt_iso, ChartDldrFormatScheduleRunIso(when));

  wxDateTime soon_after = when;
  soon_after.Add(wxTimeSpan(0, 30, 0));
  EXPECT_FALSE(schedule.ShouldRunNow(soon_after));

  EXPECT_TRUE(ChartDldrCommitScheduleAttemptStart(
      schedule, when, &PersistAlwaysOk, nullptr));
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::Pending);
  EXPECT_EQ(schedule.last_attempt_iso, ChartDldrFormatScheduleRunIso(when));
}

TEST(ChartDldrBulkSchedule, AttemptStartPersistFailureBackoffInMemory) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  wxDateTime when;
  when.Set(2, wxDateTime::Jun, 2026, 4, 0, 0);
  EXPECT_FALSE(ChartDldrCommitScheduleAttemptStart(
      schedule, when, &PersistAlwaysFails, nullptr));
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::Aborted);

  wxDateTime soon_after = when;
  soon_after.Add(wxTimeSpan(0, 30, 0));
  EXPECT_FALSE(schedule.ShouldRunNow(soon_after));
}

TEST(ChartDldrBulkSchedule, CommitOutcomeAppliesInMemoryWhenPersistFails) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::Pending;
  schedule.last_status = wxT("in-progress");
  schedule.last_attempt_iso = wxT("2026-01-01T03:00:00Z");

  ChartDldrScheduledBulkResult result;
  result.outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  result.status_detail = wxT("done");
  EXPECT_FALSE(ChartDldrCommitScheduleOutcome(
      schedule, result, &PersistAlwaysFails, nullptr));
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::BulkSuccess);
  EXPECT_EQ(schedule.last_status, wxT("done"));

  EXPECT_TRUE(ChartDldrCommitScheduleOutcome(
      schedule, result, &PersistAlwaysOk, nullptr));
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::BulkSuccess);
  EXPECT_EQ(schedule.last_status, wxT("done"));
}

TEST(ChartDldrBulkSchedule, StalePendingIsPromotedOnceOnLoad) {
  ChartDldrScheduleConfig schedule;
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::Pending);
  EXPECT_FALSE(ChartDldrPromoteStalePending(schedule));
  schedule.last_attempt_iso = wxT("2026-06-02 04:00:00");
  EXPECT_TRUE(ChartDldrPromoteStalePending(schedule));
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::Aborted);
  EXPECT_FALSE(schedule.last_status.empty());
  EXPECT_FALSE(ChartDldrPromoteStalePending(schedule));
}
