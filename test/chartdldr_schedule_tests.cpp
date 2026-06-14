/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <wx/datetime.h>

#include <gtest/gtest.h>

#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"

namespace {

wxDateTime LocalTime(int day, int hour, int minute) {
  wxDateTime dt;
  dt.Set(day, wxDateTime::Jun, 2026, hour, minute, 0);
  return dt;
}

ChartDldrScheduleConfig MakeSchedule(bool enabled, int hour, int minute) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = enabled;
  schedule.SetTime(hour, minute);
  return schedule;
}

}  // namespace

TEST(ChartDldrSchedule, DisabledNeverRuns) {
  ChartDldrScheduleConfig schedule = MakeSchedule(false, 3, 0);
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 12, 0)),
            ChartDldrScheduleEligibility::Disabled);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 12, 0)));
}

TEST(ChartDldrSchedule, BeforeScheduledTime) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 2, 30)));
}

TEST(ChartDldrSchedule, AtScheduledMinuteWhenNeverRun) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 3, 0)));
}

TEST(ChartDldrSchedule, AfterScheduledTimeWhenNeverRun) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 4, 15)));
}

TEST(ChartDldrSchedule, SkipsWhenLastRunAtOrAfterTodaysSlot) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = "2026-06-02 03:30:00";
  schedule.last_run_iso = schedule.last_attempt_iso;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
  schedule.last_attempt_iso = "2026-06-02 03:00:00";
  schedule.last_run_iso = schedule.last_attempt_iso;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 3, 30)));
}

TEST(ChartDldrSchedule, RunsNextDayAfterPriorRun) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = "2026-06-02 03:30:00";
  schedule.last_run_iso = schedule.last_attempt_iso;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  wxDateTime next_day;
  next_day.Set(3, wxDateTime::Jun, 2026, 3, 5, 0);
  EXPECT_TRUE(schedule.ShouldRunNow(next_day));
}

TEST(ChartDldrSchedule, LegacyDateOnlyLastRun) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = "2026-06-02";
  schedule.last_run_iso = schedule.last_attempt_iso;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
  wxDateTime next_day;
  next_day.Set(3, wxDateTime::Jun, 2026, 3, 5, 0);
  EXPECT_TRUE(schedule.ShouldRunNow(next_day));
}

TEST(ChartDldrSchedule, OneMinuteBeforeScheduledMinute) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 2, 59)));
}

TEST(ChartDldrSchedule, ClampsHourAndMinute) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 99, 99);
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 23, 59)));
}

TEST(ChartDldrSchedule, InvalidLastRunIsoTreatedAsNeverRun) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_run_iso = "not-a-valid-timestamp";
  schedule.last_attempt_iso = schedule.last_run_iso;
  schedule.SanitizeTimestamps();
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrSchedule, DoesNotRunAgainAfterLastRunMarkedNow) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  const wxString now_iso = wxDateTime::Now().FormatISOCombined(' ');
  schedule.last_attempt_iso = now_iso;
  schedule.last_run_iso = now_iso;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrSchedule, BlocksImmediateRetryAfterFailedAttemptToday) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = "2026-06-02 04:00:00";
  schedule.last_run_iso = schedule.last_attempt_iso;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkAllFailed;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 30)));
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 7, 59)));
}

TEST(ChartDldrSchedule, AllowsRetryAfterMinIntervalWhenAllFailed) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = "2026-06-02 04:00:00";
  schedule.last_run_iso = schedule.last_attempt_iso;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkAllFailed;
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 8, 0)));
}

TEST(ChartDldrSchedule, MinRetryIntervalIsFourHours) {
  EXPECT_EQ(ChartDldrScheduleConfig::MinRetryMinutes(), 240);
}
