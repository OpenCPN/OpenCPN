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
  Skipped,
  BulkNoAttempts,
  BulkAllFailed,
  BulkPartialSuccess,
  BulkSuccess,
};

wxString ChartDldrScheduledNeverRunDisplayText();

/** Separator between timestamp and detail in last-run display (" - "). */
wxString ChartDldrScheduledDisplaySeparator();

wxString ChartDldrFormatScheduledRunTimestamp(const wxDateTime& run_time);

/** Persisted attempt/run timestamp (local time, seconds). */
wxString ChartDldrFormatScheduleRunIso(const wxDateTime& run_time);

bool ChartDldrParseScheduleRunIso(const wxString& iso, wxDateTime& out);

wxString ChartDldrFormatScheduledLastRunDisplay(
    const ChartDldrScheduleConfig& schedule);

bool ChartDldrScheduledOutcomeAdvancesLastRun(
    ChartDldrScheduledRunOutcome outcome);

ChartDldrScheduledRunOutcome ChartDldrScheduledOutcomeFromBulkResult(
    int downloaded_ok, int attempted, int failed);

wxString ChartDldrScheduledStatusFromBulkResult(int downloaded_ok,
                                                int attempted, int failed,
                                                int new_downloads,
                                                int updated_downloads);

void ChartDldrApplyScheduledRunOutcome(ChartDldrScheduleConfig& schedule,
                                       ChartDldrScheduledRunOutcome outcome,
                                       const wxString& status_detail,
                                       const wxDateTime* run_time = nullptr);

void ChartDldrMigrateLegacyScheduleStatus(ChartDldrScheduleConfig& schedule);

#endif  // CHARTDLDR_SCHEDULE_STATE_H_
