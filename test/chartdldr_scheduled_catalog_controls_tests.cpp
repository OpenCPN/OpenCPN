/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"

TEST(ChartDldrCatalogControls, ActiveBulkRunLocksListAndMutations) {
  const auto locked = ChartDldrCatalogControlsFor(/*run_active=*/true,
                                                  /*cancel_armed=*/false);
  EXPECT_FALSE(locked.list_enabled);
  EXPECT_FALSE(locked.mutations_enabled);
  EXPECT_FALSE(locked.download_button_enabled);

  const auto cancel_armed = ChartDldrCatalogControlsFor(/*run_active=*/true,
                                                        /*cancel_armed=*/true);
  EXPECT_FALSE(cancel_armed.list_enabled);
  EXPECT_FALSE(cancel_armed.mutations_enabled);
  EXPECT_TRUE(cancel_armed.download_button_enabled);
}

TEST(ChartDldrCatalogControls, IdlePanelEnablesMutationsAndDownload) {
  const auto idle = ChartDldrCatalogControlsFor(/*run_active=*/false,
                                                /*cancel_armed=*/false);
  EXPECT_TRUE(idle.list_enabled);
  EXPECT_TRUE(idle.mutations_enabled);
  EXPECT_TRUE(idle.download_button_enabled);
}

TEST(ChartDldrCatalogControls, InteractiveAndScheduledPoliciesStillDistinct) {
  const auto silent =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, false);
  EXPECT_TRUE(silent.IsScheduled());
  EXPECT_FALSE(silent.UiMaterialize());

  const auto visible =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, true);
  EXPECT_TRUE(visible.IsScheduled());
  EXPECT_TRUE(visible.UiMaterialize());

  EXPECT_FALSE(
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::InteractiveBulk, true)
          .IsScheduled());
}
