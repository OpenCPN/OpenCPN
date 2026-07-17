/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_ORCHESTRATE_H_
#define CHARTDLDR_BULK_ORCHESTRATE_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_panel_port.h"
#include "chartdldr_bulk_state.h"
#include "chartdldr_bulk_pump.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_scheduled_run_observer.h"

#include <memory>

class ChartDldrBulkCatalogController;
class ChartDldrBulkChartController;
class chartdldr_pi;

class ChartDldrBulkOrchestrate : private ChartDldrBulkTransferEvents {
public:
  /** Registers as the run session's transfer-events listener until dtor. */
  explicit ChartDldrBulkOrchestrate(ChartDldrBulkPanelPort& port);
  ~ChartDldrBulkOrchestrate() override;

  ChartDldrBulkOrchestrate(const ChartDldrBulkOrchestrate&) = delete;
  ChartDldrBulkOrchestrate& operator=(const ChartDldrBulkOrchestrate&) = delete;

  bool IsRunActive() const { return session_.IsActive(); }
  bool IsScheduledRunActive() const {
    return session_.IsActive() && session_.IsScheduled();
  }

  // The run session is the single owner of transfer/cancel/refresh state. The
  // panel reads its download-cancel UI state through here; between runs the
  // session is inactive and its cancel state is idle.
  ChartDldrBulkRunSession& Session() { return session_; }
  const ChartDldrBulkRunSession& Session() const { return session_; }

  ChartDldrBulkPump& Pump() { return pump_; }
  const ChartDldrBulkPump& Pump() const { return pump_; }

  /**
   * Single entry for all bulk modes. Policy comes from
   * ChartDldrBulkSessionPolicyFor(mode, ...). SelectedCharts and
   * CatalogRefresh bind the currently selected source; other modes walk all
   * configured catalogs.
   */
  bool StartBulk(ChartDldrBulkRunMode mode);

  void TeardownBulkSession(ChartDldrBulkTeardownReason reason,
                           ChartDldrBulkRunStats* out_stats = nullptr);

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
    ChartDldrBulkPostflight postflight;
  };

  // ChartDldrBulkTransferEvents: live download sinks feed the pump.
  void OnBulkTransferProgress() override;
  void OnBulkTransferEnd() override;

  bool TryStartBulkSession(chartdldr_pi* pi,
                           const ChartDldrBulkSessionPolicy& policy);
  ChartDldrBulkWalkStep StepChartDownloadPass(int catalog_index);

  bool StepBulkRunOnce();
  /**
   * Bound the shared walker to the selected catalog; the phase (prepare-only
   * vs download) comes from the session policy.
   */
  bool BindSingleCatalog(int catalog_index);
  void CleanupActiveBulkRun();
  void ApplyChartDatabaseAfterComplete(chartdldr_pi* pi);
  void FinalizeBulkRun(const ChartDldrBulkSessionPolicy& policy,
                       const ChartDldrBulkRunUiSnapshot& ui_before,
                       const ChartDldrBulkRunStats& stats);
  void FinishScheduledAbort(chartdldr_pi* pi);
  void ApplySessionEnd(const ChartDldrBulkSessionEnd& end,
                       const SessionEndContext& ctx,
                       ChartDldrBulkRunStats* out_stats);
  void EnsureIdleCatalogUi();

  ChartDldrBulkPanelPort& port_;
  // session_ before pump_/controllers so their references bind to a live
  // subobject; the controllers drive transfer/cancel/refresh through it.
  ChartDldrBulkRunSession session_;
  ChartDldrBulkPump pump_;
  std::unique_ptr<ChartDldrBulkCatalogController> catalog_;
  std::unique_ptr<ChartDldrBulkChartController> chart_;
  ChartDldrScheduledRunObserver scheduled_run_;
  bool pump_reentrant_ = false;
};

#endif  // CHARTDLDR_BULK_ORCHESTRATE_H_
