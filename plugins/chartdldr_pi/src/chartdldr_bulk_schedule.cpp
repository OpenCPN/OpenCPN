/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_schedule.h"

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_runner.h"
#include "chartdldr_pi.h"
#include "chartdldr_schedule_state.h"

#include <wx/log.h>
#include <wx/timer.h>
#include <wx/window.h>

namespace {

void ApplyScheduledSkip(chartdldr_pi* pi, ChartDldrScheduledSkipReason reason) {
  if (!pi || reason == ChartDldrScheduledSkipReason::None) {
    return;
  }
  const wxString status = ChartDldrScheduledSkipStatus(reason);
  ChartDldrScheduledBulkResult result;
  result.outcome = ChartDldrScheduledRunOutcome::Skipped;
  result.status_detail = status;
  ChartDldrApplyScheduledRunOutcome(pi->ScheduleConfig(), result);
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
  if (!pi) {
    return;
  }

  if (!pi->ScheduleConfig().ShouldRunNow(wxDateTime::Now())) {
    return;
  }

  ChartDldrRequestBulkUpdate(pi, ChartDldrBulkRunKind::Scheduled);
}

bool ChartDldrRequestBulkUpdate(chartdldr_pi* pi, ChartDldrBulkRunKind kind) {
  const ChartDldrBulkRequestInput input = pi->MakeBulkRequestInput();
  ChartDldrScheduledSkipReason scheduled_skip =
      ChartDldrScheduledSkipReason::None;
  if (!ChartDldrCanStartBulkRequest(input, kind, &scheduled_skip)) {
    if (ChartDldrBulkRunIsScheduled(kind) &&
        scheduled_skip != ChartDldrScheduledSkipReason::None) {
      ApplyScheduledSkip(pi, scheduled_skip);
    }
    return false;
  }

  if (!pi->EnsureDownloaderPanel()) {
    if (ChartDldrBulkRunIsScheduled(kind)) {
      ApplyScheduledSkip(pi, ChartDldrScheduledSkipReason::PanelUnavailable);
    }
    return false;
  }

  wxCommandEvent evt;
  return ChartDldrRunBulkUpdate(pi, kind, evt);
}

void ChartDldrFinishScheduledBulkRun(chartdldr_pi* pi,
                                     const ChartDldrBulkRunStats& stats) {
  if (!pi) {
    return;
  }

  const ChartDldrScheduledBulkResult result =
      ChartDldrScheduledBulkResultFromStats(
          stats.downloaded_ok(), stats.attempted, stats.failed,
          stats.new_downloads, stats.updated_downloads);

  ChartDldrApplyScheduledRunOutcome(pi->ScheduleConfig(), result);

  switch (result.outcome) {
    case ChartDldrScheduledRunOutcome::BulkSuccess:
      wxLogMessage("chartdldr_pi: scheduled bulk update finished: %s",
                   result.status_detail.c_str());
      break;
    case ChartDldrScheduledRunOutcome::BulkPartialSuccess:
      wxLogMessage(
          "chartdldr_pi: scheduled bulk update finished with failures: %s",
          result.status_detail.c_str());
      break;
    case ChartDldrScheduledRunOutcome::BulkNoAttempts:
      wxLogMessage("chartdldr_pi: scheduled bulk update: %s",
                   result.status_detail.c_str());
      break;
    case ChartDldrScheduledRunOutcome::BulkAllFailed:
      wxLogMessage(
          "chartdldr_pi: scheduled bulk update: %s; will retry later today",
          result.status_detail.c_str());
      break;
    case ChartDldrScheduledRunOutcome::Skipped:
      break;
  }
  pi->SaveConfig();
}

ChartDldrScheduler::ChartDldrScheduler(chartdldr_pi* pi)
    : pi_(pi), parent_(nullptr), timer_(nullptr) {}

ChartDldrScheduler::~ChartDldrScheduler() { Detach(); }

void ChartDldrScheduler::Attach(wxWindow* parent) {
  Detach();
  if (!parent || !pi_) {
    return;
  }
  parent_ = parent;
  timer_ = new wxTimer(parent_);
  parent_->Bind(wxEVT_TIMER, &ChartDldrScheduler::OnTimer, this,
                timer_->GetId());
}

void ChartDldrScheduler::Detach() {
  if (timer_) {
    timer_->Stop();
    if (parent_) {
      parent_->Unbind(wxEVT_TIMER, &ChartDldrScheduler::OnTimer, this,
                      timer_->GetId());
    }
    delete timer_;
    timer_ = nullptr;
  }
  parent_ = nullptr;
}

void ChartDldrScheduler::Restart() {
  ChartDldrRestartScheduleTimer(pi_, timer_);
}

void ChartDldrScheduler::OnTimer(wxTimerEvent& event) {
  ChartDldrOnScheduleTimer(pi_, event);
}
