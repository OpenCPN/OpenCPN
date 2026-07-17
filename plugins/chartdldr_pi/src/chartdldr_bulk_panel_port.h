/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_PANEL_PORT_H_
#define CHARTDLDR_BULK_PANEL_PORT_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_transfer.h"

#include <wx/string.h>

class ChartSource;
class chartdldr_pi;
class wxEvtHandler;
class wxWindow;

/** How ActivateCatalog updates the panel for a catalog index. */
enum class ChartDldrCatalogActivation {
  /** Plugin source id + sources-list highlight only. */
  ContextOnly,
  /** ContextOnly + reload the chart list with the given ui policy. */
  WithReload,
};

/**
 * Widget surface used by the bulk orchestrator, controllers, and chart
 * transfer helpers. Widgets and context reads only: dialogs and workflow
 * decisions live with the orchestrator (chartdldr_bulk_preflight).
 * ChartDldrPanelImpl is the only implementation.
 */
class ChartDldrBulkPanelPort {
public:
  virtual ~ChartDldrBulkPanelPort() = default;

  // identity ---------------------------------------------------------------
  virtual chartdldr_pi* GetPlugin() const = 0;
  virtual wxWindow* AsWindow() = 0;
  virtual wxEvtHandler* AsEventHandler() = 0;
  virtual bool IsShownOnScreen() const = 0;
  virtual int GetSelectedCatalog() = 0;

  // catalog activation -----------------------------------------------------
  /**
   * The single catalog-activation entry point. ContextOnly sets the active
   * plugin source id and sources-list highlight; WithReload additionally
   * reloads the chart list (and refreshes the charts label) for a valid index,
   * or clears the form for an invalid one.
   */
  virtual void ActivateCatalog(
      int catalog_index, ChartDldrCatalogActivation activation,
      const ChartDldrCatalogUiPolicy& ui = ChartDldrCatalogUiPolicy()) = 0;

  // catalog / chart list UI ------------------------------------------------
  virtual int GetChartCount() = 0;
  virtual int GetCheckedChartCount() = 0;
  virtual bool IsChartChecked(int index) const = 0;
  virtual void CaptureChartListSelectionFromWidgets() = 0;
  virtual void FocusChartsDownloadTab() = 0;
  virtual void UpdateCatalogListRow(int catalog_index, const wxString& title,
                                    const wxString& release_date,
                                    const wxString& local_path) = 0;
  virtual void UpdateChartsLabelForSource(const ChartSource& cs) = 0;
  /** Re-render the download button from the session cancel phase. */
  virtual void SyncDownloadCancelUi() = 0;
  virtual void UpdateDownloadProgress(
      int downloading, int to_download, int failed_downloads,
      const ChartDldrBulkTransferSlot& transfer) = 0;
  virtual void SetChartInfo(const wxString& info) = 0;

  // session lifecycle ------------------------------------------------------
  virtual int BulkDownloadNotebookPage() const = 0;
  virtual void SetBulkDownloadNotebookPage(int page) = 0;
  virtual void RefreshCatalogToolbar() = 0;
};

#endif  // CHARTDLDR_BULK_PANEL_PORT_H_
