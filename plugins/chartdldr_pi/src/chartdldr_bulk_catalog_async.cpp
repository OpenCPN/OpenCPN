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
    wxTheApp->ProcessPendingEvents();
    wxYield();
    return ChartDldrAsyncCatalogStepResult::InProgress;
  }

  _OCPN_DLStatus ret = OCPN_DL_NO_ERROR;
  if (!m_transfer.success || cancelled) {
    ret = cancelled ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
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
  return cancelled ? ChartDldrAsyncCatalogStepResult::Failed
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
