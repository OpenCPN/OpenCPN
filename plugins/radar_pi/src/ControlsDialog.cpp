/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
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

#include "ControlsDialog.h"
#include "GuardZone.h"
#include "MessageBox.h"
#include "RadarMarpa.h"
#include "RadarPanel.h"

PLUGIN_BEGIN_NAMESPACE

#if defined(__WXOSX__) || defined(__WXMSW__) || defined(__WXGTK__)
#define HAS_UNICODE_CHARS
#endif

#ifdef HAS_UNICODE_CHARS
#define MENU_BACK(x) (wxT("\u25c0 ") + x)
#define MENU(x) (x + wxT(" \u25b6"))
#define MENU_EDIT(x) (x + wxT(" \u25b7"))
#define MENU_WINDOW(x) (x + wxT(" ..."))
#else
#define MENU_BACK(x) (wxT("<< ") + x)
#define MENU(x) (x + wxT(" >>"))
#define MENU_EDIT(x) (x + wxT(" >"))
#define MENU_WINDOW(x) (x + wxT(" ..."))
#endif

#define MENU_NO_EDIT(x) (wxT("-") + x)

//---------------------------------------------------------------------------------------
//          Radar Control Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS(ControlsDialog, wxDialog)

BEGIN_EVENT_TABLE(ControlsDialog, wxDialog)

EVT_CLOSE(ControlsDialog::OnClose)
EVT_BUTTON(ID_BACK, ControlsDialog::OnBackClick)
EVT_BUTTON(ID_PLUS_TEN, ControlsDialog::OnPlusTenClick)
EVT_BUTTON(ID_PLUS, ControlsDialog::OnPlusClick)
EVT_BUTTON(ID_MINUS, ControlsDialog::OnMinusClick)
EVT_BUTTON(ID_MINUS_TEN, ControlsDialog::OnMinusTenClick)
EVT_BUTTON(ID_AUTO, ControlsDialog::OnAutoClick)
EVT_BUTTON(ID_OFF, ControlsDialog::OnOffClick)
EVT_BUTTON(ID_CONTROL_BUTTON, ControlsDialog::OnRadarControlButtonClick)

EVT_BUTTON(ID_INSTALLATION, ControlsDialog::OnInstallationButtonClick)
EVT_BUTTON(ID_PREFERENCES, ControlsDialog::OnPreferencesButtonClick)

EVT_BUTTON(ID_POWER, ControlsDialog::OnPowerButtonClick)
EVT_BUTTON(ID_SHOW_RADAR_PPI, ControlsDialog::OnRadarShowPPIButtonClick)
EVT_BUTTON(ID_DOCK_RADAR_PPI, ControlsDialog::OnRadarDockPPIButtonClick)
EVT_BUTTON(ID_GAIN, ControlsDialog::OnRadarGainButtonClick)

EVT_BUTTON(ID_TARGETS_ON_PPI, ControlsDialog::OnTargetsOnPPIButtonClick)
EVT_BUTTON(ID_CLEAR_TRAILS, ControlsDialog::OnClearTrailsButtonClick)
EVT_BUTTON(ID_ORIENTATION, ControlsDialog::OnOrientationButtonClick)
EVT_BUTTON(ID_VIEW_CENTER, ControlsDialog::OnViewCenterButtonClick)

EVT_BUTTON(ID_ADJUST, ControlsDialog::OnAdjustButtonClick)
EVT_BUTTON(ID_ADVANCED, ControlsDialog::OnAdvancedButtonClick)
EVT_BUTTON(ID_GUARDZONE, ControlsDialog::OnGuardZoneButtonClick)
EVT_BUTTON(ID_WINDOW, ControlsDialog::OnWindowButtonClick)
EVT_BUTTON(ID_VIEW, ControlsDialog::OnViewButtonClick)

EVT_BUTTON(ID_BEARING, ControlsDialog::OnBearingButtonClick)
EVT_BUTTON(ID_ZONE1, ControlsDialog::OnZone1ButtonClick)
EVT_BUTTON(ID_ZONE2, ControlsDialog::OnZone2ButtonClick)

EVT_BUTTON(ID_MESSAGE, ControlsDialog::OnMessageButtonClick)

EVT_BUTTON(ID_BEARING_SET, ControlsDialog::OnBearingSetButtonClick)
EVT_BUTTON(ID_CLEAR_CURSOR, ControlsDialog::OnClearCursorButtonClick)
EVT_BUTTON(ID_ACQUIRE_TARGET, ControlsDialog::OnAcquireTargetButtonClick)
EVT_BUTTON(ID_DELETE_TARGET, ControlsDialog::OnDeleteTargetButtonClick)
EVT_BUTTON(ID_DELETE_ALL_TARGETS, ControlsDialog::OnDeleteAllTargetsButtonClick)

EVT_BUTTON(ID_TRANSMIT_STANDBY, ControlsDialog::OnTransmitButtonClick)

EVT_MOVE(ControlsDialog::OnMove)
EVT_CLOSE(ControlsDialog::OnClose)

END_EVENT_TABLE()

// The following are only for logging, so don't care about translations.
string ControlTypeNames[CT_MAX] = {
    "Unused",
#define CONTROL_TYPE(x, y) y,
#include "ControlType.inc"
#undef CONTROL_TYPE
};

wxString guard_zone_names[2];

void RadarControlButton::AdjustValue(int adjustment) {
  int oldValue = m_item->GetValue();
  int newValue = oldValue;

  if (m_item->GetState() == RCS_OFF) {
    m_item->UpdateState(RCS_MANUAL);
  } else {
    newValue += adjustment;
    if (newValue < m_ci.minValue) {
      newValue = m_ci.minValue;
    } else if (newValue > m_ci.maxValue) {
      newValue = m_ci.maxValue;
    }
    m_item->Update(newValue, RCS_MANUAL);
  }

  if (m_item->IsModified()) {
    LOG_VERBOSE(wxT("%s Adjusting '%s' by %d from %d to %d"), m_parent->m_log_name.c_str(), GetName(), adjustment, oldValue,
                newValue);
    UpdateLabel();
    m_parent->m_ri->SetControlValue(m_ci.type, *m_item, this);
  }
}

bool RadarControlButton::ToggleValue() {
  if (m_no_edit) {
    int oldValue = m_item->GetValue();
    int newValue = oldValue;

    if (m_item->GetState() == RCS_MANUAL) {
      newValue += 1;
      if (newValue < m_ci.minValue) {
        newValue = m_ci.minValue;
      } else if (newValue > m_ci.maxValue) {
        newValue = m_ci.minValue;
      }
      m_item->Update(newValue);
    }
    SetState(RCS_MANUAL);
    UpdateLabel();
    return true;
  }
  return false;
}

bool RadarControlButton::ToggleState() {
  RadarControlState state = m_item->GetState();

  if (m_ci.autoValues == 0) {
    if (state != RCS_MANUAL) {
      state = RCS_MANUAL;
    } else {
      state = RCS_OFF;
    }
  } else if (state >= RCS_AUTO_1 && state < RCS_MANUAL + m_ci.autoValues) {
    state = (RadarControlState)(state + 1);
  } else {
    state = RCS_AUTO_1;
  }
  SetState(state);
  return m_ci.autoValues > 1;
}

void RadarControlButton::SetState(RadarControlState state) {
  m_item->UpdateState(state);
  LOG_VERBOSE(wxT("%s Button '%s' SetState %d value %d, max=%d"), m_parent->m_log_name.c_str(), ControlTypeNames[m_ci.type], state,
              m_item->GetValue(), m_ci.autoValues);
  m_parent->m_ri->SetControlValue(m_ci.type, *m_item, this);
}

wxString RadarControlButton::GetLabel() const {
  wxString label = wxButton::GetLabel();

  label = firstLine + "\n" + label.AfterFirst('\n');
  return label;
}

void RadarControlButton::SetFirstLine(wxString first_line) { firstLine = first_line; }

void RadarControlButton::UpdateLabel(bool force) {
  RadarControlState state;
  int value;
  wxString label;

  if (m_item->GetButton(&value, &state) || force) {
    // label << MENU_EDIT(firstLine) << wxT("\n");
    if (m_no_edit) {
      label << firstLine;
    } else {
      label << MENU_EDIT(firstLine);
    }
    label << wxT("\n");

    switch (state) {
      case RCS_OFF:
        label << _("Off");
        break;

      case RCS_MANUAL:
        if (m_ci.names) {
          if (value >= 0 && value < m_ci.nameCount) {
            label << m_ci.names[value];
          }
        } else {
          label << value * m_ci.stepValue;
        }
        if (m_ci.unit.length() > 0) {
          label << wxT(" ") << m_ci.unit;
        }
        break;

      default:
        // Various AUTO states
        if (m_ci.autoNames && state > RCS_MANUAL && state <= RCS_MANUAL + m_ci.autoValues) {
          if (m_ci.autoValues > 1) {
            label << _("Auto") << wxT(" ");
          }
          label << m_ci.autoNames[state - RCS_AUTO_1];
        } else {
          label << _("Auto");
        }
        if (m_parent->m_ri->m_showManualValueInAuto) {
          label << wxString::Format(wxT(" %d"), value * m_ci.stepValue);
          if (m_ci.unit.length() > 0) {
            label << wxT(" ") << m_ci.unit;
          }
        }
        break;
    }
    wxButton::SetLabel(label);

    IF_LOG_AT_LEVEL(LOGLEVEL_VERBOSE) {
      wxString loglabel;

      loglabel << label;
      loglabel.Replace(wxT("\n"), wxT("/"));
      LOG_VERBOSE(wxT("%s Button '%s' set state %d value %d label='%s'"), m_parent->m_log_name.c_str(), ControlTypeNames[m_ci.type],
                  state, m_item->GetValue(), loglabel.c_str());
    }
  }
}

void RadarRangeControlButton::SetRangeLabel() {
  wxString label = MENU_EDIT(firstLine) + wxT("\n") + m_parent->m_ri->GetRangeText();
  wxButton::SetLabel(label);

  IF_LOG_AT_LEVEL(LOGLEVEL_VERBOSE) {
    wxString loglabel;

    loglabel << label;
    loglabel.Replace(wxT("\n"), wxT("/"));
    LOG_VERBOSE(wxT("%s Button '%s' set state %d value %d label='%s'"), m_parent->m_log_name.c_str(), ControlTypeNames[m_ci.type],
                m_item->GetState(), m_item->GetValue(), loglabel.c_str());
  }
}

void RadarRangeControlButton::AdjustValue(int adjustment) {
  LOG_VERBOSE(wxT("%s Button '%s' adjust by %d"), m_parent->m_log_name.c_str(), GetName(), adjustment);
  m_item->UpdateState(RCS_MANUAL);
  m_parent->m_ri->AdjustRange(adjustment);  // send new value to the radar
}

