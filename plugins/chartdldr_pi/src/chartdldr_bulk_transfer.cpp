/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_transfer.h"

#include "ocpn_plugin.h"

#include "chartdldr_temp_download.h"

#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/log.h>

namespace {

constexpr int kTransferMaxMinutes = 120;
constexpr int kTransferStallMinutes = 15;

wxDateTime ChartDldrTransferClock(const wxDateTime& now) {
  return now.IsValid() ? now : wxDateTime::Now();
}

}  // namespace

/** Per-transfer evt handler: immutable generation fences late END after cancel.
 */
class ChartDldrBulkTransferEvtSink : public wxEvtHandler {
public:
  ChartDldrBulkTransferEvtSink(ChartDldrBulkTransferSlot* slot,
                               uint64_t generation,
                               ChartDldrBulkTransferEvents* events)
      : slot_(slot), generation_(generation), events_(events) {
    // OCPN_downloadEvent is not Bind-friendly (wx event-type trait).
    Connect(wxEVT_DOWNLOAD_EVENT, DownloadEventHandler());
  }

  ~ChartDldrBulkTransferEvtSink() override {
    Disconnect(wxEVT_DOWNLOAD_EVENT, DownloadEventHandler());
  }

  void Abandon() { slot_ = nullptr; }

private:
  // OCPN_downloadEvent is not Bind-friendly; funnel the one required cast here.
  static wxObjectEventFunction DownloadEventHandler() {
    return (
        wxObjectEventFunction)(wxEventFunction)&ChartDldrBulkTransferEvtSink::
        OnDownloadEvent;
  }

  void OnDownloadEvent(OCPN_downloadEvent& ev) {
    if (!slot_) {
      return;
    }
    switch (ev.getDLEventCondition()) {
      case OCPN_DL_EVENT_TYPE_END:
        ChartDldrApplyBulkDownloadEnd(
            *slot_, ev.getDLEventStatus() == OCPN_DL_NO_ERROR, generation_);
        if (events_) {
          events_->OnBulkTransferEnd();
        }
        break;
      case OCPN_DL_EVENT_TYPE_PROGRESS:
        ChartDldrApplyBulkDownloadProgress(*slot_, ev.getTotal(),
                                           ev.getTransferred(), wxDateTime(),
                                           generation_);
        if (events_) {
          events_->OnBulkTransferProgress();
        }
        break;
      default:
        break;
    }
  }

  ChartDldrBulkTransferSlot* slot_;
  const uint64_t generation_;
  ChartDldrBulkTransferEvents* const events_;
};

ChartDldrBulkTransferSlot::ChartDldrBulkTransferSlot() = default;
ChartDldrBulkTransferSlot::~ChartDldrBulkTransferSlot() = default;
ChartDldrBulkTransferSlot::ChartDldrBulkTransferSlot(
    ChartDldrBulkTransferSlot&&) noexcept = default;
ChartDldrBulkTransferSlot& ChartDldrBulkTransferSlot::operator=(
    ChartDldrBulkTransferSlot&&) noexcept = default;

uint64_t ChartDldrBulkTransferSlot::BumpGeneration() {
  ++next_generation_;
  if (next_generation_ == 0) {
    ++next_generation_;
  }
  generation = next_generation_;
  return generation;
}

void ChartDldrBulkTransferSlot::Begin(ChartDldrBulkTransferOwner new_owner,
                                      const wxDateTime& now) {
  owner = new_owner;
  complete = false;
  success = false;
  total_size = -1;
  transferred_size = 0;
  handle = 0;
  BumpGeneration();
  begin_time_ = ChartDldrTransferClock(now);
  last_progress_time_ = begin_time_;
}

void ChartDldrBulkTransferSlot::Reset() {
  owner = ChartDldrBulkTransferOwner::None;
  complete = false;
  success = false;
  total_size = -1;
  transferred_size = 0;
  handle = 0;
  begin_time_ = wxDateTime();
  last_progress_time_ = wxDateTime();
  // Keep generation and sink until the caller settles or cancels the transfer.
}

bool ChartDldrBulkTransferSlot::IsOwnedBy(
    ChartDldrBulkTransferOwner expected) const {
  return owner == expected;
}

