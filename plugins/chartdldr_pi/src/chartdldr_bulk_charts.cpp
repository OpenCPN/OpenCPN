/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_format.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"
#include "chartdldr_bulk.h"
#include "chartdldr_chart_classify.h"

#include <wx/filename.h>
#include <wx/url.h>

namespace {

void UpdateBulkDownloadProgressUi(ChartDldrPanelImpl* panel, int downloading,
                                  int to_download, int failed_downloads,
                                  const ChartDldrBulkTransferSlot& transfer) {
  if (failed_downloads) {
    panel->SetChartInfo(wxString::Format(
        _("Downloading chart %u of %u, %u downloads failed (%s / %s)"),
        downloading, to_download, failed_downloads,
        ChartDldrFormatBytes(transfer.transferred_size),
        ChartDldrFormatBytes(transfer.total_size)));
  } else {
    panel->SetChartInfo(wxString::Format(
        _("Downloading chart %u of %u (%s / %s)"), downloading, to_download,
        ChartDldrFormatBytes(transfer.transferred_size),
        ChartDldrFormatBytes(transfer.total_size)));
  }
  panel->Update();
  panel->Refresh();
}

}  // namespace

ChartDldrChartUpdateKind ChartDldrPanelImpl::ClassifyChartForDownload(
    int chart_index) {
  if (!pPlugIn->m_pChartSource || chart_index < 0 ||
      chart_index >= static_cast<int>(pPlugIn->m_pChartCatalog.charts.size())) {
    return ChartDldrChartUpdateKind::None;
  }
  Chart* chart = pPlugIn->m_pChartCatalog.charts.at(chart_index).get();
  const wxString file = chart->GetChartFilename(true);
  const bool exists =
      pPlugIn->m_pChartSource->ExistsLocaly(chart->number, file);
  const bool newer =
      exists && pPlugIn->m_pChartSource->IsNewerThanLocal(
                    chart->number, file, chart->GetUpdateDatetime());
  return ChartDldrClassifyChart(exists, newer);
}

void ChartDldrPanelImpl::FinalizePendingChartDownload(
    int chart_index, ChartDldrChartUpdateKind kind, ChartSource& source,
    const wxString& full_path, ChartDldrChartBulkState& chart_bulk) {
  if (chart_index < 0 ||
      chart_index >= static_cast<int>(pPlugIn->m_pChartCatalog.charts.size())) {
    return;
  }

  wxFileName fn(full_path);
  if (pPlugIn->ProcessFile(full_path, fn.GetPath(), true,
                           pPlugIn->m_pChartCatalog.charts.at(chart_index)
                               ->GetUpdateDatetime())) {
    source.ChartUpdated(pPlugIn->m_pChartCatalog.charts.at(chart_index)->number,
                        pPlugIn->m_pChartCatalog.charts.at(chart_index)
                            ->GetUpdateDatetime()
                            .GetTicks());
    if (kind == ChartDldrChartUpdateKind::New) {
      chart_bulk.new_downloads++;
    } else if (kind == ChartDldrChartUpdateKind::Updated) {
      chart_bulk.updated_downloads++;
    }
  } else {
    chart_bulk.failed++;
  }
}

ChartDldrBulkRunUiSnapshot ChartDldrPanelImpl::CaptureBulkUiSnapshot() const {
  ChartDldrBulkRunUiSnapshot snapshot;
  snapshot.panel_visible = wxPanel::IsShownOnScreen();
  snapshot.notebook_page = m_DLoadNB->GetSelection();
  return snapshot;
}

void ChartDldrPanelImpl::ApplyRunUiPolicy(
    const ChartDldrBulkModeProfile& profile) {
  m_download_cancel.ResetForBulkRun();
  if (profile.ui.select_download_tab) {
    m_DLoadNB->SetSelection(1);
  }
}

void ChartDldrPanelImpl::RestoreBulkNotebookPage(int page) {
  m_DLoadNB->SetSelection(page);
}

