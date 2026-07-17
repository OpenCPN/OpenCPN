/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_BULK_CHART_TRANSFER_H_
#define CHARTDLDR_PANEL_BULK_CHART_TRANSFER_H_

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_state.h"

class ChartDldrPanelImpl;
class ChartSource;
class chartdldr_pi;
struct ChartDldrPanelBulkState;

void ChartDldrDisposeChartBulkTransfer(
    ChartDldrPanelImpl& panel, ChartDldrPanelBulkState& state,
    ChartDldrChartTransferDisposition disposition, ChartSource* source,
    ChartDldrChartBulkState& chart_bulk,
    const ChartDldrBulkSessionPolicy& policy, chartdldr_pi* pi);

#endif  // CHARTDLDR_PANEL_BULK_CHART_TRANSFER_H_
