/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_STATE_H_
#define CHARTDLDR_BULK_STATE_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_async_step.h"
#include "chartdldr_chart_classify.h"

#include <wx/string.h>

class chartdldr_pi;

/** Per-catalog chart download progress (owned by bulk run session). */
struct ChartDldrChartBulkState {
  int loop_index = 0;
  int pending_index = -1;
  ChartDldrChartUpdateKind pending_kind = ChartDldrChartUpdateKind::None;
  wxString pending_path;
  bool active = false;
  int to_download = 0;
  int downloading = 0;
  int failed = 0;
  int new_downloads = 0;
  int updated_downloads = 0;

  void ResetForCatalogPass() {
    loop_index = 0;
    pending_index = -1;
    pending_kind = ChartDldrChartUpdateKind::None;
    pending_path.clear();
    downloading = 0;
    failed = 0;
    to_download = 0;
    new_downloads = 0;
    updated_downloads = 0;
    active = false;
  }

  ChartDldrBulkRunStats ToStats() const {
    ChartDldrBulkRunStats stats;
    stats.attempted = downloading;
    stats.failed = failed;
    stats.new_downloads = new_downloads;
    stats.updated_downloads = updated_downloads;
    return stats;
  }
};

/** In-flight bulk run state for interactive and scheduled orchestration. */
class ChartDldrBulkRunSession {
public:
  bool IsActive() const { return active_; }
  bool IsScheduled() const { return kind_ == ChartDldrBulkRunKind::Scheduled; }

  chartdldr_pi* Plugin() const { return pi_; }
  const ChartDldrBulkModeProfile& Profile() const { return profile_; }
  const ChartDldrBulkRunUiSnapshot& UiBefore() const { return ui_before_; }

  ChartDldrScheduledBulkRunState& ScheduledState() { return scheduled_; }
  const ChartDldrScheduledBulkRunState& ScheduledState() const {
    return scheduled_;
  }

  ChartDldrChartBulkState& ChartBulk() { return chart_bulk_; }
  const ChartDldrChartBulkState& ChartBulk() const { return chart_bulk_; }

  void Begin(chartdldr_pi* pi, ChartDldrBulkRunKind kind,
             const ChartDldrBulkModeProfile& profile,
             const ChartDldrBulkRunUiSnapshot& ui_before) {
    End();
    pi_ = pi;
    kind_ = kind;
    profile_ = profile;
    ui_before_ = ui_before;
    scheduled_ = ChartDldrScheduledBulkRunState();
    chart_bulk_.ResetForCatalogPass();
    active_ = pi != nullptr;
  }

  void End() {
    active_ = false;
    pi_ = nullptr;
    scheduled_ = ChartDldrScheduledBulkRunState();
    chart_bulk_.ResetForCatalogPass();
  }

  bool WouldYieldOnDownloadEvent() const {
    const ChartDldrTransferPoll poll =
        active_ ? profile_.charts.transfer_poll
                : ChartDldrTransferPoll::BlockUntilComplete;
    return ChartDldrShouldYieldOnDownloadEvent(active_, poll);
  }

private:
  bool active_ = false;
  ChartDldrBulkRunKind kind_ = ChartDldrBulkRunKind::Interactive;
  chartdldr_pi* pi_ = nullptr;
  ChartDldrBulkModeProfile profile_;
  ChartDldrBulkRunUiSnapshot ui_before_;
  ChartDldrScheduledBulkRunState scheduled_;
  ChartDldrChartBulkState chart_bulk_;
};

#endif  // CHARTDLDR_BULK_STATE_H_
