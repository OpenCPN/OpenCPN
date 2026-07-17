/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_BULK_STATE_H_
#define CHARTDLDR_PANEL_BULK_STATE_H_

#include "chartdldr_bulk_transfer.h"
#include "chartdldr_bulk_lifecycle.h"
#include "chartdldr_download_cancel.h"
#include "chartdldr_temp_download.h"

/** Catalog refresh fields owned with other bulk panel state. */
struct ChartDldrCatalogRefreshState {
  bool active = false;
  int index = -1;
  ChartDldrTempDownloadPaths download_paths;
  ChartDldrErrorReporting error_reporting = ChartDldrErrorReporting::Dialog;
};

/**
 * Transfer, cancel, and catalog-refresh state for bulk download flows.
 * Operation kind is derived from transfer.owner + catalog_refresh.active.
 */
struct ChartDldrPanelBulkState {
  ChartDldrBulkTransferSlot transfer;
  ChartDldrDownloadCancelState download_cancel;
  /** Meaningful while catalog_refresh.active. */
  ChartDldrCatalogRefreshState catalog_refresh;
};

#endif  // CHARTDLDR_PANEL_BULK_STATE_H_
