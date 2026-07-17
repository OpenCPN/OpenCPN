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

/** Scheduled bulk UI side effects when the panel is visible vs hidden. */
enum class ChartDldrScheduledUiPresentation {
  Silent,
  WithProgress,
};

ChartDldrScheduledUiPresentation ChartDldrScheduledUiPresentationFor(
    bool panel_visible);

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

enum class ChartDldrManualDownloadPolicy {
  Skip,
  /** User consented before session; open URLs discovered during the walk. */
  OpenAsDiscovered,
};

/** Immutable decisions completed before a bulk session starts. */
struct ChartDldrBulkRunPlan {
  bool allow_start = true;
  ChartDldrManualDownloadPolicy manual_policy =
      ChartDldrManualDownloadPolicy::Skip;
  std::vector<ChartDldrManualDownloadAction> manual_downloads;
};

ChartDldrBulkRunPlan ChartDldrSelectedChartsPreflightPlanFor(
    const std::vector<ChartDldrBulkPreflightChart>& charts,
    const wxString& target_dir);

inline bool ChartDldrBulkPlanOpensDiscoveredManualUrls(
    const ChartDldrBulkRunPlan& plan) {
  return plan.manual_policy == ChartDldrManualDownloadPolicy::OpenAsDiscovered;
}

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
 * Frozen session profile compiled once at StartBulk. The run identity is the
 * mode plus the scheduled UI presentation and manual-download plan; the
 * StartBulk side effects (preflight, confirm, manual plan, catalog binding)
 * are compiled into explicit fields by ChartDldrBulkSessionPolicyFor rather
 * than re-derived from the mode at each call site.
 */
struct ChartDldrBulkSessionPolicy {
  ChartDldrBulkRunMode mode = ChartDldrBulkRunMode::InteractiveBulk;
  ChartDldrScheduledUiPresentation scheduled_ui =
      ChartDldrScheduledUiPresentation::Silent;
  /** The only genuine per-run payload: manual-download decisions. */
  ChartDldrBulkRunPlan plan;

  // Compiled StartBulk side effects (set once in
  // ChartDldrBulkSessionPolicyFor).
  /** SelectedCharts resolves + executes its manual-download plan before start.
   */
  bool manual_plan_before_start = false;
  /** InteractiveBulk asks the user to confirm before starting. */
  bool confirm_before_start = false;
  /** How the walk binds catalogs at start. */
  ChartDldrBulkWalkBind walk_bind = ChartDldrBulkWalkBind::AllCatalogs;

  bool IsScheduled() const { return ChartDldrBulkRunModeIsScheduled(mode); }

  // Derived run behavior ---------------------------------------------------

  /** Scheduled runs silently skip charts that need a manual browser URL. */
  bool SkipManualUrlCharts() const { return IsScheduled(); }
  bool AllowEmptySelection() const {
    return walk_bind != ChartDldrBulkWalkBind::SingleDownload;
  }
  bool PreserveChartSelection() const {
    return walk_bind == ChartDldrBulkWalkBind::SingleDownload;
  }
  /**
   * A session that materializes the UI also restores the notebook page on
   * teardown; the other Ui* surfaces below are narrowings of this one.
   */
  bool UiMaterialize() const {
    return !(IsScheduled() &&
             scheduled_ui == ChartDldrScheduledUiPresentation::Silent);
  }
  bool UiShowDownloadProgress() const {
    return UiMaterialize() && walk_bind != ChartDldrBulkWalkBind::SinglePrepare;
  }
  bool UiSelectDownloadTab() const {
    return UiShowDownloadProgress() &&
           walk_bind != ChartDldrBulkWalkBind::SingleDownload;
  }
  bool FocusChartsAfter() const {
    return !IsScheduled() && walk_bind != ChartDldrBulkWalkBind::SingleDownload;
  }
  ChartDldrErrorReporting ErrorReporting() const {
    return IsScheduled() ? ChartDldrErrorReporting::SummaryLog
                         : ChartDldrErrorReporting::Dialog;
  }

  bool PreselectNew(bool pref_new) const {
    return IsScheduled() ? true : pref_new;
  }
  bool PreselectUpdated(bool pref_updated) const {
    return IsScheduled() ? true : pref_updated;
  }

  /** Catalog list apply surface compiled from session + user prefs. */
  ChartDldrCatalogUiPolicy CatalogApply(bool pref_new, bool pref_updated) const;
};

ChartDldrBulkSessionPolicy ChartDldrBulkSessionPolicyFor(
    ChartDldrBulkRunMode mode, bool panel_visible,
    ChartDldrBulkRunPlan plan = ChartDldrBulkRunPlan());

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
  ChartDldrReportBulkError(parent, policy.ErrorReporting(), msg, title);
}

inline bool ChartDldrVerboseExtractLog(
    const ChartDldrBulkSessionPolicy& policy) {
  return policy.ErrorReporting() != ChartDldrErrorReporting::SummaryLog;
}

#endif  // CHARTDLDR_BULK_SESSION_POLICY_H_
