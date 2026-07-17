/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_schedule.h"
#include "chartdldr_bulk_orchestrate.h"
#include "chartdldr_pi.h"
#include "chartdldr_schedule_config.h"
#include "chartdldr_schedule_state.h"

#include <wx/datetime.h>
#include <wx/app.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/timer.h>
#include <wx/window.h>

#include "ocpn_plugin.h"

void ChartDldrScheduler::InvalidatePendingCallbacks() {
  callback_gate_.Invalidate();
}

void ChartDldrScheduler::QueueDueScheduledBulkRequest() {
  if (!pi_ || !schedule_timer_) {
    return;
  }
  if (!pi_->ScheduleConfig().ShouldRunNow(wxDateTime::Now())) {
    return;
  }
  const uint64_t token = callback_gate_.Capture();
  // wxTimer is plugin-owned and deleted by Detach. Its pending CallAfter
  // events are therefore destroyed before the plugin library can unload.
  schedule_timer_->CallAfter([this, token]() {
    if (!pi_ || !IsDeferredScheduleCallbackValid(token)) {
      return;
    }
    ChartDldrRequestBulkUpdate(pi_, ChartDldrBulkRunMode::ScheduledBulk,
                               ChartDldrBulkRequestOrigin::Timer);
  });
}

namespace {

void ReportManualScheduledBulkBlocked(chartdldr_pi* pi,
                                      ChartDldrScheduledSkipReason reason) {
  if (!pi || reason == ChartDldrScheduledSkipReason::None) {
    return;
  }
  const wxString detail = ChartDldrScheduledSkipStatus(reason);
  const wxString message =
      wxString::Format(_("Scheduled update cannot start: %s"), detail.c_str());
  OCPNMessageBox_PlugIn(pi->m_parent_window, message, _("Scheduled updates"),
                        wxOK | wxICON_WARNING);
}

void ReportManualSessionActive(chartdldr_pi* pi) {
  if (!pi) {
    return;
  }
  const wxString detail = _(ChartDldrScheduleMessages::kSessionActive);
  const wxString message =
      wxString::Format(_("Scheduled update cannot start: %s"), detail.c_str());
  OCPNMessageBox_PlugIn(pi->m_parent_window, message, _("Scheduled updates"),
                        wxOK | wxICON_WARNING);
}

void ApplyScheduledSkip(chartdldr_pi* pi, ChartDldrScheduledSkipReason reason) {
  if (!pi || reason == ChartDldrScheduledSkipReason::None) {
    return;
  }
  ChartDldrScheduledBulkResult result;
  result.outcome = ChartDldrScheduledRunOutcome::Skipped;
  result.status_detail = ChartDldrScheduledSkipStatus(reason);
  ChartDldrCommitScheduledRunOutcome(pi, result);
}

void HandleScheduledRequestBlocked(chartdldr_pi* pi,
                                   ChartDldrScheduledSkipReason reason,
                                   ChartDldrBulkRequestOrigin origin) {
  if (origin == ChartDldrBulkRequestOrigin::Timer) {
    ApplyScheduledSkip(pi, reason);
  } else {
    ReportManualScheduledBulkBlocked(pi, reason);
  }
}

}  // namespace

bool ChartDldrPersistScheduleConfig(chartdldr_pi* pi,
                                    const ChartDldrScheduleConfig& cfg) {
  return pi && pi->SaveScheduleConfig(cfg);
}

namespace {

bool PersistScheduleConfig(const ChartDldrScheduleConfig& cfg, void* ctx) {
  return ChartDldrPersistScheduleConfig(static_cast<chartdldr_pi*>(ctx), cfg);
}

}  // namespace

bool ChartDldrCommitScheduledRunOutcome(
    chartdldr_pi* pi, const ChartDldrScheduledBulkResult& result) {
  if (!pi) {
    return false;
  }
  wxLogMessage("chartdldr_pi: scheduled bulk update: %s",
               result.status_detail.c_str());
  if (!ChartDldrCommitScheduleOutcome(pi->ScheduleConfig(), result,
                                      PersistScheduleConfig, pi)) {
    wxLogError(
        "chartdldr_pi: failed to persist scheduled run outcome (%s); "
        "recorded outcome in memory",
        result.status_detail.c_str());
    return false;
  }
  return true;
}

bool ChartDldrCommitScheduledAttemptStart(chartdldr_pi* pi,
                                          const wxDateTime& run_time) {
  if (!pi) {
    return false;
  }
  if (!ChartDldrCommitScheduleAttemptStart(pi->ScheduleConfig(), run_time,
                                           PersistScheduleConfig, pi)) {
    wxLogError(
        "chartdldr_pi: failed to persist scheduled attempt start; "
        "recorded in-memory backoff");
    return false;
  }
  return true;
}

