/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartcatalog.h"
#include "chartdldr_catalog_download.h"
#include "chartdldr_temp_download.h"

#include <wx/ffile.h>
#include <wx/filename.h>

namespace {

wxString ReadFileContents(const wxString& path) {
  wxFFile file(path);
  if (!file.IsOpened()) {
    return wxString();
  }
  wxString contents;
  file.ReadAll(&contents);
  return contents;
}

wxString MakeTempDir(const wxString& tag) {
  wxFileName dir = wxFileName::CreateTempFileName(tag);
  wxRemoveFile(dir.GetFullPath());
  wxFileName::Mkdir(dir.GetFullPath(), 0755, wxPATH_MKDIR_FULL);
  return dir.GetFullPath();
}

wxString WriteBytes(const wxString& path, const char* bytes) {
  wxFFile file(path, wxT("wb"));
  EXPECT_TRUE(file.IsOpened());
  file.Write(bytes);
  file.Close();
  return path;
}

const char* kValidCatalogHeaderXml =
    "<?xml version=\"1.0\"?>\n"
    "<RncProductCatalog>\n"
    "  <header>\n"
    "    <title>Test Catalog</title>\n"
    "    <date_created>2026-01-01</date_created>\n"
    "    <time_created>12:00:00</time_created>\n"
    "    <date_valid>2026-01-01</date_valid>\n"
    "    <time_valid>12:00:00</time_valid>\n"
    "    <dt_valid>2026-01-01T12:00:00Z</dt_valid>\n"
    "  </header>\n"
    "  <chart>\n"
    "    <title>Test Chart 1</title>\n"
    "    <zipfile_location>http://example.test/chart1.zip</zipfile_location>\n"
    "    <zipfile_datetime_iso8601>2026-01-01T12:00:00Z"
    "</zipfile_datetime_iso8601>\n"
    "  </chart>\n"
    "</RncProductCatalog>\n";

const char* kHeaderOnlyCatalogXml =
    "<?xml version=\"1.0\"?>\n"
    "<RncProductCatalog>\n"
    "  <header>\n"
    "    <title>Test Catalog</title>\n"
    "    <date_created>2026-01-01</date_created>\n"
    "    <time_created>12:00:00</time_created>\n"
    "    <dt_valid>2026-01-01T12:00:00Z</dt_valid>\n"
    "  </header>\n"
    "</RncProductCatalog>\n";

const char* kCatalogWithEmptyChartXml =
    "<?xml version=\"1.0\"?>\n"
    "<RncProductCatalog>\n"
    "  <header>\n"
    "    <title>Test Catalog</title>\n"
    "    <date_created>2026-01-01</date_created>\n"
    "    <time_created>12:00:00</time_created>\n"
    "    <dt_valid>2026-01-01T12:00:00Z</dt_valid>\n"
    "  </header>\n"
    "  <chart>\n"
    "    <title></title>\n"
    "  </chart>\n"
    "</RncProductCatalog>\n";

}  // namespace

TEST(ChartDldrBulkCatalogResult, SuccessRequestsReload) {
  const ChartDldrCatalogDownloadOutcome outcome =
      ChartDldrCatalogDownloadOutcomeFor(OCPN_DL_NO_ERROR, wxT("http://x"));

  EXPECT_TRUE(outcome.reload_catalog);
  EXPECT_FALSE(outcome.report_failure);
}

TEST(ChartDldrBulkCatalogResult, FailedReportsMessage) {
  const ChartDldrCatalogDownloadOutcome outcome =
      ChartDldrCatalogDownloadOutcomeFor(OCPN_DL_FAILED, wxT("http://example"));

  EXPECT_FALSE(outcome.reload_catalog);
  EXPECT_TRUE(outcome.report_failure);
  EXPECT_FALSE(outcome.failure_message.empty());
}

TEST(ChartDldrBulkCatalogResult, TimeoutDoesNotReloadOrAbortBulk) {
  const ChartDldrCatalogDownloadOutcome outcome =
      ChartDldrCatalogDownloadOutcomeFor(OCPN_DL_USER_TIMEOUT, wxT("http://x"));

  EXPECT_FALSE(outcome.reload_catalog);
  EXPECT_FALSE(outcome.report_failure);
}

TEST(ChartDldrBulkCatalogResult, CompletePathsCancelled) {
  ChartDldrTempDownloadPaths paths;

  EXPECT_EQ(ChartDldrCompleteTempDownloadPaths(paths, false, true),
            OCPN_DL_ABORTED);
}

TEST(ChartDldrBulkCatalogResult, CompletePathsTransferFailed) {
  ChartDldrTempDownloadPaths paths;

  EXPECT_EQ(ChartDldrCompleteTempDownloadPaths(paths, false, false),
            OCPN_DL_FAILED);
}

TEST(ChartDldrBulkCatalogResult, CompletePathsSuccessWithoutCopy) {
  ChartDldrTempDownloadPaths paths;
  paths.temp_path = wxT("/data/catalog.xml");
  paths.output_path = wxT("/data/catalog.xml");

  EXPECT_EQ(ChartDldrCompleteTempDownloadPaths(paths, true, false),
            OCPN_DL_NO_ERROR);
}

