/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_TRANSFER_H_
#define CHARTDLDR_BULK_TRANSFER_H_

enum class ChartDldrBulkTransferOwner {
  None,
  CatalogSync,
  CatalogAsync,
  ChartBulk,
};

/** Tracks one active OCPN background download on the panel. */
struct ChartDldrBulkTransferSlot {
  ChartDldrBulkTransferOwner owner = ChartDldrBulkTransferOwner::None;
  bool complete = false;
  bool success = false;
  long total_size = -1;
  long transferred_size = 0;
  long handle = 0;

  void Begin(ChartDldrBulkTransferOwner new_owner);
  void Reset();
  bool IsOwnedBy(ChartDldrBulkTransferOwner expected) const;
  bool IsInProgress() const;
  void OnProgress(long total, long transferred);
  void OnEnd(bool ok);
};

#endif  // CHARTDLDR_BULK_TRANSFER_H_
