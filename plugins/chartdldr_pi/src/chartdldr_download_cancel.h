/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_DOWNLOAD_CANCEL_H_
#define CHARTDLDR_DOWNLOAD_CANCEL_H_

/** User cancel / abort-button state for chart and catalog downloads. */
struct ChartDldrDownloadCancelState {
  bool cancel_requested = true;
  bool abort_button_armed = false;

  bool IsCancelled() const { return cancel_requested; }

  void ResetForBulkRun() { cancel_requested = false; }

  void BeginActiveDownload() {
    cancel_requested = false;
    abort_button_armed = true;
  }

  void EndActiveDownload() { abort_button_armed = false; }

  bool HandleAbortButtonClick() {
    if (!abort_button_armed) {
      return false;
    }
    cancel_requested = true;
    return true;
  }

  void ForceCancel() { cancel_requested = true; }
};

#endif  // CHARTDLDR_DOWNLOAD_CANCEL_H_
