/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk.h"

#include "ocpn_plugin.h"

#include <wx/intl.h>

namespace {

enum class ChartDldrPolicyTriState { Off, On, WhenPanelVisible };

struct ChartDldrBulkModeProfileRow {
  ChartDldrCatalogRefreshMode catalog_refresh;
  ChartDldrErrorReporting error_reporting;
  ChartDldrPolicyTriState restore_notebook_page;
  ChartDldrPolicyTriState select_download_tab;
  bool confirm_before_start;
  bool show_failure_summary;
  bool sync_list_selection;
  bool skip_manual_charts;
  bool defer_chart_db_apply;
  bool allow_empty_selection;
  bool show_download_progress_ui;
  bool show_download_result_dialogs;
  bool show_catalog_progress_dialog;
  bool focus_charts_tab_after_catalog;
};

bool TriStateValue(ChartDldrPolicyTriState value, bool panel_visible) {
  switch (value) {
    case ChartDldrPolicyTriState::Off:
      return false;
    case ChartDldrPolicyTriState::On:
      return true;
    case ChartDldrPolicyTriState::WhenPanelVisible:
      return panel_visible;
  }
  return false;
}

const ChartDldrBulkModeProfileRow& ModeProfileRow(
    const ChartDldrBulkRunUiPolicy& policy) {
  static const ChartDldrBulkModeProfileRow profiles[] = {
      {// InteractiveBulk
       ChartDldrCatalogRefreshMode::SyncBlocking,
       ChartDldrErrorReporting::Dialog, ChartDldrPolicyTriState::On,
       ChartDldrPolicyTriState::On, true, true, true, false, true, true, true,
       false, false, true},
      {// ScheduledBulk
       ChartDldrCatalogRefreshMode::AsyncIdle,
       ChartDldrErrorReporting::SummaryLog,
       ChartDldrPolicyTriState::WhenPanelVisible,
       ChartDldrPolicyTriState::WhenPanelVisible, false, false, false, true,
       true, true, false, false, false, false},
      {// SelectedCharts
       ChartDldrCatalogRefreshMode::SyncBlocking,
       ChartDldrErrorReporting::Dialog, ChartDldrPolicyTriState::Off,
       ChartDldrPolicyTriState::Off, false, false, false, false, false, false,
       true, true, false, false},
      {// CatalogRefresh
       ChartDldrCatalogRefreshMode::SyncBlocking,
       ChartDldrErrorReporting::Dialog, ChartDldrPolicyTriState::Off,
       ChartDldrPolicyTriState::Off, false, false, false, false, false, false,
       false, false, true, true},
  };
  return profiles[static_cast<size_t>(policy.mode)];
}

ChartDldrBulkRunUiPolicy MakePolicy(ChartDldrDownloadUiMode mode,
                                    bool panel_visible) {
  ChartDldrBulkRunUiPolicy policy;
  policy.mode = mode;
  policy.bulk_panel_visible = panel_visible;
  return policy;
}

}  // namespace

ChartDldrBulkModeProfile ChartDldrBulkModeProfileFor(
    const ChartDldrBulkRunUiPolicy& policy) {
  const ChartDldrBulkModeProfileRow& row = ModeProfileRow(policy);
  ChartDldrBulkModeProfile profile;
  profile.catalog_refresh = row.catalog_refresh;
  profile.error_reporting = row.error_reporting;
  profile.restore_notebook_page =
      TriStateValue(row.restore_notebook_page, policy.bulk_panel_visible);
  profile.select_download_tab =
      TriStateValue(row.select_download_tab, policy.bulk_panel_visible);
  profile.confirm_before_start = row.confirm_before_start;
  profile.show_failure_summary = row.show_failure_summary;
  profile.sync_list_selection = row.sync_list_selection;
  profile.skip_manual_charts = row.skip_manual_charts;
  profile.defer_chart_db_apply = row.defer_chart_db_apply;
  profile.allow_empty_selection = row.allow_empty_selection;
  profile.show_download_progress_ui = row.show_download_progress_ui;
  profile.show_download_result_dialogs = row.show_download_result_dialogs;
  profile.show_catalog_progress_dialog = row.show_catalog_progress_dialog;
  profile.focus_charts_tab_after_catalog = row.focus_charts_tab_after_catalog;
  return profile;
}

void ChartDldrReportBulkError(wxWindow* parent,
                              const ChartDldrBulkModeProfile& profile,
                              const wxString& msg, const wxString& title) {
  switch (profile.error_reporting) {
    case ChartDldrErrorReporting::SummaryLog:
      break;
    case ChartDldrErrorReporting::Dialog:
      if (parent) {
        OCPNMessageBox_PlugIn(parent, msg, title);
      }
      break;
  }
}

ChartDldrBulkRunUiPolicy ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind kind,
                                                     bool panel_visible) {
  if (ChartDldrBulkRunIsScheduled(kind)) {
    return MakePolicy(ChartDldrDownloadUiMode::ScheduledBulk, panel_visible);
  }
  return MakePolicy(ChartDldrDownloadUiMode::InteractiveBulk, panel_visible);
}

ChartDldrBulkModeProfile ChartDldrSelectedChartsDownloadProfile() {
  return ChartDldrBulkModeProfileFor(
      MakePolicy(ChartDldrDownloadUiMode::SelectedCharts, false));
}

ChartDldrBulkModeProfile ChartDldrInteractiveCatalogUpdateProfile() {
  return ChartDldrBulkModeProfileFor(
      MakePolicy(ChartDldrDownloadUiMode::CatalogRefresh, false));
}

bool ChartDldrBulkRunShouldRestoreUi(const ChartDldrBulkModeProfile& profile,
                                     const ChartDldrBulkRunUiSnapshot& before) {
  return profile.restore_notebook_page && before.panel_visible;
}
