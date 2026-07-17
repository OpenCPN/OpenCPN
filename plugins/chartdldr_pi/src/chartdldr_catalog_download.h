/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_CATALOG_DOWNLOAD_H_
#define CHARTDLDR_CATALOG_DOWNLOAD_H_

#include "chartdldr_bulk_lifecycle.h"
#include "chartdldr_temp_download.h"

#include "ocpn_plugin.h"

#include <wx/string.h>

class ChartCatalog;
class wxFileName;
class wxURI;
class wxWindow;

/** Pure interpretation of a catalog download status. */
struct ChartDldrCatalogDownloadOutcome {
  bool reload_catalog = false;
  bool report_failure = false;
  wxString failure_message;
};

ChartDldrCatalogDownloadOutcome ChartDldrCatalogDownloadOutcomeFor(
    _OCPN_DLStatus status, const wxString& url);

/** True when finalize succeeded and was not cancelled. */
bool ChartDldrCatalogRefreshSucceeded(_OCPN_DLStatus status, bool cancelled);

/**
 * Publishability survey for a loaded catalog. Load may succeed with invalid
 * header dates; publish requires title, release date, and usable chart cells.
 */
struct ChartDldrCatalogPublishSurvey {
  int total_cells = 0;
  int skipped_cells = 0;
  int usable_cells = 0;
  bool publishable = false;
};

ChartDldrCatalogPublishSurvey ChartDldrSurveyCatalogPublishable(
    const ChartCatalog& catalog);

/**
 * Full-parse TEMP catalog and require a real title, usable release date, and at
 * least one chart/cell with title plus download URL (or manual URL). Rejects
 * empty roots such as `<RncProductCatalog/>` and header-only catalogs.
 */
bool ChartDldrCatalogTempIsPublishable(const wxString& temp_path);

/**
 * Like ChartDldrCompleteTempDownloadPaths, but validates catalog XML before
 * publishing so a malformed HTTP 200 cannot replace a good live catalog.
 */
_OCPN_DLStatus ChartDldrCompleteValidatedCatalogDownloadPaths(
    const ChartDldrTempDownloadPaths& paths, bool transfer_success,
    bool cancelled);

void ChartDldrReportCatalogPreparePathFailure(wxWindow* parent,
                                              ChartDldrErrorReporting reporting,
                                              const wxURI& url,
                                              const wxFileName& output_fn);

#endif  // CHARTDLDR_CATALOG_DOWNLOAD_H_
