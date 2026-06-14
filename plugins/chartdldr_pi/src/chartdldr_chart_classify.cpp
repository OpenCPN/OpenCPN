/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_chart_classify.h"

ChartDldrChartUpdateKind ChartDldrClassifyChart(bool exists_locally,
                                                bool catalog_newer_than_local) {
  if (!exists_locally) {
    return ChartDldrChartUpdateKind::New;
  }
  if (catalog_newer_than_local) {
    return ChartDldrChartUpdateKind::Updated;
  }
  return ChartDldrChartUpdateKind::None;
}
