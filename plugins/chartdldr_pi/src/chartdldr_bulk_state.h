/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_STATE_H_
#define CHARTDLDR_BULK_STATE_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_panel_bulk_state.h"
#include "chartdldr_temp_download.h"

#include <wx/string.h>

class chartdldr_pi;

struct ChartDldrChartBulkState {
  int catalog_index = -1;
  int loop_index = 0;
  int pending_index = -1;
  ChartDldrChartUpdateKind pending_kind = ChartDldrChartUpdateKind::None;
  ChartDldrTempDownloadPaths pending_download_paths;
  bool active = false;
  int to_download = 0;
  int downloading = 0;
  int failed = 0;
  int new_downloads = 0;
  int updated_downloads = 0;

  void ResetForCatalogPass() {
    catalog_index = -1;
    loop_index = 0;
    pending_index = -1;
    pending_kind = ChartDldrChartUpdateKind::None;
    pending_download_paths = ChartDldrTempDownloadPaths();
    downloading = 0;
    failed = 0;
    to_download = 0;
    new_downloads = 0;
    updated_downloads = 0;
    active = false;
  }

  ChartDldrBulkRunStats ToStats() const {
    ChartDldrBulkRunStats stats;
    stats.attempted = downloading;
    stats.failed = failed;
    stats.new_downloads = new_downloads;
    stats.updated_downloads = updated_downloads;
    return stats;
  }
};

class ChartDldrBulkRunSession {
public:
  bool IsActive() const { return active_; }
  bool IsScheduled() const {
    return ChartDldrBulkRunModeIsScheduled(policy_.mode);
  }

  chartdldr_pi* Plugin() const { return pi_; }
  const ChartDldrBulkSessionPolicy& Policy() const { return policy_; }
  const ChartDldrBulkRunUiSnapshot& UiBefore() const { return ui_before_; }
  const ChartDldrBulkRunPlan& Plan() const { return policy_.plan; }
  ChartDldrBulkRunMode Mode() const { return policy_.mode; }

  ChartDldrBulkCatalogRunState& CatalogRun() { return catalog_run_; }
  const ChartDldrBulkCatalogRunState& CatalogRun() const {
    return catalog_run_;
  }

  ChartDldrChartBulkState& ChartBulk() { return chart_bulk_; }
  const ChartDldrChartBulkState& ChartBulk() const { return chart_bulk_; }

  void Begin(chartdldr_pi* pi, const ChartDldrBulkSessionPolicy& policy,
             const ChartDldrBulkRunUiSnapshot& ui_before, size_t source_count) {
    End();
    pi_ = pi;
    policy_ = policy;
    ui_before_ = ui_before;
    catalog_run_ = ChartDldrBulkCatalogRunState();
    catalog_run_.catalog_bound = static_cast<int>(source_count);
    chart_bulk_.ResetForCatalogPass();
    active_ = pi != nullptr;
  }

  void End() {
    active_ = false;
    pi_ = nullptr;
    policy_ = ChartDldrBulkSessionPolicy();
    catalog_run_ = ChartDldrBulkCatalogRunState();
    chart_bulk_.ResetForCatalogPass();
  }

private:
  bool active_ = false;
  chartdldr_pi* pi_ = nullptr;
  ChartDldrBulkSessionPolicy policy_;
  ChartDldrBulkRunUiSnapshot ui_before_;
  ChartDldrBulkCatalogRunState catalog_run_;
  ChartDldrChartBulkState chart_bulk_;
};

/** Single derived view of what the session pump should do next. */
enum class ChartDldrBulkSessionActivity {
  Idle,
  PrepareCatalog,
  DownloadCharts,
  WaitTransfer,
};

inline ChartDldrBulkSessionActivity ChartDldrBulkSessionActivityFor(
    const ChartDldrBulkRunSession& session,
    const ChartDldrPanelBulkState& panel_state) {
  if (!session.IsActive()) {
    return ChartDldrBulkSessionActivity::Idle;
  }
  // A pending abort of the current transfer must not park the pump in
  // WaitTransfer: step so the chart engine disposes the in-flight download and
  // resumes the walk instead of blocking until the transfer ends on its own.
  if (panel_state.transfer.IsInProgress() &&
      !panel_state.download_cancel.ShouldAbortCurrentTransfer()) {
    if (ChartDldrGetBulkTransferStuckReason(panel_state.transfer) ==
        ChartDldrBulkTransferStuckReason::None) {
      return ChartDldrBulkSessionActivity::WaitTransfer;
    }
  }
  if (session.CatalogRun().phase == ChartDldrBulkCatalogPhase::PrepareCatalog) {
    return ChartDldrBulkSessionActivity::PrepareCatalog;
  }
  return ChartDldrBulkSessionActivity::DownloadCharts;
}

#endif  // CHARTDLDR_BULK_STATE_H_
