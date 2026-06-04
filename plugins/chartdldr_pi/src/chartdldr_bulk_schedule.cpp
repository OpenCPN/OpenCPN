/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_schedule.h"

#include "chartdldr_panel.h"
#include "chartdldr_pi.h"
#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"

#include <wx/intl.h>
#include <wx/log.h>
#include <wx/timer.h>

namespace {

void FinishScheduledSkip(chartdldr_pi* pi, const wxString& status) {
  if (!pi) {
    return;
  }
  ChartDldrApplyScheduledRunOutcome(pi->ScheduleConfig(),
                                   ChartDldrScheduledRunOutcome::Skipped,
                                   status);
  wxLogMessage("chartdldr_pi: %s", status.c_str());
  pi->SaveConfig();
}

}  // namespace

void ChartDldrRestartScheduleTimer(chartdldr_pi* pi, wxTimer* timer) {
  if (!pi || !timer) {
    return;
  }
  timer->Stop();
  if (pi->ScheduleConfig().enabled && pi->m_allow_bulk_update) {
    timer->Start(60000, wxTIMER_CONTINUOUS);
  }
}

void ChartDldrOnScheduleTimer(chartdldr_pi* pi, wxTimerEvent& event) {
  event.Skip();
  if (!pi || pi->m_bulk_run_active) {
    return;
  }

  if (!pi->ScheduleConfig().ShouldRunNow(wxDateTime::Now())) {
    return;
  }

  ChartDldrRequestBulkUpdate(pi, ChartDldrBulkRunKind::Scheduled);
}

bool ChartDldrRequestBulkUpdate(chartdldr_pi* pi, ChartDldrBulkRunKind kind) {
  if (!pi || pi->m_bulk_run_active) {
    return false;
  }

  const bool scheduled = ChartDldrBulkRunIsScheduled(kind);

  if (!pi->m_allow_bulk_update) {
    if (scheduled) {
      FinishScheduledSkip(pi,
                          _("Skipped: enable bulk update in preferences"));
    }
    return false;
  }
  if (pi->m_ChartSources.empty()) {
    if (scheduled) {
      FinishScheduledSkip(pi, _("Skipped: no chart sources configured"));
    }
    return false;
  }
  if (!ChartDldrEnsureDownloaderPanel(pi)) {
    if (scheduled) {
      FinishScheduledSkip(pi, _("Skipped: downloader panel unavailable"));
    }
    return false;
  }

  wxCommandEvent evt;
  if (!pi->GetDownloaderPanel()) {
    return false;
  }
  return pi->GetDownloaderPanel()->RunBulkUpdate(kind, evt);
}

void ChartDldrFinishScheduledBulkRun(chartdldr_pi* pi, int downloaded_ok,
                                     int attempted, int failed) {
  if (!pi) {
    return;
  }

  const ChartDldrScheduledRunOutcome outcome =
      ChartDldrScheduledOutcomeFromBulkResult(downloaded_ok, attempted);
  const wxString status =
      ChartDldrScheduledStatusFromBulkResult(downloaded_ok, attempted, failed);

  ChartDldrApplyScheduledRunOutcome(pi->ScheduleConfig(), outcome, status);

  if (outcome == ChartDldrScheduledRunOutcome::BulkSuccess) {
    wxLogMessage("chartdldr_pi: scheduled bulk update finished: %s",
                 status.c_str());
  } else if (outcome == ChartDldrScheduledRunOutcome::BulkNoAttempts) {
    wxLogMessage(
        "chartdldr_pi: scheduled bulk update: %s; will retry later today if "
        "still past the scheduled time",
        status.c_str());
  } else {
    wxLogMessage(
        "chartdldr_pi: scheduled bulk update: %s; no successful downloads, "
        "will retry later today",
        status.c_str());
  }
  pi->SaveConfig();
}
