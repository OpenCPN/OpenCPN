/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_transfer.h"

TEST(ChartDldrScheduledStall, KeepaliveProgressDoesNotResetStallClock) {
  ChartDldrBulkTransferSlot slot;
  const wxDateTime t0(8, wxDateTime::Jul, 2026, 21, 0, 0);
  slot.Begin(ChartDldrBulkTransferOwner::ChartBulk, t0);

  // Same path as bulk transfer sink PROGRESS.
  ChartDldrApplyBulkDownloadProgress(slot, 1000, 0, t0 + wxTimeSpan(0, 1, 0));
  ChartDldrApplyBulkDownloadProgress(slot, 1000, 0, t0 + wxTimeSpan(0, 5, 0));
  ChartDldrApplyBulkDownloadProgress(slot, 1000, 0, t0 + wxTimeSpan(0, 10, 0));
  ChartDldrApplyBulkDownloadProgress(slot, 1000, 0, t0 + wxTimeSpan(0, 14, 0));

  EXPECT_TRUE(slot.IsStuck(t0 + wxTimeSpan(0, 16, 0)));
  EXPECT_EQ(slot.StuckReason(t0 + wxTimeSpan(0, 16, 0)),
            ChartDldrBulkTransferStuckReason::ProgressStalled);
}

TEST(ChartDldrScheduledStall, StallLogThenCancelAfterKeepaliveOnlyProgress) {
  ChartDldrBulkTransferSlot slot;
  const wxDateTime t0(8, wxDateTime::Jul, 2026, 21, 0, 0);
  slot.Begin(ChartDldrBulkTransferOwner::ChartBulk, t0);

  for (int minute = 1; minute <= 14; ++minute) {
    ChartDldrApplyBulkDownloadProgress(slot, 2000, 0,
                                       t0 + wxTimeSpan(0, minute, 0));
  }

  EXPECT_EQ(ChartDldrLogTransferIfStuck(slot, t0 + wxTimeSpan(0, 16, 0)),
            ChartDldrBulkTransferStuckReason::ProgressStalled);
  ChartDldrCancelAndResetBulkTransfer(slot);
  EXPECT_FALSE(slot.IsInProgress());
}

TEST(ChartDldrScheduledStall, ByteProgressThroughDispatchResetsStall) {
  ChartDldrBulkTransferSlot slot;
  const wxDateTime t0(8, wxDateTime::Jul, 2026, 21, 0, 0);
  slot.Begin(ChartDldrBulkTransferOwner::ChartBulk, t0);

  ChartDldrApplyBulkDownloadProgress(slot, 2000, 0, t0 + wxTimeSpan(0, 10, 0));
  ChartDldrApplyBulkDownloadProgress(slot, 2000, 50, t0 + wxTimeSpan(0, 11, 0));

  EXPECT_FALSE(slot.IsStuck(t0 + wxTimeSpan(0, 20, 0)));
  EXPECT_EQ(slot.transferred_size, 50);
}

TEST(ChartDldrScheduledStall, CatalogStallAdvancesWalker) {
  ChartDldrBulkTransferSlot transfer;
  const wxDateTime t0(8, wxDateTime::Jul, 2026, 21, 0, 0);
  transfer.Begin(ChartDldrBulkTransferOwner::Catalog, t0);

  ChartDldrApplyBulkDownloadProgress(transfer, 1000, 0,
                                     t0 + wxTimeSpan(0, 5, 0));
  ChartDldrApplyBulkDownloadProgress(transfer, 1000, 0,
                                     t0 + wxTimeSpan(0, 10, 0));

  const wxDateTime stalled = t0 + wxTimeSpan(0, 16, 0);
  EXPECT_EQ(ChartDldrLogTransferIfStuck(transfer, stalled),
            ChartDldrBulkTransferStuckReason::ProgressStalled);
  ChartDldrCancelAndResetBulkTransfer(transfer);
  EXPECT_FALSE(transfer.IsInProgress());

  ChartDldrBulkCatalogRunState state;
  state.catalog_bound = 3;
  ChartDldrBulkRunStats fail;
  fail.catalog_refresh_failures = 1;
  EXPECT_TRUE(
      ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance, 0, fail));
  EXPECT_EQ(state.next_catalog, 1);
  EXPECT_EQ(state.stats.catalog_refresh_failures, 1);
}

TEST(ChartDldrScheduledStall, ChartTransferStallFailsForwardToNextChart) {
  ChartDldrBulkTransferSlot transfer;
  const wxDateTime t0(8, wxDateTime::Jul, 2026, 21, 0, 0);
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk, t0);
  ChartDldrApplyBulkDownloadProgress(transfer, 5000, 10, t0);
  ChartDldrApplyBulkDownloadProgress(transfer, 5000, 10,
                                     t0 + wxTimeSpan(0, 14, 0));

  EXPECT_EQ(ChartDldrLogTransferIfStuck(transfer, t0 + wxTimeSpan(0, 16, 0)),
            ChartDldrBulkTransferStuckReason::ProgressStalled);
  ChartDldrCancelAndResetBulkTransfer(transfer);
  EXPECT_FALSE(transfer.IsInProgress());

  ChartDldrBulkCatalogRunState state;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  state.catalog_bound = 2;
  EXPECT_TRUE(ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::MoreWork));
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::DownloadChart);
  EXPECT_EQ(state.next_catalog, 0);
}

TEST(ChartDldrScheduledStall, StallDoesNotBlockCatalogAdvance) {
  ChartDldrBulkCatalogRunState state;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  state.next_catalog = 0;
  state.catalog_bound = 3;

  ChartDldrBulkRunStats counters;
  counters.attempted = 1;
  counters.failed = 1;

  EXPECT_TRUE(ChartDldrAdvanceBulkWalk(state, ChartDldrBulkWalkStep::Advance, 0,
                                       counters));
  EXPECT_EQ(state.next_catalog, 1);
  EXPECT_EQ(state.phase, ChartDldrBulkCatalogPhase::PrepareCatalog);
  EXPECT_EQ(state.stats.failed, 1);
}
