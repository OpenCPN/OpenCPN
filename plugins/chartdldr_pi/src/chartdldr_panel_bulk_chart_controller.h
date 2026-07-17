/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_BULK_CHART_CONTROLLER_H_
#define CHARTDLDR_PANEL_BULK_CHART_CONTROLLER_H_

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_state.h"

class ChartDldrBulkPanelPort;
class ChartSource;
class chartdldr_pi;

/** Chart download loop and transfer polling for bulk download. */
class ChartDldrBulkChartController {
public:
  ChartDldrBulkChartController(ChartDldrBulkPanelPort& port,
                               ChartDldrBulkRunSession& session);

  void BeginBulkChartDownload(const ChartDldrBulkSessionPolicy& policy,
                              int catalog_index);
  ChartDldrBulkWalkStep StepNextBulkChart(
      const ChartDldrBulkSessionPolicy& policy);
  /**
   * Orchestrator-only: dispose any open transfer and end the chart pass.
   * Returns pass stats when one was open.
   */
  ChartDldrBulkRunStats FinishChartPass(
      const ChartDldrBulkSessionPolicy& policy);

private:
  ChartDldrBulkWalkStep PollActiveChartTransfer(
      const ChartDldrBulkSessionPolicy& policy,
      ChartDldrChartBulkState& chart_bulk, ChartSource& source, int chart_count,
      chartdldr_pi* pi);
  ChartDldrBulkWalkStep StartNextCheckedChartDownload(
      const ChartDldrBulkSessionPolicy& policy,
      ChartDldrChartBulkState& chart_bulk, ChartSource& source, int chart_count,
      chartdldr_pi* pi);

  ChartDldrBulkPanelPort& port_;
  ChartDldrBulkRunSession& session_;
};

#endif  // CHARTDLDR_PANEL_BULK_CHART_CONTROLLER_H_
