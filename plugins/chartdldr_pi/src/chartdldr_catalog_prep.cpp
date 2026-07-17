/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_catalog_prep.h"

#include "chartdldr_bulk_chart_loop.h"
#include "chartdldr_chart_source.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/log.h>
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

bool ChartDldrPrepareCatalogDownloadPathsFromUrlAndDir(const wxString& url_str,
                                                       const wxString& dir,
                                                       wxURI& url,
                                                       wxFileName& output_fn) {
  url = wxURI(url_str);
  if (url.IsReference()) {
    return false;
  }

  output_fn.SetFullName(ChartDldrCatalogFilenameFromUrl(url.BuildURI()));
  output_fn.SetPath(dir);
  if (!wxDirExists(dir)) {
    if (!wxFileName::Mkdir(dir, 0755, wxPATH_MKDIR_FULL)) {
      return false;
    }
  }
  return true;
}

void ChartDldrPrepareChartSourceLocals(ChartSource& source) {
  source.LoadUpdateData();
  if (!source.UpdateLocalFiles()) {
    wxLogWarning(
        wxT("chartdldr_pi: failed to persist chart update metadata for \"%s\""),
        source.GetDir().c_str());
  }
}

ChartDldrChartUpdateKind ChartDldrCatalogChartStatus(
    const ChartSource& source, const wxString& filename,
    const wxDateTime& catalog_update) {
  const bool exists = source.ExistsLocaly(filename);
  const bool newer =
      exists && source.IsNewerThanLocal(filename, catalog_update);
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

bool ChartDldrCommitChartInstallStamp(ChartSource& source,
                                      const wxString& chart_filename,
                                      const wxDateTime& update_time) {
  if (source.ChartUpdated(chart_filename, update_time.GetTicks())) {
    return true;
  }
  wxLogWarning(
      _T("chartdldr_pi: chart install succeeded but stamp persist failed ")
      _T("for \"%s\""),
      chart_filename.c_str());
  return false;
}
