/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_BULK_CATALOG_ENGINE_H_
#define CHARTDLDR_PANEL_BULK_CATALOG_ENGINE_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_panel_bulk_state.h"
#include "chartdldr_catalog_download.h"
#include "chartdldr_temp_download.h"

#include "ocpn_plugin.h"

class ChartDldrPanelImpl;
class chartdldr_pi;

class ChartDldrPanelBulkCatalogEngine {
public:
  explicit ChartDldrPanelBulkCatalogEngine(ChartDldrPanelImpl& panel);

  /** Catalog apply paths take CatalogUiPolicy only (project session at the
   * boundary via policy.CatalogApply). */
  void ReloadCatalogFromDisk(int catalog_index,
                             const ChartDldrCatalogUiPolicy& ui);
  void PrepareBulkCatalog(int catalog_index,
                          const ChartDldrCatalogUiPolicy& ui);
  /** Manual catalog refresh (modal progress dialog). */
  bool UpdateChartListForCatalog(int catalog_index,
                                 const ChartDldrCatalogUiPolicy& ui);
  bool BeginCatalogRefresh(int catalog_index,
                           ChartDldrErrorReporting error_reporting);
  void CancelCatalogRefresh();

  void ActivateAndPrepareBulkCatalog(int catalog_index,
                                     const ChartDldrCatalogUiPolicy& ui);
  /**
   * Drive one prepare-phase action and return the walker step. Does not block
   * on transfer completion. charts_selected is set on CatalogReady;
   * catalog_stats accumulates catalog_refresh_failures on Advance failures.
   */
  ChartDldrBulkWalkStep RunBulkCatalogPrepareStep(
      ChartDldrBulkCatalogRunState& state, chartdldr_pi* pi,
      const ChartDldrBulkSessionPolicy& policy, int catalog_index,
      int* charts_selected = nullptr,
      ChartDldrBulkRunStats* catalog_stats = nullptr);

private:
  void ApplyCatalogDownloadOutcome(int catalog_index,
                                   const ChartDldrCatalogUiPolicy& ui,
                                   ChartDldrErrorReporting reporting,
                                   _OCPN_DLStatus status, const wxString& url);
  /** Interactive CatalogRefresh modal dialog path only. */
  _OCPN_DLStatus DownloadCatalogFileModal(const wxString& url,
                                          const wxString& output_path);

  ChartDldrPanelImpl& panel_;
  ChartDldrPanelBulkState& state_;
};

#endif  // CHARTDLDR_PANEL_BULK_CATALOG_ENGINE_H_
