/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_bulk_catalog_engine.h"

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_download.h"
#include "chartdldr_catalog_prep.h"
#include "chartcatalog.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/url.h>

ChartDldrPanelBulkCatalogEngine::ChartDldrPanelBulkCatalogEngine(
    ChartDldrPanelImpl& panel)
    : panel_(panel), state_(panel.BulkState()) {}

void ChartDldrPanelBulkCatalogEngine::ReloadCatalogFromDisk(
    int catalog_index, const ChartDldrCatalogUiPolicy& ui) {
  chartdldr_pi* const pi = panel_.GetPlugin();
  if (!pi || !ChartDldrBulkCatalogIndexInRange(catalog_index,
                                               pi->m_ChartSources.size())) {
    return;
  }

  ChartSource& cs = *pi->m_ChartSources.at(static_cast<size_t>(catalog_index));
  panel_.ReloadCatalogChartList(cs, ui.preselect_new, ui.preselect_updated,
                                ui.materialize);

  panel_.UpdateCatalogListRow(
      catalog_index, pi->m_pChartCatalog.title,
      pi->m_pChartCatalog.GetReleaseDate().Format(_T("%Y-%m-%d %H:%M")),
      cs.GetDir());
}

void ChartDldrPanelBulkCatalogEngine::PrepareBulkCatalog(
    int catalog_index, const ChartDldrCatalogUiPolicy& ui) {
  chartdldr_pi* const pi = panel_.GetPlugin();
  if (!pi || !ChartDldrBulkCatalogIndexInRange(catalog_index,
                                               pi->m_ChartSources.size())) {
    return;
  }

  if (ui.materialize) {
    panel_.UpdateChartsLabelForSource(
        *pi->m_ChartSources.at(static_cast<size_t>(catalog_index)));
  }

  if (ui.preserve_selection) {
    panel_.CaptureChartListSelectionFromWidgets();
  } else {
    ReloadCatalogFromDisk(catalog_index, ui);
  }
}

void ChartDldrPanelBulkCatalogEngine::ApplyCatalogDownloadOutcome(
    int catalog_index, const ChartDldrCatalogUiPolicy& ui,
    ChartDldrErrorReporting reporting, _OCPN_DLStatus status,
    const wxString& url) {
  const ChartDldrCatalogDownloadOutcome outcome =
      ChartDldrCatalogDownloadOutcomeFor(status, url);

  if (outcome.reload_catalog) {
    ReloadCatalogFromDisk(catalog_index, ui);
    if (ui.focus_charts_after) {
      panel_.FocusChartsDownloadTab();
    }
  }
  if (outcome.report_failure) {
    ChartDldrReportBulkError(panel_.AsWindow(), reporting,
                             outcome.failure_message, _("Chart Downloader"));
  }
}

_OCPN_DLStatus ChartDldrPanelBulkCatalogEngine::DownloadCatalogFileModal(
    const wxString& url, const wxString& output_path) {
  wxURI uri(url);
  const wxString message = _("Reading Headers: ") + uri.BuildURI();
  const int dialog_flags = OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
                           OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED |
                           OCPN_DLDS_SIZE | OCPN_DLDS_URL |
                           OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT |
                           OCPN_DLDS_AUTO_CLOSE;

  const ChartDldrTempDownloadPaths paths =
      ChartDldrTempDownloadPathsFor(output_path);
  const _OCPN_DLStatus ret = OCPN_downloadFile(
      url, paths.download_target, _("Downloading file"), message, wxNullBitmap,
      panel_.AsWindow(), dialog_flags, 10);
  return ChartDldrCompleteValidatedCatalogDownloadPaths(
      paths, ret == OCPN_DL_NO_ERROR, ret == OCPN_DL_ABORTED);
}

bool ChartDldrPanelBulkCatalogEngine::UpdateChartListForCatalog(
    int catalog_index, const ChartDldrCatalogUiPolicy& ui) {
  chartdldr_pi* const pi = panel_.GetPlugin();
  wxURI url;
  wxFileName output_fn;
  if (!ChartDldrPrepareCatalogDownloadPaths(pi, catalog_index, url,
                                            output_fn)) {
    ChartDldrReportCatalogPreparePathFailure(
        panel_.AsWindow(), ChartDldrErrorReporting::Dialog, url, output_fn);
    return false;
  }

  const _OCPN_DLStatus ret =
      DownloadCatalogFileModal(url.BuildURI(), output_fn.GetFullPath());
  ApplyCatalogDownloadOutcome(
      catalog_index, ui, ChartDldrErrorReporting::Dialog, ret, url.BuildURI());
  return ret == OCPN_DL_NO_ERROR;
}

