/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_SCHEDULE_CONFIG_H_
#define CHARTDLDR_SCHEDULE_CONFIG_H_

#include <wx/datetime.h>
#include <wx/string.h>

#include "chartdldr_schedule_state.h"

class wxFileConfig;

enum class ChartDldrScheduleEligibility {
  Disabled,
  NotYet,
  BlockedToday,
  Ready,
};

/** Persisted daily bulk-update schedule and last-run bookkeeping. */
class ChartDldrScheduleConfig {
public:
  bool enabled = false;
  int hour = 3;
  int minute = 0;
  /** Legacy key (ScheduledUpdateLastRun); eligibility uses last_attempt_iso
   * only. */
  wxString last_run_iso;
  /** Authoritative attempt timestamp; drives eligibility with last_outcome. */
  wxString last_attempt_iso;
  wxString last_status;
  ChartDldrScheduledRunOutcome last_outcome =
      ChartDldrScheduledRunOutcome::Pending;

  /** Display timestamp; eligibility uses last_attempt_iso + last_outcome. */
  const wxString& LastRunDisplayIso() const { return last_attempt_iso; }

  static int MinRetryMinutes();

  void Load(wxFileConfig* conf);
  void Save(wxFileConfig* conf) const;

  ChartDldrScheduleEligibility EligibilityAt(const wxDateTime& now) const;
  bool ShouldRunNow(const wxDateTime& now) const;

  void SetEnabled(bool value) { enabled = value; }
  void SetTime(int h, int m);
  void SetLastStatus(const wxString& status) { last_status = status; }
  const wxString& LastStatus() const { return last_status; }

  /** Drops unparseable persisted timestamps (prevents retry storms). */
  void SanitizeTimestamps();

  /** Keeps ScheduledUpdateLastRun in sync with last_attempt_iso for legacy
   * readers. */
  void SyncLegacyLastRunMirror();
};

#endif  // CHARTDLDR_SCHEDULE_CONFIG_H_
