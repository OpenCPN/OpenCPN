/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_CHART_INSTALL_H_
#define CHARTDLDR_CHART_INSTALL_H_

#include <wx/datetime.h>
#include <wx/string.h>

class ChartSource;
class chartdldr_pi;

/** Extract to chart dir and persist install stamp; both must succeed. */
bool ChartDldrCommitChartInstall(chartdldr_pi* pi, ChartSource& source,
                                 const wxString& full_path,
                                 const wxString& chart_filename,
                                 const wxDateTime& update_time,
                                 bool verbose_extract_log);

#endif  // CHARTDLDR_CHART_INSTALL_H_
