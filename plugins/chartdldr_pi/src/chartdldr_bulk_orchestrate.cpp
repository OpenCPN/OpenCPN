/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_orchestrate.h"

#include "chartdldr_bulk_catalog.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_panel_bulk.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <wx/intl.h>
#include <wx/log.h>

ChartDldrBulkOrchestrate::ChartDldrBulkOrchestrate(ChartDldrPanelImpl& panel)
    : panel_(panel) {}

bool ChartDldrBulkOrchestrate::RunInteractive(ChartDldrBulkRunKind kind,
                                              wxCommandEvent& event,
                                              ChartDldrBulkRunStats& stats) {
  if (IsRunActive()) {
    return false;
  }

  chartdldr_pi* const pi = panel_.GetPlugin();
  if (!pi) {
    return false;
  }

  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(kind, panel_.IsShownOnScreen()));
  const ChartDldrBulkRunUiSnapshot ui_before = panel_.CaptureBulkUiSnapshot();

  if (profile.ui.confirm_before_start &&
      !pi->ConfirmInteractiveBulkUpdate(&panel_)) {
    return false;
  }

  session_.Begin(pi, kind, profile, ui_before);
  panel_.ApplyRunUiPolicy(profile);
  stats = ChartDldrBulkRunStats();

  for (size_t catalog_index = 0; catalog_index < pi->m_ChartSources.size();
       ++catalog_index) {
    if (panel_.IsBulkRunCancelled()) {
      break;
    }
    ChartDldrPanelBulk::RunInteractiveCatalogPass(
        panel_, static_cast<int>(catalog_index), profile, event, stats);
  }

  FinalizeBulkRun(pi, profile, ui_before, stats);
  session_.End();
  return true;
}

bool ChartDldrBulkOrchestrate::RunSelectedChartsDownload() {
  if (IsRunActive()) {
    return false;
  }
  ChartDldrChartBulkState chart_bulk;
  ChartDldrPanelBulk::DownloadCharts(panel_,
                                     ChartDldrSelectedChartsDownloadProfile(),
                                     chart_bulk);
  return true;
}

void ChartDldrBulkOrchestrate::StartScheduledRun(chartdldr_pi* pi) {
  CancelScheduledRun();
  if (!pi) {
    return;
  }

  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled,
                                  panel_.IsShownOnScreen()));
  const ChartDldrBulkRunUiSnapshot ui_before = panel_.CaptureBulkUiSnapshot();

  session_.Begin(pi, ChartDldrBulkRunKind::Scheduled, profile, ui_before);
  panel_.ApplyRunUiPolicy(profile);

  wxLogMessage("chartdldr_pi: scheduled bulk update starting");
}

bool ChartDldrBulkOrchestrate::StepScheduledRun() {
  if (!session_.IsActive() || !session_.IsScheduled()) {
    return false;
  }

  if (panel_.IsBulkRunCancelled()) {
    FinishScheduledRun();
    return false;
  }

  chartdldr_pi* const pi = session_.Plugin();
  const size_t catalog_count = pi ? pi->m_ChartSources.size() : 0;
  if (session_.ScheduledState().next_catalog >= catalog_count) {
    FinishScheduledRun();
    return false;
  }

  const bool continue_run = StepScheduledRunCore();
  if (!continue_run) {
    FinishScheduledRun();
  }
  return continue_run;
}

void ChartDldrBulkOrchestrate::FinishScheduledRun() {
  if (!session_.IsActive() || !session_.IsScheduled()) {
    return;
  }

  chartdldr_pi* const pi = session_.Plugin();
  const ChartDldrBulkModeProfile profile = session_.Profile();
  const ChartDldrBulkRunUiSnapshot ui_before = session_.UiBefore();

  CleanupScheduledRun();

  FinalizeBulkRun(pi, profile, ui_before, session_.ScheduledState().stats);
  ChartDldrFinishScheduledBulkRun(pi, session_.ScheduledState().stats);

  if (pi) {
    pi->OnScheduledBulkRunFinished();
  }
  session_.End();
}

