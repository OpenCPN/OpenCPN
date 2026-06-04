/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_execute.h"

#include "chartdldr_pi.h"

#include <wx/event.h>
#include <wx/msgdlg.h>

bool ChartDldrExecuteBulkUpdate(ChartDldrPanelImpl* panel,
                                ChartDldrBulkRunKind kind,
                                wxCommandEvent& event,
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

  if (ui.confirm_before_start) {
    if ((panel->pPlugIn->m_preselect_new) &&
        (panel->pPlugIn->m_preselect_updated)) {
      wxMessageDialog mess(
          panel,
          _("You have chosen to update all chart catalogs.\nThen download all "
            "new and updated charts.\nThis may take a long time."),
          _("Chart Downloader"), wxOK | wxCANCEL);
      if (mess.ShowModal() == wxID_CANCEL) {
        return false;
      }
    } else if (panel->pPlugIn->m_preselect_new) {
      wxMessageDialog mess(
          panel,
          _("You have chosen to update all chart catalogs.\nThen download only "
            "new (but not updated) charts.\nThis may take a long time."),
          _("Chart Downloader"), wxOK | wxCANCEL);
      if (mess.ShowModal() == wxID_CANCEL) {
        return false;
      }
    } else if (panel->pPlugIn->m_preselect_updated) {
      wxMessageDialog mess(
          panel,
          _("You have chosen to update all chart catalogs.\nThen download only "
            "updated (but not new) charts.\nThis may take a long time."),
          _("Chart Downloader"), wxOK | wxCANCEL);
      if (mess.ShowModal() == wxID_CANCEL) {
        return false;
      }
    }
  }

  panel->pPlugIn->m_bulk_run_active = true;
  panel->updatingAll = true;
  panel->cancelled = false;
  panel->m_bulkDownloadedNew = 0;
  panel->m_bulkDownloadedUpdated = 0;

  stats = ChartDldrBulkRunStats();

  if (ui.select_download_tab) {
    panel->m_DLoadNB->SetSelection(1);
  }

  for (size_t chart_index = 0;
       chart_index < panel->pPlugIn->m_ChartSources.size(); ++chart_index) {
    if (panel->cancelled) {
      break;
    }
    panel->PrepareBulkCatalog(static_cast<int>(chart_index),
                              ui.sync_list_selection);
    panel->UpdateChartListForCatalog(static_cast<int>(chart_index), event,
                                     ui.quiet_downloads);
    panel->DownloadCharts(ui.quiet_downloads);
    stats.attempted += panel->m_downloading;
    stats.failed += panel->m_failed_downloads;
    stats.new_downloads += panel->m_bulkDownloadedNew;
    stats.updated_downloads += panel->m_bulkDownloadedUpdated;
    panel->m_bulkDownloadedNew = 0;
    panel->m_bulkDownloadedUpdated = 0;
  }

  wxLogMessage(wxString::Format(
      _T("chartdldr_pi::RunBulkUpdate() downloaded %d out of %d charts."),
      stats.downloaded_ok(), stats.attempted));

  if (ui.show_failure_summary && stats.failed > 0) {
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

  panel->updatingAll = false;
  panel->cancelled = false;
  panel->pPlugIn->m_bulk_run_active = false;

  if (ChartDldrBulkRunShouldRestoreUi(ui, ui_before)) {
    panel->m_DLoadNB->SetSelection(ui_before.notebook_page);
  }
  return true;
}
