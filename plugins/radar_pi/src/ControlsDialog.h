/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _CONTROLSDIALOG_H_
#define _CONTROLSDIALOG_H_

#include "RadarControlItem.h"
#include "radar_pi.h"
#define HAVE_CONTROL(a, b, c, d, e, f, g)
#include "SoftwareControlSet.h"
#undef HAVE_CONTROL

PLUGIN_BEGIN_NAMESPACE

enum {  // process ID's
  ID_TEXTCTRL1 = 10000,
  ID_BACK,
  ID_PLUS_TEN,
  ID_PLUS,
  ID_VALUE,
  ID_MINUS,
  ID_MINUS_TEN,
  ID_AUTO,
  ID_OFF,
  ID_CONTROL_BUTTON,

  ID_INSTALLATION,
  ID_PREFERENCES,

  ID_GAIN,

  ID_CLEAR_CURSOR,
  ID_ACQUIRE_TARGET,
  ID_DELETE_TARGET,
  ID_DELETE_ALL_TARGETS,

  ID_TARGETS_ON_PPI,
  ID_CLEAR_TRAILS,
  ID_ORIENTATION,
  ID_VIEW_CENTER,

  ID_TRANSMIT_STANDBY,

  ID_SHOW_RADAR_PPI,
  ID_DOCK_RADAR_PPI,

  ID_RADAR_OVERLAY0,
  ID_ADJUST = ID_RADAR_OVERLAY0 + MAX_CHART_CANVAS,
  ID_ADVANCED,
  ID_GUARDZONE,
  ID_WINDOW,
  ID_VIEW,
  ID_BEARING,
  ID_ZONE1,
  ID_ZONE2,
  ID_POWER,

  ID_CONFIRM_BOGEY,

  ID_MESSAGE,
  ID_BPOS,

  ID_BEARING_SET,  // next one should be BEARING_LINES higher
  ID_NEXT = ID_BEARING_SET + BEARING_LINES,

};

class RadarControlButton;
class RadarRangeControlButton;
class RadarButton;

extern wxString guard_zone_names[2];

extern string ControlTypeNames[CT_MAX];

extern wxSize g_buttonSize;

class ControlInfo {
 public:
  ControlType type;
  int autoValues;
  wxString *autoNames;
  bool hasOff;
  int defaultValue;
  int minValue;
  int maxValue;
  int stepValue;
  int nameCount;
  wxString unit;
  wxString *names;
};

//----------------------------------------------------------------------------------------------------------
//    Radar Control Dialog Specification
//----------------------------------------------------------------------------------------------------------
class ControlsDialog : public wxDialog {
  DECLARE_CLASS(ControlsDialog)
  DECLARE_EVENT_TABLE()

