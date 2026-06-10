/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_prefs_gui.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

ChartDldrScheduledPrefsWidgets ChartDldrCreateScheduledUpdateWidgets(
    wxWindow* parent) {
  ChartDldrScheduledPrefsWidgets widgets;
  widgets.sizer = new wxStaticBoxSizer(
      new wxStaticBox(parent, wxID_ANY, _("Scheduled updates")), wxVERTICAL);

  widgets.enable =
      new wxCheckBox(parent, wxID_ANY,
                     _("Enable daily bulk update while OpenCPN is running"),
                     wxDefaultPosition, wxDefaultSize, 0);
  widgets.sizer->Add(widgets.enable, 0, wxALL, 4);

  wxFlexGridSizer* timeSizer = new wxFlexGridSizer(1, 3, 4, 4);
  timeSizer->Add(new wxStaticText(parent, wxID_ANY, _("Run at:")), 0,
                 wxALL | wxALIGN_CENTER_VERTICAL, 4);
  widgets.time = new wxTextCtrl(parent, wxID_ANY, wxT("03:00"),
                                wxDefaultPosition, wxSize(64, -1), 0);
  widgets.time->SetToolTip(
      _("Local time in 24-hour HH:MM form, for example 03:00 or 15:30."));
  timeSizer->Add(widgets.time, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
  widgets.time_preview =
      new wxStaticText(parent, wxID_ANY, wxEmptyString);
  timeSizer->Add(widgets.time_preview, 1,
                 wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 4);
  widgets.sizer->Add(timeSizer, 0, wxEXPAND | wxALL, 4);

  widgets.last_run =
      new wxStaticText(parent, wxID_ANY, _("Last run: -- : --"));
  widgets.sizer->Add(widgets.last_run, 0, wxALL, 4);

  widgets.run_now = new wxButton(parent, wxID_ANY, _("Run update now"));
  widgets.sizer->Add(widgets.run_now, 0, wxALL, 4);

  return widgets;
}
