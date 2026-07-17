/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_lifecycle.h"

bool ChartDldrBulkSessionEnd::ShouldCopyStats() const {
  return ChartDldrBulkSessionIsSuccessfulComplete(reason);
}

bool ChartDldrBulkSessionEnd::ShouldCancelPanel() const {
  return !ChartDldrBulkSessionIsSuccessfulComplete(reason);
}

bool ChartDldrBulkSessionEnd::ShouldCancelGlobal() const {
  return ShouldCancelPanel() &&
         ChartDldrBulkSessionShouldCancelGlobalDownloads(reason);
}

bool ChartDldrBulkSessionEnd::ShouldFinalizeUi() const {
  return ChartDldrBulkSessionIsSuccessfulComplete(reason);
}

bool ChartDldrBulkSessionEnd::ShouldApplyChartDb() const {
  return ShouldFinalizeUi() && stats.downloaded_ok() > 0;
}

ChartDldrBulkSessionEnd::ScheduledFinish ChartDldrBulkSessionEnd::Scheduled()
    const {
  if (!was_scheduled) {
    return ScheduledFinish::None;
  }
  return ShouldFinalizeUi() ? ScheduledFinish::Complete
                            : ScheduledFinish::Abort;
}

ChartDldrBulkSessionEnd ChartDldrBulkSessionEndFor(
    ChartDldrBulkTeardownReason reason, bool was_scheduled,
    const ChartDldrBulkRunStats& stats) {
  ChartDldrBulkSessionEnd end;
  end.reason = reason;
  end.was_scheduled = was_scheduled;
  end.stats = stats;
  return end;
}
