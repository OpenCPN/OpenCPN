/***************************************************************************
 *   Copyright (C) 2026 Alec Leamas                                        *
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
 *
 * Implement text_ctrl_w_help.h
 */

#include <wx/font.h>
#include <wx/window.h>
#include <wx/textctrl.h>
#include "text_ctrl_w_help.h"

TextCtrlWithHelp::TextCtrlWithHelp(wxWindow* parent, const std::string& help_text)
    : wxTextCtrl(parent, wxID_ANY, ""),
      m_font(GetFont()),
      m_is_inited(false),
      m_help(help_text) {
  wxTextCtrl::SetFont(m_font.Italic());
  wxTextCtrl::AppendText(help_text);
  Bind(wxEVT_TEXT, [&](wxCommandEvent& ev) { OnKeypress(ev); });
}

void TextCtrlWithHelp::SetValue(const wxString& value) {
  if (!m_is_inited) return;
  wxTextCtrl::SetValue(value);
}

void TextCtrlWithHelp::ChangeValue(const wxString& value) {
  SetFont(m_font);
  m_is_inited = true;
  wxTextCtrl::ChangeValue(value);
}

void TextCtrlWithHelp::RestoreHelp() {
  SetFont(m_font.Italic());
  wxTextEntry::ChangeValue(m_help);
  m_is_inited = false;
}

void TextCtrlWithHelp::SetHelp(const std::string& help_text) {
  m_help = help_text;
  RestoreHelp();
}

void TextCtrlWithHelp::OnKeypress(wxCommandEvent& ev) {
  if (!m_is_inited) {
    // User has pressed a key which has modified the help text
    // Find the char which differ and re-insert in the  control.
    std::string value = GetValue().ToStdString();
    unsigned ix;
    for (ix = 0; ix < m_help.size(); ++ix) {
      if (m_help[ix] != value[ix]) break;
    }
    ChangeValue(value[ix]);
    SetInsertionPointEnd();

    SetFont(m_font);
    m_is_inited = true;
  }
  ev.Skip();
}
