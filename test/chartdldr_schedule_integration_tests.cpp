/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <wx/datetime.h>

#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"
#include "chartdldr_bulk.h"

namespace {

wxDateTime LocalTime(int day, int hour, int minute) {
  wxDateTime dt;
  dt.Set(day, wxDateTime::Jun, 2026, hour, minute, 0);
  return dt;
}

wxDateTime FixedRunTime() {
  wxDateTime dt;
  dt.Set(2, wxDateTime::Jun, 2026, 4, 0, 0);
  return dt;
}

}  // namespace

TEST(ChartDldrScheduleIntegration, SkipThenGateBlocksUntilTomorrow) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule,
                                    ChartDldrScheduledRunOutcome::Skipped,
                                    "no chart sources configured", &run_time);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));

  wxDateTime next_day;
  next_day.Set(3, wxDateTime::Jun, 2026, 3, 5, 0);
  EXPECT_TRUE(schedule.ShouldRunNow(next_day));
}

TEST(ChartDldrScheduleIntegration, FailedBulkBlocksImmediateRetry) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrBulkRunStats failed_stats;
  failed_stats.attempted = 4;
  failed_stats.failed = 4;
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledBulkResultFromStats(failed_stats), &run_time);
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::BulkAllFailed);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 7, 0)));
}

TEST(ChartDldrScheduleIntegration, FailedBulkAllowsSameDayRetryAfterInterval) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrBulkRunStats failed_stats;
  failed_stats.attempted = 4;
  failed_stats.failed = 4;
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledBulkResultFromStats(failed_stats), &run_time);
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 8, 0)));
}

TEST(ChartDldrScheduleIntegration, AbortedBulkAllowsSameDayRetryAfterInterval) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule, ChartDldrScheduledAbortedResult(),
                                    &run_time);
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::Aborted);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 8, 0)));
}

TEST(ChartDldrScheduleIntegration,
     MixedCatalogRefreshFailureAllowsSameDayRetry) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrBulkRunStats stats;
  stats.attempted = 5;
  stats.new_downloads = 2;
  stats.updated_downloads = 3;
  stats.catalog_refresh_failures = 1;
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledBulkResultFromStats(stats), &run_time);
  EXPECT_EQ(schedule.last_outcome,
            ChartDldrScheduledRunOutcome::CatalogRefreshFailed);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 8, 0)));
}

TEST(ChartDldrScheduleIntegration, SuccessfulBulkBlocksSameDay) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrBulkRunStats success_stats;
  success_stats.attempted = 3;
  success_stats.failed = 0;
  success_stats.new_downloads = 1;
  success_stats.updated_downloads = 1;
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledBulkResultFromStats(success_stats),
      &run_time);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrScheduleIntegration, AttemptStartBlocksImmediateRetry) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);
  ChartDldrRecordScheduledAttemptStart(schedule, FixedRunTime());
  EXPECT_EQ(schedule.last_outcome, ChartDldrScheduledRunOutcome::Pending);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 30)));
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 8, 0)));
}

TEST(ChartDldrScheduleIntegration, NoAttemptsBlocksSameDayRetry) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledRunOutcome::BulkNoAttempts,
      "No charts to update", &run_time);
  EXPECT_FALSE(schedule.last_attempt_iso.empty());
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 5, 0)));
}
