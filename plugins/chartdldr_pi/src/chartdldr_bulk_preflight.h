/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_PREFLIGHT_H_
#define CHARTDLDR_BULK_PREFLIGHT_H_

#include "chartdldr_bulk_session_policy.h"

#include <vector>

class ChartDldrBulkPanelPort;
class chartdldr_pi;
class wxWindow;

/** Build the SelectedCharts plan from the checked chart-list rows. */
ChartDldrBulkRunPlan ChartDldrBuildSelectedChartsPreflightPlan(
    ChartDldrBulkPanelPort& port, chartdldr_pi& pi);

/** Pre-start prompt for manual-download charts, then open confirmed pages. */
void ChartDldrPromptAndOpenManualDownloads(
    wxWindow* parent,
    const std::vector<ChartDldrManualDownloadAction>& actions);

/** Open manual pages without prompting (consent given at session confirm). */
void ChartDldrOpenManualDownloadUrls(
    const std::vector<ChartDldrManualDownloadAction>& actions);

#endif  // CHARTDLDR_BULK_PREFLIGHT_H_
