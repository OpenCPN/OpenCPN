/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_bulk_catalog.h"

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_orchestrate.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"

#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/tokenzr.h>
#include <wx/url.h>

namespace ChartDldrBulkCatalogInternal {

bool PrepareDownloadPaths(int catalog_index, chartdldr_pi* pi, wxURI& url,
                          wxFileName& output_fn) {
  if (!pi || catalog_index < 0 ||
      catalog_index >= static_cast<int>(pi->m_ChartSources.size())) {
    return false;
  }

  pi->SetSourceId(catalog_index);
  std::unique_ptr<ChartSource>& cs = pi->m_ChartSources.at(catalog_index);
  url = wxURI(cs->GetUrl());
  if (url.IsReference()) {
    return false;
  }

  wxStringTokenizer tk(url.GetPath(), _T("/"));
  wxString file;
  do {
    file = tk.GetNextToken();
  } while (tk.HasMoreTokens());
  output_fn.SetFullName(file);
  output_fn.SetPath(cs->GetDir());
  if (!wxDirExists(cs->GetDir())) {
    if (!wxFileName::Mkdir(cs->GetDir(), 0755, wxPATH_MKDIR_FULL)) {
      return false;
    }
  }
  return true;
}

wxString BackgroundDownloadTargetPath(const wxFileName& output_fn) {
#ifdef __ANDROID__
  wxString file_uri = output_fn.GetFullPath();
  if (!file_uri.StartsWith(_T("file://"))) {
    file_uri = _T("file://") + output_fn.GetFullPath();
  }
  return file_uri;
#else
  return output_fn.GetFullPath();
#endif
}

void ReportPathFailure(wxWindow* parent,
                       const ChartDldrBulkModeProfile& profile,
                       const wxURI& url, const wxFileName& output_fn) {
  if (url.IsReference()) {
    const wxString msg =
        _("Error, the URL to the chart source data seems wrong.");
    ChartDldrReportBulkError(parent, profile, msg, _("Error"));
    return;
  }
  const wxString msg = wxString::Format(_("Directory %s can't be created."),
                                        output_fn.GetPath().c_str());
  ChartDldrReportBulkError(parent, profile, msg, _("Chart Downloader"));
}

}  // namespace ChartDldrBulkCatalogInternal

void ChartDldrPanelImpl::SetSourceCatalogContext(int id) {
  pPlugIn->SetSourceId(id);

  m_bDeleteSource->Enable(id >= 0);
  m_bUpdateChartList->Enable(id >= 0);
  m_bEditSource->Enable(id >= 0);

  if (id >= 0 && id < static_cast<int>(pPlugIn->m_ChartSources.size())) {
    pPlugIn->m_pChartSource = pPlugIn->m_ChartSources.at(id).get();
  } else {
    pPlugIn->m_pChartSource = nullptr;
  }
}

void ChartDldrPanelImpl::ReloadCatalogFromDisk(int catalog_index) {
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }

  std::unique_ptr<ChartSource>& cs = pPlugIn->m_ChartSources.at(catalog_index);
  pPlugIn->m_pChartSource = cs.get();
  FillFromFile(cs->GetUrl(), cs->GetDir(), pPlugIn->m_preselect_new,
               pPlugIn->m_preselect_updated);

  if (catalog_index < m_lbChartSources->GetItemCount()) {
    m_lbChartSources->SetItem(catalog_index, 0, pPlugIn->m_pChartCatalog.title);
    m_lbChartSources->SetItem(
        catalog_index, 1,
        pPlugIn->m_pChartCatalog.GetReleaseDate().Format(_T("%Y-%m-%d %H:%M")));
    m_lbChartSources->SetItem(catalog_index, 2, cs->GetDir());
  }
}

void ChartDldrPanelImpl::PrepareBulkCatalog(
    int catalog_index, const ChartDldrBulkModeProfile& profile) {
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }

  if (profile.ui.sync_list_selection &&
      catalog_index < m_lbChartSources->GetItemCount()) {
    SelectCatalog(catalog_index);
  }

  if (profile.catalog.refresh == ChartDldrCatalogRefreshMode::AsyncIdle) {
    SetSourceCatalogContext(catalog_index);
    return;
  }

  SetSource(catalog_index);
}

