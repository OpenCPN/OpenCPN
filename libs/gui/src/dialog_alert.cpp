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
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/platinfo.h>
#include <wx/sizer.h>

#include "dialog_alert.h"

static void EnsureBtnSize(wxWindow* btn) {
  if (wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_WINDOWS)
    btn->SetMinSize(btn->GetSize());
  else
    btn->SetMinSize(wxSize(-1, btn->GetCharHeight() * 4));
}

AlertDialog::AlertDialog(wxWindow* parent, const std::string& title,
                         const std::string& action)
    : BaseDialog(parent, title, wxSTAY_ON_TOP | wxCAPTION),
      m_action(action),
      m_listener(nullptr) {
  DialogFooter* footer = new DialogFooter();

  if (action.empty()) {
    wxButton* close_button = new wxButton(this, wxID_CLOSE, _("Close"));

    // Fix button height in footer when dialog has no parent.
    if (!parent) EnsureBtnSize(close_button);

    close_button->Bind(wxEVT_BUTTON, &AlertDialog::OnClick, this, wxID_CLOSE);
    footer->SetCancelButton(close_button);
    footer->Realize();
  } else {
    wxButton* ok_button = new wxButton(this, wxID_OK, m_action);
    wxButton* cancel_button = new wxButton(this, wxID_CANCEL, _("Cancel"));

    // Fix button height in footer when dialog has no parent.
    if (!parent) {
      EnsureBtnSize(ok_button);
      EnsureBtnSize(cancel_button);
    }

    ok_button->Bind(wxEVT_BUTTON, &AlertDialog::OnClick, this);
    cancel_button->Bind(wxEVT_BUTTON, &AlertDialog::OnClick, this);
    footer->SetCancelButton(cancel_button);
    footer->SetAffirmativeButton(ok_button);
    footer->Realize();
  }

  auto spacing = GUI::GetSpacing(this, kDialogPadding);
  m_layout->Add(footer, wxSizerFlags().Border(wxALL, spacing).Expand());

  Bind(wxEVT_TIMER, &AlertDialog::OnTimer, this);
}

AlertDialog::AlertDialog(wxWindow* parent, const std::string& title)
    : AlertDialog(parent, title, "") {}

AlertDialog::~AlertDialog() {}

void AlertDialog::SetListener(IAlertConfirmation* listener) {
  m_listener = listener;
}

void AlertDialog::SetTimer(int seconds) {
  m_timer.SetOwner(this, -1);
  if (seconds > 0) {
    m_timer.Start(seconds * 1000, wxTIMER_ONE_SHOT);
  }
}

void AlertDialog::SetMessage(const std::string& msg) {
  m_content->Add(new wxStaticText(this, wxID_ANY, msg));
  SetInitialSize();
}

void AlertDialog::SetDefaultButton(int id) {
  auto* button = dynamic_cast<wxButton*>(FindWindowById(id));
  assert(button && "AlertDialog: Button not found");
  button->SetDefault();
  button->SetFocus();
}

void AlertDialog::SetCancelLabel(const std::string& label) {
  auto* cancel = dynamic_cast<wxButton*>(FindWindowById(wxID_CANCEL));
  assert(cancel && "AlertDialog: Cancel button not found");
  cancel->SetLabel(label);
}

int AlertDialog::GetConfirmation(wxWindow* parent, const std::string& title,
                                 const std::string& action,
                                 const std::string& msg) {
  AlertDialog* dialog = new AlertDialog(parent, title, action);
  dialog->SetMessage(msg);
  return dialog->ShowModal();
}

int AlertDialog::ShowModal() {
  Fit();
  Center(wxBOTH | wxCENTER_FRAME);
  return wxDialog::ShowModal();
}

void AlertDialog::OnClick(wxCommandEvent& event) {
  SetReturnCode(event.GetId());
  if (m_listener != nullptr) {
    m_listener->OnConfirm(event.GetId() == wxID_OK);
  } else {
    EndModal(event.GetId());
  }
}

void AlertDialog::OnTimer(wxTimerEvent& evt) {
  SetReturnCode(wxID_CLOSE);
  if (IsModal())
    EndModal(wxID_CLOSE);
  else
    Hide();
}
