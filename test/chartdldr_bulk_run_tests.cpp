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

TEST(ChartDldrBulkRunMode, ScheduledModeIdentified) {
  EXPECT_TRUE(
      ChartDldrBulkRunModeIsScheduled(ChartDldrBulkRunMode::ScheduledBulk));
  EXPECT_FALSE(
      ChartDldrBulkRunModeIsScheduled(ChartDldrBulkRunMode::InteractiveBulk));
  EXPECT_FALSE(
      ChartDldrBulkRunModeIsScheduled(ChartDldrBulkRunMode::SelectedCharts));
  EXPECT_FALSE(
      ChartDldrBulkRunModeIsScheduled(ChartDldrBulkRunMode::CatalogRefresh));
}

TEST(ChartDldrBulkPreflight, SelectedPlanOwnsOnlyCheckedManualActions) {
  const std::vector<ChartDldrBulkPreflightChart> charts{
      {true, false, wxT("automatic"), wxString()},
      {true, true, wxT("manual"), wxT("https://example.test/manual")},
      {false, true, wxT("unchecked"), wxT("https://example.test/unchecked")}};

  const ChartDldrBulkRunPlan plan =
      ChartDldrSelectedChartsPreflightPlanFor(charts, wxT("/charts"));

  EXPECT_TRUE(plan.allow_start);
  ASSERT_EQ(plan.manual_downloads.size(), 1u);
  EXPECT_EQ(plan.manual_downloads.front().title, wxT("manual"));
  EXPECT_EQ(plan.manual_downloads.front().url,
            wxT("https://example.test/manual"));
  EXPECT_EQ(plan.manual_downloads.front().target_dir, wxT("/charts"));
}

TEST(ChartDldrBulkPreflight, EmptySelectionNeverCreatesSessionPlan) {
  const std::vector<ChartDldrBulkPreflightChart> charts{
      {false, true, wxT("unchecked"), wxT("https://example.test/manual")}};
  const ChartDldrBulkRunPlan plan =
      ChartDldrSelectedChartsPreflightPlanFor(charts, wxT("/charts"));
  EXPECT_FALSE(plan.allow_start);
  EXPECT_TRUE(plan.manual_downloads.empty());
}

