/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_schedule_state.h"

TEST(ChartDldrBulkReport, FailureMessageIncludesCounts) {
  const wxString message = ChartDldrBulkChartFailureMessage(2, 5);
  EXPECT_TRUE(message.Contains(wxT("2")));
  EXPECT_TRUE(message.Contains(wxT("5")));
}

TEST(ChartDldrBulkReport, InteractiveFailureMessageIncludesCatalogFailures) {
  ChartDldrBulkRunStats stats;
  stats.catalog_refresh_failures = 2;
  const wxString message = ChartDldrBulkRunStatusMessage(
      stats, ChartDldrBulkRunStatusPresentation::InteractiveDialog);
  EXPECT_TRUE(message.Contains(wxT("2")));
  EXPECT_TRUE(message.Contains(wxT("Catalog refresh failed")));
}

TEST(ChartDldrBulkReport, InteractiveFailureMessageCombinesChartAndCatalog) {
  ChartDldrBulkRunStats stats;
  stats.attempted = 5;
  stats.failed = 1;
  stats.catalog_refresh_failures = 2;
  const wxString message = ChartDldrBulkRunStatusMessage(
      stats, ChartDldrBulkRunStatusPresentation::InteractiveDialog);
  EXPECT_TRUE(message.Contains(wxT("1")));
  EXPECT_TRUE(message.Contains(wxT("5")));
  EXPECT_TRUE(message.Contains(wxT("Catalog refresh failed")));
  EXPECT_TRUE(message.Contains(wxT("2")));
}

TEST(ChartDldrBulkReport, RunStatusMessageScheduleMatchesClassify) {
  ChartDldrBulkRunStats stats;
  stats.attempted = 4;
  stats.new_downloads = 2;
  stats.updated_downloads = 2;
  stats.catalog_refresh_failures = 1;
  EXPECT_EQ(ChartDldrBulkRunStatusMessage(
                stats, ChartDldrBulkRunStatusPresentation::ScheduleLog),
            ChartDldrClassifyBulkRun(stats).schedule_status);
}
