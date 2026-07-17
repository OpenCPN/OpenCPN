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
#include "chartdldr_catalog_prep.h"
#include "chartdldr_pi.h"
#include "chartcatalog.h"

#include "ocpn_plugin.h"

#include <wx/datetime.h>
#include <wx/intl.h>
#include <wx/timer.h>
#include <wx/utils.h>

#include <algorithm>

namespace {

void ChartDldrOnBulkTransferProgress(wxEvtHandler* listener) {
  auto* panel = dynamic_cast<ChartDldrPanelImpl*>(listener);
  if (panel && panel->Bulk().IsRunActive()) {
    panel->Bulk().OnTransferProgressTick();
  }
}

void ChartDldrOnBulkTransferEnd(wxEvtHandler* listener) {
  auto* panel = dynamic_cast<ChartDldrPanelImpl*>(listener);
  if (panel && panel->Bulk().IsRunActive()) {
    panel->Bulk().Pump().Schedule();
  }
}

}  // namespace

void ChartDldrInitPanelBulkDownloadHooks() {
  ChartDldrBulkTransferHooks hooks;
  hooks.on_progress = &ChartDldrOnBulkTransferProgress;
  hooks.on_end = &ChartDldrOnBulkTransferEnd;
  ChartDldrSetBulkTransferHooks(hooks);
}

void ChartDldrPanelImpl::ApplyBulkRunUiPolicy(
    const ChartDldrBulkSessionPolicy& policy) {
  ChartDldrDownloadCancelState& cancel = bulk_->Session().DownloadCancel();
  cancel.ResetForBulkRun();
  cancel.BeginBulkSessionCancel();
  SyncDownloadCancelButton();
  if (policy.UiSelectDownloadTab()) {
    SelectBulkDownloadTab();
  }
}

void ChartDldrPanelImpl::EndBulkSessionUi() {
  ChartDldrDownloadCancelState& cancel = bulk_->Session().DownloadCancel();
  cancel.EndActiveDownload();
  cancel.EndBulkSessionCancel();
  SyncDownloadCancelButton();
}

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

void ChartDldrPanelImpl::SelectBulkDownloadTab() { m_DLoadNB->SetSelection(1); }

void ChartDldrPanelImpl::UpdateDownloadProgress(
    int downloading, int to_download, int failed_downloads,
    const ChartDldrBulkTransferSlot& transfer) {
  if (failed_downloads) {
    SetChartInfo(wxString::Format(
        _("Downloading chart %u of %u, %u downloads failed (%s / %s)"),
        downloading, to_download, failed_downloads,
        ChartDldrFormatBytes(transfer.transferred_size),
        ChartDldrFormatBytes(transfer.total_size)));
  } else if (transfer.total_size <= 0) {
    SetChartInfo(wxString::Format(
        _("Downloading chart %u of %u (%s)"), downloading, to_download,
        ChartDldrFormatBytes(transfer.transferred_size)));
  } else {
    SetChartInfo(wxString::Format(
        _("Downloading chart %u of %u (%s / %s)"), downloading, to_download,
        ChartDldrFormatBytes(transfer.transferred_size),
        ChartDldrFormatBytes(transfer.total_size)));
  }
  Update();
  Refresh();
}

ChartDldrBulkRunPlan ChartDldrPanelImpl::BuildSelectedChartsPreflightPlan() {
  ChartDldrBulkRunPlan plan;
  chartdldr_pi* const pi = GetPlugin();
  if (!pi) {
    plan.allow_start = false;
    return plan;
  }
  std::vector<ChartDldrBulkPreflightChart> charts;
  charts.reserve(static_cast<size_t>(GetChartCount()));
  for (int i = 0; i < GetChartCount(); ++i) {
    Chart* const chart =
        static_cast<size_t>(i) < pi->m_pChartCatalog.charts.size()
            ? pi->m_pChartCatalog.charts.at(static_cast<size_t>(i)).get()
            : nullptr;
    charts.push_back(ChartDldrBulkPreflightChart{
        IsChartChecked(i), chart && chart->NeedsManualDownload(),
        chart ? chart->GetChartTitle() : wxString(),
        chart ? chart->GetManualDownloadUrl() : wxString()});
  }
  const wxString target_dir =
      pi->m_pChartSource ? pi->m_pChartSource->GetDir() : wxString();
  plan = ChartDldrSelectedChartsPreflightPlanFor(charts, target_dir);
  if (!plan.allow_start) {
    ChartDldrReportBulkError(
        this,
        ChartDldrBulkSessionPolicyFor(ChartDldrBulkRunMode::SelectedCharts,
                                      IsShownOnScreen()),
        _("No charts selected for download."), _("Chart Downloader"));
    plan.allow_start = false;
    return plan;
  }
  return plan;
}

void ChartDldrPanelImpl::ExecuteManualDownloadPlan(
    const ChartDldrBulkRunPlan& plan) {
  if (plan.manual_downloads.empty()) {
    return;
  }

  wxString body = _(
      "These charts require manual download. Open their browser pages now?\n");
  const size_t show =
      std::min<size_t>(plan.manual_downloads.size(), static_cast<size_t>(8));
  for (size_t i = 0; i < show; ++i) {
    body += wxT("\n• ");
    body += plan.manual_downloads.at(i).title;
  }
  if (plan.manual_downloads.size() > show) {
    body += wxString::Format(_("\n…and %zu more."),
                             plan.manual_downloads.size() - show);
  }
  body += _("\n\nAutomatic chart downloads will continue either way.");

  const int answer = OCPNMessageBox_PlugIn(this, body, _("Chart Downloader"),
                                           wxYES_NO | wxICON_QUESTION);
  if (answer == wxID_YES) {
    for (const ChartDldrManualDownloadAction& action : plan.manual_downloads) {
      if (!action.url.IsEmpty()) {
        wxLaunchDefaultBrowser(action.url);
      }
    }
  }
}

void ChartDldrPanelImpl::PresentBulkPostflight(
    const ChartDldrBulkPostflight& result) {
  if (result.kind == ChartDldrBulkPostflightKind::None ||
      result.message.IsEmpty()) {
    return;
  }
  SetChartInfo(result.message);
  wxLogMessage("chartdldr_pi: %s", result.message.c_str());
}

void ChartDldrPanelImpl::CancelDownload() {
  bulk_->Session().DownloadCancel().ForceCancel();
}

void ChartDldrPanelImpl::CancelBulkActivity(ChartDldrBulkCancelScope scope) {
  if (ChartDldrShouldPreserveScheduledRun(scope,
                                          bulk_->IsScheduledRunActive())) {
    return;
  }
  bulk_->TeardownBulkSession(ChartDldrTeardownReasonForCancelScope(scope));
}
