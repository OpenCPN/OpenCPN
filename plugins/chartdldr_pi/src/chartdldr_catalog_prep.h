/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_CATALOG_PREP_H_
#define CHARTDLDR_CATALOG_PREP_H_

#include "chartdldr_chart_classify.h"

#include <wx/datetime.h>
#include <wx/string.h>

class ChartSource;

/** Last path segment of a catalog download URL (e.g. US_CA.xml). */
wxString ChartDldrCatalogFilenameFromUrl(const wxString& url);

/** Scan on-disk chart files / update metadata before catalog comparison. */
void ChartDldrPrepareChartSourceLocals(ChartSource& source);

ChartDldrChartUpdateKind ChartDldrCatalogChartStatus(
    const ChartSource& source, const wxString& chart_number,
    const wxString& filename, const wxDateTime& catalog_update);

bool ChartDldrShouldPreselectChart(ChartDldrChartUpdateKind kind,
                                   bool preselect_new, bool preselect_updated);

wxString ChartDldrChartStatusLabel(ChartDldrChartUpdateKind kind);

#endif  // CHARTDLDR_CATALOG_PREP_H_
