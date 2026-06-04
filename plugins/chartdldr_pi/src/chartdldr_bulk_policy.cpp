/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk.h"

#include <wx/intl.h>

bool ChartDldrPolicyUsesLogForErrors(const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::ScheduledBulk;
}

bool ChartDldrPolicyConfirmBeforeStart(const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::InteractiveBulk;
}

bool ChartDldrPolicyRestoreNotebookPage(
    const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::InteractiveBulk ||
         (policy.mode == ChartDldrDownloadUiMode::ScheduledBulk &&
          policy.bulk_panel_visible);
}

bool ChartDldrPolicyShowFailureSummary(const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::InteractiveBulk;
}

bool ChartDldrPolicySelectDownloadTab(const ChartDldrBulkRunUiPolicy& policy) {
  if (policy.mode == ChartDldrDownloadUiMode::InteractiveBulk) {
    return true;
  }
  return policy.mode == ChartDldrDownloadUiMode::ScheduledBulk &&
         policy.bulk_panel_visible;
}

bool ChartDldrPolicySyncListSelection(const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::InteractiveBulk;
}

bool ChartDldrPolicySkipManualCharts(const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::ScheduledBulk;
}

bool ChartDldrPolicyDeferChartDbApply(const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::InteractiveBulk ||
         policy.mode == ChartDldrDownloadUiMode::ScheduledBulk;
}

bool ChartDldrPolicyAllowEmptySelection(
    const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::InteractiveBulk ||
         policy.mode == ChartDldrDownloadUiMode::ScheduledBulk;
}

bool ChartDldrPolicyShowDownloadProgressUi(
    const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::InteractiveBulk ||
         policy.mode == ChartDldrDownloadUiMode::SelectedCharts;
}

bool ChartDldrPolicyShowDownloadResultDialogs(
    const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::SelectedCharts;
}

bool ChartDldrPolicyShowCatalogProgressDialog(
    const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::CatalogRefresh;
}

bool ChartDldrPolicyFocusChartsTabAfterCatalog(
    const ChartDldrBulkRunUiPolicy& policy) {
  return policy.mode == ChartDldrDownloadUiMode::InteractiveBulk;
}

ChartDldrBulkRunUiPolicy ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind kind,
                                                     bool panel_visible) {
  ChartDldrBulkRunUiPolicy policy;
  policy.bulk_panel_visible = panel_visible;
  policy.mode = ChartDldrBulkRunIsScheduled(kind)
                    ? ChartDldrDownloadUiMode::ScheduledBulk
                    : ChartDldrDownloadUiMode::InteractiveBulk;
  return policy;
}

ChartDldrBulkRunUiPolicy ChartDldrSelectedChartsDownloadPolicy() {
  ChartDldrBulkRunUiPolicy policy;
  policy.mode = ChartDldrDownloadUiMode::SelectedCharts;
  return policy;
}

ChartDldrBulkRunUiPolicy ChartDldrInteractiveCatalogUpdatePolicy() {
  ChartDldrBulkRunUiPolicy policy;
  policy.mode = ChartDldrDownloadUiMode::CatalogRefresh;
  return policy;
}

bool ChartDldrBulkRunShouldRestoreUi(const ChartDldrBulkRunUiPolicy& policy,
                                     const ChartDldrBulkRunUiSnapshot& before) {
  return ChartDldrPolicyRestoreNotebookPage(policy) && before.panel_visible;
}

bool ChartDldrCanStartBulkRequest(
    const ChartDldrBulkRequestInput& input, ChartDldrBulkRunKind kind,
    ChartDldrScheduledSkipReason* scheduled_skip) {
  if (input.bulk_run_active) {
    return false;
  }

  if (ChartDldrBulkRunIsScheduled(kind)) {
    const ChartDldrScheduledSkipReason reason =
        ChartDldrEvaluateScheduledBulkSkip(input.allow_bulk_update,
                                           input.has_chart_sources);
    if (scheduled_skip) {
      *scheduled_skip = reason;
    }
    return reason == ChartDldrScheduledSkipReason::None;
  }

  if (scheduled_skip) {
    *scheduled_skip = ChartDldrScheduledSkipReason::None;
  }
  return input.allow_bulk_update && input.has_chart_sources;
}

ChartDldrScheduledSkipReason ChartDldrEvaluateScheduledBulkSkip(
    bool allow_bulk_update, bool has_chart_sources) {
  if (!allow_bulk_update) {
    return ChartDldrScheduledSkipReason::BulkDisabled;
  }
  if (!has_chart_sources) {
    return ChartDldrScheduledSkipReason::NoSources;
  }
  return ChartDldrScheduledSkipReason::None;
}

wxString ChartDldrScheduledSkipStatus(ChartDldrScheduledSkipReason reason) {
  switch (reason) {
    case ChartDldrScheduledSkipReason::BulkDisabled:
      return _("bulk update disabled in preferences");
    case ChartDldrScheduledSkipReason::NoSources:
      return _("no chart sources configured");
    case ChartDldrScheduledSkipReason::PanelUnavailable:
      return _("downloader panel unavailable");
    case ChartDldrScheduledSkipReason::None:
      break;
  }
  return wxEmptyString;
}