TEST(ChartDldrBulkSessionPolicy, ModeUiMatrix) {
  const ChartDldrBulkSessionPolicy scheduled_hidden =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, false);
  EXPECT_EQ(scheduled_hidden.error_reporting,
            ChartDldrErrorReporting::SummaryLog);
  EXPECT_EQ(scheduled_hidden.mode, ChartDldrBulkRunMode::ScheduledBulk);
  EXPECT_TRUE(scheduled_hidden.scheduled);
  EXPECT_FALSE(scheduled_hidden.ui_materialize);
  EXPECT_FALSE(scheduled_hidden.ui_show_download_progress);
  EXPECT_FALSE(scheduled_hidden.confirm_before_start);
  EXPECT_TRUE(scheduled_hidden.skip_manual_url_charts);
  EXPECT_FALSE(scheduled_hidden.collect_manual_urls);
  EXPECT_TRUE(scheduled_hidden.allow_empty_selection);
  EXPECT_EQ(scheduled_hidden.walk_bind, ChartDldrBulkWalkBind::AllCatalogs);
  EXPECT_FALSE(scheduled_hidden.manual_plan_before_start);
  EXPECT_FALSE(scheduled_hidden.focus_charts_after);
  EXPECT_TRUE(scheduled_hidden.preselect_all_charts);

  const ChartDldrBulkSessionPolicy scheduled_visible =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, true);
  EXPECT_TRUE(scheduled_visible.scheduled);
  EXPECT_TRUE(scheduled_visible.ui_materialize);
  EXPECT_TRUE(scheduled_visible.ui_select_download_tab);
  EXPECT_TRUE(scheduled_visible.ui_show_download_progress);
  EXPECT_TRUE(scheduled_visible.skip_manual_url_charts);

  const ChartDldrBulkSessionPolicy interactive = ChartDldrBulkSessionPolicyFor(
      ChartDldrBulkRunMode::InteractiveBulk, false);
  EXPECT_EQ(interactive.error_reporting, ChartDldrErrorReporting::Dialog);
  EXPECT_EQ(interactive.mode, ChartDldrBulkRunMode::InteractiveBulk);
  EXPECT_FALSE(interactive.scheduled);
  EXPECT_TRUE(interactive.ui_show_download_progress);
  EXPECT_TRUE(interactive.allow_empty_selection);
  EXPECT_FALSE(interactive.skip_manual_url_charts);
  EXPECT_TRUE(interactive.collect_manual_urls);
  EXPECT_TRUE(interactive.confirm_before_start);
  EXPECT_FALSE(interactive.preserve_chart_selection);
  EXPECT_EQ(interactive.walk_bind, ChartDldrBulkWalkBind::AllCatalogs);
  EXPECT_TRUE(interactive.focus_charts_after);
  EXPECT_FALSE(interactive.manual_plan_before_start);
  EXPECT_FALSE(interactive.preselect_all_charts);

  const ChartDldrBulkSessionPolicy selected = ChartDldrBulkSessionPolicyFor(
      ChartDldrBulkRunMode::SelectedCharts, false);
  EXPECT_FALSE(selected.allow_empty_selection);
  EXPECT_EQ(selected.mode, ChartDldrBulkRunMode::SelectedCharts);
  EXPECT_FALSE(selected.scheduled);
  EXPECT_TRUE(selected.ui_materialize);
  EXPECT_TRUE(selected.ui_show_download_progress);
  EXPECT_FALSE(selected.ui_select_download_tab);
  EXPECT_TRUE(selected.preserve_chart_selection);
  EXPECT_EQ(selected.walk_bind, ChartDldrBulkWalkBind::SingleDownload);
  EXPECT_TRUE(selected.manual_plan_before_start);
  EXPECT_FALSE(selected.collect_manual_urls);
  EXPECT_FALSE(selected.focus_charts_after);
  EXPECT_FALSE(selected.confirm_before_start);

  const ChartDldrBulkSessionPolicy catalog_refresh =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::CatalogRefresh, true);
  EXPECT_EQ(catalog_refresh.mode, ChartDldrBulkRunMode::CatalogRefresh);
  EXPECT_FALSE(catalog_refresh.scheduled);
  EXPECT_TRUE(catalog_refresh.allow_empty_selection);
  EXPECT_FALSE(catalog_refresh.confirm_before_start);
  EXPECT_EQ(catalog_refresh.walk_bind, ChartDldrBulkWalkBind::SinglePrepare);
  EXPECT_TRUE(catalog_refresh.focus_charts_after);
  EXPECT_TRUE(catalog_refresh.ui_materialize);
  EXPECT_FALSE(catalog_refresh.ui_select_download_tab);
  EXPECT_FALSE(catalog_refresh.ui_show_download_progress);
  EXPECT_EQ(catalog_refresh.error_reporting, ChartDldrErrorReporting::Dialog);

  const ChartDldrCatalogUiPolicy from_interactive =
      interactive.CatalogApply(false, true);
  EXPECT_TRUE(from_interactive.materialize);
  EXPECT_FALSE(from_interactive.preserve_selection);
  EXPECT_FALSE(from_interactive.preselect_new);
  EXPECT_TRUE(from_interactive.preselect_updated);
  EXPECT_TRUE(from_interactive.focus_charts_after);

  const ChartDldrCatalogUiPolicy from_selected =
      selected.CatalogApply(false, false);
  EXPECT_TRUE(from_selected.preserve_selection);
  EXPECT_FALSE(from_selected.focus_charts_after);

  const ChartDldrCatalogUiPolicy from_catalog_refresh =
      catalog_refresh.CatalogApply(false, true);
  EXPECT_TRUE(from_catalog_refresh.materialize);
  EXPECT_FALSE(from_catalog_refresh.preserve_selection);
  EXPECT_FALSE(from_catalog_refresh.preselect_new);
  EXPECT_TRUE(from_catalog_refresh.preselect_updated);
  EXPECT_TRUE(from_catalog_refresh.focus_charts_after);

  const ChartDldrCatalogUiPolicy browse =
      ChartDldrBrowseCatalogUiPolicy(false, true);
  EXPECT_TRUE(browse.materialize);
  EXPECT_FALSE(browse.preselect_new);
  EXPECT_TRUE(browse.preselect_updated);
  EXPECT_FALSE(browse.focus_charts_after);
}

TEST(ChartDldrBulkSessionPolicy, RestoreOnlyWhenPanelWasVisible) {
  const ChartDldrBulkSessionPolicy policy =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, true);
  ChartDldrBulkRunUiSnapshot visible;
  visible.panel_visible = true;
  visible.notebook_page = 2;
  EXPECT_TRUE(ChartDldrBulkRunShouldRestoreUi(policy, visible));

  ChartDldrBulkRunUiSnapshot hidden;
  hidden.panel_visible = false;
  hidden.notebook_page = 2;
  EXPECT_FALSE(ChartDldrBulkRunShouldRestoreUi(policy, hidden));
}

TEST(ChartDldrBulkRunStats, AccumulateAddsFields) {
  ChartDldrBulkRunStats stats;
  ChartDldrBulkRunStats delta;
  delta.attempted = 3;
  delta.failed = 1;
  delta.new_downloads = 2;
  delta.updated_downloads = 1;
  delta.catalog_refresh_failures = 2;
  stats.Accumulate(delta);
  EXPECT_EQ(stats.attempted, 3);
  EXPECT_EQ(stats.failed, 1);
  EXPECT_EQ(stats.new_downloads, 2);
  EXPECT_EQ(stats.updated_downloads, 1);
  EXPECT_EQ(stats.catalog_refresh_failures, 2);
}
