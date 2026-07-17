/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_chart_loop.h"

#include <wx/filename.h>
#include <wx/url.h>

ChartDldrChartUpdateKind ChartDldrClassifyChart(bool exists_locally,
                                                bool catalog_newer_than_local) {
  if (!exists_locally) {
    return ChartDldrChartUpdateKind::New;
  }
  if (catalog_newer_than_local) {
    return ChartDldrChartUpdateKind::Updated;
  }
  return ChartDldrChartUpdateKind::None;
}

void ChartDldrRecordChartDownloadFailure(ChartDldrChartBulkState& chart_bulk) {
  chart_bulk.failed++;
}

void ChartDldrRecordChartDownloadOutcome(bool process_ok,
                                         ChartDldrChartUpdateKind kind,
                                         ChartDldrChartBulkState& chart_bulk) {
  if (process_ok) {
    if (kind == ChartDldrChartUpdateKind::New) {
      chart_bulk.new_downloads++;
    } else if (kind == ChartDldrChartUpdateKind::Updated) {
      chart_bulk.updated_downloads++;
    }
    return;
  }
  ChartDldrRecordChartDownloadFailure(chart_bulk);
}

ChartDldrBulkWalkStep ChartDldrBulkChartStepAfterCompletedTransfer(
    bool cancelled, int loop_index, int chart_count, int to_download) {
  if (cancelled) {
    return ChartDldrBulkWalkStep::Advance;
  }
  if (loop_index < chart_count && to_download) {
    return ChartDldrBulkWalkStep::MoreWork;
  }
  return ChartDldrBulkWalkStep::Advance;
}

int ChartDldrBulkChartSuccessfulDownloadCount(
    const ChartDldrChartBulkState& chart_bulk) {
  return chart_bulk.downloading - chart_bulk.failed;
}

ChartDldrPrepareCheckedChartDownloadResult ChartDldrPrepareCheckedChartDownload(
    const ChartDldrBulkChartDownloadInfo& chart, const wxString& target_dir,
    wxString& download_url, ChartDldrTempDownloadPaths& paths) {
  if (chart.needs_manual_download) {
    return ChartDldrPrepareCheckedChartDownloadResult::SkipManual;
  }

  wxURI url(chart.download_uri);
  if (url.IsReference()) {
    return ChartDldrPrepareCheckedChartDownloadResult::InvalidUrl;
  }

  wxFileName fn;
  fn.SetFullName(chart.filename);
  fn.SetPath(target_dir);
  paths = ChartDldrTempDownloadPathsFor(fn.GetFullPath());
  download_url = url.BuildURI();
  return ChartDldrPrepareCheckedChartDownloadResult::Ready;
}
