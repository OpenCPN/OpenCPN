/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_CHART_CLASSIFY_H_
#define CHARTDLDR_CHART_CLASSIFY_H_

enum class ChartDldrChartUpdateKind { None, New, Updated };

/** Classify from pre-download local state (before install / ChartUpdated). */
ChartDldrChartUpdateKind ChartDldrClassifyChart(bool exists_locally,
                                                bool catalog_newer_than_local);

inline bool ChartDldrChartIndexInRange(int index, int chart_count) {
  return index >= 0 && index < chart_count;
}

#endif  // CHARTDLDR_CHART_CLASSIFY_H_
