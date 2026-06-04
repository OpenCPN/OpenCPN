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

bool ChartDldrScheduledOutcomeAdvancesLastRun(
    ChartDldrScheduledRunOutcome outcome) {
  return outcome == ChartDldrScheduledRunOutcome::Skipped ||
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
                                                int attempted, int failed) {
  if (attempted == 0) {
    return _("No charts downloaded");
  }
  if (failed == 0) {
    return wxString::Format(_("OK (%d charts)"), downloaded_ok);
  }
  return wxString::Format(_("%d failed of %d"), failed, attempted);
}

void ChartDldrApplyScheduledRunOutcome(ChartDldrScheduleConfig& schedule,
                                       ChartDldrScheduledRunOutcome outcome,
                                       const wxString& status) {
  schedule.SetLastStatus(status);
  if (ChartDldrScheduledOutcomeAdvancesLastRun(outcome)) {
    schedule.last_run_iso = wxDateTime::Now().FormatISOCombined(' ');
  }
}
