/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_state.h"

namespace {

void BeginSession(ChartDldrBulkRunSession& session, ChartDldrBulkRunKind kind) {
  const ChartDldrBulkRunUiPolicy policy =
      ChartDldrBulkRunUiPolicyFor(kind, false);
  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(policy);
  session.Begin(reinterpret_cast<chartdldr_pi*>(0x1), kind, profile, {});
}

}  // namespace

TEST(ChartDldrBulkYieldIntegration, OnDownloadEventYieldsWhenNoBulkSession) {
  ChartDldrBulkRunSession session;
  EXPECT_FALSE(session.IsActive());
  EXPECT_TRUE(session.WouldYieldOnDownloadEvent());
}

TEST(ChartDldrBulkYieldIntegration, OnDownloadEventBlocksYieldDuringScheduledPollOnly) {
  ChartDldrBulkRunSession session;
  BeginSession(session, ChartDldrBulkRunKind::Scheduled);
  EXPECT_TRUE(session.IsActive());
  EXPECT_EQ(session.Profile().charts.transfer_poll, ChartDldrTransferPoll::PollOnly);
  EXPECT_FALSE(session.WouldYieldOnDownloadEvent());
}

TEST(ChartDldrBulkYieldIntegration, OnDownloadEventYieldsDuringInteractiveBlockingRun) {
  ChartDldrBulkRunSession session;
  BeginSession(session, ChartDldrBulkRunKind::Interactive);
  EXPECT_TRUE(session.IsActive());
  EXPECT_EQ(session.Profile().charts.transfer_poll,
            ChartDldrTransferPoll::BlockUntilComplete);
  EXPECT_TRUE(session.WouldYieldOnDownloadEvent());
}
