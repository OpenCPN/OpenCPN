/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_transfer.h"

TEST(ChartDldrBulkTransfer, IsStuckWhenIdle) {
  ChartDldrBulkTransferSlot transfer;
  EXPECT_FALSE(transfer.IsStuck(wxDateTime::Now()));
}

TEST(ChartDldrBulkTransfer, CompletedChartBulkOwnerIsNotInProgress) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  transfer.OnEnd(false);
  EXPECT_TRUE(transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk));
  EXPECT_FALSE(transfer.IsInProgress());
  EXPECT_FALSE(transfer.IsOwnedBy(ChartDldrBulkTransferOwner::Catalog));
}

TEST(ChartDldrBulkTransfer, IsNotStuckWhenHandleUnsetShortlyAfterStart) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);

  wxDateTime now = wxDateTime::Now();
  now.Add(wxTimeSpan(0, 3, 0));
  EXPECT_FALSE(transfer.IsStuck(now));
}

TEST(ChartDldrBulkTransfer, IsStuckWhenProgressStalls) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);

  wxDateTime now = wxDateTime::Now();
  now.Add(wxTimeSpan(0, 16, 0));
  EXPECT_TRUE(transfer.IsStuck(now));
  EXPECT_EQ(transfer.StuckReason(now),
            ChartDldrBulkTransferStuckReason::ProgressStalled);
}

TEST(ChartDldrBulkTransfer, MaxDurationExceededIsDistinctReason) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::Catalog);

  wxDateTime now = wxDateTime::Now();
  now.Add(wxTimeSpan(2, 1, 0));
  EXPECT_TRUE(transfer.IsStuck(now));
  EXPECT_EQ(transfer.StuckReason(now),
            ChartDldrBulkTransferStuckReason::MaxDurationExceeded);
}

TEST(ChartDldrBulkTransfer, StuckReasonReportsWithoutAborting) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);

  wxDateTime now = wxDateTime::Now();
  now.Add(wxTimeSpan(0, 16, 0));
  EXPECT_EQ(transfer.StuckReason(now),
            ChartDldrBulkTransferStuckReason::ProgressStalled);
  EXPECT_TRUE(transfer.IsInProgress());
}

TEST(ChartDldrBulkTransfer, StuckReasonLeavesSlotActive) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::Catalog);

  wxDateTime now = wxDateTime::Now();
  now.Add(wxTimeSpan(2, 1, 0));
  EXPECT_EQ(transfer.StuckReason(now),
            ChartDldrBulkTransferStuckReason::MaxDurationExceeded);
  EXPECT_TRUE(transfer.IsInProgress());
  ChartDldrCancelAndResetBulkTransfer(transfer);
  EXPECT_FALSE(transfer.IsInProgress());
}

TEST(ChartDldrBulkTransfer, StuckReasonOnStallLeavesSlotActive) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);

  wxDateTime now = wxDateTime::Now();
  now.Add(wxTimeSpan(0, 16, 0));
  EXPECT_EQ(transfer.StuckReason(now),
            ChartDldrBulkTransferStuckReason::ProgressStalled);
  EXPECT_TRUE(transfer.IsInProgress());
  ChartDldrCancelAndResetBulkTransfer(transfer);
  EXPECT_FALSE(transfer.IsInProgress());
}

TEST(ChartDldrBulkTransfer, IsNotStuckAfterReset) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  transfer.Reset();
  EXPECT_FALSE(transfer.IsStuck(wxDateTime::Now()));
}

TEST(ChartDldrBulkTransfer, StuckTransferReactionVariesBySite) {
  ChartDldrBulkTransferSlot transfer;
  const wxDateTime t0(1, wxDateTime::Jan, 2026, 12, 0, 0);
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk, t0);
  transfer.OnProgress(100, 0, t0);
  const wxDateTime stalled_at = t0 + wxTimeSpan(0, 16, 0);

  EXPECT_EQ(
      ChartDldrReactToStuckTransfer(
          transfer, ChartDldrStuckTransferSite::OrchestratorStall, stalled_at),
      ChartDldrStuckTransferReaction::SchedulePump);
  EXPECT_EQ(ChartDldrReactToStuckTransfer(
                transfer, ChartDldrStuckTransferSite::ChartControllerPoll,
                stalled_at),
            ChartDldrStuckTransferReaction::AbortChartPass);
  EXPECT_EQ(
      ChartDldrReactToStuckTransfer(
          transfer, ChartDldrStuckTransferSite::CatalogPrepare, stalled_at),
      ChartDldrStuckTransferReaction::AbortCatalogRefresh);
}

