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

ChartDldrScheduleConfig MakeSchedule(bool enabled, int hour, int minute) {
  ChartDldrScheduleConfig schedule;
  schedule.enabled = enabled;
  schedule.SetTime(hour, minute);
  return schedule;
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
  EXPECT_TRUE(MakeSchedule(true, 99, 99).ShouldRunNow(LocalTime(2, 23, 59)));
}

TEST(ChartDldrScheduleConfig, CheckIntervalMatchesManual) {
  EXPECT_EQ(ChartDldrScheduleTiming::kCheckIntervalMs, 60 * 1000);
}

TEST(ChartDldrScheduleConfig, EligibilityGate) {
  ChartDldrScheduleConfig schedule = MakeSchedule(false, 3, 0);
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 12, 0)),
            ChartDldrScheduleEligibility::Disabled);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 12, 0)));

  schedule = MakeSchedule(true, 3, 0);
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 2, 0)),
            ChartDldrScheduleEligibility::NotYet);
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 2, 30)));
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 2, 59)));
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 3, 0)));
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 4, 15)));
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 4, 0)),
            ChartDldrScheduleEligibility::Ready);

  schedule.last_attempt_iso = "2026-06-02 03:30:00";
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 4, 0)),
            ChartDldrScheduleEligibility::BlockedToday);
  schedule.last_attempt_iso = "2026-06-02 03:00:00";
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 3, 30)));

  wxDateTime next_day;
  next_day.Set(3, wxDateTime::Jun, 2026, 3, 5, 0);
  EXPECT_TRUE(schedule.ShouldRunNow(next_day));
}

TEST(ChartDldrScheduleConfig, LegacyDateOnlyAttemptIso) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = "2026-06-02";
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
  wxDateTime next_day;
  next_day.Set(3, wxDateTime::Jun, 2026, 3, 5, 0);
  EXPECT_TRUE(schedule.ShouldRunNow(next_day));
}

TEST(ChartDldrScheduleConfig, SameDayRetryAfterAllFailed) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = "2026-06-02 04:00:00";
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkAllFailed;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 30)));
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 7, 59)));
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 8, 0)));
}

TEST(ChartDldrScheduleConfig, LaterSlotDoesNotBypassSameDaySuccess) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 5, 0);
  schedule.last_attempt_iso = "2026-06-02 03:30:00";
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 5, 30)));
  EXPECT_EQ(schedule.EligibilityAt(LocalTime(2, 5, 30)),
            ChartDldrScheduleEligibility::BlockedToday);
}

TEST(ChartDldrScheduleConfig, LaterSlotDoesNotBypassFailedRetryWindow) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 5, 0);
  schedule.last_attempt_iso = "2026-06-02 03:30:00";
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkAllFailed;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 7, 29)));
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 7, 30)));
}

TEST(ChartDldrScheduleConfig, InvalidAttemptIsoTreatedAsNeverRun) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = "not-a-valid-timestamp";
  schedule.SanitizeTimestamps();
  EXPECT_TRUE(schedule.last_attempt_iso.empty());
  EXPECT_TRUE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrScheduleConfig, DoesNotRunAgainAfterAttemptMarkedNow) {
  ChartDldrScheduleConfig schedule = MakeSchedule(true, 3, 0);
  schedule.last_attempt_iso = wxDateTime::Now().FormatISOCombined(' ');
  schedule.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
  EXPECT_FALSE(schedule.ShouldRunNow(LocalTime(2, 4, 0)));
}

TEST(ChartDldrScheduleConfig, SanitizeDropsInvalidTimestamps) {
  ChartDldrScheduleConfig schedule;
  schedule.last_attempt_iso = "also-invalid";
  schedule.SanitizeTimestamps();
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
    written.last_attempt_iso = "2026-06-02 14:45:00";
    written.last_status = "2 update 1 new";
    written.last_outcome = ChartDldrScheduledRunOutcome::BulkSuccess;
    ASSERT_TRUE(written.Save(&conf));
    ASSERT_TRUE(conf.Flush());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig loaded;
    loaded.Load(&conf);
    EXPECT_TRUE(loaded.enabled);
    EXPECT_EQ(loaded.hour, 14);
    EXPECT_EQ(loaded.minute, 45);
    EXPECT_EQ(loaded.last_attempt_iso, "2026-06-02 14:45:00");
    EXPECT_EQ(loaded.last_status, "2 update 1 new");
    EXPECT_EQ(loaded.last_outcome, ChartDldrScheduledRunOutcome::BulkSuccess);

    wxString last_attempt_key;
    conf.Read(_T("ScheduledUpdateLastAttempt"), &last_attempt_key);
    EXPECT_EQ(last_attempt_key, "2026-06-02 14:45:00");

    // Legacy LastRun is no longer written; attempt is authoritative.
    wxString last_run_key;
    EXPECT_FALSE(conf.Read(_T("ScheduledUpdateLastRun"), &last_run_key));
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

TEST(ChartDldrScheduleConfig, LoadLegacyLastRunDateAloneDoesNotSetAttempt) {
  ScopedConfigFile config_file("chartdldr_sched_legacy");
  const wxString& path = config_file.path();

  ASSERT_TRUE(WriteLegacyLastRunDateIni(path, "2026-06-01"));

  wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
  ChartDldrScheduleConfig loaded;
  loaded.Load(&conf);
  EXPECT_TRUE(loaded.last_attempt_iso.empty());
}

TEST(ChartDldrScheduleConfig, LoadMigratesLegacyLastRunWithStatus) {
  ScopedConfigFile config_file("chartdldr_sched_legacy_status");
  const wxString& path = config_file.path();

  {
    wxFFile touch(path, "wb");
    ASSERT_TRUE(touch.IsOpened());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    conf.Write(_T("ScheduledUpdateLastRun"), _T("2026-06-01 03:00:00"));
    conf.Write(_T("ScheduledUpdateLastStatus"), _T("2 update 1 new"));
    ASSERT_TRUE(conf.Flush());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig loaded;
    loaded.Load(&conf);
    EXPECT_EQ(loaded.last_attempt_iso, "2026-06-01 03:00:00");
    EXPECT_EQ(loaded.last_status, "2 update 1 new");
  }
}

TEST(ChartDldrScheduleConfig, LoadMissingOutcomeDefaultsToPending) {
  ScopedConfigFile config_file("chartdldr_sched_legacy_outcome");
  const wxString& path = config_file.path();

  {
    wxFFile touch(path, "wb");
    ASSERT_TRUE(touch.IsOpened());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    conf.Write(_T("ScheduledUpdateLastAttempt"), _T("2026-06-02 04:00:00"));
    conf.Write(_T("ScheduledUpdateLastStatus"), _T("failed: 4 of 4 charts"));
    ASSERT_TRUE(conf.Flush());
  }

  {
    wxFileConfig conf(wxEmptyString, wxEmptyString, path, wxEmptyString, 0);
    ChartDldrScheduleConfig loaded;
    loaded.Load(&conf);
    EXPECT_EQ(loaded.last_outcome, ChartDldrScheduledRunOutcome::Pending);
    EXPECT_TRUE(
        ChartDldrScheduledOutcomeAllowsSameDayRetry(loaded.last_outcome));
  }
}
