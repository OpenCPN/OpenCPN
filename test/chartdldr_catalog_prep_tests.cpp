/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <wx/datetime.h>
#include <wx/dir.h>
#include <wx/ffile.h>
#include <wx/filename.h>

#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_source.h"

namespace {

wxString MakeTempChartDir() {
  wxFileName dir = wxFileName::CreateTempFileName(wxT("chartdldr_prep_test"));
  wxRemoveFile(dir.GetFullPath());
  wxFileName::Mkdir(dir.GetFullPath(), 0755, wxPATH_MKDIR_FULL);
  return dir.GetFullPath();
}

void TouchChartFile(const wxString& dir, const wxString& name) {
  wxFFile file(dir + wxFileName::GetPathSeparator() + name, wxT("wb"));
  ASSERT_TRUE(file.IsOpened());
  file.Write("x", 1);
  file.Close();
}

}  // namespace

TEST(ChartDldrCatalogPrep, CatalogFilenameFromUrl) {
  EXPECT_EQ(ChartDldrCatalogFilenameFromUrl(wxT("http://example.test/US_CA.xml")),
            wxT("US_CA.xml"));
  EXPECT_EQ(ChartDldrCatalogFilenameFromUrl(
                  wxT("https://charts.noaa.gov/ENC/US_CA/US_CA.xml")),
            wxT("US_CA.xml"));
}

TEST(ChartDldrCatalogPrep, ShouldPreselectOnlyNewAndUpdated) {
  EXPECT_TRUE(ChartDldrShouldPreselectChart(ChartDldrChartUpdateKind::New, true,
                                            false));
  EXPECT_TRUE(ChartDldrShouldPreselectChart(ChartDldrChartUpdateKind::Updated,
                                            false, true));
  EXPECT_FALSE(ChartDldrShouldPreselectChart(ChartDldrChartUpdateKind::None,
                                               true, true));
}

TEST(ChartDldrCatalogPrep, PrepareLocalsBeforeExistsLocaly) {
  const wxString dir = MakeTempChartDir();
  TouchChartFile(dir, wxT("US5CA11M.zip"));

  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);

  EXPECT_FALSE(source.ExistsLocaly(wxT("US5CA11M"), wxT("US5CA11M.zip")));

  ChartDldrPrepareChartSourceLocals(source);

  EXPECT_TRUE(source.ExistsLocaly(wxT("US5CA11M"), wxT("US5CA11M.zip")));
  EXPECT_FALSE(source.ExistsLocaly(wxT("US5CA12M"), wxT("US5CA12M.zip")));

  const wxDateTime catalog_update = wxDateTime::Now();
  EXPECT_EQ(ChartDldrCatalogChartStatus(source, wxT("US5CA11M"),
                                        wxT("US5CA11M.zip"), catalog_update),
            ChartDldrChartUpdateKind::None);
  EXPECT_EQ(ChartDldrCatalogChartStatus(source, wxT("US5CA12M"),
                                        wxT("US5CA12M.zip"), catalog_update),
            ChartDldrChartUpdateKind::New);

  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrCatalogPrep, CatalogChartStatusDetectsUpdatedChart) {
  const wxString dir = MakeTempChartDir();
  const wxString chart_path = dir + wxFileName::GetPathSeparator() + wxT("US5CA11M.zip");
  TouchChartFile(dir, wxT("US5CA11M.zip"));

  const wxDateTime old_local = wxDateTime::Now() - wxTimeSpan::Days(30);
  wxFileName(chart_path).SetTimes(&old_local, &old_local, &old_local);

  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);
  ChartDldrPrepareChartSourceLocals(source);

  const wxDateTime catalog_update = wxDateTime::Now();
  EXPECT_EQ(ChartDldrCatalogChartStatus(source, wxT("US5CA11M"),
                                        wxT("US5CA11M.zip"), catalog_update),
            ChartDldrChartUpdateKind::Updated);

  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}
