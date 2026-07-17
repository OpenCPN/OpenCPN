/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_orchestrate.h"

#include "chartdldr_bulk_schedule.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_panel_bulk_catalog_controller.h"
#include "chartdldr_panel_bulk_chart_controller.h"
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
      pump_(*this, *panel.AsEventHandler()),
      catalog_(std::make_unique<ChartDldrPanelBulkCatalogController>(panel_,
                                                                     session_)),
      chart_(std::make_unique<ChartDldrPanelBulkChartController>(panel_,
                                                                 session_)) {}

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
    scheduled_run_.OnStarting(static_cast<int>(pi->m_ChartSources.size()));
  }
  return true;
}

void ChartDldrBulkOrchestrate::EnsureIdleCatalogUi() {
  panel_.EndBulkSessionUi();
  panel_.RefreshCatalogToolbar();
}

void ChartDldrBulkOrchestrate::TeardownBulkSession(
    ChartDldrBulkTeardownReason reason, ChartDldrBulkRunStats* out_stats) {
  pump_.SetTransferStallTimerRunning(false);

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

  scheduled_run_.Reset();
  // Cancel-panel + in-flight catalog-refresh disposal must run while the
  // session still owns transfer/cancel state; session_.End() then clears it.
  if (end.ShouldCancelPanel()) {
    panel_.CancelDownload();
  }
  CleanupActiveBulkRun();
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
  if (!session_.DownloadCancel().HandleDownloadButtonClick()) {
    return;
  }
  // Let the next pump turn dispose the in-flight transfer: an "Abort download"
  // discards only the current chart and the chart engine resumes the walk,
  // while a "Cancel update" exits the pump loop and tears the session down.
  // Both dispose the transfer (record the failure and clean the temp file)
  // rather than resetting it here and leaking that accounting.
  pump_.Schedule();
}

void ChartDldrBulkOrchestrate::FinishScheduledAbort(chartdldr_pi* pi) {
  if (!pi) {
    return;
  }
  ChartDldrCommitScheduledRunOutcome(pi, ChartDldrScheduledAbortedResult());
}

bool ChartDldrBulkOrchestrate::PumpBulkRun() {
  if (pump_reentrant_) {
    return session_.IsActive();
  }
  pump_reentrant_ = true;

  while (session_.IsActive() && !panel_.IsBulkRunCancelled()) {
    switch (ChartDldrBulkSessionActivityFor(session_)) {
      case ChartDldrBulkSessionActivity::WaitTransfer:
        pump_.SetTransferStallTimerRunning(true);
        pump_reentrant_ = false;
        return true;
      case ChartDldrBulkSessionActivity::PrepareCatalog:
      case ChartDldrBulkSessionActivity::DownloadCharts:
        pump_.SetTransferStallTimerRunning(false);
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
  scheduled_run_.OnStillRunning(session_, session_.Plugin());
}

void ChartDldrBulkOrchestrate::OnTransferStallTick() {
  if (!session_.IsActive()) {
    pump_.SetTransferStallTimerRunning(false);
    return;
  }

  ChartDldrBulkTransferSlot& transfer = session_.Transfer();
  if (!transfer.IsInProgress()) {
    // Cancel clears the transfer without posting END. Resume the walk.
    pump_.SetTransferStallTimerRunning(false);
    pump_.Schedule();
    return;
  }

  OnTransferProgressTick();

  const ChartDldrBulkSessionPolicy& policy = session_.Policy();
  if (policy.UiShowDownloadProgress() &&
      transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    panel_.UpdateDownloadProgress(session_.ChartBulk().downloading,
                                  session_.ChartBulk().to_download,
                                  session_.ChartBulk().failed, transfer);
  }

  // Do not CancelAndReset here — Step/Poll must dispose so chart failures
  // and catalog refresh outcomes stay consistent.
  if (ChartDldrReactToStuckTransfer(
          transfer, ChartDldrStuckTransferSite::OrchestratorStall) ==
      ChartDldrStuckTransferReaction::SchedulePump) {
    pump_.SetTransferStallTimerRunning(false);
    pump_.Schedule();
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

bool ChartDldrBulkOrchestrate::BindCatalogPrepareOnly(int catalog_index) {
  chartdldr_pi* const pi = session_.Plugin();
  if (!pi || !ChartDldrBulkCatalogIndexInRange(catalog_index,
                                               pi->m_ChartSources.size())) {
    return false;
  }
  return ChartDldrBindCatalogPrepareWalk(session_.CatalogRun(), catalog_index);
}

bool ChartDldrBulkOrchestrate::StepBulkRunOnce() {
  if (!session_.IsActive() || panel_.IsBulkRunCancelled()) {
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

  // Scheduled runs snapshot the pre-step state so the observer can detect
  // milestone edges; interactive runs skip it entirely.
  const bool scheduled = session_.IsScheduled();
  const ChartDldrScheduledStepObservation before =
      scheduled ? scheduled_run_.BeginStep(session_)
                : ChartDldrScheduledStepObservation();

  const int catalog_index = state.next_catalog;
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

  if (scheduled) {
    scheduled_run_.OnStep(before, session_, pi, walk_step, charts_selected,
                          catalog_counters);
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
  pump_.Schedule();
  return true;
}

bool ChartDldrBulkOrchestrate::RefreshCatalogManual(int catalog_index) {
  if (!ChartDldrRequireValidCatalogIndex(panel_.GetPlugin(), catalog_index,
                                         panel_.AsWindow())) {
    return false;
  }

  chartdldr_pi* const pi = panel_.GetPlugin();
  if (!pi) {
    return false;
  }

  const ChartDldrBulkSessionPolicy policy = ChartDldrBulkSessionPolicyFor(
      ChartDldrBulkRunMode::CatalogRefresh, panel_.IsShownOnScreen());
  if (!TryStartBulkSession(pi, policy)) {
    return false;
  }

  panel_.RefreshCatalogToolbar();

  if (!BindCatalogPrepareOnly(catalog_index)) {
    TeardownBulkSession(ChartDldrBulkTeardownReason::FailedStart);
    return false;
  }

  pump_.Schedule();
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
