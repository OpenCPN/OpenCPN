/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_SESSION_POLICY_H_
#define CHARTDLDR_BULK_SESSION_POLICY_H_

#include "chartdldr_bulk_lifecycle.h"

#include <wx/intl.h>
#include <wx/string.h>
#include <wx/window.h>

#include <vector>

/** Catalog control model while a bulk run is active. */
struct ChartDldrCatalogControlsState {
  bool list_enabled = true;
  bool mutations_enabled = true;
  bool download_button_enabled = true;
};

inline ChartDldrCatalogControlsState ChartDldrCatalogControlsFor(
    bool run_active, bool cancel_armed) {
  ChartDldrCatalogControlsState state;
  state.list_enabled = !run_active;
  state.mutations_enabled = !run_active;
  state.download_button_enabled = !run_active || cancel_armed;
  return state;
}

inline wxString ChartDldrFormatBytes(double bytes) {
  if (bytes <= 0) {
    return "?";
  }
  return wxString::Format(_T("%.1fMB"), bytes / 1024 / 1024);
}

inline wxString ChartDldrFormatBytes(long bytes) {
  return ChartDldrFormatBytes(static_cast<double>(bytes));
}

/** Session kinds only. Browse / manual catalog refresh use CatalogUiPolicy. */
enum class ChartDldrBulkRunMode {
  InteractiveBulk,
  ScheduledBulk,
  SelectedCharts,
};

inline bool ChartDldrBulkRunModeIsScheduled(ChartDldrBulkRunMode mode) {
  return mode == ChartDldrBulkRunMode::ScheduledBulk;
}

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
 * Frozen session profile compiled once at StartBulk. The run identity is just
 * the mode plus the scheduled UI presentation; every behavioral switch is
 * derived from that pair rather than stored as an independently settable
 * boolean. This removes the previous correlated-boolean matrix, whose product
 * space allowed states that no real run could ever produce.
 */
struct ChartDldrBulkSessionPolicy {
  ChartDldrBulkRunMode mode = ChartDldrBulkRunMode::InteractiveBulk;
  ChartDldrScheduledUiPresentation scheduled_ui =
      ChartDldrScheduledUiPresentation::Silent;
  /** The only genuine per-run payload: manual-download decisions. */
  ChartDldrBulkRunPlan plan;

  bool IsScheduled() const { return ChartDldrBulkRunModeIsScheduled(mode); }

  // Derived run behavior ---------------------------------------------------

  bool SkipManualCharts() const { return IsScheduled(); }
  bool AllowEmptySelection() const {
    return mode != ChartDldrBulkRunMode::SelectedCharts;
  }
  bool ConfirmBeforeStart() const {
    return mode == ChartDldrBulkRunMode::InteractiveBulk;
  }
  bool PreserveChartSelection() const {
    return mode == ChartDldrBulkRunMode::SelectedCharts;
  }
  bool ForceFullSelection() const { return IsScheduled(); }
  /** SelectedCharts: skip prepare and download only the selected catalog. */
  bool BindSelectedCatalogOnly() const {
    return mode == ChartDldrBulkRunMode::SelectedCharts;
  }
  bool UiMaterialize() const {
    return !(IsScheduled() &&
             scheduled_ui == ChartDldrScheduledUiPresentation::Silent);
  }
  bool UiRestoreNotebook() const {
    return mode == ChartDldrBulkRunMode::InteractiveBulk ||
           mode == ChartDldrBulkRunMode::SelectedCharts ||
           ScheduledWithProgress();
  }
  bool UiSelectDownloadTab() const {
    return mode == ChartDldrBulkRunMode::InteractiveBulk ||
           ScheduledWithProgress();
  }
  bool UiShowDownloadProgress() const {
    return mode == ChartDldrBulkRunMode::InteractiveBulk ||
           mode == ChartDldrBulkRunMode::SelectedCharts ||
           ScheduledWithProgress();
  }
  bool FocusChartsAfter() const {
    return mode == ChartDldrBulkRunMode::InteractiveBulk;
  }
  ChartDldrErrorReporting ErrorReporting() const {
    return IsScheduled() ? ChartDldrErrorReporting::SummaryLog
                         : ChartDldrErrorReporting::Dialog;
  }

  bool PreselectNew(bool pref_new) const {
    return ForceFullSelection() ? true : pref_new;
  }
  bool PreselectUpdated(bool pref_updated) const {
    return ForceFullSelection() ? true : pref_updated;
  }

  /** Catalog list apply surface compiled from session + user prefs. */
  ChartDldrCatalogUiPolicy CatalogApply(bool pref_new, bool pref_updated) const;

private:
  bool ScheduledWithProgress() const {
    return IsScheduled() &&
           scheduled_ui == ChartDldrScheduledUiPresentation::WithProgress;
  }
};

ChartDldrBulkSessionPolicy ChartDldrBulkSessionPolicyFor(
    ChartDldrBulkRunMode mode, bool panel_visible,
    ChartDldrBulkRunPlan plan = ChartDldrBulkRunPlan());

ChartDldrCatalogUiPolicy ChartDldrBrowseCatalogUiPolicy(bool pref_new,
                                                        bool pref_updated);
ChartDldrCatalogUiPolicy ChartDldrManualCatalogRefreshUiPolicy();

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
