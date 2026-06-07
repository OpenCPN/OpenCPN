/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <fstream>

#include <wx/datetime.h>
#include <wx/ffile.h>
#include <wx/fileconf.h>
#include <wx/filename.h>

#include "chartdldr_schedule_config.h"

namespace {

wxDateTime LocalTime(int day, int hour, int minute) {
  wxDateTime dt;
  dt.Set(day, wxDateTime::Jun, 2026, hour, minute, 0);
  return dt;
}

// Windows CI hangs when wxFileConfig creates/flushes under the system temp dir
// or without wxWidgets initialized (see Actions at LoadSaveRoundTrip).
class ScopedConfigFile {
 public:
  explicit ScopedConfigFile(const char* stem) {
    static unsigned serial = 0;
    const wxString name =
        wxString::Format("%s_%u.conf", stem, static_cast<unsigned>(++serial));
    path_ = wxFileName(wxString::FromUTF8(TESTDATA), name).GetFullPath();
    if (wxFileExists(path_)) {
      wxRemoveFile(path_);
    }
  }

  const wxString& path() const { return path_; }

  ~ScopedConfigFile() {
    if (wxFileExists(path_)) {
      wxRemoveFile(path_);
    }
  }

 private:
  wxString path_;
};

bool WriteLegacyLastRunDateIni(const wxString& path, const wxString& date) {
  std::ofstream out(path.ToStdString());
  if (!out.good()) {
    return false;
  }
  out << "ScheduledUpdateLastRunDate=" << date.ToUTF8().data() << "\n";
  return out.good();
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

TEST(ChartDldrScheduleConfig, EligibilityStates) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = false;
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 4, 0)),
            ChartDldrScheduleEligibility::Disabled);
  schedule.enabled = true;
  schedule.SetTime(3, 0);
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 2, 0)),
            ChartDldrScheduleEligibility::NotYet);
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 4, 0)),
            ChartDldrScheduleEligibility::Ready);
  schedule.last_attempt_iso = "2026-06-02 04:00:00";
  schedule.last_run_iso = schedule.last_attempt_iso;
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 4, 0)),
            ChartDldrScheduleEligibility::BlockedToday);
}

TEST(ChartDldrScheduleConfig, SanitizeDropsInvalidTimestamps) {
  ChartDldrScheduleConfig schedule;
  schedule.last_run_iso = "not-a-valid-timestamp";
  schedule.last_attempt_iso = "also-invalid";
  schedule.SanitizeTimestamps();
  EXPECT_TRUE(schedule.last_run_iso.empty());
  EXPECT_TRUE(schedule.last_attempt_iso.empty());
}

TEST(ChartDldrScheduleConfig, MinRetryMinutesIsFourHours) {
  EXPECT_EQ(ChartDldrScheduleConfig::MinRetryMinutes(), 240);
}

TEST(ChartDldrScheduleConfig, LoadSaveRoundTrip) {
  ScopedConfigFile config_file("chartdldr_sched_test");
  const wxString& path = config_file.path();

  {
    wxFFile touch(path, "wb");
    ASSERT_TRUE(touch.IsOpened());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig written;
    written.enabled = true;
    written.SetTime(14, 45);
    written.last_run_iso = "2026-06-02 14:45:00";
    written.last_attempt_iso = "2026-06-02 14:45:00";
    written.last_status = "2 update 1 new";
    written.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
    written.Save(&conf);
    ASSERT_TRUE(conf.Flush());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig loaded;
    loaded.Load(&conf);
    EXPECT_TRUE(loaded.enabled);
    EXPECT_EQ(loaded.hour, 14);
    EXPECT_EQ(loaded.minute, 45);
    EXPECT_EQ(loaded.last_run_iso, "2026-06-02 14:45:00");
    EXPECT_EQ(loaded.last_attempt_iso, "2026-06-02 14:45:00");
    EXPECT_EQ(loaded.last_status, "2 update 1 new");
    EXPECT_EQ(loaded.last_outcome, ChartDldrScheduledRunOutcome::BulkSuccess);
  }
}

TEST(ChartDldrScheduleConfig, LoadRejectsInvalidOutcomeValue) {
  ScopedConfigFile config_file("chartdldr_sched_invalid_outcome");
  const wxString& path = config_file.path();

  {
    wxFFile touch(path, "wb");
    ASSERT_TRUE(touch.IsOpened());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    conf.Write(_T("ScheduledUpdateLastOutcome"), 999L);
    conf.Write(_T("ScheduledUpdateLastAttempt"), _T("2026-06-02 04:00:00"));
    ASSERT_TRUE(conf.Flush());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig loaded;
    loaded.Load(&conf);
    EXPECT_EQ(loaded.last_outcome, ChartDldrScheduledRunOutcome::Pending);
  }
}

TEST(ChartDldrScheduleConfig, LoadMigratesLegacyLastRunDate) {
  ScopedConfigFile config_file("chartdldr_sched_legacy");
  const wxString& path = config_file.path();

  ASSERT_TRUE(WriteLegacyLastRunDateIni(path, "2026-06-01"));

  wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
  ChartDldrScheduleConfig loaded;
  loaded.Load(&conf);
  EXPECT_EQ(loaded.last_run_iso, "2026-06-01");
}
