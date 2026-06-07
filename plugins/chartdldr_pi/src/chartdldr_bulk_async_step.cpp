/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_async_step.h"

#include "chartdldr_bulk.h"

ChartDldrScheduledBulkStepDecision ChartDldrDecideScheduledBulkStep(
    const ChartDldrScheduledBulkStepInput& input) {
  ChartDldrScheduledBulkStepDecision decision;
  decision.next_catalog = input.next_catalog;
  decision.next_phase = input.phase;

  if (input.next_catalog >= input.catalog_count) {
    return decision;
  }

  switch (input.phase) {
    case ChartDldrScheduledBulkPhase::SelectCatalog:
      if (input.catalog_refresh_started) {
        decision.next_phase = ChartDldrScheduledBulkPhase::RefreshCatalog;
        decision.continue_run = true;
        return decision;
      }
      decision.next_phase = ChartDldrScheduledBulkPhase::SelectCatalog;
      decision.next_catalog = input.next_catalog + 1;
      decision.continue_run = decision.next_catalog < input.catalog_count;
      return decision;

    case ChartDldrScheduledBulkPhase::RefreshCatalog:
      if (input.catalog_step == ChartDldrAsyncCatalogStepResult::InProgress) {
        decision.continue_run = true;
        return decision;
      }
      if (input.catalog_step == ChartDldrAsyncCatalogStepResult::Failed) {
        decision.next_phase = ChartDldrScheduledBulkPhase::SelectCatalog;
        decision.next_catalog = input.next_catalog + 1;
        decision.continue_run = decision.next_catalog < input.catalog_count;
        return decision;
      }
      decision.next_phase = ChartDldrScheduledBulkPhase::DownloadChart;
      decision.begin_chart_download = true;
      decision.continue_run = true;
      return decision;

    case ChartDldrScheduledBulkPhase::DownloadChart:
      switch (input.chart_step) {
        case ChartDldrBulkChartStepResult::TransferInProgress:
        case ChartDldrBulkChartStepResult::MoreCharts:
          decision.continue_run = true;
          return decision;
        case ChartDldrBulkChartStepResult::NotActive:
        case ChartDldrBulkChartStepResult::Finished:
          decision.end_chart_download = true;
          decision.accumulate_stats = true;
          decision.next_phase = ChartDldrScheduledBulkPhase::SelectCatalog;
          decision.next_catalog = input.next_catalog + 1;
          decision.continue_run =
              decision.next_catalog < input.catalog_count;
          return decision;
      }
      break;
  }

  return decision;
}

ChartDldrScheduledBulkStepDecision ChartDldrAdvanceScheduledBulkRun(
    ChartDldrScheduledBulkRunState& state,
    const ChartDldrScheduledBulkStepInput& input,
    const ChartDldrBulkRunStats& catalog_stats) {
  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrDecideScheduledBulkStep(input);

  if (decision.accumulate_stats) {
    state.stats.Accumulate(catalog_stats);
  }

  state.phase = decision.next_phase;
  state.next_catalog = decision.next_catalog;
  return decision;
}
