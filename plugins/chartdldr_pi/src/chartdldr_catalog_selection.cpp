/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_catalog_selection.h"

#include "chartcatalog.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_source.h"

void ChartDldrCatalogSelection::Clear() {
  kinds.clear();
  checked.clear();
  new_count = 0;
  updated_count = 0;
}

int ChartDldrCatalogSelection::Count() const {
  return static_cast<int>(kinds.size());
}

int ChartDldrCatalogSelection::CheckedCount() const {
  int count = 0;
  for (bool is_checked : checked) {
    if (is_checked) {
      ++count;
    }
  }
  return count;
}

bool ChartDldrCatalogSelection::IsChecked(int index) const {
  return index >= 0 && index < Count() &&
         checked.at(static_cast<size_t>(index));
}

ChartDldrChartUpdateKind ChartDldrCatalogSelection::KindAt(int index) const {
  if (index < 0 || index >= Count()) {
    return ChartDldrChartUpdateKind::None;
  }
  return kinds.at(static_cast<size_t>(index));
}

void ChartDldrCatalogSelection::SetChecked(int index, bool value) {
  if (index >= 0 && index < Count()) {
    checked.at(static_cast<size_t>(index)) = value;
  }
}

void ChartDldrCatalogSelection::SetAllChecked(bool value) {
  for (size_t i = 0; i < checked.size(); ++i) {
    checked[i] = value;
  }
}

void ChartDldrCatalogSelection::SetCheckedForKind(ChartDldrChartUpdateKind kind,
                                                  bool value) {
  for (int i = 0; i < Count(); ++i) {
    if (KindAt(i) == kind) {
      SetChecked(i, value);
    }
  }
}

void ChartDldrCatalogSelection::InvertAllChecked() {
  for (size_t i = 0; i < checked.size(); ++i) {
    checked[i] = !checked[i];
  }
}

ChartDldrCatalogSelection ChartDldrBuildCatalogSelection(
    const ChartCatalog& catalog, const ChartSource& source, bool preselect_new,
    bool preselect_updated) {
  ChartDldrCatalogSelection selection;
  selection.kinds.reserve(catalog.charts.size());
  selection.checked.reserve(catalog.charts.size());

  for (size_t i = 0; i < catalog.charts.size(); ++i) {
    const wxString file = catalog.charts.at(i)->GetChartFilename(true);
    const ChartDldrChartUpdateKind kind = ChartDldrCatalogChartStatus(
        source, file, catalog.charts.at(i)->GetUpdateDatetime());
    selection.kinds.push_back(kind);
    if (kind == ChartDldrChartUpdateKind::New) {
      selection.new_count++;
    } else if (kind == ChartDldrChartUpdateKind::Updated) {
      selection.updated_count++;
    }
    selection.checked.push_back(
        ChartDldrShouldPreselectChart(kind, preselect_new, preselect_updated));
  }

  return selection;
}
