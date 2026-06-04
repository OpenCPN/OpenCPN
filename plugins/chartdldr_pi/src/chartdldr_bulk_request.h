/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_REQUEST_H_
#define CHARTDLDR_BULK_REQUEST_H_

#include "chartdldr_bulk_run.h"

#include <wx/string.h>

enum class ChartDldrScheduledSkipReason {
  None,
  BulkDisabled,
  NoSources,
  PanelUnavailable,
};

/** Snapshot of plugin state needed to accept or skip a bulk request. */
struct ChartDldrBulkRequestInput {
  bool allow_bulk_update = false;
  bool has_chart_sources = false;
  bool bulk_run_active = false;
};

/** Returns false when the request must not start (sets scheduled_skip when scheduled). */
bool ChartDldrCanStartBulkRequest(const ChartDldrBulkRequestInput& input,
                                  ChartDldrBulkRunKind kind,
                                  ChartDldrScheduledSkipReason* scheduled_skip);

/** Pure checks before EnsureDownloaderPanel; None means proceed to panel init. */
ChartDldrScheduledSkipReason ChartDldrEvaluateScheduledBulkSkip(
    bool allow_bulk_update, bool has_chart_sources);

wxString ChartDldrScheduledSkipStatus(ChartDldrScheduledSkipReason reason);

#endif  // CHARTDLDR_BULK_REQUEST_H_
