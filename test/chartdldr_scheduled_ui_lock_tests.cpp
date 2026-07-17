/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "chartdldr_bulk.h"
#include "chartdldr_download_cancel.h"

TEST(ChartDldrScheduledUiLock, DownloadButtonUnderRunLockRequiresCancelArmed) {
  EXPECT_FALSE(ChartDldrCatalogControlsFor(/*run_active=*/true,
                                           /*cancel_armed=*/false)
                   .download_button_enabled);
  EXPECT_TRUE(ChartDldrCatalogControlsFor(/*run_active=*/true,
                                          /*cancel_armed=*/true)
                  .download_button_enabled);
}

TEST(ChartDldrScheduledUiLock, DownloadButtonEnabledWhenIdle) {
  EXPECT_TRUE(ChartDldrCatalogControlsFor(/*run_active=*/false,
                                          /*cancel_armed=*/false)
                  .download_button_enabled);
}

TEST(ChartDldrScheduledUiLock, SessionCancelArmsDownloadButtonUnderLock) {
  ChartDldrDownloadCancelState cancel;
  cancel.ResetForBulkRun();
  cancel.BeginBulkSessionCancel();
  EXPECT_EQ(cancel.phase, ChartDldrDownloadCancelPhase::SessionCancellable);
  EXPECT_TRUE(ChartDldrCatalogControlsFor(
                  /*run_active=*/true, cancel.IsDownloadButtonCancelArmed())
                  .download_button_enabled);
}

TEST(ChartDldrScheduledUiLock, ChartAbortArmsDownloadButtonUnderLock) {
  ChartDldrDownloadCancelState cancel;
  cancel.ResetForBulkRun();
  cancel.BeginBulkSessionCancel();
  cancel.BeginActiveDownload();
  EXPECT_EQ(cancel.phase, ChartDldrDownloadCancelPhase::ChartDownloadActive);
  EXPECT_TRUE(ChartDldrCatalogControlsFor(
                  /*run_active=*/true, cancel.IsDownloadButtonCancelArmed())
                  .download_button_enabled);
}

TEST(ChartDldrScheduledUiLock, EndActiveDownloadRestoresSessionCancel) {
  ChartDldrDownloadCancelState cancel;
  cancel.ResetForBulkRun();
  cancel.BeginBulkSessionCancel();
  cancel.BeginActiveDownload();
  cancel.EndActiveDownload();
  EXPECT_EQ(cancel.phase, ChartDldrDownloadCancelPhase::SessionCancellable);
  EXPECT_EQ(ChartDldrDownloadCancelButtonLabel(cancel.phase),
            wxString(_("Cancel update")));
}

TEST(ChartDldrScheduledUiLock, DownloadButtonClickHonorsSessionCancel) {
  ChartDldrDownloadCancelState cancel;
  cancel.ResetForBulkRun();
  cancel.BeginBulkSessionCancel();
  EXPECT_TRUE(cancel.HandleDownloadButtonClick());
  EXPECT_TRUE(cancel.IsSessionCancelled());
  EXPECT_TRUE(cancel.ShouldAbortCurrentTransfer());
}

TEST(ChartDldrScheduledUiLock, AbortDownloadDoesNotCancelSession) {
  ChartDldrDownloadCancelState cancel;
  cancel.ResetForBulkRun();
  cancel.BeginBulkSessionCancel();
  cancel.BeginActiveDownload();
  ASSERT_EQ(cancel.phase, ChartDldrDownloadCancelPhase::ChartDownloadActive);

  // "Abort download" targets only the in-flight chart transfer.
  EXPECT_TRUE(cancel.HandleDownloadButtonClick());
  EXPECT_TRUE(cancel.ShouldAbortCurrentTransfer());
  EXPECT_FALSE(cancel.IsSessionCancelled());

  // Once the current transfer is disposed the abort is consumed and the walk
  // resumes; the session keeps running.
  cancel.ConsumeAbortCurrentTransfer();
  EXPECT_FALSE(cancel.ShouldAbortCurrentTransfer());
  EXPECT_FALSE(cancel.IsSessionCancelled());
  EXPECT_EQ(cancel.phase, ChartDldrDownloadCancelPhase::ChartDownloadActive);
}

TEST(ChartDldrScheduledUiLock, SessionCancelDuringChartAbortEndsRun) {
  ChartDldrDownloadCancelState cancel;
  cancel.ResetForBulkRun();
  cancel.BeginBulkSessionCancel();
  cancel.BeginActiveDownload();

  // A pending whole-session cancel implies aborting the current transfer and
  // survives re-arming the next chart download.
  cancel.ForceCancel();
  EXPECT_TRUE(cancel.IsSessionCancelled());
  EXPECT_TRUE(cancel.ShouldAbortCurrentTransfer());
  cancel.ConsumeAbortCurrentTransfer();
  EXPECT_TRUE(cancel.IsSessionCancelled());
  cancel.BeginActiveDownload();
  EXPECT_TRUE(cancel.IsSessionCancelled());
}

TEST(ChartDldrScheduledUiLock, OptionsClosePreservesScheduledContract) {
  EXPECT_TRUE(ChartDldrShouldPreserveScheduledRun(
      ChartDldrBulkCancelScope::OptionsClosed, true));
  EXPECT_FALSE(ChartDldrShouldPreserveScheduledRun(
      ChartDldrBulkCancelScope::PluginShutdown, true));
  EXPECT_TRUE(ChartDldrCatalogControlsFor(/*run_active=*/false,
                                          /*cancel_armed=*/false)
                  .download_button_enabled);
}
