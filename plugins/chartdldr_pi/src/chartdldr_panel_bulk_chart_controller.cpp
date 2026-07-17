/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_bulk_chart_controller.h"

#include "chartdldr_bulk_chart_loop.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_bulk_panel_view.h"
#include "chartdldr_panel_bulk_chart_transfer.h"
#include "chartdldr_temp_download.h"
#include "chartcatalog.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/url.h>
#include <wx/utils.h>

namespace {

void RecordCheckedChartFailure(ChartDldrChartDownloadView& panel,
                               const ChartDldrBulkSessionPolicy& policy,
                               ChartDldrChartBulkState& chart_bulk,
                               const wxString& error_msg = wxString(),
                               const wxString& title = _("Error")) {
  if (!error_msg.empty()) {
    ChartDldrReportBulkError(panel.AsWindow(), policy, error_msg, title);
  }
  ChartDldrRecordChartDownloadFailure(chart_bulk);
}

Chart* ChartAt(chartdldr_pi* pi, int index) {
  if (!pi || index < 0 ||
      index >= static_cast<int>(pi->m_pChartCatalog.charts.size())) {
    return nullptr;
  }
  return pi->m_pChartCatalog.charts.at(static_cast<size_t>(index)).get();
}

ChartDldrChartUpdateKind ChartUpdateKindAt(chartdldr_pi* pi, int index,
                                           const ChartSource& source) {
  Chart* const chart = ChartAt(pi, index);
  if (!chart) {
    return ChartDldrChartUpdateKind::None;
  }
  return ChartDldrCatalogChartStatus(source, chart->GetChartFilename(true),
                                     chart->GetUpdateDatetime());
}

}  // namespace

ChartDldrPanelBulkChartController::ChartDldrPanelBulkChartController(
    ChartDldrChartDownloadView& panel, ChartDldrBulkRunSession& session)
    : panel_(panel), session_(session) {}

void ChartDldrPanelBulkChartController::BeginBulkChartDownload(
    const ChartDldrBulkSessionPolicy& policy, int catalog_index,
    ChartDldrChartBulkState& chart_bulk) {
  if (!panel_.GetPlugin()) {
    return;
  }

  const int selected = panel_.GetCheckedChartCount();
  if (!selected && !policy.AllowEmptySelection()) {
    ChartDldrReportBulkError(panel_.AsWindow(), policy,
                             _("No charts selected for download."),
                             _("Chart Downloader"));
    return;
  }

  chart_bulk.catalog_index = catalog_index;
  chart_bulk.to_download = selected;
  chart_bulk.downloading = 0;
  chart_bulk.failed = 0;
  chart_bulk.new_downloads = 0;
  chart_bulk.updated_downloads = 0;
  chart_bulk.loop_index = 0;
  chart_bulk.pending_index = -1;
  chart_bulk.pending_download_paths = ChartDldrTempDownloadPaths();
  chart_bulk.active = true;

  panel_.ArmChartDownloadCancelUi();
  session_.Transfer().Reset();
}

