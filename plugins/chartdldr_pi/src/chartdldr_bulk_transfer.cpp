/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_bulk_transfer.h"

void ChartDldrBulkTransferSlot::Begin(ChartDldrBulkTransferOwner new_owner) {
  owner = new_owner;
  complete = false;
  success = false;
  total_size = -1;
  transferred_size = 0;
  handle = 0;
}

void ChartDldrBulkTransferSlot::Reset() {
  owner = ChartDldrBulkTransferOwner::None;
  complete = false;
  success = false;
  total_size = -1;
  transferred_size = 0;
  handle = 0;
}

bool ChartDldrBulkTransferSlot::IsOwnedBy(
    ChartDldrBulkTransferOwner expected) const {
  return owner == expected;
}

bool ChartDldrBulkTransferSlot::IsInProgress() const {
  return owner != ChartDldrBulkTransferOwner::None && !complete;
}

void ChartDldrBulkTransferSlot::OnProgress(long total, long transferred) {
  if (transferred > transferred_size) {
    total_size = total;
    transferred_size = transferred;
  }
}

void ChartDldrBulkTransferSlot::OnEnd(bool ok) {
  complete = true;
  success = ok;
}
