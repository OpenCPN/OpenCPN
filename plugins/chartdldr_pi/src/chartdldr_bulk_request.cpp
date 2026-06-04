/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_request.h"

#include <wx/intl.h>

bool ChartDldrCanStartBulkRequest(const ChartDldrBulkRequestInput& input,
                                  ChartDldrBulkRunKind kind,
                                  ChartDldrScheduledSkipReason* scheduled_skip) {
  if (input.bulk_run_active) {
    return false;
  }

  if (ChartDldrBulkRunIsScheduled(kind)) {
    const ChartDldrScheduledSkipReason reason = ChartDldrEvaluateScheduledBulkSkip(
        input.allow_bulk_update, input.has_chart_sources);
    if (scheduled_skip) {
      *scheduled_skip = reason;
    }
    return reason == ChartDldrScheduledSkipReason::None;
  }

  if (scheduled_skip) {
    *scheduled_skip = ChartDldrScheduledSkipReason::None;
  }
  return input.allow_bulk_update && input.has_chart_sources;
}

ChartDldrScheduledSkipReason ChartDldrEvaluateScheduledBulkSkip(
    bool allow_bulk_update, bool has_chart_sources) {
  if (!allow_bulk_update) {
    return ChartDldrScheduledSkipReason::BulkDisabled;
  }
  if (!has_chart_sources) {
    return ChartDldrScheduledSkipReason::NoSources;
  }
  return ChartDldrScheduledSkipReason::None;
}

wxString ChartDldrScheduledSkipStatus(ChartDldrScheduledSkipReason reason) {
  switch (reason) {
    case ChartDldrScheduledSkipReason::BulkDisabled:
      return _("Skipped: enable bulk update in preferences");
    case ChartDldrScheduledSkipReason::NoSources:
      return _("Skipped: no chart sources configured");
    case ChartDldrScheduledSkipReason::PanelUnavailable:
      return _("Skipped: downloader panel unavailable");
    case ChartDldrScheduledSkipReason::None:
      break;
  }
  return wxEmptyString;
}
