/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_schedule_state.h"

TEST(ChartDldrBulkTeardown, ReasonAndCancelScopePolicies) {
  EXPECT_EQ(ChartDldrBulkTeardownReasonAfterLoop(false),
            ChartDldrBulkTeardownReason::Completed);
  EXPECT_EQ(ChartDldrBulkTeardownReasonAfterLoop(true),
            ChartDldrBulkTeardownReason::UserCancelled);
  EXPECT_TRUE(ChartDldrShouldPreserveScheduledRun(
      ChartDldrBulkCancelScope::OptionsClosed, true));
  EXPECT_FALSE(ChartDldrShouldPreserveScheduledRun(
      ChartDldrBulkCancelScope::PluginShutdown, true));
  EXPECT_EQ(ChartDldrTeardownReasonForCancelScope(
                ChartDldrBulkCancelScope::PluginShutdown),
            ChartDldrBulkTeardownReason::Shutdown);
}

TEST(ChartDldrBulkTeardown, CompletedSessionEndIsDerivedFromRealStats) {
  ChartDldrBulkRunStats stats;
  stats.attempted = 3;
  stats.failed = 1;
  const ChartDldrBulkSessionEnd end = ChartDldrBulkSessionEndFor(
      ChartDldrBulkTeardownReason::Completed, true, stats);

  EXPECT_FALSE(end.ShouldCancelGlobal());
  EXPECT_TRUE(end.ShouldCopyStats());
  EXPECT_TRUE(end.ShouldFinalizeUi());
  EXPECT_TRUE(end.ShouldApplyChartDb());
  EXPECT_EQ(end.Scheduled(),
            ChartDldrBulkSessionEnd::ScheduledFinish::Complete);
}

TEST(ChartDldrBulkTeardown, CancelNeverAppliesPartiallyDownloadedCharts) {
  ChartDldrBulkRunStats stats;
  stats.attempted = 5;
  const ChartDldrBulkSessionEnd end = ChartDldrBulkSessionEndFor(
      ChartDldrBulkTeardownReason::UserCancelled, true, stats);

  EXPECT_TRUE(end.ShouldCancelGlobal());
  EXPECT_FALSE(end.ShouldCopyStats());
  EXPECT_FALSE(end.ShouldFinalizeUi());
  EXPECT_FALSE(end.ShouldApplyChartDb());
  EXPECT_EQ(end.Scheduled(), ChartDldrBulkSessionEnd::ScheduledFinish::Abort);
}

TEST(ChartDldrBulkTeardown, FailedStartNeverCancelsGlobalDownloads) {
  ChartDldrBulkRunStats stats;
  const ChartDldrBulkSessionEnd end = ChartDldrBulkSessionEndFor(
      ChartDldrBulkTeardownReason::FailedStart, false, stats);

  EXPECT_FALSE(end.ShouldCancelGlobal());
  EXPECT_FALSE(end.ShouldFinalizeUi());
  EXPECT_FALSE(end.ShouldApplyChartDb());
  EXPECT_EQ(end.Scheduled(), ChartDldrBulkSessionEnd::ScheduledFinish::None);
}

TEST(ChartDldrBulkTeardown, ShutdownCancelsGlobalAndAbortsScheduled) {
  ChartDldrBulkRunStats stats;
  const ChartDldrBulkSessionEnd end = ChartDldrBulkSessionEndFor(
      ChartDldrBulkTeardownReason::Shutdown, true, stats);

  EXPECT_TRUE(end.ShouldCancelGlobal());
  EXPECT_FALSE(end.ShouldFinalizeUi());
  EXPECT_EQ(end.Scheduled(), ChartDldrBulkSessionEnd::ScheduledFinish::Abort);
}

TEST(ChartDldrBulkTeardown, CompletedWithZeroOkSkipsChartDb) {
  ChartDldrBulkRunStats stats;
  stats.attempted = 3;
  stats.failed = 3;
  const ChartDldrBulkSessionEnd end = ChartDldrBulkSessionEndFor(
      ChartDldrBulkTeardownReason::Completed, true, stats);

  EXPECT_TRUE(end.ShouldCopyStats());
  EXPECT_TRUE(end.ShouldFinalizeUi());
  EXPECT_FALSE(end.ShouldApplyChartDb());
  EXPECT_EQ(end.Scheduled(),
            ChartDldrBulkSessionEnd::ScheduledFinish::Complete);
}

TEST(ChartDldrBulkTeardown, ShutdownPostflightIsSuppressed) {
  ChartDldrBulkRunStats stats;
  stats.failed = 1;
  const ChartDldrBulkPostflight postflight = ChartDldrBulkPostflightFor(
      ChartDldrBulkTeardownReason::Shutdown,
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::InteractiveBulk,
                                    true),
      stats);
  EXPECT_EQ(postflight.kind, ChartDldrBulkPostflightKind::None);
  EXPECT_TRUE(postflight.message.empty());
}

TEST(ChartDldrBulkTeardown, InteractiveTeardownProducesOneTypedPostflight) {
  ChartDldrBulkRunStats stats;
  stats.attempted = 2;
  stats.failed = 1;
  const ChartDldrBulkSessionPolicy policy =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::SelectedCharts, true);

  const ChartDldrBulkPostflight completed = ChartDldrBulkPostflightFor(
      ChartDldrBulkTeardownReason::Completed, policy, stats);
  EXPECT_EQ(completed.kind, ChartDldrBulkPostflightKind::Summary);
  EXPECT_FALSE(completed.message.empty());

  const ChartDldrBulkPostflight cancelled = ChartDldrBulkPostflightFor(
      ChartDldrBulkTeardownReason::UserCancelled, policy, stats);
  EXPECT_EQ(cancelled.kind, ChartDldrBulkPostflightKind::Cancelled);
  EXPECT_FALSE(cancelled.message.empty());
}

TEST(ChartDldrBulkTeardown, ScheduledCompletedPostflightIsSuppressed) {
  ChartDldrBulkRunStats stats;
  stats.attempted = 2;
  stats.updated_downloads = 2;
  const ChartDldrBulkPostflight postflight = ChartDldrBulkPostflightFor(
      ChartDldrBulkTeardownReason::Completed,
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::ScheduledBulk, false),
      stats);
  EXPECT_EQ(postflight.kind, ChartDldrBulkPostflightKind::None);
  EXPECT_TRUE(postflight.message.empty());
}

TEST(ChartDldrBulkTeardown, ReportBulkErrorIsSilentWhileModalsSuppressed) {
  const ChartDldrBulkSessionPolicy dialog_policy =
      ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::InteractiveBulk,
                                    true);
  EXPECT_FALSE(ChartDldrBulkModalsSuppressed());
  ChartDldrEnterBulkModalSuppress();
  EXPECT_TRUE(ChartDldrBulkModalsSuppressed());
  // Must not open a modal under the pump (would re-enter DeInit / UAF).
  ChartDldrReportBulkError(nullptr, dialog_policy, wxT("pump failure"),
                           wxT("Error"));
  ChartDldrLeaveBulkModalSuppress();
  EXPECT_FALSE(ChartDldrBulkModalsSuppressed());
}

TEST(ChartDldrScheduleState, AbortedAllowsSameDayRetry) {
  EXPECT_TRUE(ChartDldrScheduledOutcomeAllowsSameDayRetry(
      ChartDldrScheduledRunOutcome::Aborted));
  const ChartDldrScheduledBulkResult aborted =
      ChartDldrScheduledAbortedResult();
  EXPECT_EQ(aborted.outcome, ChartDldrScheduledRunOutcome::Aborted);
  EXPECT_FALSE(aborted.status_detail.empty());
}
