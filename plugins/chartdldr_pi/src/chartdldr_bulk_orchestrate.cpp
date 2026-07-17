/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_orchestrate.h"

#include "chartdldr_bulk_preflight.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_bulk_transfer.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_chart_source.h"
#include "chartdldr_panel_bulk_catalog_controller.h"
#include "chartdldr_panel_bulk_chart_controller.h"
#include "chartdldr_pi.h"
#include "chartdldr_schedule_state.h"
#include "chartcatalog.h"

#include "ocpn_plugin.h"

#include <wx/app.h>
#include <wx/intl.h>
#include <wx/log.h>

ChartDldrBulkOrchestrate::ChartDldrBulkOrchestrate(ChartDldrBulkPanelPort& port)
    : port_(port),
      pump_(*this, *port.AsEventHandler()),
      catalog_(
          std::make_unique<ChartDldrBulkCatalogController>(port, session_)),
      chart_(std::make_unique<ChartDldrBulkChartController>(port, session_)) {
  session_.SetTransferEvents(this);
}

ChartDldrBulkOrchestrate::~ChartDldrBulkOrchestrate() {
  session_.SetTransferEvents(nullptr);
}

void ChartDldrBulkOrchestrate::OnBulkTransferProgress() {
  if (session_.IsActive()) {
    OnTransferProgressTick();
  }
}

void ChartDldrBulkOrchestrate::OnBulkTransferEnd() {
  if (session_.IsActive()) {
    pump_.Schedule();
  }
}

ChartDldrBulkWalkStep ChartDldrBulkOrchestrate::StepChartDownloadPass(
    int catalog_index) {
  if (!session_.ChartBulk().active) {
    chart_->BeginBulkChartDownload(catalog_index);
  }

  return chart_->StepNextBulkChart();
}

bool ChartDldrBulkOrchestrate::TryStartBulkSession(
    chartdldr_pi* pi, const ChartDldrBulkSessionPolicy& policy,
    const ChartDldrBulkRunUiSnapshot& ui_before) {
  // A leftover scheduled session must not finish as success.
  if (policy.scheduled && IsScheduledRunActive()) {
    TeardownBulkSession(ChartDldrBulkTeardownReason::UserCancelled);
  }
  if (!pi || IsRunActive()) {
    return false;
  }

  if (policy.scheduled) {
    const wxDateTime now = wxDateTime::Now();
    if (!ChartDldrCommitScheduledAttemptStart(pi, now)) {
      wxLogWarning(
          "chartdldr_pi: scheduled bulk start aborted (schedule state not "
          "saved)");
      return false;
    }
  }

  // Begin arms the session-cancel state; the panel only re-renders.
  session_.Begin(pi, policy, ui_before, pi->m_ChartSources.size());
  port_.SyncDownloadCancelUi();
  if (policy.ui_select_download_tab) {
    port_.FocusChartsDownloadTab();
  }

  if (policy.scheduled) {
    scheduled_run_.OnStarting(static_cast<int>(pi->m_ChartSources.size()));
  }
  return true;
}

void ChartDldrBulkOrchestrate::EnsureIdleCatalogUi() {
  port_.SyncDownloadCancelUi();
  port_.RefreshCatalogToolbar();
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
  session_.CatalogRun().stats.Accumulate(chart_->FinishChartPass());

  ChartDldrBulkOrchestrate::SessionEndContext ctx;
  ctx.pi = session_.Plugin();
  ctx.policy = session_.Policy();
  ctx.ui_before = session_.UiBefore();
  ctx.stats = session_.CatalogRun().stats;
  ctx.manual_actions = session_.ManualActions();
  ctx.postflight = ChartDldrBulkPostflightFor(reason, ctx.policy, ctx.stats);

  const ChartDldrBulkSessionEnd end =
      ChartDldrBulkSessionEndFor(reason, ctx.policy.scheduled, ctx.stats);

  scheduled_run_.Reset();
  // In-flight catalog-refresh disposal must run while the session still owns
  // the transfer; session_.End() then clears all run state.
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
    FinalizeBulkRun(ctx.policy, ctx.ui_before, ctx.stats);
    // Consent came from the interactive-start confirmation; a cancelled or
    // failed run opens nothing.
    ChartDldrOpenManualDownloadUrls(ctx.manual_actions);
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
  PresentPostflight(ctx.postflight);
}

void ChartDldrBulkOrchestrate::PresentPostflight(
    const ChartDldrBulkPostflight& result) {
  if (result.kind == ChartDldrBulkPostflightKind::None ||
      result.message.IsEmpty()) {
    return;
  }
  // Render the single non-modal result: chart-info line + log.
  port_.SetChartInfo(result.message);
  wxLogMessage("chartdldr_pi: %s", result.message.c_str());
}

