/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_orchestrate.h"

#include "chartdldr_bulk_schedule.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <wx/intl.h>
#include <wx/log.h>

ChartDldrBulkOrchestrate::ChartDldrBulkOrchestrate(ChartDldrPanelImpl* panel)
    : panel_(panel) {}

chartdldr_pi* ChartDldrBulkOrchestrate::Plugin() const {
  return panel_ ? panel_->GetPlugin() : nullptr;
}

bool ChartDldrBulkOrchestrate::RunInteractive(ChartDldrBulkRunKind kind,
                                              wxCommandEvent& event,
                                              ChartDldrBulkRunStats& stats) {
  if (!panel_) {
    return false;
  }

  chartdldr_pi* const pi = Plugin();
  if (!pi || pi->m_bulk_run_active) {
    return false;
  }

  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(kind, panel_->IsShownOnScreen()));
  const ChartDldrBulkRunUiSnapshot ui_before = panel_->CaptureBulkUiSnapshot();

  if (profile.confirm_before_start &&
      !pi->ConfirmInteractiveBulkUpdate(panel_)) {
    return false;
  }

  pi->m_bulk_run_active = true;
  panel_->BeginBulkRunSession(profile);
  stats = ChartDldrBulkRunStats();

  for (size_t catalog_index = 0; catalog_index < pi->m_ChartSources.size();
       ++catalog_index) {
    if (panel_->IsBulkRunCancelled()) {
      break;
    }
    RunCatalogPassInteractive(static_cast<int>(catalog_index), profile, event,
                              stats);
  }

  FinalizeBulkRun(pi, profile, ui_before, stats);
  pi->m_bulk_run_active = false;
  return true;
}

void ChartDldrBulkOrchestrate::StartScheduledRun(chartdldr_pi* pi) {
  CancelScheduledRun();
  if (!pi || !panel_) {
    return;
  }

  scheduled_profile_ = ChartDldrBulkModeProfileFor(ChartDldrBulkRunUiPolicyFor(
      ChartDldrBulkRunKind::Scheduled, panel_->IsShownOnScreen()));
  scheduled_ui_before_ = panel_->CaptureBulkUiSnapshot();

  pi->m_bulk_run_active = true;
  panel_->BeginBulkRunSession(scheduled_profile_);
  scheduled_state_ = ChartDldrScheduledBulkRunState();
  scheduled_pi_ = pi;
  scheduled_active_ = true;

  wxLogMessage("chartdldr_pi: scheduled bulk update starting");
}

bool ChartDldrBulkOrchestrate::StepScheduledRun() {
  if (!scheduled_active_ || !scheduled_pi_ || !panel_) {
    return false;
  }

  if (panel_->IsBulkRunCancelled()) {
    FinishScheduledRun();
    return false;
  }

  const size_t catalog_count = scheduled_pi_->m_ChartSources.size();
  if (scheduled_state_.next_catalog >= catalog_count) {
    FinishScheduledRun();
    return false;
  }

  const bool continue_run =
      StepScheduledRunCore(scheduled_state_, scheduled_profile_, catalog_count);

  if (!continue_run) {
    FinishScheduledRun();
  }
  return continue_run;
}

void ChartDldrBulkOrchestrate::FinishScheduledRun() {
  if (!scheduled_active_ || !scheduled_pi_) {
    return;
  }

  CleanupScheduledRun(scheduled_profile_);

  FinalizeBulkRun(scheduled_pi_, scheduled_profile_, scheduled_ui_before_,
                  scheduled_state_.stats);

  ChartDldrFinishScheduledBulkRun(scheduled_pi_, scheduled_state_.stats);

  scheduled_pi_->m_bulk_run_active = false;
  scheduled_pi_->OnScheduledBulkRunFinished();
  scheduled_active_ = false;
  scheduled_pi_ = nullptr;
}

void ChartDldrBulkOrchestrate::CancelScheduledRun() {
  if (!scheduled_active_) {
    return;
  }
  panel_->CancelDownload();
  FinishScheduledRun();
}

void ChartDldrBulkOrchestrate::RefreshCatalogManual(int catalog_index,
                                                    wxCommandEvent& event) {
  if (!panel_) {
    return;
  }
  const ChartDldrBulkModeProfile profile =
      ChartDldrInteractiveCatalogUpdateProfile();
  panel_->UpdateChartListForCatalog(catalog_index, event, profile);
}

void ChartDldrBulkOrchestrate::CleanupScheduledRun(
    const ChartDldrBulkModeProfile& profile) {
  if (!panel_) {
    return;
  }
  panel_->CancelAsyncCatalogRefresh();
  if (panel_->IsBulkChartDownloadActive()) {
    panel_->EndBulkChartDownload(profile);
  }
}

void ChartDldrBulkOrchestrate::AccumulateCatalogStats(
    ChartDldrBulkRunStats& stats) {
  if (!panel_) {
    return;
  }
  stats.Accumulate(panel_->TakeBulkCatalogStats());
  panel_->ResetBulkCatalogCounters();
}

