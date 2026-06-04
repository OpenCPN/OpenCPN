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

TEST(ChartDldrBulkRunKind, ScheduledVsInteractive) {
  EXPECT_TRUE(ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind::Scheduled));
  EXPECT_FALSE(
      ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind::Interactive));
}

TEST(ChartDldrBulkRunUiPolicy, ScheduledUsesQuietHiddenPanelFlow) {
  const ChartDldrBulkRunUiPolicy visible =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, true);
  EXPECT_EQ(visible.mode, ChartDldrDownloadUiMode::ScheduledBulk);
  EXPECT_TRUE(ChartDldrPolicyUsesLogForErrors(visible));
  EXPECT_FALSE(ChartDldrPolicyConfirmBeforeStart(visible));
  EXPECT_FALSE(ChartDldrPolicyShowFailureSummary(visible));
  EXPECT_TRUE(ChartDldrPolicyRestoreNotebookPage(visible));
  EXPECT_TRUE(ChartDldrPolicySelectDownloadTab(visible));
  EXPECT_FALSE(ChartDldrPolicySyncListSelection(visible));

  const ChartDldrBulkRunUiPolicy hidden =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, false);
  EXPECT_EQ(hidden.mode, ChartDldrDownloadUiMode::ScheduledBulk);
  EXPECT_TRUE(ChartDldrPolicyUsesLogForErrors(hidden));
  EXPECT_FALSE(ChartDldrPolicyRestoreNotebookPage(hidden));
  EXPECT_FALSE(ChartDldrPolicySelectDownloadTab(hidden));
  EXPECT_FALSE(ChartDldrPolicySyncListSelection(hidden));
}

TEST(ChartDldrBulkRunUiPolicy, InteractiveShowsConfirmAndErrors) {
  const ChartDldrBulkRunUiPolicy policy =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Interactive, true);
  EXPECT_EQ(policy.mode, ChartDldrDownloadUiMode::InteractiveBulk);
  EXPECT_FALSE(ChartDldrPolicyUsesLogForErrors(policy));
  EXPECT_TRUE(ChartDldrPolicyConfirmBeforeStart(policy));
  EXPECT_TRUE(ChartDldrPolicyShowFailureSummary(policy));
  EXPECT_TRUE(ChartDldrPolicyRestoreNotebookPage(policy));
  EXPECT_TRUE(ChartDldrPolicySelectDownloadTab(policy));
  EXPECT_TRUE(ChartDldrPolicySyncListSelection(policy));
}

TEST(ChartDldrBulkRunUiPolicy, SelectedChartsAndCatalogPresets) {
  const ChartDldrBulkRunUiPolicy selected =
      ChartDldrSelectedChartsDownloadPolicy();
  EXPECT_EQ(selected.mode, ChartDldrDownloadUiMode::SelectedCharts);
  EXPECT_FALSE(ChartDldrPolicyDeferChartDbApply(selected));
  EXPECT_FALSE(ChartDldrPolicyAllowEmptySelection(selected));
  EXPECT_TRUE(ChartDldrPolicyShowDownloadResultDialogs(selected));

  const ChartDldrBulkRunUiPolicy catalog =
      ChartDldrInteractiveCatalogUpdatePolicy();
  EXPECT_EQ(catalog.mode, ChartDldrDownloadUiMode::CatalogRefresh);
  EXPECT_TRUE(ChartDldrPolicyShowCatalogProgressDialog(catalog));
  EXPECT_FALSE(ChartDldrPolicyDeferChartDbApply(catalog));
}

TEST(ChartDldrBulkRunUiPolicy, RestoreOnlyWhenPanelWasVisible) {
  const ChartDldrBulkRunUiPolicy policy =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, true);
  ChartDldrBulkRunUiSnapshot visible;
  visible.panel_visible = true;
  visible.notebook_page = 2;
  EXPECT_TRUE(ChartDldrBulkRunShouldRestoreUi(policy, visible));

  ChartDldrBulkRunUiSnapshot hidden;
  hidden.panel_visible = false;
  hidden.notebook_page = 2;
  EXPECT_FALSE(ChartDldrBulkRunShouldRestoreUi(policy, hidden));
}

TEST(ChartDldrBulkRunUiPolicy, HiddenScheduledRunDoesNotRestoreNotebook) {
  const ChartDldrBulkRunUiPolicy policy =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, false);
  ChartDldrBulkRunUiSnapshot snapshot;
  snapshot.panel_visible = false;
  snapshot.notebook_page = 0;
  EXPECT_FALSE(ChartDldrBulkRunShouldRestoreUi(policy, snapshot));
}