void ChartDldrPanelImpl::BeginBulkChartDownload(
    const ChartDldrBulkModeProfile& profile,
    ChartDldrChartBulkState& chart_bulk) {
  EnsureDownloadHandlerConnected();

  const int chart_count =
      static_cast<int>(pPlugIn->m_pChartCatalog.charts.size());
  int selected = 0;
  for (int i = 0; i < chart_count; ++i) {
    if (isChartChecked(i)) {
      ++selected;
    }
  }
  if (!selected && !profile.charts.allow_empty_selection) {
    OCPNMessageBox_PlugIn(this, _("No charts selected for download."));
    return;
  }
  const int catalog_index = pPlugIn->GetSourceId();
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }

  m_download_cancel.BeginActiveDownload();
  chart_bulk.to_download = selected;
  chart_bulk.downloading = 0;
  chart_bulk.failed = 0;
  chart_bulk.new_downloads = 0;
  chart_bulk.updated_downloads = 0;
  chart_bulk.loop_index = 0;
  chart_bulk.pending_index = -1;
  chart_bulk.pending_path.clear();
  chart_bulk.active = true;

  DisableForDownload(false);
  m_bDnldCharts->SetLabel(_("Abort download"));
  m_transfer.Reset();
}

void ChartDldrPanelImpl::FinalizeActiveBulkChartTransfer(
    ChartSource& source, ChartDldrChartBulkState& chart_bulk) {
  if (m_download_cancel.IsCancelled()) {
    if (m_transfer.handle) {
      OCPN_cancelDownloadFileBackground(m_transfer.handle);
    }
    m_transfer.Reset();
    return;
  }

  if (m_transfer.success) {
    FinalizePendingChartDownload(chart_bulk.pending_index,
                                 chart_bulk.pending_kind, source,
                                 chart_bulk.pending_path, chart_bulk);
  } else {
    if (wxFileExists(chart_bulk.pending_path)) {
      wxRemoveFile(chart_bulk.pending_path);
    }
    chart_bulk.failed++;
  }

  m_transfer.Reset();
}

ChartDldrBulkChartStepResult ChartDldrPanelImpl::StepNextBulkChart(
    const ChartDldrBulkModeProfile& profile,
    ChartDldrChartBulkState& chart_bulk) {
  if (!chart_bulk.active) {
    return ChartDldrBulkChartStepResult::NotActive;
  }

  if (m_download_cancel.IsCancelled()) {
    return ChartDldrBulkChartStepResult::Finished;
  }

  const bool allow_download_ui_updates =
      profile.ui.show_download_progress_ui && wxPanel::IsShownOnScreen();
  const int catalog_index = pPlugIn->GetSourceId();
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return ChartDldrBulkChartStepResult::Finished;
  }
  std::unique_ptr<ChartSource>& cs = pPlugIn->m_ChartSources.at(catalog_index);

  const int chart_count =
      static_cast<int>(pPlugIn->m_pChartCatalog.charts.size());

  if (m_transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    if (m_transfer.IsInProgress() && !m_download_cancel.IsCancelled()) {
      if (allow_download_ui_updates) {
        UpdateBulkDownloadProgressUi(this, chart_bulk.downloading,
                                     chart_bulk.to_download, chart_bulk.failed,
                                     m_transfer);
      }
      return ChartDldrBulkChartStepResult::TransferInProgress;
    }

    FinalizeActiveBulkChartTransfer(*cs, chart_bulk);
    if (m_download_cancel.IsCancelled()) {
      return ChartDldrBulkChartStepResult::Finished;
    }

    if (chart_bulk.loop_index < chart_count && chart_bulk.to_download) {
      return ChartDldrBulkChartStepResult::MoreCharts;
    }
    return ChartDldrBulkChartStepResult::Finished;
  }

  for (int i = chart_bulk.loop_index; i < chart_count && chart_bulk.to_download;
       ++i) {
    chart_bulk.loop_index = i + 1;
    if (m_download_cancel.IsCancelled()) {
      return ChartDldrBulkChartStepResult::Finished;
    }
    if (!isChartChecked(i)) {
      continue;
    }

    const int index = i;
    if (pPlugIn->m_pChartCatalog.charts.at(index)->NeedsManualDownload()) {
      if (profile.charts.skip_manual_charts) {
        continue;
      }
      if (wxID_YES ==
          OCPNMessageBox_PlugIn(
              this,
              wxString::Format(
                  _("The selected chart '%s' can't be downloaded automatically, do you want me to open a browser window and download them manually?\n\n \
After downloading the charts, please extract them to %s"),
                  pPlugIn->m_pChartCatalog.charts.at(index)->title.c_str(),
                  pPlugIn->m_pChartSource->GetDir().c_str()),
              _("Chart Downloader"), wxYES_NO | wxCENTRE | wxICON_QUESTION)) {
        wxLaunchDefaultBrowser(
            pPlugIn->m_pChartCatalog.charts.at(index)->GetManualDownloadUrl());
      }
      continue;
    }

    wxURI url(pPlugIn->m_pChartCatalog.charts.at(index)->GetDownloadLocation());
    if (url.IsReference()) {
      OCPNMessageBox_PlugIn(
          this,
          wxString::Format(
              _("Error, the URL to the chart (%s) data seems wrong."),
              url.BuildURI().c_str()),
          _("Error"));
      this->Enable();
      chart_bulk.active = false;
      return ChartDldrBulkChartStepResult::Finished;
    }
    wxString file =
        pPlugIn->m_pChartCatalog.charts.at(index)->GetChartFilename();
    wxFileName fn;
    fn.SetFullName(file);
    fn.SetPath(cs->GetDir());
    wxString path = fn.GetFullPath();
    if (wxFileExists(path)) {
      wxRemoveFile(path);
    }

#ifdef __ANDROID__
    wxString file_path = _T("file://") + fn.GetFullPath();
#else
    wxString file_path = fn.GetFullPath();
#endif

    chart_bulk.downloading++;
    chart_bulk.pending_index = index;
    chart_bulk.pending_kind = ClassifyChartForDownload(index);
    chart_bulk.pending_path = path;

    m_transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
    OCPN_downloadFileBackground(url.BuildURI(), file_path, this,
                                &m_transfer.handle);
    return ChartDldrBulkChartStepResult::TransferInProgress;
  }

  return ChartDldrBulkChartStepResult::Finished;
}

