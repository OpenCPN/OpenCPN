/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk_state.h"
#include "chartdldr_chart_classify.h"

TEST(ChartDldrChartIndex, InRangeAcceptsValidIndices) {
  EXPECT_TRUE(ChartDldrChartIndexInRange(0, 3));
  EXPECT_TRUE(ChartDldrChartIndexInRange(2, 3));
}

TEST(ChartDldrChartIndex, RejectsOutOfRangeIndices) {
  EXPECT_FALSE(ChartDldrChartIndexInRange(-1, 3));
  EXPECT_FALSE(ChartDldrChartIndexInRange(3, 3));
  EXPECT_FALSE(ChartDldrChartIndexInRange(4, 3));
}

TEST(ChartDldrChartIndex, EmptyListRejectsAll) {
  EXPECT_FALSE(ChartDldrChartIndexInRange(0, 0));
}

TEST(ChartDldrChartBulkState, ToStatsMapsCounters) {
  ChartDldrChartBulkState chart_bulk;
  chart_bulk.downloading = 4;
  chart_bulk.failed = 1;
  chart_bulk.new_downloads = 2;
  chart_bulk.updated_downloads = 1;

  const ChartDldrBulkRunStats stats = chart_bulk.ToStats();
  EXPECT_EQ(stats.attempted, 4);
  EXPECT_EQ(stats.failed, 1);
  EXPECT_EQ(stats.new_downloads, 2);
  EXPECT_EQ(stats.updated_downloads, 1);
}

TEST(ChartDldrBulkYieldPolicy, AllowsYieldOutsideBulkRun) {
  EXPECT_TRUE(ChartDldrShouldYieldOnDownloadEvent(
      false, ChartDldrTransferPoll::PollOnly));
  EXPECT_TRUE(ChartDldrShouldYieldOnDownloadEvent(
      false, ChartDldrTransferPoll::BlockUntilComplete));
}

TEST(ChartDldrBulkYieldPolicy, BlocksYieldDuringPollOnlyBulkRun) {
  EXPECT_FALSE(ChartDldrShouldYieldOnDownloadEvent(
      true, ChartDldrTransferPoll::PollOnly));
}

TEST(ChartDldrBulkYieldPolicy, AllowsYieldDuringBlockingBulkRun) {
  EXPECT_TRUE(ChartDldrShouldYieldOnDownloadEvent(
      true, ChartDldrTransferPoll::BlockUntilComplete));
}
