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

bool ChartDldrScheduledOutcomeAdvancesLastRun(
    ChartDldrScheduledRunOutcome outcome);

ChartDldrScheduledRunOutcome ChartDldrScheduledOutcomeFromBulkResult(
    int downloaded_ok, int attempted);

wxString ChartDldrScheduledStatusFromBulkResult(int downloaded_ok,
                                                int attempted, int failed);

void ChartDldrApplyScheduledRunOutcome(ChartDldrScheduleConfig& schedule,
                                       ChartDldrScheduledRunOutcome outcome,
                                       const wxString& status);

#endif  // CHARTDLDR_SCHEDULE_STATE_H_
