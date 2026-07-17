/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_NOTIFIER_H_
#define CHARTDLDR_BULK_NOTIFIER_H_

#include <wx/string.h>

/** Pre-session confirmation only; pump and teardown never display modals. */
class ChartDldrBulkNotifier {
public:
  virtual ~ChartDldrBulkNotifier() = default;

  /** Confirm before interactive Update All. Scheduled skips this. */
  virtual bool ConfirmInteractiveStart() = 0;
};

/** No prompt — used in unit tests. */
class ChartDldrNullBulkNotifier : public ChartDldrBulkNotifier {
public:
  bool ConfirmInteractiveStart() override { return true; }
};

#endif  // CHARTDLDR_BULK_NOTIFIER_H_
