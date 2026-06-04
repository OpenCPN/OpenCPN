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

enum class ChartDldrBulkRunKind { Interactive, Scheduled };

inline bool ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind kind) {
  return kind == ChartDldrBulkRunKind::Scheduled;
}

/** How download/catalog code reports errors and drives wx UI. */
enum class ChartDldrDownloadUiMode {
  /** User "Update all" from the panel. */
  InteractiveBulk,
  /** Timer-driven bulk while OpenCPN runs. */
  ScheduledBulk,
  /** User "Download selected charts". */
  SelectedCharts,
  /** User refreshes one catalog XML. */
  CatalogRefresh,
};

/**
 * Bulk orchestration lives in ChartDldrBulkRunner / ChartDldrRunBulkUpdate.
 * ChartDldrPanelImpl is only the wx transfer parent for OCPN_download*
 * callbacks.
 */
struct ChartDldrBulkRunUiPolicy {
  ChartDldrDownloadUiMode mode = ChartDldrDownloadUiMode::SelectedCharts;
  /** Set for bulk modes: panel was on screen when the run started. */
  bool bulk_panel_visible = false;
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
};

enum class ChartDldrScheduledSkipReason {
  None,
  BulkDisabled,
  NoSources,
  PanelUnavailable,
};

struct ChartDldrBulkRequestInput {
  bool allow_bulk_update = false;
  bool has_chart_sources = false;
  bool bulk_run_active = false;
};

bool ChartDldrPolicyUsesLogForErrors(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyConfirmBeforeStart(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyRestoreNotebookPage(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyShowFailureSummary(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicySelectDownloadTab(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicySyncListSelection(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicySkipManualCharts(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyDeferChartDbApply(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyAllowEmptySelection(const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyShowDownloadProgressUi(
    const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyShowDownloadResultDialogs(
    const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyShowCatalogProgressDialog(
    const ChartDldrBulkRunUiPolicy& policy);
bool ChartDldrPolicyFocusChartsTabAfterCatalog(
    const ChartDldrBulkRunUiPolicy& policy);

ChartDldrBulkRunUiPolicy ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind kind,
                                                     bool panel_visible);

ChartDldrBulkRunUiPolicy ChartDldrSelectedChartsDownloadPolicy();
ChartDldrBulkRunUiPolicy ChartDldrInteractiveCatalogUpdatePolicy();

bool ChartDldrBulkRunShouldRestoreUi(const ChartDldrBulkRunUiPolicy& policy,
                                     const ChartDldrBulkRunUiSnapshot& before);

bool ChartDldrCanStartBulkRequest(const ChartDldrBulkRequestInput& input,
                                  ChartDldrBulkRunKind kind,
                                  ChartDldrScheduledSkipReason* scheduled_skip);

ChartDldrScheduledSkipReason ChartDldrEvaluateScheduledBulkSkip(
    bool allow_bulk_update, bool has_chart_sources);

wxString ChartDldrScheduledSkipStatus(ChartDldrScheduledSkipReason reason);

#endif  // CHARTDLDR_BULK_H_
