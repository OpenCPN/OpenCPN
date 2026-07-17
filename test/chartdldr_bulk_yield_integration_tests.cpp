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

TEST(ChartDldrBulkSession, OwnsImmutableCopyOfResolvedRunPlan) {
  ChartDldrBulkRunSession session;
  ChartDldrBulkRunPlan plan;
  plan.manual_policy = ChartDldrManualDownloadPolicy::OpenAsDiscovered;
  const ChartDldrBulkSessionPolicy policy = ChartDldrBulkSessionPolicyFor(
      ChartDldrBulkRunMode::InteractiveBulk, true, plan);

  session.Begin(reinterpret_cast<chartdldr_pi*>(0x1), policy, {}, 0);
  plan.manual_policy = ChartDldrManualDownloadPolicy::Skip;

  EXPECT_EQ(session.Plan().manual_policy,
            ChartDldrManualDownloadPolicy::OpenAsDiscovered);
  session.End();
  EXPECT_EQ(session.Plan().manual_policy, ChartDldrManualDownloadPolicy::Skip);
}
