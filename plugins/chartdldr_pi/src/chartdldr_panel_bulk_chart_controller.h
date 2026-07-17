/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_BULK_CHART_CONTROLLER_H_
#define CHARTDLDR_PANEL_BULK_CHART_CONTROLLER_H_

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_state.h"

class ChartDldrChartDownloadView;
class ChartSource;
class chartdldr_pi;

/** Chart download loop and transfer polling for bulk download. */
class ChartDldrPanelBulkChartController {
public:
  ChartDldrPanelBulkChartController(ChartDldrChartDownloadView& panel,
                                    ChartDldrBulkRunSession& session);

  void BeginBulkChartDownload(const ChartDldrBulkSessionPolicy& policy,
                              int catalog_index,
                              ChartDldrChartBulkState& chart_bulk);
  ChartDldrBulkWalkStep StepNextBulkChart(
      const ChartDldrBulkSessionPolicy& policy,
      ChartDldrChartBulkState& chart_bulk);
  /**
   * Orchestrator-only: dispose any open transfer and end the chart pass.
   * Returns pass stats when one was open.
   */
  ChartDldrBulkRunStats CloseActiveChartPass(
      const ChartDldrBulkSessionPolicy& policy,
      ChartDldrChartBulkState& chart_bulk);
  void EndBulkChartDownload(const ChartDldrBulkSessionPolicy& policy,
                            ChartDldrChartBulkState& chart_bulk);

private:
  ChartDldrBulkWalkStep PollActiveChartTransfer(
      const ChartDldrBulkSessionPolicy& policy,
      ChartDldrChartBulkState& chart_bulk, ChartSource& source, int chart_count,
      chartdldr_pi* pi);
  ChartDldrBulkWalkStep StartNextCheckedChartDownload(
      const ChartDldrBulkSessionPolicy& policy,
      ChartDldrChartBulkState& chart_bulk, ChartSource& source, int chart_count,
      chartdldr_pi* pi);

  ChartDldrChartDownloadView& panel_;
  ChartDldrBulkRunSession& session_;
};

#endif  // CHARTDLDR_PANEL_BULK_CHART_CONTROLLER_H_
