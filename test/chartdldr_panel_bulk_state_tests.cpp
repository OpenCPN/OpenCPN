/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_panel_bulk_state.h"
#include "chartdldr_bulk_transfer.h"

TEST(ChartDldrPanelBulkState, CatalogRefreshResetClearsFields) {
  ChartDldrPanelBulkState state;
  state.catalog_refresh.active = true;
  state.catalog_refresh.index = 3;
  state.catalog_refresh.download_paths.temp_path = wxT("/tmp/catalog.tmp");
  state.catalog_refresh.download_paths.output_path = wxT("/data/catalog.xml");
  state.catalog_refresh.download_paths.download_target =
      wxT("file:///tmp/catalog.tmp");

  state.catalog_refresh = ChartDldrCatalogRefreshState();

  EXPECT_FALSE(state.catalog_refresh.active);
  EXPECT_EQ(state.catalog_refresh.index, -1);
  EXPECT_TRUE(state.catalog_refresh.download_paths.temp_path.empty());
  EXPECT_TRUE(state.catalog_refresh.download_paths.output_path.empty());
  EXPECT_TRUE(state.catalog_refresh.download_paths.download_target.empty());
}

TEST(ChartDldrPanelBulkState, TransferSlotStartsUnowned) {
  ChartDldrPanelBulkState state;
  EXPECT_FALSE(state.transfer.IsInProgress());
  EXPECT_EQ(state.transfer.owner, ChartDldrBulkTransferOwner::None);
  EXPECT_FALSE(state.catalog_refresh.active);
}

TEST(ChartDldrPanelBulkState, NeedsEventDrainOnlyWhileInProgress) {
  ChartDldrPanelBulkState state;
  EXPECT_FALSE(ChartDldrTransferNeedsEventDrain(state.transfer));

  state.transfer.Begin(ChartDldrBulkTransferOwner::Catalog);
  EXPECT_TRUE(ChartDldrTransferNeedsEventDrain(state.transfer));
  EXPECT_TRUE(state.transfer.IsInProgress());

  ChartDldrCancelAndResetBulkTransfer(state.transfer);
  EXPECT_FALSE(state.transfer.IsInProgress());
  EXPECT_FALSE(ChartDldrTransferNeedsEventDrain(state.transfer));
}
