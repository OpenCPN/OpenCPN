/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_schedule_state.h"

TEST(ChartDldrScheduledCatalogAdvance,
     ProductionWalkerCrossesEmptyFailureAndDownloadedCatalogs) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 3;

  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::CatalogReady, 0));
  EXPECT_EQ(state.next_catalog, 1);

  ChartDldrBulkRunStats fail;
  fail.catalog_refresh_failures = 1;
  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance, 0, fail));
  EXPECT_EQ(state.next_catalog, 2);
  EXPECT_EQ(state.stats.catalog_refresh_failures, 1);

  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::CatalogReady, 2));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::DownloadChart);
  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::TransferWait));

  ChartDldrBulkRunStats catalog_stats;
  catalog_stats.attempted = 2;
  catalog_stats.updated_downloads = 2;
  EXPECT_FALSE(ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance,
                                        0, catalog_stats));
  EXPECT_EQ(state.next_catalog, 3);
  EXPECT_EQ(state.stats.attempted, 2);
  EXPECT_EQ(state.stats.updated_downloads, 2);
  EXPECT_EQ(ChartDldrScheduledBulkResultFromStats(state.stats).outcome,
            ChartDldrScheduledRunOutcome::CatalogRefreshFailed);
}
