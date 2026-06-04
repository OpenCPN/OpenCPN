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

class wxFileConfig;

/** Persisted daily bulk-update schedule and last-run bookkeeping. */
class ChartDldrScheduleConfig {
public:
  bool enabled = false;
  int hour = 3;
  int minute = 0;
  wxString last_run_iso;
  /** ISO timestamp of the last attempt (display + bookkeeping). */
  wxString last_attempt_iso;
  /** Detail only, e.g. "2 update 3 new" (no timestamp prefix). */
  wxString last_status;

  void Load(wxFileConfig* conf);
  void Save(wxFileConfig* conf) const;

  bool ShouldRunNow(const wxDateTime& now) const;

  void SetEnabled(bool value) { enabled = value; }
  void SetTime(int h, int m);
  void SetLastStatus(const wxString& status) { last_status = status; }
  const wxString& LastStatus() const { return last_status; }
};

#endif  // CHARTDLDR_SCHEDULE_CONFIG_H_
