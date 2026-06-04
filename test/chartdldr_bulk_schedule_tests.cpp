/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_run_kind.h"
#include "chartdldr_schedule_state.h"

TEST(ChartDldrBulkRunKind, ScheduledVsInteractive) {
  EXPECT_TRUE(ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind::Scheduled));
  EXPECT_FALSE(
      ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind::Interactive));
}

TEST(ChartDldrBulkSchedule, SkipAndSuccessShareAdvancePolicy) {
  EXPECT_TRUE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledRunOutcome::Skipped));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledOutcomeFromBulkResult(1, 3)));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledOutcomeFromBulkResult(0, 3)));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledOutcomeFromBulkResult(0, 0)));
}

TEST(ChartDldrBulkSchedule, PartialFailureStillSuccessWhenDownloadsOk) {
  EXPECT_EQ(ChartDldrScheduledOutcomeFromBulkResult(2, 5),
            ChartDldrScheduledRunOutcome::BulkSuccess);
  EXPECT_TRUE(ChartDldrScheduledOutcomeAdvancesLastRun(
      ChartDldrScheduledOutcomeFromBulkResult(2, 5)));
}
