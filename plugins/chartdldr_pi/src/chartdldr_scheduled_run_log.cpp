/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_scheduled_run_log.h"

#include <wx/log.h>

namespace {

wxDateTime ResolveNow(const wxDateTime& now) {
  return now.IsValid() ? now : wxDateTime::Now();
}

wxString QuoteName(const wxString& name) {
  if (name.empty()) {
    return wxT("\"?\"");
  }
  return wxString::Format(wxT("\"%s\""), name.c_str());
}

void LogIfPresent(const wxString& line) {
  if (!line.empty()) {
    wxLogMessage("%s", line.mb_str());
  }
}

}  // namespace

wxString ChartDldrFormatScheduledStarting(int source_count) {
  return wxString::Format(
      wxT("chartdldr_pi: scheduled bulk update starting (%d sources)"),
      source_count);
}

wxString ChartDldrFormatScheduledCatalogRefresh(int catalog_one_based,
                                                int catalog_count,
                                                const wxString& catalog_name,
                                                bool refresh_ok,
                                                int charts_selected) {
  if (!refresh_ok) {
    return wxString::Format(
        wxT("chartdldr_pi: catalog %d/%d %s: refresh failed"),
        catalog_one_based, catalog_count, QuoteName(catalog_name).c_str());
  }
  return wxString::Format(
      wxT("chartdldr_pi: catalog %d/%d %s: refresh ok, %d selected"),
      catalog_one_based, catalog_count, QuoteName(catalog_name).c_str(),
      charts_selected);
}

wxString ChartDldrFormatScheduledDownloading(int chart_one_based,
                                             int charts_to_download,
                                             const wxString& chart_title) {
  return wxString::Format(wxT("chartdldr_pi: downloading %d/%d %s"),
                          chart_one_based, charts_to_download,
                          QuoteName(chart_title).c_str());
}

wxString ChartDldrFormatScheduledCatalogDone(int catalog_one_based,
                                             int catalog_count, int updated,
                                             int newly, int failed) {
  return wxString::Format(
      wxT("chartdldr_pi: catalog %d/%d done (%d update, %d new, %d failed)"),
      catalog_one_based, catalog_count, updated, newly, failed);
}

wxString ChartDldrFormatScheduledStillRunning(
    const ChartDldrScheduledRunLogSnapshot& snap, int elapsed_minutes) {
  if (snap.phase == ChartDldrBulkCatalogPhase::DownloadChart &&
      snap.charts_to_download > 0) {
    return wxString::Format(
        wxT("chartdldr_pi: scheduled bulk still running: catalog %d/%d "
            "downloading %d/%d (%dm elapsed)"),
        snap.catalog_index + 1, snap.catalog_count, snap.charts_downloading,
        snap.charts_to_download, elapsed_minutes);
  }
  if (snap.catalog_refresh_active) {
    return wxString::Format(
        wxT("chartdldr_pi: scheduled bulk still running: catalog %d/%d "
            "refreshing %s (%dm elapsed)"),
        snap.catalog_index + 1, snap.catalog_count,
        QuoteName(snap.catalog_name).c_str(), elapsed_minutes);
  }
  return wxString::Format(
      wxT("chartdldr_pi: scheduled bulk still running: catalog %d/%d (%dm "
          "elapsed)"),
      snap.catalog_index + 1, snap.catalog_count, elapsed_minutes);
}

void ChartDldrScheduledRunLogger::Reset() {
  active_ = false;
  run_start_ = wxDateTime();
  last_milestone_ = wxDateTime();
  last_ = ChartDldrScheduledRunLogSnapshot();
}

void ChartDldrScheduledRunLogger::OnStarting(const wxDateTime& now) {
  Reset();
  active_ = true;
  run_start_ = ResolveNow(now);
  last_milestone_ = run_start_;
}

bool ChartDldrScheduledRunLogger::FingerprintEquals(
    const ChartDldrScheduledRunLogSnapshot& snap) const {
  return last_.catalog_index == snap.catalog_index &&
         last_.phase == snap.phase &&
         last_.catalog_refresh_active == snap.catalog_refresh_active &&
         last_.pending_chart_index == snap.pending_chart_index &&
         last_.charts_downloading == snap.charts_downloading &&
         last_.charts_to_download == snap.charts_to_download &&
         last_.charts_failed == snap.charts_failed;
}

