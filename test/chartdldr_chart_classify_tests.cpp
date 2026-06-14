/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_chart_classify.h"

TEST(ChartDldrChartClassify, NewWhenNotLocal) {
  EXPECT_EQ(ChartDldrClassifyChart(false, false),
            ChartDldrChartUpdateKind::New);
  EXPECT_EQ(ChartDldrClassifyChart(false, true), ChartDldrChartUpdateKind::New);
}

TEST(ChartDldrChartClassify, UpdatedWhenLocalAndCatalogNewer) {
  EXPECT_EQ(ChartDldrClassifyChart(true, true),
            ChartDldrChartUpdateKind::Updated);
}

TEST(ChartDldrChartClassify, NoneWhenLocalAndNotNewer) {
  EXPECT_EQ(ChartDldrClassifyChart(true, false),
            ChartDldrChartUpdateKind::None);
}
