/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_state.h"

namespace {

void BeginSession(ChartDldrBulkRunSession& session, ChartDldrBulkRunMode mode,
                  bool panel_visible) {
  const ChartDldrBulkSessionPolicy policy =
      ChartDldrBulkSessionPolicyFor(mode, panel_visible);
  session.Begin(reinterpret_cast<chartdldr_pi*>(0x1), policy, {}, 0);
}

}  // namespace

TEST(ChartDldrBulkSession, ScheduledSessionIsActiveAndScheduled) {
  ChartDldrBulkRunSession session;
  EXPECT_FALSE(session.IsActive());
  BeginSession(session, ChartDldrBulkRunMode::ScheduledBulk, false);
  EXPECT_TRUE(session.IsActive());
  EXPECT_TRUE(session.IsScheduled());
  EXPECT_EQ(session.Mode(), ChartDldrBulkRunMode::ScheduledBulk);
}

TEST(ChartDldrBulkSession, InteractiveSessionIsActiveNotScheduled) {
  ChartDldrBulkRunSession session;
  BeginSession(session, ChartDldrBulkRunMode::InteractiveBulk, true);
  EXPECT_TRUE(session.IsActive());
  EXPECT_FALSE(session.IsScheduled());
  EXPECT_EQ(session.Mode(), ChartDldrBulkRunMode::InteractiveBulk);
}

TEST(ChartDldrBulkSession, BeginArmsSessionCancelAndEndClearsManualActions) {
  ChartDldrBulkRunSession session;
  BeginSession(session, ChartDldrBulkRunMode::InteractiveBulk, true);

  // Begin owns the cancel state: the run starts session-cancellable.
  EXPECT_EQ(session.DownloadCancel().phase,
            ChartDldrDownloadCancelPhase::SessionCancellable);
  EXPECT_FALSE(session.DownloadCancel().IsSessionCancelled());

  session.ManualActions().push_back(ChartDldrManualDownloadAction{
      wxT("manual"), wxT("https://example.test/manual"), wxT("/charts")});
  EXPECT_EQ(session.ManualActions().size(), 1u);

  session.End();
  EXPECT_TRUE(session.ManualActions().empty());
  EXPECT_EQ(session.DownloadCancel().phase, ChartDldrDownloadCancelPhase::Idle);
}
