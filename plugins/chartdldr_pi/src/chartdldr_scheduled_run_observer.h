/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_SCHEDULED_RUN_OBSERVER_H_
#define CHARTDLDR_SCHEDULED_RUN_OBSERVER_H_

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_scheduled_run_log.h"

class ChartDldrBulkRunSession;
class chartdldr_pi;

/** Pre-step scheduled state captured before a walk step advances the session.
 */
struct ChartDldrScheduledStepObservation {
  ChartDldrBulkCatalogPhase phase_before =
      ChartDldrBulkCatalogPhase::PrepareCatalog;
  int catalog_index_before = 0;
  int charts_downloading_before = 0;
};

/**
 * Owns scheduled-run operational logging so the orchestrator's walk kernel
 * stays free of log scaffolding. Inactive until OnStarting; BeginStep / OnStep
 * / OnStillRunning are cheap no-ops for interactive runs.
 */
class ChartDldrScheduledRunObserver {
public:
  void OnStarting(int source_count);
  void Reset();

  /** Cheap snapshot of the pre-step state used to detect milestone edges. */
  ChartDldrScheduledStepObservation BeginStep(
      const ChartDldrBulkRunSession& session) const;

  /** Emit phase/chart transition milestones + still-running for one step. */
  void OnStep(const ChartDldrScheduledStepObservation& before,
              const ChartDldrBulkRunSession& session, chartdldr_pi* pi,
              ChartDldrBulkWalkStep walk_step, int charts_selected,
              const ChartDldrBulkRunStats& catalog_counters);

  /** Heartbeat while a transfer is in progress: emit still-running if due. */
  void OnStillRunning(const ChartDldrBulkRunSession& session, chartdldr_pi* pi);

private:
  ChartDldrScheduledRunLogger logger_;
  bool active_ = false;
};

#endif  // CHARTDLDR_SCHEDULED_RUN_OBSERVER_H_