void ChartDldrScheduledRunLogger::RememberMilestone(const wxDateTime& now) {
  last_milestone_ = ResolveNow(now);
}

void ChartDldrScheduledRunLogger::RememberFingerprint(
    const ChartDldrScheduledRunLogSnapshot& snap, const wxDateTime& now) {
  last_ = snap;
  last_milestone_ = ResolveNow(now);
}

wxString ChartDldrScheduledRunLogger::NoteCatalogRefresh(
    int catalog_one_based, int catalog_count, const wxString& catalog_name,
    bool refresh_ok, int charts_selected, const wxDateTime& now) {
  if (!active_) {
    return wxString();
  }
  RememberMilestone(now);
  return ChartDldrFormatScheduledCatalogRefresh(catalog_one_based,
                                                catalog_count, catalog_name,
                                                refresh_ok, charts_selected);
}

wxString ChartDldrScheduledRunLogger::NoteDownloading(
    int chart_one_based, int charts_to_download, const wxString& chart_title,
    const wxDateTime& now) {
  if (!active_) {
    return wxString();
  }
  RememberMilestone(now);
  return ChartDldrFormatScheduledDownloading(chart_one_based,
                                             charts_to_download, chart_title);
}

wxString ChartDldrScheduledRunLogger::NoteCatalogDone(int catalog_one_based,
                                                      int catalog_count,
                                                      int updated, int newly,
                                                      int failed,
                                                      const wxDateTime& now) {
  if (!active_) {
    return wxString();
  }
  RememberMilestone(now);
  return ChartDldrFormatScheduledCatalogDone(catalog_one_based, catalog_count,
                                             updated, newly, failed);
}

wxString ChartDldrScheduledRunLogger::NoteStillRunning(
    const ChartDldrScheduledRunLogSnapshot& snap, const wxDateTime& now) {
  if (!active_) {
    return wxString();
  }
  const wxDateTime when = ResolveNow(now);

  if (FingerprintEquals(snap) && last_milestone_.IsValid() &&
      (when - last_milestone_).GetMinutes() >= kScheduledStillRunningMinutes) {
    const int elapsed =
        run_start_.IsValid() ? (when - run_start_).GetMinutes() : 0;
    RememberFingerprint(snap, when);
    return ChartDldrFormatScheduledStillRunning(snap, elapsed);
  }

  if (!FingerprintEquals(snap)) {
    // State moved without a named milestone; keep the still-running clock
    // honest.
    RememberFingerprint(snap, when);
  }
  return wxString();
}

void ChartDldrScheduledRunLogger::OnWalkStep(
    const ChartDldrScheduledBulkStepLogInput& step,
    ChartDldrBulkWalkStep walk_step, int charts_selected,
    const ChartDldrBulkRunStats& catalog_counters,
    const ChartDldrScheduledRunLogSnapshot& after_snap, const wxDateTime& now) {
  if (step.phase_before == ChartDldrBulkCatalogPhase::PrepareCatalog &&
      (walk_step == ChartDldrBulkWalkStep::CatalogReady ||
       walk_step == ChartDldrBulkWalkStep::Advance)) {
    LogIfPresent(NoteCatalogRefresh(
        step.catalog_index_before + 1, step.catalog_count, step.catalog_name,
        walk_step == ChartDldrBulkWalkStep::CatalogReady, charts_selected,
        now));
  }

  if (step.phase_before == ChartDldrBulkCatalogPhase::DownloadChart &&
      step.pending_chart_index >= 0 &&
      step.charts_downloading > step.charts_downloading_before) {
    LogIfPresent(NoteDownloading(step.charts_downloading,
                                 step.charts_to_download, step.chart_title,
                                 now));
  }

  if (step.phase_before == ChartDldrBulkCatalogPhase::DownloadChart &&
      walk_step == ChartDldrBulkWalkStep::Advance) {
    LogIfPresent(NoteCatalogDone(
        step.catalog_index_before + 1, step.catalog_count,
        catalog_counters.updated_downloads, catalog_counters.new_downloads,
        catalog_counters.failed, now));
  }

  LogIfPresent(NoteStillRunning(after_snap, now));
}
