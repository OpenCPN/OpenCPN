/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_orchestrate.h"

#include "chartdldr_bulk_schedule.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_panel_bulk_catalog_engine.h"
#include "chartdldr_panel_bulk_chart_engine.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"
#include "chartdldr_schedule_state.h"
#include "chartcatalog.h"

#include "ocpn_plugin.h"

#include <wx/app.h>
#include <wx/intl.h>
#include <wx/log.h>

ChartDldrBulkOrchestrate::ChartDldrBulkOrchestrate(ChartDldrPanelImpl& panel)
    : panel_(panel),
      catalog_(std::make_unique<ChartDldrPanelBulkCatalogEngine>(panel_)),
      chart_(std::make_unique<ChartDldrPanelBulkChartEngine>(panel_)) {}

ChartDldrBulkOrchestrate::~ChartDldrBulkOrchestrate() = default;

ChartDldrBulkWalkStep ChartDldrBulkOrchestrate::StepChartDownloadPass(
    int catalog_index) {
  const ChartDldrBulkSessionPolicy& policy = session_.Policy();
  ChartDldrChartBulkState& chart_bulk = session_.ChartBulk();

  if (!chart_bulk.active) {
    chart_->BeginBulkChartDownload(policy, catalog_index, chart_bulk);
  }

  return chart_->StepNextBulkChart(policy, chart_bulk);
}

bool ChartDldrBulkOrchestrate::TryStartBulkSession(
    chartdldr_pi* pi, const ChartDldrBulkSessionPolicy& policy) {
  // A leftover scheduled session must not finish as success.
  if (ChartDldrBulkRunModeIsScheduled(policy.mode) && IsScheduledRunActive()) {
    TeardownBulkSession(ChartDldrBulkTeardownReason::UserCancelled);
  }
  if (!pi || IsRunActive()) {
    return false;
  }

  const bool panel_visible = panel_.IsShownOnScreen();
  ChartDldrBulkRunUiSnapshot ui_before;
  ui_before.panel_visible = panel_visible;
  ui_before.notebook_page = panel_.BulkDownloadNotebookPage();

  if (ChartDldrBulkRunModeIsScheduled(policy.mode)) {
    const wxDateTime now = wxDateTime::Now();
    if (!ChartDldrCommitScheduledAttemptStart(pi, now)) {
      wxLogWarning(
          "chartdldr_pi: scheduled bulk start aborted (schedule state not "
          "saved)");
      return false;
    }
  }

  session_.Begin(pi, policy, ui_before, pi->m_ChartSources.size());
  panel_.ApplyBulkRunUiPolicy(policy);

  if (ChartDldrBulkRunModeIsScheduled(policy.mode)) {
    const int source_count = static_cast<int>(pi->m_ChartSources.size());
    scheduled_log_.OnStarting();
    wxLogMessage("%s", ChartDldrFormatScheduledStarting(source_count).mb_str());
  }
  return true;
}

void ChartDldrBulkOrchestrate::EnsureIdleCatalogUi() {
  panel_.EndBulkSessionUi();
  panel_.RefreshCatalogToolbar();
}

void ChartDldrBulkOrchestrate::EnsureTransferStallTimer(bool running) {
  panel_.SetTransferStallTimerRunning(running);
}

void ChartDldrBulkOrchestrate::TeardownBulkSession(
    ChartDldrBulkTeardownReason reason, ChartDldrBulkRunStats* out_stats) {
  EnsureTransferStallTimer(false);

  if (!session_.IsActive()) {
    EnsureIdleCatalogUi();
    return;
  }

  // Orchestrator owns pass close: Advance closes at catalog boundaries;
  // teardown closes a pass still open when the loop exits early.
  session_.CatalogRun().stats.Accumulate(
      chart_->CloseActiveChartPass(session_.Policy(), session_.ChartBulk()));

  ChartDldrBulkOrchestrate::SessionEndContext ctx;
  ctx.pi = session_.Plugin();
  ctx.policy = session_.Policy();
  ctx.ui_before = session_.UiBefore();
  ctx.stats = session_.CatalogRun().stats;
  ctx.was_scheduled = session_.IsScheduled();
  ctx.postflight = ChartDldrBulkPostflightFor(reason, ctx.policy, ctx.stats);

  const ChartDldrBulkSessionEnd end =
      ChartDldrBulkSessionEndFor(reason, ctx.was_scheduled, ctx.stats);

  scheduled_log_.Reset();
  session_.End();
  EnsureIdleCatalogUi();

  ApplySessionEnd(end, ctx, out_stats);
}

