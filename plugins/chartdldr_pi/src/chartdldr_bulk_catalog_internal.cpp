/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_catalog_internal.h"

#include "chartdldr_bulk.h"
#include "chartdldr_pi.h"

#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/tokenzr.h>
#include <wx/url.h>
#include <wx/window.h>

namespace ChartDldrBulkCatalogInternal {

bool PrepareDownloadPaths(int catalog_index, chartdldr_pi* pi, wxURI& url,
                            wxFileName& output_fn) {
  if (!pi || catalog_index < 0 ||
      catalog_index >= static_cast<int>(pi->m_ChartSources.size())) {
    return false;
  }

  pi->SetSourceId(catalog_index);
  std::unique_ptr<ChartSource>& cs = pi->m_ChartSources.at(catalog_index);
  url = wxURI(cs->GetUrl());
  if (url.IsReference()) {
    return false;
  }

  wxStringTokenizer tk(url.GetPath(), _T("/"));
  wxString file;
  do {
    file = tk.GetNextToken();
  } while (tk.HasMoreTokens());
  output_fn.SetFullName(file);
  output_fn.SetPath(cs->GetDir());
  if (!wxDirExists(cs->GetDir())) {
    if (!wxFileName::Mkdir(cs->GetDir(), 0755, wxPATH_MKDIR_FULL)) {
      return false;
    }
  }
  return true;
}

wxString BackgroundDownloadTargetPath(const wxFileName& output_fn) {
#ifdef __ANDROID__
  wxString file_uri = output_fn.GetFullPath();
  if (!file_uri.StartsWith(_T("file://"))) {
    file_uri = _T("file://") + output_fn.GetFullPath();
  }
  return file_uri;
#else
  return output_fn.GetFullPath();
#endif
}

void ReportPathFailure(wxWindow* parent, const ChartDldrBulkModeProfile& profile,
                       const wxURI& url, const wxFileName& output_fn) {
  if (url.IsReference()) {
    const wxString msg =
        _("Error, the URL to the chart source data seems wrong.");
    ChartDldrReportBulkError(parent, profile, msg, _("Error"));
    return;
  }
  const wxString msg = wxString::Format(_("Directory %s can't be created."),
                                        output_fn.GetPath().c_str());
  ChartDldrReportBulkError(parent, profile, msg, _("Chart Downloader"));
}

}  // namespace ChartDldrBulkCatalogInternal
