/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_ORCHESTRATE_H_
#define CHARTDLDR_BULK_ORCHESTRATE_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_async_step.h"

class ChartDldrPanelImpl;
class chartdldr_pi;
class wxCommandEvent;

/** Sole bulk friend of ChartDldrPanelImpl; interactive + scheduled
 * orchestration. */
class ChartDldrBulkOrchestrate {
public:
  explicit ChartDldrBulkOrchestrate(ChartDldrPanelImpl* panel = nullptr);

  bool RunInteractive(ChartDldrBulkRunKind kind, wxCommandEvent& event,
                      ChartDldrBulkRunStats& stats);

  void StartScheduledRun(chartdldr_pi* pi);
  bool StepScheduledRun();
  void CancelScheduledRun();
  bool IsScheduledRunActive() const { return scheduled_active_; }

  /** Manual "Update selected catalog" from the panel UI (sync catalog refresh).
   */
  void RefreshCatalogManual(int catalog_index, wxCommandEvent& event);

private:
  chartdldr_pi* Plugin() const;
  void FinishScheduledRun();
  void CleanupScheduledRun(const ChartDldrBulkModeProfile& profile);
  void AccumulateCatalogStats(ChartDldrBulkRunStats& stats);
  void RunCatalogPassInteractive(int catalog_index,
                                 const ChartDldrBulkModeProfile& profile,
                                 wxCommandEvent& event,
                                 ChartDldrBulkRunStats& stats);
  bool StepScheduledRunCore(ChartDldrScheduledBulkRunState& state,
                            const ChartDldrBulkModeProfile& profile,
                            size_t catalog_count);
  void FinalizeBulkRun(chartdldr_pi* pi,
                       const ChartDldrBulkModeProfile& profile,
                       const ChartDldrBulkRunUiSnapshot& ui_before,
                       const ChartDldrBulkRunStats& stats);
  void ApplyScheduledStepDecision(
      const ChartDldrBulkModeProfile& profile,
      const ChartDldrScheduledBulkStepDecision& decision);
  bool TryStartCatalogRefresh(int catalog_index,
                              const ChartDldrBulkModeProfile& profile,
                              wxCommandEvent* event);
  ChartDldrAsyncCatalogStepResult StepCatalogRefresh(
      const ChartDldrBulkModeProfile& profile);

  ChartDldrPanelImpl* panel_;
  chartdldr_pi* scheduled_pi_ = nullptr;
  ChartDldrScheduledBulkRunState scheduled_state_;
  ChartDldrBulkModeProfile scheduled_profile_;
  ChartDldrBulkRunUiSnapshot scheduled_ui_before_;
  bool scheduled_active_ = false;
};

#endif  // CHARTDLDR_BULK_ORCHESTRATE_H_
