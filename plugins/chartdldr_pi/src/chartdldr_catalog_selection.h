/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_CATALOG_SELECTION_H_
#define CHARTDLDR_CATALOG_SELECTION_H_

#include "chartdldr_bulk_catalog_run.h"

#include <cstddef>
#include <vector>

class ChartCatalog;
class ChartSource;

/** In-memory chart update/selection state for bulk download (UI optional). */
struct ChartDldrCatalogSelection {
  std::vector<ChartDldrChartUpdateKind> kinds;
  std::vector<bool> checked;
  size_t new_count = 0;
  size_t updated_count = 0;

  void Clear();
  int Count() const;
  int CheckedCount() const;
  bool IsChecked(int index) const;
  ChartDldrChartUpdateKind KindAt(int index) const;
  void SetChecked(int index, bool value);
  void SetAllChecked(bool value);
  void SetCheckedForKind(ChartDldrChartUpdateKind kind, bool value);
  void InvertAllChecked();
};

ChartDldrCatalogSelection ChartDldrBuildCatalogSelection(
    const ChartCatalog& catalog, const ChartSource& source, bool preselect_new,
    bool preselect_updated);

#endif  // CHARTDLDR_CATALOG_SELECTION_H_