bool ChartDldrBulkTransferSlot::IsInProgress() const {
  return owner != ChartDldrBulkTransferOwner::None && !complete;
}

bool ChartDldrBulkTransferSlot::IsStuck(const wxDateTime& now) const {
  return StuckReason(now) != ChartDldrBulkTransferStuckReason::None;
}

ChartDldrBulkTransferStuckReason ChartDldrBulkTransferSlot::StuckReason(
    const wxDateTime& now) const {
  if (!IsInProgress() || !begin_time_.IsValid()) {
    return ChartDldrBulkTransferStuckReason::None;
  }

  const wxTimeSpan elapsed = now - begin_time_;
  if (elapsed.GetMinutes() >= kTransferMaxMinutes) {
    return ChartDldrBulkTransferStuckReason::MaxDurationExceeded;
  }

  if (last_progress_time_.IsValid()) {
    const wxTimeSpan stalled = now - last_progress_time_;
    if (stalled.GetMinutes() >= kTransferStallMinutes) {
      return ChartDldrBulkTransferStuckReason::ProgressStalled;
    }
  }

  return ChartDldrBulkTransferStuckReason::None;
}

void ChartDldrBulkTransferSlot::OnProgress(long total, long transferred,
                                           const wxDateTime& now) {
  if (transferred > transferred_size) {
    transferred_size = transferred;
    last_progress_time_ = ChartDldrTransferClock(now);
  }
  if (total >= 0) {
    total_size = total;
  }
}

void ChartDldrBulkTransferSlot::OnEnd(bool ok) {
  complete = true;
  success = ok;
}

void ChartDldrApplyBulkDownloadProgress(ChartDldrBulkTransferSlot& slot,
                                        long total, long transferred,
                                        const wxDateTime& now,
                                        uint64_t expected_generation) {
  if (slot.owner == ChartDldrBulkTransferOwner::None) {
    return;
  }
  if (expected_generation != 0 && expected_generation != slot.generation) {
    return;
  }
  slot.OnProgress(total, transferred, now);
}

void ChartDldrApplyBulkDownloadEnd(ChartDldrBulkTransferSlot& slot, bool ok,
                                   uint64_t expected_generation) {
  if (slot.owner == ChartDldrBulkTransferOwner::None) {
    return;
  }
  if (expected_generation != 0 && expected_generation != slot.generation) {
    return;
  }
  slot.OnEnd(ok);
  // Live sink is replaced on next Begin/Cancel; keep until then.
}

bool ChartDldrTryStartBackgroundDownload(ChartDldrBulkTransferSlot& slot,
                                         ChartDldrBulkTransferOwner owner,
                                         const wxString& url,
                                         const wxString& path,
                                         ChartDldrBulkTransferEvents* events) {
  if (slot.IsInProgress()) {
    return false;
  }
  slot.abandoned_sink.reset();
  slot.live_sink.reset();
  slot.Begin(owner);
  slot.live_sink = std::make_unique<ChartDldrBulkTransferEvtSink>(
      &slot, slot.generation, events);
  const _OCPN_DLStatus started = OCPN_downloadFileBackground(
      url, path, slot.live_sink.get(), &slot.handle);
  if (started != OCPN_DL_STARTED) {
    slot.live_sink.reset();
    slot.Reset();
    return false;
  }
  return true;
}

namespace {

wxString ChartDldrBulkTransferOwnerLabel(ChartDldrBulkTransferOwner owner) {
  switch (owner) {
    case ChartDldrBulkTransferOwner::Catalog:
      return wxT("catalog");
    case ChartDldrBulkTransferOwner::ChartBulk:
      return wxT("chart bulk");
    case ChartDldrBulkTransferOwner::None:
      break;
  }
  return wxT("unknown");
}

wxString ChartDldrBulkTransferStuckReasonLabel(
    ChartDldrBulkTransferStuckReason reason) {
  switch (reason) {
    case ChartDldrBulkTransferStuckReason::ProgressStalled:
      return wxT("no progress");
    case ChartDldrBulkTransferStuckReason::MaxDurationExceeded:
      return wxT("max duration");
    case ChartDldrBulkTransferStuckReason::None:
      break;
  }
  return wxT("unknown");
}

void LogStuckTransfer(const ChartDldrBulkTransferSlot& slot,
                      ChartDldrBulkTransferStuckReason reason) {
  if (reason == ChartDldrBulkTransferStuckReason::None) {
    return;
  }
  wxLogWarning(wxT("chartdldr_pi: aborted stuck %s transfer (%s, transferred "
                   "%ld bytes)"),
               ChartDldrBulkTransferOwnerLabel(slot.owner).c_str(),
               ChartDldrBulkTransferStuckReasonLabel(reason).c_str(),
               slot.transferred_size);
}

}  // namespace