void ChartDldrBulkOrchestrate::ApplySessionEnd(
    const ChartDldrBulkSessionEnd& end, const SessionEndContext& ctx,
    ChartDldrBulkRunStats* out_stats) {
  if (end.ShouldCopyStats() && out_stats) {
    *out_stats = ctx.stats;
  }
  if (end.ShouldCancelPanel()) {
    panel_.CancelDownload();
  }
  CleanupActiveBulkRun();
  if (end.ShouldCancelGlobal()) {
#ifndef __ANDROID__
    OCPN_cancelDownloadFileBackground(0);
#endif
  }
  if (end.ShouldFinalizeUi()) {
    FinalizeBulkRun(ctx.policy, ctx.ui_before, ctx.stats, ctx.was_scheduled);
  }
  if (end.ShouldApplyChartDb()) {
    ApplyChartDatabaseAfterComplete(ctx.pi);
  }
  switch (end.Scheduled()) {
    case ChartDldrBulkSessionEnd::ScheduledFinish::Complete:
      if (ctx.pi) {
        ChartDldrFinishScheduledBulkRun(ctx.pi, ctx.stats);
      }
      break;
    case ChartDldrBulkSessionEnd::ScheduledFinish::Abort:
      FinishScheduledAbort(ctx.pi);
      break;
    case ChartDldrBulkSessionEnd::ScheduledFinish::None:
      break;
  }
  panel_.PresentBulkPostflight(ctx.postflight);
}

void ChartDldrBulkOrchestrate::RequestBulkUserCancel() {
  ChartDldrPanelBulkState& state = panel_.BulkState();
  if (!state.download_cancel.HandleDownloadButtonClick()) {
    return;
  }
  // Let the next pump turn dispose the in-flight transfer: an "Abort download"
  // discards only the current chart and the chart engine resumes the walk,
  // while a "Cancel update" exits the pump loop and tears the session down.
  // Both dispose the transfer (record the failure and clean the temp file)
  // rather than resetting it here and leaking that accounting.
  ScheduleBulkPump();
}

void ChartDldrBulkOrchestrate::FinishScheduledAbort(chartdldr_pi* pi) {
  if (!pi) {
    return;
  }
  ChartDldrCommitScheduledRunOutcome(pi, ChartDldrScheduledAbortedResult());
}

void ChartDldrBulkOrchestrate::ScheduleBulkPump() { panel_.ScheduleBulkPump(); }

bool ChartDldrBulkOrchestrate::PumpBulkRun() {
  if (pump_reentrant_) {
    return session_.IsActive();
  }
  pump_reentrant_ = true;

  while (session_.IsActive() && !panel_.IsBulkRunCancelled()) {
    switch (ChartDldrBulkSessionActivityFor(session_, panel_.BulkState())) {
      case ChartDldrBulkSessionActivity::WaitTransfer:
        EnsureTransferStallTimer(true);
        pump_reentrant_ = false;
        return true;
      case ChartDldrBulkSessionActivity::PrepareCatalog:
      case ChartDldrBulkSessionActivity::DownloadCharts:
        EnsureTransferStallTimer(false);
        if (!StepBulkRunOnce()) {
          pump_reentrant_ = false;
          return false;
        }
        break;
      default:
        wxFAIL_MSG("unexpected bulk session activity while active");
        pump_reentrant_ = false;
        return false;
    }
  }

  pump_reentrant_ = false;
  return false;
}

void ChartDldrBulkOrchestrate::OnTransferProgressTick() {
  if (!session_.IsActive() || !session_.IsScheduled()) {
    return;
  }
  chartdldr_pi* const pi = session_.Plugin();
  if (!pi) {
    return;
  }
  const wxString line =
      scheduled_log_.NoteStillRunning(CaptureScheduledLogSnapshot(pi));
  if (!line.empty()) {
    wxLogMessage("%s", line.mb_str());
  }
}

void ChartDldrBulkOrchestrate::OnTransferStallTick() {
  if (!session_.IsActive()) {
    EnsureTransferStallTimer(false);
    return;
  }

  ChartDldrPanelBulkState& state = panel_.BulkState();
  if (!state.transfer.IsInProgress()) {
    // Cancel clears the transfer without posting END. Resume the walk.
    EnsureTransferStallTimer(false);
    ScheduleBulkPump();
    return;
  }

  OnTransferProgressTick();

  const ChartDldrBulkSessionPolicy& policy = session_.Policy();
  if (policy.UiShowDownloadProgress() &&
      state.transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    panel_.UpdateDownloadProgress(session_.ChartBulk().downloading,
                                  session_.ChartBulk().to_download,
                                  session_.ChartBulk().failed, state.transfer);
  }

  // Do not CancelAndReset here — Step/Poll must dispose so chart failures
  // and catalog refresh outcomes stay consistent.
  if (ChartDldrReactToStuckTransfer(
          state.transfer, ChartDldrStuckTransferSite::OrchestratorStall) ==
      ChartDldrStuckTransferReaction::SchedulePump) {
    EnsureTransferStallTimer(false);
    ScheduleBulkPump();
  }
}

