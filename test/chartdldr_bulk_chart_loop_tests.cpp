/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_chart_loop.h"
#include "chartdldr_temp_download.h"

#include <wx/filename.h>

TEST(ChartDldrBulkChartLoop, SuccessIncrementsNewDownloads) {
  ChartDldrChartBulkState chart_bulk;
  ChartDldrRecordChartDownloadOutcome(true, ChartDldrChartUpdateKind::New,
                                      chart_bulk);
  EXPECT_EQ(chart_bulk.new_downloads, 1);
  EXPECT_EQ(chart_bulk.updated_downloads, 0);
  EXPECT_EQ(chart_bulk.failed, 0);
}

TEST(ChartDldrBulkChartLoop, SuccessIncrementsUpdatedDownloads) {
  ChartDldrChartBulkState chart_bulk;
  ChartDldrRecordChartDownloadOutcome(true, ChartDldrChartUpdateKind::Updated,
                                      chart_bulk);
  EXPECT_EQ(chart_bulk.new_downloads, 0);
  EXPECT_EQ(chart_bulk.updated_downloads, 1);
  EXPECT_EQ(chart_bulk.failed, 0);
}

TEST(ChartDldrBulkChartLoop, FailureIncrementsFailedCount) {
  ChartDldrChartBulkState chart_bulk;
  ChartDldrRecordChartDownloadOutcome(false, ChartDldrChartUpdateKind::New,
                                      chart_bulk);
  EXPECT_EQ(chart_bulk.failed, 1);
  EXPECT_EQ(chart_bulk.new_downloads, 0);
}

TEST(ChartDldrBulkChartLoop, RecordFailureIncrementsFailedCount) {
  ChartDldrChartBulkState chart_bulk;
  ChartDldrRecordChartDownloadFailure(chart_bulk);
  EXPECT_EQ(chart_bulk.failed, 1);
}

TEST(ChartDldrBulkChartLoop, CancelledTransferCompletesPass) {
  EXPECT_EQ(ChartDldrBulkChartStepAfterCompletedTransfer(true, 0, 5, 3),
            ChartDldrBulkWalkStep::Advance);
}

TEST(ChartDldrBulkChartLoop, MoreChartsWhenSelectionRemains) {
  EXPECT_EQ(ChartDldrBulkChartStepAfterCompletedTransfer(false, 2, 5, 3),
            ChartDldrBulkWalkStep::MoreWork);
}

TEST(ChartDldrBulkChartLoop, PassCompleteWhenLoopExhausted) {
  EXPECT_EQ(ChartDldrBulkChartStepAfterCompletedTransfer(false, 5, 5, 3),
            ChartDldrBulkWalkStep::Advance);
}

TEST(ChartDldrBulkChartLoop, SuccessfulDownloadCount) {
  ChartDldrChartBulkState chart_bulk;
  chart_bulk.downloading = 4;
  chart_bulk.failed = 1;
  EXPECT_EQ(ChartDldrBulkChartSuccessfulDownloadCount(chart_bulk), 3);
}

TEST(ChartDldrBulkChartLoop, InFlightCancelCountsAsFailure) {
  ChartDldrChartBulkState chart_bulk;
  chart_bulk.downloading = 1;
  chart_bulk.failed = 0;
  ChartDldrRecordChartDownloadFailure(chart_bulk);
  EXPECT_EQ(chart_bulk.failed, 1);
  EXPECT_EQ(ChartDldrBulkChartSuccessfulDownloadCount(chart_bulk), 0);
}

TEST(ChartDldrBulkChartLoop, PrepareCheckedChartDownloadSkipsManual) {
  ChartDldrBulkChartDownloadInfo chart;
  chart.needs_manual_download = true;
  wxString download_url;
  ChartDldrTempDownloadPaths paths;
  EXPECT_EQ(ChartDldrPrepareCheckedChartDownload(chart, wxT("/tmp"),
                                                 download_url, paths),
            ChartDldrPrepareCheckedChartDownloadResult::SkipManual);
}

TEST(ChartDldrBulkChartLoop, PrepareCheckedChartDownloadRejectsInvalidUrl) {
  ChartDldrBulkChartDownloadInfo chart;
  chart.download_uri = wxT("#fragment");
  chart.filename = wxT("chart.zip");
  wxString download_url;
  ChartDldrTempDownloadPaths paths;
  EXPECT_EQ(ChartDldrPrepareCheckedChartDownload(chart, wxT("/tmp"),
                                                 download_url, paths),
            ChartDldrPrepareCheckedChartDownloadResult::InvalidUrl);
}

TEST(ChartDldrBulkChartLoop, PrepareCheckedChartDownloadBuildsPaths) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_chart_build"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  ChartDldrBulkChartDownloadInfo chart;
  chart.download_uri = wxT("http://example.test/chart.zip");
  chart.filename = wxT("chart.zip");
  wxString download_url;
  ChartDldrTempDownloadPaths paths;
  EXPECT_EQ(ChartDldrPrepareCheckedChartDownload(chart, output_dir.GetPath(),
                                                 download_url, paths),
            ChartDldrPrepareCheckedChartDownloadResult::Ready);
  EXPECT_FALSE(download_url.empty());
  EXPECT_FALSE(paths.output_path.empty());
  EXPECT_FALSE(paths.temp_path.empty());
  EXPECT_FALSE(paths.download_target.empty());
  EXPECT_NE(paths.temp_path, paths.output_path);
  ChartDldrRemoveTempDownload(paths.temp_path);
  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkChartLoop, PrepareCheckedChartDownloadPreservesExistingFile) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_chart_prepare"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString existing_path =
      output_dir.GetPath() + wxFileName::GetPathSeparator() + wxT("chart.zip");
  {
    wxFile file(existing_path, wxFile::write);
    file.Write("existing");
  }

  ChartDldrBulkChartDownloadInfo chart;
  chart.download_uri = wxT("http://example.test/chart.zip");
  chart.filename = wxT("chart.zip");
  wxString download_url;
  ChartDldrTempDownloadPaths paths;
  EXPECT_EQ(ChartDldrPrepareCheckedChartDownload(chart, output_dir.GetPath(),
                                                 download_url, paths),
            ChartDldrPrepareCheckedChartDownloadResult::Ready);
  EXPECT_TRUE(wxFileExists(existing_path));

  ChartDldrRemoveTempDownload(paths.temp_path);
  wxRemoveFile(existing_path);
  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}
