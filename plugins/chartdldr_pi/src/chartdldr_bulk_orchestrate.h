/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_ORCHESTRATE_H_
#define CHARTDLDR_BULK_ORCHESTRATE_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_async_step.h"
#include "chartdldr_bulk_state.h"

class ChartDldrPanelImpl;
class wxCommandEvent;

/** Interactive + scheduled bulk orchestration. */
class ChartDldrBulkOrchestrate {
public:
  explicit ChartDldrBulkOrchestrate(ChartDldrPanelImpl& panel);

  bool IsRunActive() const { return session_.IsActive(); }
  bool IsScheduledRunActive() const {
    return session_.IsActive() && session_.IsScheduled();
  }
  bool ShouldYieldOnDownloadEvent() const {
    return session_.WouldYieldOnDownloadEvent();
  }

  bool RunInteractive(ChartDldrBulkRunKind kind, wxCommandEvent& event,
                      ChartDldrBulkRunStats& stats);
  bool RunSelectedChartsDownload();

  void StartScheduledRun(chartdldr_pi* pi);
  bool StepScheduledRun();
  void CancelScheduledRun();

  void RefreshCatalogManual(int catalog_index, wxCommandEvent& event);

private:
  void FinishScheduledRun();
  void CleanupScheduledRun();
  bool StepScheduledRunCore();
  void FinalizeBulkRun(chartdldr_pi* pi,
                       const ChartDldrBulkModeProfile& profile,
                       const ChartDldrBulkRunUiSnapshot& ui_before,
                       const ChartDldrBulkRunStats& stats);
  void ApplyScheduledStepDecision(
      const ChartDldrScheduledBulkStepDecision& decision);

  ChartDldrPanelImpl& panel_;
  ChartDldrBulkRunSession session_;
};

#endif  // CHARTDLDR_BULK_ORCHESTRATE_H_
