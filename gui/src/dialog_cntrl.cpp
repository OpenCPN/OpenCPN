/***************************************************************************
 *   Copyright (C) 2025 by NoCodeHummel                                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include "dialog_cntrl.h"
#include "model/gui.h"

TextField::TextField(wxWindow* parent, wxWindowID id, const wxString& value,
                     const wxPoint& pos, const wxSize& size, long style)
    : wxTextCtrl(new wxPanel(parent), id, value, pos, size, style) {
  m_sizer = new wxBoxSizer(wxVERTICAL);
  m_sizer->Add(this, 0, wxEXPAND);

  auto* panel = dynamic_cast<wxPanel*>(GetParent());
  assert(panel && "Textfield: Wrong parent type");
  panel->SetSizer(m_sizer);
  m_error_text = new wxStaticText(panel, wxID_ANY, "");
}

void TextField::OnError(const wxString& msg = "") {
  bool has_error = m_error_text->GetLabel().Len() > 0;
  m_error_text->SetLabel(msg);

  if (msg.IsEmpty()) {
    m_sizer->Detach(m_error_text);
    PropagateResize(this);
  } else if (has_error) {
    m_error_text->ClearBackground();
    m_error_text->Refresh();
  } else {
    m_error_text->SetForegroundColour(*wxRED);
    m_sizer->Insert(1, m_error_text, 0, wxALL | wxEXPAND, 4);
    PropagateResize(this);
  }
}

void TextField::SetValidator(const wxValidator& validator) {
  wxTextCtrl::SetValidator(validator);
  OnError("");
}

void TextField::OnTextChanged(wxCommandEvent& event) {
  auto* textCtrl = dynamic_cast<TextField*>(event.GetEventObject());
  if (textCtrl) {
    textCtrl->Validate();
  }
}

bool TextValidator::Validate(wxWindow* parent) {
  auto* text_field = dynamic_cast<TextField*>(GetWindow());
  if (text_field) {
    wxString err = IsValid(text_field->GetValue());
    text_field->OnError(err);
    return err.IsEmpty();
  } else
    return true;
}
