/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_bulk_chart_transfer.h"

#include "chartdldr_bulk_chart_loop.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_install.h"
#include "chartdldr_bulk_panel_port.h"
#include "chartdldr_pi.h"
#include "chartcatalog.h"
#include "chartdldr_chart_source.h"

#include "ocpn_plugin.h"

#include <wx/intl.h>

#ifdef __ANDROID__
#include "androidSupport.h"
#endif

namespace {

#ifdef __ANDROID__
struct AndroidInstallBusyScope {
  explicit AndroidInstallBusyScope(ChartDldrBulkPanelPort& port) {
    port.SetChartInfo(_("Installing charts."));
    androidShowBusyIcon();
  }
  ~AndroidInstallBusyScope() { androidHideBusyIcon(); }
};
#endif

void FinalizePendingChartDownload(ChartDldrBulkPanelPort& port, int chart_index,
                                  ChartDldrChartUpdateKind kind,
                                  ChartSource& source,
                                  const wxString& full_path,
                                  ChartDldrChartBulkState& chart_bulk,
                                  const ChartDldrBulkSessionPolicy& policy,
                                  chartdldr_pi* pi) {
  Chart* const chart = ChartDldrChartAt(pi->m_pChartCatalog, chart_index);
  if (!chart || !pi) {
    return;
  }

#ifdef __ANDROID__
  AndroidInstallBusyScope android_busy(port);
#endif
  const bool install_ok = ChartDldrCommitChartInstall(
      pi, source, full_path, chart->GetChartFilename(true),
      chart->GetUpdateDatetime(), ChartDldrVerboseExtractLog(policy));
  ChartDldrRecordChartDownloadOutcome(install_ok, kind, chart_bulk);
}

}  // namespace

void ChartDldrDisposeChartBulkTransfer(
    ChartDldrBulkPanelPort& port, ChartDldrBulkTransferSlot& transfer,
    const ChartDldrDownloadCancelState& download_cancel,
    ChartDldrChartTransferDisposition disposition, ChartSource* source,
    ChartDldrChartBulkState& chart_bulk,
    const ChartDldrBulkSessionPolicy& policy, chartdldr_pi* pi) {
  if (!chart_bulk.active ||
      !transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    return;
  }

  const ChartDldrChartTransferFacts facts{
      transfer.IsInProgress(),
      transfer.IsInProgress() ? false : transfer.success,
      transfer.IsInProgress() || download_cancel.ShouldAbortCurrentTransfer(),
      chart_bulk.pending_download_paths,
  };

  switch (disposition) {
    case ChartDldrChartTransferDisposition::None:
      return;

    case ChartDldrChartTransferDisposition::Settle: {
      if (!source || !pi || chart_bulk.pending_index < 0) {
        ChartDldrDisposeChartBulkTransfer(
            port, transfer, download_cancel,
            ChartDldrChartTransferDisposition::Abort, source, chart_bulk,
            policy, pi);
        return;
      }
      const _OCPN_DLStatus status = ChartDldrFinishBackgroundTempDownload(
          transfer, facts.cancelled, facts.paths, facts.success);
      chart_bulk.pending_download_paths = ChartDldrTempDownloadPaths();
      if (status == OCPN_DL_NO_ERROR) {
        FinalizePendingChartDownload(
            port, chart_bulk.pending_index, chart_bulk.pending_kind, *source,
            facts.paths.output_path, chart_bulk, policy, pi);
      } else {
        ChartDldrRecordChartDownloadFailure(chart_bulk);
      }
      return;
    }

    case ChartDldrChartTransferDisposition::Abort: {
      if (chart_bulk.pending_index < 0) {
        ChartDldrCancelAndResetBulkTransfer(transfer);
        return;
      }

      if (facts.in_progress) {
        ChartDldrCancelAndResetBulkTransfer(transfer);
      } else {
        transfer.Reset();
      }

      ChartDldrCompleteTempDownloadPaths(facts.paths, facts.success,
                                         facts.cancelled);
      chart_bulk.pending_download_paths = ChartDldrTempDownloadPaths();
      ChartDldrRecordChartDownloadFailure(chart_bulk);
      return;
    }
  }
}
