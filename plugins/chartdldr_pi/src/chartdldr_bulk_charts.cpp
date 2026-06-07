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
    const wxString& full_path) {
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
    if (pPlugIn->m_bulk_run_active) {
      if (kind == ChartDldrChartUpdateKind::New) {
        m_bulkDownloadedNew++;
      } else if (kind == ChartDldrChartUpdateKind::Updated) {
        m_bulkDownloadedUpdated++;
      }
    }
  } else {
    m_failed_downloads++;
  }
}

ChartDldrBulkRunUiSnapshot ChartDldrPanelImpl::CaptureBulkUiSnapshot() const {
  ChartDldrBulkRunUiSnapshot snapshot;
  snapshot.panel_visible = IsShownOnScreen();
  snapshot.notebook_page = m_DLoadNB->GetSelection();
  return snapshot;
}

void ChartDldrPanelImpl::BeginBulkRunSession(
    const ChartDldrBulkModeProfile& profile) {
  cancelled = false;
  m_bulkDownloadedNew = 0;
  m_bulkDownloadedUpdated = 0;
  if (profile.select_download_tab) {
    m_DLoadNB->SetSelection(1);
  }
}

void ChartDldrPanelImpl::ResetBulkCatalogCounters() {
  m_bulkDownloadedNew = 0;
  m_bulkDownloadedUpdated = 0;
}

void ChartDldrPanelImpl::RestoreBulkNotebookPage(int page) {
  m_DLoadNB->SetSelection(page);
}

void ChartDldrPanelImpl::BeginBulkChartDownload(
    const ChartDldrBulkModeProfile& profile) {
  EnsureDownloadHandlerConnected();

  if (!GetCheckedChartCount() && !profile.allow_empty_selection) {
    OCPNMessageBox_PlugIn(this, _("No charts selected for download."));
    return;
  }
  const int catalog_index = pPlugIn->GetSourceId();
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }

  cancelled = false;
  to_download = GetCheckedChartCount();
  m_downloading = 0;
  m_failed_downloads = 0;
  DisableForDownload(false);
  m_bDnldCharts->SetLabel(_("Abort download"));
  DownloadIsCancel = true;
  m_bulkChartLoopIndex = 0;
  m_bulkChartDownloadActive = true;
  m_transfer.Reset();
  m_bulkChartPendingIndex = -1;
  m_bulkChartPendingPath.clear();
}

void ChartDldrPanelImpl::FinalizeActiveBulkChartTransfer(ChartSource& source) {
  if (cancelled) {
    if (m_transfer.handle) {
      OCPN_cancelDownloadFileBackground(m_transfer.handle);
    }
    m_transfer.Reset();
    return;
  }

  if (m_transfer.success) {
    FinalizePendingChartDownload(m_bulkChartPendingIndex,
                                 m_bulkChartPendingKind, source,
                                 m_bulkChartPendingPath);
  } else {
    if (wxFileExists(m_bulkChartPendingPath)) {
      wxRemoveFile(m_bulkChartPendingPath);
    }
    m_failed_downloads++;
  }

  m_transfer.Reset();
}

