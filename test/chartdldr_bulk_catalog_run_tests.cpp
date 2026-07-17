#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_schedule_state.h"

TEST(ChartDldrScheduledBulkStep, PrepareCatalogContinuesAfterRefreshBegin) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 2;

  EXPECT_TRUE(ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::MoreWork));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);
  EXPECT_EQ(state.next_catalog, 0);
}

TEST(ChartDldrScheduledBulkStep,
     PrepareCatalogFailedRecordsFailureAndAdvances) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 2;

  ChartDldrBulkRunStats fail;
  fail.catalog_refresh_failures = 1;
  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance, 0, fail));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);
  EXPECT_EQ(state.next_catalog, 1);
  EXPECT_EQ(state.stats.catalog_refresh_failures, 1);
}

TEST(ChartDldrScheduledBulkRun,
     AllCatalogRefreshFailuresRecordCatalogRefreshFailed) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 2;
  int steps = 0;
  const int max_steps = 20;

  while (ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                           state.catalog_bound) &&
         steps++ < max_steps) {
    ChartDldrBulkRunStats fail;
    fail.catalog_refresh_failures = 1;
    if (!ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance, 0,
                                  fail)) {
      break;
    }
  }

  EXPECT_EQ(state.next_catalog, 2);
  EXPECT_EQ(state.stats.catalog_refresh_failures, 2);
  EXPECT_EQ(state.stats.attempted, 0);
  EXPECT_EQ(ChartDldrClassifyBulkRun(state.stats).outcome,
            ChartDldrScheduledRunOutcome::CatalogRefreshFailed);
}

TEST(ChartDldrScheduledBulkStep, PrepareCatalogInProgressContinues) {
  ChartDldrBulkCatalogRunState state;
  state.next_catalog = 1;
  state.catalog_bound = 3;

  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::TransferWait));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);
  EXPECT_EQ(state.next_catalog, 1);
}

TEST(ChartDldrScheduledBulkStep, PrepareCatalogCompleteBeginsChartDownload) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 1;

  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::CatalogReady, 2));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::DownloadChart);
}

TEST(ChartDldrScheduledBulkStep,
     PrepareCatalogCompleteSkipsDownloadWhenNoneSelected) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 2;

  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::CatalogReady, 0));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);
  EXPECT_EQ(state.next_catalog, 1);
}

TEST(ChartDldrScheduledBulkStep, ChartTransferInProgressContinues) {
  ChartDldrBulkCatalogRunState state;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  state.catalog_bound = 2;

  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::TransferWait));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::DownloadChart);
  EXPECT_EQ(state.next_catalog, 0);
}

TEST(ChartDldrScheduledBulkStep, ChartPassCompleteAccumulatesAndAdvances) {
  ChartDldrBulkCatalogRunState state;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  state.catalog_bound = 2;

  EXPECT_TRUE(ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);
  EXPECT_EQ(state.next_catalog, 1);
}

TEST(ChartDldrScheduledBulkStep, LastCatalogFinishesRun) {
  ChartDldrBulkCatalogRunState state;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  state.next_catalog = 1;
  state.catalog_bound = 2;

  EXPECT_FALSE(ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance));
  EXPECT_EQ(state.next_catalog, 2);
}

TEST(ChartDldrScheduledBulkStep, CatalogBoundStopsMidSourceList) {
  ChartDldrBulkCatalogRunState state;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  state.next_catalog = 2;
  state.catalog_bound = 3;

  EXPECT_TRUE(ChartDldrBulkCatalogWalkContinues(2, 3));
  EXPECT_FALSE(ChartDldrBulkCatalogWalkContinues(3, 3));

  EXPECT_FALSE(ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance));
  EXPECT_EQ(state.next_catalog, 3);
}

TEST(ChartDldrScheduledBulkRun, AdvanceAccumulatesStatsAtCatalogBoundary) {
  ChartDldrBulkCatalogRunState state;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  state.next_catalog = 0;
  state.catalog_bound = 1;

  ChartDldrBulkRunStats counters;
  counters.attempted = 5;
  counters.failed = 1;
  counters.new_downloads = 3;
  counters.updated_downloads = 1;

  EXPECT_FALSE(ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance,
                                        0, counters));
  EXPECT_EQ(state.stats.attempted, 5);
  EXPECT_EQ(state.stats.failed, 1);
  EXPECT_EQ(state.stats.new_downloads, 3);
  EXPECT_EQ(state.stats.updated_downloads, 1);
  EXPECT_EQ(state.next_catalog, 1);
}

TEST(ChartDldrScheduledBulkRun, AdvanceMovesToDownloadAfterRefresh) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 1;

  ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::CatalogReady, 1);
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::DownloadChart);
}

