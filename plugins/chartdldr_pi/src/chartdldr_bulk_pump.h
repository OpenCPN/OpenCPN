/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_PUMP_H_
#define CHARTDLDR_BULK_PUMP_H_

#include <cstdint>
#include <memory>

class ChartDldrBulkOrchestrate;
class wxEvtHandler;
class wxTimer;
class wxTimerEvent;

/**
 * Owns the deferred bulk-run pump (CallAfter epoch fencing + transfer stall
 * timer). Lives with the orchestrator so the panel is not the pump runtime.
 */
class ChartDldrBulkPump {
public:
  ChartDldrBulkPump(ChartDldrBulkOrchestrate& orchestrate,
                    wxEvtHandler& handler);
  ~ChartDldrBulkPump();

  ChartDldrBulkPump(const ChartDldrBulkPump&) = delete;
  ChartDldrBulkPump& operator=(const ChartDldrBulkPump&) = delete;

  void Schedule();
  void InvalidatePending();
  void SetTransferStallTimerRunning(bool running);

private:
  friend class ChartDldrBulkPumpGuard;

  void Enter();
  void Leave();
  void OnTransferStallTimer(wxTimerEvent& event);

  ChartDldrBulkOrchestrate& orchestrate_;
  wxEvtHandler& handler_;
  uint64_t epoch_ = 0;
  std::unique_ptr<wxTimer> stall_timer_;
};

/** RAII: marks modal error dialogs suppressed for nested CallAfter pump work.
 */
class ChartDldrBulkPumpGuard {
public:
  explicit ChartDldrBulkPumpGuard(ChartDldrBulkPump& pump);
  ~ChartDldrBulkPumpGuard();

  ChartDldrBulkPumpGuard(const ChartDldrBulkPumpGuard&) = delete;
  ChartDldrBulkPumpGuard& operator=(const ChartDldrBulkPumpGuard&) = delete;

private:
  ChartDldrBulkPump& pump_;
};

#endif  // CHARTDLDR_BULK_PUMP_H_