 public:
  ControlsDialog() {
    // Initialize all members that need initialization
    m_hide = false;
    m_hide_temporarily = true;

    m_from_control = 0;

    m_panel_position = wxDefaultPosition;

    m_pi = 0;
    m_ri = 0;
    m_top_sizer = 0;
    m_control_sizer = 0;
    m_parent = 0;
    m_advanced_sizer = 0;
    m_window_sizer = 0;
    m_view_sizer = 0;
    m_edit_sizer = 0;
    m_guard_sizer = 0;
    m_adjust_sizer = 0;
    m_cursor_sizer = 0;
    m_installation_sizer = 0;
    m_power_sizer = 0;
    m_transmit_sizer = 0;  // Controls disabled if not transmitting
    m_from_sizer = 0;      // If on edit control, this is where the button is from
    m_current_sizer = 0;
    m_from_control = 0;  // Only set when in edit mode
    m_plus_ten_button = 0;
    m_plus_button = 0;
    m_value_text = 0;
    m_comment_text = 0;
    m_minus_button = 0;
    m_minus_ten_button = 0;
    m_auto_button = 0;
    m_off_button = 0;
    m_power_button = 0;
    m_guard_zone = 0;
    m_guard_zone_text = 0;
    m_guard_zone_type = 0;
    m_outer_range = 0;
    m_inner_range = 0;
    m_start_bearing = 0;
    m_end_bearing = 0;
    m_arpa_box = 0;
    m_alarm = 0;
    CLEAR_STRUCT(m_bearing_buttons);
    m_clear_cursor = 0;
    m_acquire_target = 0;
    m_delete_target = 0;
    m_delete_all = 0;
    m_target_trails_button = 0;
    m_targets_on_ppi_button = 0;
    m_trails_motion_button = 0;
    m_clear_trails_button = 0;
    m_orientation_button = 0;
    m_power_sub_button = 0;
    m_transmit_button = 0;
    m_standby_button = 0;
    m_timed_idle_button = 0;
    m_timed_run_button = 0;
    m_interference_rejection_button = 0;
    m_target_separation_button = 0;
    m_noise_rejection_button = 0;
    m_target_boost_button = 0;
    m_target_expansion_button = 0;
    m_scan_speed_button = 0;
    m_bearing_alignment_button = 0;
    m_no_transmit_start_button = 0;
    m_no_transmit_end_button = 0;
    m_antenna_height_button = 0;
    m_antenna_forward_button = 0;
    m_antenna_starboard_button = 0;
    m_local_interference_rejection_button = 0;
    m_side_lobe_suppression_button = 0;
    m_main_bang_size_button = 0;
    m_show_ppi_button = 0;
    CLEAR_STRUCT(m_overlay_button);
    m_range_button = 0;
    m_transparency_button = 0;  // TODO: Set it on change
    m_refresh_rate_button = 0;  // TODO: Set it on change
    m_gain_button = 0;
    m_sea_button = 0;
    m_rain_button = 0;
    m_ftc_button = 0;
    m_adjust_button = 0;
    m_cursor_menu = 0;
    m_doppler_button = 0;

    for (size_t i = 0; i < ARRAY_SIZE(m_ctrl); i++) {
      m_ctrl[i].type = CT_NONE;
      m_ctrl[i].names = 0;
      m_ctrl[i].autoNames = 0;
      m_ctrl[i].hasOff = false;
    }
  };
  ~ControlsDialog();

  bool Create(wxWindow *parent, radar_pi *pi, RadarInfo *ri, wxWindowID id = wxID_ANY, const wxString &caption = _("Radar"),
              const wxPoint &pos = wxDefaultPosition);

  void AdjustRange(int adjustment);
  wxString &GetRangeText();
  void SetTimedIdleIndex(int index);
  void UpdateGuardZoneState();
  void UpdateDialogShown(bool resize);
  void UpdateControlValues(bool force);
  void SetErrorMessage(wxString &msg);
  void ShowBogeys(wxString text, bool confirmed);
  void HideBogeys();

  void HideTemporarily();  // When a second dialog (which is not a child class) takes over -- aka GuardZone
  void UnHideTemporarily();
  void ShowDialog();
  void HideDialog();
  void ShowCursorPane() { SwitchTo(m_cursor_sizer, wxT("cursor")); }

  radar_pi *m_pi;
  RadarInfo *m_ri;
  wxString m_log_name;
  wxBoxSizer *m_top_sizer;
  wxBoxSizer *m_control_sizer;
  wxPoint m_panel_position;
  ControlInfo m_ctrl[CT_MAX];

 protected:
  void DefineControl(ControlType ct, int autoValues, wxString auto_names[], int defaultValue, int minValue, int maxValue,
                     int stepValue, int nameCount, wxString names[]) {
    m_ctrl[ct].type = ct;
    if (defaultValue == CTD_DEF_OFF) {
      m_ctrl[ct].hasOff = true;
      defaultValue = CTD_DEF_ZERO;
    }
    m_ctrl[ct].defaultValue = defaultValue;
    m_ctrl[ct].minValue = minValue;
    m_ctrl[ct].maxValue = maxValue;
    m_ctrl[ct].stepValue = stepValue;
    m_ctrl[ct].nameCount = nameCount;

    // To simplify the macros a control without autovalues passes in
    // CTD_AUTO_NO, which is an array of 1 with length zero.
    if (autoValues == 1 && auto_names[0].length() == 0) {
      autoValues = 0;
      m_ctrl[ct].autoNames = 0;
    }
    m_ctrl[ct].autoValues = autoValues;

    if (autoValues > 0) {
      m_ctrl[ct].autoNames = new wxString[autoValues];
      for (int i = 0; i < autoValues; i++) {
        m_ctrl[ct].autoNames[i] = auto_names[i];
      }
    }

    if (nameCount == 1 && names[0].length() > 0) {
      m_ctrl[ct].unit = names[0];
    } else if (nameCount > 0 && names[0].length() > 0) {
      m_ctrl[ct].names = new wxString[nameCount];
      for (int i = 0; i < nameCount; i++) {
        m_ctrl[ct].names[i] = names[i];
      }
    }
  }

