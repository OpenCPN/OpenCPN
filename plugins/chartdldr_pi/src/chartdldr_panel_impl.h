/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_IMPL_H_
#define CHARTDLDR_PANEL_IMPL_H_

#include "chartdldrgui.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_panel_port.h"
#include "chartdldr_bulk_state.h"
#include "chartdldr_catalog_selection.h"
#include "chartdldr_bulk_chart_loop.h"
#include "chartdldr_panel_chart_list_view.h"

#include <memory>
#include <vector>

class ChartDldrBulkOrchestrate;
class ChartSource;
class chartdldr_pi;
class wxPanel;
class wxTimer;
class ChartDldrPanelImpl;

/** wx panel implementation for the chart downloader options page. */
class ChartDldrPanelImpl : public ChartDldrPanel,
                           public ChartDldrBulkPanelPort {
public:
  ~ChartDldrPanelImpl() override;
  ChartDldrPanelImpl(chartdldr_pi* plugin = NULL, wxWindow* parent = NULL,
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

  chartdldr_pi* GetPlugin() const override { return pPlugIn; }
  ChartDldrBulkOrchestrate& Bulk() { return *bulk_; }
  const ChartDldrBulkOrchestrate& Bulk() const { return *bulk_; }
  ChartDldrPanelChartListView& ChartListView() { return chart_list_view_; }
  const ChartDldrPanelChartListView& ChartListView() const {
    return chart_list_view_;
  }

  // ChartDldrBulkPanelPort -------------------------------------------------
  bool IsShownOnScreen() const override {
    return ChartDldrPanel::IsShownOnScreen();
  }

  wxWindow* AsWindow() override { return this; }
  wxEvtHandler* AsEventHandler() override { return this; }
  void ActivateCatalog(
      int catalog_index, ChartDldrCatalogActivation activation,
      const ChartDldrCatalogUiPolicy& ui = ChartDldrCatalogUiPolicy()) override;
  void CaptureChartListSelectionFromWidgets() override {
    chart_list_view_.CaptureSelectionFromWidgets();
  }
  void SetChartInfo(const wxString& info) override {
    ChartDldrPanel::SetChartInfo(info);
  }

  void SelectCatalog(int item);
  int GetSelectedCatalog() override;
  void RefreshCatalogToolbar() override;
  /** OptionsClosed preserves an active scheduled run; PluginShutdown cancels.
   */
  void CancelBulkActivity(ChartDldrBulkCancelScope scope);

  int GetChartCount() override;
  int GetCheckedChartCount() override;
  bool IsChartChecked(int index) const override;
  ChartDldrChartUpdateKind ChartKindAt(int index) const;

  void UpdateCatalogListRow(int catalog_index, const wxString& title,
                            const wxString& release_date,
                            const wxString& local_path) override;
  void FocusChartsDownloadTab() override;
  void SetDownloadChartsButtonLabel(const wxString& label);
  void SyncDownloadCancelUi() override;
  void UpdateChartsLabelForSource(const ChartSource& cs) override;
  void UpdateDownloadProgress(
      int downloading, int to_download, int failed_downloads,
      const ChartDldrBulkTransferSlot& transfer) override;

  int BulkDownloadNotebookPage() const override;
  void SetBulkDownloadNotebookPage(int page) override;

#if defined(CHART_LIST)
  bool isNew(int item) override;
  bool isUpdated(int item) override;
#endif /* CHART_LIST */

protected:
  void SetSource(int id);
  void SelectSource(wxListEvent& event) override;
  void AddSource(wxCommandEvent& event) override;
  void DeleteSource(wxCommandEvent& event) override;
  void EditSource(wxCommandEvent& event) override;
  void UpdateChartList(wxCommandEvent& event) override;
  void OnDownloadCharts(wxCommandEvent& event) override;

#if defined(CHART_LIST)
  void OnSelectChartItem(wxDataViewEvent& event);
  void OnChartListContextMenu(wxDataViewEvent& event);
  void OnSelectNewCharts(wxCommandEvent& event) override;
  void OnSelectUpdatedCharts(wxCommandEvent& event) override;
  void OnSelectAllCharts(wxCommandEvent& event) override;
#endif /* CHART_LIST */

  void DoHelp(wxCommandEvent& event) override {
#ifdef __WXMSW__
    wxLaunchDefaultBrowser(_T("file:///") + *GetpSharedDataLocation() +
                           _T("plugins/chartdldr_pi/data/doc/index.html"));
#else
    wxLaunchDefaultBrowser(_T("file://") + *GetpSharedDataLocation() +
                           _T("plugins/chartdldr_pi/data/doc/index.html"));
#endif
  }
  void UpdateAllCharts(wxCommandEvent& event) override;
  void OnShowLocalDir(wxCommandEvent& event) override;
  void OnPaint(wxPaintEvent& event) override;
  void OnLeftDClick(wxMouseEvent& event) override;

  void CleanForm();
  void OnContextMenu(wxMouseEvent& event) override;

  DECLARE_EVENT_TABLE()

private:
  friend class ChartDldrPanelChartListView;

  void ApplyCatalogToolbarState();
  void SyncDownloadChartsButtonEnabled();

  /** Set the active plugin source id and sources-list highlight. */
  void SetActiveCatalogContext(int catalog_index);
  /** Reload the chart list widgets for the given source from disk. */
  void ReloadCatalogChartList(ChartSource& cs, bool selnew, bool selupd,
                              bool materialize);
  /** Read the sources-list widget selection (raw click), not the active id. */
  int ReadSelectedCatalogFromWidget() const;

  void LoadCatalogSelectionFromDisk(const wxString& path, bool selnew,
                                    bool selupd);

  chartdldr_pi* pPlugIn;
  bool m_populated;

  void OnPopupClick(wxCommandEvent& evt);
  void AppendCatalog(std::unique_ptr<ChartSource>& cs);
  void DoEditSource();
  void BindPanelEventHandlers();
  void MarkMacCatalogListNeedsRefresh();

  bool m_macCatalogListNeedsRefresh = true;
  bool m_bInfoHold;
  ChartDldrCatalogSelection m_catalogSelection;

  ChartDldrPanelChartListView chart_list_view_;
  std::unique_ptr<ChartDldrBulkOrchestrate> bulk_;
};

#endif  // CHARTDLDR_PANEL_IMPL_H_
