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
#include <wx/event.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>

#include "dialog_alert.h"

AlertDialog::AlertDialog(wxWindow* parent, const std::string& title,
                         const std::string& action)
    : BaseDialog(parent, title, wxSTAY_ON_TOP | wxCAPTION),
      m_action(action),
      m_listener(nullptr) {
  DialogFooter* footer = new DialogFooter();
  wxButton* ok_button = new wxButton(this, wxID_OK, m_action);
  wxButton* cancel_button = new wxButton(this, wxID_CANCEL, _("Cancel"));
  footer->AddButton(cancel_button);
  footer->AddButton(ok_button);
  footer->Realize();

  Bind(wxEVT_BUTTON, &AlertDialog::OnConfirm, this, wxID_OK);
  Bind(wxEVT_BUTTON, &AlertDialog::OnCancel, this, wxID_CANCEL);
  m_layout->Add(footer, wxSizerFlags().Border(
                            wxALL, GUI::GetSpacing(this, kDialogPadding)));
}

AlertDialog::~AlertDialog() {}

void AlertDialog::SetListener(IAlertConfirmation* listener) {
  m_listener = listener;
}

void AlertDialog::SetMessage(const std::string& msg) {
  m_content->Add(new wxStaticText(this, wxID_ANY, msg));
  m_content->SetSizeHints(this);
}

int AlertDialog::ShowModal() {
  // Adjust the dialog size.
  m_layout->Fit(this);
  wxSize size(GetSize());
  if (size.x < size.y * 3 / 2) {
    size.x = size.y * 3 / 2;
    SetSize(size);
  }
  Centre(wxBOTH | wxCENTER_FRAME);
  return wxDialog::ShowModal();
}

int AlertDialog::GetConfirmation(wxWindow* parent, const std::string& title,
                                 const std::string& action,
                                 const std::string& msg) {
  AlertDialog* dialog = new AlertDialog(parent, title, action);
  dialog->SetMessage(msg);
  return dialog->Show();
}

void AlertDialog::OnCancel(wxCommandEvent& event) {
  SetReturnCode(wxID_NO);
  if (m_listener != nullptr) {
    m_listener->OnConfirm(false);
    this->Destroy();
  } else {
    EndModal(wxID_NO);
  }
}

void AlertDialog::OnConfirm(wxCommandEvent& event) {
  SetReturnCode(wxID_YES);
  if (m_listener != nullptr) {
    m_listener->OnConfirm(true);
    this->Destroy();
  } else {
    EndModal(wxID_YES);
  }
}
