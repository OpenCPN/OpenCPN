/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_schedule_state.h"

#include "chartdldr_schedule_config.h"

#include <wx/datetime.h>
#include <wx/intl.h>

wxString ChartDldrScheduledNeverRunDisplayText() { return _("-- : --"); }

wxString ChartDldrFormatScheduledRunTimestamp(const wxDateTime& run_time) {
  return run_time.Format(_("%Y-%m-%d %H:%M"));
}

wxString ChartDldrFormatScheduledLastRunDisplay(
    const ChartDldrScheduleConfig& schedule) {
  if (schedule.last_status.IsEmpty() && schedule.last_run_iso.IsEmpty()) {
    return ChartDldrScheduledNeverRunDisplayText();
  }
  if (!schedule.last_status.IsEmpty()) {
    return schedule.last_status;
  }
  wxDateTime run_time;
  if (run_time.ParseISOCombined(schedule.last_run_iso)) {
    return ChartDldrFormatScheduledRunTimestamp(run_time);
  }
  return schedule.last_run_iso;
}

bool ChartDldrScheduledOutcomeAdvancesLastRun(
    ChartDldrScheduledRunOutcome outcome) {
  return outcome == ChartDldrScheduledRunOutcome::Skipped ||
         outcome == ChartDldrScheduledRunOutcome::BulkNoAttempts ||
         outcome == ChartDldrScheduledRunOutcome::BulkSuccess;
}

ChartDldrScheduledRunOutcome ChartDldrScheduledOutcomeFromBulkResult(
    int downloaded_ok, int attempted) {
  if (attempted == 0) {
    return ChartDldrScheduledRunOutcome::BulkNoAttempts;
  }
  if (downloaded_ok > 0) {
    return ChartDldrScheduledRunOutcome::BulkSuccess;
  }
  return ChartDldrScheduledRunOutcome::BulkAllFailed;
}

wxString ChartDldrScheduledStatusFromBulkResult(int downloaded_ok,
                                                int attempted, int failed,
                                                int new_downloads,
                                                int updated_downloads) {
  if (attempted == 0) {
    return _("No charts to update");
  }
  if (downloaded_ok > 0) {
    return wxString::Format(_("%d update %d new"), updated_downloads,
                            new_downloads);
  }
  return wxString::Format(_("failed: %d of %d charts"), failed, attempted);
}

void ChartDldrApplyScheduledRunOutcome(ChartDldrScheduleConfig& schedule,
                                       ChartDldrScheduledRunOutcome outcome,
                                       const wxString& status_detail,
                                       const wxDateTime* run_time) {
  wxDateTime when = wxDateTime::Now();
  if (run_time && run_time->IsValid()) {
    when = *run_time;
  }

  const wxString stamp = ChartDldrFormatScheduledRunTimestamp(when);
  schedule.last_status = stamp + " " + status_detail;
  if (ChartDldrScheduledOutcomeAdvancesLastRun(outcome)) {
    schedule.last_run_iso = when.FormatISOCombined(' ');
  }
}
