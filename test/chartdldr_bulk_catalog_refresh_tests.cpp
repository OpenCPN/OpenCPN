/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"

TEST(ChartDldrBulkCatalogRefresh, ScheduledUsesSummaryLog) {
  const ChartDldrBulkSessionPolicy policy =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, false);
  EXPECT_EQ(policy.ErrorReporting(), ChartDldrErrorReporting::SummaryLog);
}

TEST(ChartDldrBulkCatalogRefresh, InteractiveUsesDialog) {
  const ChartDldrBulkSessionPolicy policy = ChartDldrBulkSessionPolicyFor(
      ChartDldrBulkRunMode::InteractiveBulk, false);
  EXPECT_EQ(policy.ErrorReporting(), ChartDldrErrorReporting::Dialog);
}

TEST(ChartDldrBulkCatalogRefresh, ManualCatalogUiFocusesCharts) {
  const ChartDldrCatalogUiPolicy ui = ChartDldrManualCatalogRefreshUiPolicy();
  EXPECT_TRUE(ui.materialize);
  EXPECT_TRUE(ui.focus_charts_after);
  EXPECT_FALSE(ui.preserve_selection);
}