TEST(ChartDldrScheduledBulkRun, SimulatedTwoCatalogRun) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 2;
  bool chart_download_active = false;
  bool async_refresh_started = false;
  int steps = 0;
  const int max_steps = 50;

  while (ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                           state.catalog_bound) &&
         steps++ < max_steps) {
    bool continue_run = false;

    if (state.phase == ChartDldrBulkCatalogPhase::PrepareCatalog) {
      if (!async_refresh_started) {
        continue_run =
            ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::MoreWork);
        async_refresh_started = true;
      } else {
        continue_run = ChartDldrAdvanceBulkWalk(
            state, ChartDldrBulkWalkStep::CatalogReady, 2);
        chart_download_active = true;
        async_refresh_started = false;
      }
    } else {
      ChartDldrBulkRunStats counters;
      if (chart_download_active) {
        counters.attempted = 2;
      }
      continue_run = ChartDldrAdvanceBulkWalk(
          state, ChartDldrBulkWalkStep::Advance, 0, counters);
      chart_download_active = false;
    }

    if (!continue_run) {
      break;
    }
  }

  EXPECT_EQ(state.next_catalog, 2);
  EXPECT_EQ(state.stats.attempted, 4);
  EXPECT_FALSE(chart_download_active);
}

TEST(ChartDldrScheduledBulkRun, ChartTransferSteps) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 1;
  bool chart_download_active = false;
  bool transfer_in_progress = false;
  bool async_refresh_started = false;
  int steps = 0;
  const int max_steps = 50;

  while (ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                           state.catalog_bound) &&
         steps++ < max_steps) {
    bool continue_run = false;

    if (state.phase == ChartDldrBulkCatalogPhase::PrepareCatalog) {
      if (!async_refresh_started) {
        continue_run =
            ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::MoreWork);
        async_refresh_started = true;
      } else {
        continue_run = ChartDldrAdvanceBulkWalk(
            state, ChartDldrBulkWalkStep::CatalogReady, 1);
        chart_download_active = true;
        transfer_in_progress = true;
        async_refresh_started = false;
      }
    } else if (transfer_in_progress) {
      continue_run =
          ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::TransferWait);
      transfer_in_progress = false;
    } else if (chart_download_active) {
      ChartDldrBulkRunStats counters;
      counters.attempted = 1;
      continue_run = ChartDldrAdvanceBulkWalk(
          state, ChartDldrBulkWalkStep::Advance, 0, counters);
      chart_download_active = false;
    } else {
      continue_run =
          ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance);
    }

    if (!continue_run) {
      break;
    }
  }

  EXPECT_EQ(state.next_catalog, 1);
  EXPECT_EQ(state.stats.attempted, 1);
  EXPECT_FALSE(chart_download_active);
}

TEST(ChartDldrBulkCatalogWalk, BindSingleCatalogWalkDownloadSkipsPrepare) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 5;
  state.next_catalog = 0;
  state.phase = ChartDldrBulkCatalogPhase::PrepareCatalog;

  EXPECT_TRUE(ChartDldrBindSingleCatalogWalk(
      state, 2, ChartDldrBulkCatalogPhase::DownloadChart));
  EXPECT_EQ(state.next_catalog, 2);
  EXPECT_EQ(state.catalog_bound, 3);
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::DownloadChart);
  EXPECT_TRUE(ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                                state.catalog_bound));
}

TEST(ChartDldrBulkCatalogWalk, BindSingleCatalogWalkPrepareKeepsPrepare) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 5;
  state.next_catalog = 0;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;

  EXPECT_TRUE(ChartDldrBindSingleCatalogWalk(
      state, 1, ChartDldrBulkCatalogPhase::PrepareCatalog));
  EXPECT_EQ(state.next_catalog, 1);
  EXPECT_EQ(state.catalog_bound, 2);
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);
  EXPECT_TRUE(ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                                state.catalog_bound));
}

TEST(ChartDldrBulkCatalogWalk, BindSingleCatalogWalkRejectsNegativeIndex) {
  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 2;
  EXPECT_FALSE(ChartDldrBindSingleCatalogWalk(
      state, -1, ChartDldrBulkCatalogPhase::DownloadChart));
  EXPECT_EQ(state.next_catalog, 0);
  EXPECT_EQ(state.catalog_bound, 2);
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);
  EXPECT_FALSE(ChartDldrBindSingleCatalogWalk(
      state, -1, ChartDldrBulkCatalogPhase::PrepareCatalog));
}

TEST(ChartDldrBulkCatalogWalk, SinglePrepareBindsLikeSingleDownloadSibling) {
  // StartBulk(CatalogRefresh) binds prepare-phase; SelectedCharts binds
  // download-phase. Both pin a single catalog index.
  ChartDldrBulkCatalogRunState prepare;
  EXPECT_TRUE(ChartDldrBindSingleCatalogWalk(
      prepare, 4, ChartDldrBulkCatalogPhase::PrepareCatalog));
  EXPECT_EQ(prepare.next_catalog, 4);
  EXPECT_EQ(prepare.catalog_bound, 5);
  EXPECT_EQ(prepare.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);

  ChartDldrBulkCatalogRunState download;
  EXPECT_TRUE(ChartDldrBindSingleCatalogWalk(
      download, 4, ChartDldrBulkCatalogPhase::DownloadChart));
  EXPECT_EQ(download.next_catalog, 4);
  EXPECT_EQ(download.catalog_bound, 5);
  EXPECT_EQ(download.phase, ChartDldrBulkCatalogPhase::DownloadChart);

  const ChartDldrBulkSessionPolicy policy =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::CatalogRefresh, true);
  EXPECT_EQ(policy.walk_bind, ChartDldrBulkWalkBind::SinglePrepare);
  EXPECT_NE(policy.walk_bind, ChartDldrBulkWalkBind::AllCatalogs);
}
