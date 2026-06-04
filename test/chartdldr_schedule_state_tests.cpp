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
  EXPECT_FALSE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledRunOutcome::BulkNoAttempts));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledRunOutcome::BulkAllFailed));
}

TEST(ChartDldrScheduleState, SkippedAdvancesLastRunIso) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = wxEmptyString;
  ChartDldrApplyScheduledRunOutcome(schedule,
                                   ChartDldrScheduledRunOutcome::Skipped,
                                   "Skipped: test");
  EXPECT_FALSE(schedule.last_run_iso.empty());
  EXPECT_EQ(schedule.last_status, "Skipped: test");
}

TEST(ChartDldrScheduleState, BulkAllFailedDoesNotAdvanceLastRunIso) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = wxEmptyString;
  ChartDldrApplyScheduledRunOutcome(schedule,
                                   ChartDldrScheduledRunOutcome::BulkAllFailed,
                                   "failed");
  EXPECT_TRUE(schedule.last_run_iso.empty());
  EXPECT_EQ(schedule.last_status, "failed");
}

TEST(ChartDldrScheduleState, BulkNoAttemptsDoesNotAdvanceLastRunIso) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = "2026-06-01 03:00:00";
  const wxString prior = schedule.last_run_iso;
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledRunOutcome::BulkNoAttempts, "empty");
  EXPECT_EQ(schedule.last_run_iso, prior);
}

TEST(ChartDldrScheduleState, BulkSuccessAdvancesLastRunIso) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = wxEmptyString;
  ChartDldrApplyScheduledRunOutcome(schedule,
                                   ChartDldrScheduledRunOutcome::BulkSuccess,
                                   "OK (3 charts)");
  EXPECT_FALSE(schedule.last_run_iso.empty());
}

TEST(ChartDldrScheduleState, SkippedAndBulkSuccessAreOnlyAdvancingOutcomes) {
  for (auto outcome :
       {ChartDldrScheduledRunOutcome::BulkNoAttempts,
        ChartDldrScheduledRunOutcome::BulkAllFailed}) {
    EXPECT_FALSE(ChartDldrScheduledOutcomeAdvancesLastRun(outcome));
  }
}

TEST(ChartDldrScheduleState, StatusFromBulkResult) {
  const wxString none = ChartDldrScheduledStatusFromBulkResult(0, 0, 0);
  const wxString partial = ChartDldrScheduledStatusFromBulkResult(3, 5, 2);
  const wxString ok = ChartDldrScheduledStatusFromBulkResult(5, 5, 0);
  EXPECT_FALSE(none.empty());
  EXPECT_FALSE(partial.empty());
  EXPECT_FALSE(ok.empty());
  EXPECT_NE(partial, ok);
  EXPECT_NE(none, ok);
}