void ChartDldrRestartScheduleTimer(chartdldr_pi* pi, wxTimer* timer) {
  if (!pi || !timer) {
    return;
  }
  timer->Stop();
  pi->Scheduler().InvalidatePendingCallbacks();
  if (pi->ScheduleConfig().enabled) {
    timer->Start(ChartDldrScheduleTiming::kCheckIntervalMs, wxTIMER_CONTINUOUS);
    pi->Scheduler().QueueDueScheduledBulkRequest();
  }
}

void ChartDldrOnScheduleTimer(chartdldr_pi* pi, wxTimerEvent& event) {
  event.Skip();
  if (!pi) {
    return;
  }
  pi->Scheduler().QueueDueScheduledBulkRequest();
}

bool ChartDldrRequestBulkUpdate(chartdldr_pi* pi, ChartDldrBulkRunMode mode,
                                ChartDldrBulkRequestOrigin origin) {
  const ChartDldrBulkRequestInput input = pi->MakeBulkRequestInput();
  const ChartDldrBulkRequestEligibility eligibility =
      ChartDldrEvaluateBulkRequest(input, mode, origin, pi->ScheduleConfig(),
                                   wxDateTime::Now());
  if (!eligibility.IsAllowed()) {
    if (eligibility.IsStaleTimer()) {
      return false;
    }
    // Busy session is a start blocker, not a scheduled skip reason.
    if (origin == ChartDldrBulkRequestOrigin::Manual &&
        input.blocker == ChartDldrBulkStartBlocker::SessionActive) {
      ReportManualSessionActive(pi);
      return false;
    }
    // eligibility.skip is only set for scheduled requests.
    if (eligibility.skip != ChartDldrScheduledSkipReason::None) {
      HandleScheduledRequestBlocked(pi, eligibility.skip, origin);
    }
    return false;
  }

  // A missing panel/orchestrator only counts as a scheduled skip for scheduled
  // requests; the skip reason encodes that, so no IsScheduled re-test is
  // needed.
  const ChartDldrScheduledSkipReason panel_unavailable_skip =
      ChartDldrBulkRunModeIsScheduled(mode)
          ? ChartDldrScheduledSkipReason::PanelUnavailable
          : ChartDldrScheduledSkipReason::None;
  auto block_panel_unavailable = [&]() {
    if (panel_unavailable_skip != ChartDldrScheduledSkipReason::None) {
      HandleScheduledRequestBlocked(pi, panel_unavailable_skip, origin);
    }
  };

  if (!pi->EnsureDownloaderPanel()) {
    block_panel_unavailable();
    return false;
  }

  ChartDldrBulkOrchestrate* const bulk = pi->GetBulkOrchestrate();
  if (!bulk) {
    block_panel_unavailable();
    return false;
  }

  return bulk->StartBulk(mode);
}

void ChartDldrFinishScheduledBulkRun(chartdldr_pi* pi,
                                     const ChartDldrBulkRunStats& stats) {
  if (!pi) {
    return;
  }
  ChartDldrCommitScheduledRunOutcome(
      pi, ChartDldrScheduledBulkResultFromStats(stats));
}

ChartDldrScheduler::ChartDldrScheduler(chartdldr_pi* pi)
    : pi_(pi), parent_(nullptr), schedule_timer_(nullptr) {}

ChartDldrScheduler::~ChartDldrScheduler() { Detach(); }

void ChartDldrScheduler::Attach(wxWindow* parent) {
  Detach();
  if (!parent || !pi_) {
    return;
  }
  parent_ = parent;
  schedule_timer_ = new wxTimer(parent_);
  parent_->Bind(wxEVT_TIMER, &ChartDldrScheduler::OnScheduleCheckTimer, this,
                schedule_timer_->GetId());
}

void ChartDldrScheduler::Detach() {
  InvalidatePendingCallbacks();
  if (schedule_timer_) {
    schedule_timer_->Stop();
    if (parent_) {
      parent_->Unbind(wxEVT_TIMER, &ChartDldrScheduler::OnScheduleCheckTimer,
                      this, schedule_timer_->GetId());
    }
    delete schedule_timer_;
    schedule_timer_ = nullptr;
  }
  parent_ = nullptr;
}

void ChartDldrScheduler::Restart() {
  ChartDldrRestartScheduleTimer(pi_, schedule_timer_);
}

void ChartDldrScheduler::OnScheduleCheckTimer(wxTimerEvent& event) {
  ChartDldrOnScheduleTimer(pi_, event);
}