ChartDldrBulkWalkStep
ChartDldrPanelBulkChartController::PollActiveChartTransfer(
    const ChartDldrBulkSessionPolicy& policy,
    ChartDldrChartBulkState& chart_bulk, ChartSource& source, int chart_count,
    chartdldr_pi* pi) {
  ChartDldrBulkTransferSlot& transfer = session_.Transfer();
  ChartDldrDownloadCancelState& download_cancel = session_.DownloadCancel();
  if (transfer.IsInProgress() &&
      !download_cancel.ShouldAbortCurrentTransfer()) {
    if (ChartDldrReactToStuckTransfer(
            transfer, ChartDldrStuckTransferSite::ChartEnginePoll) ==
        ChartDldrStuckTransferReaction::AbortChartPass) {
      ChartDldrDisposeChartBulkTransfer(
          panel_, transfer, download_cancel,
          ChartDldrChartTransferDisposition::Abort, &source, chart_bulk, policy,
          pi);
      return ChartDldrBulkWalkStep::MoreWork;
    }
    if (policy.UiShowDownloadProgress()) {
      panel_.UpdateDownloadProgress(chart_bulk.downloading,
                                    chart_bulk.to_download, chart_bulk.failed,
                                    transfer);
    }
    return ChartDldrBulkWalkStep::TransferWait;
  }

  ChartDldrDisposeChartBulkTransfer(
      panel_, transfer, download_cancel,
      ChartDldrResolveChartTransferDisposition(
          transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk),
          transfer.IsInProgress(), transfer.success,
          download_cancel.ShouldAbortCurrentTransfer()),
      &source, chart_bulk, policy, pi);
  // Only a whole-session cancel ends the pass here; a one-shot abort of the
  // current chart is consumed so the walk continues to the next checked chart.
  download_cancel.ConsumeAbortCurrentTransfer();
  return ChartDldrBulkChartStepAfterCompletedTransfer(
      download_cancel.IsSessionCancelled(), chart_bulk.loop_index, chart_count,
      chart_bulk.to_download);
}

ChartDldrBulkWalkStep
ChartDldrPanelBulkChartController::StartNextCheckedChartDownload(
    const ChartDldrBulkSessionPolicy& policy,
    ChartDldrChartBulkState& chart_bulk, ChartSource& source, int chart_count,
    chartdldr_pi* pi) {
  ChartSource* const catalog_source = &source;
  for (int i = chart_bulk.loop_index; i < chart_count && chart_bulk.to_download;
       ++i) {
    chart_bulk.loop_index = i + 1;
    if (session_.DownloadCancel().IsSessionCancelled()) {
      return ChartDldrBulkWalkStep::Advance;
    }
    if (!panel_.IsChartChecked(i)) {
      continue;
    }

    const int index = i;
    Chart* const chart = ChartAt(pi, index);
    if (!chart) {
      continue;
    }

    wxString download_url;
    ChartDldrTempDownloadPaths paths;
    const ChartDldrBulkChartDownloadInfo chart_info{
        chart->NeedsManualDownload(), chart->GetDownloadLocation(),
        chart->GetChartFilename()};
    switch (ChartDldrPrepareCheckedChartDownload(chart_info, source.GetDir(),
                                                 download_url, paths)) {
      case ChartDldrPrepareCheckedChartDownloadResult::SkipManual:
        if (ChartDldrBulkPlanOpensDiscoveredManualUrls(policy.plan) &&
            !chart->GetManualDownloadUrl().IsEmpty()) {
          wxLaunchDefaultBrowser(chart->GetManualDownloadUrl());
        }
        if (!policy.SkipManualCharts()) {
          const wxString extract_dir =
              catalog_source ? catalog_source->GetDir() : wxString();
          wxLogMessage(
              wxT("chartdldr_pi: skipping manual-download chart \"%s\" "
                  "(extract to %s)"),
              chart->GetChartTitle().c_str(), extract_dir.c_str());
        }
        continue;
      case ChartDldrPrepareCheckedChartDownloadResult::InvalidUrl:
        // Count the dequeue as an attempt before recording the failure.
        chart_bulk.downloading++;
        RecordCheckedChartFailure(
            panel_, policy, chart_bulk,
            wxString::Format(
                _("Error, the URL to the chart (%s) data seems wrong."),
                chart->GetDownloadLocation().c_str()));
        continue;
      case ChartDldrPrepareCheckedChartDownloadResult::Ready:
        break;
    }

    chart_bulk.downloading++;
    chart_bulk.pending_index = index;
    chart_bulk.pending_kind = ChartUpdateKindAt(pi, index, source);

    if (!ChartDldrTryStartBackgroundDownload(
            session_.Transfer(), panel_.AsEventHandler(),
            ChartDldrBulkTransferOwner::ChartBulk, download_url,
            paths.download_target)) {
      ChartDldrRemoveTempDownload(paths.temp_path);
      RecordCheckedChartFailure(panel_, policy, chart_bulk);
      continue;
    }
    chart_bulk.pending_download_paths = paths;
    return ChartDldrBulkWalkStep::TransferWait;
  }

  return ChartDldrBulkWalkStep::Advance;
}