TEST(ChartDldrBulkTransfer, CancelAndResetClearsActiveTransfer) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  transfer.handle = 0;

  ChartDldrCancelAndResetBulkTransfer(transfer);

  EXPECT_FALSE(transfer.IsInProgress());
  EXPECT_EQ(transfer.owner, ChartDldrBulkTransferOwner::None);
  EXPECT_EQ(transfer.handle, 0);
  EXPECT_EQ(transfer.live_sink, nullptr);
}

TEST(ChartDldrBulkTransfer, ByteProgressResetsStallTimer) {
  ChartDldrBulkTransferSlot transfer;
  const wxDateTime t0(1, wxDateTime::Jan, 2026, 12, 0, 0);
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk, t0);
  transfer.OnProgress(100, 25, t0);

  EXPECT_FALSE(transfer.IsStuck(t0 + wxTimeSpan(0, 10, 0)));
  EXPECT_EQ(transfer.transferred_size, 25);
}

TEST(ChartDldrBulkTransfer, NonIncreasingTransferredDoesNotResetStallTimer) {
  ChartDldrBulkTransferSlot transfer;
  const wxDateTime t0(1, wxDateTime::Jan, 2026, 12, 0, 0);
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk, t0);

  // Keepalive / duplicate progress with no byte growth must not postpone stall.
  transfer.OnProgress(100, 0, t0 + wxTimeSpan(0, 5, 0));
  transfer.OnProgress(100, 0, t0 + wxTimeSpan(0, 10, 0));
  transfer.OnProgress(1000, 50, t0 + wxTimeSpan(0, 11, 0));
  transfer.OnProgress(1000, 50, t0 + wxTimeSpan(0, 14, 0));
  transfer.OnProgress(1000, 50, t0 + wxTimeSpan(0, 20, 0));

  const wxDateTime stalled_at =
      t0 + wxTimeSpan(0, 11, 0) + wxTimeSpan(0, 16, 0);
  EXPECT_TRUE(transfer.IsStuck(stalled_at));
  EXPECT_EQ(transfer.StuckReason(stalled_at),
            ChartDldrBulkTransferStuckReason::ProgressStalled);
  EXPECT_EQ(transfer.transferred_size, 50);
}

TEST(ChartDldrBulkTransfer, TryStartResetsSlotWhenDownloadDoesNotStart) {
  ChartDldrBulkTransferSlot transfer;

  EXPECT_FALSE(ChartDldrTryStartBackgroundDownload(
      transfer, ChartDldrBulkTransferOwner::ChartBulk,
      wxT("http://example.test/chart.zip"), wxT("/tmp/chart.zip")));
  EXPECT_FALSE(transfer.IsInProgress());
  EXPECT_EQ(transfer.owner, ChartDldrBulkTransferOwner::None);
  EXPECT_EQ(transfer.handle, 0);
}

TEST(ChartDldrBulkTransfer, TryStartRejectsAnActiveSlotWithoutMutation) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::Catalog);
  const uint64_t generation = transfer.generation;

  EXPECT_FALSE(ChartDldrTryStartBackgroundDownload(
      transfer, ChartDldrBulkTransferOwner::ChartBulk,
      wxT("http://example.test/chart.zip"), wxT("/tmp/chart.zip")));
  EXPECT_TRUE(transfer.IsInProgress());
  EXPECT_EQ(transfer.owner, ChartDldrBulkTransferOwner::Catalog);
  EXPECT_EQ(transfer.generation, generation);
}

TEST(ChartDldrBulkTransfer, BeginBumpsGeneration) {
  ChartDldrBulkTransferSlot transfer;
  EXPECT_EQ(transfer.generation, 0u);
  transfer.Begin(ChartDldrBulkTransferOwner::Catalog);
  const uint64_t first = transfer.generation;
  EXPECT_GT(first, 0u);
  transfer.Reset();
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  EXPECT_GT(transfer.generation, first);
}

