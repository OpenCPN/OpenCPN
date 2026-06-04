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

namespace {

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

TEST(ChartDldrScheduleState, OutcomeFromBulkResult) {
  EXPECT_EQ(ChartDldrScheduledOutcomeFromBulkResult(0, 0),
            ChartDldrScheduledRunOutcome::BulkNoAttempts);
  EXPECT_EQ(ChartDldrScheduledOutcomeFromBulkResult(0, 5),
            ChartDldrScheduledRunOutcome::BulkAllFailed);
  EXPECT_EQ(ChartDldrScheduledOutcomeFromBulkResult(3, 5),
            ChartDldrScheduledRunOutcome::BulkSuccess);
  EXPECT_EQ(ChartDldrScheduledOutcomeFromBulkResult(1, 1),
            ChartDldrScheduledRunOutcome::BulkSuccess);
}

TEST(ChartDldrScheduleState, AdvanceLastRunPolicy) {
  EXPECT_TRUE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledRunOutcome::Skipped));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledRunOutcome::BulkSuccess));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledRunOutcome::BulkNoAttempts));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledRunOutcome::BulkAllFailed));
}

TEST(ChartDldrScheduleState, SkippedAdvancesLastRunIso) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = wxEmptyString;
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule,
                                   ChartDldrScheduledRunOutcome::Skipped,
                                   "no chart sources configured", &run_time);
  EXPECT_FALSE(schedule.last_run_iso.empty());
  EXPECT_EQ(schedule.last_status, "2026-06-02 15:30 no chart sources configured");
}

TEST(ChartDldrScheduleState, BulkAllFailedDoesNotAdvanceLastRunIso) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = wxEmptyString;
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule,
                                   ChartDldrScheduledRunOutcome::BulkAllFailed,
                                   "failed: 4 of 4 charts", &run_time);
  EXPECT_TRUE(schedule.last_run_iso.empty());
  EXPECT_EQ(schedule.last_status, "2026-06-02 15:30 failed: 4 of 4 charts");
}

TEST(ChartDldrScheduleState, BulkNoAttemptsAdvancesLastRunIso) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = wxEmptyString;
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule,
                                   ChartDldrScheduledRunOutcome::BulkNoAttempts,
                                   "No charts to update", &run_time);
  EXPECT_FALSE(schedule.last_run_iso.empty());
  EXPECT_EQ(schedule.last_status,
            "2026-06-02 15:30 No charts to update");
}

TEST(ChartDldrScheduleState, BulkSuccessAdvancesLastRunIso) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = wxEmptyString;
  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(schedule,
                                   ChartDldrScheduledRunOutcome::BulkSuccess,
                                   "2 update 3 new", &run_time);
  EXPECT_FALSE(schedule.last_run_iso.empty());
  EXPECT_EQ(schedule.last_status, "2026-06-02 15:30 2 update 3 new");
}

TEST(ChartDldrScheduleState, StatusFromBulkResult) {
  EXPECT_EQ(ChartDldrScheduledStatusFromBulkResult(0, 0, 0, 0, 0),
            "No charts to update");
  EXPECT_EQ(ChartDldrScheduledStatusFromBulkResult(3, 5, 2, 2, 1),
            "1 update 2 new");
  EXPECT_EQ(ChartDldrScheduledStatusFromBulkResult(0, 4, 4, 0, 0),
            "failed: 4 of 4 charts");
}
