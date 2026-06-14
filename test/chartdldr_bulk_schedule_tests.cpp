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
#include "chartdldr_schedule_state.h"

TEST(ChartDldrBulkSchedule, SkipAndSuccessBlockSameDayRetry) {
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::Skipped));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledOutcomeFromBulkResult(1, 3, 0)));
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledOutcomeFromBulkResult(0, 3, 3)));
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledOutcomeFromBulkResult(0, 0, 0)));
}

TEST(ChartDldrBulkSchedule, PartialFailureIsDistinctOutcome) {
  EXPECT_EQ(ChartDldrScheduledOutcomeFromBulkResult(2, 5, 3),
            ChartDldrScheduledRunOutcome::BulkPartialSuccess);
  EXPECT_FALSE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledOutcomeFromBulkResult(2, 5, 3)));
  EXPECT_EQ(ChartDldrScheduledStatusFromBulkResult(2, 5, 3, 1, 1),
            "1 update 1 new, 3 failed");
}

TEST(ChartDldrBulkSchedule, PendingAttemptAllowsThrottledRetry) {
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::Pending));
}
