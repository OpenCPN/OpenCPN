/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_IMPL_H_
#define CHARTDLDR_PANEL_IMPL_H_

#include "chartdldrgui.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_notifier.h"
#include "chartdldr_bulk_panel_view.h"
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
                           public ChartDldrCatalogView,
                           public ChartDldrChartDownloadView {
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
  ChartDldrBulkNotifier& BulkNotifier() { return *notifier_; }
  ChartDldrPanelChartListView& ChartListView() { return chart_list_view_; }
  const ChartDldrPanelChartListView& ChartListView() const {
    return chart_list_view_;
  }

  // ChartDldrCatalogView / ChartDldrChartDownloadView ----------------------
  wxWindow* AsWindow() override { return this; }
  wxEvtHandler* AsEventHandler() override { return this; }
  void SelectActiveCatalog(int catalog_index) override {
    SetSource(catalog_index);
  }
  void CaptureChartListSelectionFromWidgets() override {
    chart_list_view_.CaptureSelectionFromWidgets();
  }
  void ArmChartDownloadCancelUi() override;
  void DisarmChartDownloadCancelUi() override;
  void SetChartInfo(const wxString& info) override {
    ChartDldrPanel::SetChartInfo(info);
  }

  void SelectCatalog(int item);
  int GetSelectedCatalog() override;
  void RefreshCatalogToolbar();
  void CancelDownload();
  /** OptionsClosed preserves an active scheduled run; PluginShutdown cancels.
   */
  void CancelBulkActivity(ChartDldrBulkCancelScope scope);
  bool IsBulkRunCancelled() const;

  void ApplyBulkRunUiPolicy(const ChartDldrBulkSessionPolicy& policy);
  void EndBulkSessionUi();
  int GetCheckedChartCount() override;
  bool IsChartChecked(int index) const override;
  ChartDldrChartUpdateKind ChartKindAt(int index) const;

  void UpdateCatalogListRow(int catalog_index, const wxString& title,
                            const wxString& release_date,
                            const wxString& local_path) override;
  void FocusChartsDownloadTab() override;
  void SetDownloadChartsButtonLabel(const wxString& label);
  void SyncDownloadCancelButton();
  void ReloadCatalogChartList(ChartSource& cs, bool selnew, bool selupd,
                              bool materialize) override;
  void SetActiveCatalogContext(int catalog_index) override;
  void UpdateChartsLabelForSource(const ChartSource& cs) override;
  void RefreshChartListForSource(int catalog_index,
                                 const ChartDldrCatalogUiPolicy& ui) override;
  void UpdateDownloadProgress(
      int downloading, int to_download, int failed_downloads,
      const ChartDldrBulkTransferSlot& transfer) override;

  int BulkDownloadNotebookPage() const;
  void SetBulkDownloadNotebookPage(int page);
  void SelectBulkDownloadTab();

  /** Build/execute manual browser decisions before a selected-chart session. */
  ChartDldrBulkRunPlan BuildSelectedChartsPreflightPlan();
  void ExecuteManualDownloadPlan(const ChartDldrBulkRunPlan& plan);
  /** Render the single postflight result without a nested event loop. */
  void PresentBulkPostflight(const ChartDldrBulkPostflight& result);

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

  int GetChartCount();
  void CheckAllCharts(bool value);
  void InvertCheckAllCharts();
  void CheckNewCharts(bool value);
  void CheckUpdatedCharts(bool value);

  DECLARE_EVENT_TABLE()

private:
  friend class ChartDldrPanelChartListView;

  void ApplyCatalogToolbarState();
  void SyncDownloadChartsButtonEnabled();

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
  std::unique_ptr<ChartDldrBulkNotifier> notifier_;
};

#endif  // CHARTDLDR_PANEL_IMPL_H_
