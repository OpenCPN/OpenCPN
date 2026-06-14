/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_PREFS_GUI_H_
#define CHARTDLDR_PREFS_GUI_H_

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

#endif  // CHARTDLDR_PREFS_GUI_H_
