/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"
#include "chartdldr_bulk.h"

namespace {

wxString ScheduledDisplay(const wxString& stamp, const wxString& detail) {
  return stamp + ChartDldrScheduledDisplaySeparator() + detail;
}

wxDateTime FixedRunTime() {
  wxDateTime dt;
  dt.Set(2, wxDateTime::Jun, 2026, 15, 30, 0);
  return dt;
}

}  // namespace

TEST(ChartDldrScheduleState, NeverRunDisplayText) {
  ChartDldrScheduleConfig schedule;
  EXPECT_EQ(ChartDldrScheduledNeverRunDisplayText(), "-- : --");
  EXPECT_EQ(ChartDldrFormatScheduledLastRunDisplay(schedule), "-- : --");
}

TEST(ChartDldrScheduleState, ClassifyBulkRunOutcome) {
  ChartDldrBulkRunStats none;
  EXPECT_EQ(ChartDldrClassifyBulkRun(none).outcome,
            ChartDldrScheduledRunOutcome::BulkNoAttempts);

  ChartDldrBulkRunStats all_failed;
  all_failed.attempted = 5;
  all_failed.failed = 5;
  EXPECT_EQ(ChartDldrClassifyBulkRun(all_failed).outcome,
            ChartDldrScheduledRunOutcome::BulkAllFailed);

  ChartDldrBulkRunStats success;
  success.attempted = 5;
  success.failed = 0;
  success.new_downloads = 3;
  success.updated_downloads = 2;
  EXPECT_EQ(ChartDldrClassifyBulkRun(success).outcome,
            ChartDldrScheduledRunOutcome::BulkSuccess);

  ChartDldrBulkRunStats partial;
  partial.attempted = 5;
  partial.failed = 2;
  partial.new_downloads = 1;
  partial.updated_downloads = 2;
  EXPECT_EQ(ChartDldrClassifyBulkRun(partial).outcome,
            ChartDldrScheduledRunOutcome::BulkPartialSuccess);

  ChartDldrBulkRunStats single;
  single.attempted = 1;
  single.failed = 0;
  EXPECT_EQ(ChartDldrClassifyBulkRun(single).outcome,
            ChartDldrScheduledRunOutcome::BulkSuccess);

  ChartDldrBulkRunStats catalog_failed;
  catalog_failed.catalog_refresh_failures = 2;
  EXPECT_EQ(ChartDldrClassifyBulkRun(catalog_failed).outcome,
            ChartDldrScheduledRunOutcome::CatalogRefreshFailed);

  ChartDldrBulkRunStats mixed_catalog_failed;
  mixed_catalog_failed.attempted = 4;
  mixed_catalog_failed.new_downloads = 2;
  mixed_catalog_failed.updated_downloads = 2;
  mixed_catalog_failed.catalog_refresh_failures = 1;
  EXPECT_EQ(ChartDldrClassifyBulkRun(mixed_catalog_failed).outcome,
            ChartDldrScheduledRunOutcome::CatalogRefreshFailed);
  EXPECT_EQ(ChartDldrClassifyBulkRun(mixed_catalog_failed).schedule_status,
            "2 update 2 new, 1 catalog refresh failed");
}

TEST(ChartDldrScheduleState, SameDayRetryPolicy) {
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::Skipped));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::BulkSuccess));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::BulkNoAttempts));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::BulkPartialSuccess));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::BulkAllFailed));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::CatalogRefreshFailed));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::Pending));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::Aborted));
}

TEST(ChartDldrScheduleState, SkippedStoresDetailAndAttemptIso) {
  ChartDldrScheduleConfig schedule;
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule,
                                    ChartDldrScheduledRunOutcome::Skipped,
                                    "no chart sources configured", &run_time);
  EXPECT_FALSE(schedule.last_attempt_iso.empty());
  EXPECT_EQ(schedule.last_status, "no chart sources configured");
  EXPECT_EQ(
      ChartDldrFormatScheduledLastRunDisplay(schedule),
      ScheduledDisplay("2026-06-02 15:30", "no chart sources configured"));
}

TEST(ChartDldrScheduleState, BulkAllFailedStoresOutcomeAndAllowsRetry) {
  ChartDldrScheduleConfig schedule;
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule,
                                    ChartDldrScheduledRunOutcome::BulkAllFailed,
                                    "failed: 4 of 4 charts", &run_time);
  EXPECT_FALSE(schedule.last_attempt_iso.empty());
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::BulkAllFailed);
  EXPECT_EQ(schedule.last_status, "failed: 4 of 4 charts");
  EXPECT_EQ(ChartDldrFormatScheduledLastRunDisplay(schedule),
            ScheduledDisplay("2026-06-02 15:30", "failed: 4 of 4 charts"));
}

TEST(ChartDldrScheduleState, BulkNoAttemptsRecordsOutcome) {
  ChartDldrScheduleConfig schedule;
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledRunOutcome::BulkNoAttempts,
      "No charts to update", &run_time);
  EXPECT_FALSE(schedule.last_attempt_iso.empty());
  EXPECT_EQ(schedule.last_status, "No charts to update");
  EXPECT_EQ(ChartDldrFormatScheduledLastRunDisplay(schedule),
            ScheduledDisplay("2026-06-02 15:30", "No charts to update"));
}

