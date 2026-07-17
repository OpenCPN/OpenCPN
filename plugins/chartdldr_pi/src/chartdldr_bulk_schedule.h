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

#include <atomic>
#include <cstdint>

class ChartDldrScheduleConfig;
class chartdldr_pi;
class ChartDldrPanelImpl;
class wxDateTime;
class wxTimer;
class wxTimerEvent;
class wxWindow;

struct ChartDldrBulkRunStats;
struct ChartDldrScheduledBulkResult;

enum class ChartDldrScheduledSkipReason {
  None,
  BulkDisabled,
  NoSources,
  PanelUnavailable,
};

enum class ChartDldrBulkStartBlocker {
  None,
  SessionActive,
};

struct ChartDldrBulkRequestInput {
  bool allow_bulk_update = false;
  bool has_chart_sources = false;
  ChartDldrBulkStartBlocker blocker = ChartDldrBulkStartBlocker::None;
};

/** Who initiated a scheduled bulk request (affects skip persistence). */
enum class ChartDldrBulkRequestOrigin {
  Timer,
  Manual,
};

enum class ChartDldrBulkRequestStatus {
  Allowed,
  StaleTimer,
  Denied,
};

struct ChartDldrBulkRequestEligibility {
  ChartDldrBulkRequestStatus status = ChartDldrBulkRequestStatus::Denied;
  ChartDldrScheduledSkipReason skip = ChartDldrScheduledSkipReason::None;

  bool IsAllowed() const {
    return status == ChartDldrBulkRequestStatus::Allowed;
  }
  bool IsStaleTimer() const {
    return status == ChartDldrBulkRequestStatus::StaleTimer;
  }
};

ChartDldrBulkRequestEligibility ChartDldrEvaluateBulkRequest(
    const ChartDldrBulkRequestInput& input, ChartDldrBulkRunMode mode,
    ChartDldrBulkRequestOrigin origin, const ChartDldrScheduleConfig& schedule,
    const wxDateTime& now);

ChartDldrScheduledSkipReason ChartDldrEvaluateScheduledBulkSkip(
    bool allow_bulk_update, bool has_chart_sources);

wxString ChartDldrScheduledSkipStatus(ChartDldrScheduledSkipReason reason);

/** Invalidates deferred timer callbacks after plugin shutdown begins. */
class ChartDldrScheduleCallbackGate {
public:
  ChartDldrScheduleCallbackGate() : token_(NextToken()) {}

  uint64_t Capture() const { return token_; }
  void Invalidate() {
    ++token_;
    if (token_ == 0) {
      ++token_;
    }
  }
  /** Rejects captured 0 so a dead instance's default token never matches. */
  bool IsValid(uint64_t captured) const {
    return captured != 0 && token_ == captured;
  }

private:
  static uint64_t NextToken() {
    static std::atomic<uint64_t> counter{0};
    const uint64_t next = ++counter;
    return next == 0 ? ++counter : next;
  }

  uint64_t token_;
};

void ChartDldrRestartScheduleTimer(chartdldr_pi* pi, wxTimer* timer);
void ChartDldrOnScheduleTimer(chartdldr_pi* pi, wxTimerEvent& event);
bool ChartDldrRequestBulkUpdate(
    chartdldr_pi* pi, ChartDldrBulkRunMode mode,
    ChartDldrBulkRequestOrigin origin = ChartDldrBulkRequestOrigin::Timer);
void ChartDldrFinishScheduledBulkRun(chartdldr_pi* pi,
                                     const ChartDldrBulkRunStats& stats);

/**
 * Persist an explicit outcome, then assign it to live state.
 */
bool ChartDldrCommitScheduledRunOutcome(
    chartdldr_pi* pi, const ChartDldrScheduledBulkResult& result);

/** Persist attempt-start (see schedule_state.h persist policy). */
bool ChartDldrCommitScheduledAttemptStart(chartdldr_pi* pi,
                                          const wxDateTime& run_time);

bool ChartDldrPersistScheduleConfig(chartdldr_pi* pi,
                                    const ChartDldrScheduleConfig& cfg);

/**
 * Owns the schedule-check timer. Once a run is allowed to start,
 * ChartDldrBulkOrchestrate runs it back-to-back like interactive Update All.
 */
class ChartDldrScheduler {
public:
  explicit ChartDldrScheduler(chartdldr_pi* pi);
  ~ChartDldrScheduler();

  void Attach(wxWindow* parent);
  void Detach();
  void Restart();

  void InvalidatePendingCallbacks();
  void QueueDueScheduledBulkRequest();
  bool IsDeferredScheduleCallbackValid(uint64_t token) const {
    return callback_gate_.IsValid(token);
  }

private:
  void OnScheduleCheckTimer(wxTimerEvent& event);

  chartdldr_pi* pi_;
  wxWindow* parent_;
  wxTimer* schedule_timer_;
  ChartDldrScheduleCallbackGate callback_gate_;
};

#endif  // CHARTDLDR_BULK_SCHEDULE_H_
