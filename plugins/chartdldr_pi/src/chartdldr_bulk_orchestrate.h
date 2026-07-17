/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_ORCHESTRATE_H_
#define CHARTDLDR_BULK_ORCHESTRATE_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_state.h"
#include "chartdldr_scheduled_run_log.h"

#include <memory>

class ChartDldrPanelBulkCatalogEngine;
class ChartDldrPanelBulkChartEngine;
class ChartDldrPanelImpl;
class chartdldr_pi;

class ChartDldrBulkOrchestrate {
public:
  explicit ChartDldrBulkOrchestrate(ChartDldrPanelImpl& panel);
  ~ChartDldrBulkOrchestrate();

  ChartDldrBulkOrchestrate(const ChartDldrBulkOrchestrate&) = delete;
  ChartDldrBulkOrchestrate& operator=(const ChartDldrBulkOrchestrate&) = delete;

  bool IsRunActive() const { return session_.IsActive(); }
  bool IsScheduledRunActive() const {
    return session_.IsActive() && session_.IsScheduled();
  }

  /**
   * Single entry for all bulk modes. Policy comes from
   * ChartDldrBulkSessionPolicyFor(mode, ...). SelectedCharts downloads the
   * currently selected source only; other modes walk configured catalogs.
   */
  bool StartBulk(ChartDldrBulkRunMode mode);

  void TeardownBulkSession(ChartDldrBulkTeardownReason reason,
                           ChartDldrBulkRunStats* out_stats = nullptr);

  bool RefreshCatalogManual(int catalog_index);

  /** Schedule a deferred PumpBulkRun (avoids re-entrancy from transfer END). */
  void ScheduleBulkPump();
  /**
   * Step while idle; return when a transfer is in progress or the walk ends.
   * Returns true if the session is still active and may need another pump.
   */
  bool PumpBulkRun();
  /** Heartbeat / UI tick while a transfer is in progress (stall timer). */
  void OnTransferProgressTick();
  /** Stall-timer / END path: abort stuck transfers then schedule a pump. */
  void OnTransferStallTick();

  /** Download-button cancel / abort during an active bulk session. */
  void RequestBulkUserCancel();

private:
  struct SessionEndContext {
    chartdldr_pi* pi = nullptr;
    ChartDldrBulkSessionPolicy policy;
    ChartDldrBulkRunUiSnapshot ui_before;
    ChartDldrBulkRunStats stats;
    bool was_scheduled = false;
    ChartDldrBulkPostflight postflight;
  };

  bool TryStartBulkSession(chartdldr_pi* pi,
                           const ChartDldrBulkSessionPolicy& policy);
  ChartDldrBulkWalkStep StepChartDownloadPass(int catalog_index);

  bool StepBulkRunOnce();
  /** Bound the shared walker to one catalog and skip into DownloadChart. */
  bool BindSelectedCatalogDownload(int catalog_index);
  void CleanupActiveBulkRun();
  void ApplyChartDatabaseAfterComplete(chartdldr_pi* pi);
  void FinalizeBulkRun(const ChartDldrBulkSessionPolicy& policy,
                       const ChartDldrBulkRunUiSnapshot& ui_before,
                       const ChartDldrBulkRunStats& stats, bool was_scheduled);
  void FinishScheduledAbort(chartdldr_pi* pi);
  void ApplySessionEnd(const ChartDldrBulkSessionEnd& end,
                       const SessionEndContext& ctx,
                       ChartDldrBulkRunStats* out_stats);
  ChartDldrScheduledRunLogSnapshot CaptureScheduledLogSnapshot(
      chartdldr_pi* pi) const;
  void EnsureIdleCatalogUi();
  void EnsureTransferStallTimer(bool running);

  ChartDldrPanelImpl& panel_;
  std::unique_ptr<ChartDldrPanelBulkCatalogEngine> catalog_;
  std::unique_ptr<ChartDldrPanelBulkChartEngine> chart_;
  ChartDldrBulkRunSession session_;
  ChartDldrScheduledRunLogger scheduled_log_;
  bool pump_reentrant_ = false;
};

#endif  // CHARTDLDR_BULK_ORCHESTRATE_H_
