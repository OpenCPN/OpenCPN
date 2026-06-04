/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk_run.h"

TEST(ChartDldrBulkRunKind, ScheduledVsInteractive) {
  EXPECT_TRUE(ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind::Scheduled));
  EXPECT_FALSE(
      ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind::Interactive));
}

TEST(ChartDldrBulkRunUiPolicy, ScheduledUsesQuietHiddenPanelFlow) {
  const ChartDldrBulkRunUiPolicy visible =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, true);
  EXPECT_TRUE(visible.quiet_downloads);
  EXPECT_FALSE(visible.confirm_before_start);
  EXPECT_FALSE(visible.show_failure_summary);
  EXPECT_TRUE(visible.restore_notebook_page);
  EXPECT_TRUE(visible.select_download_tab);
  EXPECT_FALSE(visible.sync_list_selection);

  const ChartDldrBulkRunUiPolicy hidden =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, false);
  EXPECT_TRUE(hidden.quiet_downloads);
  EXPECT_FALSE(hidden.restore_notebook_page);
  EXPECT_FALSE(hidden.select_download_tab);
  EXPECT_FALSE(hidden.sync_list_selection);
}

TEST(ChartDldrBulkRunUiPolicy, InteractiveShowsConfirmAndErrors) {
  const ChartDldrBulkRunUiPolicy policy =
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Interactive, true);
  EXPECT_FALSE(policy.quiet_downloads);
  EXPECT_TRUE(policy.confirm_before_start);
  EXPECT_TRUE(policy.show_failure_summary);
  EXPECT_TRUE(policy.restore_notebook_page);
  EXPECT_TRUE(policy.select_download_tab);
  EXPECT_TRUE(policy.sync_list_selection);
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
