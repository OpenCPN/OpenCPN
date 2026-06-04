/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_run.h"

ChartDldrBulkRunUiPolicy ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind kind,
                                                     bool panel_visible) {
  ChartDldrBulkRunUiPolicy policy;
  if (ChartDldrBulkRunIsScheduled(kind)) {
    policy.quiet_downloads = true;
    policy.confirm_before_start = false;
    policy.show_failure_summary = false;
    policy.restore_notebook_page = panel_visible;
    policy.select_download_tab = panel_visible;
    policy.sync_list_selection = false;
    return policy;
  }

  policy.quiet_downloads = false;
  policy.confirm_before_start = true;
  policy.show_failure_summary = true;
  policy.restore_notebook_page = panel_visible;
  policy.select_download_tab = true;
  policy.sync_list_selection = true;
  return policy;
}

bool ChartDldrBulkRunShouldRestoreUi(const ChartDldrBulkRunUiPolicy& policy,
                                     const ChartDldrBulkRunUiSnapshot& before) {
  return policy.restore_notebook_page && before.panel_visible;
}
