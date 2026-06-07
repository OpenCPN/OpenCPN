/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"

TEST(ChartDldrBulkRequest, CanStartUsesInputSnapshot) {
  ChartDldrBulkRequestInput active_run;
  active_run.allow_bulk_update = true;
  active_run.has_chart_sources = true;
  active_run.bulk_run_active = true;
  EXPECT_FALSE(ChartDldrCanStartBulkRequest(
      active_run, ChartDldrBulkRunKind::Scheduled, nullptr));

  ChartDldrBulkRequestInput ready;
  ready.allow_bulk_update = true;
  ready.has_chart_sources = true;
  ChartDldrScheduledSkipReason skip =
      ChartDldrScheduledSkipReason::BulkDisabled;
  EXPECT_TRUE(ChartDldrCanStartBulkRequest(
      ready, ChartDldrBulkRunKind::Scheduled, &skip));
  EXPECT_EQ(skip, ChartDldrScheduledSkipReason::None);

  ChartDldrBulkRequestInput interactive_blocked;
  interactive_blocked.allow_bulk_update = false;
  interactive_blocked.has_chart_sources = true;
  EXPECT_FALSE(ChartDldrCanStartBulkRequest(
      interactive_blocked, ChartDldrBulkRunKind::Interactive, nullptr));
}

TEST(ChartDldrBulkRequest, EvaluateSkipReasonPriority) {
  EXPECT_EQ(ChartDldrEvaluateScheduledBulkSkip(false, false),
            ChartDldrScheduledSkipReason::BulkDisabled);
  EXPECT_EQ(ChartDldrEvaluateScheduledBulkSkip(false, true),
            ChartDldrScheduledSkipReason::BulkDisabled);
  EXPECT_EQ(ChartDldrEvaluateScheduledBulkSkip(true, false),
            ChartDldrScheduledSkipReason::NoSources);
  EXPECT_EQ(ChartDldrEvaluateScheduledBulkSkip(true, true),
            ChartDldrScheduledSkipReason::None);
}

TEST(ChartDldrBulkRequest, SkipStatusAdvancesLastRun) {
  ChartDldrScheduleConfig schedule;
  wxDateTime run_time;
  run_time.Set(2, wxDateTime::Jun, 2026, 12, 0, 0);
  const wxString reason =
      ChartDldrScheduledSkipStatus(ChartDldrScheduledSkipReason::NoSources);
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledRunOutcome::Skipped, reason, &run_time);
  EXPECT_FALSE(schedule.last_run_iso.empty());
  EXPECT_TRUE(schedule.last_status.EndsWith(reason));
  EXPECT_FALSE(reason.empty());
}

TEST(ChartDldrBulkRequest, EachSkipReasonHasStatusText) {
  for (auto reason : {ChartDldrScheduledSkipReason::BulkDisabled,
                      ChartDldrScheduledSkipReason::NoSources,
                      ChartDldrScheduledSkipReason::PanelUnavailable}) {
    EXPECT_FALSE(ChartDldrScheduledSkipStatus(reason).empty());
  }
  EXPECT_TRUE(
      ChartDldrScheduledSkipStatus(ChartDldrScheduledSkipReason::None).empty());
}
