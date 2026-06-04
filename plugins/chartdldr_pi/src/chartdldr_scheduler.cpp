/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_scheduler.h"

#include "chartdldr_bulk_schedule.h"
#include "chartdldr_pi.h"

#include <wx/timer.h>
#include <wx/window.h>

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
