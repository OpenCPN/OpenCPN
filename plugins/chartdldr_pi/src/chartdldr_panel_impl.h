/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_IMPL_H_
#define CHARTDLDR_PANEL_IMPL_H_

#include "chartdldrgui.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_bulk_state.h"
#include "chartdldr_chart_classify.h"
#include "chartdldr_download_cancel.h"

#include <memory>
#include <vector>

class chartdldr_pi;
class ChartDldrBulkOrchestrate;
class ChartSource;

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
  void SelectCatalog(int item);
  int GetSelectedCatalog();
  void SetBulkUpdate(bool bulk_update);
  void onDLEvent(OCPN_downloadEvent& ev);
  void CancelDownload();
  ChartDldrBulkOrchestrate& Bulk() { return *bulk_; }
  bool IsBulkRunCancelled() const { return m_download_cancel.IsCancelled(); }

  // Bulk subsystem API (orchestration + catalog refresh modules).
  ChartDldrBulkRunUiSnapshot CaptureBulkUiSnapshot() const;
  void ApplyRunUiPolicy(const ChartDldrBulkModeProfile& profile);
  void RestoreBulkNotebookPage(int page);
  void PrepareBulkCatalog(int catalog_index,
                          const ChartDldrBulkModeProfile& profile);
  void ReloadCatalogFromDisk(int catalog_index);
  void UpdateChartListForCatalog(int catalog_index, wxCommandEvent& event,
                                 const ChartDldrBulkModeProfile& profile);
  bool BeginAsyncCatalogRefresh(int catalog_index,
                                const ChartDldrBulkModeProfile& profile);
  ChartDldrAsyncCatalogStepResult StepAsyncCatalogRefresh();
  void CancelAsyncCatalogRefresh();
  void BeginBulkChartDownload(const ChartDldrBulkModeProfile& profile,
                              ChartDldrChartBulkState& chart_bulk);
  ChartDldrBulkChartStepResult StepNextBulkChart(
      const ChartDldrBulkModeProfile& profile,
      ChartDldrChartBulkState& chart_bulk);
  void EndBulkChartDownload(const ChartDldrBulkModeProfile& profile,
                            ChartDldrChartBulkState& chart_bulk);
  bool WaitForTransfer(const ChartDldrBulkModeProfile& profile,
                       bool allow_ui_updates,
                       const ChartDldrChartBulkState* chart_bulk);
  int GetCheckedChartCount();

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
  void EnsureDownloadHandlerConnected();
  _OCPN_DLStatus DownloadCatalogFile(const wxString& url,
                                     const wxString& output_path,
                                     bool show_progress_dialog,
                                     const ChartDldrBulkModeProfile& profile);
  void OnDownloadCharts(wxCommandEvent& event) override;

  void OnSelectChartItem(wxCommandEvent& event);
  void OnSelectNewCharts(wxCommandEvent& event);
  void OnSelectUpdatedCharts(wxCommandEvent& event);
  void OnSelectAllCharts(wxCommandEvent& event);

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
  void FillFromFile(wxString url, wxString dir, bool selnew = false,
                    bool selupd = false);

  void OnContextMenu(wxMouseEvent& event) override;

  int GetChartCount();
  bool isChartChecked(int i);
  void CheckAllCharts(bool value);
  void InvertCheckAllCharts();

  void CheckNewCharts(bool value);
  void CheckUpdatedCharts(bool value);

  DECLARE_EVENT_TABLE()

private:
  void ActivateCatalogSourceContext(int catalog_index);
  void PopulateChartListFromDisk(ChartSource& cs, bool selnew, bool selupd);
  void RefreshChartListForSource(int catalog_index);
  void UpdateChartsLabelForSource(const ChartSource& cs);
  ChartDldrChartUpdateKind ChartKindAt(int index) const;

  bool DownloadChart(wxString url, wxString file, wxString title);

  ChartDldrDownloadCancelState m_download_cancel;
  chartdldr_pi* pPlugIn;
  bool m_populated;

  void OnPopupClick(wxCommandEvent& evt);
  void AppendCatalog(std::unique_ptr<ChartSource>& cs);
  void DoEditSource();

  ChartDldrBulkTransferSlot m_transfer;
  bool m_asyncCatalogRefreshActive = false;
  int m_asyncCatalogIndex = -1;
  wxString m_asyncCatalogTempPath;
  wxString m_asyncCatalogOutputPath;
  ChartDldrBulkModeProfile m_asyncCatalogProfile;

  void FinalizePendingChartDownload(int chart_index,
                                    ChartDldrChartUpdateKind kind,
                                    ChartSource& source,
                                    const wxString& full_path,
                                    ChartDldrChartBulkState& chart_bulk,
                                    const ChartDldrBulkModeProfile& profile);
  void FinalizeActiveBulkChartTransfer(ChartSource& source,
                                       ChartDldrChartBulkState& chart_bulk,
                                       const ChartDldrBulkModeProfile& profile);
  void HandleCatalogDownloadResult(int catalog_index, _OCPN_DLStatus ret,
                                   const wxString& url,
                                   const ChartDldrBulkModeProfile& profile);

  void DisableForDownload(bool enabled);
  bool m_bconnected;
  bool m_bInfoHold;
  size_t m_newCharts;
  size_t m_updatedCharts;
  std::vector<ChartDldrChartUpdateKind> m_chartKinds;

  std::unique_ptr<ChartDldrBulkOrchestrate> bulk_;
};

#endif  // CHARTDLDR_PANEL_IMPL_H_
