/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_prefs_time.h"

TEST(ChartDldrPrefsTime, ParseValid24HourTimes) {
  int hour = -1;
  int minute = -1;
  EXPECT_TRUE(ChartDldrParseScheduledTimeEntry("03:00", hour, minute));
  EXPECT_EQ(hour, 3);
  EXPECT_EQ(minute, 0);

  EXPECT_TRUE(ChartDldrParseScheduledTimeEntry(" 15:30 ", hour, minute));
  EXPECT_EQ(hour, 15);
  EXPECT_EQ(minute, 30);
}

TEST(ChartDldrPrefsTime, RejectsInvalidTimes) {
  int hour = 0;
  int minute = 0;
  EXPECT_FALSE(ChartDldrParseScheduledTimeEntry("", hour, minute));
  EXPECT_FALSE(ChartDldrParseScheduledTimeEntry("3:00", hour, minute));
  EXPECT_FALSE(ChartDldrParseScheduledTimeEntry("25:00", hour, minute));
  EXPECT_FALSE(ChartDldrParseScheduledTimeEntry("12:60", hour, minute));
  EXPECT_FALSE(ChartDldrParseScheduledTimeEntry("noon", hour, minute));
}

TEST(ChartDldrPrefsTime, FormatEntryClampsAndPads) {
  EXPECT_EQ(ChartDldrFormatScheduledTimeEntry(3, 5), "03:05");
  EXPECT_EQ(ChartDldrFormatScheduledTimeEntry(99, 99), "23:59");
  EXPECT_EQ(ChartDldrFormatScheduledTimeEntry(-1, -1), "00:00");
}

TEST(ChartDldrPrefsTime, PreviewEmptyUnderUnitTests) {
  EXPECT_TRUE(ChartDldrFormatScheduledTimePreview(13, 30).IsEmpty());
  EXPECT_FALSE(ChartDldrShowsScheduledTimePreview());
}

TEST(ChartDldrPrefsTime, RequiresZeroPaddedHour) {
  int hour = 0;
  int minute = 0;
  EXPECT_FALSE(ChartDldrParseScheduledTimeEntry("3:00", hour, minute));
  EXPECT_TRUE(ChartDldrParseScheduledTimeEntry("03:00", hour, minute));
}

TEST(ChartDldrPrefsTime, RoundTripEntryFormat) {
  int hour = 7;
  int minute = 45;
  const wxString entry = ChartDldrFormatScheduledTimeEntry(hour, minute);
  int parsed_hour = 0;
  int parsed_minute = 0;
  EXPECT_TRUE(
      ChartDldrParseScheduledTimeEntry(entry, parsed_hour, parsed_minute));
  EXPECT_EQ(parsed_hour, hour);
  EXPECT_EQ(parsed_minute, minute);
}
