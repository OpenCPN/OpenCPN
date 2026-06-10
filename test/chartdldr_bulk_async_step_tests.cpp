#include <gtest/gtest.h>

#include "chartdldr_bulk_async_step.h"

TEST(ChartDldrScheduledBulkStep, SelectCatalogStartsRefresh) {
  ChartDldrScheduledBulkStepInput input;
  input.phase = ChartDldrScheduledBulkPhase::SelectCatalog;
  input.catalog_refresh_started = true;
  input.next_catalog = 0;
  input.catalog_count = 2;

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrDecideScheduledBulkStep(input);
  EXPECT_EQ(decision.next_phase, ChartDldrScheduledBulkPhase::RefreshCatalog);
  EXPECT_TRUE(decision.continue_run);
  EXPECT_EQ(decision.next_catalog, 0u);
}

TEST(ChartDldrScheduledBulkStep, SelectCatalogSkipsWhenRefreshNotStarted) {
  ChartDldrScheduledBulkStepInput input;
  input.phase = ChartDldrScheduledBulkPhase::SelectCatalog;
  input.catalog_refresh_started = false;
  input.next_catalog = 0;
  input.catalog_count = 2;

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrDecideScheduledBulkStep(input);
  EXPECT_EQ(decision.next_phase, ChartDldrScheduledBulkPhase::SelectCatalog);
  EXPECT_TRUE(decision.continue_run);
  EXPECT_EQ(decision.next_catalog, 1u);
}

TEST(ChartDldrScheduledBulkStep, RefreshInProgressContinues) {
  ChartDldrScheduledBulkStepInput input;
  input.phase = ChartDldrScheduledBulkPhase::RefreshCatalog;
  input.catalog_step = ChartDldrAsyncCatalogStepResult::InProgress;
  input.next_catalog = 1;
  input.catalog_count = 3;

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrDecideScheduledBulkStep(input);
  EXPECT_EQ(decision.next_phase, ChartDldrScheduledBulkPhase::RefreshCatalog);
  EXPECT_TRUE(decision.continue_run);
  EXPECT_EQ(decision.next_catalog, 1u);
}

TEST(ChartDldrScheduledBulkStep, RefreshCompleteBeginsChartDownload) {
  ChartDldrScheduledBulkStepInput input;
  input.phase = ChartDldrScheduledBulkPhase::RefreshCatalog;
  input.catalog_step = ChartDldrAsyncCatalogStepResult::Complete;
  input.next_catalog = 0;
  input.catalog_count = 1;

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrDecideScheduledBulkStep(input);
  EXPECT_EQ(decision.next_phase, ChartDldrScheduledBulkPhase::DownloadChart);
  EXPECT_TRUE(decision.begin_chart_download);
  EXPECT_TRUE(decision.continue_run);
}

TEST(ChartDldrScheduledBulkStep, ChartTransferInProgressContinues) {
  ChartDldrScheduledBulkStepInput input;
  input.phase = ChartDldrScheduledBulkPhase::DownloadChart;
  input.chart_step = ChartDldrBulkChartStepResult::TransferInProgress;
  input.next_catalog = 0;
  input.catalog_count = 2;

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrDecideScheduledBulkStep(input);
  EXPECT_EQ(decision.next_phase, ChartDldrScheduledBulkPhase::DownloadChart);
  EXPECT_TRUE(decision.continue_run);
  EXPECT_FALSE(decision.end_chart_download);
}

TEST(ChartDldrScheduledBulkStep, ChartFinishedAccumulatesAndAdvances) {
  ChartDldrScheduledBulkStepInput input;
  input.phase = ChartDldrScheduledBulkPhase::DownloadChart;
  input.chart_step = ChartDldrBulkChartStepResult::Finished;
  input.next_catalog = 0;
  input.catalog_count = 2;

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrDecideScheduledBulkStep(input);
  EXPECT_EQ(decision.next_phase, ChartDldrScheduledBulkPhase::SelectCatalog);
  EXPECT_EQ(decision.next_catalog, 1u);
  EXPECT_TRUE(decision.end_chart_download);
  EXPECT_TRUE(decision.continue_run);
}

TEST(ChartDldrScheduledBulkStep, LastCatalogFinishesRun) {
  ChartDldrScheduledBulkStepInput input;
  input.phase = ChartDldrScheduledBulkPhase::DownloadChart;
  input.chart_step = ChartDldrBulkChartStepResult::Finished;
  input.next_catalog = 1;
  input.catalog_count = 2;

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrDecideScheduledBulkStep(input);
  EXPECT_EQ(decision.next_catalog, 2u);
  EXPECT_FALSE(decision.continue_run);
}

