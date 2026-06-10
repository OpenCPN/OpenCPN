/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <wx/event.h>

#include "chartdldr_bulk.h"

TEST(ChartDldrCatalogRefreshPath, SyncBlockingWithoutEventIsInvalid) {
  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Interactive, true));
  ASSERT_EQ(profile.catalog.refresh, ChartDldrCatalogRefreshMode::SyncBlocking);
  EXPECT_EQ(ChartDldrResolveCatalogRefreshPath(
                profile, ChartDldrCatalogRefreshContext::None()),
            ChartDldrCatalogRefreshPath::Invalid);
}

TEST(ChartDldrCatalogRefreshPath, AsyncIdleProfileIgnoresSyncEvent) {
  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, false));
  ASSERT_EQ(profile.catalog.refresh, ChartDldrCatalogRefreshMode::AsyncIdle);
  wxCommandEvent event;
  EXPECT_EQ(ChartDldrResolveCatalogRefreshPath(
                profile, ChartDldrCatalogRefreshContext::WithEvent(event)),
            ChartDldrCatalogRefreshPath::AsyncIdle);
}

TEST(ChartDldrCatalogRefreshPath, SyncBlockingWithEventUsesSyncPath) {
  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Interactive, true));
  wxCommandEvent event;
  EXPECT_EQ(ChartDldrResolveCatalogRefreshPath(
                profile, ChartDldrCatalogRefreshContext::WithEvent(event)),
            ChartDldrCatalogRefreshPath::SyncBlocking);
}

TEST(ChartDldrCatalogRefreshPath, AsyncIdleWithoutEventUsesAsyncPath) {
  const ChartDldrBulkModeProfile profile = ChartDldrBulkModeProfileFor(
      ChartDldrBulkRunUiPolicyFor(ChartDldrBulkRunKind::Scheduled, false));
  EXPECT_EQ(ChartDldrResolveCatalogRefreshPath(
                profile, ChartDldrCatalogRefreshContext::None()),
            ChartDldrCatalogRefreshPath::AsyncIdle);
}
