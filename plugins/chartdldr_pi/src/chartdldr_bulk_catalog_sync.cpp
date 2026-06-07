/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_internal.h"

#include <wx/filename.h>
#include <wx/url.h>

bool ChartDldrPanelImpl::WaitForBackgroundDownload(bool allow_ui_updates) {
  while (m_transfer.IsInProgress() && m_transfer.success && !cancelled) {
    if (allow_ui_updates) {
      Update();
      Refresh();
    }
    wxTheApp->ProcessPendingEvents();
    wxYield();
    wxMilliSleep(20);
  }
  return m_transfer.success && !cancelled;
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
  if (!WaitForBackgroundDownload(false)) {
    m_transfer.Reset();
    return cancelled ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
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
  if (!WaitForBackgroundDownload(false)) {
    wxRemoveFile(temp_path);
    m_transfer.Reset();
    return cancelled ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
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
  if (!ChartDldrBulkCatalogInternal::PrepareDownloadPaths(catalog_index, pPlugIn,
                                                          url, output_fn)) {
    ChartDldrBulkCatalogInternal::ReportPathFailure(this, profile, url,
                                                    output_fn);
    return;
  }

  const _OCPN_DLStatus ret = DownloadCatalogFile(
      url.BuildURI(), output_fn.GetFullPath(),
      profile.show_catalog_progress_dialog);
  HandleCatalogDownloadResult(catalog_index, ret, url.BuildURI(), profile);
}
