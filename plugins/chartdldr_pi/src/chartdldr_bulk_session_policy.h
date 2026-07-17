/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_SESSION_POLICY_H_
#define CHARTDLDR_BULK_SESSION_POLICY_H_

#include "chartdldr_bulk_lifecycle.h"

#include <wx/string.h>
#include <wx/window.h>

#include <vector>

/** Session kinds only. Browse uses CatalogUiPolicy without a session. */
enum class ChartDldrBulkRunMode {
  InteractiveBulk,
  ScheduledBulk,
  SelectedCharts,
  /** Single-catalog async refresh; no chart download pass. */
  CatalogRefresh,
};

inline bool ChartDldrBulkRunModeIsScheduled(ChartDldrBulkRunMode mode) {
  return mode == ChartDldrBulkRunMode::ScheduledBulk;
}

/** How a bulk session binds the catalog walk. */
enum class ChartDldrBulkWalkBind {
  /** Walk every configured catalog (interactive / scheduled bulk). */
  AllCatalogs,
  /** Bind the selected catalog and run its chart-download pass only. */
  SingleDownload,
  /** Bind the selected catalog and run its catalog-refresh (prepare) only. */
  SinglePrepare,
};

struct ChartDldrManualDownloadAction {
  wxString title;
  wxString url;
  wxString target_dir;
};

struct ChartDldrBulkPreflightChart {
  bool checked = false;
  bool needs_manual_download = false;
  wxString title;
  wxString manual_url;
};

/**
 * Manual-download decisions resolved before a bulk session starts.
 * SelectedCharts fills this preflight (the charts are known up front);
 * InteractiveBulk collects actions on the session during the walk instead.
 */
struct ChartDldrBulkRunPlan {
  bool allow_start = true;
  std::vector<ChartDldrManualDownloadAction> manual_downloads;
};

ChartDldrBulkRunPlan ChartDldrSelectedChartsPreflightPlanFor(
    const std::vector<ChartDldrBulkPreflightChart>& charts,
    const wxString& target_dir);

/**
 * Non-session catalog UI (SetSource browse / manual Update chart list).
 * Also the projection of SessionPolicy used by catalog apply paths.
 */
struct ChartDldrCatalogUiPolicy {
  bool materialize = true;
  bool preserve_selection = false;
  bool preselect_new = true;
  bool preselect_updated = true;
  bool focus_charts_after = false;
};

/**
 * Frozen session profile compiled once by ChartDldrBulkSessionPolicyFor at
 * StartBulk. Every behavior below is a plain field: nothing is re-derived
 * from the mode at call sites.
 */
struct ChartDldrBulkSessionPolicy {
  ChartDldrBulkRunMode mode = ChartDldrBulkRunMode::InteractiveBulk;

  // Compiled StartBulk side effects.
  /** SelectedCharts resolves + executes its manual-download plan before start.
   */
  bool manual_plan_before_start = false;
  /** InteractiveBulk asks the user to confirm before starting. */
  bool confirm_before_start = false;
  /** How the walk binds catalogs at start. */
  ChartDldrBulkWalkBind walk_bind = ChartDldrBulkWalkBind::AllCatalogs;

  // Compiled run behavior.
  bool scheduled = false;
  /** Scheduled runs silently skip charts that need a manual browser URL. */
  bool skip_manual_url_charts = false;
  /** InteractiveBulk records manual-URL charts; opened on successful end. */
  bool collect_manual_urls = false;
  bool allow_empty_selection = true;
  bool preserve_chart_selection = false;
  /** Scheduled runs preselect all charts regardless of user prefs. */
  bool preselect_all_charts = false;
  /** Materialized sessions also restore the notebook page on teardown; the
   * two ui_* fields below are narrowings of this one. */
  bool ui_materialize = true;
  bool ui_show_download_progress = true;
  bool ui_select_download_tab = true;
  bool focus_charts_after = false;
  ChartDldrErrorReporting error_reporting = ChartDldrErrorReporting::Dialog;

  /** Catalog list apply surface compiled from session + user prefs. */
  ChartDldrCatalogUiPolicy CatalogApply(bool pref_new, bool pref_updated) const;
};

ChartDldrBulkSessionPolicy ChartDldrBulkSessionPolicyFor(
    ChartDldrBulkRunMode mode, bool panel_visible);

ChartDldrCatalogUiPolicy ChartDldrBrowseCatalogUiPolicy(bool pref_new,
                                                        bool pref_updated);

struct ChartDldrBulkRunUiSnapshot {
  bool panel_visible = false;
  int notebook_page = 0;
};

bool ChartDldrBulkRunShouldRestoreUi(const ChartDldrBulkSessionPolicy& policy,
                                     const ChartDldrBulkRunUiSnapshot& before);

inline void ChartDldrReportBulkError(wxWindow* parent,
                                     const ChartDldrBulkSessionPolicy& policy,
                                     const wxString& msg,
                                     const wxString& title) {
  ChartDldrReportBulkError(parent, policy.error_reporting, msg, title);
}

inline bool ChartDldrVerboseExtractLog(
    const ChartDldrBulkSessionPolicy& policy) {
  return policy.error_reporting != ChartDldrErrorReporting::SummaryLog;
}

#endif  // CHARTDLDR_BULK_SESSION_POLICY_H_
