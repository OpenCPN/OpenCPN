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
  wxPanel* panel = dynamic_cast<wxPanel*>(GetParent());
  wxBoxSizer* nameSizer = new wxBoxSizer(wxVERTICAL);
  nameSizer->Add(this, 0, wxEXPAND);
  panel->SetSizer(nameSizer);
}

int TextField::GetSizerIndex(wxSizer* sizer) {
  if (sizer) {
    for (size_t i = 0; i < sizer->GetItemCount(); ++i) {
      wxSizerItem* item = sizer->GetItem(i);
      if (item->GetWindow() == this) {
        return static_cast<int>(i);
      }
    }
  }
  return wxNOT_FOUND;
}

void TextField::onError(const wxString& msg = wxEmptyString) {
  if (m_errorText) {
    if (msg.IsEmpty()) {
      wxSizer* sizer = GetParent()->GetSizer();
      sizer->Detach(m_errorText.get());
      m_errorText.reset();
      PropagateResize(this);
    } else {
      m_errorText->ClearBackground();
      m_errorText->SetLabel(msg);
      m_errorText->Refresh();
    }
  } else {
    if (msg.IsEmpty()) {
      SetBackgroundColour(*wxWHITE);
      Refresh();

    } else {
      wxSizer* sizer = GetParent()->GetSizer();
      int index = GetSizerIndex(sizer);  // field position in sizer

      if (sizer && index >= 0) {
        m_errorText.reset(
            new wxStaticText(GetParent(), wxID_ANY, msg, wxDefaultPosition));
        m_errorText->SetForegroundColour(*wxRED);

        sizer->Insert(index + 1, m_errorText.get(), 0, wxALL | wxEXPAND, 4);
        PropagateResize(this);
      } else {
        wxMessageDialog popup(this, msg, "Error", wxOK | wxICON_ERROR);
        popup.ShowModal();
        SetBackgroundColour(*wxRED);
        Refresh();
      }
    }
  }
}

void TextField::SetValidator(const wxValidator& validator) {
  wxTextCtrl::SetValidator(validator);
  onError("");
}

void TextField::OnTextChanged(wxCommandEvent& event) {
  TextField* textCtrl = dynamic_cast<TextField*>(event.GetEventObject());
  if (textCtrl) {
    textCtrl->Validate();
  }
}

bool TextValidator::Validate(wxWindow* parent) {
  TextField* text_field = dynamic_cast<TextField*>(GetWindow());

  if (text_field) {
    wxString err = IsValid(text_field->GetValue());
    text_field->onError(err);
    return err.IsEmpty();
  } else
    return true;
}
