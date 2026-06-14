/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk.h"

#include "ocpn_plugin.h"

#include <wx/intl.h>

namespace {

enum class ChartDldrPolicyTriState { Off, On, WhenPanelVisible };

struct ChartDldrBulkModeProfileRow {
  ChartDldrCatalogRefreshMode catalog_refresh =
      ChartDldrCatalogRefreshMode::SyncBlocking;
  ChartDldrErrorReporting error_reporting = ChartDldrErrorReporting::Dialog;
  ChartDldrPolicyTriState restore_notebook_page = ChartDldrPolicyTriState::Off;
  ChartDldrPolicyTriState select_download_tab = ChartDldrPolicyTriState::Off;
  bool confirm_before_start = false;
  bool show_failure_summary = false;
  bool sync_list_selection = false;
  bool skip_manual_charts = false;
  bool defer_chart_db_apply = false;
  bool allow_empty_selection = false;
  bool show_download_progress_ui = false;
  bool show_download_result_dialogs = false;
  bool show_catalog_progress_dialog = false;
  bool focus_charts_tab_after_catalog = false;
  ChartDldrPolicyTriState defer_catalog_rescan = ChartDldrPolicyTriState::Off;
  ChartDldrTransferPoll transfer_poll = ChartDldrTransferPoll::PollOnly;
};

bool ResolveTriState(ChartDldrPolicyTriState value, bool panel_visible,
                     bool active_when_panel_visible) {
  switch (value) {
    case ChartDldrPolicyTriState::Off:
      return false;
    case ChartDldrPolicyTriState::On:
      return true;
    case ChartDldrPolicyTriState::WhenPanelVisible:
      return active_when_panel_visible ? panel_visible : !panel_visible;
  }
  return false;
}

ChartDldrBulkModeProfileRow InteractiveBulkProfileRow() {
  ChartDldrBulkModeProfileRow row;
  row.catalog_refresh = ChartDldrCatalogRefreshMode::SyncBlocking;
  row.error_reporting = ChartDldrErrorReporting::Dialog;
  row.restore_notebook_page = ChartDldrPolicyTriState::On;
  row.select_download_tab = ChartDldrPolicyTriState::On;
  row.confirm_before_start = true;
  row.show_failure_summary = true;
  row.sync_list_selection = true;
  row.skip_manual_charts = false;
  row.defer_chart_db_apply = true;
  row.allow_empty_selection = true;
  row.show_download_progress_ui = true;
  row.show_download_result_dialogs = true;
  row.show_catalog_progress_dialog = false;
  row.focus_charts_tab_after_catalog = true;
  row.defer_catalog_rescan = ChartDldrPolicyTriState::Off;
  row.transfer_poll = ChartDldrTransferPoll::BlockUntilComplete;
  return row;
}

ChartDldrBulkModeProfileRow ScheduledBulkProfileRow() {
  ChartDldrBulkModeProfileRow row;
  row.catalog_refresh = ChartDldrCatalogRefreshMode::AsyncIdle;
  row.error_reporting = ChartDldrErrorReporting::SummaryLog;
  row.restore_notebook_page = ChartDldrPolicyTriState::WhenPanelVisible;
  row.select_download_tab = ChartDldrPolicyTriState::WhenPanelVisible;
  row.confirm_before_start = false;
  row.show_failure_summary = false;
  row.sync_list_selection = false;
  row.skip_manual_charts = true;
  row.defer_chart_db_apply = true;
  row.allow_empty_selection = true;
  row.show_download_progress_ui = false;
  row.show_download_result_dialogs = false;
  row.show_catalog_progress_dialog = false;
  row.focus_charts_tab_after_catalog = false;
  row.defer_catalog_rescan = ChartDldrPolicyTriState::WhenPanelVisible;
  row.transfer_poll = ChartDldrTransferPoll::PollOnly;
  return row;
}

ChartDldrBulkModeProfileRow SelectedChartsProfileRow() {
  ChartDldrBulkModeProfileRow row;
  row.catalog_refresh = ChartDldrCatalogRefreshMode::SyncBlocking;
  row.error_reporting = ChartDldrErrorReporting::Dialog;
  row.allow_empty_selection = false;
  row.show_download_progress_ui = true;
  row.show_download_result_dialogs = true;
  row.defer_catalog_rescan = ChartDldrPolicyTriState::Off;
  row.transfer_poll = ChartDldrTransferPoll::BlockUntilComplete;
  return row;
}

ChartDldrBulkModeProfileRow CatalogRefreshProfileRow() {
  ChartDldrBulkModeProfileRow row;
  row.catalog_refresh = ChartDldrCatalogRefreshMode::SyncBlocking;
  row.error_reporting = ChartDldrErrorReporting::Dialog;
  row.show_catalog_progress_dialog = true;
  row.focus_charts_tab_after_catalog = true;
  row.defer_catalog_rescan = ChartDldrPolicyTriState::Off;
  row.transfer_poll = ChartDldrTransferPoll::BlockUntilComplete;
  return row;
}

const ChartDldrBulkModeProfileRow& ModeProfileRow(
    const ChartDldrBulkRunUiPolicy& policy) {
  static const ChartDldrBulkModeProfileRow kInteractive =
      InteractiveBulkProfileRow();
  static const ChartDldrBulkModeProfileRow kScheduled =
      ScheduledBulkProfileRow();
  static const ChartDldrBulkModeProfileRow kSelected =
      SelectedChartsProfileRow();
  static const ChartDldrBulkModeProfileRow kCatalog =
      CatalogRefreshProfileRow();

  switch (policy.mode) {
    case ChartDldrDownloadUiMode::InteractiveBulk:
      return kInteractive;
    case ChartDldrDownloadUiMode::ScheduledBulk:
      return kScheduled;
    case ChartDldrDownloadUiMode::SelectedCharts:
      return kSelected;
    case ChartDldrDownloadUiMode::CatalogRefresh:
      return kCatalog;
  }
  return kSelected;
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
  const bool panel_visible = policy.bulk_panel_visible;
  ChartDldrBulkModeProfile profile;
  profile.catalog.refresh = row.catalog_refresh;
  profile.catalog.show_progress_dialog = row.show_catalog_progress_dialog;
  profile.error_reporting = row.error_reporting;
  profile.ui.restore_notebook_page =
      ResolveTriState(row.restore_notebook_page, panel_visible, true);
  profile.ui.select_download_tab =
      ResolveTriState(row.select_download_tab, panel_visible, true);
  profile.ui.confirm_before_start = row.confirm_before_start;
  profile.ui.show_failure_summary = row.show_failure_summary;
  profile.ui.sync_list_selection = row.sync_list_selection;
  profile.ui.show_download_progress_ui = row.show_download_progress_ui;
  profile.ui.show_download_result_dialogs = row.show_download_result_dialogs;
  profile.ui.focus_charts_tab_after_catalog =
      row.focus_charts_tab_after_catalog;
  profile.ui.defer_catalog_rescan =
      ResolveTriState(row.defer_catalog_rescan, panel_visible, false);
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
