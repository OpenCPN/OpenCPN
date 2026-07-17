/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_pump.h"

#include "chartdldr_bulk_lifecycle.h"
#include "chartdldr_bulk_orchestrate.h"

#include <wx/event.h>
#include <wx/timer.h>

ChartDldrBulkPump::ChartDldrBulkPump(ChartDldrBulkOrchestrate& orchestrate,
                                     wxEvtHandler& handler)
    : orchestrate_(orchestrate), handler_(handler) {}

ChartDldrBulkPump::~ChartDldrBulkPump() {
  InvalidatePending();
  if (stall_timer_ && stall_timer_->IsRunning()) {
    stall_timer_->Stop();
  }
}

void ChartDldrBulkPump::Enter() { ChartDldrEnterBulkModalSuppress(); }

void ChartDldrBulkPump::Leave() { ChartDldrLeaveBulkModalSuppress(); }

void ChartDldrBulkPump::InvalidatePending() { ++epoch_; }

void ChartDldrBulkPump::Schedule() {
  const uint64_t epoch = epoch_;
  // Queue on the panel handler: wxEvtHandler destruction removes pending
  // events before the plugin library can unload.
  handler_.CallAfter([this, epoch]() {
    if (epoch != epoch_) {
      return;
    }
    ChartDldrBulkPumpGuard guard(*this);
    if (!orchestrate_.PumpBulkRun() && orchestrate_.IsRunActive()) {
      orchestrate_.TeardownBulkSession(ChartDldrBulkTeardownReasonAfterLoop(
          orchestrate_.Session().DownloadCancel().IsSessionCancelled()));
    }
  });
}

void ChartDldrBulkPump::SetTransferStallTimerRunning(bool running) {
  if (!stall_timer_) {
    stall_timer_ = std::make_unique<wxTimer>(&handler_);
    handler_.Bind(wxEVT_TIMER, &ChartDldrBulkPump::OnTransferStallTimer, this,
                  stall_timer_->GetId());
  }
  if (running) {
    if (!stall_timer_->IsRunning()) {
      stall_timer_->Start(200, wxTIMER_CONTINUOUS);
    }
  } else if (stall_timer_->IsRunning()) {
    stall_timer_->Stop();
  }
}

void ChartDldrBulkPump::OnTransferStallTimer(wxTimerEvent& event) {
  (void)event;
  orchestrate_.OnTransferStallTick();
}

ChartDldrBulkPumpGuard::ChartDldrBulkPumpGuard(ChartDldrBulkPump& pump)
    : pump_(pump) {
  pump_.Enter();
}

ChartDldrBulkPumpGuard::~ChartDldrBulkPumpGuard() { pump_.Leave(); }