TEST(ChartDldrScheduledBulkRun, AdvanceAccumulatesStatsAtCatalogBoundary) {
  ChartDldrScheduledBulkRunState state;
  state.phase = ChartDldrScheduledBulkPhase::DownloadChart;
  state.next_catalog = 0;

  ChartDldrScheduledBulkStepInput input;
  input.phase = ChartDldrScheduledBulkPhase::DownloadChart;
  input.chart_step = ChartDldrBulkChartStepResult::Finished;
  input.next_catalog = 0;
  input.catalog_count = 1;

  ChartDldrBulkRunStats counters;
  counters.attempted = 5;
  counters.failed = 1;
  counters.new_downloads = 3;
  counters.updated_downloads = 1;

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrAdvanceScheduledBulkRun(state, input, counters);
  EXPECT_TRUE(decision.end_chart_download);
  EXPECT_EQ(state.stats.attempted, 5);
  EXPECT_EQ(state.stats.failed, 1);
  EXPECT_EQ(state.stats.new_downloads, 3);
  EXPECT_EQ(state.stats.updated_downloads, 1);
  EXPECT_EQ(state.next_catalog, 1u);
  EXPECT_FALSE(decision.continue_run);
}

TEST(ChartDldrScheduledBulkRun, SimulatedTwoCatalogRun) {
  ChartDldrScheduledBulkRunState state;
  const size_t catalog_count = 2;
  bool chart_download_active = false;
  int steps = 0;
  const int max_steps = 50;

  while (state.next_catalog < catalog_count && steps++ < max_steps) {
    ChartDldrScheduledBulkStepInput input;
    input.phase = state.phase;
    input.next_catalog = state.next_catalog;
    input.catalog_count = catalog_count;

    if (state.phase == ChartDldrScheduledBulkPhase::SelectCatalog) {
      input.catalog_refresh_started = true;
    } else if (state.phase == ChartDldrScheduledBulkPhase::RefreshCatalog) {
      input.catalog_step = ChartDldrAsyncCatalogStepResult::Complete;
    } else {
      input.chart_step = chart_download_active
                             ? ChartDldrBulkChartStepResult::Finished
                             : ChartDldrBulkChartStepResult::NotActive;
    }

    ChartDldrBulkRunStats counters;
    if (state.phase == ChartDldrScheduledBulkPhase::DownloadChart &&
        chart_download_active) {
      counters.attempted = 2;
      counters.failed = 0;
    }

    const ChartDldrScheduledBulkStepDecision decision =
        ChartDldrAdvanceScheduledBulkRun(state, input, counters);

    if (decision.begin_chart_download) {
      EXPECT_FALSE(chart_download_active);
      chart_download_active = true;
    }
    if (decision.end_chart_download) {
      EXPECT_TRUE(chart_download_active);
      chart_download_active = false;
    }

    if (!decision.continue_run) {
      break;
    }
  }

  EXPECT_EQ(state.next_catalog, catalog_count);
  EXPECT_EQ(state.stats.attempted, 4);
  EXPECT_FALSE(chart_download_active);
}

TEST(ChartDldrScheduledBulkRun, PollOnlyChartTransferSteps) {
  ChartDldrScheduledBulkRunState state;
  const size_t catalog_count = 1;
  bool chart_download_active = false;
  bool transfer_in_progress = false;
  int steps = 0;
  const int max_steps = 50;

  while (state.next_catalog < catalog_count && steps++ < max_steps) {
    ChartDldrScheduledBulkStepInput input;
    input.phase = state.phase;
    input.next_catalog = state.next_catalog;
    input.catalog_count = catalog_count;

    if (state.phase == ChartDldrScheduledBulkPhase::SelectCatalog) {
      input.catalog_refresh_started = true;
    } else if (state.phase == ChartDldrScheduledBulkPhase::RefreshCatalog) {
      input.catalog_step = ChartDldrAsyncCatalogStepResult::Complete;
    } else if (transfer_in_progress) {
      input.chart_step = ChartDldrBulkChartStepResult::TransferInProgress;
    } else if (chart_download_active) {
      input.chart_step = ChartDldrBulkChartStepResult::Finished;
    } else {
      input.chart_step = ChartDldrBulkChartStepResult::NotActive;
    }

    ChartDldrBulkRunStats counters;
    if (state.phase == ChartDldrScheduledBulkPhase::DownloadChart &&
        chart_download_active && !transfer_in_progress) {
      counters.attempted = 1;
    }

    const ChartDldrScheduledBulkStepDecision decision =
        ChartDldrAdvanceScheduledBulkRun(state, input, counters);

    if (decision.begin_chart_download) {
      chart_download_active = true;
      transfer_in_progress = true;
    }
    if (input.chart_step == ChartDldrBulkChartStepResult::TransferInProgress) {
      transfer_in_progress = false;
    }
    if (decision.end_chart_download) {
      chart_download_active = false;
    }
    if (decision.end_chart_download) {
      EXPECT_EQ(state.stats.attempted, 1);
    }

    if (!decision.continue_run) {
      break;
    }
  }

  EXPECT_EQ(state.next_catalog, catalog_count);
  EXPECT_EQ(state.stats.attempted, 1);
  EXPECT_FALSE(chart_download_active);
}
