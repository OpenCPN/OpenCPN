/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_LIFECYCLE_H_
#define CHARTDLDR_BULK_LIFECYCLE_H_

#include <wx/string.h>
#include <wx/window.h>

enum class ChartDldrErrorReporting {
  Dialog,
  /** Scheduled bulk: one summary line per run, no per-item log noise. */
  SummaryLog,
};

/** Options close may leave a scheduled run alone; destroy always tears down. */
enum class ChartDldrBulkCancelScope {
  OptionsClosed,
  PluginShutdown,
};

struct ChartDldrBulkRunStats {
  int attempted = 0;
  int failed = 0;
  int new_downloads = 0;
  int updated_downloads = 0;
  int catalog_refresh_failures = 0;

  int downloaded_ok() const { return attempted - failed; }

  void Accumulate(const ChartDldrBulkRunStats& delta) {
    attempted += delta.attempted;
    failed += delta.failed;
    new_downloads += delta.new_downloads;
    updated_downloads += delta.updated_downloads;
    catalog_refresh_failures += delta.catalog_refresh_failures;
  }
};

/**
 * Why a bulk session ends. One funnel: TeardownBulkSession(reason).
 * Completed runs finalize + optional chart DB apply + scheduled stats finish.
 * Other reasons cancel downloads and never apply the chart DB.
 */
enum class ChartDldrBulkTeardownReason {
  Completed,
  UserCancelled,
  Shutdown,
  FailedStart,
};

inline ChartDldrBulkTeardownReason ChartDldrBulkTeardownReasonAfterLoop(
    bool user_cancelled) {
  return user_cancelled ? ChartDldrBulkTeardownReason::UserCancelled
                        : ChartDldrBulkTeardownReason::Completed;
}

inline bool ChartDldrShouldPreserveScheduledRun(ChartDldrBulkCancelScope scope,
                                                bool scheduled_run_active) {
  return scope == ChartDldrBulkCancelScope::OptionsClosed &&
         scheduled_run_active;
}

inline ChartDldrBulkTeardownReason ChartDldrTeardownReasonForCancelScope(
    ChartDldrBulkCancelScope scope) {
  return scope == ChartDldrBulkCancelScope::PluginShutdown
             ? ChartDldrBulkTeardownReason::Shutdown
             : ChartDldrBulkTeardownReason::UserCancelled;
}

inline bool ChartDldrBulkSessionShouldCancelGlobalDownloads(
    ChartDldrBulkTeardownReason reason) {
  return reason == ChartDldrBulkTeardownReason::UserCancelled ||
         reason == ChartDldrBulkTeardownReason::Shutdown;
}

inline bool ChartDldrBulkSessionIsSuccessfulComplete(
    ChartDldrBulkTeardownReason reason) {
  return reason == ChartDldrBulkTeardownReason::Completed;
}

enum class ChartDldrBulkRunStatusPresentation {
  ScheduleLog,
  InteractiveDialog,
};

enum class ChartDldrBulkPostflightKind {
  None,
  Summary,
  Cancelled,
};

/** One non-modal result emitted after teardown; Shutdown always yields None. */
struct ChartDldrBulkPostflight {
  ChartDldrBulkPostflightKind kind = ChartDldrBulkPostflightKind::None;
  wxString message;
};

struct ChartDldrBulkSessionPolicy;

wxString ChartDldrBulkChartFailureMessage(int failed, int attempted);
wxString ChartDldrBulkRunStatusMessage(
    const ChartDldrBulkRunStats& stats,
    ChartDldrBulkRunStatusPresentation presentation);
ChartDldrBulkPostflight ChartDldrBulkPostflightFor(
    ChartDldrBulkTeardownReason reason,
    const ChartDldrBulkSessionPolicy& policy,
    const ChartDldrBulkRunStats& stats);

/** Nesting depth for suppressing modal error dialogs during a CallAfter pump
 * turn. Not a reentrancy guard for the walk itself (that is pump_reentrant_).
 */
void ChartDldrEnterBulkModalSuppress();
void ChartDldrLeaveBulkModalSuppress();
bool ChartDldrBulkModalsSuppressed();

void ChartDldrReportBulkError(wxWindow* parent,
                              ChartDldrErrorReporting reporting,
                              const wxString& msg, const wxString& title);

/** Sealed session end: one applicator reads these predicates. */
struct ChartDldrBulkSessionEnd {
  ChartDldrBulkTeardownReason reason = ChartDldrBulkTeardownReason::Completed;
  bool scheduled = false;
  ChartDldrBulkRunStats stats;

  enum class ScheduledFinish { None, Complete, Abort };

  bool ShouldCopyStats() const;
  bool ShouldCancelPanel() const;
  bool ShouldCancelGlobal() const;
  bool ShouldFinalizeUi() const;
  bool ShouldApplyChartDb() const;
  ScheduledFinish Scheduled() const;
};

ChartDldrBulkSessionEnd ChartDldrBulkSessionEndFor(
    ChartDldrBulkTeardownReason reason, bool scheduled,
    const ChartDldrBulkRunStats& stats);

#endif  // CHARTDLDR_BULK_LIFECYCLE_H_
