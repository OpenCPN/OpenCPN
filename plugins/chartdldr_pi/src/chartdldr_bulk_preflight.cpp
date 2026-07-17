/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_bulk_preflight.h"

#include "chartdldr_bulk_panel_port.h"
#include "chartdldr_pi.h"
#include "chartcatalog.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_source.h"

#include "ocpn_plugin.h"

#include <wx/intl.h>
#include <wx/utils.h>

#include <algorithm>

ChartDldrBulkRunPlan ChartDldrBuildSelectedChartsPreflightPlan(
    ChartDldrBulkPanelPort& port, chartdldr_pi& pi) {
  std::vector<ChartDldrBulkPreflightChart> charts;
  const int chart_count = port.GetChartCount();
  charts.reserve(static_cast<size_t>(chart_count));
  for (int i = 0; i < chart_count; ++i) {
    Chart* const chart = ChartDldrChartAt(pi.m_pChartCatalog, i);
    charts.push_back(ChartDldrBulkPreflightChart{
        port.IsChartChecked(i), chart && chart->NeedsManualDownload(),
        chart ? chart->GetChartTitle() : wxString(),
        chart ? chart->GetManualDownloadUrl() : wxString()});
  }
  const wxString target_dir =
      pi.m_pChartSource ? pi.m_pChartSource->GetDir() : wxString();
  return ChartDldrSelectedChartsPreflightPlanFor(charts, target_dir);
}

void ChartDldrOpenManualDownloadUrls(
    const std::vector<ChartDldrManualDownloadAction>& actions) {
  for (const ChartDldrManualDownloadAction& action : actions) {
    if (!action.url.IsEmpty()) {
      wxLaunchDefaultBrowser(action.url);
    }
  }
}

void ChartDldrPromptAndOpenManualDownloads(
    wxWindow* parent,
    const std::vector<ChartDldrManualDownloadAction>& actions) {
  if (actions.empty()) {
    return;
  }

  wxString body = _(
      "These charts require manual download. Open their browser pages now?\n");
  const size_t show = std::min<size_t>(actions.size(), static_cast<size_t>(8));
  for (size_t i = 0; i < show; ++i) {
    body += wxT("\n• ");
    body += actions.at(i).title;
  }
  if (actions.size() > show) {
    body += wxString::Format(_("\n…and %zu more."), actions.size() - show);
  }
  body += _("\n\nAutomatic chart downloads will continue either way.");

  const int answer = OCPNMessageBox_PlugIn(parent, body, _("Chart Downloader"),
                                           wxYES_NO | wxICON_QUESTION);
  if (answer == wxID_YES) {
    ChartDldrOpenManualDownloadUrls(actions);
  }
}
