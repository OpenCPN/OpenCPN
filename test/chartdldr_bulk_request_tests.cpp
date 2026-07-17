/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <wx/datetime.h>

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"

namespace {

ChartDldrScheduleConfig ReadyTimerSchedule() {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);
  return schedule;
}

wxDateTime JuneMorning() {
  wxDateTime now;
  now.Set(2, wxDateTime::Jun, 2026, 4, 0, 0);
  return now;
}

ChartDldrBulkRequestInput ReadyBulkInput() {
  ChartDldrBulkRequestInput input;
  input.allow_bulk_update = true;
  input.has_chart_sources = true;
  return input;
}

}  // namespace

TEST(ChartDldrBulkRequest, EvaluateUsesInputSnapshot) {
  const wxDateTime now = JuneMorning();
  ChartDldrScheduleConfig schedule;

  ChartDldrBulkRequestInput active_run = ReadyBulkInput();
  active_run.blocker = ChartDldrBulkStartBlocker::SessionActive;
  const ChartDldrBulkRequestEligibility active = ChartDldrEvaluateBulkRequest(
      active_run, ChartDldrBulkRunMode::ScheduledBulk,
      ChartDldrBulkRequestOrigin::Timer, schedule, now);
  EXPECT_FALSE(active.IsAllowed());
  EXPECT_FALSE(active.IsStaleTimer());
  EXPECT_EQ(active.status, ChartDldrBulkRequestStatus::Denied);
  EXPECT_EQ(active.skip, ChartDldrScheduledSkipReason::None);

  const ChartDldrBulkRequestEligibility allowed = ChartDldrEvaluateBulkRequest(
      ReadyBulkInput(), ChartDldrBulkRunMode::ScheduledBulk,
      ChartDldrBulkRequestOrigin::Timer, ReadyTimerSchedule(), now);
  EXPECT_TRUE(allowed.IsAllowed());
  EXPECT_EQ(allowed.skip, ChartDldrScheduledSkipReason::None);

  ChartDldrBulkRequestInput interactive_blocked = ReadyBulkInput();
  interactive_blocked.allow_bulk_update = false;
  const ChartDldrBulkRequestEligibility interactive =
      ChartDldrEvaluateBulkRequest(
          interactive_blocked, ChartDldrBulkRunMode::InteractiveBulk,
          ChartDldrBulkRequestOrigin::Manual, schedule, now);
  EXPECT_FALSE(interactive.IsAllowed());
  EXPECT_EQ(interactive.status, ChartDldrBulkRequestStatus::Denied);
}

TEST(ChartDldrBulkRequest, ManualSessionActiveDeniesWithoutSkipReason) {
  // Busy is ChartDldrBulkStartBlocker::SessionActive — Manual feedback is
  // reported from ChartDldrRequestBulkUpdate, not via ScheduledSkipReason.
  ChartDldrBulkRequestInput active_run = ReadyBulkInput();
  active_run.blocker = ChartDldrBulkStartBlocker::SessionActive;
  const ChartDldrBulkRequestEligibility eligibility =
      ChartDldrEvaluateBulkRequest(active_run,
                                   ChartDldrBulkRunMode::ScheduledBulk,
                                   ChartDldrBulkRequestOrigin::Manual,
                                   ReadyTimerSchedule(), JuneMorning());
  EXPECT_FALSE(eligibility.IsAllowed());
  EXPECT_EQ(eligibility.status, ChartDldrBulkRequestStatus::Denied);
  EXPECT_EQ(eligibility.skip, ChartDldrScheduledSkipReason::None);
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
  EXPECT_FALSE(schedule.last_attempt_iso.empty());
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

TEST(ChartDldrBulkRequest, TimerOriginRevalidatedAfterSameDayRun) {
  ChartDldrScheduleConfig schedule = ReadyTimerSchedule();

  wxDateTime run_time = JuneMorning();
  ChartDldrBulkRunStats success_stats;
  success_stats.attempted = 2;
  success_stats.updated_downloads = 2;
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledBulkResultFromStats(success_stats),
      &run_time);

  wxDateTime later;
  later.Set(2, wxDateTime::Jun, 2026, 5, 0, 0);
  const ChartDldrBulkRequestEligibility eligibility =
      ChartDldrEvaluateBulkRequest(
          ReadyBulkInput(), ChartDldrBulkRunMode::ScheduledBulk,
          ChartDldrBulkRequestOrigin::Timer, schedule, later);
  EXPECT_TRUE(eligibility.IsStaleTimer());
  EXPECT_FALSE(eligibility.IsAllowed());
  EXPECT_EQ(eligibility.skip, ChartDldrScheduledSkipReason::None);
}

TEST(ChartDldrBulkRequest, ManualOriginBypassesTimerScheduleGate) {
  ChartDldrScheduleConfig schedule = ReadyTimerSchedule();

  wxDateTime run_time = JuneMorning();
  ChartDldrBulkRunStats success_stats;
  success_stats.attempted = 2;
  success_stats.updated_downloads = 2;
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledBulkResultFromStats(success_stats),
      &run_time);

  wxDateTime later;
  later.Set(2, wxDateTime::Jun, 2026, 5, 0, 0);
  const ChartDldrBulkRequestEligibility eligibility =
      ChartDldrEvaluateBulkRequest(
          ReadyBulkInput(), ChartDldrBulkRunMode::ScheduledBulk,
          ChartDldrBulkRequestOrigin::Manual, schedule, later);
  EXPECT_TRUE(eligibility.IsAllowed());
  EXPECT_EQ(eligibility.skip, ChartDldrScheduledSkipReason::None);
}

TEST(ChartDldrBulkRequest, TimerOriginRequiresEnabledSchedule) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = false;
  schedule.SetTime(22, 0);

  wxDateTime now;
  now.Set(2, wxDateTime::Jun, 2026, 23, 0, 0);
  const ChartDldrBulkRequestEligibility eligibility =
      ChartDldrEvaluateBulkRequest(
          ReadyBulkInput(), ChartDldrBulkRunMode::ScheduledBulk,
          ChartDldrBulkRequestOrigin::Timer, schedule, now);
  EXPECT_TRUE(eligibility.IsStaleTimer());
  EXPECT_FALSE(eligibility.IsAllowed());
}
