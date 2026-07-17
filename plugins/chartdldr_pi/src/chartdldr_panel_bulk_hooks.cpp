/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_impl.h"

#include "chartdldr_bulk.h"
#include "chartdldr_bulk_orchestrate.h"
#include "chartdldr_bulk_panel_ui.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <wx/intl.h>

void ChartDldrPanelImpl::OnDownloadCharts(wxCommandEvent& event) {
  if (bulk_->Session().DownloadCancel().IsDownloadButtonCancelArmed()) {
    bulk_->RequestBulkUserCancel();
    return;
  }
  (void)event;

  const int catalog_index = GetSelectedCatalog();
  if (!ChartDldrRequireValidCatalogIndex(pPlugIn, catalog_index, this)) {
    return;
  }

  bulk_->StartBulk(ChartDldrBulkRunMode::SelectedCharts);
}

void ChartDldrPanelImpl::UpdateAllCharts(wxCommandEvent& event) {
  (void)event;
  bulk_->StartBulk(ChartDldrBulkRunMode::InteractiveBulk);
}

int ChartDldrPanelImpl::BulkDownloadNotebookPage() const {
  return m_DLoadNB->GetSelection();
}

void ChartDldrPanelImpl::SetBulkDownloadNotebookPage(int page) {
  m_DLoadNB->SetSelection(page);
}

void ChartDldrPanelImpl::UpdateDownloadProgress(
    int downloading, int to_download, int failed_downloads,
    const ChartDldrBulkTransferSlot& transfer) {
  SetChartInfo(ChartDldrDownloadProgressMessage(downloading, to_download,
                                                failed_downloads, transfer));
  Update();
  Refresh();
}

void ChartDldrPanelImpl::CancelBulkActivity(ChartDldrBulkCancelScope scope) {
  if (ChartDldrShouldPreserveScheduledRun(scope,
                                          bulk_->IsScheduledRunActive())) {
    return;
  }
  bulk_->TeardownBulkSession(ChartDldrTeardownReasonForCancelScope(scope));
}
