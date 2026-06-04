/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_SCHEDULE_STATE_H_
#define CHARTDLDR_SCHEDULE_STATE_H_

#include <wx/datetime.h>
#include <wx/string.h>

class ChartDldrScheduleConfig;

/** Result of a scheduled bulk attempt (timer or prefs "Run update now"). */
enum class ChartDldrScheduledRunOutcome {
  /** Preconditions failed; do not retry until tomorrow. */
  Skipped,
  /** Bulk ran but nothing was attempted (e.g. no charts selected). */
  BulkNoAttempts,
  /** Charts were attempted but none succeeded. */
  BulkAllFailed,
  /** At least one chart downloaded successfully. */
  BulkSuccess,
};

wxString ChartDldrScheduledNeverRunDisplayText();

wxString ChartDldrFormatScheduledRunTimestamp(const wxDateTime& run_time);

wxString ChartDldrFormatScheduledLastRunDisplay(
    const ChartDldrScheduleConfig& schedule);

bool ChartDldrScheduledOutcomeAdvancesLastRun(
    ChartDldrScheduledRunOutcome outcome);

ChartDldrScheduledRunOutcome ChartDldrScheduledOutcomeFromBulkResult(
    int downloaded_ok, int attempted);

wxString ChartDldrScheduledStatusFromBulkResult(int downloaded_ok,
                                                int attempted, int failed,
                                                int new_downloads,
                                                int updated_downloads);

void ChartDldrApplyScheduledRunOutcome(ChartDldrScheduleConfig& schedule,
                                       ChartDldrScheduledRunOutcome outcome,
                                       const wxString& status_detail,
                                       const wxDateTime* run_time = nullptr);

#endif  // CHARTDLDR_SCHEDULE_STATE_H_
