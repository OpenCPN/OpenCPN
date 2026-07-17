/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_bulk_chart_transfer.h"

#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_install.h"
#include "chartdldr_panel_impl.h"
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
  explicit AndroidInstallBusyScope(ChartDldrPanelImpl& panel) {
    panel.SetChartInfo(_("Installing charts."));
    androidShowBusyIcon();
  }
  ~AndroidInstallBusyScope() { androidHideBusyIcon(); }
};
#endif

Chart* ChartAt(chartdldr_pi* pi, int index) {
  if (!pi || index < 0 ||
      index >= static_cast<int>(pi->m_pChartCatalog.charts.size())) {
    return nullptr;
  }
  return pi->m_pChartCatalog.charts.at(static_cast<size_t>(index)).get();
}

void FinalizePendingChartDownload(
    ChartDldrPanelImpl& panel, int chart_index,
    ChartDldrChartUpdateKind kind, ChartSource& source,
    const wxString& full_path, ChartDldrChartBulkState& chart_bulk,
    const ChartDldrBulkSessionPolicy& policy, chartdldr_pi* pi) {
  Chart* const chart = ChartAt(pi, chart_index);
  if (!chart || !pi) {
    return;
  }

#ifdef __ANDROID__
  AndroidInstallBusyScope android_busy(panel);
#endif
  const bool install_ok = ChartDldrCommitChartInstall(
      pi, source, full_path, chart->GetChartFilename(true),
      chart->GetUpdateDatetime(), ChartDldrVerboseExtractLog(policy));
  ChartDldrRecordChartDownloadOutcome(install_ok, kind, chart_bulk);
}

}  // namespace

void ChartDldrDisposeChartBulkTransfer(
    ChartDldrPanelImpl& panel, ChartDldrPanelBulkState& state,
    ChartDldrChartTransferDisposition disposition, ChartSource* source,
    ChartDldrChartBulkState& chart_bulk,
    const ChartDldrBulkSessionPolicy& policy, chartdldr_pi* pi) {
  if (!chart_bulk.active ||
      !state.transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    return;
  }

  switch (disposition) {
    case ChartDldrChartTransferDisposition::None:
      return;

    case ChartDldrChartTransferDisposition::Settle: {
      if (!source || !pi || chart_bulk.pending_index < 0) {
        ChartDldrDisposeChartBulkTransfer(
            panel, state, ChartDldrChartTransferDisposition::Abort, source,
            chart_bulk, policy, pi);
        return;
      }
      const ChartDldrTempDownloadPaths paths =
          chart_bulk.pending_download_paths;
      const _OCPN_DLStatus status = ChartDldrFinishBackgroundTempDownload(
          state, paths, state.transfer.success);
      chart_bulk.pending_download_paths = ChartDldrTempDownloadPaths();
      if (status == OCPN_DL_NO_ERROR) {
        FinalizePendingChartDownload(panel, chart_bulk.pending_index,
                                     chart_bulk.pending_kind, *source,
                                     paths.output_path, chart_bulk, policy, pi);
      } else {
        ChartDldrRecordChartDownloadFailure(chart_bulk);
      }
      return;
    }

    case ChartDldrChartTransferDisposition::Abort: {
      if (chart_bulk.pending_index < 0) {
        ChartDldrCancelAndResetBulkTransfer(state.transfer);
        return;
      }

      const ChartDldrTempDownloadPaths paths =
          chart_bulk.pending_download_paths;
      const bool in_progress = state.transfer.IsInProgress();
      const bool transfer_success =
          in_progress ? false : state.transfer.success;
      const bool cancelled =
          in_progress || state.download_cancel.ShouldAbortCurrentTransfer();

      if (in_progress) {
        ChartDldrCancelAndResetBulkTransfer(state.transfer);
      } else {
        state.transfer.Reset();
      }

      ChartDldrCompleteTempDownloadPaths(paths, transfer_success, cancelled);
      chart_bulk.pending_download_paths = ChartDldrTempDownloadPaths();
      ChartDldrRecordChartDownloadFailure(chart_bulk);
      return;
    }
  }
}