ChartDldrBulkWalkStep ChartDldrPanelBulkChartController::StepNextBulkChart(
    const ChartDldrBulkSessionPolicy& policy,
    ChartDldrChartBulkState& chart_bulk) {
  if (!chart_bulk.active) {
    return ChartDldrBulkWalkStep::NotActive;
  }
  if (session_.DownloadCancel().IsSessionCancelled()) {
    return ChartDldrBulkWalkStep::Advance;
  }

  chartdldr_pi* const pi = panel_.GetPlugin();
  const int catalog_index = chart_bulk.catalog_index;
  if (!pi || !ChartDldrBulkCatalogIndexInRange(catalog_index,
                                               pi->m_ChartSources.size())) {
    return ChartDldrBulkWalkStep::Advance;
  }

  ChartSource& source =
      *pi->m_ChartSources.at(static_cast<size_t>(catalog_index));
  const int chart_count = static_cast<int>(pi->m_pChartCatalog.charts.size());

  if (session_.Transfer().IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    return PollActiveChartTransfer(policy, chart_bulk, source, chart_count, pi);
  }
  return StartNextCheckedChartDownload(policy, chart_bulk, source, chart_count,
                                       pi);
}

ChartDldrBulkRunStats ChartDldrPanelBulkChartController::CloseActiveChartPass(
    const ChartDldrBulkSessionPolicy& policy,
    ChartDldrChartBulkState& chart_bulk) {
  if (!chart_bulk.active) {
    return ChartDldrBulkRunStats();
  }

  chartdldr_pi* const pi = panel_.GetPlugin();
  const int catalog_index = chart_bulk.catalog_index;
  ChartSource* source = nullptr;
  if (pi && ChartDldrBulkCatalogIndexInRange(catalog_index,
                                             pi->m_ChartSources.size())) {
    source = pi->m_ChartSources.at(static_cast<size_t>(catalog_index)).get();
  }

  ChartDldrBulkTransferSlot& transfer = session_.Transfer();
  ChartDldrDownloadCancelState& download_cancel = session_.DownloadCancel();
  ChartDldrDisposeChartBulkTransfer(
      panel_, transfer, download_cancel,
      ChartDldrResolveChartTransferDisposition(
          transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk),
          transfer.IsInProgress(), transfer.success,
          download_cancel.ShouldAbortCurrentTransfer()),
      source, chart_bulk, policy, pi);

  const ChartDldrBulkRunStats stats = chart_bulk.ToStats();
  EndBulkChartDownload(policy, chart_bulk);
  return stats;
}

void ChartDldrPanelBulkChartController::EndBulkChartDownload(
    const ChartDldrBulkSessionPolicy& policy,
    ChartDldrChartBulkState& chart_bulk) {
  if (!chart_bulk.active) {
    return;
  }

  if (session_.Transfer().IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    ChartDldrCancelAndResetBulkTransfer(session_.Transfer());
  }

  panel_.DisarmChartDownloadCancelUi();
  chart_bulk.active = false;

  if (policy.UiMaterialize()) {
    chartdldr_pi* const pi = panel_.GetPlugin();
    const ChartDldrCatalogUiPolicy ui = policy.CatalogApply(
        pi ? pi->m_preselect_new : true, pi ? pi->m_preselect_updated : true);
    const int restore_catalog = chart_bulk.catalog_index;
    if (restore_catalog >= 0) {
      panel_.RefreshChartListForSource(restore_catalog, ui);
    } else {
      panel_.SelectActiveCatalog(panel_.GetSelectedCatalog());
    }
  }
}
