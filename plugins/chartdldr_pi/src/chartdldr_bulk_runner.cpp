/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_runner.h"

#include "chartdldr_bulk_schedule.h"
#include "chartdldr_pi.h"

#include <wx/event.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>

bool ChartDldrRunBulkUpdate(chartdldr_pi* pi, ChartDldrBulkRunKind kind,
                            wxCommandEvent& event) {
  if (!pi) {
    return false;
  }

  ChartDldrPanelImpl* const panel = pi->GetDownloaderPanel();
  if (!panel) {
    return false;
  }

  ChartDldrBulkRunStats stats;
  if (!ChartDldrBulkRunner::Run(panel, kind, event, stats)) {
    return false;
  }

  if (ChartDldrBulkRunIsScheduled(kind)) {
    ChartDldrFinishScheduledBulkRun(pi, stats);
  }
  return true;
}

bool ChartDldrBulkRunner::ConfirmBeforeStart(ChartDldrPanelImpl* panel) {
  if (!panel) {
    return false;
  }

  wxString message;
  if (panel->pPlugIn->m_preselect_new && panel->pPlugIn->m_preselect_updated) {
    message =
        _("You have chosen to update all chart catalogs.\nThen download "
          "all new and updated charts.\nThis may take a long time.");
  } else if (panel->pPlugIn->m_preselect_new) {
    message =
        _("You have chosen to update all chart catalogs.\nThen download "
          "only new (but not updated) charts.\nThis may take a long "
          "time.");
  } else if (panel->pPlugIn->m_preselect_updated) {
    message =
        _("You have chosen to update all chart catalogs.\nThen download "
          "only updated (but not new) charts.\nThis may take a long "
          "time.");
  } else {
    return true;
  }

  wxMessageDialog mess(panel, message, _("Chart Downloader"), wxOK | wxCANCEL);
  return mess.ShowModal() != wxID_CANCEL;
}

bool ChartDldrBulkRunner::Run(ChartDldrPanelImpl* panel,
                              ChartDldrBulkRunKind kind, wxCommandEvent& event,
                              ChartDldrBulkRunStats& stats) {
  if (!panel || panel->pPlugIn->m_bulk_run_active) {
    return false;
  }

  const bool panel_visible = panel->IsShownOnScreen();
  const ChartDldrBulkRunUiPolicy ui =
      ChartDldrBulkRunUiPolicyFor(kind, panel_visible);
  ChartDldrBulkRunUiSnapshot ui_before;
  ui_before.panel_visible = panel_visible;
  ui_before.notebook_page = panel->m_DLoadNB->GetSelection();

  if (ChartDldrPolicyConfirmBeforeStart(ui) && !ConfirmBeforeStart(panel)) {
    return false;
  }

  panel->pPlugIn->m_bulk_run_active = true;
  panel->cancelled = false;
  panel->m_bulkDownloadedNew = 0;
  panel->m_bulkDownloadedUpdated = 0;
  stats = ChartDldrBulkRunStats();

  if (ChartDldrPolicySelectDownloadTab(ui)) {
    panel->m_DLoadNB->SetSelection(1);
  }

  for (size_t chart_index = 0;
       chart_index < panel->pPlugIn->m_ChartSources.size(); ++chart_index) {
    if (panel->cancelled) {
      break;
    }
    panel->PrepareBulkCatalog(static_cast<int>(chart_index),
                              ChartDldrPolicySyncListSelection(ui));
    panel->UpdateChartListForCatalog(static_cast<int>(chart_index), event, ui);
    panel->DownloadCharts(ui);
    stats.attempted += panel->m_downloading;
    stats.failed += panel->m_failed_downloads;
    stats.new_downloads += panel->m_bulkDownloadedNew;
    stats.updated_downloads += panel->m_bulkDownloadedUpdated;
    panel->m_bulkDownloadedNew = 0;
    panel->m_bulkDownloadedUpdated = 0;
  }

  wxLogMessage(wxString::Format(
      _T("ChartDldrBulkRunner: downloaded %d out of %d charts."),
      stats.downloaded_ok(), stats.attempted));

  if (ChartDldrPolicyShowFailureSummary(ui) && stats.failed > 0) {
    OCPNMessageBox_PlugIn(
        panel,
        wxString::Format(_("%d out of %d charts failed to download.\nCheck the "
                           "list, verify there is a working Internet "
                           "connection and repeat the operation if needed."),
                         stats.failed, stats.attempted),
        _("Chart Downloader"), wxOK | wxICON_ERROR);
  }

  if (stats.downloaded_ok() > 0) {
    panel->pPlugIn->ApplyChartDatabaseUpdate();
  }

  panel->pPlugIn->m_bulk_run_active = false;

  if (ChartDldrBulkRunShouldRestoreUi(ui, ui_before)) {
    panel->m_DLoadNB->SetSelection(ui_before.notebook_page);
  }
  return true;
}
