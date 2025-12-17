/***************************************************************************
 *   Copyright (C) 2025  Alec Leamas                                       *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 * Implement user_colors_dlg.h
 */

#include <functional>
#include <iostream>

#include <wx/button.h>
#include <wx/clrpicker.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "model/config_vars.h"
#include "model/gui_events.h"
#include "model/navmsg_filter.h"

#include "user_colors_dlg.h"
#include "tty_scroll.h"

static constexpr const char* const kDialogName = "DataMonitorColors";

/** Dialog top frame */
class UserColoursDlg : public wxFrame {
public:
  UserColoursDlg(wxWindow* parent)
      : wxFrame(parent, wxID_ANY, _("Data Monitor: colours setup")),
        m_top_panel(nullptr) {
    SetName(kDialogName);
    auto on_ok = [&] {
      m_top_panel->Apply();
      Destroy();
    };
    auto on_apply = [&] { m_top_panel->Apply(); };
    auto on_cancel = [&] { m_top_panel->Cancel(); };
    auto buttons = new ButtonSizer(this, on_ok, on_apply, on_cancel);
    auto vbox = new wxBoxSizer(wxVERTICAL);
    m_top_panel = new TopPanel(this);
    vbox->Add(m_top_panel, wxSizerFlags(1).Border().Expand());
    vbox->Add(new wxStaticLine(this, wxID_ANY), wxSizerFlags(0).Expand());
    vbox->Add(buttons, wxSizerFlags(0).Expand().Border());
    SetSizer(vbox);
    Fit();
  }

private:
  using ButtonHandler = std::function<void()>;

  /** State used to support Cancel i.e. restore values to initial state. */
  class StoredConfig {
  public:
    StoredConfig() { Load(); }

    void Load() {
      dm_dropped = g_dm_dropped;
      dm_filtered = g_dm_filtered;
      dm_input = g_dm_input;
      dm_not_ok = g_dm_not_ok;
      dm_ok = g_dm_ok;
      dm_output = g_dm_output;
    }

    void Save() {
      g_dm_dropped = WorkValue(
          dm_dropped, NavmsgStatus(NavmsgStatus::Accepted::kFilteredDropped));
      g_dm_filtered = WorkValue(
          dm_filtered, NavmsgStatus(NavmsgStatus::Accepted::kFilteredNoOutput));
      g_dm_input =
          WorkValue(dm_input, NavmsgStatus(NavmsgStatus::Direction::kInput));
      g_dm_not_ok =
          WorkValue(dm_not_ok, NavmsgStatus(NavmsgStatus::State::kMalformed));
      g_dm_ok =
          WorkValue(dm_ok, NavmsgStatus(NavmsgStatus::Direction::kHandled));
      g_dm_output =
          WorkValue(dm_output, NavmsgStatus(NavmsgStatus::Direction::kOutput));
    }

  private:
    unsigned dm_dropped;
    unsigned dm_filtered;
    unsigned dm_input;
    unsigned dm_not_ok;
    unsigned dm_ok;
    unsigned dm_output;

    UserColorsByState default_colors;

    unsigned WorkValue(unsigned stored_value, NavmsgStatus ns) {
      if (stored_value == kUndefinedColor) return default_colors(ns).GetRGB();
      return stored_value;
    }
  };

  /** The six color pickers and a "Restore Defaults" button in a grid.  */
  class TopPanel : public wxPanel {
  public:
    TopPanel(wxWindow* parent) : wxPanel(parent) {
      auto grid = new wxGridSizer(2);

      NavmsgStatus ns(NavmsgStatus::Direction::kHandled);
      grid->Add(new wxStaticText(this, wxID_ANY, _("Input - OK")));
      m_msg_ok_pick = new wxColourPickerCtrl(this, wxID_ANY, m_colors(ns));
      grid->Add(m_msg_ok_pick);

      grid->Add(new wxStaticText(this, wxID_ANY, _("Errors")));
      ns = NavmsgStatus(NavmsgStatus::State::kMalformed);
      m_msg_not_ok_pick = new wxColourPickerCtrl(this, wxID_ANY, m_colors(ns));
      grid->Add(m_msg_not_ok_pick);

      grid->Add(new wxStaticText(this, wxID_ANY, _("Filtered, no output")));
      ns = NavmsgStatus(NavmsgStatus::Accepted::kFilteredNoOutput);
      m_msg_filtered_pick =
          new wxColourPickerCtrl(this, wxID_ANY, m_colors(ns));
      grid->Add(m_msg_filtered_pick);

      grid->Add(new wxStaticText(this, wxID_ANY, _("Filtered, dropped")));
      ns = NavmsgStatus(NavmsgStatus::Accepted::kFilteredDropped);
      m_msg_dropped_pick = new wxColourPickerCtrl(this, wxID_ANY, m_colors(ns));
      grid->Add(m_msg_dropped_pick);

      grid->Add(new wxStaticText(this, wxID_ANY, _("Output")));
      ns = NavmsgStatus(NavmsgStatus::Direction::kOutput);
      m_msg_output_pick = new wxColourPickerCtrl(this, wxID_ANY, m_colors(ns));
      grid->Add(m_msg_output_pick);

      grid->Add(new wxStaticText(this, wxID_ANY, _("Input event")));
      ns = NavmsgStatus(NavmsgStatus::Direction::kInput);
      m_msg_input_pick = new wxColourPickerCtrl(this, wxID_ANY, m_colors(ns));
      grid->Add(m_msg_input_pick);

      auto reset_btn = new wxButton(this, wxID_UNDO, _("Restore defaults"));
      reset_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                      [&](wxCommandEvent&) { Reset(); });
      grid->Add(reset_btn);

