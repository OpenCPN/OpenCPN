/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_prefs.h"

#include "chartdldr_bulk_run.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_pi.h"
#include "chartdldr_prefs_time.h"
#include "chartdldr_schedule_state.h"

#include <wx/intl.h>

ChartDldrPrefsDlgImpl::ChartDldrPrefsDlgImpl(wxWindow* parent,
                                             chartdldr_pi* plugin)
    : ChartDldrPrefsDlg(parent), plugin_(plugin) {
  m_sdbSizerBtnsOK->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlgImpl::OnOkClick), NULL, this);

  m_btnRunScheduledNow->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlgImpl::OnRunScheduledUpdateNow),
      NULL, this);

  m_cbScheduledEnable->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlgImpl::OnScheduledEnable), NULL,
      this);
  m_tcScheduledTime->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(ChartDldrPrefsDlgImpl::OnScheduledTimeChanged),
      NULL, this);
  ChartDldrUpdateScheduledTimePreviewWidgets(m_tcScheduledTime,
                                             m_stScheduledTimePreview);
}

ChartDldrPrefsDlgImpl::~ChartDldrPrefsDlgImpl() {
  m_sdbSizerBtnsOK->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlgImpl::OnOkClick), NULL, this);
  m_btnRunScheduledNow->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlgImpl::OnRunScheduledUpdateNow),
      NULL, this);
}

void ChartDldrPrefsDlgImpl::WarnScheduledPrerequisites() {
  if (!plugin_) {
    return;
  }

  wxString msg;
  if (!plugin_->HasChartSources()) {
    msg += _(
        "No chart sources are configured. Add sources on the Chart Downloader "
        "tab before scheduled updates can download charts.\n");
  }
  if (!m_cbSelectNew->GetValue() || !m_cbSelectUpdated->GetValue()) {
    msg += _(
        "Enable both \"All new charts\" and \"All updated charts\" so "
        "scheduled runs download the full set of available updates.\n");
  }
  if (!msg.IsEmpty()) {
    OCPNMessageBox_PlugIn(this, msg, _("Scheduled updates"),
                          wxOK | wxICON_WARNING);
  }
}

void ChartDldrPrefsDlgImpl::ApplyScheduledPrerequisitesOnSave() {
  if (!m_cbScheduledEnable->GetValue()) {
    return;
  }
  m_cbBulkUpdate->SetValue(true);
  m_cbSelectNew->SetValue(true);
  m_cbSelectUpdated->SetValue(true);
}

void ChartDldrPrefsDlgImpl::OnScheduledEnable(wxCommandEvent& event) {
  if (m_cbScheduledEnable->GetValue()) {
    WarnScheduledPrerequisites();
  }
  ChartDldrUpdateScheduledTimePreviewWidgets(m_tcScheduledTime,
                                             m_stScheduledTimePreview);
  event.Skip();
}

void ChartDldrPrefsDlgImpl::OnScheduledTimeChanged(wxCommandEvent& event) {
  ChartDldrUpdateScheduledTimePreviewWidgets(m_tcScheduledTime,
                                             m_stScheduledTimePreview);
  event.Skip();
}

void ChartDldrPrefsDlgImpl::OnRunScheduledUpdateNow(wxCommandEvent& event) {
  event.Skip();
  if (!plugin_) {
    return;
  }
  if (!ValidateScheduledTimeInput()) {
    return;
  }
  ApplyScheduledPrerequisitesOnSave();
  WarnScheduledPrerequisites();
  plugin_->UpdatePrefs(this);
  if (plugin_->RequestBulkUpdate(ChartDldrBulkRunKind::Scheduled)) {
    SetSchedulePreferences(plugin_->ScheduleConfig());
  }
}

bool ChartDldrPrefsDlgImpl::ValidateScheduledTimeInput() {
  int hour = 0;
  int minute = 0;
  if (!ChartDldrParseScheduledTimeEntry(m_tcScheduledTime->GetValue(), hour,
                                      minute)) {
    OCPNMessageBox_PlugIn(
        this,
        _("Enter the scheduled time as HH:MM in 24-hour local time "
          "(for example 03:00 or 15:30)."),
        _("Scheduled updates"), wxOK | wxICON_WARNING);
    return false;
  }

  m_tcScheduledTime->SetValue(ChartDldrFormatScheduledTimeEntry(hour, minute));
  ChartDldrUpdateScheduledTimePreviewWidgets(m_tcScheduledTime,
                                             m_stScheduledTimePreview);
  return true;
}

void ChartDldrPrefsDlgImpl::SetPath(const wxString path) {
  m_tcDefaultDir->SetValue(path);
}

void ChartDldrPrefsDlgImpl::GetPreferences(bool& preselect_new,
                                           bool& preselect_updated,
                                           bool& bulk_update) {
  preselect_new = m_cbSelectNew->GetValue();
  preselect_updated = m_cbSelectUpdated->GetValue();
  bulk_update = m_cbBulkUpdate->GetValue();
}

void ChartDldrPrefsDlgImpl::SetPreferences(bool preselect_new,
                                           bool preselect_updated,
                                           bool bulk_update) {
  m_cbSelectNew->SetValue(preselect_new);
  m_cbSelectUpdated->SetValue(preselect_updated);
  m_cbBulkUpdate->SetValue(bulk_update);
}

bool ChartDldrPrefsDlgImpl::GetSchedulePreferences(
    ChartDldrScheduleConfig& schedule) const {
  schedule.SetEnabled(m_cbScheduledEnable->GetValue());
  int hour = 0;
  int minute = 0;
  if (!ChartDldrParseScheduledTimeEntry(m_tcScheduledTime->GetValue(), hour,
                                      minute)) {
    return false;
  }
  schedule.SetTime(hour, minute);
  return true;
}

void ChartDldrPrefsDlgImpl::SetSchedulePreferences(
    const ChartDldrScheduleConfig& schedule) {
  m_cbScheduledEnable->SetValue(schedule.enabled);
  m_tcScheduledTime->SetValue(
      ChartDldrFormatScheduledTimeEntry(schedule.hour, schedule.minute));
  wxString label = _("Last run:");
  label << " " << ChartDldrFormatScheduledLastRunDisplay(schedule);
  m_stScheduledLastRun->SetLabel(label);
  ChartDldrUpdateScheduledTimePreviewWidgets(m_tcScheduledTime,
                                             m_stScheduledTimePreview);
}

void ChartDldrPrefsDlgImpl::OnOkClick(wxCommandEvent& event) {
  if (!ValidateScheduledTimeInput()) {
    return;
  }

  if (!wxDirExists(m_tcDefaultDir->GetValue())) {
    if (!wxFileName::Mkdir(m_tcDefaultDir->GetValue(), 0755,
                           wxPATH_MKDIR_FULL)) {
      OCPNMessageBox_PlugIn(
          this,
          wxString::Format(_("Directory %s can't be created."),
                           m_tcDefaultDir->GetValue().c_str()),
          _("Chart Downloader"));
      return;
    }
  }

  ApplyScheduledPrerequisitesOnSave();
  if (m_cbScheduledEnable->GetValue()) {
    WarnScheduledPrerequisites();
  }

  if (plugin_) {
    plugin_->UpdatePrefs(this);
  }

  event.Skip();
  EndModal(wxID_OK);
}
