/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_CATALOG_H_
#define CHARTDLDR_BULK_CATALOG_H_

#include "chartdldr_bulk.h"

#include <wx/string.h>

class chartdldr_pi;
class ChartDldrPanelImpl;
class wxFileName;
class wxURI;
class wxWindow;

struct ChartDldrBulkModeProfile;

bool ChartDldrBeginCatalogRefresh(ChartDldrPanelImpl& panel, int catalog_index,
                                  const ChartDldrBulkModeProfile& profile,
                                  const ChartDldrCatalogRefreshContext& ctx);

ChartDldrAsyncCatalogStepResult ChartDldrStepCatalogRefresh(
    ChartDldrPanelImpl& panel, const ChartDldrBulkModeProfile& profile);

namespace ChartDldrBulkCatalogInternal {

bool PrepareDownloadPaths(int catalog_index, chartdldr_pi* pi, wxURI& url,
                          wxFileName& output_fn);

wxString BackgroundDownloadTargetPath(const wxFileName& output_fn);

void ReportPathFailure(wxWindow* parent,
                       const ChartDldrBulkModeProfile& profile,
                       const wxURI& url, const wxFileName& output_fn);

}  // namespace ChartDldrBulkCatalogInternal

#endif  // CHARTDLDR_BULK_CATALOG_H_
