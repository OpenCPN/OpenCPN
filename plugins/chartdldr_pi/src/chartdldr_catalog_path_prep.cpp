/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_catalog_prep.h"

#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <memory>

#include <wx/intl.h>
#include <wx/url.h>

bool ChartDldrRequireValidCatalogIndex(const chartdldr_pi* pi, int index,
                                       wxWindow* parent) {
  const size_t count = pi ? pi->m_ChartSources.size() : 0;
  if (ChartDldrBulkCatalogIndexInRange(index, count)) {
    return true;
  }
  if (parent) {
    OCPNMessageBox_PlugIn(parent, _("Please select a chart catalog first."),
                          _("Chart Downloader"), wxOK | wxICON_INFORMATION);
  }
  return false;
}

bool ChartDldrPrepareCatalogDownloadPaths(chartdldr_pi* pi, int catalog_index,
                                          wxURI& url, wxFileName& output_fn) {
  if (!pi || !ChartDldrBulkCatalogIndexInRange(catalog_index,
                                               pi->m_ChartSources.size())) {
    return false;
  }

  const ChartSource& cs =
      *pi->m_ChartSources.at(static_cast<size_t>(catalog_index));
  return ChartDldrPrepareCatalogDownloadPathsFromUrlAndDir(
      cs.GetUrl(), cs.GetDir(), url, output_fn);
}
