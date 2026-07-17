/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_CATALOG_PREP_H_
#define CHARTDLDR_CATALOG_PREP_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"

#include <wx/datetime.h>
#include <wx/string.h>

class ChartSource;
class chartdldr_pi;
class wxFileName;
class wxURI;
class wxWindow;

/** Last path segment of a catalog download URL (e.g. US_CA.xml). */
wxString ChartDldrCatalogFilenameFromUrl(const wxString& url);

/** Scan on-disk chart files / update metadata before catalog comparison. */
void ChartDldrPrepareChartSourceLocals(ChartSource& source);

ChartDldrChartUpdateKind ChartDldrCatalogChartStatus(
    const ChartSource& source, const wxString& filename,
    const wxDateTime& catalog_update);

bool ChartDldrShouldPreselectChart(ChartDldrChartUpdateKind kind,
                                   bool preselect_new, bool preselect_updated);

wxString ChartDldrChartStatusLabel(ChartDldrChartUpdateKind kind);

bool ChartDldrRequireValidCatalogIndex(const chartdldr_pi* pi, int index,
                                       wxWindow* parent);

/** Pure path prep from URL and target directory (no plugin dependency). */
bool ChartDldrPrepareCatalogDownloadPathsFromUrlAndDir(const wxString& url_str,
                                                       const wxString& dir,
                                                       wxURI& url,
                                                       wxFileName& output_fn);

bool ChartDldrPrepareCatalogDownloadPaths(chartdldr_pi* pi, int catalog_index,
                                          wxURI& url, wxFileName& output_fn);

/**
 * Persist install metadata after extract. Returns false when stamp cannot be
 * saved (in-memory map is rolled back).
 */
bool ChartDldrCommitChartInstallStamp(ChartSource& source,
                                      const wxString& chart_filename,
                                      const wxDateTime& update_time);

#endif  // CHARTDLDR_CATALOG_PREP_H_
