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

#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"

namespace {

wxDateTime LocalTime(int day, int hour, int minute) {
  wxDateTime dt;
  dt.Set(day, wxDateTime::Jun, 2026, hour, minute, 0);
  return dt;
}

wxDateTime FixedRunTime() {
  wxDateTime dt;
  dt.Set(2, wxDateTime::Jun, 2026, 4, 0, 0);
  return dt;
}

}  // namespace

TEST(ChartDldrScheduleIntegration, SkipThenGateBlocksUntilTomorrow) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledRunOutcome::Skipped,
      "no chart sources configured", &run_time);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));

  wxDateTime next_day;
  next_day.Set(3, wxDateTime::Jun, 2026, 3, 5, 0);
  EXPECT_TRUE(schedule.ShouldRunNow(next_day));
}

TEST(ChartDldrScheduleIntegration, FailedBulkAllowsSameDayRetry) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);
  schedule.last_run_iso = wxEmptyString;

  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledOutcomeFromBulkResult(0, 4),
      ChartDldrScheduledStatusFromBulkResult(0, 4, 4, 0, 0), &run_time);
  EXPECT_TRUE(schedule.last_run_iso.empty());
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrScheduleIntegration, SuccessfulBulkBlocksSameDay) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledOutcomeFromBulkResult(2, 3),
      ChartDldrScheduledStatusFromBulkResult(2, 3, 1, 1, 1), &run_time);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrScheduleIntegration, NoAttemptsBlocksSameDayRetry) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  const wxDateTime run_time = FixedRunTime();
  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledRunOutcome::BulkNoAttempts,
      "No charts to update",
      &run_time);
  EXPECT_FALSE(schedule.last_run_iso.empty());
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 5, 0)));
}
