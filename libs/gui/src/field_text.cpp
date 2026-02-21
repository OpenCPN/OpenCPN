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

#include <wx/notebook.h>

#include "field_text.h"
#include "form_grid.h"
#include "ui_utils.h"

TextField::TextField(wxWindow* parent, const wxString& label,
                     const wxString& value, wxWindowID id)
    : wxTextCtrl(new wxPanel(parent), id, value, wxDefaultPosition,
                 wxDefaultSize, 0) {
  auto* panel = dynamic_cast<wxPanel*>(GetParent());
  assert(panel && "Textfield: Wrong parent type");

  // Add field panel and label
  auto* grid = dynamic_cast<FormGrid*>(parent->GetSizer());
  assert(grid && "Textfield: Wrong parent sizer");
  wxStaticText* text_label = new wxStaticText(parent, wxID_ANY, label);
  grid->Add(text_label, wxSizerFlags(0).Align(wxALIGN_TOP));
  grid->Add(panel, wxSizerFlags(0).Expand());

  // Sizer for field with error text.
  m_error_text = new wxStaticText(panel, wxID_ANY, "");
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(this, wxSizerFlags(0).Expand());
  sizer->Add(m_error_text, wxSizerFlags(0).Expand());
  m_error_text->Hide();
  panel->SetSizer(sizer);
  sizer->Fit(panel);
}

void TextField::OnError(const wxString& msg = "") {
  bool has_error = m_error_text->GetLabel().Len() > 0;
  m_error_text->ClearBackground();
  m_error_text->SetLabel(msg);
  m_error_text->SetForegroundColour(*wxRED);
  m_error_text->Show(msg.Len() > 0);
  m_error_text->Refresh();

  // Update layout when error status changed.
  if ((msg.Len() > 0) != has_error) {
    GUI::LayoutResizeEvent(this);
  }
}

void TextField::SetValidator(const wxValidator& validator) {
  wxTextCtrl::SetValidator(validator);
  OnError();
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
