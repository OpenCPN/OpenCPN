/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_chart_list_view.h"

#include "chartdldr_catalog_prep.h"
#include "chartdldr_bulk_chart_loop.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"

#include <memory>

ChartDldrPanelChartListView::ChartDldrPanelChartListView(
    ChartDldrPanelImpl& panel)
    : panel_(panel) {}

ChartDldrPanelChartListView::RowData ChartDldrPanelChartListView::RowDataAt(
    int index) const {
  Chart* const chart =
      panel_.pPlugIn->m_pChartCatalog.charts.at(static_cast<size_t>(index))
          .get();
  const ChartDldrChartUpdateKind kind = panel_.m_catalogSelection.KindAt(index);
  RowData row;
  row.title = chart->GetChartTitle();
  row.status = ChartDldrChartStatusLabel(kind);
  row.latest = chart->GetUpdateDatetime().Format(_T("%Y-%m-%d"));
  row.checked = panel_.m_catalogSelection.IsChecked(index);
  return row;
}

void ChartDldrPanelChartListView::MaterializeWidgets(bool materialize) {
  if (!materialize) {
    return;
  }
  RenderFromSelection();
}

void ChartDldrPanelChartListView::RenderFromSelection() {
  if (panel_.m_catalogSelection.Count() <= 0) {
    return;
  }

#if defined(CHART_LIST)
  for (int i = 0; i < panel_.m_catalogSelection.Count(); ++i) {
    const RowData row = RowDataAt(i);
    wxVector<wxVariant> data;
    data.push_back(wxVariant(row.checked));
    data.push_back(wxVariant(row.status));
    data.push_back(wxVariant(row.latest));
    data.push_back(wxVariant(row.title));
    panel_.getChartList()->AppendItem(data);
  }
#else
  for (int i = 0; i < panel_.m_catalogSelection.Count(); ++i) {
    const RowData row = RowDataAt(i);
    auto pC = std::make_unique<ChartPanel>(
        panel_.m_scrollWinChartList, wxID_ANY, wxDefaultPosition,
        wxSize(-1, -1), row.title, row.status, row.latest, &panel_,
        row.checked);
    pC->GetCB()->Bind(wxEVT_CHECKBOX, [this, i](wxCommandEvent& event) {
      panel_.m_catalogSelection.SetChecked(i, event.IsChecked());
      UpdateSelectionInfoLabel();
    });
    pC->Connect(wxEVT_RIGHT_DOWN,
                wxMouseEventHandler(ChartDldrPanel::OnContextMenu), NULL,
                &panel_);
    panel_.m_boxSizerCharts->Add(pC.get(), 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
    panel_.m_panelArray.push_back(std::move(pC));
  }
  panel_.m_scrollWinChartList->ClearBackground();
  panel_.m_scrollWinChartList->FitInside();
  panel_.m_scrollWinChartList->GetSizer()->Layout();
  panel_.Layout();
  panel_.m_scrollWinChartList->ClearBackground();
#endif /* CHART_LIST */

  materialized_ = true;
  UpdateSelectionInfoLabel();
}

void ChartDldrPanelChartListView::SyncFromSelection() {
  if (!materialized_ || WidgetCount() != panel_.m_catalogSelection.Count()) {
    materialized_ = false;
    return;
  }
#if defined(CHART_LIST)
  panel_.m_bInfoHold = true;
#endif
  for (int i = 0; i < panel_.m_catalogSelection.Count(); ++i) {
#if defined(CHART_LIST)
    panel_.getChartList()->SetToggleValue(
        panel_.m_catalogSelection.IsChecked(i), i, 0);
#else
    panel_.m_panelArray.at(static_cast<size_t>(i))
        ->GetCB()
        ->SetValue(panel_.m_catalogSelection.IsChecked(i));
#endif /* CHART_LIST */
  }
#if defined(CHART_LIST)
  panel_.m_bInfoHold = false;
#endif
  UpdateSelectionInfoLabel();
}

void ChartDldrPanelChartListView::CaptureSelectionFromWidgets() {
  if (!materialized_ || WidgetCount() != panel_.m_catalogSelection.Count()) {
    return;
  }
#if defined(CHART_LIST)
  for (int i = 0; i < panel_.m_catalogSelection.Count(); ++i) {
    panel_.m_catalogSelection.SetChecked(
        i, panel_.getChartList()->GetToggleValue(i, 0));
  }
#else
  for (int i = 0; i < panel_.m_catalogSelection.Count(); ++i) {
    panel_.m_catalogSelection.SetChecked(
        i, panel_.m_panelArray.at(static_cast<size_t>(i))->GetCB()->GetValue());
  }
#endif /* CHART_LIST */
  UpdateSelectionInfoLabel();
}

void ChartDldrPanelChartListView::UpdateSelectionInfoLabel() {
#if defined(CHART_LIST)
  panel_.SetChartInfo(wxString::Format(
      _("%lu charts total, %lu updated, %lu new, %lu selected"),
      panel_.pPlugIn->m_pChartCatalog.charts.size(),
      panel_.m_catalogSelection.updated_count,
      panel_.m_catalogSelection.new_count, panel_.GetCheckedChartCount()));
#else
  panel_.SetChartInfo(
      wxString::Format(_("%lu charts total, %lu updated, %lu new"),
                       panel_.pPlugIn->m_pChartCatalog.charts.size(),
                       panel_.m_catalogSelection.updated_count,
                       panel_.m_catalogSelection.new_count));
#endif /* CHART_LIST */
}

void ChartDldrPanelChartListView::ClearWidgets() {
  materialized_ = false;
#if defined(CHART_LIST)
  panel_.clearChartList();
#else
  if (panel_.m_boxSizerCharts) {
    panel_.m_boxSizerCharts->Clear(false);
  }
  panel_.m_panelArray.clear();
  panel_.m_scrollWinChartList->ClearBackground();
#endif /* CHART_LIST */
}

int ChartDldrPanelChartListView::WidgetCount() const {
#if defined(CHART_LIST)
  return panel_.getChartList()->GetItemCount();
#else
  return static_cast<int>(panel_.m_panelArray.size());
#endif /* CHART_LIST */
}
