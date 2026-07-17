/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_TRANSFER_H_
#define CHARTDLDR_BULK_TRANSFER_H_

#include "chartdldr_bulk_lifecycle.h"
#include "chartdldr_temp_download.h"

#include "ocpn_plugin.h"

#include <cstdint>
#include <memory>
#include <wx/datetime.h>

class ChartDldrBulkTransferEvtSink;
class wxString;

enum class ChartDldrBulkTransferOwner {
  None,
  Catalog,
  ChartBulk,
};

enum class ChartDldrBulkTransferStuckReason {
  None,
  ProgressStalled,
  MaxDurationExceeded,
};

/** Pump/UI observer notified after a live sink applies a download event. */
class ChartDldrBulkTransferEvents {
public:
  virtual ~ChartDldrBulkTransferEvents() = default;
  virtual void OnBulkTransferProgress() = 0;
  virtual void OnBulkTransferEnd() = 0;
};

/** Tracks one active OCPN background download on the panel. */
struct ChartDldrBulkTransferSlot {
  ChartDldrBulkTransferOwner owner = ChartDldrBulkTransferOwner::None;
  bool complete = false;
  bool success = false;
  long total_size = -1;
  long transferred_size = 0;
  long handle = 0;
  /** Bumped on Begin and CancelAndReset. */
  uint64_t generation = 0;

  ChartDldrBulkTransferSlot();
  ~ChartDldrBulkTransferSlot();
  ChartDldrBulkTransferSlot(ChartDldrBulkTransferSlot&&) noexcept;
  ChartDldrBulkTransferSlot& operator=(ChartDldrBulkTransferSlot&&) noexcept;
  ChartDldrBulkTransferSlot(const ChartDldrBulkTransferSlot&) = delete;
  ChartDldrBulkTransferSlot& operator=(const ChartDldrBulkTransferSlot&) =
      delete;

  /** Advance the slot-local event fence generation. */
  uint64_t BumpGeneration();
  void Begin(ChartDldrBulkTransferOwner new_owner,
             const wxDateTime& now = wxDateTime());
  void Reset();
  bool IsOwnedBy(ChartDldrBulkTransferOwner expected) const;
  bool IsInProgress() const;
  ChartDldrBulkTransferStuckReason StuckReason(const wxDateTime& now) const;
  bool IsStuck(const wxDateTime& now) const;
  /** Updates sizes; stall clock advances only when transferred grows. */
  void OnProgress(long total, long transferred,
                  const wxDateTime& now = wxDateTime());
  void OnEnd(bool ok);

  std::unique_ptr<ChartDldrBulkTransferEvtSink> live_sink;
  /** Cancelled event target; abandoned and retained until replacement/dtor. */
  std::unique_ptr<ChartDldrBulkTransferEvtSink> abandoned_sink;

private:
  uint64_t next_generation_ = 0;
  wxDateTime begin_time_;
  wxDateTime last_progress_time_;
};

/**
 * Apply a background-download progress/end event to the active bulk slot.
 * expected_generation must match the sink's immutable generation (non-zero).
 */
void ChartDldrApplyBulkDownloadProgress(ChartDldrBulkTransferSlot& slot,
                                        long total, long transferred,
                                        const wxDateTime& now = wxDateTime(),
                                        uint64_t expected_generation = 0);
void ChartDldrApplyBulkDownloadEnd(ChartDldrBulkTransferSlot& slot, bool ok,
                                   uint64_t expected_generation = 0);

/**
 * Start an OCPN background download into the slot. The optional events listener
 * is stored on the live sink and notified after each progress/end event.
 */
bool ChartDldrTryStartBackgroundDownload(
    ChartDldrBulkTransferSlot& slot, ChartDldrBulkTransferOwner owner,
    const wxString& url, const wxString& path,
    ChartDldrBulkTransferEvents* events = nullptr);

void ChartDldrCancelAndResetBulkTransfer(ChartDldrBulkTransferSlot& slot);

/** Where a stuck-transfer check runs; drives the reaction policy. */
enum class ChartDldrStuckTransferSite {
  OrchestratorStall,
  ChartControllerPoll,
  CatalogPrepare,
};

/** Single policy for all stuck-transfer call sites. */
enum class ChartDldrStuckTransferReaction {
  None,
  /** Stall timer: schedule pump; Step/Poll disposes the transfer. */
  SchedulePump,
  AbortChartPass,
  AbortCatalogRefresh,
};

/** Log and classify a stuck transfer for the given call site. */
ChartDldrStuckTransferReaction ChartDldrReactToStuckTransfer(
    const ChartDldrBulkTransferSlot& slot, ChartDldrStuckTransferSite site,
    const wxDateTime& now = wxDateTime());

inline bool ChartDldrTransferNeedsEventDrain(
    const ChartDldrBulkTransferSlot& slot) {
  return slot.IsInProgress();
}

/**
 * Settle a completed chart temp download and clear the transfer slot.
 * abort_current_transfer maps a pending user abort/cancel onto the temp gate.
 */
_OCPN_DLStatus ChartDldrFinishBackgroundTempDownload(
    ChartDldrBulkTransferSlot& transfer, bool abort_current_transfer,
    const ChartDldrTempDownloadPaths& paths, bool transfer_success);

#ifdef UNIT_TESTS
/** Install a live sink without starting OCPN (generation fencing / abandon). */
void ChartDldrInstallLiveTransferSinkForTest(ChartDldrBulkTransferSlot& slot);
/** Deliver END to abandoned_sink when present; returns false if none. */
bool ChartDldrDispatchAbandonedTransferEndForTest(
    ChartDldrBulkTransferSlot& slot, bool ok);
#endif

#endif  // CHARTDLDR_BULK_TRANSFER_H_
