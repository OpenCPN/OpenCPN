/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_SCHEDULED_RUN_LOG_H_
#define CHARTDLDR_SCHEDULED_RUN_LOG_H_

#include "chartdldr_bulk_catalog_run.h"

#include <wx/datetime.h>
#include <wx/string.h>

/** Minutes of unchanged scheduled state before a still-running line. */
inline constexpr int kScheduledStillRunningMinutes = 15;

/** Fingerprint for still-running notes (not used for milestone emission). */
struct ChartDldrScheduledRunLogSnapshot {
  int catalog_index = 0;
  int catalog_count = 0;
  ChartDldrBulkCatalogPhase phase = ChartDldrBulkCatalogPhase::PrepareCatalog;
  bool catalog_refresh_active = false;
  int charts_downloading = 0;
  int charts_to_download = 0;
  int charts_failed = 0;
  int pending_chart_index = -1;
  wxString catalog_name;
  wxString chart_title;
};

/** Pure formatters for scheduled-run operational milestones. */
wxString ChartDldrFormatScheduledStarting(int source_count);
wxString ChartDldrFormatScheduledCatalogRefresh(int catalog_one_based,
                                                int catalog_count,
                                                const wxString& catalog_name,
                                                bool refresh_ok,
                                                int charts_selected);
wxString ChartDldrFormatScheduledDownloading(int chart_one_based,
                                             int charts_to_download,
                                             const wxString& chart_title);
wxString ChartDldrFormatScheduledCatalogDone(int catalog_one_based,
                                             int catalog_count, int updated,
                                             int newly, int failed);
wxString ChartDldrFormatScheduledStillRunning(
    const ChartDldrScheduledRunLogSnapshot& snap, int elapsed_minutes);

/** Inputs for scheduled milestone emission on one bulk step. */
struct ChartDldrScheduledBulkStepLogInput {
  ChartDldrBulkCatalogPhase phase_before =
      ChartDldrBulkCatalogPhase::PrepareCatalog;
  int catalog_index_before = 0;
  int catalog_count = 0;
  int charts_downloading_before = 0;
  int charts_downloading = 0;
  int charts_to_download = 0;
  int pending_chart_index = -1;
  wxString catalog_name;
  wxString chart_title;
};

/**
 * Scheduled-run milestones: emit at known transitions; fingerprint is only
 * for the 15-minute still-running note.
 */
class ChartDldrScheduledRunLogger {
public:
  void Reset();
  void OnStarting(const wxDateTime& now = wxDateTime());

  wxString NoteCatalogRefresh(int catalog_one_based, int catalog_count,
                              const wxString& catalog_name, bool refresh_ok,
                              int charts_selected,
                              const wxDateTime& now = wxDateTime());
  wxString NoteDownloading(int chart_one_based, int charts_to_download,
                           const wxString& chart_title,
                           const wxDateTime& now = wxDateTime());
  wxString NoteCatalogDone(int catalog_one_based, int catalog_count,
                           int updated, int newly, int failed,
                           const wxDateTime& now = wxDateTime());

  /**
   * Emit still-running if fingerprint unchanged for 15 minutes; otherwise
   * update the fingerprint silently when state moved.
   */
  wxString NoteStillRunning(const ChartDldrScheduledRunLogSnapshot& snap,
                            const wxDateTime& now = wxDateTime());

  /** Phase/chart transition notes + still-running; logs non-empty lines. */
  void OnWalkStep(const ChartDldrScheduledBulkStepLogInput& step,
                  ChartDldrBulkWalkStep walk_step, int charts_selected,
                  const ChartDldrBulkRunStats& catalog_counters,
                  const ChartDldrScheduledRunLogSnapshot& after_snap,
                  const wxDateTime& now = wxDateTime());

private:
  bool FingerprintEquals(const ChartDldrScheduledRunLogSnapshot& snap) const;
  void RememberMilestone(const wxDateTime& now);
  void RememberFingerprint(const ChartDldrScheduledRunLogSnapshot& snap,
                           const wxDateTime& now);

  bool active_ = false;
  wxDateTime run_start_;
  wxDateTime last_milestone_;
  ChartDldrScheduledRunLogSnapshot last_;
};

#endif  // CHARTDLDR_SCHEDULED_RUN_LOG_H_
