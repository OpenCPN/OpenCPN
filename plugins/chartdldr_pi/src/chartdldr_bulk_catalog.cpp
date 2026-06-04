/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_pi.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_runner.h"

#include <wx/filename.h>
#include <wx/url.h>

void ChartDldrPanelImpl::PrepareBulkCatalog(int catalog_index,
                                            bool sync_list_selection) {
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }

  if (sync_list_selection && catalog_index < m_lbChartSources->GetItemCount()) {
    SelectCatalog(catalog_index);
  }
  SetSource(catalog_index);
}

void ChartDldrPanelImpl::EnsureDownloadHandlerConnected() {
  if (!m_bconnected) {
    Connect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
    m_bconnected = true;
  }
}

bool ChartDldrPanelImpl::WaitForBackgroundDownload(bool allow_ui_updates) {
  while (!m_bTransferComplete && m_bTransferSuccess && !cancelled) {
    if (allow_ui_updates) {
      Update();
      Refresh();
    }
    wxTheApp->ProcessPendingEvents();
    wxYield();
    wxMilliSleep(20);
  }
  return m_bTransferSuccess && !cancelled;
}

_OCPN_DLStatus ChartDldrPanelImpl::DownloadCatalogFile(
    const wxString& url, const wxString& output_path,
    bool show_progress_dialog) {
  wxURI uri(url);
  const wxString message = _("Reading Headers: ") + uri.BuildURI();

#ifdef __ANDROID__
  wxString file_URI = output_path;
  if (!file_URI.StartsWith(_T("file://"))) {
    file_URI = _T("file://") + output_path;
  }
  if (show_progress_dialog) {
    return OCPN_downloadFile(
        url, file_URI, _("Downloading file"), message, wxNullBitmap, this,
        OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
            OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE |
            OCPN_DLDS_URL | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT |
            OCPN_DLDS_AUTO_CLOSE,
        10);
  }
  EnsureDownloadHandlerConnected();
  m_bTransferComplete = false;
  m_bTransferSuccess = true;
  long handle = 0;
  const _OCPN_DLStatus started =
      OCPN_downloadFileBackground(url, file_URI, this, &handle);
  if (started != OCPN_DL_STARTED) {
    return OCPN_DL_FAILED;
  }
  if (!WaitForBackgroundDownload(false)) {
    return cancelled ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
  }
  return OCPN_DL_NO_ERROR;
#else
  wxFileName tfn = wxFileName::CreateTempFileName(output_path);
  const wxString temp_path = tfn.GetFullPath();

  if (show_progress_dialog) {
    const _OCPN_DLStatus ret = OCPN_downloadFile(
        url, temp_path, _("Downloading file"), message, wxNullBitmap, this,
        OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
            OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE |
            OCPN_DLDS_URL | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT |
            OCPN_DLDS_AUTO_CLOSE,
        10);
    if (ret == OCPN_DL_NO_ERROR && wxCopyFile(temp_path, output_path)) {
      wxRemoveFile(temp_path);
      return OCPN_DL_NO_ERROR;
    }
    wxRemoveFile(temp_path);
    return ret;
  }

  EnsureDownloadHandlerConnected();
  m_bTransferComplete = false;
  m_bTransferSuccess = true;
  long handle = 0;
  const _OCPN_DLStatus started =
      OCPN_downloadFileBackground(url, temp_path, this, &handle);
  if (started != OCPN_DL_STARTED) {
    wxRemoveFile(temp_path);
    return OCPN_DL_FAILED;
  }
  if (!WaitForBackgroundDownload(false)) {
    wxRemoveFile(temp_path);
    return cancelled ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
  }
  if (!wxCopyFile(temp_path, output_path)) {
    wxRemoveFile(temp_path);
    return OCPN_DL_FAILED;
  }
  wxRemoveFile(temp_path);
  return OCPN_DL_NO_ERROR;
#endif
}

void ChartDldrPanelImpl::UpdateChartList(wxCommandEvent& event) {
  UpdateChartListForCatalog(GetSelectedCatalog(), event,
                            ChartDldrInteractiveCatalogUpdatePolicy());
}

