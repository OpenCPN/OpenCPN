/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_RUN_H_
#define CHARTDLDR_BULK_RUN_H_

enum class ChartDldrBulkRunKind { Interactive, Scheduled };

inline bool ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind kind) {
  return kind == ChartDldrBulkRunKind::Scheduled;
}

/** How a bulk run drives the downloader panel (testable without wx widgets). */
struct ChartDldrBulkRunUiPolicy {
  bool quiet_downloads = false;
  bool confirm_before_start = false;
  bool restore_notebook_page = false;
  bool show_failure_summary = false;
  /** Switch notebook to the download tab before iterating sources. */
  bool select_download_tab = false;
  /** Sync wxListCtrl selection while iterating (interactive only). */
  bool sync_list_selection = false;
};

struct ChartDldrBulkRunUiSnapshot {
  bool panel_visible = false;
  int notebook_page = 0;
};

ChartDldrBulkRunUiPolicy ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind kind,
                                                     bool panel_visible);

bool ChartDldrBulkRunShouldRestoreUi(const ChartDldrBulkRunUiPolicy& policy,
                                     const ChartDldrBulkRunUiSnapshot& before);

#endif  // CHARTDLDR_BULK_RUN_H_