void ChartDldrPanelImpl::HandleCatalogDownloadResult(
    int catalog_index, _OCPN_DLStatus ret, const wxString& url,
    const ChartDldrBulkModeProfile& profile) {
  switch (ret) {
    case OCPN_DL_NO_ERROR: {
      ReloadCatalogFromDisk(catalog_index);
      if (profile.ui.focus_charts_tab_after_catalog) {
        m_DLoadNB->SetSelection(1);
      }
      break;
    }
    case OCPN_DL_FAILED: {
      const wxString msg = wxString::Format(
          _("Failed to Download Catalog: %s \nVerify there is a working "
            "Internet connection."),
          url.c_str());
      ChartDldrReportBulkError(this, profile, msg, _("Chart Downloader"));
      break;
    }
    case OCPN_DL_USER_TIMEOUT:
    case OCPN_DL_ABORTED:
      m_download_cancel.ForceCancel();
      break;
    case OCPN_DL_UNKNOWN:
    case OCPN_DL_STARTED:
      break;
    default:
      wxASSERT(false);
  }
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
  m_transfer.Begin(ChartDldrBulkTransferOwner::CatalogSync);
  const _OCPN_DLStatus started =
      OCPN_downloadFileBackground(url, file_URI, this, &m_transfer.handle);
  if (started != OCPN_DL_STARTED) {
    m_transfer.Reset();
    return OCPN_DL_FAILED;
  }
  const ChartDldrBulkModeProfile profile =
      ChartDldrInteractiveCatalogUpdateProfile();
  if (!WaitForTransfer(profile, false, nullptr)) {
    m_transfer.Reset();
    return m_download_cancel.IsCancelled() ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
  }
  m_transfer.Reset();
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

  m_transfer.Begin(ChartDldrBulkTransferOwner::CatalogSync);
  const _OCPN_DLStatus started =
      OCPN_downloadFileBackground(url, temp_path, this, &m_transfer.handle);
  if (started != OCPN_DL_STARTED) {
    wxRemoveFile(temp_path);
    m_transfer.Reset();
    return OCPN_DL_FAILED;
  }
  const ChartDldrBulkModeProfile profile =
      ChartDldrInteractiveCatalogUpdateProfile();
  if (!WaitForTransfer(profile, false, nullptr)) {
    wxRemoveFile(temp_path);
    m_transfer.Reset();
    return m_download_cancel.IsCancelled() ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
  }
  if (!wxCopyFile(temp_path, output_path)) {
    wxRemoveFile(temp_path);
    m_transfer.Reset();
    return OCPN_DL_FAILED;
  }
  wxRemoveFile(temp_path);
  m_transfer.Reset();
  return OCPN_DL_NO_ERROR;
#endif
}

void ChartDldrPanelImpl::UpdateChartListForCatalog(
    int catalog_index, wxCommandEvent& event,
    const ChartDldrBulkModeProfile& profile) {
  (void)event;

  wxURI url;
  wxFileName output_fn;
  if (!ChartDldrBulkCatalogInternal::PrepareDownloadPaths(
          catalog_index, pPlugIn, url, output_fn)) {
    ChartDldrBulkCatalogInternal::ReportPathFailure(this, profile, url,
                                                    output_fn);
    return;
  }

  const _OCPN_DLStatus ret =
      DownloadCatalogFile(url.BuildURI(), output_fn.GetFullPath(),
                          profile.catalog.show_progress_dialog);
  HandleCatalogDownloadResult(catalog_index, ret, url.BuildURI(), profile);
}

bool ChartDldrPanelImpl::BeginAsyncCatalogRefresh(
    int catalog_index, const ChartDldrBulkModeProfile& profile) {
  CancelAsyncCatalogRefresh();

  wxURI url;
  wxFileName output_fn;
  if (!ChartDldrBulkCatalogInternal::PrepareDownloadPaths(
          catalog_index, pPlugIn, url, output_fn)) {
    ChartDldrBulkCatalogInternal::ReportPathFailure(this, profile, url,
                                                    output_fn);
    return false;
  }

  EnsureDownloadHandlerConnected();
  m_asyncCatalogIndex = catalog_index;
  m_asyncCatalogProfile = profile;
  m_asyncCatalogOutputPath = output_fn.GetFullPath();

#ifdef __ANDROID__
  m_asyncCatalogTempPath =
      ChartDldrBulkCatalogInternal::BackgroundDownloadTargetPath(output_fn);
#else
  wxFileName tfn = wxFileName::CreateTempFileName(m_asyncCatalogOutputPath);
  m_asyncCatalogTempPath = tfn.GetFullPath();
#endif

  m_transfer.Begin(ChartDldrBulkTransferOwner::CatalogAsync);
  const _OCPN_DLStatus started = OCPN_downloadFileBackground(
      url.BuildURI(), m_asyncCatalogTempPath, this, &m_transfer.handle);
  if (started != OCPN_DL_STARTED) {
#ifndef __ANDROID__
    wxRemoveFile(m_asyncCatalogTempPath);
#endif
    m_asyncCatalogTempPath.clear();
    m_asyncCatalogOutputPath.clear();
    m_asyncCatalogIndex = -1;
    m_transfer.Reset();
    return false;
  }

  m_asyncCatalogRefreshActive = true;
  return true;
}

