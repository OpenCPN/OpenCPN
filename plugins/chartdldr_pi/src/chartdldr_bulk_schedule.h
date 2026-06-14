/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_SCHEDULE_H_
#define CHARTDLDR_BULK_SCHEDULE_H_

#include "chartdldr_bulk.h"

class chartdldr_pi;
class wxIdleEvent;
class wxTimer;
class wxTimerEvent;
class wxWindow;

struct ChartDldrBulkRunStats;

enum class ChartDldrScheduledSkipReason {
  None,
  BulkDisabled,
  NoSources,
  PanelUnavailable,
};

struct ChartDldrBulkRequestInput {
  bool allow_bulk_update = false;
  bool has_chart_sources = false;
  bool bulk_run_active = false;
};

bool ChartDldrCanStartBulkRequest(const ChartDldrBulkRequestInput& input,
                                  ChartDldrBulkRunKind kind,
                                  ChartDldrScheduledSkipReason* scheduled_skip);

ChartDldrScheduledSkipReason ChartDldrEvaluateScheduledBulkSkip(
    bool allow_bulk_update, bool has_chart_sources);

wxString ChartDldrScheduledSkipStatus(ChartDldrScheduledSkipReason reason);

void ChartDldrRestartScheduleTimer(chartdldr_pi* pi, wxTimer* timer);
void ChartDldrOnScheduleTimer(chartdldr_pi* pi, wxTimerEvent& event);
bool ChartDldrRequestBulkUpdate(chartdldr_pi* pi, ChartDldrBulkRunKind kind);
void ChartDldrFinishScheduledBulkRun(chartdldr_pi* pi,
                                     const ChartDldrBulkRunStats& stats);

class ChartDldrScheduler {
public:
  explicit ChartDldrScheduler(chartdldr_pi* pi);
  ~ChartDldrScheduler();

  void Attach(wxWindow* parent);
  void Detach();
  void Restart();
  void EnsureIdleWatch(bool enable);

private:
  void OnTimer(wxTimerEvent& event);
  void OnIdle(wxIdleEvent& event);

  chartdldr_pi* pi_;
  wxWindow* parent_;
  wxTimer* timer_;
  bool idle_bound_ = false;
};

#endif  // CHARTDLDR_BULK_SCHEDULE_H_
