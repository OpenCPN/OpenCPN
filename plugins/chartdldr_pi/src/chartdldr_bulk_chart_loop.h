/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_CHART_LOOP_H_
#define CHARTDLDR_BULK_CHART_LOOP_H_

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_state.h"
#include "chartdldr_temp_download.h"

#include <wx/string.h>

/** Classify from pre-download local state (before install / ChartUpdated). */
ChartDldrChartUpdateKind ChartDldrClassifyChart(bool exists_locally,
                                                bool catalog_newer_than_local);

inline bool ChartDldrChartIndexInRange(int index, int chart_count) {
  return index >= 0 && index < chart_count;
}

void ChartDldrRecordChartDownloadOutcome(bool process_ok,
                                         ChartDldrChartUpdateKind kind,
                                         ChartDldrChartBulkState& chart_bulk);

void ChartDldrRecordChartDownloadFailure(ChartDldrChartBulkState& chart_bulk);

ChartDldrBulkWalkStep ChartDldrBulkChartStepAfterCompletedTransfer(
    bool cancelled, int loop_index, int chart_count, int to_download);

int ChartDldrBulkChartSuccessfulDownloadCount(
    const ChartDldrChartBulkState& chart_bulk);

struct ChartDldrBulkChartDownloadInfo {
  bool needs_manual_download = false;
  wxString download_uri;
  wxString filename;
};

enum class ChartDldrPrepareCheckedChartDownloadResult {
  Ready,
  SkipManual,
  InvalidUrl,
};

ChartDldrPrepareCheckedChartDownloadResult ChartDldrPrepareCheckedChartDownload(
    const ChartDldrBulkChartDownloadInfo& chart, const wxString& target_dir,
    wxString& download_url, ChartDldrTempDownloadPaths& paths);

#endif  // CHARTDLDR_BULK_CHART_LOOP_H_
