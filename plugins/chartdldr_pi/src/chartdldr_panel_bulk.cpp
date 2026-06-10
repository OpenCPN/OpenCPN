/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_panel_bulk.h"

#include "chartdldr_bulk_catalog.h"
#include "chartdldr_panel_impl.h"

void ChartDldrPanelBulk::RunInteractiveCatalogPass(
    ChartDldrPanelImpl& panel, int catalog_index,
    const ChartDldrBulkModeProfile& profile, wxCommandEvent& event,
    ChartDldrBulkRunStats& stats) {
  ChartDldrChartBulkState chart_bulk;
  panel.PrepareBulkCatalog(catalog_index, profile);
  ChartDldrBeginCatalogRefresh(
      panel, catalog_index, profile,
      ChartDldrCatalogRefreshContext::WithEvent(event));
  DownloadCharts(panel, profile, chart_bulk);
  stats.Accumulate(chart_bulk.ToStats());
}

void ChartDldrPanelBulk::DownloadCharts(ChartDldrPanelImpl& panel,
                                        const ChartDldrBulkModeProfile& profile,
                                        ChartDldrChartBulkState& chart_bulk) {
  panel.BeginBulkChartDownload(profile, chart_bulk);
  while (true) {
    const ChartDldrBulkChartStepResult step =
        panel.StepNextBulkChart(profile, chart_bulk);
    if (step == ChartDldrBulkChartStepResult::TransferInProgress) {
      if (profile.charts.transfer_poll ==
          ChartDldrTransferPoll::BlockUntilComplete) {
        panel.WaitForTransfer(
            profile,
            profile.ui.show_download_progress_ui && panel.IsShownOnScreen(),
            &chart_bulk);
        continue;
      }
      break;
    }
    if (step == ChartDldrBulkChartStepResult::MoreCharts) {
      continue;
    }
    break;
  }
  panel.EndBulkChartDownload(profile, chart_bulk);
}