ChartDldrBulkChartStepResult ChartDldrPanelImpl::StepNextBulkChart(
    const ChartDldrBulkModeProfile& profile) {
  if (!m_bulkChartDownloadActive) {
    return ChartDldrBulkChartStepResult::NotActive;
  }

  if (cancelled) {
    return ChartDldrBulkChartStepResult::Finished;
  }

  const bool allow_download_ui_updates =
      profile.show_download_progress_ui && IsShownOnScreen();
  const int catalog_index = pPlugIn->GetSourceId();
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return ChartDldrBulkChartStepResult::Finished;
  }
  std::unique_ptr<ChartSource>& cs = pPlugIn->m_ChartSources.at(catalog_index);

  if (m_transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    wxTheApp->ProcessPendingEvents();
    wxYield();

    if (m_transfer.IsInProgress() && !cancelled) {
      if (allow_download_ui_updates) {
        UpdateBulkDownloadProgressUi(this, m_downloading, to_download,
                                     m_failed_downloads, m_transfer);
      }
      return ChartDldrBulkChartStepResult::TransferInProgress;
    }

    FinalizeActiveBulkChartTransfer(*cs);
    if (cancelled) {
      return ChartDldrBulkChartStepResult::Finished;
    }

    if (m_bulkChartLoopIndex < GetChartCount() && to_download) {
      return ChartDldrBulkChartStepResult::MoreCharts;
    }
    return ChartDldrBulkChartStepResult::Finished;
  }

  for (int i = m_bulkChartLoopIndex; i < GetChartCount() && to_download; ++i) {
    m_bulkChartLoopIndex = i + 1;
    if (cancelled) {
      return ChartDldrBulkChartStepResult::Finished;
    }
    if (!isChartChecked(i)) {
      continue;
    }

    const int index = i;
    if (pPlugIn->m_pChartCatalog.charts.at(index)->NeedsManualDownload()) {
      if (profile.skip_manual_charts) {
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
      m_bulkChartDownloadActive = false;
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

    m_downloading++;
    m_bulkChartPendingIndex = index;
    m_bulkChartPendingKind = ClassifyChartForDownload(index);
    m_bulkChartPendingPath = path;

    m_transfer.Begin(ChartDldrBulkTransferOwner::ChartBulk);
    OCPN_downloadFileBackground(url.BuildURI(), file_path, this,
                                &m_transfer.handle);
    wxTheApp->ProcessPendingEvents();
    wxYield();
    return ChartDldrBulkChartStepResult::TransferInProgress;
  }

  return ChartDldrBulkChartStepResult::Finished;
}

void ChartDldrPanelImpl::EndBulkChartDownload(
    const ChartDldrBulkModeProfile& profile) {
  if (!m_bulkChartDownloadActive) {
    return;
  }

  if (m_transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk) &&
      m_transfer.handle) {
    OCPN_cancelDownloadFileBackground(m_transfer.handle);
    m_transfer.Reset();
  }

  DisableForDownload(true);
  m_bDnldCharts->SetLabel(_("Download selected charts"));
  DownloadIsCancel = false;
  m_bulkChartDownloadActive = false;

  const int restore_catalog = pPlugIn->GetSourceId();
  if (restore_catalog >= 0) {
    SetSource(restore_catalog);
  } else {
    SetSource(GetSelectedCatalog());
  }

  if (m_failed_downloads > 0 && profile.show_download_result_dialogs &&
      !cancelled) {
    OCPNMessageBox_PlugIn(
        this,
        wxString::Format(_("%d out of %d charts failed to download.\nCheck the "
                           "list, verify there is a working Internet "
                           "connection and repeat the operation if needed."),
                         m_failed_downloads, m_downloading),
        _("Chart Downloader"), wxOK | wxICON_ERROR);
  }

  if (cancelled && profile.show_download_result_dialogs) {
    OCPNMessageBox_PlugIn(this, _("Chart download cancelled."),
                          _("Chart Downloader"), wxOK | wxICON_INFORMATION);
  }

  if ((m_downloading - m_failed_downloads > 0) &&
      !profile.defer_chart_db_apply) {
    pPlugIn->ApplyChartDatabaseUpdate();
  }
}

void ChartDldrPanelImpl::DownloadCharts(
    const ChartDldrBulkModeProfile& profile) {
  BeginBulkChartDownload(profile);
  while (true) {
    const ChartDldrBulkChartStepResult step = StepNextBulkChart(profile);
    if (step == ChartDldrBulkChartStepResult::TransferInProgress ||
        step == ChartDldrBulkChartStepResult::MoreCharts) {
      continue;
    }
    break;
  }
  EndBulkChartDownload(profile);
}
