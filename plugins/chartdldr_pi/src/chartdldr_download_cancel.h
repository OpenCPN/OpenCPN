/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_DOWNLOAD_CANCEL_H_
#define CHARTDLDR_DOWNLOAD_CANCEL_H_

#include <wx/intl.h>
#include <wx/string.h>

/** Cancel UX phase for bulk / chart downloads. */
enum class ChartDldrDownloadCancelPhase {
  Idle,
  /** Bulk session running; download button cancels the session. */
  SessionCancellable,
  /** Chart pass active; download button aborts the current download. */
  ChartDownloadActive,
};

/**
 * What the user asked the download button to do. Kept separate from the UX
 * phase so that "Abort download" (drop the in-flight chart) and "Cancel update"
 * (end the whole bulk run) can no longer be confused: a single overloaded
 * boolean previously made an abort of one chart tear down an entire Update All.
 */
enum class ChartDldrDownloadCancelAction {
  None,
  /** Discard the current chart/catalog transfer only; keep walking. */
  AbortCurrentTransfer,
  /** End the entire bulk session. */
  CancelSession,
};

/** User cancel state for chart and catalog downloads. */
struct ChartDldrDownloadCancelState {
  ChartDldrDownloadCancelPhase phase = ChartDldrDownloadCancelPhase::Idle;

  /** True only when the whole bulk session must stop (pump teardown, stop
   * walking catalogs and charts). */
  bool IsSessionCancelled() const {
    return pending_ == ChartDldrDownloadCancelAction::CancelSession;
  }

  /** True when the in-flight transfer should be discarded. A session cancel
   * implies discarding the current transfer as well. */
  bool ShouldAbortCurrentTransfer() const {
    return pending_ == ChartDldrDownloadCancelAction::AbortCurrentTransfer ||
           pending_ == ChartDldrDownloadCancelAction::CancelSession;
  }

  ChartDldrDownloadCancelAction PendingAction() const { return pending_; }

  void ResetForBulkRun() {
    phase = ChartDldrDownloadCancelPhase::Idle;
    resume_after_download_ = ChartDldrDownloadCancelPhase::Idle;
    pending_ = ChartDldrDownloadCancelAction::None;
  }

  void BeginBulkSessionCancel() {
    pending_ = ChartDldrDownloadCancelAction::None;
    phase = ChartDldrDownloadCancelPhase::SessionCancellable;
  }

  void EndBulkSessionCancel() {
    phase = ChartDldrDownloadCancelPhase::Idle;
    resume_after_download_ = ChartDldrDownloadCancelPhase::Idle;
  }

  void BeginActiveDownload() {
    // A leftover abort from the previous chart must not silently drop the next
    // one; a pending whole-session cancel, however, is preserved.
    if (pending_ != ChartDldrDownloadCancelAction::CancelSession) {
      pending_ = ChartDldrDownloadCancelAction::None;
    }
    resume_after_download_ =
        phase == ChartDldrDownloadCancelPhase::SessionCancellable
            ? ChartDldrDownloadCancelPhase::SessionCancellable
            : ChartDldrDownloadCancelPhase::Idle;
    phase = ChartDldrDownloadCancelPhase::ChartDownloadActive;
  }

  /** Leave chart download; restore the phase that was nested underneath and
   * consume any abort scoped to that chart. */
  void EndActiveDownload() {
    if (phase != ChartDldrDownloadCancelPhase::ChartDownloadActive) {
      return;
    }
    phase = resume_after_download_;
    resume_after_download_ = ChartDldrDownloadCancelPhase::Idle;
    if (pending_ == ChartDldrDownloadCancelAction::AbortCurrentTransfer) {
      pending_ = ChartDldrDownloadCancelAction::None;
    }
  }

  bool IsDownloadButtonCancelArmed() const {
    return phase != ChartDldrDownloadCancelPhase::Idle;
  }

  /** Route a button click to the action implied by the current phase. */
  bool HandleDownloadButtonClick() {
    switch (phase) {
      case ChartDldrDownloadCancelPhase::ChartDownloadActive:
        pending_ = ChartDldrDownloadCancelAction::AbortCurrentTransfer;
        return true;
      case ChartDldrDownloadCancelPhase::SessionCancellable:
        pending_ = ChartDldrDownloadCancelAction::CancelSession;
        return true;
      case ChartDldrDownloadCancelPhase::Idle:
        return false;
    }
    return false;
  }

  /** Consume a one-shot abort of the current transfer once it is disposed, so
   * the chart pass resumes with the next chart. A whole-session cancel is left
   * intact. */
  void ConsumeAbortCurrentTransfer() {
    if (pending_ == ChartDldrDownloadCancelAction::AbortCurrentTransfer) {
      pending_ = ChartDldrDownloadCancelAction::None;
    }
  }

  /** Force a full-session cancel (teardown paths that must not resume). */
  void ForceCancel() { pending_ = ChartDldrDownloadCancelAction::CancelSession; }

private:
  ChartDldrDownloadCancelPhase resume_after_download_ =
      ChartDldrDownloadCancelPhase::Idle;
  ChartDldrDownloadCancelAction pending_ =
      ChartDldrDownloadCancelAction::None;
};

inline wxString ChartDldrDownloadCancelButtonLabel(
    ChartDldrDownloadCancelPhase phase) {
  switch (phase) {
    case ChartDldrDownloadCancelPhase::ChartDownloadActive:
      return _("Abort download");
    case ChartDldrDownloadCancelPhase::SessionCancellable:
      return _("Cancel update");
    case ChartDldrDownloadCancelPhase::Idle:
      return _("Download selected charts");
  }
  return _("Download selected charts");
}

#endif  // CHARTDLDR_DOWNLOAD_CANCEL_H_
