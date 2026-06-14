/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_catalog_prep.h"

#include "chartdldr_chart_source.h"

#include <wx/intl.h>
#include <wx/tokenzr.h>
#include <wx/url.h>

wxString ChartDldrCatalogFilenameFromUrl(const wxString& url) {
  const wxURI uri(url);
  const wxString path = uri.HasScheme() ? uri.GetPath() : url;
  wxStringTokenizer tk(path, _T("/"));
  wxString file;
  while (tk.HasMoreTokens()) {
    file = tk.GetNextToken();
  }
  return file;
}

void ChartDldrPrepareChartSourceLocals(ChartSource& source) {
  source.LoadUpdateData();
  source.UpdateLocalFiles();
}

ChartDldrChartUpdateKind ChartDldrCatalogChartStatus(
    const ChartSource& source, const wxString& chart_number,
    const wxString& filename, const wxDateTime& catalog_update) {
  const bool exists = source.ExistsLocaly(chart_number, filename);
  const bool newer =
      exists && source.IsNewerThanLocal(chart_number, filename, catalog_update);
  return ChartDldrClassifyChart(exists, newer);
}

bool ChartDldrShouldPreselectChart(ChartDldrChartUpdateKind kind,
                                   bool preselect_new, bool preselect_updated) {
  switch (kind) {
    case ChartDldrChartUpdateKind::New:
      return preselect_new;
    case ChartDldrChartUpdateKind::Updated:
      return preselect_updated;
    case ChartDldrChartUpdateKind::None:
      return false;
  }
  return false;
}

wxString ChartDldrChartStatusLabel(ChartDldrChartUpdateKind kind) {
  switch (kind) {
    case ChartDldrChartUpdateKind::New:
      return _("New");
    case ChartDldrChartUpdateKind::Updated:
      return _("Out of date");
    case ChartDldrChartUpdateKind::None:
      return _("Up to date");
  }
  return wxEmptyString;
}
