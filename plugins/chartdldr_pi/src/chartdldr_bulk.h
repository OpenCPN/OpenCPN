/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_H_
#define CHARTDLDR_BULK_H_

#include <wx/string.h>
#include <wx/window.h>

enum class ChartDldrBulkRunKind { Interactive, Scheduled };

enum class ChartDldrCatalogRefreshMode {
  SyncBlocking,
  AsyncIdle,
};

enum class ChartDldrAsyncCatalogStepResult {
  NotActive,
  InProgress,
  Complete,
  Failed,
};

enum class ChartDldrBulkChartStepResult {
  NotActive,
  TransferInProgress,
  MoreCharts,
  Finished,
};

inline bool ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind kind) {
  return kind == ChartDldrBulkRunKind::Scheduled;
}

/** How download/catalog code reports errors and drives wx UI. */
enum class ChartDldrDownloadUiMode {
  InteractiveBulk,
  ScheduledBulk,
  SelectedCharts,
  CatalogRefresh,
};

enum class ChartDldrErrorReporting {
  Dialog,
  /** Scheduled bulk: one summary line per run, no per-item log noise. */
  SummaryLog,
};

/**
 * Bulk orchestration lives in ChartDldrBulkOrchestrate (panel friend).
 * ChartDldrPanelImpl is only the wx transfer parent for OCPN_download*
 * callbacks.
 */
struct ChartDldrBulkRunUiPolicy {
  ChartDldrDownloadUiMode mode = ChartDldrDownloadUiMode::SelectedCharts;
  bool bulk_panel_visible = false;
};

/** Resolved UI behavior for a bulk run policy. */
struct ChartDldrBulkModeProfile {
  ChartDldrCatalogRefreshMode catalog_refresh =
      ChartDldrCatalogRefreshMode::SyncBlocking;
  ChartDldrErrorReporting error_reporting = ChartDldrErrorReporting::Dialog;
  bool restore_notebook_page = false;
  bool select_download_tab = false;
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
};

struct ChartDldrBulkRunUiSnapshot {
  bool panel_visible = false;
  int notebook_page = 0;
};

struct ChartDldrBulkRunStats {
  int attempted = 0;
  int failed = 0;
  int new_downloads = 0;
  int updated_downloads = 0;

  int downloaded_ok() const { return attempted - failed; }

  void Accumulate(const ChartDldrBulkRunStats& delta) {
    attempted += delta.attempted;
    failed += delta.failed;
    new_downloads += delta.new_downloads;
    updated_downloads += delta.updated_downloads;
  }
};

ChartDldrBulkModeProfile ChartDldrBulkModeProfileFor(
    const ChartDldrBulkRunUiPolicy& policy);

void ChartDldrReportBulkError(wxWindow* parent,
                              const ChartDldrBulkModeProfile& profile,
                              const wxString& msg, const wxString& title);

ChartDldrBulkRunUiPolicy ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind kind,
                                                     bool panel_visible);

ChartDldrBulkModeProfile ChartDldrSelectedChartsDownloadProfile();
ChartDldrBulkModeProfile ChartDldrInteractiveCatalogUpdateProfile();

bool ChartDldrBulkRunShouldRestoreUi(const ChartDldrBulkModeProfile& profile,
                                     const ChartDldrBulkRunUiSnapshot& before);

#endif  // CHARTDLDR_BULK_H_