TEST(ChartDldrBulkTransfer, ApplyIgnoresEventsWhenOwnerNone) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  const uint64_t cancelled = transfer.generation;
  ChartDldrCancelAndResetBulkTransfer(transfer);
  EXPECT_EQ(transfer.owner, ChartDldrBulkTransferOwner::None);

  ChartDldrApplyBulkDownloadProgress(transfer, 100, 50, wxDateTime(),
                                     cancelled);
  EXPECT_EQ(transfer.transferred_size, 0);

  ChartDldrApplyBulkDownloadEnd(transfer, true, cancelled);
  EXPECT_FALSE(transfer.complete);
}

TEST(ChartDldrBulkTransfer, ApplyIgnoresMismatchedGeneration) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  const uint64_t gen = transfer.generation;

  ChartDldrApplyBulkDownloadProgress(transfer, 100, 10, wxDateTime(), gen + 1);
  EXPECT_EQ(transfer.transferred_size, 0);

  ChartDldrApplyBulkDownloadProgress(transfer, 100, 10, wxDateTime(), gen);
  EXPECT_EQ(transfer.transferred_size, 10);

  ChartDldrApplyBulkDownloadEnd(transfer, true, gen + 1);
  EXPECT_FALSE(transfer.complete);

  ChartDldrApplyBulkDownloadEnd(transfer, true, gen);
  EXPECT_TRUE(transfer.complete);
  EXPECT_TRUE(transfer.success);
}

TEST(ChartDldrBulkTransfer, CancelledGenerationDoesNotCorruptNewBegin) {
  ChartDldrBulkTransferSlot transfer;
  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  const uint64_t cancelled = transfer.generation;
  ChartDldrCancelAndResetBulkTransfer(transfer);

  transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  const uint64_t active = transfer.generation;
  EXPECT_NE(active, cancelled);

  ChartDldrApplyBulkDownloadEnd(transfer, true, cancelled);
  EXPECT_FALSE(transfer.complete);
  EXPECT_TRUE(transfer.IsInProgress());

  ChartDldrApplyBulkDownloadProgress(transfer, 100, 40, wxDateTime(),
                                     cancelled);
  EXPECT_EQ(transfer.transferred_size, 0);

  ChartDldrApplyBulkDownloadEnd(transfer, true, active);
  EXPECT_TRUE(transfer.complete);
  EXPECT_TRUE(transfer.success);
}

TEST(ChartDldrBulkTransfer, CancelParksAbandonedSinkAndIgnoresLateEnd) {
  ChartDldrBulkTransferSlot transfer;
  ChartDldrInstallLiveTransferSinkForTest(transfer);
  EXPECT_NE(transfer.live_sink, nullptr);
  EXPECT_EQ(transfer.abandoned_sink, nullptr);

  ChartDldrCancelAndResetBulkTransfer(transfer);
  EXPECT_EQ(transfer.live_sink, nullptr);
  EXPECT_NE(transfer.abandoned_sink, nullptr);
  EXPECT_FALSE(transfer.IsInProgress());

  // Abandoned sinks drop late END (slot_ nulled); no completion mutation.
  EXPECT_TRUE(ChartDldrDispatchAbandonedTransferEndForTest(transfer, true));
  EXPECT_FALSE(transfer.complete);
  EXPECT_EQ(transfer.owner, ChartDldrBulkTransferOwner::None);
}

TEST(ChartDldrBulkTransfer, TryStartClearsAbandonedSinkWithoutDrainGate) {
  ChartDldrBulkTransferSlot transfer;
  ChartDldrInstallLiveTransferSinkForTest(transfer);
  ChartDldrCancelAndResetBulkTransfer(transfer);
  EXPECT_NE(transfer.abandoned_sink, nullptr);

  // Start fails without a real OCPN download backend, but still clears the
  // abandoned sink before attempting Begin (no retired drain gate).
  EXPECT_FALSE(ChartDldrTryStartBackgroundDownload(
      transfer, ChartDldrBulkTransferOwner::ChartBulk,
      wxT("http://example.test/chart.zip"), wxT("/tmp/chart.zip")));
  EXPECT_EQ(transfer.abandoned_sink, nullptr);
  EXPECT_FALSE(transfer.IsInProgress());
}