ChartDldrAsyncCatalogStepResult ChartDldrPanelImpl::StepAsyncCatalogRefresh() {
  if (!m_asyncCatalogRefreshActive) {
    return ChartDldrAsyncCatalogStepResult::NotActive;
  }

  if (m_transfer.IsInProgress()) {
    return ChartDldrAsyncCatalogStepResult::InProgress;
  }

  _OCPN_DLStatus ret = OCPN_DL_NO_ERROR;
  if (!m_transfer.success || m_download_cancel.IsCancelled()) {
    ret = m_download_cancel.IsCancelled() ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
  } else {
#ifndef __ANDROID__
    if (!wxCopyFile(m_asyncCatalogTempPath, m_asyncCatalogOutputPath)) {
      ret = OCPN_DL_FAILED;
    }
    wxRemoveFile(m_asyncCatalogTempPath);
#endif
  }

  const int catalog_index = m_asyncCatalogIndex;
  const ChartDldrBulkModeProfile profile = m_asyncCatalogProfile;
  const wxString url = pPlugIn->m_ChartSources.at(catalog_index)->GetUrl();
  m_asyncCatalogRefreshActive = false;
  m_asyncCatalogIndex = -1;
  m_asyncCatalogTempPath.clear();
  m_asyncCatalogOutputPath.clear();
  m_transfer.Reset();

  HandleCatalogDownloadResult(catalog_index, ret, url, profile);
  if (ret != OCPN_DL_NO_ERROR) {
    return ChartDldrAsyncCatalogStepResult::Failed;
  }
  return m_download_cancel.IsCancelled()
             ? ChartDldrAsyncCatalogStepResult::Failed
             : ChartDldrAsyncCatalogStepResult::Complete;
}

void ChartDldrPanelImpl::CancelAsyncCatalogRefresh() {
  if (!m_asyncCatalogRefreshActive) {
    return;
  }
  if (m_transfer.handle) {
    OCPN_cancelDownloadFileBackground(m_transfer.handle);
  }
#ifndef __ANDROID__
  if (!m_asyncCatalogTempPath.empty()) {
    wxRemoveFile(m_asyncCatalogTempPath);
  }
#endif
  m_asyncCatalogRefreshActive = false;
  m_asyncCatalogIndex = -1;
  m_asyncCatalogTempPath.clear();
  m_asyncCatalogOutputPath.clear();
  m_transfer.Reset();
}

bool ChartDldrBeginCatalogRefresh(ChartDldrPanelImpl& panel, int catalog_index,
                                  const ChartDldrBulkModeProfile& profile,
                                  const ChartDldrCatalogRefreshContext& ctx) {
  switch (ChartDldrResolveCatalogRefreshPath(profile, ctx)) {
    case ChartDldrCatalogRefreshPath::AsyncIdle:
      return panel.BeginAsyncCatalogRefresh(catalog_index, profile);
    case ChartDldrCatalogRefreshPath::SyncBlocking:
      panel.UpdateChartListForCatalog(catalog_index, *ctx.sync_event, profile);
      return true;
    case ChartDldrCatalogRefreshPath::Invalid:
      return false;
  }
  return false;
}

ChartDldrAsyncCatalogStepResult ChartDldrStepCatalogRefresh(
    ChartDldrPanelImpl& panel, const ChartDldrBulkModeProfile& profile) {
  if (profile.catalog.refresh == ChartDldrCatalogRefreshMode::AsyncIdle) {
    return panel.StepAsyncCatalogRefresh();
  }
  return ChartDldrAsyncCatalogStepResult::Complete;
}

void ChartDldrPanelImpl::UpdateChartList(wxCommandEvent& event) {
  Bulk().RefreshCatalogManual(GetSelectedCatalog(), event);
}

void ChartDldrPanelImpl::UpdateAllCharts(wxCommandEvent& event) {
  ChartDldrBulkRunStats stats;
  Bulk().RunInteractive(ChartDldrBulkRunKind::Interactive, event, stats);
}
