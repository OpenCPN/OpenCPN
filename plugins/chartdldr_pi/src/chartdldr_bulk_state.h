/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_STATE_H_
#define CHARTDLDR_BULK_STATE_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_download_cancel.h"
#include "chartdldr_temp_download.h"

#include <wx/string.h>

#include <vector>

class chartdldr_pi;

/**
 * Catalog-refresh payload carried by the active run session. "In progress" is
 * derived from the transfer owner (ChartDldrBulkTransferOwner::Catalog); these
 * fields are meaningful only while that transfer is live.
 */
struct ChartDldrCatalogRefreshPayload {
  int index = -1;
  ChartDldrTempDownloadPaths download_paths;
  ChartDldrErrorReporting error_reporting = ChartDldrErrorReporting::Dialog;
};

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
  bool IsScheduled() const { return policy_.scheduled; }

  chartdldr_pi* Plugin() const { return pi_; }
  const ChartDldrBulkSessionPolicy& Policy() const { return policy_; }
  const ChartDldrBulkRunUiSnapshot& UiBefore() const { return ui_before_; }
  ChartDldrBulkRunMode Mode() const { return policy_.mode; }

  /** Manual-URL charts discovered during the walk (collect_manual_urls);
   * offered to the user once after a successful run. */
  std::vector<ChartDldrManualDownloadAction>& ManualActions() {
    return manual_actions_;
  }
  const std::vector<ChartDldrManualDownloadAction>& ManualActions() const {
    return manual_actions_;
  }

  ChartDldrBulkCatalogRunState& CatalogRun() { return catalog_run_; }
  const ChartDldrBulkCatalogRunState& CatalogRun() const {
    return catalog_run_;
  }

  ChartDldrChartBulkState& ChartBulk() { return chart_bulk_; }
  const ChartDldrChartBulkState& ChartBulk() const { return chart_bulk_; }

  // The single owner of the in-flight transfer, the download-button cancel
  // state, and the catalog-refresh payload for the whole run. Controllers and
  // the panel operate on these through the session rather than a parallel copy.
  ChartDldrBulkTransferSlot& Transfer() { return transfer_; }
  const ChartDldrBulkTransferSlot& Transfer() const { return transfer_; }
  ChartDldrDownloadCancelState& DownloadCancel() { return download_cancel_; }
  const ChartDldrDownloadCancelState& DownloadCancel() const {
    return download_cancel_;
  }
  ChartDldrCatalogRefreshPayload& CatalogRefresh() { return catalog_refresh_; }
  const ChartDldrCatalogRefreshPayload& CatalogRefresh() const {
    return catalog_refresh_;
  }

  // Pump/UI listener injected into the live download sinks the controllers
  // start through this session. Set once by the orchestrator; survives Begin/
  // End so late events always reach the (still-registered) owner.
  void SetTransferEvents(ChartDldrBulkTransferEvents* events) {
    transfer_events_ = events;
  }
  ChartDldrBulkTransferEvents* TransferEvents() const {
    return transfer_events_;
  }
  /** A catalog refresh is in progress iff the transfer slot is Catalog-owned.
   */
  bool CatalogRefreshInProgress() const {
    return transfer_.IsOwnedBy(ChartDldrBulkTransferOwner::Catalog);
  }

  bool Begin(chartdldr_pi* pi, const ChartDldrBulkSessionPolicy& policy,
             const ChartDldrBulkRunUiSnapshot& ui_before, size_t source_count) {
    End();
    pi_ = pi;
    policy_ = policy;
    ui_before_ = ui_before;
    catalog_run_ = ChartDldrBulkCatalogRunState();
    catalog_run_.catalog_bound = static_cast<int>(source_count);
    chart_bulk_.ResetForCatalogPass();
    // The session owns cancel state: a beginning run is session-cancellable.
    download_cancel_.ResetForBulkRun();
    download_cancel_.BeginBulkSessionCancel();
    active_ = pi != nullptr;
    return active_;
  }

  void End() {
    active_ = false;
    pi_ = nullptr;
    policy_ = ChartDldrBulkSessionPolicy();
    catalog_run_ = ChartDldrBulkCatalogRunState();
    chart_bulk_.ResetForCatalogPass();
    manual_actions_.clear();
    // Transfers are disposed by the orchestrator/controllers before End();
    // Reset keeps the retained sinks + generation so any late download event is
    // fenced rather than delivered to a freed slot.
    transfer_.Reset();
    download_cancel_ = ChartDldrDownloadCancelState();
    catalog_refresh_ = ChartDldrCatalogRefreshPayload();
  }

private:
  bool active_ = false;
  chartdldr_pi* pi_ = nullptr;
  ChartDldrBulkSessionPolicy policy_;
  ChartDldrBulkRunUiSnapshot ui_before_;
  ChartDldrBulkCatalogRunState catalog_run_;
  ChartDldrChartBulkState chart_bulk_;
  ChartDldrBulkTransferSlot transfer_;
  ChartDldrDownloadCancelState download_cancel_;
  ChartDldrCatalogRefreshPayload catalog_refresh_;
  std::vector<ChartDldrManualDownloadAction> manual_actions_;
  ChartDldrBulkTransferEvents* transfer_events_ = nullptr;
};

/** Single derived view of what the session pump should do next. */
enum class ChartDldrBulkSessionActivity {
  Idle,
  PrepareCatalog,
  DownloadCharts,
  WaitTransfer,
};

inline ChartDldrBulkSessionActivity ChartDldrBulkSessionActivityFor(
    const ChartDldrBulkRunSession& session) {
  if (!session.IsActive()) {
    return ChartDldrBulkSessionActivity::Idle;
  }
  // A pending abort of the current transfer must not park the pump in
  // WaitTransfer: step so the chart controller disposes the in-flight download
  // and resumes the walk instead of blocking until the transfer ends on its
  // own.
  if (session.Transfer().IsInProgress() &&
      !session.DownloadCancel().ShouldAbortCurrentTransfer()) {
    if (session.Transfer().StuckReason(wxDateTime::Now()) ==
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