void ChartDldrBulkOrchestrate::CancelScheduledRun() {
  if (!session_.IsActive() || !session_.IsScheduled()) {
    return;
  }
  panel_.CancelDownload();
  FinishScheduledRun();
}

void ChartDldrBulkOrchestrate::RefreshCatalogManual(int catalog_index,
                                                    wxCommandEvent& event) {
  panel_.UpdateChartListForCatalog(
      catalog_index, event, ChartDldrInteractiveCatalogUpdateProfile());
}

void ChartDldrBulkOrchestrate::CleanupScheduledRun() {
  panel_.CancelAsyncCatalogRefresh();
  if (session_.ChartBulk().active) {
    panel_.EndBulkChartDownload(session_.Profile(), session_.ChartBulk());
  }
}

void ChartDldrBulkOrchestrate::ApplyScheduledStepDecision(
    const ChartDldrScheduledBulkStepDecision& decision) {
  const ChartDldrBulkModeProfile& profile = session_.Profile();
  ChartDldrChartBulkState& chart_bulk = session_.ChartBulk();
  if (decision.begin_chart_download) {
    panel_.BeginBulkChartDownload(profile, chart_bulk);
  }
  if (decision.end_chart_download) {
    panel_.EndBulkChartDownload(profile, chart_bulk);
  }
}

bool ChartDldrBulkOrchestrate::StepScheduledRunCore() {
  ChartDldrScheduledBulkRunState& state = session_.ScheduledState();
  const ChartDldrBulkModeProfile& profile = session_.Profile();
  chartdldr_pi* const pi = session_.Plugin();
  if (!pi || state.next_catalog >= pi->m_ChartSources.size()) {
    return false;
  }

  const int catalog_index = static_cast<int>(state.next_catalog);
  ChartDldrScheduledBulkStepInput input;
  input.phase = state.phase;
  input.next_catalog = state.next_catalog;
  input.catalog_count = pi->m_ChartSources.size();

  if (state.phase == ChartDldrScheduledBulkPhase::SelectCatalog) {
    panel_.PrepareBulkCatalog(catalog_index, profile);
    input.catalog_refresh_started = ChartDldrBeginCatalogRefresh(
        panel_, catalog_index, profile,
        ChartDldrCatalogRefreshContext::None());
  } else if (state.phase == ChartDldrScheduledBulkPhase::RefreshCatalog) {
    input.catalog_step = ChartDldrStepCatalogRefresh(panel_, profile);
  } else {
    input.chart_step =
        panel_.StepNextBulkChart(profile, session_.ChartBulk());
  }

  ChartDldrBulkRunStats catalog_counters;
  if (state.phase == ChartDldrScheduledBulkPhase::DownloadChart) {
    catalog_counters = session_.ChartBulk().ToStats();
  }

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrAdvanceScheduledBulkRun(state, input, catalog_counters);
  ApplyScheduledStepDecision(decision);
  return decision.continue_run;
}

void ChartDldrBulkOrchestrate::FinalizeBulkRun(
    chartdldr_pi* pi, const ChartDldrBulkModeProfile& profile,
    const ChartDldrBulkRunUiSnapshot& ui_before,
    const ChartDldrBulkRunStats& stats) {
  wxLogMessage(
      wxString::Format(_T("ChartDldrBulk: downloaded %d out of %d charts."),
                       stats.downloaded_ok(), stats.attempted));

  if (profile.ui.show_failure_summary && stats.failed > 0) {
    OCPNMessageBox_PlugIn(
        &panel_,
        wxString::Format(_("%d out of %d charts failed to download.\nCheck the "
                           "list, verify there is a working Internet "
                           "connection and repeat the operation if needed."),
                         stats.failed, stats.attempted),
        _("Chart Downloader"), wxOK | wxICON_ERROR);
  }

  if (stats.downloaded_ok() > 0 && pi) {
    pi->ApplyChartDatabaseUpdate();
  }

  if (ChartDldrBulkRunShouldRestoreUi(profile, ui_before)) {
    panel_.RestoreBulkNotebookPage(ui_before.notebook_page);
  }
}
