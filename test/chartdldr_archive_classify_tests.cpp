/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_archive_classify.h"

TEST(ChartDldrArchiveClassify, ExtensionTable) {
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/chart.zip")),
            ChartDldrArchiveKind::Zip);
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/chart.ZIP")),
            ChartDldrArchiveKind::Zip);
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/chart.rar")),
            ChartDldrArchiveKind::Rar);
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/chart.tar.gz")),
            ChartDldrArchiveKind::MultiFormat);
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/chart.tgz")),
            ChartDldrArchiveKind::MultiFormat);
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/chart.gzip")),
            ChartDldrArchiveKind::MultiFormat);
  EXPECT_NE(ChartDldrClassifyArchive(wxT("/tmp/chart.gzip")),
            ChartDldrArchiveKind::Zip);
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/chart.7z")),
            ChartDldrArchiveKind::MultiFormat);
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/US5CA11M.zip")),
            ChartDldrArchiveKind::Zip);
  EXPECT_EQ(ChartDldrClassifyArchive(wxT("/tmp/chart.xml")),
            ChartDldrArchiveKind::Plain);
}