  bool m_hide;
  bool m_hide_temporarily;
  time_t m_auto_hide_timeout;  // At what time do we hide the dialog

  wxWindow *m_parent;
  wxBoxSizer *m_advanced_sizer;
  wxBoxSizer *m_window_sizer;
  wxBoxSizer *m_view_sizer;
  wxBoxSizer *m_edit_sizer;
  wxBoxSizer *m_guard_sizer;      // The edit sizer for guard zone 1 or 2
  wxBoxSizer *m_guardzone_sizer;  // The menu sizer with Guard Zone 1 + 2
  wxBoxSizer *m_adjust_sizer;
  wxBoxSizer *m_cursor_sizer;
  wxBoxSizer *m_installation_sizer;
  wxBoxSizer *m_power_sizer;
  wxBoxSizer *m_transmit_sizer;  // Controls disabled if not transmitting
  wxBoxSizer *m_from_sizer;      // If on edit control, this is where the button is from
  wxBoxSizer *m_current_sizer;   // The currently shown sizer

  // Edit Controls
  RadarControlButton *m_from_control;  // Only set when in edit mode

  // The 'edit' control has these buttons:
  wxButton *m_plus_ten_button;
  wxButton *m_plus_button;
  wxStaticText *m_value_text;
  wxStaticText *m_comment_text;
  wxButton *m_minus_button;
  wxButton *m_minus_ten_button;
  wxButton *m_auto_button;
  wxButton *m_off_button;

  // Main control
  wxButton *m_guard_1_button;
  wxButton *m_guard_2_button;
  wxButton *m_power_button;

  // Guard Zone Edit

  GuardZone *m_guard_zone;
  wxStaticText *m_guard_zone_text;
  wxRadioBox *m_guard_zone_type;
  wxTextCtrl *m_outer_range;
  wxTextCtrl *m_inner_range;
  wxTextCtrl *m_start_bearing;
  wxTextCtrl *m_end_bearing;
  wxCheckBox *m_arpa_box;
  wxCheckBox *m_alarm;

  // Cursor controls
  wxButton *m_bearing_buttons[BEARING_LINES];
  wxButton *m_clear_cursor;
  wxButton *m_acquire_target;
  wxButton *m_delete_target;
  wxButton *m_delete_all;

  // View controls
  RadarControlButton *m_targets_on_ppi_button;
  RadarControlButton *m_doppler_button;
  RadarControlButton *m_target_trails_button;
  RadarControlButton *m_trails_motion_button;
  wxButton *m_clear_trails_button;
  RadarControlButton *m_orientation_button;
  RadarControlButton *m_view_center_button;

  // Power controls
  wxButton *m_power_sub_button;
  wxButton *m_transmit_button;
  wxButton *m_standby_button;
  RadarControlButton *m_timed_idle_button;
  RadarControlButton *m_timed_run_button;

  // Advanced controls
  RadarControlButton *m_interference_rejection_button;
  RadarControlButton *m_target_separation_button;
  RadarControlButton *m_noise_rejection_button;
  RadarControlButton *m_target_boost_button;
  RadarControlButton *m_target_expansion_button;
  RadarControlButton *m_scan_speed_button;

  // Installation controls
  RadarControlButton *m_bearing_alignment_button;
  RadarControlButton *m_no_transmit_start_button;
  RadarControlButton *m_no_transmit_end_button;
  RadarControlButton *m_antenna_height_button;
  RadarControlButton *m_antenna_forward_button;
  RadarControlButton *m_antenna_starboard_button;
  RadarControlButton *m_local_interference_rejection_button;
  RadarControlButton *m_side_lobe_suppression_button;
  RadarControlButton *m_main_bang_size_button;

  // Window controls
  RadarButton *m_show_ppi_button;
  RadarButton *m_dock_ppi_button;
  RadarControlButton *m_overlay_button[MAX_CHART_CANVAS];
  RadarControlButton *m_transparency_button;

