/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_chart_install.h"

#include "chartdldr_catalog_prep.h"
#include "chartdldr_pi.h"

#include <wx/filename.h>

bool ChartDldrCommitChartInstall(chartdldr_pi* pi, ChartSource& source,
                                 const wxString& full_path,
                                 const wxString& chart_filename,
                                 const wxDateTime& update_time,
                                 bool verbose_extract_log) {
  if (!pi) {
    return false;
  }
  wxFileName fn(full_path);
  if (!pi->ProcessFile(full_path, fn.GetPath(), true, update_time,
                       verbose_extract_log)) {
    return false;
  }
  return ChartDldrCommitChartInstallStamp(source, chart_filename, update_time);
}
