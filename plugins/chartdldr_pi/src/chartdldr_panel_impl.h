/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_IMPL_H_
#define CHARTDLDR_PANEL_IMPL_H_

#include "chartdldrgui.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_orchestrate.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_chart_classify.h"

class chartdldr_pi;
class ChartSource;

/** wx panel implementation for the chart downloader options page. */
class ChartDldrPanelImpl : public ChartDldrPanel {
  friend class chartdldr_pi;
  friend class ChartDldrBulkOrchestrate;

private:
  bool DownloadChart(wxString url, wxString file, wxString title);
  int to_download;

  bool cancelled;
  bool DownloadIsCancel;
  chartdldr_pi* pPlugIn;
  bool m_populated;

  void OnPopupClick(wxCommandEvent& evt);
  int GetSelectedCatalog();
  void AppendCatalog(std::unique_ptr<ChartSource>& cs);
  void DoEditSource();

  ChartDldrBulkTransferSlot m_transfer;
  int m_failed_downloads;
  int m_downloading;
  int m_bulkDownloadedNew;
  int m_bulkDownloadedUpdated;
  int m_bulkChartLoopIndex = 0;
  bool m_bulkChartDownloadActive = false;
  int m_bulkChartPendingIndex = -1;
  ChartDldrChartUpdateKind m_bulkChartPendingKind =
      ChartDldrChartUpdateKind::None;
  wxString m_bulkChartPendingPath;
  bool m_asyncCatalogRefreshActive = false;
  int m_asyncCatalogIndex = -1;
  wxString m_asyncCatalogTempPath;
  wxString m_asyncCatalogOutputPath;
  ChartDldrBulkModeProfile m_asyncCatalogProfile;

  ChartDldrChartUpdateKind ClassifyChartForDownload(int chart_index);
  void FinalizePendingChartDownload(int chart_index,
                                    ChartDldrChartUpdateKind kind,
                                    ChartSource& source,
                                    const wxString& full_path);
  void FinalizeActiveBulkChartTransfer(ChartSource& source);
  void HandleCatalogDownloadResult(int catalog_index, _OCPN_DLStatus ret,
                                   const wxString& url,
                                   const ChartDldrBulkModeProfile& profile);

  void DownloadCharts(const ChartDldrBulkModeProfile& profile);
  void BeginBulkChartDownload(const ChartDldrBulkModeProfile& profile);
  ChartDldrBulkChartStepResult StepNextBulkChart(
      const ChartDldrBulkModeProfile& profile);
  void EndBulkChartDownload(const ChartDldrBulkModeProfile& profile);
  bool IsBulkRunCancelled() const { return cancelled; }
  bool IsBulkChartDownloadActive() const { return m_bulkChartDownloadActive; }
  ChartDldrBulkRunStats TakeBulkCatalogStats() const;
  void ResetBulkCatalogCounters();
  void RestoreBulkNotebookPage(int page);
  chartdldr_pi* GetPlugin() const { return pPlugIn; }
  ChartDldrBulkRunUiSnapshot CaptureBulkUiSnapshot() const;
  void BeginBulkRunSession(const ChartDldrBulkModeProfile& profile);
  void PrepareBulkCatalog(int catalog_index, bool sync_list_selection);
  void UpdateChartListForCatalog(int catalog_index, wxCommandEvent& event,
                                 const ChartDldrBulkModeProfile& profile);
  bool BeginAsyncCatalogRefresh(int catalog_index,
                                const ChartDldrBulkModeProfile& profile);
  ChartDldrAsyncCatalogStepResult StepAsyncCatalogRefresh();
  void CancelAsyncCatalogRefresh();

  void DisableForDownload(bool enabled);
  bool m_bconnected;
  bool m_bInfoHold;
  size_t m_newCharts;
  size_t m_updatedCharts;

protected:
  void SetSource(int id);
  void SelectSource(wxListEvent& event) override;
  void AddSource(wxCommandEvent& event) override;
  void DeleteSource(wxCommandEvent& event) override;
  void EditSource(wxCommandEvent& event) override;
  void UpdateChartList(wxCommandEvent& event) override;
  void EnsureDownloadHandlerConnected();
  bool WaitForBackgroundDownload(bool allow_ui_updates);
  _OCPN_DLStatus DownloadCatalogFile(const wxString& url,
                                     const wxString& output_path,
                                     bool show_progress_dialog);
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
  void SetBulkUpdate(bool bulk_update);

  int GetChartCount();
  int GetCheckedChartCount();
  bool isChartChecked(int i);
  void CheckAllCharts(bool value);
  void InvertCheckAllCharts();

  void CheckNewCharts(bool value);
  void CheckUpdatedCharts(bool value);

public:
  ~ChartDldrPanelImpl();
  ChartDldrPanelImpl(chartdldr_pi* plugin = NULL, wxWindow* parent = NULL,
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
  void SelectCatalog(int item);
  void onDLEvent(OCPN_downloadEvent& ev);
  void CancelDownload() {
    Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
    cancelled = true;
    m_bconnected = false;
  }

  ChartDldrBulkOrchestrate& Bulk() { return bulk_; }

  DECLARE_EVENT_TABLE()

private:
  ChartDldrBulkOrchestrate bulk_;
};

#endif  // CHARTDLDR_PANEL_IMPL_H_
