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

TEST(ChartDldrBulkModeProfile, ScheduledUsesQuietHiddenPanelFlow) {
  const ChartDldrBulkRunUiPolicy visible =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, true);
  const ChartDldrBulkModeProfile visible_profile =
      ChartDldrBulkModeProfileFor(visible);
  EXPECT_EQ(visible.mode, ChartDldrDownloadUiMode::ScheduledBulk);
  EXPECT_EQ(visible_profile.error_reporting, ChartDldrErrorReporting::SummaryLog);
  EXPECT_FALSE(visible_profile.confirm_before_start);
  EXPECT_FALSE(visible_profile.show_failure_summary);
  EXPECT_TRUE(visible_profile.restore_notebook_page);
  EXPECT_TRUE(visible_profile.select_download_tab);
  EXPECT_FALSE(visible_profile.sync_list_selection);
  EXPECT_TRUE(visible_profile.skip_manual_charts);
  EXPECT_EQ(visible_profile.catalog_refresh, ChartDldrCatalogRefreshMode::AsyncIdle);

  const ChartDldrBulkRunUiPolicy hidden =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, false);
  const ChartDldrBulkModeProfile hidden_profile =
      ChartDldrBulkModeProfileFor(hidden);
  EXPECT_EQ(hidden.mode, ChartDldrDownloadUiMode::ScheduledBulk);
  EXPECT_EQ(hidden_profile.error_reporting, ChartDldrErrorReporting::SummaryLog);
  EXPECT_FALSE(hidden_profile.restore_notebook_page);
  EXPECT_FALSE(hidden_profile.select_download_tab);
  EXPECT_FALSE(hidden_profile.sync_list_selection);
}

TEST(ChartDldrBulkModeProfile, InteractiveShowsConfirmAndErrors) {
  const ChartDldrBulkRunUiPolicy policy =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Interactive, true);
  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(policy);
  EXPECT_EQ(policy.mode, ChartDldrDownloadUiMode::InteractiveBulk);
  EXPECT_EQ(profile.error_reporting, ChartDldrErrorReporting::Dialog);
  EXPECT_TRUE(profile.confirm_before_start);
  EXPECT_TRUE(profile.show_failure_summary);
  EXPECT_TRUE(profile.restore_notebook_page);
  EXPECT_TRUE(profile.select_download_tab);
  EXPECT_TRUE(profile.sync_list_selection);
  EXPECT_FALSE(profile.skip_manual_charts);
  EXPECT_EQ(profile.catalog_refresh, ChartDldrCatalogRefreshMode::SyncBlocking);
}

TEST(ChartDldrBulkModeProfile, SelectedChartsAndCatalogPresets) {
  const ChartDldrBulkModeProfile selected =
      ChartDldrSelectedChartsDownloadProfile();
  EXPECT_FALSE(selected.defer_chart_db_apply);
  EXPECT_FALSE(selected.allow_empty_selection);
  EXPECT_TRUE(selected.show_download_result_dialogs);

  const ChartDldrBulkModeProfile catalog =
      ChartDldrInteractiveCatalogUpdateProfile();
  EXPECT_TRUE(catalog.show_catalog_progress_dialog);
  EXPECT_FALSE(catalog.defer_chart_db_apply);
}

TEST(ChartDldrBulkModeProfile, RestoreOnlyWhenPanelWasVisible) {
  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, true));
  ChartDldrBulkRunUiSnapshot visible;
  visible.panel_visible = true;
  visible.notebook_page = 2;
  EXPECT_TRUE(ChartDldrBulkRunShouldRestoreUi(profile, visible));

  ChartDldrBulkRunUiSnapshot hidden;
  hidden.panel_visible = false;
  hidden.notebook_page = 2;
  EXPECT_FALSE(ChartDldrBulkRunShouldRestoreUi(profile, hidden));
}

TEST(ChartDldrBulkModeProfile, HiddenScheduledRunDoesNotRestoreNotebook) {
  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, false));
  ChartDldrBulkRunUiSnapshot snapshot;
  snapshot.panel_visible = false;
  snapshot.notebook_page = 0;
  EXPECT_FALSE(ChartDldrBulkRunShouldRestoreUi(profile, snapshot));
}

TEST(ChartDldrBulkRunStats, AccumulateAddsFields) {
  ChartDldrBulkRunStats stats;
  ChartDldrBulkRunStats delta;
  delta.attempted = 3;
  delta.failed = 1;
  delta.new_downloads = 2;
  delta.updated_downloads = 1;
  stats.Accumulate(delta);
  EXPECT_EQ(stats.attempted, 3);
  EXPECT_EQ(stats.failed, 1);
  EXPECT_EQ(stats.new_downloads, 2);
  EXPECT_EQ(stats.updated_downloads, 1);
}
