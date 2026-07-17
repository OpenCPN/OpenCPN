/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_CHART_LIST_VIEW_H_
#define CHARTDLDR_PANEL_CHART_LIST_VIEW_H_

#include "chartdldr_bulk_catalog_run.h"

class ChartDldrPanelImpl;

/** Chart-list wx widget materialization for ChartDldrPanelImpl. */
class ChartDldrPanelChartListView {
public:
  explicit ChartDldrPanelChartListView(ChartDldrPanelImpl& panel);

  void MaterializeWidgets(bool materialize);
  void RenderFromSelection();
  void SyncFromSelection();
  /** Copies widget checkbox state into m_catalogSelection before a bulk run. */
  void CaptureSelectionFromWidgets();
  void ClearWidgets();
  int WidgetCount() const;
  void UpdateSelectionInfoLabel();
  bool IsMaterialized() const { return materialized_; }

private:
  struct RowData {
    wxString title;
    wxString status;
    wxString latest;
    bool checked = false;
  };

  RowData RowDataAt(int index) const;

  ChartDldrPanelImpl& panel_;
  bool materialized_ = false;
};

#endif  // CHARTDLDR_PANEL_CHART_LIST_VIEW_H_
