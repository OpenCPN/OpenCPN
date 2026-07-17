/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_IMPL_H_
#define CHARTDLDR_PANEL_IMPL_H_

#include "chartdldrgui.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_run.h"
#include "chartdldr_bulk_notifier.h"
#include "chartdldr_panel_bulk_state.h"
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

/** RAII: marks the panel (and global) bulk-pump active. */
class ChartDldrBulkPumpGuard {
public:
  explicit ChartDldrBulkPumpGuard(ChartDldrPanelImpl& panel);
  ~ChartDldrBulkPumpGuard();

  ChartDldrBulkPumpGuard(const ChartDldrBulkPumpGuard&) = delete;
  ChartDldrBulkPumpGuard& operator=(const ChartDldrBulkPumpGuard&) = delete;

private:
  ChartDldrPanelImpl& panel_;
};

/** wx panel implementation for the chart downloader options page. */
class ChartDldrPanelImpl : public ChartDldrPanel {
public:
  ~ChartDldrPanelImpl();
  ChartDldrPanelImpl(chartdldr_pi* plugin = NULL, wxWindow* parent = NULL,
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

  chartdldr_pi* GetPlugin() const { return pPlugIn; }
  ChartDldrPanelBulkState& BulkState() { return m_bulkState_; }
  const ChartDldrPanelBulkState& BulkState() const { return m_bulkState_; }
  ChartDldrBulkOrchestrate& Bulk() { return *bulk_; }
  const ChartDldrBulkOrchestrate& Bulk() const { return *bulk_; }
  ChartDldrBulkNotifier& BulkNotifier() { return *notifier_; }
  ChartDldrPanelChartListView& ChartListView() { return chart_list_view_; }
  const ChartDldrPanelChartListView& ChartListView() const {
    return chart_list_view_;
  }

  // Bulk engines drive the panel directly (no forwarding facade): the panel
  // *is* the collaborator, so these expose the few extra operations the walk
  // needs without a mirror class.
  wxWindow* AsWindow() { return this; }
  wxEvtHandler* AsEventHandler() { return this; }
  void SelectActiveCatalog(int catalog_index) { SetSource(catalog_index); }
  void CaptureChartListSelectionFromWidgets() {
    chart_list_view_.CaptureSelectionFromWidgets();
  }
  void ArmChartDownloadCancelUi() {
    m_bulkState_.download_cancel.BeginActiveDownload();
    SyncDownloadCancelButton();
  }
  void DisarmChartDownloadCancelUi() {
    m_bulkState_.download_cancel.EndActiveDownload();
    SyncDownloadCancelButton();
  }

  void SelectCatalog(int item);
  int GetSelectedCatalog();
  void RefreshCatalogToolbar();
  void CancelDownload();
  /** OptionsClosed preserves an active scheduled run; PluginShutdown cancels.
   */
  void CancelBulkActivity(ChartDldrBulkCancelScope scope);
  bool IsBulkRunCancelled() const {
    return m_bulkState_.download_cancel.IsSessionCancelled();
  }

  void ApplyBulkRunUiPolicy(const ChartDldrBulkSessionPolicy& policy);
  void EndBulkSessionUi();
  int GetCheckedChartCount();
  bool IsChartChecked(int index) const;
  ChartDldrChartUpdateKind ChartKindAt(int index) const;

  void UpdateCatalogListRow(int catalog_index, const wxString& title,
                            const wxString& release_date,
                            const wxString& local_path);
  void FocusChartsDownloadTab();
  void SetDownloadChartsButtonLabel(const wxString& label);
  void SyncDownloadCancelButton();
  void ReloadCatalogChartList(ChartSource& cs, bool selnew, bool selupd,
                              bool materialize);
  void SetActiveCatalogContext(int catalog_index);
  void UpdateChartsLabelForSource(const ChartSource& cs);
  void RefreshChartListForSource(int catalog_index,
                                 const ChartDldrCatalogUiPolicy& ui);
  void UpdateDownloadProgress(int downloading, int to_download,
                              int failed_downloads,
                              const ChartDldrBulkTransferSlot& transfer);

  int BulkDownloadNotebookPage() const;
  void SetBulkDownloadNotebookPage(int page);
  void SelectBulkDownloadTab();

  void EnterBulkPump();
  void LeaveBulkPump();
  /** Invalidate queued ScheduleBulkPump CallAfters (DeInit / destroy). */
  void InvalidatePendingBulkPumps();
  void ScheduleBulkPump();
  void SetTransferStallTimerRunning(bool running);

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
  friend class ChartDldrBulkPumpGuard;

  void ApplyCatalogToolbarState();
  void SyncDownloadChartsButtonEnabled();
  void OnTransferStallTimer(wxTimerEvent& event);

  void LoadCatalogSelectionFromDisk(const wxString& path, bool selnew,
                                    bool selupd);

  ChartDldrPanelBulkState m_bulkState_;
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

  uint64_t bulk_pump_epoch_ = 0;
  std::unique_ptr<wxTimer> transfer_stall_timer_;
};

#endif  // CHARTDLDR_PANEL_IMPL_H_
