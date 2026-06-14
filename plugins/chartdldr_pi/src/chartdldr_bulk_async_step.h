/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_ASYNC_STEP_H_
#define CHARTDLDR_BULK_ASYNC_STEP_H_

#include "chartdldr_bulk.h"

#include <cstddef>

enum class ChartDldrScheduledBulkPhase {
  SelectCatalog,
  RefreshCatalog,
  DownloadChart,
};

struct ChartDldrScheduledBulkStepInput {
  ChartDldrScheduledBulkPhase phase =
      ChartDldrScheduledBulkPhase::SelectCatalog;
  size_t next_catalog = 0;
  size_t catalog_count = 0;
  bool catalog_refresh_started = false;
  ChartDldrAsyncCatalogStepResult catalog_step =
      ChartDldrAsyncCatalogStepResult::NotActive;
  ChartDldrBulkChartStepResult chart_step =
      ChartDldrBulkChartStepResult::NotActive;
  int charts_selected_for_download = 0;
};

struct ChartDldrScheduledBulkStepDecision {
  ChartDldrScheduledBulkPhase next_phase =
      ChartDldrScheduledBulkPhase::SelectCatalog;
  size_t next_catalog = 0;
  bool continue_run = false;
  bool begin_chart_download = false;
  bool end_chart_download = false;
};

struct ChartDldrScheduledBulkRunState {
  ChartDldrScheduledBulkPhase phase =
      ChartDldrScheduledBulkPhase::SelectCatalog;
  size_t next_catalog = 0;
  ChartDldrBulkRunStats stats;
};

ChartDldrScheduledBulkStepDecision ChartDldrDecideScheduledBulkStep(
    const ChartDldrScheduledBulkStepInput& input);

/** Applies decision to run state; returns the same decision for panel actions.
 */
ChartDldrScheduledBulkStepDecision ChartDldrAdvanceScheduledBulkRun(
    ChartDldrScheduledBulkRunState& state,
    const ChartDldrScheduledBulkStepInput& input,
    const ChartDldrBulkRunStats& catalog_stats);

#endif  // CHARTDLDR_BULK_ASYNC_STEP_H_