void ChartDldrPanelImpl::UpdateChartListForCatalog(
    int catalog_index, wxCommandEvent& event,
    const ChartDldrBulkRunUiPolicy& policy) {
  (void)event;
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }

  SetSource(catalog_index);
  std::unique_ptr<ChartSource>& cs = pPlugIn->m_ChartSources.at(catalog_index);
  wxURI url(cs->GetUrl());
  if (url.IsReference()) {
    const wxString msg =
        _("Error, the URL to the chart source data seems wrong.");
    if (ChartDldrPolicyUsesLogForErrors(policy)) {
      wxLogMessage("chartdldr_pi: %s", msg.c_str());
    } else {
      OCPNMessageBox_PlugIn(this, msg, _("Error"));
    }
    return;
  }

  wxStringTokenizer tk(url.GetPath(), _T("/"));
  wxString file;
  do {
    file = tk.GetNextToken();
  } while (tk.HasMoreTokens());
  wxFileName fn;
  fn.SetFullName(file);
  fn.SetPath(cs->GetDir());
  if (!wxDirExists(cs->GetDir())) {
    if (!wxFileName::Mkdir(cs->GetDir(), 0755, wxPATH_MKDIR_FULL)) {
      const wxString msg = wxString::Format(_("Directory %s can't be created."),
                                            cs->GetDir().c_str());
      if (ChartDldrPolicyUsesLogForErrors(policy)) {
        wxLogMessage("chartdldr_pi: %s", msg.c_str());
      } else {
        OCPNMessageBox_PlugIn(this, msg, _("Chart Downloader"));
      }
      return;
    }
  }

  const bool show_progress_dialog =
      ChartDldrPolicyShowCatalogProgressDialog(policy);
  const _OCPN_DLStatus ret =
      DownloadCatalogFile(cs->GetUrl(), fn.GetFullPath(), show_progress_dialog);
  const bool bok = (ret == OCPN_DL_NO_ERROR);

  switch (ret) {
    case OCPN_DL_NO_ERROR: {
      if (bok) {
        if (catalog_index < m_lbChartSources->GetItemCount()) {
          m_lbChartSources->SetItem(catalog_index, 0,
                                    pPlugIn->m_pChartCatalog.title);
          m_lbChartSources->SetItem(
              catalog_index, 1,
              pPlugIn->m_pChartCatalog.GetReleaseDate().Format(
                  _T("%Y-%m-%d %H:%M")));
          m_lbChartSources->SetItem(catalog_index, 2, cs->GetDir());
        }

      } else {
        const wxString msg = wxString::Format(
            _("Failed to Find New Catalog: %s "), url.BuildURI().c_str());
        if (ChartDldrPolicyUsesLogForErrors(policy)) {
          wxLogMessage("chartdldr_pi: %s", msg.c_str());
        } else {
          OCPNMessageBox_PlugIn(this, msg, _("Chart Downloader"),
                                wxOK | wxICON_ERROR);
        }
      }
      break;
    }
    case OCPN_DL_FAILED: {
      const wxString msg = wxString::Format(
          _("Failed to Download Catalog: %s \nVerify there is a working "
            "Internet connection."),
          url.BuildURI().c_str());
      if (ChartDldrPolicyUsesLogForErrors(policy)) {
        wxLogMessage("chartdldr_pi: %s", msg.c_str());
      } else {
        OCPNMessageBox_PlugIn(this, msg, _("Chart Downloader"),
                              wxOK | wxICON_ERROR);
      }
      break;
    }

    case OCPN_DL_USER_TIMEOUT:
    case OCPN_DL_ABORTED: {
      cancelled = true;
      break;
    }

    case OCPN_DL_UNKNOWN:
    case OCPN_DL_STARTED: {
      break;
    }

    default:
      wxASSERT(false);
  }

  if ((ret == OCPN_DL_NO_ERROR) && bok &&
      ChartDldrPolicyFocusChartsTabAfterCatalog(policy)) {
    m_DLoadNB->SetSelection(1);
  }
}

void ChartDldrPanelImpl::UpdateAllCharts(wxCommandEvent& event) {
  ChartDldrRunBulkUpdate(pPlugIn, ChartDldrBulkRunKind::Interactive, event);
}