  // Adjust controls
  RadarRangeControlButton *m_range_button;
  RadarControlButton *m_refresh_rate_button;
  RadarControlButton *m_gain_button;
  RadarControlButton *m_sea_button;
  RadarControlButton *m_rain_button;
  RadarControlButton *m_ftc_button;
  wxButton *m_adjust_button;
  wxButton *m_cursor_menu;

  // Status text
  wxStaticText *m_status_text;

  // Methods common to any radar
  void EnsureWindowNearOpenCPNWindow();
  void ShowGuardZone(int zone);
  void SetGuardZoneVisibility();
  void SetMenuAutoHideTimeout();
  void SwitchTo(wxBoxSizer *to, const wxChar *name);
  bool UpdateSizersButtonsShown();

 public:
  void Resize(bool force);
  void CreateControls();
  void DisableRadarControls();
  void EnableRadarControls();

  // Methods that we know that every radar must or may implement its own way
  virtual void UpdateRadarSpecificState(){};

  // Callbacks when a button is pressed

  void OnClose(wxCloseEvent &event);
  void OnIdOKClick(wxCommandEvent &event);
  void OnMove(wxMoveEvent &event);

  void OnPlusTenClick(wxCommandEvent &event);
  void OnPlusClick(wxCommandEvent &event);
  void OnBackClick(wxCommandEvent &event);
  void OnMinusClick(wxCommandEvent &event);
  void OnMinusTenClick(wxCommandEvent &event);
  void OnAutoClick(wxCommandEvent &event);
  void OnOffClick(wxCommandEvent &event);

  void OnAdjustButtonClick(wxCommandEvent &event);
  void OnAdvancedButtonClick(wxCommandEvent &event);
  void OnGuardZoneButtonClick(wxCommandEvent &event);
  void OnWindowButtonClick(wxCommandEvent &event);
  void OnViewButtonClick(wxCommandEvent &event);
  void OnInstallationButtonClick(wxCommandEvent &event);
  void OnPreferencesButtonClick(wxCommandEvent &event);

  void OnRadarGainButtonClick(wxCommandEvent &event);

  void OnPowerButtonClick(wxCommandEvent &event);
  void OnRadarShowPPIButtonClick(wxCommandEvent &event);
  void OnRadarDockPPIButtonClick(wxCommandEvent &event);
  void OnRadarOverlayButtonClick(wxCommandEvent &event);
  void OnMessageButtonClick(wxCommandEvent &event);

  void OnTargetsOnPPIButtonClick(wxCommandEvent &event);
  void OnClearTrailsButtonClick(wxCommandEvent &event);
  void OnOrientationButtonClick(wxCommandEvent &event);
  void OnViewCenterButtonClick(wxCommandEvent &event);

  void OnRadarControlButtonClick(wxCommandEvent &event);

  void OnZone1ButtonClick(wxCommandEvent &event);
  void OnZone2ButtonClick(wxCommandEvent &event);

  void OnClearCursorButtonClick(wxCommandEvent &event);
  void OnAcquireTargetButtonClick(wxCommandEvent &event);
  void OnDeleteTargetButtonClick(wxCommandEvent &event);
  void OnDeleteAllTargetsButtonClick(wxCommandEvent &event);
  void OnBearingSetButtonClick(wxCommandEvent &event);
  void OnBearingButtonClick(wxCommandEvent &event);

  void OnConfirmBogeyButtonClick(wxCommandEvent &event);

  void OnTransmitButtonClick(wxCommandEvent &event);

  void EnterEditMode(RadarControlButton *button);

  void OnGuardZoneModeClick(wxCommandEvent &event);
  void OnInner_Range_Value(wxCommandEvent &event);
  void OnOuter_Range_Value(wxCommandEvent &event);
  void OnStart_Bearing_Value(wxCommandEvent &event);
  void OnEnd_Bearing_Value(wxCommandEvent &event);
  void OnARPAClick(wxCommandEvent &event);
  void OnAlarmClick(wxCommandEvent &event);
};

class RadarButton : public wxButton {
 public:
  RadarButton(){

  };

  RadarButton(ControlsDialog *parent, wxWindowID id, wxSize buttonSize, const wxString &label) {
    Create(parent, id, label, wxDefaultPosition, buttonSize, 0, wxDefaultValidator, label);

    m_parent = parent;
    m_pi = m_parent->m_pi;
    SetFont(m_parent->m_pi->m_font);
    SetLabel(label);  // Use the \n on Mac to enforce double height button
  }

