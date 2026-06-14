/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_format.h"

#include <wx/intl.h>

wxString ChartDldrFormatBytes(double bytes) {
  if (bytes <= 0) {
    return "?";
  }
  return wxString::Format(_T("%.1fMB"), bytes / 1024 / 1024);
}

wxString ChartDldrFormatBytes(long bytes) {
  return ChartDldrFormatBytes(static_cast<double>(bytes));
}
