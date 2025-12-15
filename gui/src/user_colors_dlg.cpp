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
 * Implement filteer_colours_dlg.h
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
#include "user_colors_dlg.h"

class UserColoursDlg : public wxFrame {
public:
  UserColoursDlg(wxWindow* parent)
      : wxFrame(parent, wxID_ANY, _("Data Monitor: colours setup")),
        m_top_panel(nullptr) {
    auto on_ok = [&] {
      m_top_panel->Apply();
      Destroy();
    };
    auto on_apply = [&] { m_top_panel->Apply(); };
    auto on_cancel = [&] { m_top_panel->Cancel(); };
    auto buttons_panel = new wxPanel(this);
    auto btn_panel_sizer = new wxBoxSizer(wxVERTICAL);
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
  class TopPanel : public wxPanel {
  public:
    TopPanel(wxWindow* parent) : wxPanel(parent) {
      auto grid = new wxGridSizer(2);
      grid->Add(new wxStaticText(this, wxID_ANY, _("Input - OK")));
      m_msg_ok_pick = new wxColourPickerCtrl(this, wxID_ANY, *wxGREEN);
      grid->Add(m_msg_ok_pick);
      grid->Add(new wxStaticText(this, wxID_ANY, _("Errors")));
      m_msg_not_ok_pick = new wxColourPickerCtrl(this, wxID_ANY, *wxRED);
      grid->Add(m_msg_not_ok_pick);
      grid->Add(new wxStaticText(this, wxID_ANY, _("Filtered, no output")));
      m_msg_filtered_pick =
          new wxColourPickerCtrl(this, wxID_ANY, wxColour("CORAL"));
      grid->Add(m_msg_filtered_pick);
      grid->Add(new wxStaticText(this, wxID_ANY, _("Filtered, dropped")));
      m_msg_dropped_pick =
          new wxColourPickerCtrl(this, wxID_ANY, wxColour("MAROON"));
      grid->Add(m_msg_dropped_pick);
      grid->Add(new wxStaticText(this, wxID_ANY, _("Output")));
      m_msg_output_pick = new wxColourPickerCtrl(this, wxID_ANY, *wxBLUE);
      grid->Add(m_msg_output_pick);
      grid->Add(new wxStaticText(this, wxID_ANY, _("Input event")));
      m_msg_input_pick =
          new wxColourPickerCtrl(this, wxID_ANY, wxColour("ORANGE"));
      grid->Add(m_msg_input_pick);

      SetSizer(grid);
      Cancel();
    }

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

    void Cancel() {
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

  private:
    wxColourPickerCtrl* m_msg_ok_pick;
    wxColourPickerCtrl* m_msg_not_ok_pick;
    wxColourPickerCtrl* m_msg_filtered_pick;
    wxColourPickerCtrl* m_msg_dropped_pick;
    wxColourPickerCtrl* m_msg_output_pick;
    wxColourPickerCtrl* m_msg_input_pick;
  };

  using ButtonHandler = std::function<void()>;

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
  auto dialog = new UserColoursDlg(parent);
  dialog->Show();
  return dialog;
}
