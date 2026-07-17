/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <wx/datetime.h>
#include <wx/dir.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/url.h>

#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_bulk_chart_loop.h"
#include "chartdldr_bulk_state.h"

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
  EXPECT_EQ(
      ChartDldrCatalogFilenameFromUrl(wxT("http://example.test/US_CA.xml")),
      wxT("US_CA.xml"));
  EXPECT_EQ(ChartDldrCatalogFilenameFromUrl(
                wxT("https://charts.noaa.gov/ENC/US_CA/US_CA.xml")),
            wxT("US_CA.xml"));
}

TEST(ChartDldrCatalogPrep, PrepareDownloadPathsFromUrlAndDir) {
  const wxString dir = MakeTempChartDir();
  wxURI url;
  wxFileName output_fn;
  EXPECT_TRUE(ChartDldrPrepareCatalogDownloadPathsFromUrlAndDir(
      wxT("http://example.test/US_CA.xml"), dir, url, output_fn));
  EXPECT_EQ(output_fn.GetFullName(), wxT("US_CA.xml"));
  EXPECT_EQ(output_fn.GetPath(), dir);
  EXPECT_TRUE(wxDirExists(dir));
  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrCatalogPrep, PrepareDownloadPathsRejectsBadUrl) {
  const wxString dir = MakeTempChartDir();
  wxURI url;
  wxFileName output_fn;
  EXPECT_FALSE(ChartDldrPrepareCatalogDownloadPathsFromUrlAndDir(
      wxT(""), dir, url, output_fn));
  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
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

  EXPECT_FALSE(source.ExistsLocaly(wxT("US5CA11M.zip")));

  ChartDldrPrepareChartSourceLocals(source);

  EXPECT_TRUE(source.ExistsLocaly(wxT("US5CA11M.zip")));
  EXPECT_FALSE(source.ExistsLocaly(wxT("US5CA12M.zip")));

  const wxDateTime catalog_update = wxDateTime::Now();
  EXPECT_EQ(
      ChartDldrCatalogChartStatus(source, wxT("US5CA11M.zip"), catalog_update),
      ChartDldrChartUpdateKind::None);
  EXPECT_EQ(
      ChartDldrCatalogChartStatus(source, wxT("US5CA12M.zip"), catalog_update),
      ChartDldrChartUpdateKind::New);

  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrCatalogPrep, CatalogChartStatusDetectsUpdatedChart) {
  const wxString dir = MakeTempChartDir();
  const wxString chart_path =
      dir + wxFileName::GetPathSeparator() + wxT("US5CA11M.zip");
  TouchChartFile(dir, wxT("US5CA11M.zip"));

  const wxDateTime old_local = wxDateTime::Now() - wxTimeSpan::Days(30);
  wxFileName(chart_path).SetTimes(&old_local, &old_local, &old_local);

  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);
  ChartDldrPrepareChartSourceLocals(source);

  const wxDateTime catalog_update = wxDateTime::Now();
  EXPECT_EQ(
      ChartDldrCatalogChartStatus(source, wxT("US5CA11M.zip"), catalog_update),
      ChartDldrChartUpdateKind::Updated);

  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrCatalogPrep, ChartUpdatedMarksPresentWithoutRescan) {
  const wxString dir = MakeTempChartDir();
  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);

  EXPECT_FALSE(source.ExistsLocaly(wxT("US5CA11M.zip")));

  const wxDateTime stamp = wxDateTime::Now();
  EXPECT_TRUE(source.ChartUpdated(wxT("US5CA11M.zip"), stamp.GetTicks()));

  // Same stamp map drives presence and freshness — no parallel file list.
  EXPECT_TRUE(source.ExistsLocaly(wxT("US5CA11M.zip")));
  EXPECT_EQ(ChartDldrCatalogChartStatus(source, wxT("US5CA11M.zip"), stamp),
            ChartDldrChartUpdateKind::None);

  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrCatalogPrep, PrepareLocalsDropsMetadataForRemovedCharts) {
  const wxString dir = MakeTempChartDir();
  const wxString chart_path =
      dir + wxFileName::GetPathSeparator() + wxT("US5CA11M.zip");
  TouchChartFile(dir, wxT("US5CA11M.zip"));

  const wxDateTime old_local = wxDateTime::Now() - wxTimeSpan::Days(30);
  wxFileName(chart_path).SetTimes(&old_local, &old_local, &old_local);

  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);
  ChartDldrPrepareChartSourceLocals(source);

  const wxDateTime catalog_update = wxDateTime::Now();
  EXPECT_EQ(
      ChartDldrCatalogChartStatus(source, wxT("US5CA11M.zip"), catalog_update),
      ChartDldrChartUpdateKind::Updated);

  // Simulate successful download: filename stem stamped with catalog time.
  EXPECT_TRUE(
      source.ChartUpdated(wxT("US5CA11M.zip"), catalog_update.GetTicks()));
  wxRemoveFile(chart_path);

  // Reloaded prepare must rescans disk and treat the chart as missing (New),
  // not trust stale chartdldr_pi.dat stamps.
  ChartSource reloaded(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);
  ChartDldrPrepareChartSourceLocals(reloaded);
  EXPECT_EQ(ChartDldrCatalogChartStatus(reloaded, wxT("US5CA11M.zip"),
                                        catalog_update),
            ChartDldrChartUpdateKind::New);
  EXPECT_FALSE(reloaded.ExistsLocaly(wxT("US5CA11M.zip")));

  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrCatalogPrep, CommitChartInstallStampFailsWhenMetadataCannotPersist) {
  const wxString dir = MakeTempChartDir();
  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);

  wxFileName dir_fn(dir);
  ASSERT_TRUE(dir_fn.SetPermissions(0500));

  const wxDateTime stamp = wxDateTime::Now();
  EXPECT_FALSE(
      ChartDldrCommitChartInstallStamp(source, wxT("US5CA11M.zip"), stamp));
  EXPECT_FALSE(source.ExistsLocaly(wxT("US5CA11M.zip")));

  dir_fn.SetPermissions(wxS_DIR_DEFAULT);
  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrCatalogPrep, InstallOutcomeTreatsStampPersistFailureAsBulkFailure) {
  const wxString dir = MakeTempChartDir();
  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);

  wxFileName dir_fn(dir);
  ASSERT_TRUE(dir_fn.SetPermissions(0500));

  const bool install_ok = ChartDldrCommitChartInstallStamp(
      source, wxT("US5CA11M.zip"), wxDateTime::Now());

  ChartDldrChartBulkState chart_bulk;
  ChartDldrRecordChartDownloadOutcome(install_ok,
                                      ChartDldrChartUpdateKind::New,
                                      chart_bulk);
  EXPECT_FALSE(install_ok);
  EXPECT_EQ(chart_bulk.failed, 1);
  EXPECT_EQ(chart_bulk.new_downloads, 0);

  dir_fn.SetPermissions(wxS_DIR_DEFAULT);
  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}
