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
#include <wx/fileconf.h>
#include <wx/filename.h>

#include "chartdldr_schedule_config.h"

namespace {

wxDateTime LocalTime(int day, int hour, int minute) {
  wxDateTime dt;
  dt.Set(day, wxDateTime::Jun, 2026, hour, minute, 0);
  return dt;
}

}  // namespace

TEST(ChartDldrScheduleConfig, SetTimeClamps) {
  ChartDldrScheduleConfig schedule;
  schedule.SetTime(99, 99);
  EXPECT_EQ(schedule.hour, 23);
  EXPECT_EQ(schedule.minute, 59);
}

TEST(ChartDldrScheduleConfig, ShouldRunDelegatesToScheduleGate) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = true;
  schedule.SetTime(3, 0);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 2, 30)));
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrScheduleConfig, LoadSaveRoundTrip) {
  const wxString path = wxFileName::CreateTempFileName("chartdldr_sched_test");
  ASSERT_FALSE(path.empty());

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig written;
    written.enabled = true;
    written.SetTime(14, 45);
    written.last_run_iso = "2026-06-02 14:45:00";
    written.last_status = "OK (2 charts)";
    written.Save(&conf);
    conf.Flush();
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig loaded;
    loaded.Load(&conf);
    EXPECT_TRUE(loaded.enabled);
    EXPECT_EQ(loaded.hour, 14);
    EXPECT_EQ(loaded.minute, 45);
    EXPECT_EQ(loaded.last_run_iso, "2026-06-02 14:45:00");
    EXPECT_EQ(loaded.last_status, "OK (2 charts)");
  }

  wxRemoveFile(path);
}

TEST(ChartDldrScheduleConfig, LoadMigratesLegacyLastRunDate) {
  const wxString path = wxFileName::CreateTempFileName("chartdldr_sched_legacy");
  ASSERT_FALSE(path.empty());

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    conf.Write("ScheduledUpdateLastRunDate", "2026-06-01");
    conf.Flush();
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig loaded;
    loaded.Load(&conf);
    EXPECT_EQ(loaded.last_run_iso, "2026-06-01");
  }

  wxRemoveFile(path);
}
