/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_schedule.h"

#include "chartdldr_bulk_policy.h"
#include "chartdldr_panel.h"
#include "chartdldr_pi.h"
#include "chartdldr_schedule.h"

#include <wx/timer.h>

namespace {

void MarkScheduledLastRunNow(chartdldr_pi* pi) {
  if (!pi) {
    return;
  }
  pi->m_scheduled_last_run_iso = wxDateTime::Now().FormatISOCombined(' ');
}

void SkipBulkUpdate(chartdldr_pi* pi, const wxString& status) {
  if (!pi) {
    return;
  }
  pi->m_scheduled_last_status = status;
  wxLogMessage("chartdldr_pi: %s", status.c_str());
  MarkScheduledLastRunNow(pi);
  pi->SaveConfig();
}

}  // namespace

void ChartDldrRestartScheduleTimer(chartdldr_pi* pi) {
  if (!pi || !pi->m_schedule_timer) {
    return;
  }
  pi->m_schedule_timer->Stop();
  if (pi->m_scheduled_enabled && pi->m_allow_bulk_update) {
    pi->m_schedule_timer->Start(60000, wxTIMER_CONTINUOUS);
  }
}

void ChartDldrOnScheduleTimer(chartdldr_pi* pi, wxTimerEvent& event) {
  event.Skip();
  if (!pi || pi->m_bulk_run_active) {
    return;
  }

  if (!ChartDldrScheduleShouldRun(wxDateTime::Now(), pi->m_scheduled_enabled,
                                  pi->m_scheduled_hour, pi->m_scheduled_minute,
                                  pi->m_scheduled_last_run_iso)) {
    return;
  }

  ChartDldrRequestBulkUpdate(pi, false);
}

bool ChartDldrRequestBulkUpdate(chartdldr_pi* pi, bool interactive) {
  if (!pi || pi->m_bulk_run_active) {
    return false;
  }
  if (!pi->m_allow_bulk_update) {
    if (!interactive) {
      SkipBulkUpdate(pi, _("Skipped: enable bulk update in preferences"));
    }
    return false;
  }
  if (pi->m_ChartSources.empty()) {
    if (!interactive) {
      SkipBulkUpdate(pi, _("Skipped: no chart sources configured"));
    }
    return false;
  }
  if (!ChartDldrEnsureDownloaderPanel(pi)) {
    if (!interactive) {
      SkipBulkUpdate(pi, _("Skipped: downloader panel unavailable"));
    }
    return false;
  }

  wxCommandEvent evt;
  return pi->m_dldrpanel->RunBulkUpdate(interactive, evt);
}

void ChartDldrRecordScheduledBulkRunResult(chartdldr_pi* pi, int downloaded_ok,
                                           int attempted, int failed) {
  if (!pi) {
    return;
  }

  if (attempted == 0) {
    pi->m_scheduled_last_status = _("No charts downloaded");
  } else if (failed == 0) {
    pi->m_scheduled_last_status =
        wxString::Format(_("OK (%d charts)"), downloaded_ok);
  } else {
    pi->m_scheduled_last_status = wxString::Format(
        _("%d failed of %d"), failed, attempted);
  }

  if (ChartDldrShouldAdvanceScheduledLastRun(downloaded_ok, attempted, failed)) {
    wxLogMessage("chartdldr_pi: scheduled bulk update finished: %s",
                 pi->m_scheduled_last_status.c_str());
    MarkScheduledLastRunNow(pi);
  } else if (attempted == 0) {
    wxLogMessage(
        "chartdldr_pi: scheduled bulk update: %s; will retry later today if "
        "still past the scheduled time",
        pi->m_scheduled_last_status.c_str());
  } else {
    wxLogMessage(
        "chartdldr_pi: scheduled bulk update: %s; no successful downloads, "
        "will retry later today",
        pi->m_scheduled_last_status.c_str());
  }
  pi->SaveConfig();
}
