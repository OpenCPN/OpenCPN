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

struct ChartDldrPanelBulkState;
class ChartDldrBulkTransferEvtSink;
class wxEvtHandler;
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

/** Optional UI/pump hooks while a sink owns download events. */
struct ChartDldrBulkTransferHooks {
  void (*on_progress)(wxEvtHandler* listener) = nullptr;
  void (*on_end)(wxEvtHandler* listener) = nullptr;
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

void ChartDldrSetBulkTransferHooks(const ChartDldrBulkTransferHooks& hooks);

bool ChartDldrTryStartBackgroundDownload(ChartDldrBulkTransferSlot& slot,
                                         wxEvtHandler* listener,
                                         ChartDldrBulkTransferOwner owner,
                                         const wxString& url,
                                         const wxString& path);

void ChartDldrCancelAndResetBulkTransfer(ChartDldrBulkTransferSlot& slot);

bool ChartDldrBulkTransferIsStuck(const ChartDldrBulkTransferSlot& slot);

ChartDldrBulkTransferStuckReason ChartDldrGetBulkTransferStuckReason(
    const ChartDldrBulkTransferSlot& slot,
    const wxDateTime& now = wxDateTime());

ChartDldrBulkTransferStuckReason ChartDldrLogTransferIfStuck(
    const ChartDldrBulkTransferSlot& slot,
    const wxDateTime& now = wxDateTime());

/** Where a stuck-transfer check runs; drives the reaction policy. */
enum class ChartDldrStuckTransferSite {
  OrchestratorStall,
  ChartEnginePoll,
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

ChartDldrStuckTransferReaction ChartDldrReactToStuckTransfer(
    const ChartDldrBulkTransferSlot& slot, ChartDldrStuckTransferSite site,
    const wxDateTime& now = wxDateTime());

void ChartDldrLogBulkTransferStuck(const ChartDldrBulkTransferSlot& slot,
                                   ChartDldrBulkTransferStuckReason reason);

inline bool ChartDldrTransferNeedsEventDrain(
    const ChartDldrBulkTransferSlot& slot) {
  return slot.IsInProgress();
}

_OCPN_DLStatus ChartDldrFinishBackgroundTempDownload(
    ChartDldrPanelBulkState& state, const ChartDldrTempDownloadPaths& paths,
    bool transfer_success);

#ifdef UNIT_TESTS
/** Install a live sink without starting OCPN (generation fencing / abandon). */
void ChartDldrInstallLiveTransferSinkForTest(ChartDldrBulkTransferSlot& slot,
                                             wxEvtHandler* listener = nullptr);
/** Deliver END to abandoned_sink when present; returns false if none. */
bool ChartDldrDispatchAbandonedTransferEndForTest(
    ChartDldrBulkTransferSlot& slot, bool ok);
#endif

#endif  // CHARTDLDR_BULK_TRANSFER_H_
