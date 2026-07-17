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
#include "chartdldr_bulk_panel_port.h"
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

void RecordCheckedChartFailure(ChartDldrBulkPanelPort& port,
                               const ChartDldrBulkSessionPolicy& policy,
                               ChartDldrChartBulkState& chart_bulk,
                               const wxString& error_msg = wxString(),
                               const wxString& title = _("Error")) {
  if (!error_msg.empty()) {
    ChartDldrReportBulkError(port.AsWindow(), policy, error_msg, title);
  }
  ChartDldrRecordChartDownloadFailure(chart_bulk);
}

ChartDldrChartUpdateKind ChartUpdateKindAt(chartdldr_pi* pi, int index,
                                           const ChartSource& source) {
  Chart* const chart = ChartDldrChartAt(pi->m_pChartCatalog, index);
  if (!chart) {
    return ChartDldrChartUpdateKind::None;
  }
  return ChartDldrCatalogChartStatus(source, chart->GetChartFilename(true),
                                     chart->GetUpdateDatetime());
}

}  // namespace

ChartDldrBulkChartController::ChartDldrBulkChartController(
    ChartDldrBulkPanelPort& port, ChartDldrBulkRunSession& session)
    : port_(port), session_(session) {}

void ChartDldrBulkChartController::BeginBulkChartDownload(
    const ChartDldrBulkSessionPolicy& policy, int catalog_index) {
  if (!port_.GetPlugin()) {
    return;
  }

  ChartDldrChartBulkState& chart_bulk = session_.ChartBulk();
  const int selected = port_.GetCheckedChartCount();
  if (!selected && !policy.AllowEmptySelection()) {
    ChartDldrReportBulkError(port_.AsWindow(), policy,
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

  port_.ArmChartDownloadCancelUi();
  session_.Transfer().Reset();
}

ChartDldrBulkWalkStep ChartDldrBulkChartController::PollActiveChartTransfer(
    const ChartDldrBulkSessionPolicy& policy,
    ChartDldrChartBulkState& chart_bulk, ChartSource& source, int chart_count,
    chartdldr_pi* pi) {
  ChartDldrBulkTransferSlot& transfer = session_.Transfer();
  ChartDldrDownloadCancelState& download_cancel = session_.DownloadCancel();
  if (transfer.IsInProgress() &&
      !download_cancel.ShouldAbortCurrentTransfer()) {
    if (ChartDldrReactToStuckTransfer(
            transfer, ChartDldrStuckTransferSite::ChartControllerPoll) ==
        ChartDldrStuckTransferReaction::AbortChartPass) {
      ChartDldrDisposeChartBulkTransfer(
          port_, transfer, download_cancel,
          ChartDldrChartTransferDisposition::Abort, &source, chart_bulk, policy,
          pi);
      return ChartDldrBulkWalkStep::MoreWork;
    }
    if (policy.UiShowDownloadProgress()) {
      port_.UpdateDownloadProgress(chart_bulk.downloading,
                                   chart_bulk.to_download, chart_bulk.failed,
                                   transfer);
    }
    return ChartDldrBulkWalkStep::TransferWait;
  }

  ChartDldrDisposeChartBulkTransfer(
      port_, transfer, download_cancel,
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
ChartDldrBulkChartController::StartNextCheckedChartDownload(
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
    if (!port_.IsChartChecked(i)) {
      continue;
    }

    const int index = i;
    Chart* const chart = ChartDldrChartAt(pi->m_pChartCatalog, index);
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
        if (!policy.SkipManualUrlCharts()) {
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
            port_, policy, chart_bulk,
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
            session_.Transfer(), ChartDldrBulkTransferOwner::ChartBulk,
            download_url, paths.download_target, session_.TransferEvents())) {
      ChartDldrRemoveTempDownload(paths.temp_path);
      RecordCheckedChartFailure(port_, policy, chart_bulk);
      continue;
    }
    chart_bulk.pending_download_paths = paths;
    return ChartDldrBulkWalkStep::TransferWait;
  }

  return ChartDldrBulkWalkStep::Advance;
}

ChartDldrBulkWalkStep ChartDldrBulkChartController::StepNextBulkChart(
    const ChartDldrBulkSessionPolicy& policy) {
  ChartDldrChartBulkState& chart_bulk = session_.ChartBulk();
  if (!chart_bulk.active) {
    return ChartDldrBulkWalkStep::NotActive;
  }
  if (session_.DownloadCancel().IsSessionCancelled()) {
    return ChartDldrBulkWalkStep::Advance;
  }

  chartdldr_pi* const pi = port_.GetPlugin();
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

ChartDldrBulkRunStats ChartDldrBulkChartController::FinishChartPass(
    const ChartDldrBulkSessionPolicy& policy) {
  ChartDldrChartBulkState& chart_bulk = session_.ChartBulk();
  if (!chart_bulk.active) {
    return ChartDldrBulkRunStats();
  }

  chartdldr_pi* const pi = port_.GetPlugin();
  const int catalog_index = chart_bulk.catalog_index;
  ChartSource* source = nullptr;
  if (pi && ChartDldrBulkCatalogIndexInRange(catalog_index,
                                             pi->m_ChartSources.size())) {
    source = pi->m_ChartSources.at(static_cast<size_t>(catalog_index)).get();
  }

  ChartDldrBulkTransferSlot& transfer = session_.Transfer();
  ChartDldrDownloadCancelState& download_cancel = session_.DownloadCancel();
  ChartDldrDisposeChartBulkTransfer(
      port_, transfer, download_cancel,
      ChartDldrResolveChartTransferDisposition(
          transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk),
          transfer.IsInProgress(), transfer.success,
          download_cancel.ShouldAbortCurrentTransfer()),
      source, chart_bulk, policy, pi);

  const ChartDldrBulkRunStats stats = chart_bulk.ToStats();

  if (session_.Transfer().IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    ChartDldrCancelAndResetBulkTransfer(session_.Transfer());
  }

  port_.DisarmChartDownloadCancelUi();
  chart_bulk.active = false;

  if (policy.UiMaterialize()) {
    chartdldr_pi* const pi = port_.GetPlugin();
    const ChartDldrCatalogUiPolicy ui = policy.CatalogApply(
        pi ? pi->m_preselect_new : true, pi ? pi->m_preselect_updated : true);
    port_.ActivateCatalog(chart_bulk.catalog_index,
                          ChartDldrCatalogActivation::WithReload, ui);
  }

  return stats;
}
