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

#include "chartdldr_schedule.h"

namespace {

wxDateTime LocalTime(int day, int hour, int minute) {
  wxDateTime dt;
  dt.Set(day, wxDateTime::Jun, 2026, hour, minute, 0);
  return dt;
}

}  // namespace

TEST(ChartDldrSchedule, DisabledNeverRuns) {
  EXPECT_FALSE(
      ChartDldrScheduleShouldRun(LocalTime(2, 12, 0), false, 3, 0, ""));
}

TEST(ChartDldrSchedule, BeforeScheduledTime) {
  EXPECT_FALSE(ChartDldrScheduleShouldRun(LocalTime(2, 2, 30), true, 3, 0, ""));
}

TEST(ChartDldrSchedule, AtScheduledMinuteWhenNeverRun) {
  EXPECT_TRUE(ChartDldrScheduleShouldRun(LocalTime(2, 3, 0), true, 3, 0, ""));
}

TEST(ChartDldrSchedule, AfterScheduledTimeWhenNeverRun) {
  EXPECT_TRUE(ChartDldrScheduleShouldRun(LocalTime(2, 4, 15), true, 3, 0, ""));
}

TEST(ChartDldrSchedule, SkipsWhenLastRunAtOrAfterTodaysSlot) {
  EXPECT_FALSE(ChartDldrScheduleShouldRun(LocalTime(2, 4, 0), true, 3, 0,
                                          "2026-06-02 03:30:00"));
  EXPECT_FALSE(ChartDldrScheduleShouldRun(LocalTime(2, 3, 30), true, 3, 0,
                                          "2026-06-02 03:00:00"));
}

TEST(ChartDldrSchedule, RunsNextDayAfterPriorRun) {
  wxDateTime next_day;
  next_day.Set(3, wxDateTime::Jun, 2026, 3, 5, 0);
  EXPECT_TRUE(ChartDldrScheduleShouldRun(next_day, true, 3, 0,
                                         "2026-06-02 03:30:00"));
}

TEST(ChartDldrSchedule, LegacyDateOnlyLastRun) {
  EXPECT_FALSE(
      ChartDldrScheduleShouldRun(LocalTime(2, 4, 0), true, 3, 0, "2026-06-02"));
  wxDateTime next_day;
  next_day.Set(3, wxDateTime::Jun, 2026, 3, 5, 0);
  EXPECT_TRUE(ChartDldrScheduleShouldRun(next_day, true, 3, 0, "2026-06-02"));
}

TEST(ChartDldrSchedule, OneMinuteBeforeScheduledMinute) {
  EXPECT_FALSE(ChartDldrScheduleShouldRun(LocalTime(2, 2, 59), true, 3, 0, ""));
}

TEST(ChartDldrSchedule, ClampsHourAndMinute) {
  EXPECT_TRUE(ChartDldrScheduleShouldRun(LocalTime(2, 23, 59), true, 99, 99, ""));
}

TEST(ChartDldrSchedule, InvalidLastRunIsoTreatedAsNeverRun) {
  EXPECT_TRUE(ChartDldrScheduleShouldRun(LocalTime(2, 4, 0), true, 3, 0,
                                          "not-a-valid-timestamp"));
}

TEST(ChartDldrSchedule, DoesNotRunAgainAfterLastRunMarkedNow) {
  const wxString now_iso = wxDateTime::Now().FormatISOCombined(' ');
  EXPECT_FALSE(ChartDldrScheduleShouldRun(LocalTime(2, 4, 0), true, 3, 0,
                                          now_iso));
}
