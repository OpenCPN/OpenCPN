/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_SCHEDULE_H_
#define CHARTDLDR_SCHEDULE_H_

#include <wx/datetime.h>
#include <wx/string.h>

/**
 * Returns true when a scheduled bulk update should start.
 *
 * Fires once per local day after @a hour:@a minute until a run completes at or
 * after that day's scheduled moment (see @a last_run_iso).
 */
bool ChartDldrScheduleShouldRun(const wxDateTime& now, bool enabled, int hour,
                                int minute, const wxString& last_run_iso);

#endif  // CHARTDLDR_SCHEDULE_H_