bool ChartDldrBulkOrchestrate::BindSelectedCatalogDownload(int catalog_index) {
  chartdldr_pi* const pi = session_.Plugin();
  if (!pi || !ChartDldrBulkCatalogIndexInRange(catalog_index,
                                               pi->m_ChartSources.size())) {
    return false;
  }

  const ChartDldrBulkSessionPolicy& policy = session_.Policy();
  catalog_->ActivateAndPrepareBulkCatalog(
      catalog_index,
      policy.CatalogApply(pi->m_preselect_new, pi->m_preselect_updated));
  session_.ChartBulk().ResetForCatalogPass();
  return ChartDldrBindCatalogWalk(session_.CatalogRun(), catalog_index);
}

ChartDldrScheduledRunLogSnapshot
ChartDldrBulkOrchestrate::CaptureScheduledLogSnapshot(chartdldr_pi* pi) const {
  ChartDldrScheduledRunLogSnapshot snap;
  const ChartDldrBulkCatalogRunState& catalog = session_.CatalogRun();
  const ChartDldrChartBulkState& charts = session_.ChartBulk();
  snap.catalog_index = catalog.next_catalog;
  snap.catalog_count = pi ? static_cast<int>(pi->m_ChartSources.size()) : 0;
  snap.phase = catalog.phase;
  snap.catalog_refresh_active = panel_.BulkState().catalog_refresh.active;
  snap.charts_downloading = charts.downloading;
  snap.charts_to_download = charts.to_download;
  snap.charts_failed = charts.failed;
  snap.pending_chart_index = charts.pending_index;
  if (pi && ChartDldrBulkCatalogIndexInRange(catalog.next_catalog,
                                             pi->m_ChartSources.size())) {
    snap.catalog_name =
        pi->m_ChartSources.at(static_cast<size_t>(catalog.next_catalog))
            ->GetName();
  }
  if (charts.pending_index >= 0 && pi &&
      charts.pending_index <
          static_cast<int>(pi->m_pChartCatalog.charts.size())) {
    snap.chart_title = pi->m_pChartCatalog.charts
                           .at(static_cast<size_t>(charts.pending_index))
                           ->GetChartTitle();
  }
  return snap;
}

bool ChartDldrBulkOrchestrate::StepBulkRunOnce() {
  if (!session_.IsActive()) {
    return false;
  }
  if (panel_.IsBulkRunCancelled()) {
    return false;
  }

  chartdldr_pi* const pi = session_.Plugin();
  if (!pi) {
    return false;
  }
  ChartDldrBulkCatalogRunState& state = session_.CatalogRun();
  const ChartDldrBulkSessionPolicy& policy = session_.Policy();

  if (!ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                         state.catalog_bound)) {
    return false;
  }

  const bool log_milestones = session_.IsScheduled();
  const int catalog_index = state.next_catalog;
  const ChartDldrBulkCatalogPhase phase_before = state.phase;
  const int charts_downloading_before = session_.ChartBulk().downloading;

  ChartDldrScheduledBulkStepLogInput step;
  if (log_milestones) {
    step.phase_before = phase_before;
    step.catalog_index_before = catalog_index;
    step.catalog_count = static_cast<int>(pi->m_ChartSources.size());
    step.charts_downloading_before = charts_downloading_before;
    if (ChartDldrBulkCatalogIndexInRange(catalog_index,
                                         pi->m_ChartSources.size())) {
      step.catalog_name =
          pi->m_ChartSources.at(static_cast<size_t>(catalog_index))->GetName();
    }
  }

  ChartDldrBulkWalkStep walk_step = ChartDldrBulkWalkStep::NotActive;
  int charts_selected = 0;
  ChartDldrBulkRunStats catalog_counters;

  if (state.phase == ChartDldrBulkCatalogPhase::PrepareCatalog) {
    walk_step = catalog_->RunBulkCatalogPrepareStep(
        state, pi, policy, catalog_index, &charts_selected, &catalog_counters);
  } else {
    walk_step = StepChartDownloadPass(catalog_index);
    if (ChartDldrBulkWalkStepNeedsPassClose(walk_step)) {
      catalog_counters =
          chart_->CloseActiveChartPass(policy, session_.ChartBulk());
    }
  }

  const bool continue_run = ChartDldrAdvanceBulkWalk(
      state, walk_step, charts_selected, catalog_counters);

  if (log_milestones) {
    const ChartDldrChartBulkState& charts = session_.ChartBulk();
    step.charts_downloading = charts.downloading;
    step.charts_to_download = charts.to_download;
    step.pending_chart_index = charts.pending_index;
    if (charts.pending_index >= 0 &&
        charts.pending_index <
            static_cast<int>(pi->m_pChartCatalog.charts.size())) {
      step.chart_title = pi->m_pChartCatalog.charts
                             .at(static_cast<size_t>(charts.pending_index))
                             ->GetChartTitle();
    }
    scheduled_log_.OnWalkStep(step, walk_step, charts_selected,
                              catalog_counters,
                              CaptureScheduledLogSnapshot(pi));
  }

  return continue_run;
}

