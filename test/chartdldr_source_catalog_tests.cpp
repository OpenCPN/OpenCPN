/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_source_catalog.h"

#include <wx/file.h>
#include <wx/filename.h>

namespace {

class SourceCatalogFixture : public ::testing::Test {
protected:
  void TearDown() override {
    if (!path_.IsEmpty() && wxFileExists(path_)) {
      wxRemoveFile(path_);
    }
  }

  wxString WriteTemp(const std::string& contents) {
    path_ = wxFileName::CreateTempFileName(wxT("chartdldr_src_cat_test"));
    wxFile file(path_, wxFile::write);
    EXPECT_TRUE(file.IsOpened());
    file.Write(contents.c_str(), contents.size());
    file.Close();
    return path_;
  }

  wxString path_;
};

const char* kValidCatalog =
    "<?xml version=\"1.0\"?>\n"
    "<ChartCatalog>\n"
    "  <sections>\n"
    "    <section>\n"
    "      <name>Basemaps</name>\n"
    "      <catalogs>\n"
    "        <catalog>\n"
    "          <name>All GSHHG basemaps</name>\n"
    "          <type>GSHHG</type>\n"
    "          <location>https://example.test/GSHHG_Catalog.xml</location>\n"
    "          <dir>{USERDATA}/GSHHG</dir>\n"
    "        </catalog>\n"
    "      </catalogs>\n"
    "    </section>\n"
    "  </sections>\n"
    "</ChartCatalog>\n";

}  // namespace

TEST_F(SourceCatalogFixture, AcceptsWellFormedCatalog) {
  EXPECT_TRUE(ChartDldrSourceCatalogXmlIsPublishable(WriteTemp(kValidCatalog)));
}

TEST_F(SourceCatalogFixture, AcceptsNestedSectionCatalog) {
  const char* nested =
      "<?xml version=\"1.0\"?>\n"
      "<ChartCatalog>\n"
      "  <sections>\n"
      "    <section>\n"
      "      <name>USA</name>\n"
      "      <sections>\n"
      "        <section>\n"
      "          <name>ENC</name>\n"
      "          <catalogs>\n"
      "            <catalog>\n"
      "              <name>NOAA ENC</name>\n"
      "              <location>https://example.test/enc.xml</location>\n"
      "            </catalog>\n"
      "          </catalogs>\n"
      "        </section>\n"
      "      </sections>\n"
      "    </section>\n"
      "  </sections>\n"
      "</ChartCatalog>\n";
  EXPECT_TRUE(ChartDldrSourceCatalogXmlIsPublishable(WriteTemp(nested)));
}

TEST_F(SourceCatalogFixture, RejectsMissingFile) {
  EXPECT_FALSE(ChartDldrSourceCatalogXmlIsPublishable(
      wxT("/nonexistent/chartdldr/never-here.xml")));
}

TEST_F(SourceCatalogFixture, RejectsEmptyPath) {
  EXPECT_FALSE(ChartDldrSourceCatalogXmlIsPublishable(wxString()));
}

TEST_F(SourceCatalogFixture, RejectsHtmlErrorPage) {
  // An HTTP 200 that actually returns an error page must not be published.
  const char* html =
      "<!DOCTYPE html>\n<html><head><title>404</title></head>"
      "<body>Not found</body></html>\n";
  EXPECT_FALSE(ChartDldrSourceCatalogXmlIsPublishable(WriteTemp(html)));
}

TEST_F(SourceCatalogFixture, RejectsTruncatedXml) {
  const char* truncated =
      "<?xml version=\"1.0\"?>\n<ChartCatalog>\n  <sections>\n    <section>\n";
  EXPECT_FALSE(ChartDldrSourceCatalogXmlIsPublishable(WriteTemp(truncated)));
}

TEST_F(SourceCatalogFixture, RejectsEmptyCatalogRoot) {
  const char* empty = "<?xml version=\"1.0\"?>\n<ChartCatalog/>\n";
  EXPECT_FALSE(ChartDldrSourceCatalogXmlIsPublishable(WriteTemp(empty)));
}

TEST_F(SourceCatalogFixture, RejectsCatalogWithoutLocation) {
  const char* no_loc =
      "<?xml version=\"1.0\"?>\n"
      "<ChartCatalog>\n"
      "  <sections>\n"
      "    <section>\n"
      "      <name>Basemaps</name>\n"
      "      <catalogs>\n"
      "        <catalog>\n"
      "          <name>Broken entry</name>\n"
      "        </catalog>\n"
      "      </catalogs>\n"
      "    </section>\n"
      "  </sections>\n"
      "</ChartCatalog>\n";
  EXPECT_FALSE(ChartDldrSourceCatalogXmlIsPublishable(WriteTemp(no_loc)));
}

TEST_F(SourceCatalogFixture, RejectsUnrelatedXmlWithoutSections) {
  const char* other =
      "<?xml version=\"1.0\"?>\n"
      "<SomethingElse>\n"
      "  <catalog><name>x</name><location>y</location></catalog>\n"
      "</SomethingElse>\n";
  EXPECT_FALSE(ChartDldrSourceCatalogXmlIsPublishable(WriteTemp(other)));
}
