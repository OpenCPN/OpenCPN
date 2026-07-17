/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <wx/datetime.h>
#include <wx/dir.h>
#include <wx/ffile.h>
#include <wx/filename.h>

#include "chartcatalog.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_catalog_selection.h"
#include "chartdldr_chart_source.h"

namespace {

wxString MakeTempChartDir() {
  wxFileName dir = wxFileName::CreateTempFileName(wxT("chartdldr_sel_test"));
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

wxString WriteMinimalCatalog(const wxString& dir) {
  const wxString path = dir + wxFileName::GetPathSeparator() + wxT("US_CA.xml");
  wxFFile file(path, wxT("wb"));
  EXPECT_TRUE(file.IsOpened());
  file.Write(
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
      "    <number>US5CA11M</number>\n"
      "    <title>Existing Chart</title>\n"
      "    "
      "<zipfile_location>http://example.test/US5CA11M.zip</zipfile_location>\n"
      "    "
      "<zipfile_datetime_iso8601>2026-01-01T12:00:00Z</"
      "zipfile_datetime_iso8601>\n"
      "  </chart>\n"
      "  <chart>\n"
      "    <number>US5CA12M</number>\n"
      "    <title>New Chart</title>\n"
      "    "
      "<zipfile_location>http://example.test/US5CA12M.zip</zipfile_location>\n"
      "    "
      "<zipfile_datetime_iso8601>2026-01-02T12:00:00Z</"
      "zipfile_datetime_iso8601>\n"
      "  </chart>\n"
      "</RncProductCatalog>\n");
  file.Close();
  return path;
}

}  // namespace

TEST(ChartDldrCatalogSelection, CheckedCountAndKindFilters) {
  ChartDldrCatalogSelection selection;
  selection.kinds = {ChartDldrChartUpdateKind::New,
                     ChartDldrChartUpdateKind::Updated,
                     ChartDldrChartUpdateKind::None};
  selection.checked = {true, false, true};
  selection.new_count = 1;
  selection.updated_count = 1;

  EXPECT_EQ(selection.Count(), 3);
  EXPECT_EQ(selection.CheckedCount(), 2);
  EXPECT_TRUE(selection.IsChecked(0));
  EXPECT_FALSE(selection.IsChecked(1));

  selection.SetCheckedForKind(ChartDldrChartUpdateKind::Updated, true);
  EXPECT_EQ(selection.CheckedCount(), 3);

  selection.SetAllChecked(false);
  EXPECT_EQ(selection.CheckedCount(), 0);

  selection.InvertAllChecked();
  EXPECT_EQ(selection.CheckedCount(), 3);
}

TEST(ChartDldrCatalogSelection, ClearResetsState) {
  ChartDldrCatalogSelection selection;
  selection.kinds.push_back(ChartDldrChartUpdateKind::New);
  selection.checked.push_back(true);
  selection.new_count = 1;
  selection.Clear();
  EXPECT_EQ(selection.Count(), 0);
  EXPECT_EQ(selection.CheckedCount(), 0);
}

TEST(ChartDldrCatalogSelection, BuildFromCatalogPreselectsNewCharts) {
  const wxString dir = MakeTempChartDir();
  TouchChartFile(dir, wxT("US5CA11M.zip"));
  const wxString catalog_path = WriteMinimalCatalog(dir);

  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);
  ChartDldrPrepareChartSourceLocals(source);

  ChartCatalog catalog;
  ASSERT_TRUE(catalog.LoadFromFile(catalog_path, false));
  ASSERT_EQ(catalog.charts.size(), 2u);

  const ChartDldrCatalogSelection selection =
      ChartDldrBuildCatalogSelection(catalog, source, true, false);

  EXPECT_EQ(selection.Count(), 2);
  EXPECT_EQ(selection.new_count, 1u);
  EXPECT_EQ(selection.updated_count, 0u);
  EXPECT_EQ(selection.KindAt(0), ChartDldrChartUpdateKind::None);
  EXPECT_EQ(selection.KindAt(1), ChartDldrChartUpdateKind::New);
  EXPECT_FALSE(selection.IsChecked(0));
  EXPECT_TRUE(selection.IsChecked(1));
  EXPECT_EQ(selection.CheckedCount(), 1);

  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrCatalogSelection, BuildFromCatalogSupportsHeadlessCheckedCount) {
  const wxString dir = MakeTempChartDir();
  TouchChartFile(dir, wxT("US5CA11M.zip"));
  const wxString catalog_path = WriteMinimalCatalog(dir);

  ChartSource source(wxT("US_CA"), wxT("http://example.test/US_CA.xml"), dir);
  ChartDldrPrepareChartSourceLocals(source);

  ChartCatalog catalog;
  ASSERT_TRUE(catalog.LoadFromFile(catalog_path, false));

  const ChartDldrCatalogSelection selection =
      ChartDldrBuildCatalogSelection(catalog, source, true, true);

  EXPECT_EQ(selection.CheckedCount(), 1);
  EXPECT_EQ(selection.new_count, 1u);

  wxDir::Remove(dir, wxPATH_RMDIR_RECURSIVE);
}