      SetSizer(grid);
    }

    /** Update globals from GUI state. */
    void Apply() {
      g_dm_ok = static_cast<unsigned>(m_msg_ok_pick->GetColour().GetRGB());
      g_dm_not_ok =
          static_cast<unsigned>(m_msg_not_ok_pick->GetColour().GetRGB());
      g_dm_filtered =
          static_cast<unsigned>(m_msg_filtered_pick->GetColour().GetRGB());
      g_dm_dropped =
          static_cast<unsigned>(m_msg_dropped_pick->GetColour().GetRGB());
      g_dm_output =
          static_cast<unsigned>(m_msg_output_pick->GetColour().GetRGB());
      g_dm_input =
          static_cast<unsigned>(m_msg_input_pick->GetColour().GetRGB());
      GuiEvents::GetInstance().on_dm_colors_change.Notify();
    }

    /** Update GUI state from globals. */
    void Cancel() {
      m_stored_config.Save();
      m_msg_ok_pick->SetColour(wxColour(static_cast<unsigned long>(g_dm_ok)));
      m_msg_not_ok_pick->SetColour(
          wxColour(static_cast<unsigned long>(g_dm_not_ok)));
      m_msg_filtered_pick->SetColour(
          wxColour(static_cast<unsigned long>(g_dm_filtered)));
      m_msg_dropped_pick->SetColour(
          wxColour(static_cast<unsigned long>(g_dm_dropped)));
      m_msg_output_pick->SetColour(
          wxColour(static_cast<unsigned long>(g_dm_output)));
      m_msg_input_pick->SetColour(
          wxColour(static_cast<unsigned long>(g_dm_input)));
    }

    /** Reset GUI state to initial, hardcoded values */
    void Reset() {
      const static NavmsgStatus OkStatus =
          NavmsgStatus(NavmsgStatus::Direction::kHandled);
      const static NavmsgStatus NotOkStatus =
          NavmsgStatus(NavmsgStatus::State::kMalformed);
      const static NavmsgStatus NoOutputStatus =
          NavmsgStatus(NavmsgStatus::Accepted::kFilteredNoOutput);
      const static NavmsgStatus DroppedStatus =
          NavmsgStatus(NavmsgStatus::Accepted::kFilteredDropped);
      const static NavmsgStatus OutputStatus =
          NavmsgStatus(NavmsgStatus::Direction::kOutput);
      const static NavmsgStatus InputStatus =
          NavmsgStatus(NavmsgStatus::Direction::kInput);

      m_msg_ok_pick->SetColour(wxColour(m_std_colors(OkStatus).GetRGB()));
      m_msg_not_ok_pick->SetColour(
          wxColour(m_std_colors(NotOkStatus).GetRGB()));
      m_msg_filtered_pick->SetColour(
          wxColour(m_std_colors(NoOutputStatus).GetRGB()));
      m_msg_dropped_pick->SetColour(
          wxColour(m_std_colors(DroppedStatus).GetRGB()));
      m_msg_output_pick->SetColour(
          wxColour(m_std_colors(OutputStatus).GetRGB()));
      m_msg_input_pick->SetColour(wxColour(m_std_colors(InputStatus).GetRGB()));
      m_stored_config.Load();
    }

  private:
    wxColourPickerCtrl* m_msg_ok_pick;
    wxColourPickerCtrl* m_msg_not_ok_pick;
    wxColourPickerCtrl* m_msg_filtered_pick;
    wxColourPickerCtrl* m_msg_dropped_pick;
    wxColourPickerCtrl* m_msg_output_pick;
    wxColourPickerCtrl* m_msg_input_pick;

    UserColorsByState m_colors;
    StdColorsByState m_std_colors;
    StoredConfig m_stored_config;
  };

  /** The three Apply, Cancel and OK buttons. */
  class ButtonSizer : public wxStdDialogButtonSizer {
  public:
    ButtonSizer(wxWindow* parent, ButtonHandler on_ok, ButtonHandler on_apply,
                ButtonHandler on_cancel)
        : wxStdDialogButtonSizer(),
          m_on_ok(on_ok),
          m_on_apply(on_apply),
          m_on_cancel(on_cancel) {
      auto ok_btn = new wxButton(parent, wxID_OK);
      ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                   [&](wxCommandEvent& ev) { m_on_ok(); });
      AddButton(ok_btn);
      auto cancel_btn = new wxButton(parent, wxID_CANCEL);
      cancel_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                       [&](wxCommandEvent& ev) { m_on_cancel(); });
      AddButton(cancel_btn);
      auto apply_btn = new wxButton(parent, wxID_APPLY);
      apply_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                      [&](wxCommandEvent& ev) { m_on_apply(); });
      AddButton(apply_btn);
      Realize();
    }

  protected:
    ButtonHandler m_on_ok;
    ButtonHandler m_on_apply;
    ButtonHandler m_on_cancel;
  };

private:
  TopPanel* m_top_panel;
};

wxFrame* UserColorsDlg(wxWindow* parent) {
  auto current_dlg = wxWindow::FindWindowByName(kDialogName);
  if (current_dlg) {
    current_dlg->Raise();
    return nullptr;
  } else {
    auto dialog = new UserColoursDlg(parent);
    dialog->Show();
    return dialog;
  }
}
