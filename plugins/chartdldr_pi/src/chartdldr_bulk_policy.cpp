/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk.h"

#include "chartdldr_schedule_state.h"

#include "ocpn_plugin.h"

#include <wx/intl.h>
#include <wx/log.h>

namespace {

/** Depth of nested CallAfter pump turns that must not open modal dialogs. */
int g_bulk_modal_suppress_depth = 0;

}  // namespace

void ChartDldrEnterBulkModalSuppress() { ++g_bulk_modal_suppress_depth; }

void ChartDldrLeaveBulkModalSuppress() {
  if (g_bulk_modal_suppress_depth > 0) {
    --g_bulk_modal_suppress_depth;
  }
}

bool ChartDldrBulkModalsSuppressed() { return g_bulk_modal_suppress_depth > 0; }

ChartDldrBulkSessionPolicy ChartDldrBulkSessionPolicyFor(
    ChartDldrBulkRunMode mode, bool panel_visible) {
  // Compile every mode-derived behavior into plain fields here; call sites
  // read fields, never the mode.
  ChartDldrBulkSessionPolicy policy;
  policy.mode = mode;
  switch (mode) {
    case ChartDldrBulkRunMode::SelectedCharts:
      policy.manual_plan_before_start = true;
      policy.walk_bind = ChartDldrBulkWalkBind::SingleDownload;
      break;
    case ChartDldrBulkRunMode::InteractiveBulk:
      policy.confirm_before_start = true;
      policy.collect_manual_urls = true;
      break;
    case ChartDldrBulkRunMode::CatalogRefresh:
      policy.walk_bind = ChartDldrBulkWalkBind::SinglePrepare;
      break;
    case ChartDldrBulkRunMode::ScheduledBulk:
      policy.scheduled = true;
      policy.skip_manual_url_charts = true;
      policy.preselect_all_charts = true;
      policy.error_reporting = ChartDldrErrorReporting::SummaryLog;
      break;
  }

  policy.allow_empty_selection =
      policy.walk_bind != ChartDldrBulkWalkBind::SingleDownload;
  policy.preserve_chart_selection =
      policy.walk_bind == ChartDldrBulkWalkBind::SingleDownload;
  policy.ui_materialize = !(policy.scheduled && !panel_visible);
  policy.ui_show_download_progress =
      policy.ui_materialize &&
      policy.walk_bind != ChartDldrBulkWalkBind::SinglePrepare;
  policy.ui_select_download_tab =
      policy.ui_show_download_progress &&
      policy.walk_bind != ChartDldrBulkWalkBind::SingleDownload;
  policy.focus_charts_after =
      !policy.scheduled &&
      policy.walk_bind != ChartDldrBulkWalkBind::SingleDownload;
  return policy;
}

ChartDldrCatalogUiPolicy ChartDldrBrowseCatalogUiPolicy(bool pref_new,
                                                        bool pref_updated) {
  ChartDldrCatalogUiPolicy ui;
  ui.materialize = true;
  ui.preserve_selection = false;
  ui.preselect_new = pref_new;
  ui.preselect_updated = pref_updated;
  ui.focus_charts_after = false;
  return ui;
}

ChartDldrCatalogUiPolicy ChartDldrBulkSessionPolicy::CatalogApply(
    bool pref_new, bool pref_updated) const {
  ChartDldrCatalogUiPolicy ui;
  ui.materialize = ui_materialize;
  ui.preserve_selection = preserve_chart_selection;
  ui.preselect_new = preselect_all_charts || pref_new;
  ui.preselect_updated = preselect_all_charts || pref_updated;
  ui.focus_charts_after = focus_charts_after;
  return ui;
}

ChartDldrBulkRunPlan ChartDldrSelectedChartsPreflightPlanFor(
    const std::vector<ChartDldrBulkPreflightChart>& charts,
    const wxString& target_dir) {
  ChartDldrBulkRunPlan plan;
  plan.allow_start = false;
  for (const ChartDldrBulkPreflightChart& chart : charts) {
    if (!chart.checked) {
      continue;
    }
    plan.allow_start = true;
    if (chart.needs_manual_download) {
      plan.manual_downloads.push_back(ChartDldrManualDownloadAction{
          chart.title, chart.manual_url, target_dir});
    }
  }
  return plan;
}

wxString ChartDldrBulkChartFailureMessage(int failed, int attempted) {
  return wxString::Format(
      _("%d out of %d charts failed to download.\nCheck the "
        "list, verify there is a working Internet "
        "connection and repeat the operation if needed."),
      failed, attempted);
}

wxString ChartDldrBulkRunStatusMessage(
    const ChartDldrBulkRunStats& stats,
    ChartDldrBulkRunStatusPresentation presentation) {
  const ChartDldrBulkRunClassification classified =
      ChartDldrClassifyBulkRun(stats);
  switch (presentation) {
    case ChartDldrBulkRunStatusPresentation::ScheduleLog:
      return classified.schedule_status;
    case ChartDldrBulkRunStatusPresentation::InteractiveDialog:
      return classified.interactive_message;
  }
  return classified.schedule_status;
}

ChartDldrBulkPostflight ChartDldrBulkPostflightFor(
    ChartDldrBulkTeardownReason reason,
    const ChartDldrBulkSessionPolicy& policy,
    const ChartDldrBulkRunStats& stats) {
  ChartDldrBulkPostflight result;
  if (reason == ChartDldrBulkTeardownReason::Shutdown ||
      reason == ChartDldrBulkTeardownReason::FailedStart || policy.scheduled) {
    return result;
  }
  if (reason == ChartDldrBulkTeardownReason::UserCancelled) {
    result.kind = ChartDldrBulkPostflightKind::Cancelled;
    result.message = _("Chart download cancelled.");
    return result;
  }
  if (reason == ChartDldrBulkTeardownReason::Completed &&
      (stats.failed > 0 || stats.catalog_refresh_failures > 0)) {
    result.kind = ChartDldrBulkPostflightKind::Summary;
    result.message = ChartDldrBulkRunStatusMessage(
        stats, ChartDldrBulkRunStatusPresentation::InteractiveDialog);
  }
  return result;
}

void ChartDldrReportBulkError(wxWindow* parent,
                              ChartDldrErrorReporting reporting,
                              const wxString& msg, const wxString& title) {
  // Never nest a modal under the bulk pump (DeInit from MessageBox can unload
  // the plugin DLL before LeaveBulkModalSuppress unwinds).
  if (ChartDldrBulkModalsSuppressed()) {
    wxLogWarning(wxT("chartdldr_pi: %s: %s"), title.c_str(), msg.c_str());
    return;
  }
  switch (reporting) {
    case ChartDldrErrorReporting::SummaryLog:
      break;
    case ChartDldrErrorReporting::Dialog:
      if (parent) {
        OCPNMessageBox_PlugIn(parent, msg, title);
      }
      break;
  }
}

bool ChartDldrBulkRunShouldRestoreUi(const ChartDldrBulkSessionPolicy& policy,
                                     const ChartDldrBulkRunUiSnapshot& before) {
  return policy.ui_materialize && before.panel_visible;
}