bool ChartDldrPanelBulkCatalogEngine::BeginCatalogRefresh(
    int catalog_index, ChartDldrErrorReporting error_reporting) {
  CancelCatalogRefresh();

  // Chart bulk owns the transfer slot; do not start a catalog refresh over it.
  if (state_.transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    return false;
  }

  chartdldr_pi* const pi = panel_.GetPlugin();
  wxURI url;
  wxFileName output_fn;
  if (!ChartDldrPrepareCatalogDownloadPaths(pi, catalog_index, url,
                                            output_fn)) {
    ChartDldrReportCatalogPreparePathFailure(panel_.AsWindow(), error_reporting,
                                             url, output_fn);
    return false;
  }

  ChartDldrCatalogRefreshState& async = state_.catalog_refresh;
  async.index = catalog_index;
  async.error_reporting = error_reporting;
  async.download_paths = ChartDldrTempDownloadPathsFor(output_fn.GetFullPath());

  if (!ChartDldrTryStartBackgroundDownload(
          state_.transfer, panel_.AsEventHandler(),
          ChartDldrBulkTransferOwner::Catalog, url.BuildURI(),
          async.download_paths.download_target)) {
    ChartDldrRemoveTempDownload(async.download_paths.temp_path);
    async = ChartDldrCatalogRefreshState();
    return false;
  }

  async.active = true;
  return true;
}

void ChartDldrPanelBulkCatalogEngine::CancelCatalogRefresh() {
  ChartDldrCatalogRefreshState& async = state_.catalog_refresh;
  if (!async.active) {
    return;
  }
  ChartDldrCancelAndResetBulkTransfer(state_.transfer);
  ChartDldrRemoveTempDownload(async.download_paths.temp_path);
  async = ChartDldrCatalogRefreshState();
}

void ChartDldrPanelBulkCatalogEngine::ActivateAndPrepareBulkCatalog(
    int catalog_index, const ChartDldrCatalogUiPolicy& ui) {
  panel_.SetActiveCatalogContext(catalog_index);
  PrepareBulkCatalog(catalog_index, ui);
}

ChartDldrBulkWalkStep
ChartDldrPanelBulkCatalogEngine::RunBulkCatalogPrepareStep(
    ChartDldrBulkCatalogRunState& state, chartdldr_pi* pi,
    const ChartDldrBulkSessionPolicy& policy, int catalog_index,
    int* charts_selected, ChartDldrBulkRunStats* catalog_stats) {
  (void)state;

  chartdldr_pi* const prefs_pi = panel_.GetPlugin();
  const ChartDldrCatalogUiPolicy ui =
      policy.CatalogApply(prefs_pi ? prefs_pi->m_preselect_new : true,
                          prefs_pi ? prefs_pi->m_preselect_updated : true);

  auto fail_advance = [&]() {
    if (catalog_stats) {
      catalog_stats->catalog_refresh_failures = 1;
    }
    return ChartDldrBulkWalkStep::Advance;
  };

  if (state_.catalog_refresh.active) {
    panel_.SetActiveCatalogContext(catalog_index);
    ChartDldrCatalogRefreshState& async = state_.catalog_refresh;

    if (state_.transfer.IsInProgress()) {
      if (ChartDldrReactToStuckTransfer(
              state_.transfer, ChartDldrStuckTransferSite::CatalogPrepare) ==
          ChartDldrStuckTransferReaction::AbortCatalogRefresh) {
        CancelCatalogRefresh();
        return fail_advance();
      }
      return ChartDldrBulkWalkStep::TransferWait;
    }

    const _OCPN_DLStatus ret = ChartDldrCompleteValidatedCatalogDownloadPaths(
        async.download_paths, state_.transfer.success,
        state_.download_cancel.IsSessionCancelled());
    const int done_index = async.index;
    wxString url;
    if (pi && ChartDldrBulkCatalogIndexInRange(done_index,
                                               pi->m_ChartSources.size())) {
      url = pi->m_ChartSources.at(static_cast<size_t>(done_index))->GetUrl();
    }
    async = ChartDldrCatalogRefreshState();
    state_.transfer.Reset();

    ApplyCatalogDownloadOutcome(done_index, ui, policy.ErrorReporting(), ret,
                                url);
    if (!ChartDldrCatalogRefreshSucceeded(
            ret, state_.download_cancel.IsSessionCancelled())) {
      return fail_advance();
    }
    const int selected = panel_.GetCheckedChartCount();
    if (charts_selected) {
      *charts_selected = selected;
    }
    return ChartDldrBulkWalkStep::CatalogReady;
  }

  ActivateAndPrepareBulkCatalog(catalog_index, ui);
  if (!BeginCatalogRefresh(catalog_index, policy.ErrorReporting())) {
    return fail_advance();
  }
  if (state_.transfer.IsInProgress()) {
    return ChartDldrBulkWalkStep::TransferWait;
  }
  return ChartDldrBulkWalkStep::MoreWork;
}
