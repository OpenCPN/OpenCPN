/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_schedule.h"
#include "chartdldr_pi.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_schedule_state.h"

#include <wx/app.h>
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

  wxTheApp->CallAfter([pi]() {
    ChartDldrRequestBulkUpdate(pi, ChartDldrBulkRunKind::Scheduled);
  });
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

  if (ChartDldrBulkRunIsScheduled(kind)) {
    ChartDldrRecordScheduledAttemptStart(pi->ScheduleConfig(),
                                         wxDateTime::Now());
    pi->SaveConfig();
    pi->StartAsyncBulkUpdate();
    return true;
  }

  wxCommandEvent evt;
  ChartDldrPanelImpl* const panel = pi->GetDownloaderPanel();
  if (!panel) {
    return false;
  }
  ChartDldrBulkRunStats stats;
  return panel->Bulk().RunInteractive(kind, evt, stats);
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
  wxLogMessage("chartdldr_pi: scheduled bulk update: %s",
               result.status_detail.c_str());
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
  EnsureIdleWatch(false);
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

void ChartDldrScheduler::EnsureIdleWatch(bool enable) {
  if (!parent_) {
    return;
  }
  if (enable && !idle_bound_) {
    parent_->Bind(wxEVT_IDLE, &ChartDldrScheduler::OnIdle, this);
    idle_bound_ = true;
  } else if (!enable && idle_bound_) {
    parent_->Unbind(wxEVT_IDLE, &ChartDldrScheduler::OnIdle, this);
    idle_bound_ = false;
  }
}

void ChartDldrScheduler::OnTimer(wxTimerEvent& event) {
  ChartDldrOnScheduleTimer(pi_, event);
}

void ChartDldrScheduler::OnIdle(wxIdleEvent& event) {
  ChartDldrPanelImpl* const panel =
      pi_ ? pi_->GetDownloaderPanel() : nullptr;
  if (!panel || !panel->Bulk().IsScheduledRunActive()) {
    EnsureIdleWatch(false);
    return;
  }

  if (panel->Bulk().StepScheduledRun()) {
    event.RequestMore();
  } else {
    EnsureIdleWatch(false);
  }
}