void ChartDldrBulkOrchestrate::ApplyScheduledStepDecision(
    const ChartDldrBulkModeProfile& profile,
    const ChartDldrScheduledBulkStepDecision& decision) {
  if (!panel_) {
    return;
  }
  if (decision.begin_chart_download) {
    panel_->BeginBulkChartDownload(profile);
  }
  if (decision.end_chart_download) {
    panel_->EndBulkChartDownload(profile);
  }
  if (decision.accumulate_stats) {
    panel_->ResetBulkCatalogCounters();
  }
}

bool ChartDldrBulkOrchestrate::TryStartCatalogRefresh(
    int catalog_index, const ChartDldrBulkModeProfile& profile,
    wxCommandEvent* event) {
  if (!panel_) {
    return false;
  }
  if (profile.catalog_refresh == ChartDldrCatalogRefreshMode::AsyncIdle) {
    return panel_->BeginAsyncCatalogRefresh(catalog_index, profile);
  }
  if (event) {
    panel_->UpdateChartListForCatalog(catalog_index, *event, profile);
  }
  return true;
}

ChartDldrAsyncCatalogStepResult ChartDldrBulkOrchestrate::StepCatalogRefresh(
    const ChartDldrBulkModeProfile& profile) {
  if (!panel_) {
    return ChartDldrAsyncCatalogStepResult::NotActive;
  }
  if (profile.catalog_refresh == ChartDldrCatalogRefreshMode::AsyncIdle) {
    return panel_->StepAsyncCatalogRefresh();
  }
  return ChartDldrAsyncCatalogStepResult::Complete;
}

void ChartDldrBulkOrchestrate::RunCatalogPassInteractive(
    int catalog_index, const ChartDldrBulkModeProfile& profile,
    wxCommandEvent& event, ChartDldrBulkRunStats& stats) {
  if (!panel_) {
    return;
  }
  panel_->PrepareBulkCatalog(catalog_index, profile.sync_list_selection);
  TryStartCatalogRefresh(catalog_index, profile, &event);
  panel_->DownloadCharts(profile);
  AccumulateCatalogStats(stats);
}

bool ChartDldrBulkOrchestrate::StepScheduledRunCore(
    ChartDldrScheduledBulkRunState& state,
    const ChartDldrBulkModeProfile& profile, size_t catalog_count) {
  if (!panel_ || state.next_catalog >= catalog_count) {
    return false;
  }

  const int catalog_index = static_cast<int>(state.next_catalog);
  ChartDldrScheduledBulkStepInput input;
  input.phase = state.phase;
  input.next_catalog = state.next_catalog;
  input.catalog_count = catalog_count;

  if (state.phase == ChartDldrScheduledBulkPhase::SelectCatalog) {
    panel_->PrepareBulkCatalog(catalog_index, profile.sync_list_selection);
    input.catalog_refresh_started =
        TryStartCatalogRefresh(catalog_index, profile, nullptr);
  } else if (state.phase == ChartDldrScheduledBulkPhase::RefreshCatalog) {
    input.catalog_step = StepCatalogRefresh(profile);
  } else {
    input.chart_step = panel_->StepNextBulkChart(profile);
  }

  ChartDldrBulkRunStats catalog_counters;
  if (state.phase == ChartDldrScheduledBulkPhase::DownloadChart) {
    catalog_counters = panel_->TakeBulkCatalogStats();
  }

  const ChartDldrScheduledBulkStepDecision decision =
      ChartDldrAdvanceScheduledBulkRun(state, input, catalog_counters);
  ApplyScheduledStepDecision(profile, decision);
  return decision.continue_run;
}

void ChartDldrBulkOrchestrate::FinalizeBulkRun(
    chartdldr_pi* pi, const ChartDldrBulkModeProfile& profile,
    const ChartDldrBulkRunUiSnapshot& ui_before,
    const ChartDldrBulkRunStats& stats) {
  wxLogMessage(
      wxString::Format(_T("ChartDldrBulk: downloaded %d out of %d charts."),
                       stats.downloaded_ok(), stats.attempted));

  if (profile.show_failure_summary && stats.failed > 0 && panel_) {
    OCPNMessageBox_PlugIn(
        panel_,
        wxString::Format(_("%d out of %d charts failed to download.\nCheck the "
                           "list, verify there is a working Internet "
                           "connection and repeat the operation if needed."),
                         stats.failed, stats.attempted),
        _("Chart Downloader"), wxOK | wxICON_ERROR);
  }

  if (stats.downloaded_ok() > 0 && pi) {
    pi->ApplyChartDatabaseUpdate();
  }

  if (panel_ && ChartDldrBulkRunShouldRestoreUi(profile, ui_before)) {
    panel_->RestoreBulkNotebookPage(ui_before.notebook_page);
  }
}