TEST(ChartDldrBulkCatalogResult, CatalogRefreshSucceededOnOk) {
  EXPECT_TRUE(ChartDldrCatalogRefreshSucceeded(OCPN_DL_NO_ERROR, false));
}

TEST(ChartDldrBulkCatalogResult, CatalogRefreshFailedOnError) {
  EXPECT_FALSE(ChartDldrCatalogRefreshSucceeded(OCPN_DL_FAILED, false));
}

TEST(ChartDldrBulkCatalogResult, CatalogRefreshFailedWhenCancelled) {
  EXPECT_FALSE(ChartDldrCatalogRefreshSucceeded(OCPN_DL_NO_ERROR, true));
}

TEST(ChartDldrBulkCatalogResult, CompleteValidatedPathsSucceeds) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_valid"));
  const wxString output_path =
      dir + wxFileName::GetPathSeparator() + wxT("catalog.xml");
  const wxString live_bytes = "live-catalog";
  WriteBytes(output_path, live_bytes);

  ChartDldrTempDownloadPaths paths = ChartDldrTempDownloadPathsFor(output_path);
  WriteBytes(paths.temp_path, kValidCatalogHeaderXml);

  EXPECT_EQ(ChartDldrCompleteValidatedCatalogDownloadPaths(paths, true, false),
            OCPN_DL_NO_ERROR);
  EXPECT_TRUE(wxFileExists(output_path));
  EXPECT_FALSE(wxFileExists(paths.temp_path));
  EXPECT_NE(ReadFileContents(output_path), wxString(live_bytes));
  EXPECT_TRUE(ReadFileContents(output_path).Contains(wxT("RncProductCatalog")));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult, CompleteValidatedPathsPreservesLiveOnInvalid) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_invalid"));
  const wxString output_path =
      dir + wxFileName::GetPathSeparator() + wxT("catalog.xml");
  const wxString live_bytes = "live-good-catalog";
  WriteBytes(output_path, live_bytes);

  ChartDldrTempDownloadPaths paths = ChartDldrTempDownloadPathsFor(output_path);
  WriteBytes(paths.temp_path, "HTTP 200 but not catalog XML");

  EXPECT_EQ(ChartDldrCompleteValidatedCatalogDownloadPaths(paths, true, false),
            OCPN_DL_FAILED);
  EXPECT_EQ(ReadFileContents(output_path), wxString(live_bytes));
  EXPECT_FALSE(wxFileExists(paths.temp_path));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult, StateSuffixedEncRootNameIsAccepted) {
  // NOAA regional catalogs use EncProductCatalogCA / OR / WA, not the bare
  // EncProductCatalog root.
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_state_root"));
  const wxString path = dir + wxFileName::GetPathSeparator() + wxT("ca.xml");
  WriteBytes(path,
             "<?xml version=\"1.0\"?>\n"
             "<EncProductCatalogCA>\n"
             "  <Header>\n"
             "    <title>ENC Product Catalog CA</title>\n"
             "    <date_created>2026-01-01</date_created>\n"
             "    <time_created>12:00:00</time_created>\n"
             "    <dt_valid>2026-01-01T12:00:00Z</dt_valid>\n"
             "  </Header>\n"
             "  <cell>\n"
             "    <title>Test Chart 1</title>\n"
             "    <zipfile_location>http://example.test/chart1.zip"
             "</zipfile_location>\n"
             "  </cell>\n"
             "</EncProductCatalogCA>\n");
  EXPECT_TRUE(ChartDldrCatalogTempIsPublishable(path));
  wxRemoveFile(path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult, EmptyDateCreatedIsRejected) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_empty_date"));
  const wxString path =
      dir + wxFileName::GetPathSeparator() + wxT("bad_date.xml");
  WriteBytes(path,
             "<?xml version=\"1.0\"?>\n"
             "<RncProductCatalog>\n"
             "  <header>\n"
             "    <title>Test Catalog</title>\n"
             "    <date_created></date_created>\n"
             "    <time_created>12:00:00</time_created>\n"
             "  </header>\n"
             "  <chart>\n"
             "    <title>Test Chart 1</title>\n"
             "    <zipfile_location>http://example.test/chart1.zip"
             "</zipfile_location>\n"
             "  </chart>\n"
             "</RncProductCatalog>\n");
  ChartCatalog catalog;
  EXPECT_TRUE(catalog.LoadFromFile(path, false));
  EXPECT_EQ(catalog.charts.size(), 1u);
  EXPECT_FALSE(ChartDldrCatalogTempIsPublishable(path));
  wxRemoveFile(path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult, IncompleteIencCellIsRejectedNotCrash) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_ienc_incomplete"));
  const wxString path =
      dir + wxFileName::GetPathSeparator() + wxT("ienc_bad.xml");
  // Cell without s57_file: accessors must not null-deref; publishability fails.
  WriteBytes(path,
             "<?xml version=\"1.0\"?>\n"
             "<IENCU37ProductCatalog>\n"
             "  <Header>\n"
             "    <title>IENC Test</title>\n"
             "    <date_created>2026-01-01</date_created>\n"
             "    <time_created>12:00:00</time_created>\n"
             "    <dt_valid>2026-01-01T12:00:00Z</dt_valid>\n"
             "  </Header>\n"
             "  <Cell>\n"
             "    <name>U37XX</name>\n"
             "  </Cell>\n"
             "</IENCU37ProductCatalog>\n");
  EXPECT_FALSE(ChartDldrCatalogTempIsPublishable(path));
  wxRemoveFile(path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult, EmptyRncProductCatalogDoesNotOverwriteLive) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_empty_root"));
  const wxString output_path =
      dir + wxFileName::GetPathSeparator() + wxT("catalog.xml");
  const wxString live_bytes = kValidCatalogHeaderXml;
  WriteBytes(output_path, live_bytes);

  ChartDldrTempDownloadPaths paths = ChartDldrTempDownloadPathsFor(output_path);
  WriteBytes(paths.temp_path,
             "<?xml version=\"1.0\"?>\n<RncProductCatalog/>\n");

  EXPECT_FALSE(ChartDldrCatalogTempIsPublishable(paths.temp_path));
  EXPECT_EQ(ChartDldrCompleteValidatedCatalogDownloadPaths(paths, true, false),
            OCPN_DL_FAILED);
  EXPECT_EQ(ReadFileContents(output_path), wxString(live_bytes));
  EXPECT_FALSE(wxFileExists(paths.temp_path));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult,
     HeaderOnlyCatalogWithoutChartsDoesNotOverwriteLive) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_header_only"));
  const wxString output_path =
      dir + wxFileName::GetPathSeparator() + wxT("catalog.xml");
  const wxString live_bytes = kValidCatalogHeaderXml;
  WriteBytes(output_path, live_bytes);

  ChartDldrTempDownloadPaths paths = ChartDldrTempDownloadPathsFor(output_path);
  WriteBytes(paths.temp_path, kHeaderOnlyCatalogXml);

  EXPECT_FALSE(ChartDldrCatalogTempIsPublishable(paths.temp_path));
  EXPECT_EQ(ChartDldrCompleteValidatedCatalogDownloadPaths(paths, true, false),
            OCPN_DL_FAILED);
  EXPECT_EQ(ReadFileContents(output_path), wxString(live_bytes));
  EXPECT_FALSE(wxFileExists(paths.temp_path));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult,
     CatalogWithEmptyChartEntryDoesNotOverwriteLive) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_empty_chart"));
  const wxString output_path =
      dir + wxFileName::GetPathSeparator() + wxT("catalog.xml");
  const wxString live_bytes = kValidCatalogHeaderXml;
  WriteBytes(output_path, live_bytes);

  ChartDldrTempDownloadPaths paths = ChartDldrTempDownloadPathsFor(output_path);
  WriteBytes(paths.temp_path, kCatalogWithEmptyChartXml);

  EXPECT_FALSE(ChartDldrCatalogTempIsPublishable(paths.temp_path));
  EXPECT_EQ(ChartDldrCompleteValidatedCatalogDownloadPaths(paths, true, false),
            OCPN_DL_FAILED);
  EXPECT_EQ(ReadFileContents(output_path), wxString(live_bytes));
  EXPECT_FALSE(wxFileExists(paths.temp_path));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult,
     CompleteValidatedPathsTransportSuccessInvalidXmlIsFailed) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_complete_invalid"));
  const wxString output_path =
      dir + wxFileName::GetPathSeparator() + wxT("catalog.xml");
  const wxString live_bytes = "existing-valid";
  WriteBytes(output_path, live_bytes);

  ChartDldrTempDownloadPaths paths = ChartDldrTempDownloadPathsFor(output_path);
  WriteBytes(paths.temp_path, "<html>error page</html>");

  EXPECT_EQ(ChartDldrCompleteValidatedCatalogDownloadPaths(paths, true, false),
            OCPN_DL_FAILED);
  EXPECT_FALSE(ChartDldrCatalogRefreshSucceeded(OCPN_DL_FAILED, false));
  EXPECT_EQ(ReadFileContents(output_path), wxString(live_bytes));
  EXPECT_FALSE(wxFileExists(paths.temp_path));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrBulkCatalogResult,
     CompleteValidatedPathsTransportSuccessValidXmlPublishes) {
  const wxString dir = MakeTempDir(wxT("chartdldr_catalog_complete_valid"));
  const wxString output_path =
      dir + wxFileName::GetPathSeparator() + wxT("catalog.xml");
  WriteBytes(output_path, "old");

  ChartDldrTempDownloadPaths paths = ChartDldrTempDownloadPathsFor(output_path);
  WriteBytes(paths.temp_path, kValidCatalogHeaderXml);

  EXPECT_EQ(ChartDldrCompleteValidatedCatalogDownloadPaths(paths, true, false),
            OCPN_DL_NO_ERROR);
  EXPECT_TRUE(ReadFileContents(output_path).Contains(wxT("RncProductCatalog")));
  EXPECT_FALSE(wxFileExists(paths.temp_path));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);
}
