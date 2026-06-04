/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk_policy.h"

TEST(ChartDldrBulkSchedule, AdvanceLastRunOnlyAfterSuccessfulDownload) {
  EXPECT_FALSE(ChartDldrShouldAdvanceScheduledLastRun(0, 0, 0));
  EXPECT_FALSE(ChartDldrShouldAdvanceScheduledLastRun(0, 5, 5));
  EXPECT_FALSE(ChartDldrShouldAdvanceScheduledLastRun(0, 5, 2));
  EXPECT_TRUE(ChartDldrShouldAdvanceScheduledLastRun(3, 5, 2));
  EXPECT_TRUE(ChartDldrShouldAdvanceScheduledLastRun(1, 1, 0));
}
