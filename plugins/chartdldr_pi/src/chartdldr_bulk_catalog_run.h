/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_CATALOG_RUN_H_
#define CHARTDLDR_BULK_CATALOG_RUN_H_

#include "chartdldr_bulk_lifecycle.h"

#include <cstddef>

/**
 * One walker step from catalog or chart engines. AdvanceBulkWalk consumes this;
 * engines never mutate the walk state themselves.
 */
enum class ChartDldrBulkWalkStep {
  NotActive,
  /** Transfer in flight — pump yields. */
  TransferWait,
  /** Stay in the current phase (more charts, or refresh started). */
  MoreWork,
  /** Catalog refresh succeeded with a selection — enter DownloadChart. */
  CatalogReady,
  /** Finish this catalog and move to the next (fail / empty / pass done). */
  Advance,
};

enum class ChartDldrChartUpdateKind { None, New, Updated };

/** How to dispose a chart-bulk transfer slot. */
enum class ChartDldrChartTransferDisposition {
  None,
  Settle,
  Abort,
};

inline ChartDldrChartTransferDisposition
ChartDldrResolveChartTransferDisposition(bool owned_by_chart, bool in_progress,
                                         bool success, bool cancelled) {
  if (!owned_by_chart) {
    return ChartDldrChartTransferDisposition::None;
  }
  if (!in_progress && success && !cancelled) {
    return ChartDldrChartTransferDisposition::Settle;
  }
  return ChartDldrChartTransferDisposition::Abort;
}

inline bool ChartDldrBulkWalkStepNeedsPassClose(ChartDldrBulkWalkStep step) {
  return step == ChartDldrBulkWalkStep::Advance;
}

enum class ChartDldrBulkCatalogPhase {
  PrepareCatalog,
  DownloadChart,
};

struct ChartDldrBulkCatalogRunState {
  ChartDldrBulkCatalogPhase phase = ChartDldrBulkCatalogPhase::PrepareCatalog;
  int next_catalog = 0;
  /** Exclusive walk end; set to source_count (or selected+1) at session start.
   */
  int catalog_bound = 0;
  ChartDldrBulkRunStats stats;
};

inline bool ChartDldrBulkCatalogIndexInRange(int catalog_index,
                                             size_t catalog_count) {
  return catalog_index >= 0 && catalog_index < static_cast<int>(catalog_count);
}

/** Whether next_catalog is still inside the half-open walk [0, catalog_bound).
 */
inline bool ChartDldrBulkCatalogWalkContinues(int next_catalog,
                                              int catalog_bound) {
  return next_catalog >= 0 && next_catalog < catalog_bound;
}

/**
 * Advance the catalog walk for one engine step. charts_selected is used when
 * step is CatalogReady. catalog_stats accumulate on Advance from DownloadChart.
 */
bool ChartDldrAdvanceBulkWalk(
    ChartDldrBulkCatalogRunState& state, ChartDldrBulkWalkStep step,
    int charts_selected = 0,
    const ChartDldrBulkRunStats& catalog_stats = ChartDldrBulkRunStats());

/**
 * Bind the walk to a single catalog and skip prepare (SelectedCharts).
 * The only production writer of phase/bound besides AdvanceBulkWalk.
 */
bool ChartDldrBindCatalogWalk(ChartDldrBulkCatalogRunState& state,
                              int catalog_index);

#endif  // CHARTDLDR_BULK_CATALOG_RUN_H_
