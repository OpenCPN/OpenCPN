/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <vector>

#include "chartdldr_schedule_state.h"
#include "chartdldr_scheduled_run_log.h"

TEST(ChartDldrScheduledRunLog, FormatsStartingAndCatalogMilestones) {
  EXPECT_EQ(ChartDldrFormatScheduledStarting(3),
            wxT("chartdldr_pi: scheduled bulk update starting (3 sources)"));
  EXPECT_EQ(ChartDldrFormatScheduledCatalogRefresh(1, 3, wxT("CA"), true, 0),
            wxT("chartdldr_pi: catalog 1/3 \"CA\": refresh ok, 0 selected"));
  EXPECT_EQ(ChartDldrFormatScheduledCatalogRefresh(2, 3, wxT("OR"), false, 0),
            wxT("chartdldr_pi: catalog 2/3 \"OR\": refresh failed"));
  EXPECT_EQ(ChartDldrFormatScheduledDownloading(1, 2, wxT("US5CA1CK")),
            wxT("chartdldr_pi: downloading 1/2 \"US5CA1CK\""));
  EXPECT_EQ(ChartDldrFormatScheduledCatalogDone(2, 3, 1, 0, 0),
            wxT("chartdldr_pi: catalog 2/3 done (1 update, 0 new, 0 failed)"));
}

TEST(ChartDldrScheduledRunLog, NoteCatalogRefreshEmitsAndSkipsIdlePoll) {
  ChartDldrScheduledRunLogger logger;
  const wxDateTime t0(1, wxDateTime::Jan, 2026, 21, 0, 0);
  logger.OnStarting(t0);

  EXPECT_EQ(logger.NoteCatalogRefresh(1, 3, wxT("CA"), true, 0, t0),
            wxT("chartdldr_pi: catalog 1/3 \"CA\": refresh ok, 0 selected"));

  ChartDldrScheduledRunLogSnapshot idle;
  idle.catalog_index = 1;
  idle.catalog_count = 3;
  idle.phase = ChartDldrBulkCatalogPhase::PrepareCatalog;
  idle.catalog_refresh_active = true;
  idle.catalog_name = wxT("OR");
  EXPECT_TRUE(logger.NoteStillRunning(idle, t0 + wxTimeSpan(0, 1, 0)).empty());
}

TEST(ChartDldrScheduledRunLog, NoteStillRunningAfterStallWindow) {
  ChartDldrScheduledRunLogger logger;
  const wxDateTime t0(1, wxDateTime::Jan, 2026, 21, 0, 0);
  logger.OnStarting(t0);

  EXPECT_TRUE(logger.NoteDownloading(1, 1, wxT("US5CA1CK"), t0)
                  .Contains(wxT("downloading 1/1")));

  ChartDldrScheduledRunLogSnapshot snap;
  snap.catalog_index = 0;
  snap.catalog_count = 3;
  snap.phase = ChartDldrBulkCatalogPhase::DownloadChart;
  snap.charts_downloading = 1;
  snap.charts_to_download = 1;
  snap.pending_chart_index = 4;
  snap.catalog_name = wxT("CA");
  snap.chart_title = wxT("US5CA1CK");

  // Establish fingerprint after download-start milestone.
  EXPECT_TRUE(logger.NoteStillRunning(snap, t0).empty());
  EXPECT_TRUE(logger.NoteStillRunning(snap, t0 + wxTimeSpan(0, 5, 0)).empty());

  const wxString still =
      logger.NoteStillRunning(snap, t0 + wxTimeSpan(0, 15, 0));
  EXPECT_EQ(still,
            wxT("chartdldr_pi: scheduled bulk still running: catalog 1/3 "
                "downloading 1/1 (15m elapsed)"));
}

TEST(ChartDldrScheduledRunLog, NoteCatalogDoneOnChartPassFinish) {
  ChartDldrScheduledRunLogger logger;
  logger.OnStarting(wxDateTime(1, wxDateTime::Jan, 2026, 21, 0, 0));
  EXPECT_EQ(logger.NoteCatalogDone(2, 3, 1, 0, 0),
            wxT("chartdldr_pi: catalog 2/3 done (1 update, 0 new, 0 failed)"));
}

TEST(ChartDldrScheduledRunLog, MilestoneSequenceMatchesOpsShape) {
  ChartDldrScheduledRunLogger logger;
  const wxDateTime t0(8, wxDateTime::Jul, 2026, 21, 0, 0);
  logger.OnStarting(t0);

  std::vector<wxString> lines;
  lines.push_back(ChartDldrFormatScheduledStarting(3));
  lines.push_back(logger.NoteCatalogRefresh(1, 3, wxT("SRC1"), true, 0, t0));
  lines.push_back(logger.NoteCatalogRefresh(2, 3, wxT("SRC2"), true, 1, t0));
  lines.push_back(logger.NoteDownloading(1, 1, wxT("CHART"), t0));
  lines.push_back(logger.NoteCatalogDone(2, 3, 1, 0, 0, t0));
  lines.push_back(logger.NoteCatalogRefresh(3, 3, wxT("SRC3"), true, 0, t0));

  EXPECT_EQ(lines[0],
            wxT("chartdldr_pi: scheduled bulk update starting (3 sources)"));
  EXPECT_TRUE(lines[1].Contains(wxT("catalog 1/3 \"SRC1\": refresh ok, 0")));
  EXPECT_TRUE(lines[2].Contains(wxT("catalog 2/3 \"SRC2\": refresh ok, 1")));
  EXPECT_TRUE(lines[3].Contains(wxT("downloading 1/1 \"CHART\"")));
  EXPECT_TRUE(lines[4].Contains(wxT("catalog 2/3 done")));
  EXPECT_TRUE(lines[5].Contains(wxT("catalog 3/3 \"SRC3\": refresh ok, 0")));
}

TEST(ChartDldrScheduledRunLog, ZeroChartRefreshLinesHaveNoDownload) {
  ChartDldrScheduledRunLogger logger;
  const wxDateTime t0(8, wxDateTime::Jul, 2026, 21, 0, 0);
  logger.OnStarting(t0);

  int refresh_lines = 0;
  for (int i = 0; i < 3; ++i) {
    const wxString line = logger.NoteCatalogRefresh(
        i + 1, 3, wxString::Format(wxT("SRC%d"), i + 1), true, 0, t0);
    EXPECT_TRUE(line.Contains(wxT("refresh ok, 0 selected")));
    EXPECT_FALSE(line.Contains(wxT("downloading")));
    ++refresh_lines;
  }
  EXPECT_EQ(refresh_lines, 3);
}

TEST(ChartDldrScheduledRunLog, FinishSummaryViaScheduleStateNotDuplicated) {
  ChartDldrBulkRunStats stats;
  stats.updated_downloads = 1;
  stats.attempted = 1;
  const auto result = ChartDldrScheduledBulkResultFromStats(stats);
  const wxString summary = wxString::Format(
      wxT("chartdldr_pi: scheduled bulk update: %s"), result.status_detail);
  EXPECT_TRUE(summary.StartsWith(wxT("chartdldr_pi: scheduled bulk update:")));
  EXPECT_FALSE(summary.Contains(wxT("ChartDldrBulk:")));
}
