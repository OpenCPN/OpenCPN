/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_catalog_run.h"

namespace {

bool AdvanceToNextCatalog(ChartDldrBulkCatalogRunState& state) {
  state.phase = ChartDldrBulkCatalogPhase::PrepareCatalog;
  state.next_catalog += 1;
  return ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                           state.catalog_bound);
}

bool AfterCatalogReady(ChartDldrBulkCatalogRunState& state,
                       int charts_selected) {
  if (charts_selected <= 0) {
    return AdvanceToNextCatalog(state);
  }
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  return true;
}

}  // namespace

bool ChartDldrAdvanceBulkWalk(ChartDldrBulkCatalogRunState& state,
                              ChartDldrBulkWalkStep step, int charts_selected,
                              const ChartDldrBulkRunStats& catalog_stats) {
  if (!ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                         state.catalog_bound)) {
    return false;
  }

  switch (step) {
    case ChartDldrBulkWalkStep::NotActive:
      return false;

    case ChartDldrBulkWalkStep::TransferWait:
    case ChartDldrBulkWalkStep::MoreWork:
      return true;

    case ChartDldrBulkWalkStep::CatalogReady:
      return AfterCatalogReady(state, charts_selected);

    case ChartDldrBulkWalkStep::Advance:
      state.stats.Accumulate(catalog_stats);
      return AdvanceToNextCatalog(state);
  }

  return false;
}

bool ChartDldrBindCatalogWalk(ChartDldrBulkCatalogRunState& state,
                              int catalog_index) {
  if (catalog_index < 0) {
    return false;
  }
  state.next_catalog = catalog_index;
  state.catalog_bound = catalog_index + 1;
  state.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  return ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                           state.catalog_bound);
}
