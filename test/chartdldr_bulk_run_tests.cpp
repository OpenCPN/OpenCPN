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
  EXPECT_FALSE(ChartDldrBulkPlanOpensDiscoveredManualUrls(plan));
}

TEST(ChartDldrBulkPreflight, EmptySelectionNeverCreatesSessionPlan) {
  const std::vector<ChartDldrBulkPreflightChart> charts{
      {false, true, wxT("unchecked"), wxT("https://example.test/manual")}};
  const ChartDldrBulkRunPlan plan =
      ChartDldrSelectedChartsPreflightPlanFor(charts, wxT("/charts"));
  EXPECT_FALSE(plan.allow_start);
  EXPECT_TRUE(plan.manual_downloads.empty());
}

TEST(ChartDldrBulkPreflight, UpdateAllResolvedPolicyOpensWithoutPrompting) {
  ChartDldrBulkRunPlan plan;
  plan.manual_policy = ChartDldrManualDownloadPolicy::OpenAsDiscovered;
  EXPECT_TRUE(ChartDldrBulkPlanOpensDiscoveredManualUrls(plan));
}

TEST(ChartDldrBulkSessionPolicy, ModeUiMatrix) {
  const ChartDldrBulkSessionPolicy scheduled_hidden =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, false);
  EXPECT_EQ(ChartDldrScheduledUiPresentationFor(false),
            ChartDldrScheduledUiPresentation::Silent);
  EXPECT_EQ(ChartDldrScheduledUiPresentationFor(true),
            ChartDldrScheduledUiPresentation::WithProgress);
  EXPECT_EQ(scheduled_hidden.ErrorReporting(),
            ChartDldrErrorReporting::SummaryLog);
  EXPECT_EQ(scheduled_hidden.mode, ChartDldrBulkRunMode::ScheduledBulk);
  EXPECT_EQ(scheduled_hidden.scheduled_ui,
            ChartDldrScheduledUiPresentation::Silent);
  EXPECT_TRUE(scheduled_hidden.IsScheduled());
  EXPECT_FALSE(scheduled_hidden.UiMaterialize());
  EXPECT_FALSE(scheduled_hidden.UiShowDownloadProgress());
  EXPECT_FALSE(scheduled_hidden.confirm_before_start);
  EXPECT_TRUE(scheduled_hidden.SkipManualUrlCharts());
  EXPECT_TRUE(scheduled_hidden.AllowEmptySelection());
  EXPECT_EQ(scheduled_hidden.walk_bind, ChartDldrBulkWalkBind::AllCatalogs);
  EXPECT_FALSE(scheduled_hidden.manual_plan_before_start);
  EXPECT_FALSE(scheduled_hidden.FocusChartsAfter());
  EXPECT_TRUE(scheduled_hidden.PreselectNew(false));
  EXPECT_TRUE(scheduled_hidden.PreselectUpdated(false));

  const ChartDldrBulkSessionPolicy scheduled_visible =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, true);
  EXPECT_EQ(scheduled_visible.scheduled_ui,
            ChartDldrScheduledUiPresentation::WithProgress);
  EXPECT_TRUE(scheduled_visible.IsScheduled());
  EXPECT_TRUE(scheduled_visible.UiMaterialize());
  EXPECT_TRUE(scheduled_visible.UiSelectDownloadTab());
  EXPECT_TRUE(scheduled_visible.UiShowDownloadProgress());
  EXPECT_TRUE(scheduled_visible.SkipManualUrlCharts());

  const ChartDldrBulkSessionPolicy interactive = ChartDldrBulkSessionPolicyFor(
      ChartDldrBulkRunMode::InteractiveBulk, false);
  EXPECT_EQ(interactive.ErrorReporting(), ChartDldrErrorReporting::Dialog);
  EXPECT_EQ(interactive.mode, ChartDldrBulkRunMode::InteractiveBulk);
  EXPECT_FALSE(interactive.IsScheduled());
  EXPECT_TRUE(interactive.UiShowDownloadProgress());
  EXPECT_TRUE(interactive.AllowEmptySelection());
  EXPECT_FALSE(interactive.SkipManualUrlCharts());
  EXPECT_TRUE(interactive.confirm_before_start);
  EXPECT_FALSE(interactive.PreserveChartSelection());
  EXPECT_EQ(interactive.walk_bind, ChartDldrBulkWalkBind::AllCatalogs);
  EXPECT_TRUE(interactive.FocusChartsAfter());
  EXPECT_FALSE(interactive.manual_plan_before_start);
  EXPECT_FALSE(interactive.PreselectNew(false));
  EXPECT_TRUE(interactive.PreselectNew(true));
  EXPECT_FALSE(interactive.PreselectUpdated(false));
  EXPECT_TRUE(interactive.PreselectUpdated(true));
  EXPECT_EQ(interactive.plan.manual_policy,
            ChartDldrManualDownloadPolicy::OpenAsDiscovered);

  const ChartDldrBulkSessionPolicy selected = ChartDldrBulkSessionPolicyFor(
      ChartDldrBulkRunMode::SelectedCharts, false);
  EXPECT_FALSE(selected.AllowEmptySelection());
  EXPECT_EQ(selected.mode, ChartDldrBulkRunMode::SelectedCharts);
  EXPECT_FALSE(selected.IsScheduled());
  EXPECT_TRUE(selected.UiMaterialize());
  EXPECT_TRUE(selected.UiShowDownloadProgress());
  EXPECT_TRUE(selected.PreserveChartSelection());
  EXPECT_EQ(selected.walk_bind, ChartDldrBulkWalkBind::SingleDownload);
  EXPECT_TRUE(selected.manual_plan_before_start);
  EXPECT_FALSE(selected.FocusChartsAfter());
  EXPECT_FALSE(selected.confirm_before_start);

  const ChartDldrBulkSessionPolicy catalog_refresh =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::CatalogRefresh, true);
  EXPECT_EQ(catalog_refresh.mode, ChartDldrBulkRunMode::CatalogRefresh);
  EXPECT_FALSE(catalog_refresh.IsScheduled());
  EXPECT_TRUE(catalog_refresh.AllowEmptySelection());
  EXPECT_FALSE(catalog_refresh.confirm_before_start);
  EXPECT_EQ(catalog_refresh.walk_bind, ChartDldrBulkWalkBind::SinglePrepare);
  EXPECT_TRUE(catalog_refresh.FocusChartsAfter());
  EXPECT_TRUE(catalog_refresh.UiMaterialize());
  EXPECT_FALSE(catalog_refresh.UiSelectDownloadTab());
  EXPECT_FALSE(catalog_refresh.UiShowDownloadProgress());
  EXPECT_EQ(catalog_refresh.ErrorReporting(), ChartDldrErrorReporting::Dialog);

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
