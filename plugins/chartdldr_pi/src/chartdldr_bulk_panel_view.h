/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_PANEL_VIEW_H_
#define CHARTDLDR_BULK_PANEL_VIEW_H_

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_selection.h"

#include <wx/string.h>

class ChartSource;
class chartdldr_pi;
class wxEvtHandler;
class wxWindow;

/**
 * Catalog-prepare / catalog-list surface used by
 * ChartDldrPanelBulkCatalogController. Chart-download UI lives on
 * ChartDldrChartDownloadView.
 */
class ChartDldrCatalogView {
public:
  virtual ~ChartDldrCatalogView() = default;

  virtual chartdldr_pi* GetPlugin() const = 0;
  virtual wxWindow* AsWindow() = 0;
  virtual wxEvtHandler* AsEventHandler() = 0;

  virtual int GetCheckedChartCount() = 0;
  virtual void SetActiveCatalogContext(int catalog_index) = 0;
  virtual void CaptureChartListSelectionFromWidgets() = 0;
  virtual void FocusChartsDownloadTab() = 0;
  virtual void UpdateCatalogListRow(int catalog_index, const wxString& title,
                                    const wxString& release_date,
                                    const wxString& local_path) = 0;
  virtual void UpdateChartsLabelForSource(const ChartSource& cs) = 0;
  virtual void ReloadCatalogChartList(ChartSource& cs, bool selnew, bool selupd,
                                      bool materialize) = 0;
};

/**
 * Chart-download loop surface used by ChartDldrPanelBulkChartController and
 * chart transfer helpers. Catalog UI lives on ChartDldrCatalogView.
 */
class ChartDldrChartDownloadView {
public:
  virtual ~ChartDldrChartDownloadView() = default;

  virtual chartdldr_pi* GetPlugin() const = 0;
  virtual wxWindow* AsWindow() = 0;
  virtual wxEvtHandler* AsEventHandler() = 0;

  virtual void ArmChartDownloadCancelUi() = 0;
  virtual void DisarmChartDownloadCancelUi() = 0;

  virtual int GetCheckedChartCount() = 0;
  virtual bool IsChartChecked(int index) const = 0;
  virtual int GetSelectedCatalog() = 0;
  virtual void SelectActiveCatalog(int catalog_index) = 0;
  virtual void RefreshChartListForSource(
      int catalog_index, const ChartDldrCatalogUiPolicy& ui) = 0;
  virtual void UpdateDownloadProgress(
      int downloading, int to_download, int failed_downloads,
      const ChartDldrBulkTransferSlot& transfer) = 0;
  virtual void SetChartInfo(const wxString& info) = 0;
};

#endif  // CHARTDLDR_BULK_PANEL_VIEW_H_