bool RadarRangeControlButton::ToggleState() {
  RadarControlState state = m_item->GetState();

  LOG_VERBOSE(wxT("%s Button '%s' toggle Auto %d"), m_parent->m_log_name.c_str(), GetName(), state);

  // If any of the canvases has this as overlay we allow auto range
  bool allowManual = false;
  for (int i = 0; i < CANVAS_COUNT; i++) {
    if (m_parent->m_ri->m_overlay_canvas[i].GetValue() > 0) {
      allowManual = true;
      break;
    }
  }
  if (state >= RCS_AUTO_1 && allowManual) {
    state = RCS_MANUAL;
  } else {
    state = RCS_AUTO_1;
  }
  m_item->UpdateState(state);
  SetState(state);
  return false;
}

ControlsDialog::~ControlsDialog() {
  wxPoint pos = GetPosition();
  // When radar panel is hidden GetPosition() sometimes may return very large numbers
  if (pos.x < 5000 && pos.y < 5000 && pos.x > -500 && pos.y > -500) {
    m_pi->m_settings.control_pos[m_ri->m_radar] = pos;
    LOG_DIALOG(wxT("%s saved position %d,%d"), m_log_name.c_str(), pos.x, pos.y);
  }
}

bool ControlsDialog::Create(wxWindow* parent, radar_pi* ppi, RadarInfo* ri, wxWindowID id, const wxString& caption,
                            const wxPoint& pos) {
  m_parent = parent;
  m_pi = ppi;
  m_ri = ri;

  m_log_name = wxString::Format(wxT("radar_pi: Radar %c ControlDialog:"), (char)(ri->m_radar + 'A'));

#ifdef __WXMSW__
  long wstyle = wxSYSTEM_MENU | wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN;
#endif
#ifdef __WXMAC__
  long wstyle = wxCLOSE_BOX | wxCAPTION;
  // long wstyle = wxCLOSE_BOX | wxCAPTION | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP;

  wstyle |= wxSTAY_ON_TOP;  // Radar AUI windows, when float, are already FLOAT_ON_PARENT and we don't seem to be on top of those.
  wstyle |= wxFRAME_FLOAT_ON_PARENT;  // Float on our parent
  wstyle |= wxFRAME_TOOL_WINDOW;      // This causes window to hide when OpenCPN is not activated, but this doesn't wo
#endif
#ifdef __WXGTK__
  long wstyle = wxCLOSE_BOX | wxCAPTION | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR;
#endif

  /*guard_zone_names[0] = _("Off");*/
  guard_zone_names[0] = _("Arc");
  guard_zone_names[1] = _("Circle");

  if (!wxDialog::Create(parent, id, caption, pos, wxDefaultSize, wstyle)) {
    return false;
  }

  CreateControls();
  return true;
}

void ControlsDialog::EnsureWindowNearOpenCPNWindow() {
#define PROXIMITY_MARGIN 32

  wxWindow* parent = m_pi->m_parent_window;
  while (parent->GetParent()) {
    parent = parent->GetParent();
  }
  wxPoint oPos = parent->GetScreenPosition();
  wxSize oSize = parent->GetSize();
  oSize.x += PROXIMITY_MARGIN;
  oSize.y += PROXIMITY_MARGIN;

  wxPoint mPos = GetPosition();
  wxSize mSize = GetSize();
  mSize.x += PROXIMITY_MARGIN;
  mSize.y += PROXIMITY_MARGIN;

  bool move = false;

  // LOG_DIALOG(wxT("%s control %d,%d is near OpenCPN at %d,%d to %d,%d?"), m_log_name.c_str(), mPos.x, mPos.y, oPos.x, oPos.y
  // , oPos.x + oSize.x, oPos.y + oSize.y);

  if (mPos.x + mSize.x < oPos.x) {
    mPos.x = oPos.x;
    move = true;
  }
  if (oPos.x + oSize.x < mPos.x) {
    mPos.x = oPos.x + oSize.x - mSize.x;
    move = true;
  }
  if (mPos.y + mSize.y < oPos.y) {
    mPos.y = oPos.y;
    move = true;
  }
  if (oPos.y + oSize.y < mPos.y) {
    mPos.y = oPos.y + oSize.y - mSize.y;
    move = true;
  }
  if (move) {
    LOG_DIALOG(wxT("%s Move control dialog to %d,%d to be near OpenCPN at %d,%d to %d,%d"), m_log_name.c_str(), mPos.x, mPos.y,
               oPos.x, oPos.y, oPos.x + oSize.x, oPos.y + oSize.y);
  }
  SetPosition(mPos);
}

void ControlsDialog::SetMenuAutoHideTimeout() {
  if (m_top_sizer->IsShown(m_control_sizer)) {
    switch (m_pi->m_settings.menu_auto_hide) {
      case 1:
        m_auto_hide_timeout = time(0) + 10;
        break;
      case 2:
        m_auto_hide_timeout = time(0) + 30;
        break;
      default:
        m_auto_hide_timeout = 0;
        break;
    }
  } else {
    m_auto_hide_timeout = 0;
  }
}

void ControlsDialog::ShowGuardZone(int zone) {
  double conversionFactor = RangeUnitsToMeters[m_pi->m_settings.range_units];

  m_guard_zone = m_ri->m_guard_zone[zone];

  wxString GuardZoneText;
  if (zone == 0) {
    GuardZoneText << _("Guard Zone 1 Green");
  }
  if (zone == 1) {
    GuardZoneText << _("Guard Zone 2 Blue");
  }
  m_guard_zone_text->SetLabel(GuardZoneText);

  m_guard_zone_type->SetSelection(m_guard_zone->m_type);
  m_inner_range->SetValue(wxString::Format(wxT("%2.2f"), m_guard_zone->m_inner_range / conversionFactor));
  m_outer_range->SetValue(wxString::Format(wxT("%2.2f"), m_guard_zone->m_outer_range / conversionFactor));

  AngleDegrees bearing = m_guard_zone->m_start_bearing;
  m_start_bearing->SetValue(wxString::Format(wxT("%d"), bearing));

  bearing = m_guard_zone->m_end_bearing;
  while (bearing >= 180.0) {
    bearing -= 360.;
  }
  bearing = round(bearing);
  m_end_bearing->SetValue(wxString::Format(wxT("%d"), bearing));
  m_alarm->SetValue(m_guard_zone->m_alarm_on ? 1 : 0);
  m_arpa_box->SetValue(m_guard_zone->m_arpa_on ? 1 : 0);
  m_guard_zone->m_show_time = time(0);

  SetGuardZoneVisibility();
  SwitchTo(m_guard_sizer, wxT("guard"));
}

void ControlsDialog::SetGuardZoneVisibility() {
  GuardZoneType zoneType = (GuardZoneType)m_guard_zone_type->GetSelection();
  m_guard_zone->SetType(zoneType);

  if (zoneType == GZ_CIRCLE) {
    m_start_bearing->Disable();
    m_end_bearing->Disable();
    m_inner_range->Enable();
    m_outer_range->Enable();

  } else {
    m_start_bearing->Enable();
    m_end_bearing->Enable();
    m_inner_range->Enable();
    m_outer_range->Enable();
  }
  m_guard_sizer->Layout();
}

void ControlsDialog::UpdateGuardZoneState() {
  wxString label1, label2, label3, label4;
  if (m_ri->m_guard_zone[0]->m_alarm_on) {
    label3 << _T(" + ") << _("Alarm");
  }
  if (m_ri->m_guard_zone[0]->m_arpa_on) {
    label3 << _T(" + ") << _("Arpa");
  }
  if (!m_ri->m_guard_zone[0]->m_alarm_on && !m_ri->m_guard_zone[0]->m_arpa_on) {
    label3 << _(" Off");
  }

  if (m_ri->m_guard_zone[1]->m_alarm_on) {
    label4 << _T(" + ") << _("Alarm");
  }
  if (m_ri->m_guard_zone[1]->m_arpa_on) {
    label4 << _T(" + ") << _("Arpa");
  }
  if (!m_ri->m_guard_zone[1]->m_alarm_on && !m_ri->m_guard_zone[1]->m_arpa_on) {
    label4 << _(" Off");
  }

  label1 << MENU(_("Guard zone") + wxT(" 1 Green")) + wxT("\n") + guard_zone_names[m_ri->m_guard_zone[0]->m_type] + label3;
  m_guard_1_button->SetLabel(label1);

  label2 << MENU(_("Guard zone") + wxT(" 2 Blue")) + wxT("\n") + guard_zone_names[m_ri->m_guard_zone[1]->m_type] + label4;
  m_guard_2_button->SetLabel(label2);
}

void ControlsDialog::SwitchTo(wxBoxSizer* to, const wxChar* name) {
  if (!m_top_sizer || !m_from_sizer || (to == m_current_sizer && m_top_sizer->IsShown(to))) {
    return;
  }
  if (m_current_sizer != to) {
    m_top_sizer->Hide(m_current_sizer);
  }
  m_top_sizer->Show(to);
  m_current_sizer = to;
  if (to != m_edit_sizer) {
    m_from_sizer = to;
  }
  LOG_VERBOSE(wxT("%s switch to control view %s"), m_log_name.c_str(), name);

  UpdateRadarSpecificState();
  UpdateGuardZoneState();
  SetMenuAutoHideTimeout();
  Resize(true);
}

wxSize g_buttonSize;

