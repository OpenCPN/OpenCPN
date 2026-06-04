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

}  // namespace

TEST(ChartDldrScheduleIntegration, SkipThenGateBlocksUntilTomorrow) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  ChartDldrApplyScheduledRunOutcome(schedule,
                                   ChartDldrScheduledRunOutcome::Skipped,
                                   "Skipped: no chart sources configured");
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

  ChartDldrApplyScheduledRunOutcome(
      schedule,
      ChartDldrScheduledOutcomeFromBulkResult(0, 4),
      ChartDldrScheduledStatusFromBulkResult(0, 4, 4));
  EXPECT_TRUE(schedule.last_run_iso.empty());
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrScheduleIntegration, SuccessfulBulkBlocksSameDay) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledOutcomeFromBulkResult(2, 3),
      ChartDldrScheduledStatusFromBulkResult(2, 3, 1));
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrScheduleIntegration, NoAttemptsDoesNotBlockRetry) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);

  ChartDldrApplyScheduledRunOutcome(
      schedule, ChartDldrScheduledOutcomeFromBulkResult(0, 0),
      ChartDldrScheduledStatusFromBulkResult(0, 0, 0));
  EXPECT_TRUE(schedule.last_run_iso.empty());
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 5, 0)));
}
