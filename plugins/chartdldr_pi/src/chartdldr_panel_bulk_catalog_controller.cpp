/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_bulk_catalog_controller.h"

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_download.h"
#include "chartdldr_catalog_prep.h"
#include "chartcatalog.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_bulk_panel_view.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/url.h>

ChartDldrPanelBulkCatalogController::ChartDldrPanelBulkCatalogController(
    ChartDldrCatalogView& panel, ChartDldrBulkRunSession& session)
    : panel_(panel), session_(session) {}

void ChartDldrPanelBulkCatalogController::ReloadCatalogFromDisk(
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

void ChartDldrPanelBulkCatalogController::PrepareBulkCatalog(
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

void ChartDldrPanelBulkCatalogController::ApplyCatalogDownloadOutcome(
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

bool ChartDldrPanelBulkCatalogController::BeginCatalogRefresh(
    int catalog_index, ChartDldrErrorReporting error_reporting) {
  CancelCatalogRefresh();

  // Chart bulk owns the transfer slot; do not start a catalog refresh over it.
  if (session_.Transfer().IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
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

  ChartDldrCatalogRefreshPayload& refresh = session_.CatalogRefresh();
  refresh.index = catalog_index;
  refresh.error_reporting = error_reporting;
  refresh.download_paths =
      ChartDldrTempDownloadPathsFor(output_fn.GetFullPath());

  // "In progress" is derived from Catalog transfer ownership; a successful
  // start below flips CatalogRefreshInProgress() true with no separate flag.
  if (!ChartDldrTryStartBackgroundDownload(
          session_.Transfer(), panel_.AsEventHandler(),
          ChartDldrBulkTransferOwner::Catalog, url.BuildURI(),
          refresh.download_paths.download_target)) {
    ChartDldrRemoveTempDownload(refresh.download_paths.temp_path);
    refresh = ChartDldrCatalogRefreshPayload();
    return false;
  }

  return true;
}

void ChartDldrPanelBulkCatalogController::CancelCatalogRefresh() {
  if (!session_.CatalogRefreshInProgress()) {
    return;
  }
  ChartDldrCancelAndResetBulkTransfer(session_.Transfer());
  ChartDldrRemoveTempDownload(
      session_.CatalogRefresh().download_paths.temp_path);
  session_.CatalogRefresh() = ChartDldrCatalogRefreshPayload();
}

void ChartDldrPanelBulkCatalogController::ActivateAndPrepareBulkCatalog(
    int catalog_index, const ChartDldrCatalogUiPolicy& ui) {
  panel_.SetActiveCatalogContext(catalog_index);
  PrepareBulkCatalog(catalog_index, ui);
}

ChartDldrBulkWalkStep
ChartDldrPanelBulkCatalogController::RunBulkCatalogPrepareStep(
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

  if (session_.CatalogRefreshInProgress()) {
    panel_.SetActiveCatalogContext(catalog_index);
    ChartDldrCatalogRefreshPayload& refresh = session_.CatalogRefresh();

    if (session_.Transfer().IsInProgress()) {
      if (ChartDldrReactToStuckTransfer(
              session_.Transfer(),
              ChartDldrStuckTransferSite::CatalogPrepare) ==
          ChartDldrStuckTransferReaction::AbortCatalogRefresh) {
        CancelCatalogRefresh();
        return fail_advance();
      }
      return ChartDldrBulkWalkStep::TransferWait;
    }

    const _OCPN_DLStatus ret = ChartDldrCompleteValidatedCatalogDownloadPaths(
        refresh.download_paths, session_.Transfer().success,
        session_.DownloadCancel().IsSessionCancelled());
    const int done_index = refresh.index;
    wxString url;
    if (pi && ChartDldrBulkCatalogIndexInRange(done_index,
                                               pi->m_ChartSources.size())) {
      url = pi->m_ChartSources.at(static_cast<size_t>(done_index))->GetUrl();
    }
    refresh = ChartDldrCatalogRefreshPayload();
    session_.Transfer().Reset();

    ApplyCatalogDownloadOutcome(done_index, ui, policy.ErrorReporting(), ret,
                                url);
    if (!ChartDldrCatalogRefreshSucceeded(
            ret, session_.DownloadCancel().IsSessionCancelled())) {
      return fail_advance();
    }
    if (policy.SkipChartDownloadAfterRefresh()) {
      if (charts_selected) {
        *charts_selected = 0;
      }
      return ChartDldrBulkWalkStep::Advance;
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
  if (session_.Transfer().IsInProgress()) {
    return ChartDldrBulkWalkStep::TransferWait;
  }
  return ChartDldrBulkWalkStep::MoreWork;
}
