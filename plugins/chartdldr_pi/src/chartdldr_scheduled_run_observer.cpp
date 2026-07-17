/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_scheduled_run_observer.h"

#include "chartdldr_bulk_state.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_pi.h"
#include "chartcatalog.h"

#include <wx/log.h>

namespace {

wxString ResolveCatalogName(chartdldr_pi* pi, int catalog_index) {
  if (!pi || !ChartDldrBulkCatalogIndexInRange(catalog_index,
                                               pi->m_ChartSources.size())) {
    return wxString();
  }
  return pi->m_ChartSources.at(static_cast<size_t>(catalog_index))->GetName();
}

wxString ResolveChartTitle(chartdldr_pi* pi, int chart_index) {
  if (!pi || chart_index < 0 ||
      chart_index >= static_cast<int>(pi->m_pChartCatalog.charts.size())) {
    return wxString();
  }
  return pi->m_pChartCatalog.charts.at(static_cast<size_t>(chart_index))
      ->GetChartTitle();
}

ChartDldrScheduledRunLogSnapshot CaptureSnapshot(
    const ChartDldrBulkRunSession& session, chartdldr_pi* pi) {
  ChartDldrScheduledRunLogSnapshot snap;
  const ChartDldrBulkCatalogRunState& catalog = session.CatalogRun();
  const ChartDldrChartBulkState& charts = session.ChartBulk();
  snap.catalog_index = catalog.next_catalog;
  snap.catalog_count = pi ? static_cast<int>(pi->m_ChartSources.size()) : 0;
  snap.phase = catalog.phase;
  snap.catalog_refresh_active = session.CatalogRefreshInProgress();
  snap.charts_downloading = charts.downloading;
  snap.charts_to_download = charts.to_download;
  snap.charts_failed = charts.failed;
  snap.pending_chart_index = charts.pending_index;
  snap.catalog_name = ResolveCatalogName(pi, catalog.next_catalog);
  snap.chart_title = ResolveChartTitle(pi, charts.pending_index);
  return snap;
}

}  // namespace

void ChartDldrScheduledRunObserver::OnStarting(int source_count) {
  active_ = true;
  logger_.OnStarting();
  wxLogMessage("%s", ChartDldrFormatScheduledStarting(source_count).mb_str());
}

void ChartDldrScheduledRunObserver::Reset() {
  active_ = false;
  logger_.Reset();
}

ChartDldrScheduledStepObservation ChartDldrScheduledRunObserver::BeginStep(
    const ChartDldrBulkRunSession& session) const {
  ChartDldrScheduledStepObservation obs;
  if (!active_) {
    return obs;
  }
  obs.phase_before = session.CatalogRun().phase;
  obs.catalog_index_before = session.CatalogRun().next_catalog;
  obs.charts_downloading_before = session.ChartBulk().downloading;
  return obs;
}

void ChartDldrScheduledRunObserver::OnStep(
    const ChartDldrScheduledStepObservation& before,
    const ChartDldrBulkRunSession& session, chartdldr_pi* pi,
    ChartDldrBulkWalkStep walk_step, int charts_selected,
    const ChartDldrBulkRunStats& catalog_counters) {
  if (!active_) {
    return;
  }
  const ChartDldrChartBulkState& charts = session.ChartBulk();

  ChartDldrScheduledBulkStepLogInput step;
  step.phase_before = before.phase_before;
  step.catalog_index_before = before.catalog_index_before;
  step.catalog_count = pi ? static_cast<int>(pi->m_ChartSources.size()) : 0;
  step.charts_downloading_before = before.charts_downloading_before;
  step.catalog_name = ResolveCatalogName(pi, before.catalog_index_before);
  step.charts_downloading = charts.downloading;
  step.charts_to_download = charts.to_download;
  step.pending_chart_index = charts.pending_index;
  step.chart_title = ResolveChartTitle(pi, charts.pending_index);

  logger_.OnWalkStep(step, walk_step, charts_selected, catalog_counters,
                     CaptureSnapshot(session, pi));
}

void ChartDldrScheduledRunObserver::OnStillRunning(
    const ChartDldrBulkRunSession& session, chartdldr_pi* pi) {
  if (!active_) {
    return;
  }
  const wxString line = logger_.NoteStillRunning(CaptureSnapshot(session, pi));
  if (!line.empty()) {
    wxLogMessage("%s", line.mb_str());
  }
}
