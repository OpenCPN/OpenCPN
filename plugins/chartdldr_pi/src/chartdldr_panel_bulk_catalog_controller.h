/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_BULK_CATALOG_CONTROLLER_H_
#define CHARTDLDR_PANEL_BULK_CATALOG_CONTROLLER_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_state.h"
#include "chartdldr_catalog_download.h"
#include "chartdldr_temp_download.h"

#include "ocpn_plugin.h"

class ChartDldrBulkPanelPort;
class chartdldr_pi;

class ChartDldrBulkCatalogController {
public:
  ChartDldrBulkCatalogController(ChartDldrBulkPanelPort& port,
                                 ChartDldrBulkRunSession& session);

  /** Apply the catalog UI policy: preserve selection or reload + row update. */
  void ApplyCatalogUi(int catalog_index, const ChartDldrCatalogUiPolicy& ui);
  bool BeginCatalogRefresh(int catalog_index,
                           ChartDldrErrorReporting error_reporting);
  void CancelCatalogRefresh();

  /**
   * Drive one prepare-phase action and return the walker step. Does not block
   * on transfer completion. Reads the plugin and policy from the session.
   * charts_selected is set on CatalogReady; catalog_stats accumulates
   * catalog_refresh_failures on Advance failures.
   */
  ChartDldrBulkWalkStep RunBulkCatalogPrepareStep(
      int catalog_index, int* charts_selected = nullptr,
      ChartDldrBulkRunStats* catalog_stats = nullptr);

private:
  void ApplyCatalogDownloadOutcome(int catalog_index,
                                   const ChartDldrCatalogUiPolicy& ui,
                                   ChartDldrErrorReporting reporting,
                                   _OCPN_DLStatus status, const wxString& url);

  ChartDldrBulkPanelPort& port_;
  ChartDldrBulkRunSession& session_;
};

#endif  // CHARTDLDR_PANEL_BULK_CATALOG_CONTROLLER_H_
