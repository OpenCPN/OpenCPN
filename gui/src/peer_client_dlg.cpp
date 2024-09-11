/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Peer-peer data sharing.
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
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
 **************************************************************************/

#include <cassert>
#include <iostream>
#include <sstream>

#include <wx/fileconf.h>
#include <wx/json_defs.h>
#include <wx/jsonreader.h>
#include <wx/tokenzr.h>

#include <curl/curl.h>

#include "model/peer_client.h"
#include "model/rest_server.h"
#include "model/semantic_vers.h"
#include "model/config_vars.h"

#include "peer_client_dlg.h"
#include "ocpn_frame.h"
#include "FontMgr.h"
#include "gui_lib.h"

extern MyFrame* gFrame;

struct MemoryStruct {
  char* memory;
  size_t size;
};

PinConfirmDlg::PinConfirmDlg() {
  m_ok_btn = NULL;
  m_cancel_btn = NULL;
  premtext = NULL;
}

PinConfirmDlg::PinConfirmDlg(wxWindow* parent, wxWindowID id,
                             const wxString& caption, const wxString& hint,
                             const wxPoint& pos, const wxSize& size,
                             long style) {
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont(*pif);
  Create(parent, id, caption, hint, pos, size, style);
}

PinConfirmDlg::~PinConfirmDlg() {
  delete m_ok_btn;
  delete m_cancel_btn;
}

/** Enable OK button iff input represents a four digit pincode. */
void PinConfirmDlg::OnTextChange(wxCommandEvent&) {
  auto txt = m_pin_textctrl->GetValue().ToStdString();
  int value = -1;
  if (txt.size() >= 4) {
    try {
      value = std::stoi(txt);
    } catch (std::exception&) {
      ;
    }
  }
  m_ok_btn->Enable(value >= 0 && txt.size() >= 4);
}

bool PinConfirmDlg::Create(wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxString& hint,
                           const wxPoint& pos, const wxSize& size, long style) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls(hint);
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();
  return TRUE;
}

void PinConfirmDlg::CreateControls(const wxString&) {
  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer2);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText(
      this, -1, "A loooooooooooooooooooooooooooooooooooooooooooooong line\n");
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  m_pin_textctrl = new wxTextCtrl(this, wxID_ANY, "        ", wxDefaultPosition,
                                  wxDefaultSize, wxTE_CENTRE);
  itemBoxSizer2->Add(m_pin_textctrl, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);

  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_cancel_btn = new wxButton(this, wxID_CANCEL);

  m_cancel_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                     [&](wxCommandEvent e) { OnCancelClick(e); });
  itemBoxSizer16->Add(m_cancel_btn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_ok_btn = new wxButton(this, wxID_OK);
  itemBoxSizer16->Add(m_ok_btn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_ok_btn->SetDefault();
  m_ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                 [&](wxCommandEvent e) { OnOKClick(e); });
  m_ok_btn->Enable(false);
  m_pin_textctrl->Bind(wxEVT_TEXT,
                       [&](wxCommandEvent& ev) { OnTextChange(ev); });
}

void PinConfirmDlg::SetMessage(const wxString& msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void PinConfirmDlg::SetPincodeText(const wxString& message) {
  m_pin_textctrl->ChangeValue(message);
  m_pin_textctrl->Show();
  GetSizer()->Fit(this);
}

void PinConfirmDlg::OnOKClick(wxCommandEvent&) { EndModal(wxID_OK); }

void PinConfirmDlg::OnCancelClick(wxCommandEvent&) { EndModal(wxID_CANCEL); }
