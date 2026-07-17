/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_schedule.h"
#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"

#include <wx/datetime.h>
#include <wx/intl.h>

ChartDldrBulkRequestEligibility ChartDldrEvaluateBulkRequest(
    const ChartDldrBulkRequestInput& input, ChartDldrBulkRunMode mode,
    ChartDldrBulkRequestOrigin origin, const ChartDldrScheduleConfig& schedule,
    const wxDateTime& now) {
  ChartDldrBulkRequestEligibility result;
  // Deny while a bulk session is already active. Timers stay quiet so the next
  // check can retry without a same-day skip; manual feedback is handled by the
  // request caller from the blocker, not as a scheduled skip reason.
  if (input.blocker != ChartDldrBulkStartBlocker::None) {
    return result;
  }

  if (ChartDldrBulkRunModeIsScheduled(mode)) {
    if (origin == ChartDldrBulkRequestOrigin::Timer &&
        !schedule.ShouldRunNow(now)) {
      result.status = ChartDldrBulkRequestStatus::StaleTimer;
      return result;
    }

    result.skip = ChartDldrEvaluateScheduledBulkSkip(input.allow_bulk_update,
                                                     input.has_chart_sources);
    if (result.skip == ChartDldrScheduledSkipReason::None) {
      result.status = ChartDldrBulkRequestStatus::Allowed;
    }
    return result;
  }

  if (input.allow_bulk_update && input.has_chart_sources) {
    result.status = ChartDldrBulkRequestStatus::Allowed;
  }
  return result;
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
      return _(ChartDldrScheduleMessages::kBulkDisabled);
    case ChartDldrScheduledSkipReason::NoSources:
      return _(ChartDldrScheduleMessages::kNoSources);
    case ChartDldrScheduledSkipReason::PanelUnavailable:
      return _(ChartDldrScheduleMessages::kPanelUnavailable);
    case ChartDldrScheduledSkipReason::None:
      break;
  }
  return wxEmptyString;
}