bool ChartDldrBulkOrchestrate::StartBulk(ChartDldrBulkRunMode mode) {
  chartdldr_pi* const pi = panel_.GetPlugin();
  if (!pi) {
    return false;
  }

  // Build the run plan once at entry; SessionPolicy drives bind/confirm after.
  ChartDldrBulkRunPlan run_plan;
  if (mode == ChartDldrBulkRunMode::SelectedCharts) {
    run_plan = panel_.BuildSelectedChartsPreflightPlan();
    if (!run_plan.allow_start) {
      return false;
    }
  }

  const ChartDldrBulkSessionPolicy policy = ChartDldrBulkSessionPolicyFor(
      mode, panel_.IsShownOnScreen(), std::move(run_plan));
  if (policy.ConfirmBeforeStart() &&
      !panel_.BulkNotifier().ConfirmInteractiveStart()) {
    return false;
  }
  if (policy.BindSelectedCatalogOnly()) {
    panel_.ExecuteManualDownloadPlan(policy.plan);
  }

  if (!TryStartBulkSession(pi, policy)) {
    return false;
  }

  panel_.RefreshCatalogToolbar();

  if (policy.BindSelectedCatalogOnly()) {
    if (!BindSelectedCatalogDownload(panel_.GetSelectedCatalog())) {
      TeardownBulkSession(ChartDldrBulkTeardownReason::FailedStart);
      return false;
    }
  }

  // All modes are async: pump via CallAfter / transfer END / stall timer.
  // TeardownBulkSession runs when PumpBulkRun reports the walk has ended.
  ScheduleBulkPump();
  return true;
}

bool ChartDldrBulkOrchestrate::RefreshCatalogManual(int catalog_index) {
  if (!ChartDldrRequireValidCatalogIndex(panel_.GetPlugin(), catalog_index,
                                         panel_.AsWindow())) {
    return false;
  }
  panel_.SetActiveCatalogContext(catalog_index);
  ChartDldrCatalogUiPolicy ui = ChartDldrManualCatalogRefreshUiPolicy();
  if (chartdldr_pi* const pi = panel_.GetPlugin()) {
    ui.preselect_new = pi->m_preselect_new;
    ui.preselect_updated = pi->m_preselect_updated;
  }
  catalog_->UpdateChartListForCatalog(catalog_index, ui);
  return true;
}

void ChartDldrBulkOrchestrate::CleanupActiveBulkRun() {
  // Chart-bulk transfers are owned by CloseActiveChartPass. Cleanup only
  // cancels an in-flight catalog refresh (and its transfer ownership).
  catalog_->CancelCatalogRefresh();
}

void ChartDldrBulkOrchestrate::ApplyChartDatabaseAfterComplete(
    chartdldr_pi* pi) {
  if (pi) {
    pi->ApplyChartDatabaseUpdate();
  }
}

void ChartDldrBulkOrchestrate::FinalizeBulkRun(
    const ChartDldrBulkSessionPolicy& policy,
    const ChartDldrBulkRunUiSnapshot& ui_before,
    const ChartDldrBulkRunStats& stats, bool was_scheduled) {
  // Scheduled runs log the summary once via ChartDldrFinishScheduledBulkRun.
  if (!was_scheduled) {
    const wxString summary = ChartDldrBulkRunStatusMessage(
        stats, ChartDldrBulkRunStatusPresentation::ScheduleLog);
    wxLogMessage(wxString::Format(_T("ChartDldrBulk: %s"), summary.c_str()));
  }

  if (ChartDldrBulkRunShouldRestoreUi(policy, ui_before)) {
    panel_.SetBulkDownloadNotebookPage(ui_before.notebook_page);
  }
}
