/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_PANEL_UI_H_
#define CHARTDLDR_BULK_PANEL_UI_H_

#include <wx/intl.h>
#include <wx/string.h>

/** Catalog control model while a bulk run is active. */
struct ChartDldrCatalogControlsState {
  bool list_enabled = true;
  bool mutations_enabled = true;
  bool download_button_enabled = true;
};

inline ChartDldrCatalogControlsState ChartDldrCatalogControlsFor(
    bool run_active, bool cancel_armed) {
  ChartDldrCatalogControlsState state;
  state.list_enabled = !run_active;
  state.mutations_enabled = !run_active;
  state.download_button_enabled = !run_active || cancel_armed;
  return state;
}

inline wxString ChartDldrFormatBytes(double bytes) {
  if (bytes <= 0) {
    return "?";
  }
  return wxString::Format(_T("%.1fMB"), bytes / 1024 / 1024);
}

inline wxString ChartDldrFormatBytes(long bytes) {
  return ChartDldrFormatBytes(static_cast<double>(bytes));
}

#endif  // CHARTDLDR_BULK_PANEL_UI_H_
