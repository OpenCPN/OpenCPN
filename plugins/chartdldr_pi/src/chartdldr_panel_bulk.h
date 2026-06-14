/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_BULK_H_
#define CHARTDLDR_PANEL_BULK_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_state.h"

class ChartDldrPanelImpl;
class wxCommandEvent;

/** Multi-step chart bulk flows over ChartDldrPanelImpl wx primitives. */
struct ChartDldrPanelBulk {
  ChartDldrPanelBulk() = delete;

  static void RunInteractiveCatalogPass(ChartDldrPanelImpl& panel,
                                        int catalog_index,
                                        const ChartDldrBulkModeProfile& profile,
                                        wxCommandEvent& event,
                                        ChartDldrBulkRunStats& stats);
  static void DownloadCharts(ChartDldrPanelImpl& panel,
                             const ChartDldrBulkModeProfile& profile,
                             ChartDldrChartBulkState& chart_bulk);
};

#endif  // CHARTDLDR_PANEL_BULK_H_