void ChartDldrBulkOrchestrate::RequestBulkUserCancel() {
  if (!session_.DownloadCancel().HandleDownloadButtonClick()) {
    return;
  }
  // Let the next pump turn dispose the in-flight transfer: an "Abort download"
  // discards only the current chart and the chart controller resumes the walk,
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

  while (session_.IsActive() &&
         !session_.DownloadCancel().IsSessionCancelled()) {
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
  if (!session_.IsActive() || !session_.Policy().scheduled) {
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
  if (policy.ui_show_download_progress &&
      transfer.IsOwnedBy(ChartDldrBulkTransferOwner::ChartBulk)) {
    port_.UpdateDownloadProgress(session_.ChartBulk().downloading,
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

bool ChartDldrBulkOrchestrate::BindSingleCatalog(int catalog_index) {
  chartdldr_pi* const pi = session_.Plugin();
  if (!pi || !ChartDldrBulkCatalogIndexInRange(catalog_index,
                                               pi->m_ChartSources.size())) {
    return false;
  }

  const ChartDldrBulkSessionPolicy& policy = session_.Policy();
  if (policy.walk_bind == ChartDldrBulkWalkBind::SingleDownload) {
    // SelectedCharts skips prepare: activate the catalog now and start the
    // chart pass directly.
    catalog_->ApplyCatalogUi(
        catalog_index,
        policy.CatalogApply(pi->m_preselect_new, pi->m_preselect_updated));
    session_.ChartBulk().ResetForCatalogPass();
  }
  return ChartDldrBindSingleCatalogWalk(
      session_.CatalogRun(), catalog_index,
      policy.walk_bind == ChartDldrBulkWalkBind::SinglePrepare
          ? ChartDldrBulkCatalogPhase::PrepareCatalog
          : ChartDldrBulkCatalogPhase::DownloadChart);
}

bool ChartDldrBulkOrchestrate::StepBulkRunOnce() {
  if (!session_.IsActive() || session_.DownloadCancel().IsSessionCancelled()) {
    return false;
  }

  chartdldr_pi* const pi = session_.Plugin();
  if (!pi) {
    return false;
  }
  ChartDldrBulkCatalogRunState& state = session_.CatalogRun();

  if (!ChartDldrBulkCatalogWalkContinues(state.next_catalog,
                                         state.catalog_bound)) {
    return false;
  }

  // Observer no-ops until OnStarting (scheduled); always call so the walk
  // kernel stays free of mode branches.
  const ChartDldrScheduledStepObservation before =
      scheduled_run_.BeginStep(session_);

  const int catalog_index = state.next_catalog;
  ChartDldrBulkWalkStep walk_step = ChartDldrBulkWalkStep::NotActive;
  int charts_selected = 0;
  ChartDldrBulkRunStats catalog_counters;

  if (state.phase == ChartDldrBulkCatalogPhase::PrepareCatalog) {
    walk_step = catalog_->RunBulkCatalogPrepareStep(
        catalog_index, &charts_selected, &catalog_counters);
  } else {
    walk_step = StepChartDownloadPass(catalog_index);
    if (ChartDldrBulkWalkStepNeedsPassClose(walk_step)) {
      catalog_counters = chart_->FinishChartPass();
    }
  }

  const bool continue_run = ChartDldrAdvanceBulkWalk(
      state, walk_step, charts_selected, catalog_counters);

  scheduled_run_.OnStep(before, session_, pi, walk_step, charts_selected,
                        catalog_counters);

  return continue_run;
}

bool ChartDldrBulkOrchestrate::StartBulk(ChartDldrBulkRunMode mode) {
  chartdldr_pi* const pi = port_.GetPlugin();
  if (!pi) {
    return false;
  }

  // One UI snapshot feeds both the policy compile and the session restore.
  ChartDldrBulkRunUiSnapshot ui_before;
  ui_before.panel_visible = port_.IsShownOnScreen();
  ui_before.notebook_page = port_.BulkDownloadNotebookPage();

  const ChartDldrBulkSessionPolicy policy =
      ChartDldrBulkSessionPolicyFor(mode, ui_before.panel_visible);

  ChartDldrBulkRunPlan preflight;
  if (policy.manual_plan_before_start) {
    preflight = ChartDldrBuildSelectedChartsPreflightPlan(port_, *pi);
    if (!preflight.allow_start) {
      // A manual click path: always a dialog, never the scheduled summary log.
      ChartDldrReportBulkError(
          port_.AsWindow(), ChartDldrErrorReporting::Dialog,
          _("No charts selected for download."), _("Chart Downloader"));
      return false;
    }
  }

  if (policy.confirm_before_start &&
      !pi->ConfirmInteractiveBulkUpdate(port_.AsWindow())) {
    return false;
  }
  if (policy.manual_plan_before_start) {
    ChartDldrPromptAndOpenManualDownloads(port_.AsWindow(),
                                          preflight.manual_downloads);
  }

  const bool binds_single =
      policy.walk_bind != ChartDldrBulkWalkBind::AllCatalogs;
  if (binds_single && !ChartDldrRequireValidCatalogIndex(
                          pi, port_.GetSelectedCatalog(), port_.AsWindow())) {
    return false;
  }

  if (!TryStartBulkSession(pi, policy, ui_before)) {
    return false;
  }

  port_.RefreshCatalogToolbar();

  if (binds_single && !BindSingleCatalog(port_.GetSelectedCatalog())) {
    TeardownBulkSession(ChartDldrBulkTeardownReason::FailedStart);
    return false;
  }

  // All modes are async: pump via CallAfter / transfer END / stall timer.
  // TeardownBulkSession runs when PumpBulkRun reports the walk has ended.
  pump_.Schedule();
  return true;
}

void ChartDldrBulkOrchestrate::CleanupActiveBulkRun() {
  // Chart-bulk transfers are owned by FinishChartPass. Cleanup only
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
    const ChartDldrBulkRunStats& stats) {
  // Scheduled runs log the summary once via ChartDldrFinishScheduledBulkRun.
  if (!policy.scheduled) {
    const wxString summary = ChartDldrBulkRunStatusMessage(
        stats, ChartDldrBulkRunStatusPresentation::ScheduleLog);
    wxLogMessage(wxString::Format(_T("ChartDldrBulk: %s"), summary.c_str()));
  }

  if (ChartDldrBulkRunShouldRestoreUi(policy, ui_before)) {
    port_.SetBulkDownloadNotebookPage(ui_before.notebook_page);
  }
}
