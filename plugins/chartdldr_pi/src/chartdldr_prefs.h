/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_PREFS_H_
#define CHARTDLDR_PREFS_H_

#include "chartdldrgui.h"
#include "chartdldr_schedule_config.h"

class chartdldr_pi;

class wxButton;
class wxCheckBox;
class wxStaticBoxSizer;
class wxStaticText;
class wxTextCtrl;
class wxWindow;

struct ChartDldrScheduledPrefsWidgets {
  wxStaticBoxSizer* sizer = nullptr;
  wxCheckBox* enable = nullptr;
  wxTextCtrl* time = nullptr;
  wxStaticText* time_preview = nullptr;
  wxStaticText* last_run = nullptr;
  wxButton* run_now = nullptr;
};

ChartDldrScheduledPrefsWidgets ChartDldrCreateScheduledUpdateWidgets(
    wxWindow* parent);

class ChartDldrPrefsDlgImpl : public ChartDldrPrefsDlg {
protected:
  void OnOkClick(wxCommandEvent& event);
  void OnScheduledEnable(wxCommandEvent& event);
  void OnScheduledTimeChanged(wxCommandEvent& event);
  void OnRunScheduledUpdateNow(wxCommandEvent& event);

  void WarnScheduledPrerequisites();
  void ApplyScheduledRunPrerequisites();
  void ApplyScheduledPrerequisitesOnSave();

  chartdldr_pi* plugin_;

public:
  bool ValidateScheduledTimeInput();
  ChartDldrPrefsDlgImpl(wxWindow* parent, chartdldr_pi* plugin);
  ~ChartDldrPrefsDlgImpl();

  wxString GetPath() { return m_tcDefaultDir->GetValue(); }
  void SetPath(const wxString path);
  void GetPreferences(bool& preselect_new, bool& preselect_updated,
                      bool& bulk_update);
  void SetPreferences(bool preselect_new, bool preselect_updated,
                      bool bulk_update);
  bool GetSchedulePreferences(ChartDldrScheduleConfig& schedule) const;
  void SetSchedulePreferences(const ChartDldrScheduleConfig& schedule);
};

#endif  // CHARTDLDR_PREFS_H_
