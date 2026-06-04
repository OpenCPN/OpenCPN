/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_pi.h"
#include "chartdldr_bulk.h"
#include "chartdldr_chart_classify.h"

#include <wx/filename.h>
#include <wx/url.h>

namespace {

wxString FormatBytes(double bytes) {
  if (bytes <= 0) {
    return "?";
  }
  return wxString::Format(_T("%.1fMB"), bytes / 1024 / 1024);
}

wxString FormatBytes(long bytes) {
  return FormatBytes(static_cast<double>(bytes));
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

void ChartDldrPanelImpl::DownloadCharts(
    const ChartDldrBulkRunUiPolicy& policy) {
  EnsureDownloadHandlerConnected();
  const bool allow_download_ui_updates =
      ChartDldrPolicyShowDownloadProgressUi(policy) && IsShownOnScreen();

  if (!GetCheckedChartCount() && !ChartDldrPolicyAllowEmptySelection(policy)) {
    OCPNMessageBox_PlugIn(this, _("No charts selected for download."));
    return;
  }
  const int catalog_index = pPlugIn->GetSourceId();
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }
  std::unique_ptr<ChartSource>& cs = pPlugIn->m_ChartSources.at(catalog_index);

  cancelled = false;
  to_download = GetCheckedChartCount();
  m_downloading = 0;
  m_failed_downloads = 0;
  DisableForDownload(false);
  m_bDnldCharts->SetLabel(_("Abort download"));
  DownloadIsCancel = true;

  wxFileName downloaded_p;
  int idx = -1;
  ChartDldrChartUpdateKind pending_kind = ChartDldrChartUpdateKind::None;

  for (int i = 0; i < GetChartCount() && to_download; i++) {
    int index = i;
    if (cancelled) {
      break;
    }

    if (idx >= 0) {
      FinalizePendingChartDownload(idx, pending_kind, *cs,
                                   downloaded_p.GetFullPath());
      idx = -1;
      pending_kind = ChartDldrChartUpdateKind::None;
    }

    if (!isChartChecked(i)) {
      continue;
    }
    m_bTransferComplete = false;
    m_bTransferSuccess = true;
    m_totalsize = -1;
    m_transferredsize = 0;
    m_downloading++;
    if (pPlugIn->m_pChartCatalog.charts.at(index)->NeedsManualDownload()) {
      if (ChartDldrPolicySkipManualCharts(policy)) {
        wxLogMessage("chartdldr_pi: skipping manual-download chart '%s'",
                     pPlugIn->m_pChartCatalog.charts.at(index)->title.c_str());
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
      return;
    }
    wxString file =
        pPlugIn->m_pChartCatalog.charts.at(index)->GetChartFilename();
    wxFileName fn;
    fn.SetFullName(file);
    fn.SetPath(cs->GetDir());
    wxString path = fn.GetFullPath();
    if (wxFileExists(path)) wxRemoveFile(path);
    wxString title = pPlugIn->m_pChartCatalog.charts.at(index)->GetChartTitle();

#ifdef __ANDROID__
    wxString file_path = _T("file://") + fn.GetFullPath();
#else
    wxString file_path = fn.GetFullPath();
#endif

    pending_kind = ClassifyChartForDownload(index);

    long handle;
    OCPN_downloadFileBackground(url.BuildURI(), file_path, this, &handle);

    while (!m_bTransferComplete && m_bTransferSuccess && !cancelled) {
      if (allow_download_ui_updates) {
        if (m_failed_downloads) {
          SetChartInfo(wxString::Format(
              _("Downloading chart %u of %u, %u downloads failed (%s / %s)"),
              m_downloading, to_download, m_failed_downloads,
              FormatBytes(m_transferredsize), FormatBytes(m_totalsize)));
        } else {
          SetChartInfo(wxString::Format(
              _("Downloading chart %u of %u (%s / %s)"), m_downloading,
              to_download, FormatBytes(m_transferredsize),
              FormatBytes(m_totalsize)));
        }
        Update();
        Refresh();
      }
      wxTheApp->ProcessPendingEvents();
      wxYield();
      wxMilliSleep(20);
    }

    if (cancelled) {
      idx = -1;
      OCPN_cancelDownloadFileBackground(handle);
    }

    if (m_bTransferSuccess && !cancelled) {
      idx = index;
      downloaded_p = path;
    } else {
      idx = -1;
      if (wxFileExists(path)) wxRemoveFile(path);
      m_failed_downloads++;
    }
  }
  if (idx >= 0) {
    FinalizePendingChartDownload(idx, pending_kind, *cs,
                                 downloaded_p.GetFullPath());
  }
  DisableForDownload(true);
  m_bDnldCharts->SetLabel(_("Download selected charts"));
  DownloadIsCancel = false;
  const int restore_catalog = pPlugIn->GetSourceId();
  if (restore_catalog >= 0) {
    SetSource(restore_catalog);
  } else {
    SetSource(GetSelectedCatalog());
  }
  if (m_failed_downloads > 0 &&
      ChartDldrPolicyShowDownloadResultDialogs(policy) && !cancelled) {
    OCPNMessageBox_PlugIn(
        this,
        wxString::Format(_("%d out of %d charts failed to download.\nCheck the "
                           "list, verify there is a working Internet "
                           "connection and repeat the operation if needed."),
                         m_failed_downloads, m_downloading),
        _("Chart Downloader"), wxOK | wxICON_ERROR);
  }

  if (cancelled && ChartDldrPolicyShowDownloadResultDialogs(policy)) {
    OCPNMessageBox_PlugIn(this, _("Chart download cancelled."),
                          _("Chart Downloader"), wxOK | wxICON_INFORMATION);
  }

  if ((m_downloading - m_failed_downloads > 0) &&
      !ChartDldrPolicyDeferChartDbApply(policy)) {
    pPlugIn->ApplyChartDatabaseUpdate();
  }
}
