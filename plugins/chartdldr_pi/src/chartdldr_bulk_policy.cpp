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
  ChartDldrTransferPoll transfer_poll;
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
       false, false, true, ChartDldrTransferPoll::BlockUntilComplete},
      {// ScheduledBulk
       ChartDldrCatalogRefreshMode::AsyncIdle,
       ChartDldrErrorReporting::SummaryLog,
       ChartDldrPolicyTriState::WhenPanelVisible,
       ChartDldrPolicyTriState::WhenPanelVisible, false, false, false, true,
       true, true, false, false, false, false, ChartDldrTransferPoll::PollOnly},
      {// SelectedCharts
       ChartDldrCatalogRefreshMode::SyncBlocking,
       ChartDldrErrorReporting::Dialog, ChartDldrPolicyTriState::Off,
       ChartDldrPolicyTriState::Off, false, false, false, false, false, false,
       true, true, false, false, ChartDldrTransferPoll::BlockUntilComplete},
      {// CatalogRefresh
       ChartDldrCatalogRefreshMode::SyncBlocking,
       ChartDldrErrorReporting::Dialog, ChartDldrPolicyTriState::Off,
       ChartDldrPolicyTriState::Off, false, false, false, false, false, false,
       false, false, true, true, ChartDldrTransferPoll::BlockUntilComplete},
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
  profile.catalog.refresh = row.catalog_refresh;
  profile.catalog.show_progress_dialog = row.show_catalog_progress_dialog;
  profile.error_reporting = row.error_reporting;
  profile.ui.restore_notebook_page =
      TriStateValue(row.restore_notebook_page, policy.bulk_panel_visible);
  profile.ui.select_download_tab =
      TriStateValue(row.select_download_tab, policy.bulk_panel_visible);
  profile.ui.confirm_before_start = row.confirm_before_start;
  profile.ui.show_failure_summary = row.show_failure_summary;
  profile.ui.sync_list_selection = row.sync_list_selection;
  profile.ui.show_download_progress_ui = row.show_download_progress_ui;
  profile.ui.show_download_result_dialogs = row.show_download_result_dialogs;
  profile.ui.focus_charts_tab_after_catalog =
      row.focus_charts_tab_after_catalog;
  profile.charts.skip_manual_charts = row.skip_manual_charts;
  profile.charts.defer_chart_db_apply = row.defer_chart_db_apply;
  profile.charts.allow_empty_selection = row.allow_empty_selection;
  profile.charts.transfer_poll = row.transfer_poll;
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
  return profile.ui.restore_notebook_page && before.panel_visible;
}

bool ChartDldrShouldYieldOnDownloadEvent(bool bulk_run_active,
                                         ChartDldrTransferPoll transfer_poll) {
  if (!bulk_run_active) {
    return true;
  }
  return transfer_poll == ChartDldrTransferPoll::BlockUntilComplete;
}

ChartDldrCatalogRefreshPath ChartDldrResolveCatalogRefreshPath(
    const ChartDldrBulkModeProfile& profile,
    const ChartDldrCatalogRefreshContext& ctx) {
  if (profile.catalog.refresh == ChartDldrCatalogRefreshMode::AsyncIdle) {
    return ChartDldrCatalogRefreshPath::AsyncIdle;
  }
  if (ctx.sync_event == nullptr) {
    return ChartDldrCatalogRefreshPath::Invalid;
  }
  return ChartDldrCatalogRefreshPath::SyncBlocking;
}
