/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk_state.h"
#include "chartdldr_bulk_transfer.h"

TEST(ChartDldrCatalogRefreshPayload, ResetClearsFields) {
  ChartDldrCatalogRefreshPayload refresh;
  refresh.index = 3;
  refresh.download_paths.temp_path = wxT("/tmp/catalog.tmp");
  refresh.download_paths.output_path = wxT("/data/catalog.xml");
  refresh.download_paths.download_target = wxT("file:///tmp/catalog.tmp");

  refresh = ChartDldrCatalogRefreshPayload();

  EXPECT_EQ(refresh.index, -1);
  EXPECT_TRUE(refresh.download_paths.temp_path.empty());
  EXPECT_TRUE(refresh.download_paths.output_path.empty());
  EXPECT_TRUE(refresh.download_paths.download_target.empty());
}

TEST(ChartDldrBulkRunSession, TransferSlotStartsUnowned) {
  ChartDldrBulkRunSession session;
  EXPECT_FALSE(session.Transfer().IsInProgress());
  EXPECT_EQ(session.Transfer().owner, ChartDldrBulkTransferOwner::None);
  EXPECT_FALSE(session.CatalogRefreshInProgress());
}

TEST(ChartDldrBulkRunSession, CatalogRefreshInProgressDerivesFromOwner) {
  ChartDldrBulkRunSession session;
  EXPECT_FALSE(session.CatalogRefreshInProgress());

  // A Catalog-owned transfer is the sole signal that a refresh is live.
  session.Transfer().Begin(ChartDldrBulkTransferOwner::Catalog);
  EXPECT_TRUE(session.CatalogRefreshInProgress());

  // A chart-bulk transfer must never read as a catalog refresh.
  ChartDldrCancelAndResetBulkTransfer(session.Transfer());
  session.Transfer().Begin(ChartDldrBulkTransferOwner::ChartBulk);
  EXPECT_FALSE(session.CatalogRefreshInProgress());

  ChartDldrCancelAndResetBulkTransfer(session.Transfer());
  EXPECT_FALSE(session.CatalogRefreshInProgress());
}

TEST(ChartDldrBulkRunSession, NeedsEventDrainOnlyWhileInProgress) {
  ChartDldrBulkRunSession session;
  EXPECT_FALSE(ChartDldrTransferNeedsEventDrain(session.Transfer()));

  session.Transfer().Begin(ChartDldrBulkTransferOwner::Catalog);
  EXPECT_TRUE(ChartDldrTransferNeedsEventDrain(session.Transfer()));
  EXPECT_TRUE(session.Transfer().IsInProgress());

  ChartDldrCancelAndResetBulkTransfer(session.Transfer());
  EXPECT_FALSE(session.Transfer().IsInProgress());
  EXPECT_FALSE(ChartDldrTransferNeedsEventDrain(session.Transfer()));
}