  ControlsDialog *m_parent;
  radar_pi *m_pi;

  void SetLabel(const wxString &label) {
    wxString newLabel;

#ifdef __WXOSX__
    newLabel << wxT("\n");
#endif
    newLabel << label;
#ifdef __WXOSX__
    newLabel << wxT("\n");
#endif
    wxButton::SetLabel(newLabel);
  }
};

class DynamicStaticText : public wxStaticText {
 public:
  DynamicStaticText() {}

  DynamicStaticText(wxWindow *parent, wxWindowID id, const wxString &label, const wxPoint &pos = wxDefaultPosition,
                    const wxSize &size = wxDefaultSize, long style = 0, const wxString &name = wxStaticTextNameStr) {
    Create(parent, id, label, pos, size, style, name);
  }

  void SetLabel(const wxString &label) {
    wxStaticText::SetLabel(label);
    SetSize(GetTextExtent(label));
  }
};

class RadarControlButton : public wxButton {
  friend class RadarRangeControlButton;

 public:
  RadarControlButton(){

  };

  RadarControlButton(ControlsDialog *parent, wxWindowID id, const wxString &label, ControlInfo &ctrl, RadarControlItem *item,
                     const wxString &newUnit = wxT(""), const wxString &newComment = wxT("")) {
    Create(parent, id, label + wxT("\n"), wxDefaultPosition, g_buttonSize, 0, wxDefaultValidator, label);

    m_parent = parent;
    m_pi = m_parent->m_pi;
    m_ci = ctrl;

    if (label.StartsWith(wxT("-"))) {
      firstLine = label.Mid(1);
      m_no_edit = true;
    } else if (m_ci.maxValue < m_ci.minValue + 4) {
      firstLine = label;
      m_no_edit = true;
    } else {
      firstLine = label;
      m_no_edit = false;
    }

    if (newUnit.length() > 0) {
      m_ci.unit = newUnit;
    }
    m_comment = newComment;

    this->SetFont(m_parent->m_pi->m_font);
    m_item = item;
    UpdateLabel(true);
  }

  virtual void AdjustValue(int adjustment);
  virtual bool ToggleValue();  // Returns true if value was toggled to next value
  virtual bool ToggleState();  // Returns desired new state for Auto/Off button show.
  virtual void SetState(RadarControlState state);
  virtual void UpdateLabel(bool force = false);
  virtual void SetFirstLine(wxString first_line);
  wxString GetLabel() const;

  wxString m_comment;
  RadarControlItem *m_item;
  ControlInfo m_ci;

 private:
  wxString firstLine;
  bool m_no_edit;

  ControlsDialog *m_parent;
  radar_pi *m_pi;  // could be accessed through m_parent but the M_SETTINGS macro requires it directly in this class.0
};

class RadarRangeControlButton : public RadarControlButton {
 public:
  RadarRangeControlButton(ControlsDialog *parent, wxWindowID id, wxSize buttonSize, const wxString &label, RadarControlItem *item) {
    Create(parent, id, label + wxT("\n"), wxDefaultPosition, buttonSize, 0, wxDefaultValidator, label);

    m_parent = parent;
    m_pi = m_parent->m_pi;
    m_item = item;
    m_no_edit = false;
    firstLine = label;
    m_ci.minValue = 0;
    m_ci.maxValue = 0;
    m_ci.autoValues = 1;
    m_ci.autoNames = 0;
    m_ci.unit = wxT("");
    m_ci.names = 0;
    m_ci.type = CT_RANGE;
    m_ci.hasOff = false;
    m_ci.defaultValue = 0;
    m_ci.stepValue = 1;

    SetFont(m_parent->m_pi->m_font);
  }

  virtual void AdjustValue(int adjustment);
  bool ToggleState();  // Returns desired new state for Auto button show
  void SetRangeLabel();
};

// This sets up the initializer macro in the constructor of the
// derived control dialogs.
#define HAVE_CONTROL(a, b, c, d, e, f, g) \
  wxString a##_auto_names[] = b;          \
  wxString a##_names[] = g;               \
  DefineControl(a, ARRAY_SIZE(a##_auto_names), a##_auto_names, c, d, e, f, ARRAY_SIZE(a##_names), a##_names);
#define SKIP_CONTROL(a)

PLUGIN_END_NAMESPACE

#endif