void ChartDldrPanelImpl::EndBulkChartDownload(
    const ChartDldrBulkModeProfile& profile,
    ChartDldrChartBulkState& chart_bulk) {
  if (!chart_bulk.active) {
    return;
  }

  if (m_transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk) &&
      m_transfer.handle) {
    OCPN_cancelDownloadFileBackground(m_transfer.handle);
    m_transfer.Reset();
  }

  DisableForDownload(true);
  m_bDnldCharts->SetLabel(_("Download selected charts"));
  m_download_cancel.EndActiveDownload();
  chart_bulk.active = false;

  const int restore_catalog = pPlugIn->GetSourceId();
  if (restore_catalog >= 0) {
    SetSource(restore_catalog);
  } else {
    SetSource(GetSelectedCatalog());
  }

  if (chart_bulk.failed > 0 && profile.ui.show_download_result_dialogs &&
      !m_download_cancel.IsCancelled()) {
    OCPNMessageBox_PlugIn(
        this,
        wxString::Format(_("%d out of %d charts failed to download.\nCheck the "
                           "list, verify there is a working Internet "
                           "connection and repeat the operation if needed."),
                         chart_bulk.failed, chart_bulk.downloading),
        _("Chart Downloader"), wxOK | wxICON_ERROR);
  }

  if (m_download_cancel.IsCancelled() &&
      profile.ui.show_download_result_dialogs) {
    OCPNMessageBox_PlugIn(this, _("Chart download cancelled."),
                          _("Chart Downloader"), wxOK | wxICON_INFORMATION);
  }

  if ((chart_bulk.downloading - chart_bulk.failed > 0) &&
      !profile.charts.defer_chart_db_apply) {
    pPlugIn->ApplyChartDatabaseUpdate();
  }
}

bool ChartDldrPanelImpl::WaitForTransfer(
    const ChartDldrBulkModeProfile& profile, bool allow_ui_updates,
    const ChartDldrChartBulkState* chart_bulk) {
  // Blocking interactive waits call wxYield() here. Scheduled PollOnly runs
  // must not enter this loop; onDLEvent uses session.WouldYieldOnDownloadEvent()
  // (wxYieldIfNeeded) so PollOnly bulk steps never re-enter via download events.
  const bool use_yield =
      profile.charts.transfer_poll == ChartDldrTransferPoll::BlockUntilComplete;
  while (m_transfer.IsInProgress() && m_transfer.success &&
         !m_download_cancel.IsCancelled()) {
    if (allow_ui_updates && chart_bulk) {
      UpdateBulkDownloadProgressUi(this, chart_bulk->downloading,
                                   chart_bulk->to_download, chart_bulk->failed,
                                   m_transfer);
    } else if (allow_ui_updates) {
      Update();
      Refresh();
    }
    wxTheApp->ProcessPendingEvents();
    if (use_yield) {
      wxYield();
    }
    wxMilliSleep(20);
  }
  return m_transfer.success && !m_download_cancel.IsCancelled();
}