ChartDldrStuckTransferReaction ChartDldrReactToStuckTransfer(
    const ChartDldrBulkTransferSlot& slot, ChartDldrStuckTransferSite site,
    const wxDateTime& now) {
  const ChartDldrBulkTransferStuckReason reason =
      slot.StuckReason(now.IsValid() ? now : wxDateTime::Now());
  if (reason == ChartDldrBulkTransferStuckReason::None) {
    return ChartDldrStuckTransferReaction::None;
  }
  LogStuckTransfer(slot, reason);
  switch (site) {
    case ChartDldrStuckTransferSite::OrchestratorStall:
      return ChartDldrStuckTransferReaction::SchedulePump;
    case ChartDldrStuckTransferSite::ChartControllerPoll:
      return ChartDldrStuckTransferReaction::AbortChartPass;
    case ChartDldrStuckTransferSite::CatalogPrepare:
      return ChartDldrStuckTransferReaction::AbortCatalogRefresh;
  }
  return ChartDldrStuckTransferReaction::None;
}

void ChartDldrCancelAndResetBulkTransfer(ChartDldrBulkTransferSlot& slot) {
  const bool was_in_progress = slot.IsInProgress();
  if (was_in_progress) {
#if defined(__ANDROID__)
    if (slot.handle) {
      OCPN_cancelDownloadFileBackground(slot.handle);
    }
#else
    // Desktop curl cancel aborts the active thread; handle is often left unset.
    OCPN_cancelDownloadFileBackground(slot.handle);
#endif
    slot.abandoned_sink.reset();
    if (slot.live_sink) {
      slot.live_sink->Abandon();
      slot.abandoned_sink = std::move(slot.live_sink);
    }
  } else {
    slot.live_sink.reset();
  }
  slot.Reset();
  // Bump generation so the slot's live generation never equals a prior one.
  slot.BumpGeneration();
}

_OCPN_DLStatus ChartDldrFinishBackgroundTempDownload(
    ChartDldrBulkTransferSlot& transfer, bool abort_current_transfer,
    const ChartDldrTempDownloadPaths& paths, bool transfer_success) {
  const _OCPN_DLStatus status = ChartDldrCompleteTempDownloadPaths(
      paths, transfer_success, abort_current_transfer);
  if (status == OCPN_DL_NO_ERROR) {
    transfer.Reset();
    transfer.live_sink.reset();
    transfer.abandoned_sink.reset();
  } else if (transfer.IsInProgress()) {
    ChartDldrCancelAndResetBulkTransfer(transfer);
  } else {
    transfer.Reset();
    transfer.live_sink.reset();
    transfer.abandoned_sink.reset();
  }
  return status;
}

#ifdef UNIT_TESTS
void ChartDldrInstallLiveTransferSinkForTest(ChartDldrBulkTransferSlot& slot) {
  slot.abandoned_sink.reset();
  slot.live_sink.reset();
  if (!slot.IsInProgress()) {
    slot.Begin(ChartDldrBulkTransferOwner::ChartBulk);
  }
  slot.live_sink = std::make_unique<ChartDldrBulkTransferEvtSink>(
      &slot, slot.generation, nullptr);
}

bool ChartDldrDispatchAbandonedTransferEndForTest(
    ChartDldrBulkTransferSlot& slot, bool ok) {
  if (!slot.abandoned_sink) {
    return false;
  }
  OCPN_downloadEvent ev(wxEVT_DOWNLOAD_EVENT, 0);
  ev.setDLEventCondition(OCPN_DL_EVENT_TYPE_END);
  ev.setDLEventStatus(ok ? OCPN_DL_NO_ERROR : OCPN_DL_FAILED);
  slot.abandoned_sink->ProcessEvent(ev);
  return true;
}
#endif