void ControlsDialog::CreateControls() {
  static int BORDER = 0;
  wxString backButtonStr;
  backButtonStr << MENU_BACK(_("Back"));

  // A top-level sizer
  m_top_sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_top_sizer);

  /*
   * Here be dragons...
   * Since I haven't been able to create buttons that adapt up, and at the same
   * time calculate the biggest button, and I want all buttons to be the same width I use a trick.
   * I generate a multiline StaticText containing all the (long) button labels and find out what the
   * width of that is, and then generate the buttons using that width.
   * I know, this is a hack, but this way it works relatively nicely even with translations.
   */

  wxBoxSizer* testBox = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(testBox, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  wxString label;
  label << _("Scan speed") << wxT("\n");
  label << _("Installation") << wxT("\n");
  label << _("Antenna height") << wxT("\n");
  label << _("Antenna forward of GPS") << wxT("\n");
  label << _("Antenna starboard of GPS") << wxT("\n");
  label << _("Local interference rej.") << wxT("\n");
  label << _("Guard zones") << wxT("\n");
  label << _("Zone type") << wxT("\n");
  label << _("Guard zones") << wxT("\n");
  label << _("Inner range") << wxT("\n");
  label << _("Outer range") << wxT("\n");
  label << _("Start bearing") << wxT("\n");
  label << _("End bearing") << wxT("\n");
  label << _("Clear cursor") << wxT("\n");
  label << _("Place EBL/VRM") << wxT("\n");
  label << _("Off/Relative/True trails") << wxT("\n");
  label << _("Clear trails") << wxT("\n");
  label << _("Orientation") << wxT("\n");
  label << _("Overlay transparency") << wxT("\n");
  label << _("Overlay") << wxT("\n");
  label << _("Adjust") << wxT("\n");
  label << _("Advanced") << wxT("\n");
  label << _("View") << wxT("\n");
  label << _("EBL/VRM") << wxT("\n");
  label << _("Timed Transmit") << wxT("\n");
  label << _("Info") << wxT("\n");

  for (int i = 0; i < CT_MAX; i++) {
    label << _(ControlTypeNames[i]) << wxT("\n");
  }

  wxStaticText* testMessage =
      new wxStaticText(this, ID_BPOS, label, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  testMessage->SetFont(m_pi->m_font);
  testBox->Add(testMessage, 0, wxALL, 2);

  wxStaticText* testButtonText =
      new wxStaticText(this, ID_BPOS, wxT("Button"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  testButtonText->SetFont(m_pi->m_font);
  testBox->Add(testButtonText, 0, wxALL, 2);

  wxStaticText* testButton2Text =
      new wxStaticText(this, ID_BPOS, wxT("Button\ndata"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  testButton2Text->SetFont(m_pi->m_font);
  testBox->Add(testButton2Text, 0, wxALL, 2);

  // Do a temporary layout, then ask the window system how wide the window is...
  m_top_sizer->Fit(this);
  m_top_sizer->Layout();
  int width = m_top_sizer->GetSize().GetWidth() + 20;

  wxSize bestSize = GetBestSize();
  if (width < bestSize.GetWidth()) {
    width = bestSize.GetWidth();
  }
  if (width < 100) {
    width = 100;
  }
  if (width > 500) {
    width = 500;
  }

#define BUTTON_BORDER 4
#ifdef __WXMAC__
#define BUTTON_HEIGTH_FUDGE 1 + BUTTON_BORDER
#else
#define BUTTON_HEIGTH_FUDGE 1 + 2 * BUTTON_BORDER
#endif

  g_buttonSize = wxSize(width, testButton2Text->GetBestSize().y * BUTTON_HEIGTH_FUDGE);
  LOG_DIALOG(wxT("%s Dynamic button width = %d height = %d"), m_log_name.c_str(), g_buttonSize.x, g_buttonSize.y);

  m_top_sizer->Hide(testBox);
  m_top_sizer->Remove(testBox);
  // delete testBox; -- this core dumps!
  // Determined desired button width

  //**************** EDIT BOX ******************//
  m_edit_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_edit_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The <<Back button
  RadarButton* back_button = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_edit_sizer->Add(back_button, 0, wxALL, BORDER);

  // The +10 button
  m_plus_ten_button = new RadarButton(this, ID_PLUS_TEN, g_buttonSize, wxT("+10"));
  m_edit_sizer->Add(m_plus_ten_button, 0, wxALL, BORDER);

  // The + button
  m_plus_button = new RadarButton(this, ID_PLUS, g_buttonSize, wxT("+"));
  m_edit_sizer->Add(m_plus_button, 0, wxALL, BORDER);

  // The VALUE text
  wxSize valueSize = wxSize(g_buttonSize.x, g_buttonSize.y + 10);
  m_value_text = new wxStaticText(this, ID_VALUE, _("Value"), wxDefaultPosition, valueSize, wxALIGN_CENTRE_HORIZONTAL);
  m_edit_sizer->Add(m_value_text, 0, wxALL, BORDER);
  m_value_text->SetFont(m_pi->m_fat_font);
  m_value_text->SetBackgroundColour(*wxLIGHT_GREY);

  // The COMMENT text
  m_comment_text = new DynamicStaticText(this, ID_VALUE, wxT(""), wxDefaultPosition, g_buttonSize, wxALIGN_CENTRE_HORIZONTAL);
  m_edit_sizer->Add(m_comment_text, 0, wxALL, BORDER);
  m_comment_text->SetBackgroundColour(*wxLIGHT_GREY);

  // The - button
  m_minus_button = new RadarButton(this, ID_MINUS, g_buttonSize, wxT("-"));
  m_edit_sizer->Add(m_minus_button, 0, wxALL, BORDER);

  // The -10 button
  m_minus_ten_button = new RadarButton(this, ID_MINUS_TEN, g_buttonSize, wxT("-10"));
  m_edit_sizer->Add(m_minus_ten_button, 0, wxALL, BORDER);

  // The Auto button
  m_auto_button = new RadarButton(this, ID_AUTO, g_buttonSize, _("Auto"));
  m_edit_sizer->Add(m_auto_button, 0, wxALL, BORDER);

  // The Off button
  m_off_button = new RadarButton(this, ID_OFF, g_buttonSize, _("Off"));
  m_edit_sizer->Add(m_off_button, 0, wxALL, BORDER);

  m_top_sizer->Hide(m_edit_sizer);

  //**************** ADVANCED BOX ******************//
  // These are the controls that the users sees when the Advanced button is selected

  m_advanced_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_advanced_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The Back button
  RadarButton* bAdvancedBack = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_advanced_sizer->Add(bAdvancedBack, 0, wxALL, BORDER);

  if (m_ctrl[CT_NOISE_REJECTION].type) {
    // The NOISE REJECTION button
    m_noise_rejection_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Noise rejection"), m_ctrl[CT_NOISE_REJECTION], &m_ri->m_noise_rejection);
    m_advanced_sizer->Add(m_noise_rejection_button, 0, wxALL, BORDER);
  }

  // The TARGET EXPANSION button
  if (m_ctrl[CT_TARGET_EXPANSION].type) {
    m_target_expansion_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Target expansion"), m_ctrl[CT_TARGET_EXPANSION],
                                                       &m_ri->m_target_expansion);
    m_advanced_sizer->Add(m_target_expansion_button, 0, wxALL, BORDER);
  }

  // The REJECTION button

  if (m_ctrl[CT_INTERFERENCE_REJECTION].type) {
    m_interference_rejection_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Interference rejection"),
                                                             m_ctrl[CT_INTERFERENCE_REJECTION], &m_ri->m_interference_rejection);
    m_advanced_sizer->Add(m_interference_rejection_button, 0, wxALL, BORDER);
  }

  // The TARGET SEPARATION button
  if (m_ctrl[CT_TARGET_SEPARATION].type) {
    m_target_separation_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Target separation"),
                                                        m_ctrl[CT_TARGET_SEPARATION], &m_ri->m_target_separation);
    m_advanced_sizer->Add(m_target_separation_button, 0, wxALL, BORDER);
  }

  // The SCAN SPEED button
  if (m_ctrl[CT_SCAN_SPEED].type) {
    m_scan_speed_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Scan speed"), m_ctrl[CT_SCAN_SPEED], &m_ri->m_scan_speed);
    m_advanced_sizer->Add(m_scan_speed_button, 0, wxALL, BORDER);
  }

  // The TARGET BOOST button
  if (m_ctrl[CT_TARGET_BOOST].type) {
    m_target_boost_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Target boost"), m_ctrl[CT_TARGET_BOOST], &m_ri->m_target_boost);
    m_advanced_sizer->Add(m_target_boost_button, 0, wxALL, BORDER);
  }

  // The INSTALLATION button
  RadarButton* bInstallation = new RadarButton(this, ID_INSTALLATION, g_buttonSize, MENU(_("Installation")));
  m_advanced_sizer->Add(bInstallation, 0, wxALL, BORDER);

  // The PREFERENCES button
  RadarButton* bPreferences = new RadarButton(this, ID_PREFERENCES, g_buttonSize, MENU_WINDOW(_("Preferences")));
  m_advanced_sizer->Add(bPreferences, 0, wxALL, BORDER);

  m_top_sizer->Hide(m_advanced_sizer);

  //**************** Installation BOX ******************//
  // These are the controls that the users sees when the Installation button is selected

  m_installation_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_installation_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The Back button
  RadarButton* bInstallationBack = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_installation_sizer->Add(bInstallationBack, 0, wxALL, BORDER);

  // The BEARING ALIGNMENT button
  if (m_ctrl[CT_BEARING_ALIGNMENT].type) {
    m_bearing_alignment_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Bearing alignment"),
                                                        m_ctrl[CT_BEARING_ALIGNMENT], &m_ri->m_bearing_alignment);
    m_installation_sizer->Add(m_bearing_alignment_button, 0, wxALL, BORDER);
  }

  // The NO TRANSMIT START button
  if (m_ctrl[CT_NO_TRANSMIT_START].type) {
    m_no_transmit_start_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("No transmit start"),
                                                        m_ctrl[CT_NO_TRANSMIT_START], &m_ri->m_no_transmit_start);
    m_installation_sizer->Add(m_no_transmit_start_button, 0, wxALL, BORDER);
  }

  // The NO TRANSMIT END button
  if (m_ctrl[CT_NO_TRANSMIT_END].type) {
    m_no_transmit_end_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("No transmit end"), m_ctrl[CT_NO_TRANSMIT_END], &m_ri->m_no_transmit_end);
    m_installation_sizer->Add(m_no_transmit_end_button, 0, wxALL, BORDER);
  }

  // The ANTENNA HEIGHT button
  if (m_ctrl[CT_ANTENNA_HEIGHT].type) {
    m_antenna_height_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Antenna height"), m_ctrl[CT_ANTENNA_HEIGHT], &m_ri->m_antenna_height);
    m_installation_sizer->Add(m_antenna_height_button, 0, wxALL, BORDER);
  }

  // The ANTENNA FORWARD button
  if (m_ctrl[CT_ANTENNA_FORWARD].type) {
    m_antenna_forward_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Antenna forward"), m_ctrl[CT_ANTENNA_FORWARD], &m_ri->m_antenna_forward,
                               _("m"), _("relative to GPS") + wxT("\n") + _("negative = behind"));
    m_installation_sizer->Add(m_antenna_forward_button, 0, wxALL, BORDER);
  }

  // The ANTENNA STARBOARD button
  if (m_ctrl[CT_ANTENNA_STARBOARD].type) {
    m_antenna_starboard_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Antenna starboard"), m_ctrl[CT_ANTENNA_STARBOARD],
                               &m_ri->m_antenna_starboard, _("m"), _("relative to GPS") + wxT("\n") + _("negative = port"));
    m_installation_sizer->Add(m_antenna_starboard_button, 0, wxALL, BORDER);
  }

  // The LOCAL INTERFERENCE REJECTION button
  if (m_ctrl[CT_LOCAL_INTERFERENCE_REJECTION].type) {
    m_local_interference_rejection_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Local interference rej."), m_ctrl[CT_LOCAL_INTERFERENCE_REJECTION],
                               &m_ri->m_local_interference_rejection);
    m_installation_sizer->Add(m_local_interference_rejection_button, 0, wxALL, BORDER);
  }

  // The SIDE LOBE SUPPRESSION button
  if (m_ctrl[CT_SIDE_LOBE_SUPPRESSION].type) {
    m_side_lobe_suppression_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Side lobe suppression"),
                                                            m_ctrl[CT_SIDE_LOBE_SUPPRESSION], &m_ri->m_side_lobe_suppression);
    m_installation_sizer->Add(m_side_lobe_suppression_button, 0, wxALL, BORDER);
  }

  // The MAIN BANG SIZE button
  if (m_ctrl[CT_MAIN_BANG_SIZE].type) {
    m_main_bang_size_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Main bang size"), m_ctrl[CT_MAIN_BANG_SIZE],
                                                     &m_ri->m_main_bang_size, _("pixels"));
    m_installation_sizer->Add(m_main_bang_size_button, 0, wxALL, BORDER);
  }

  m_top_sizer->Hide(m_installation_sizer);

  //***************** GUARD ZONE EDIT BOX *************//

  m_guard_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_guard_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The <<Back button
  RadarButton* guard_back_button = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_guard_sizer->Add(guard_back_button, 0, wxALL, BORDER);

  m_guard_zone_text = new wxStaticText(this, wxID_ANY, _("Guard zones"));
  m_guard_sizer->Add(m_guard_zone_text, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

  wxStaticText* type_Text = new wxStaticText(this, wxID_ANY, _("Zone type"), wxDefaultPosition, wxDefaultSize, 0);
  m_guard_sizer->Add(type_Text, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  m_guard_zone_type = new wxRadioBox(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, ARRAY_SIZE(guard_zone_names),
                                     guard_zone_names, 1, wxRA_SPECIFY_COLS);
  m_guard_sizer->Add(m_guard_zone_type, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  m_guard_zone_type->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(ControlsDialog::OnGuardZoneModeClick), NULL,
                             this);

  // Inner and Outer Ranges
  wxStaticText* inner_range_Text = new wxStaticText(this, wxID_ANY, _("Inner range"), wxDefaultPosition, wxDefaultSize, 0);
  m_guard_sizer->Add(inner_range_Text, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  m_inner_range = new wxTextCtrl(this, wxID_ANY);
  m_guard_sizer->Add(m_inner_range, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);
  m_inner_range->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ControlsDialog::OnInner_Range_Value), NULL, this);
  ///   start of copy
  wxStaticText* outer_range_Text = new wxStaticText(this, wxID_ANY, _("Outer range"), wxDefaultPosition, wxDefaultSize, 0);
  m_guard_sizer->Add(outer_range_Text, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0);

  m_outer_range = new wxTextCtrl(this, wxID_ANY);
  m_guard_sizer->Add(m_outer_range, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);
  m_outer_range->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ControlsDialog::OnOuter_Range_Value), NULL, this);

  wxStaticText* pStart_Bearing = new wxStaticText(this, wxID_ANY, _("Start bearing"), wxDefaultPosition, wxDefaultSize, 0);
  m_guard_sizer->Add(pStart_Bearing, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0);

  m_start_bearing = new wxTextCtrl(this, wxID_ANY);
  m_guard_sizer->Add(m_start_bearing, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);
  m_start_bearing->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ControlsDialog::OnStart_Bearing_Value), NULL, this);

  wxStaticText* pEnd_Bearing = new wxStaticText(this, wxID_ANY, _("End bearing"), wxDefaultPosition, wxDefaultSize, 0);
  m_guard_sizer->Add(pEnd_Bearing, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0);

  m_end_bearing = new wxTextCtrl(this, wxID_ANY);
  m_guard_sizer->Add(m_end_bearing, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);
  m_end_bearing->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ControlsDialog::OnEnd_Bearing_Value), NULL, this);

  // checkbox for ARPA
  m_arpa_box = new wxCheckBox(this, wxID_ANY, _("ARPA On"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxST_NO_AUTORESIZE);
  m_guard_sizer->Add(m_arpa_box, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  m_arpa_box->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(ControlsDialog::OnARPAClick), NULL, this);

  // checkbox for blob alarm
  m_alarm = new wxCheckBox(this, wxID_ANY, _("Alarm On"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxST_NO_AUTORESIZE);
  m_guard_sizer->Add(m_alarm, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  m_alarm->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(ControlsDialog::OnAlarmClick), NULL, this);

  m_top_sizer->Hide(m_guard_sizer);

  //**************** ADJUST BOX ******************//

  m_adjust_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_adjust_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The Back button
  RadarButton* bAdjustBack = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_adjust_sizer->Add(bAdjustBack, 0, wxALL, BORDER);

  // The RANGE button
  if (m_ctrl[CT_RANGE].type) {
    m_range_button = new RadarRangeControlButton(this, ID_CONTROL_BUTTON, g_buttonSize, _("Range"), &m_ri->m_range);
    m_adjust_sizer->Add(m_range_button, 0, wxALL, BORDER);
  }

  // The GAIN button
  if (m_ctrl[CT_GAIN].type) {
    m_gain_button = new RadarControlButton(this, ID_GAIN, _("Gain"), m_ctrl[CT_GAIN], &m_ri->m_gain);
    m_adjust_sizer->Add(m_gain_button, 0, wxALL, BORDER);
  }

  // The SEA button
  if (m_ctrl[CT_SEA].type) {
    m_sea_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Sea clutter"), m_ctrl[CT_SEA], &m_ri->m_sea);
    m_adjust_sizer->Add(m_sea_button, 0, wxALL, BORDER);
  }

  // The RAIN button
  if (m_ctrl[CT_RAIN].type) {
    m_rain_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Rain clutter"), m_ctrl[CT_RAIN], &m_ri->m_rain);
    m_adjust_sizer->Add(m_rain_button, 0, wxALL, BORDER);
  }

  // The FTC button
  if (m_ctrl[CT_FTC].type) {
    m_ftc_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("FTC"), m_ctrl[CT_FTC], &m_ri->m_ftc);
    m_adjust_sizer->Add(m_ftc_button, 0, wxALL, BORDER);
  }

  m_top_sizer->Hide(m_adjust_sizer);

  //**************** CURSOR BOX ******************//

  m_cursor_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_cursor_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The Back button
  RadarButton* bCursorBack = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_cursor_sizer->Add(bCursorBack, 0, wxALL, BORDER);

  // The CLEAR CURSOR button
  m_clear_cursor = new RadarButton(this, ID_CLEAR_CURSOR, g_buttonSize, _("Clear cursor"));
  m_cursor_sizer->Add(m_clear_cursor, 0, wxALL, BORDER);

  // The ACQUIRE TARGET button
  m_acquire_target = new RadarButton(this, ID_ACQUIRE_TARGET, g_buttonSize, _("Acquire Target"));
  m_cursor_sizer->Add(m_acquire_target, 0, wxALL, BORDER);

  // The DELETE TARGET button
  m_delete_target = new RadarButton(this, ID_DELETE_TARGET, g_buttonSize, _("Delete target"));
  m_cursor_sizer->Add(m_delete_target, 0, wxALL, BORDER);

  // The DELETE ALL button
  m_delete_all = new RadarButton(this, ID_DELETE_ALL_TARGETS, g_buttonSize, _("Delete all targets"));
  m_cursor_sizer->Add(m_delete_all, 0, wxALL, BORDER);

  for (int b = 0; b < BEARING_LINES; b++) {
    // The BEARING button
    wxString label = _("Place EBL/VRM");
    label << wxString::Format(wxT("%d"), b + 1);
    m_bearing_buttons[b] = new RadarButton(this, ID_BEARING_SET + b, g_buttonSize, label);
    m_cursor_sizer->Add(m_bearing_buttons[b], 0, wxALL, BORDER);
    m_bearing_buttons[b]->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ControlsDialog::OnBearingSetButtonClick), 0,
                                  this);
  }

  m_top_sizer->Hide(m_cursor_sizer);

  //**************** WINDOW BOX ****************//

  m_window_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_window_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The Back button
  RadarButton* bWindowBack = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_window_sizer->Add(bWindowBack, 0, wxALL, BORDER);

  // The SHOW PPI button
  m_show_ppi_button = new RadarButton(this, ID_SHOW_RADAR_PPI, g_buttonSize, _T(""));
  m_window_sizer->Add(m_show_ppi_button, 0, wxALL, BORDER);

  // The DOCK PPI button
  m_dock_ppi_button = new RadarButton(this, ID_DOCK_RADAR_PPI, g_buttonSize, _T(""));
  m_window_sizer->Add(m_dock_ppi_button, 0, wxALL, BORDER);

  // The RADAR ONLY / OVERLAY canvas 0..n buttons
  for (int i = 0; i < MAX_CHART_CANVAS; i++) {
    wxString name;
    name << _("Overlay ") << (i + 1);
    m_overlay_button[i] =
        new RadarControlButton(this, ID_RADAR_OVERLAY0 + i, name, m_ctrl[CT_OVERLAY_CANVAS], &m_ri->m_overlay_canvas[i]);
    m_window_sizer->Add(m_overlay_button[i], 0, wxALL, BORDER);
    m_overlay_button[i]->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ControlsDialog::OnRadarOverlayButtonClick), 0,
                                 this);
  }

  // The TRANSPARENCY button
  m_transparency_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Overlay transparency"), m_ctrl[CT_TRANSPARENCY],
                                                 &m_pi->m_settings.overlay_transparency);
  m_window_sizer->Add(m_transparency_button, 0, wxALL, BORDER);

  // The INFO button
  RadarButton* bMessage = new RadarButton(this, ID_MESSAGE, g_buttonSize, MENU_WINDOW(_("Info")));
  m_window_sizer->Add(bMessage, 0, wxALL, BORDER);

  m_top_sizer->Hide(m_window_sizer);

  //**************** VIEW BOX ******************//
  // These are the controls that the users sees when the View button is selected

  m_view_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_view_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The Back button
  RadarButton* bMenuBack = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_view_sizer->Add(bMenuBack, 0, wxALL, BORDER);

  // The SHOW / HIDE AIS/ARPA ON PPI button
  m_targets_on_ppi_button =
      new RadarControlButton(this, ID_TARGETS_ON_PPI, _("AIS/ARPA on PPI"), m_ctrl[CT_TARGET_ON_PPI], &m_ri->m_target_on_ppi);
  m_view_sizer->Add(m_targets_on_ppi_button, 0, wxALL, BORDER);

  // The DOPPLER button
  if (m_ctrl[CT_DOPPLER].type) {
    m_doppler_button = new RadarControlButton(this, ID_CONTROL_BUTTON, _("Doppler"), m_ctrl[CT_DOPPLER], &m_ri->m_doppler);
    m_view_sizer->Add(m_doppler_button, 0, wxALL, BORDER);
  }

  // The TARGET_TRAIL button
  if (m_ctrl[CT_TARGET_TRAILS].type) {
    m_target_trails_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Target trails"), m_ctrl[CT_TARGET_TRAILS], &m_ri->m_target_trails);
    m_view_sizer->Add(m_target_trails_button, 0, wxALL, BORDER);
    m_target_trails_button->Hide();
  }

  // The Trails Motion button
  if (m_ctrl[CT_TRAILS_MOTION].type) {
    m_trails_motion_button = new RadarControlButton(this, ID_CONTROL_BUTTON, MENU_NO_EDIT(_("Trails motion")),
                                                    m_ctrl[CT_TRAILS_MOTION], &m_ri->m_trails_motion);
    m_view_sizer->Add(m_trails_motion_button, 0, wxALL, BORDER);
  }

  // The Clear Trails button
  m_clear_trails_button = new RadarButton(this, ID_CLEAR_TRAILS, g_buttonSize, _("Clear trails"));
  m_view_sizer->Add(m_clear_trails_button, 0, wxALL, BORDER);
  m_clear_trails_button->Hide();

  // The Rotation button
  m_orientation_button =
      new RadarControlButton(this, ID_ORIENTATION, _("Orientation"), m_ctrl[CT_ORIENTATION], &m_ri->m_orientation);
  m_view_sizer->Add(m_orientation_button, 0, wxALL, BORDER);
  // Updated when we receive data

  // The Center Forward Aft button
  m_view_center_button =
      new RadarControlButton(this, ID_VIEW_CENTER, _("PPI Forward Aft View"), m_ctrl[CT_CENTER_VIEW], &m_ri->m_view_center);
  m_view_sizer->Add(m_view_center_button, 0, wxALL, BORDER);

  // The REFRESHRATE button
  if (m_ctrl[CT_REFRESHRATE].type) {
    m_refresh_rate_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Refresh rate"), m_ctrl[CT_REFRESHRATE], &m_pi->m_settings.refreshrate);
    m_view_sizer->Add(m_refresh_rate_button, 0, wxALL, BORDER);
  }

  m_top_sizer->Hide(m_view_sizer);

  //***************** POWER BOX *************//

  if (m_ctrl[CT_TIMED_IDLE].type) {
    m_power_sizer = new wxBoxSizer(wxVERTICAL);
    m_top_sizer->Add(m_power_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

    // The <<Back button
    RadarButton* power_back_button = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
    m_power_sizer->Add(power_back_button, 0, wxALL, BORDER);

    m_power_sub_button = new RadarButton(this, ID_TRANSMIT_STANDBY, g_buttonSize, wxT(""));
    m_power_sizer->Add(m_power_sub_button, 0, wxALL, BORDER);

    // The TIMED STANDBY button
    m_timed_idle_button =
        new RadarControlButton(this, ID_CONTROL_BUTTON, _("Timed Standby"), m_ctrl[CT_TIMED_IDLE], &m_ri->m_timed_idle);
    m_power_sizer->Add(m_timed_idle_button, 0, wxALL, BORDER);

    // The TIMED TRANSMIT button
    if (m_ctrl[CT_TIMED_RUN].type) {
      m_timed_run_button =
          new RadarControlButton(this, ID_CONTROL_BUTTON, _("Timed Transmit"), m_ctrl[CT_TIMED_RUN], &m_ri->m_timed_run);
      m_power_sizer->Add(m_timed_run_button, 0, wxALL, BORDER);
    }

    m_top_sizer->Hide(m_power_sizer);
  }

  //***************** GUARD ZONE BOX *************//

  m_guardzone_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_guardzone_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The <<Back button
  RadarButton* bGuardZoneBack = new RadarButton(this, ID_BACK, g_buttonSize, backButtonStr);
  m_guardzone_sizer->Add(bGuardZoneBack, 0, wxALL, BORDER);

  // The GUARD ZONE 1 button
  m_guard_1_button = new RadarButton(this, ID_ZONE1, g_buttonSize, wxT(""));
  m_guardzone_sizer->Add(m_guard_1_button, 0, wxALL, BORDER);

  // The GUARD ZONE 2 button
  m_guard_2_button = new RadarButton(this, ID_ZONE2, g_buttonSize, wxT(""));
  m_guardzone_sizer->Add(m_guard_2_button, 0, wxALL, BORDER);

  m_top_sizer->Hide(m_guardzone_sizer);

  //**************** CONTROL BOX ******************//
  // These are the controls that the users sees when the dialog is started

  // A box sizer to contain RANGE, GAIN etc button
  m_control_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_control_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

#ifdef STATUS_ON_MENU
  //**************** STATUS TEXT ***************//
  m_status_text = new wxStaticText(this, ID_STATUS_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
  wxFont f = m_status_text->GetFont();
  f.SetPointSize(f.GetPointSize() - 1);
  m_status_text->SetFont(f);
  m_control_sizer->Add(m_status_text, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);
#endif

  //***************** TRANSMIT SIZER, items hidden when not transmitting ****************//

  m_transmit_sizer = new wxBoxSizer(wxVERTICAL);
  m_control_sizer->Add(m_transmit_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  // The ADJUST button
  m_adjust_button = new RadarButton(this, ID_ADJUST, g_buttonSize, MENU(_("Adjust")));
  m_transmit_sizer->Add(m_adjust_button, 0, wxALL, BORDER);

  // The ADVANCED button
  RadarButton* bAdvanced = new RadarButton(this, ID_ADVANCED, g_buttonSize, MENU(_("Advanced")));
  m_transmit_sizer->Add(bAdvanced, 0, wxALL, BORDER);

  // The VIEW menu
  RadarButton* bView = new RadarButton(this, ID_VIEW, g_buttonSize, MENU(_("View")));
  m_transmit_sizer->Add(bView, 0, wxALL, BORDER);

  // The CURSOR button
  m_cursor_menu = new RadarButton(this, ID_BEARING, g_buttonSize, MENU(_("Cursor")));
  m_transmit_sizer->Add(m_cursor_menu, 0, wxALL, BORDER);

  // The GUARDZONE menu
  RadarButton* bGuardZone = new RadarButton(this, ID_GUARDZONE, g_buttonSize, MENU(_("Guard zones")));
  m_control_sizer->Add(bGuardZone, 0, wxALL, BORDER);

  // The WINDOW menu
  RadarButton* bWindow = new RadarButton(this, ID_WINDOW, g_buttonSize, MENU(_("Window")));
  m_control_sizer->Add(bWindow, 0, wxALL, BORDER);

  if (m_ctrl[CT_TIMED_IDLE].type) {
    // The Timer menu
    RadarButton* bTimedTransmit = new RadarButton(this, ID_POWER, g_buttonSize, MENU(_("Timed transmit")));
    m_control_sizer->Add(bTimedTransmit, 0, wxALL, BORDER);
  }

  // The Transmit button
  m_power_button = new RadarButton(this, ID_TRANSMIT_STANDBY, g_buttonSize, _("Unknown"));
  m_control_sizer->Add(m_power_button, 0, wxALL, BORDER);
  // Updated when we receive data

  // Switch on the main control sizer (only)
  m_from_sizer = m_control_sizer;
  m_current_sizer = m_control_sizer;
  m_control_sizer->Hide(m_transmit_sizer);
  m_top_sizer->Show(m_control_sizer);

  UpdateGuardZoneState();
  Resize(true);
}

void ControlsDialog::OnZone1ButtonClick(wxCommandEvent& event) { ShowGuardZone(0); }

void ControlsDialog::OnZone2ButtonClick(wxCommandEvent& event) { ShowGuardZone(1); }

void ControlsDialog::OnClose(wxCloseEvent& event) { m_pi->OnControlDialogClose(m_ri); }

void ControlsDialog::OnIdOKClick(wxCommandEvent& event) { m_pi->OnControlDialogClose(m_ri); }

void ControlsDialog::OnPlusTenClick(wxCommandEvent& event) {
  m_from_control->AdjustValue(+10);
  m_auto_button->Enable();
  m_off_button->Enable();

  wxString label = m_from_control->GetLabel();
  m_value_text->SetLabel(label);
}

void ControlsDialog::OnPlusClick(wxCommandEvent& event) {
  m_from_control->AdjustValue(+1);
  m_auto_button->Enable();
  m_off_button->Enable();

  wxString label = m_from_control->GetLabel();
  m_value_text->SetLabel(label);
}

void ControlsDialog::OnBackClick(wxCommandEvent& event) {
  if (m_current_sizer == m_edit_sizer) {
    SwitchTo(m_from_sizer, wxT("from (back click)"));
    m_from_control = 0;
  } else if (m_current_sizer == m_installation_sizer) {
    SwitchTo(m_advanced_sizer, wxT("advanced (back click)"));
  } else {
    SwitchTo(m_control_sizer, wxT("main (back click)"));
  }
}

void ControlsDialog::OnAutoClick(wxCommandEvent& event) {
  if (m_from_control->ToggleState()) {
    m_auto_button->Enable();
  } else {
    m_auto_button->Disable();
  }
  m_off_button->Enable();
}

void ControlsDialog::OnOffClick(wxCommandEvent& event) {
  m_from_control->SetState(RCS_OFF);
  m_auto_button->Enable();
  m_off_button->Disable();
}

void ControlsDialog::OnMinusClick(wxCommandEvent& event) {
  m_from_control->AdjustValue(-1);
  m_auto_button->Enable();
  m_off_button->Enable();

  wxString label = m_from_control->GetLabel();
  m_value_text->SetLabel(label);
}

void ControlsDialog::OnMinusTenClick(wxCommandEvent& event) {
  m_from_control->AdjustValue(-10);
  m_auto_button->Enable();
  m_off_button->Enable();

  wxString label = m_from_control->GetLabel();
  m_value_text->SetLabel(label);
}

void ControlsDialog::OnAdjustButtonClick(wxCommandEvent& event) { SwitchTo(m_adjust_sizer, wxT("adjust")); }

void ControlsDialog::OnAdvancedButtonClick(wxCommandEvent& event) { SwitchTo(m_advanced_sizer, wxT("advanced")); }

void ControlsDialog::OnGuardZoneButtonClick(wxCommandEvent& event) { SwitchTo(m_guardzone_sizer, wxT("guard zone")); }

void ControlsDialog::OnWindowButtonClick(wxCommandEvent& event) { SwitchTo(m_window_sizer, wxT("window")); }

void ControlsDialog::OnViewButtonClick(wxCommandEvent& event) { SwitchTo(m_view_sizer, wxT("view")); }

void ControlsDialog::OnInstallationButtonClick(wxCommandEvent& event) { SwitchTo(m_installation_sizer, wxT("installation")); }

void ControlsDialog::OnPowerButtonClick(wxCommandEvent& event) { SwitchTo(m_power_sizer, wxT("power")); }

void ControlsDialog::OnPreferencesButtonClick(wxCommandEvent& event) { m_pi->ShowPreferencesDialog(m_pi->m_parent_window); }

void ControlsDialog::OnBearingButtonClick(wxCommandEvent& event) { SwitchTo(m_cursor_sizer, wxT("bearing")); }

void ControlsDialog::OnMessageButtonClick(wxCommandEvent& event) {
  SetMenuAutoHideTimeout();

  if (m_pi->m_pMessageBox) {
    m_pi->m_pMessageBox->UpdateMessage(true);
  }
}

void ControlsDialog::OnTargetsOnPPIButtonClick(wxCommandEvent& event) {
  m_ri->m_target_on_ppi.Update(1 - m_ri->m_target_on_ppi.GetValue());
  UpdateControlValues(false);
}

void ControlsDialog::EnterEditMode(RadarControlButton* button) {
  wxString label1, label2, label3, label4;

  m_from_control = button;  // Keep a record of which button was clicked
  m_value_text->SetLabel(button->GetLabel());
  SwitchTo(m_edit_sizer, wxT("edit"));

  if (button->m_comment.length() > 0) {
    m_comment_text->SetLabel(button->m_comment);
    m_comment_text->Show();
  } else {
    m_comment_text->Hide();
  }

  RadarControlState state = m_from_control->m_item->GetState();

  if (m_from_control->m_ci.hasOff) {
    m_off_button->Show();
    if (state == RCS_OFF) {
      m_off_button->Disable();
    } else {
      m_off_button->Enable();
    }
  } else {
    m_off_button->Hide();
  }

  bool hasAuto = m_from_control->m_ci.autoValues > 0;

  if (m_from_control->m_ci.type == CT_RANGE) {  // Range only allows auto if overlay is on
    hasAuto = false;
    for (int i = 0; i < CANVAS_COUNT; i++) {
      if (m_ri->m_overlay_canvas[i].GetValue() > 0) {
        hasAuto = true;
        break;
      }
    }
  }

  if (m_from_control->m_ci.unit.length() > 0) {
    label1 << wxT("+") << m_from_control->m_ci.stepValue << wxT(" ") << m_from_control->m_ci.unit;
    label2 << wxT("-") << m_from_control->m_ci.stepValue << wxT(" ") << m_from_control->m_ci.unit;
    label3 << wxT("+") << (10 * m_from_control->m_ci.stepValue) << wxT(" ") << m_from_control->m_ci.unit;
    label4 << wxT("-") << (10 * m_from_control->m_ci.stepValue) << wxT(" ") << m_from_control->m_ci.unit;
  } else {
    if (m_from_control->m_ci.stepValue > 1) {
      label1 << wxT("+") << m_from_control->m_ci.stepValue;
      label2 << wxT("-") << m_from_control->m_ci.stepValue;
    } else {
      label1 << wxT("+");
      label2 << wxT("-");
    }
    label3 << wxT("+") << (10 * m_from_control->m_ci.stepValue);
    label4 << wxT("-") << (10 * m_from_control->m_ci.stepValue);
  }

  m_plus_button->SetLabel(label1);
  m_minus_button->SetLabel(label2);
  m_plus_ten_button->SetLabel(label3);
  m_minus_ten_button->SetLabel(label4);

  if (hasAuto) {
    m_auto_button->Show();
    if (state > RCS_MANUAL && m_from_control->m_ci.autoValues == 1) {
      m_auto_button->Disable();
    } else {
      m_auto_button->Enable();
    }
  } else {
    m_auto_button->Hide();
  }
  if (m_from_control->m_ci.maxValue > 20) {
    m_plus_ten_button->Show();
    m_minus_ten_button->Show();
  } else {
    m_plus_ten_button->Hide();
    m_minus_ten_button->Hide();
  }

  m_edit_sizer->Layout();
  m_control_sizer->Layout();
  m_top_sizer->Layout();
  Fit();
  LOG_VERBOSE(wxT("EnterEditMode fit"));
}

void ControlsDialog::OnRadarControlButtonClick(wxCommandEvent& event) {
  RadarControlButton* button = (RadarControlButton*)event.GetEventObject();

  if (!button->ToggleValue()) {
    EnterEditMode(button);
  }
}

void ControlsDialog::OnRadarDockPPIButtonClick(wxCommandEvent& event) {
  m_pi->m_settings.dock_radar[m_ri->m_radar] = !m_pi->m_settings.dock_radar[m_ri->m_radar];

  wxAuiPaneInfo& pane = m_ri->m_radar_panel->m_aui_mgr->GetPane(m_ri->m_radar_panel);
  if (m_pi->m_settings.dock_radar[m_ri->m_radar]) {  // dock PPI
    pane.dock_layer = 1;
    pane.Dockable(true).CaptionVisible().Right().Dock();
    m_ri->m_radar_panel->m_aui_mgr->Update();
    // and restore dock size if available
    if (m_pi->m_settings.dock_size > 0) {
      m_ri->m_radar_panel->m_dock =
          wxString::Format(wxT("|dock_size(%d,%d,%d)="), pane.dock_direction, pane.dock_layer, pane.dock_row);
      wxString perspective = m_ri->m_radar_panel->m_aui_mgr->SavePerspective();
      int p = perspective.Find(m_ri->m_radar_panel->m_dock);
      if (p != wxNOT_FOUND) {
        wxString newPerspective = perspective.Left(p);
        newPerspective << m_ri->m_radar_panel->m_dock;
        newPerspective << m_pi->m_settings.dock_size;
        perspective = perspective.Mid(p + m_ri->m_radar_panel->m_dock.length());
        newPerspective << wxT("|");
        newPerspective << perspective.AfterFirst(wxT('|'));
        m_ri->m_radar_panel->m_aui_mgr->LoadPerspective(newPerspective);
        LOG_DIALOG(wxT("radar_pi: %s: new perspective %s"), m_ri->m_name.c_str(), newPerspective.c_str());
      }
    }
    m_ri->m_radar_panel->m_aui_mgr->Update();

  } else {  // float PPI
    // first save dock size with Kees's hack
    m_ri->m_radar_panel->m_dock =
        wxString::Format(wxT("|dock_size(%d,%d,%d)="), pane.dock_direction, pane.dock_layer, pane.dock_row);
    wxString perspective = m_ri->m_radar_panel->m_aui_mgr->SavePerspective();
    int p = perspective.Find(m_ri->m_radar_panel->m_dock);
    if (p != wxNOT_FOUND) {
      perspective = perspective.Mid(p + m_ri->m_radar_panel->m_dock.length());
      perspective = perspective.BeforeFirst(wxT('|'));
      m_pi->m_settings.dock_size = wxAtoi(perspective);
      LOG_DIALOG(wxT("radar_pi: %s: replaced=%s, saved dock_size = %d"), m_ri->m_name.c_str(), perspective.c_str(),
                 m_pi->m_settings.dock_size);
    }

    wxAuiPaneInfo& pane = m_ri->m_radar_panel->m_aui_mgr->GetPane(m_ri->m_radar_panel);
    pane.Dockable(false).Movable(true).CloseButton().CaptionVisible().Float();
    m_ri->m_radar_panel->m_aui_mgr->Update();
  }
  m_ri->m_radar_panel->ShowFrame(true);
  m_pi->m_settings.show_radar[m_ri->m_radar] = 1;
}

void ControlsDialog::OnRadarShowPPIButtonClick(wxCommandEvent& event) {
  SetMenuAutoHideTimeout();
  bool show = true;
  if (M_SETTINGS.radar_count > 0) {
    m_pi->m_settings.show_radar[m_ri->m_radar] = !m_pi->m_settings.show_radar[m_ri->m_radar];
    LOG_DIALOG(wxT("%s OnRadarShowButton: show_radar[%d]=%d"), m_log_name.c_str(), m_ri->m_radar, show);
  }
  m_pi->NotifyRadarWindowViz();
}

void ControlsDialog::OnRadarOverlayButtonClick(wxCommandEvent& event) {
  SetMenuAutoHideTimeout();

  RadarControlButton* button = (RadarControlButton*)event.GetEventObject();
  int canvasIndex = button->GetId() - ID_RADAR_OVERLAY0;

  LOG_DIALOG(wxT("OnRadarOverlayButtonClick button=%p canvas=%d, buttonid=%i"), button, canvasIndex, button->GetId());

  if (button->m_item->GetValue() == 0) {
    // flip overlay to on
    button->m_item->Update(1);
    m_ri->m_overlay_canvas[canvasIndex].Update(1);
    m_ri->UpdateControlState(false);
    // no other radars can do overlay on same canvas
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      if (m_pi->m_radar[r] != m_ri) {
        m_pi->m_radar[r]->m_overlay_canvas[canvasIndex].Update(0);
        m_pi->m_radar[r]->UpdateControlState(false);
      }
    }
  } else {
    // flip overlay to off
    button->m_item->Update(0);
  }
  UpdateControlValues(true);
}

void ControlsDialog::OnRadarGainButtonClick(wxCommandEvent& event) { EnterEditMode((RadarControlButton*)event.GetEventObject()); }

void ControlsDialog::OnTransmitButtonClick(wxCommandEvent& event) {
  RadarState state = (RadarState)m_ri->m_state.GetButton();
  SetMenuAutoHideTimeout();

  // If we already have a running timer, then turn timed_idle_mode off
  if (m_ri->m_next_state_change.GetValue() > 1 &&
      (m_ri->m_timed_idle_hardware || m_ri->m_idle_transmit > 0 || m_ri->m_idle_standby > 0)) {
    m_timed_idle_button->SetState(RCS_OFF);
  }
  if (state == RADAR_STANDBY || state == RADAR_STOPPING || state == RADAR_SPINNING_DOWN) {
    m_ri->RequestRadarState(RADAR_TRANSMIT);
  } else {
    m_ri->RequestRadarState(RADAR_STANDBY);
  }
}

void ControlsDialog::OnClearTrailsButtonClick(wxCommandEvent& event) { m_ri->ClearTrails(); }

void ControlsDialog::OnOrientationButtonClick(wxCommandEvent& event) {
  int value = m_ri->m_orientation.GetValue() + 1;

  if (m_pi->GetHeadingSource() == HEADING_NONE) {
    value = ORIENTATION_HEAD_UP;
  } else {  // There is a heading
    if (value == ORIENTATION_NUMBER) {
      if (M_SETTINGS.developer_mode) {
        value = ORIENTATION_HEAD_UP;
      } else {
        value = ORIENTATION_STABILIZED_UP;
      }
    }
  }

  m_ri->m_orientation.Update(value);
  UpdateControlValues(false);
}

void ControlsDialog::OnViewCenterButtonClick(wxCommandEvent& event) {
  int value = m_ri->m_view_center.GetValue() + 1;
  if (value == CENTER_VIEW) {
    m_ri->m_off_center.x = 0;
    m_ri->m_off_center.y = 0;  // reset dragging
  }
  if (value > BACKWARD_VIEW) {
    value = CENTER_VIEW;
  }
  m_ri->m_view_center.Update(value);
  UpdateControlValues(false);
  value = m_ri->m_view_center.GetValue();
}

void ControlsDialog::OnBearingSetButtonClick(wxCommandEvent& event) {
  int bearing = event.GetId() - ID_BEARING_SET;
  LOG_DIALOG(wxT("%s OnBearingSetButtonClick for bearing #%d"), m_log_name.c_str(), bearing + 1);

  m_ri->SetBearing(bearing);
}

void ControlsDialog::OnClearCursorButtonClick(wxCommandEvent& event) {
  LOG_DIALOG(wxT("%s OnClearCursorButtonClick"), m_log_name.c_str());
  m_ri->SetMouseVrmEbl(0., nanl(""));
}

void ControlsDialog::OnAcquireTargetButtonClick(wxCommandEvent& event) {
  ExtendedPosition target_pos;
  target_pos.pos = m_ri->m_mouse_pos;
  LOG_DIALOG(wxT("%s OnAcquireTargetButtonClick mouse=%f/%f"), m_log_name.c_str(), target_pos.pos.lat, target_pos.pos.lon);
  m_ri->m_arpa->AcquireNewMARPATarget(target_pos);
}

void ControlsDialog::OnDeleteTargetButtonClick(wxCommandEvent& event) {
  ExtendedPosition target_pos;
  target_pos.pos = m_ri->m_mouse_pos;
  LOG_DIALOG(wxT("%s OnDeleteTargetButtonClick mouse=%f/%f"), m_log_name.c_str(), target_pos.pos.lat, target_pos.pos.lon);
  m_ri->m_arpa->DeleteTarget(target_pos);
}

void ControlsDialog::OnDeleteAllTargetsButtonClick(wxCommandEvent& event) {
  LOG_DIALOG(wxT("%s OnDeleteAllTargetsButtonClick"), m_log_name.c_str());
  for (size_t i = 0; i < M_SETTINGS.radar_count; i++) {
    if (m_pi->m_radar[i]->m_arpa) {
      m_pi->m_radar[i]->m_arpa->DeleteAllTargets();
    }
  }
}

void ControlsDialog::OnMove(wxMoveEvent& event) { event.Skip(); }

/**
 * Show/Hide sizers and buttons depending on which sizer
 * is to be shown
 *
 * @returns   True if the menu dialog was changed and needs to be resized
 */
bool ControlsDialog::UpdateSizersButtonsShown() {
  bool resize = false;

  if (m_power_sizer && m_top_sizer->IsShown(m_power_sizer)) {
    if (m_ri->m_timed_idle.GetState() == RCS_MANUAL) {
      if (!m_timed_run_button->IsShown()) {
        m_power_sizer->Show(m_timed_run_button);
        resize = true;
      }
    } else {
      if (m_timed_run_button->IsShown()) {
        m_power_sizer->Hide(m_timed_run_button);
        resize = true;
      }
    }
  }

//#define HIDE_TRANSMIT
#ifdef HIDE_TRANSMIT
  if (state != RADAR_OFF) {
    if (m_top_sizer->IsShown(m_control_sizer) && !m_control_sizer->IsShown(m_transmit_sizer)) {
      m_control_sizer->Show(m_transmit_sizer);
      resize = true;
    }
  } else {
    if (m_top_sizer->IsShown(m_control_sizer)) {
      if (m_control_sizer->IsShown(m_transmit_sizer)) {
        m_control_sizer->Hide(m_transmit_sizer);
        resize = true;
      }
    }
  }
#endif

  if (M_SETTINGS.show_radar[m_ri->m_radar]) {
    // Show PPI related buttons
    if (m_control_sizer->IsShown(m_transmit_sizer) && !m_transmit_sizer->IsShown(m_cursor_menu)) {
      m_transmit_sizer->Show(m_cursor_menu);
      resize = true;
    }
    if (m_top_sizer->IsShown(m_view_sizer) && !m_view_sizer->IsShown(m_targets_on_ppi_button)) {
      m_view_sizer->Show(m_targets_on_ppi_button);
      resize = true;
    }
    if (m_top_sizer->IsShown(m_view_sizer) && !m_view_sizer->IsShown(m_orientation_button)) {
      m_view_sizer->Show(m_orientation_button);
      resize = true;
    }
    if (m_top_sizer->IsShown(m_view_sizer) && !m_view_sizer->IsShown(m_view_center_button)) {
      m_view_sizer->Show(m_view_center_button);
      resize = true;
    }
  } else {
    // Hide PPI related buttons
    if (m_control_sizer->IsShown(m_transmit_sizer) && m_transmit_sizer->IsShown(m_cursor_menu)) {
      m_transmit_sizer->Hide(m_cursor_menu);
      resize = true;
    }
    if (m_top_sizer->IsShown(m_view_sizer) && m_view_sizer->IsShown(m_targets_on_ppi_button)) {
      m_view_sizer->Hide(m_targets_on_ppi_button);
      resize = true;
    }
    if (m_top_sizer->IsShown(m_view_sizer) && m_view_sizer->IsShown(m_orientation_button)) {
      m_view_sizer->Hide(m_orientation_button);
      resize = true;
    }
    if (m_top_sizer->IsShown(m_view_sizer) && m_view_sizer->IsShown(m_view_center_button)) {
      m_view_sizer->Hide(m_view_center_button);
      resize = true;
    }
  }

  for (int i = 0; i < CANVAS_COUNT; i++) {
    if (m_top_sizer->IsShown(m_window_sizer) && !m_window_sizer->IsShown(m_overlay_button[i])) {
      m_window_sizer->Show(m_overlay_button[i]);
    }
  }
  for (int i = CANVAS_COUNT; i < MAX_CHART_CANVAS; i++) {
    if (m_window_sizer->IsShown(m_overlay_button[i])) {
      m_window_sizer->Hide(m_overlay_button[i]);
    }
  }

  if (CANVAS_COUNT == 1) {
    if (m_top_sizer->IsShown(m_window_sizer) && m_window_sizer->IsShown(m_overlay_button[0])) {
      m_overlay_button[0]->SetFirstLine(_("Radar Overlay"));
      m_overlay_button[0]->UpdateLabel(true);
    }
  } else {
    if (m_top_sizer->IsShown(m_window_sizer) && m_window_sizer->IsShown(m_overlay_button[0])) {
      m_overlay_button[0]->SetFirstLine(_("Overlay 1"));
      m_overlay_button[0]->UpdateLabel(true);
    }
  }

  if (m_pi->GetHeadingSource() == HEADING_NONE) {
    m_orientation_button->Disable();
  } else {
    m_orientation_button->Enable();
  }

  int arpa_targets = m_ri->m_arpa->GetTargetCount();
  if (arpa_targets) {
    m_delete_target->Enable();
    m_delete_all->Enable();
  } else {
    m_delete_target->Disable();
    m_delete_all->Disable();
  }

  if (m_top_sizer->IsShown(m_view_sizer)) {
    RadarControlState trails = m_ri->m_target_trails.GetState();

    if (trails == RCS_OFF) {
      m_trails_motion_button->Disable();
      m_clear_trails_button->Disable();
    } else {
      m_trails_motion_button->Enable();
      m_clear_trails_button->Enable();
    }
  }

  if (m_top_sizer->IsShown(m_window_sizer)) {
    bool overlay = false;
    for (int i = 0; i < CANVAS_COUNT; i++) {
      if (m_ri->m_overlay_canvas[i].GetValue() > 0) {
        overlay = true;
        break;
      }
    }
    if (overlay) {
      m_transparency_button->Enable();
    } else {
      m_transparency_button->Disable();
    }
  }

  return resize;
}

void ControlsDialog::Resize(bool force) {
  bool resize = UpdateSizersButtonsShown();

  if (resize || force) {
    m_current_sizer->Layout();
    m_control_sizer->Layout();
    m_top_sizer->Layout();
    Fit();
    DimeWindow(this);
  }
}

//
// Radar is Off. Disable all controls that send commands to radar.
//
void ControlsDialog::DisableRadarControls() {
  if (m_ftc_button) {
    m_ftc_button->Disable();
  }
  if (m_sea_button) {
    m_sea_button->Disable();
  }
  if (m_gain_button) {
    m_gain_button->Disable();
  }
  if (m_rain_button) {
    m_rain_button->Disable();
  }
  if (m_interference_rejection_button) {
    m_interference_rejection_button->Disable();
  }
  if (m_target_separation_button) {
    m_target_separation_button->Disable();
  }
  if (m_noise_rejection_button) {
    m_noise_rejection_button->Disable();
  }
  if (m_target_boost_button) {
    m_target_boost_button->Disable();
  }
  if (m_target_expansion_button) {
    m_target_expansion_button->Disable();
  }
  if (m_scan_speed_button) {
    m_scan_speed_button->Disable();
  }
  if (m_bearing_alignment_button) {
    m_bearing_alignment_button->Disable();
  }
  if (m_no_transmit_start_button) {
    m_no_transmit_start_button->Disable();
  }
  if (m_no_transmit_end_button) {
    m_no_transmit_end_button->Disable();
  }
  if (m_antenna_height_button) {
    m_antenna_height_button->Disable();
  }
  if (m_local_interference_rejection_button) {
    m_local_interference_rejection_button->Disable();
  }
  if (m_side_lobe_suppression_button) {
    m_side_lobe_suppression_button->Disable();
  }
  if (m_range_button) {
    m_range_button->Disable();
  }
  if (m_power_button) {
    m_power_button->Disable();
  }
  if (m_power_sub_button) {
    m_power_sub_button->Disable();
  }
  if (m_doppler_button) {
    m_doppler_button->Disable();
  }
}

void ControlsDialog::EnableRadarControls() {
  if (m_ftc_button) {
    m_ftc_button->Enable();
  }
  if (m_sea_button) {
    m_sea_button->Enable();
  }
  if (m_gain_button) {
    m_gain_button->Enable();
  }
  if (m_rain_button) {
    m_rain_button->Enable();
  }
  if (m_interference_rejection_button) {
    m_interference_rejection_button->Enable();
  }
  if (m_target_separation_button) {
    m_target_separation_button->Enable();
  }
  if (m_noise_rejection_button) {
    m_noise_rejection_button->Enable();
  }
  if (m_target_boost_button) {
    m_target_boost_button->Enable();
  }
  if (m_target_expansion_button) {
    m_target_expansion_button->Enable();
  }
  if (m_scan_speed_button) {
    m_scan_speed_button->Enable();
  }
  if (m_bearing_alignment_button) {
    m_bearing_alignment_button->Enable();
  }
  if (m_no_transmit_start_button) {
    m_no_transmit_start_button->Enable();
  }
  if (m_no_transmit_end_button) {
    m_no_transmit_end_button->Enable();
  }
  if (m_antenna_height_button) {
    m_antenna_height_button->Enable();
  }
  if (m_local_interference_rejection_button) {
    m_local_interference_rejection_button->Enable();
  }
  if (m_side_lobe_suppression_button) {
    m_side_lobe_suppression_button->Enable();
  }
  if (m_range_button) {
    m_range_button->Enable();
  }
  if (m_power_button) {
    m_power_button->Enable();
  }
  if (m_power_sub_button) {
    m_power_sub_button->Enable();
  }
  if (m_doppler_button) {
    m_doppler_button->Enable();
  }
}

void ControlsDialog::UpdateControlValues(bool refreshAll) {
  wxString o;
  bool updateEditDialog = false;
  bool resize = false;

  if (m_ri->m_state.IsModified()) {
    refreshAll = true;
    resize = true;
  } else {
    for (int i = 0; i < CANVAS_COUNT; i++) {
      if (m_ri->m_overlay_canvas[i].IsModified()) {
        refreshAll = true;
        resize = true;
        break;
      }
    }
  }

  if (m_from_control && m_top_sizer->IsShown(m_edit_sizer)) {
    updateEditDialog = refreshAll || m_from_control->m_item->IsModified();
  }

  RadarState state = (RadarState)m_ri->m_state.GetButton();

  o << _("Start/Stop radar") << wxT("\n");
  o << m_ri->GetRadarStateText();
  if (state == RADAR_OFF) {
    DisableRadarControls();
  } else {
    EnableRadarControls();
  }
  m_power_button->SetLabel(o);
  if (m_power_sizer) {
    m_power_sub_button->SetLabel(o);
  }
  o = _("Hide/Show PPI") + wxT("\n");
  o << (m_pi->m_settings.show_radar[m_ri->m_radar] ? _("Shown") : _("Hidden"));
  m_show_ppi_button->SetLabel(o);

  o = _("Float/Dock PPI") + wxT("\n");
  o << (m_pi->m_settings.dock_radar[m_ri->m_radar] ? _("Docked") : _("Floating"));
  m_dock_ppi_button->SetLabel(o);

  for (int b = 0; b < BEARING_LINES; b++) {
    if (!isnan(m_ri->m_vrm[b])) {
      o = _("Clear EBL/VRM");
      o << wxString::Format(wxT("%d"), b + 1);
    } else {
      o = _("Place EBL/VRM");
      o << wxString::Format(wxT("%d"), b + 1);
    }
    m_bearing_buttons[b]->SetLabel(o);
  }

  m_targets_on_ppi_button->UpdateLabel();
  m_target_trails_button->UpdateLabel();
  m_trails_motion_button->UpdateLabel();
  m_orientation_button->UpdateLabel();
  m_view_center_button->UpdateLabel();
  for (int i = 0; i < CANVAS_COUNT; i++) {
    m_overlay_button[i]->UpdateLabel();
  }

  if (m_range_button && (m_ri->m_range.IsModified() || refreshAll)) {
    m_ri->m_range.GetButton();
    m_range_button->SetRangeLabel();
  }

  // gain
  if (m_gain_button) {
    m_gain_button->UpdateLabel();
  }

  //  rain
  if (m_rain_button) {
    m_rain_button->UpdateLabel();
  }

  //  FTC
  if (m_ftc_button) {
    m_ftc_button->UpdateLabel();
  }

  //   sea
  if (m_sea_button) {
    m_sea_button->UpdateLabel();
  }

  //   target_boost
  if (m_target_boost_button) {
    m_target_boost_button->UpdateLabel();
  }

  //   target_expansion
  if (m_target_expansion_button) {
    m_target_expansion_button->UpdateLabel();
  }

  //  noise_rejection
  if (m_noise_rejection_button) {
    m_noise_rejection_button->UpdateLabel();
  }

  //  target_separation
  if (m_target_separation_button) {
    m_target_separation_button->UpdateLabel();
  }

  //  interference_rejection
  if (m_interference_rejection_button) {
    m_interference_rejection_button->UpdateLabel();
  }

  // scanspeed
  if (m_scan_speed_button) {
    m_scan_speed_button->UpdateLabel();
  }

  //   antenna height
  if (m_antenna_height_button) {
    m_antenna_height_button->UpdateLabel();
  }

  //  bearing alignment
  if (m_bearing_alignment_button) {
    m_bearing_alignment_button->UpdateLabel();
  }

  //  no transmit zone
  if (m_no_transmit_start_button) {
    m_no_transmit_start_button->UpdateLabel();
  }
  if (m_no_transmit_end_button) {
    m_no_transmit_end_button->UpdateLabel();
  }

  //  local interference rejection
  if (m_local_interference_rejection_button) {
    m_local_interference_rejection_button->UpdateLabel();
  }

  // side lobe suppression
  if (m_side_lobe_suppression_button) {
    m_side_lobe_suppression_button->UpdateLabel();
  }

  if (m_main_bang_size_button) {
    m_main_bang_size_button->UpdateLabel();
  }
  if (m_antenna_starboard_button) {
    m_antenna_starboard_button->UpdateLabel();
  }
  if (m_antenna_forward_button) {
    m_antenna_forward_button->UpdateLabel();
  }

  // For these we can't use the modified state as they are shared amongst all
  // radars and thus the modified state is reset on the first radar checking it.
  if (m_transparency_button) {
    m_transparency_button->UpdateLabel(true);
  }
  if (m_timed_idle_button) {
    m_timed_idle_button->UpdateLabel(true);
  }
  if (m_timed_run_button) {
    m_timed_run_button->UpdateLabel(true);
  }
  if (m_refresh_rate_button) {
    m_refresh_rate_button->UpdateLabel(true);
  }

  if (m_doppler_button) {
    m_doppler_button->UpdateLabel();
  }

  if (updateEditDialog) {
    // Update the text that is currently shown in the edit box, this is a copy of the button itself
    EnterEditMode(m_from_control);
  } else {
    Resize(resize);
  }
}

void ControlsDialog::UpdateDialogShown(bool resize) {
  if (m_hide) {
    if (IsShown()) {
      LOG_DIALOG(wxT("%s UpdateDialogShown explicit closed: Hidden"), m_log_name.c_str());
      Hide();
    }
    return;
  }

  if (m_hide_temporarily) {
    if (IsShown()) {
      Hide();
    }
    return;
  }

  if (m_top_sizer->IsShown(m_control_sizer)) {
    if (m_auto_hide_timeout > 0 && TIMED_OUT(time(0), m_auto_hide_timeout)) {
      if (IsShown()) {
        Hide();
      }
      return;
    }
  } else {
    // If we're somewhere in the sub-window, don't close the dialog
    m_auto_hide_timeout = 0;
  }

#ifdef __WXMAC__NEVER
  // Following helps on OSX where the control is SHOW_ON_TOP to not show when no part of OCPN is focused
  wxWindow* focused = FindFocus();
  if (!focused) {
    LOG_DIALOG(wxT("%s UpdateDialogShown app not focused"), m_log_name.c_str());
    Hide();
    return;
  }
#endif

  if (!IsShown()) {
    if (!m_top_sizer->IsShown(m_control_sizer) && !m_top_sizer->IsShown(m_advanced_sizer) && !m_top_sizer->IsShown(m_view_sizer) &&
        !m_top_sizer->IsShown(m_edit_sizer) && !m_top_sizer->IsShown(m_installation_sizer) &&
        !m_top_sizer->IsShown(m_window_sizer) && !m_top_sizer->IsShown(m_guard_sizer) && !m_top_sizer->IsShown(m_guardzone_sizer) &&
        !m_top_sizer->IsShown(m_adjust_sizer) && !m_top_sizer->IsShown(m_cursor_sizer) &&
        (m_power_sizer && !m_top_sizer->IsShown(m_power_sizer))) {
      SwitchTo(m_control_sizer, wxT("main (manual open)"));
    }
    Show();
    Raise();
    resize = true;

    // If the corresponding radar panel is now in a different position from what we remembered
    // then reset the dialog to the left or right of the radar panel.
    wxPoint panelPos = m_ri->m_radar_panel->GetPos();
    bool controlInitialShow = !m_pi->m_settings.control_pos[m_ri->m_radar].IsFullySpecified();
    // bool panelShown = m_ri->m_radar_panel->IsShown();
    // bool panelMoved = !m_panel_position.IsFullySpecified() || panelPos != m_panel_position;

    if (controlInitialShow) {  // the position has never been set at all, ever
      wxSize panelSize = m_ri->m_radar_panel->GetSize();
      wxSize mySize = this->GetSize();

      wxPoint newPos;
      newPos.x = panelPos.x + panelSize.x - mySize.x;
      newPos.y = panelPos.y;
      SetPosition(newPos);
      LOG_DIALOG(wxT("%s show control menu over menu button"), m_log_name.c_str());
    } else if (controlInitialShow) {  // When all else fails set it to default position
      SetPosition(wxPoint(100 + m_ri->m_radar * 100, 100));
      LOG_DIALOG(wxT("%s show control menu at initial location"), m_log_name.c_str());
    }
    EnsureWindowNearOpenCPNWindow();  // If the position is really weird, move it
    m_pi->m_settings.control_pos[m_ri->m_radar] = GetPosition();
    m_pi->m_settings.show_radar_control[m_ri->m_radar] = true;
    m_panel_position = panelPos;
  }
  Resize(false);
}

void ControlsDialog::HideTemporarily() {
  m_hide_temporarily = true;
  UpdateDialogShown(false);
}

void ControlsDialog::UnHideTemporarily() {
  m_hide_temporarily = false;
  SetMenuAutoHideTimeout();
  UpdateDialogShown(false);
}

void ControlsDialog::ShowDialog() {
  m_hide = false;
  UnHideTemporarily();
  UpdateControlValues(true);
}

void ControlsDialog::HideDialog() {
  m_hide = true;
  m_auto_hide_timeout = 0;
  UpdateDialogShown(false);
}

void ControlsDialog::OnGuardZoneModeClick(wxCommandEvent& event) { SetGuardZoneVisibility(); }

void ControlsDialog::OnInner_Range_Value(wxCommandEvent& event) {
  wxString temp = m_inner_range->GetValue();
  double t;
  m_guard_zone->m_show_time = time(0);
  temp.ToDouble(&t);

  int conversionFactor = RangeUnitsToMeters[m_pi->m_settings.range_units];

  m_guard_zone->SetInnerRange((int)(t * conversionFactor));
}

void ControlsDialog::OnOuter_Range_Value(wxCommandEvent& event) {
  wxString temp = m_outer_range->GetValue();
  double t;
  m_guard_zone->m_show_time = time(0);
  temp.ToDouble(&t);

  int conversionFactor = RangeUnitsToMeters[m_pi->m_settings.range_units];

  m_guard_zone->SetOuterRange((int)(t * conversionFactor));
}

void ControlsDialog::OnStart_Bearing_Value(wxCommandEvent& event) {
  wxString temp = m_start_bearing->GetValue();
  long t;

  m_guard_zone->m_show_time = time(0);

  temp.ToLong(&t);
  t = MOD_DEGREES(t);
  while (t < 0) {
    t += 360;
  }
  m_guard_zone->SetStartBearing(t);
}

void ControlsDialog::OnEnd_Bearing_Value(wxCommandEvent& event) {
  wxString temp = m_end_bearing->GetValue();
  long t;

  m_guard_zone->m_show_time = time(0);

  temp.ToLong(&t);
  t = MOD_DEGREES(t);
  while (t < 0) {
    t += 360;
  }
  m_guard_zone->SetEndBearing(t);
}

void ControlsDialog::OnARPAClick(wxCommandEvent& event) {
  int arpa = m_arpa_box->GetValue();
  m_guard_zone->SetArpaOn(arpa);
}

void ControlsDialog::OnAlarmClick(wxCommandEvent& event) {
  int alarm = m_alarm->GetValue();
  m_guard_zone->SetAlarmOn(alarm);
}

PLUGIN_END_NAMESPACE