TEST(ChartDldrScheduleState, BulkSuccessRecordsOutcome) {
  ChartDldrScheduleConfig schedule;
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule,
                                    ChartDldrScheduledRunOutcome::BulkSuccess,
                                    "2 update 3 new", &run_time);
  EXPECT_FALSE(schedule.last_attempt_iso.empty());
  EXPECT_EQ(schedule.last_status, "2 update 3 new");
  EXPECT_EQ(ChartDldrFormatScheduledLastRunDisplay(schedule),
            ScheduledDisplay("2026-06-02 15:30", "2 update 3 new"));
}

TEST(ChartDldrScheduleState, CompletionPreservesAttemptStartAcrossMidnight) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.hour = 23;
  schedule.minute = 59;

  // A scheduled attempt starts at 23:59 on Jun 2 and records its start stamp.
  wxDateTime start;
  start.Set(2, wxDateTime::Jun, 2026, 23, 59, 0);
  ChartDldrRecordScheduledAttemptStart(schedule, start);
  const wxString start_iso = schedule.last_attempt_iso;
  ASSERT_FALSE(start_iso.IsEmpty());

  // It finishes a few minutes later, after midnight. The completion apply
  // carries no run_time (it flows through the outcome candidate), so it must
  // keep the start-day stamp rather than re-stamping the completion day.
  ChartDldrScheduledBulkResult result;
  result.outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  result.status_detail = "2 update 3 new";
  const ChartDldrScheduleConfig done =
      ChartDldrScheduleOutcomeCandidate(schedule, result);

  EXPECT_EQ(done.last_attempt_iso, start_iso);
  EXPECT_EQ(done.last_outcome, ChartDldrScheduledRunOutcome::BulkSuccess);

  // The same day stays blocked (a success does not allow same-day retry).
  wxDateTime same_day_later;
  same_day_later.Set(2, wxDateTime::Jun, 2026, 23, 59, 30);
  EXPECT_FALSE(done.ShouldRunNow(same_day_later));

  // The next day's slot must still fire; the midnight-crossing run must not
  // consume Jun 3's schedule.
  wxDateTime next_day_slot;
  next_day_slot.Set(3, wxDateTime::Jun, 2026, 23, 59, 0);
  EXPECT_TRUE(done.ShouldRunNow(next_day_slot));
}

TEST(ChartDldrScheduleState, ClassifyBulkRunScheduleStatus) {
  ChartDldrBulkRunStats none;
  EXPECT_EQ(ChartDldrClassifyBulkRun(none).schedule_status,
            "No charts to update");

  ChartDldrBulkRunStats success;
  success.attempted = 5;
  success.failed = 0;
  success.new_downloads = 2;
  success.updated_downloads = 1;
  EXPECT_EQ(ChartDldrClassifyBulkRun(success).schedule_status,
            "1 update 2 new");

  ChartDldrBulkRunStats partial;
  partial.attempted = 5;
  partial.failed = 2;
  partial.new_downloads = 2;
  partial.updated_downloads = 1;
  EXPECT_EQ(ChartDldrClassifyBulkRun(partial).schedule_status,
            "1 update 2 new, 2 failed");

  ChartDldrBulkRunStats failed;
  failed.attempted = 4;
  failed.failed = 4;
  EXPECT_EQ(ChartDldrClassifyBulkRun(failed).schedule_status,
            "failed: 4 of 4 charts");

  ChartDldrBulkRunStats catalog_failed;
  catalog_failed.catalog_refresh_failures = 2;
  EXPECT_EQ(ChartDldrClassifyBulkRun(catalog_failed).schedule_status,
            "catalog refresh failed: 2 sources");
}

TEST(ChartDldrScheduleState, MigrateLegacyCombinedStatus) {
  ChartDldrScheduleConfig schedule;
  schedule.last_status = "2026-06-02 15:30 2 update 3 new";
  ChartDldrMigrateLegacyScheduleStatus(schedule);
  EXPECT_EQ(schedule.last_status, "2 update 3 new");
  EXPECT_FALSE(schedule.last_attempt_iso.empty());
}

TEST(ChartDldrScheduleState, ParseScheduledRunOutcome) {
  ChartDldrScheduledRunOutcome outcome = ChartDldrScheduledRunOutcome::Pending;
  EXPECT_TRUE(ChartDldrParseScheduledRunOutcome(
      static_cast<long>(ChartDldrScheduledRunOutcome::BulkSuccess), outcome));
  EXPECT_EQ(outcome, ChartDldrScheduledRunOutcome::BulkSuccess);
  EXPECT_FALSE(ChartDldrParseScheduledRunOutcome(999, outcome));
}

TEST(ChartDldrScheduleState, ScheduledBulkResultFromStats) {
  ChartDldrBulkRunStats stats;
  stats.attempted = 5;
  stats.failed = 2;
  stats.new_downloads = 2;
  stats.updated_downloads = 1;
  const ChartDldrScheduledBulkResult result =
      ChartDldrScheduledBulkResultFromStats(stats);
  EXPECT_EQ(result.outcome, ChartDldrScheduledRunOutcome::BulkPartialSuccess);
  EXPECT_EQ(result.status_detail, "1 update 2 new, 2 failed");
}
