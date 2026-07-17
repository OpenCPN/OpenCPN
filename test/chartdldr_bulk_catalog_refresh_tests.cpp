/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"

TEST(ChartDldrBulkCatalogRefresh, ScheduledUsesSummaryLog) {
  const ChartDldrBulkSessionPolicy policy =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, false);
  EXPECT_EQ(policy.error_reporting, ChartDldrErrorReporting::SummaryLog);
}

TEST(ChartDldrBulkCatalogRefresh, InteractiveUsesDialog) {
  const ChartDldrBulkSessionPolicy policy = ChartDldrBulkSessionPolicyFor(
      ChartDldrBulkRunMode::InteractiveBulk, false);
  EXPECT_EQ(policy.error_reporting, ChartDldrErrorReporting::Dialog);
}

TEST(ChartDldrBulkCatalogRefresh,
     SinglePrepareWalkFocusesChartsViaCatalogApply) {
  const ChartDldrBulkSessionPolicy policy =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::CatalogRefresh, true);
  EXPECT_EQ(policy.walk_bind, ChartDldrBulkWalkBind::SinglePrepare);
  const ChartDldrCatalogUiPolicy ui = policy.CatalogApply(false, true);
  EXPECT_TRUE(ui.materialize);
  EXPECT_TRUE(ui.focus_charts_after);
  EXPECT_FALSE(ui.preserve_selection);
  EXPECT_FALSE(ui.preselect_new);
  EXPECT_TRUE(ui.preselect_updated);
}
