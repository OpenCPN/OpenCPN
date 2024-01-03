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

#include "model/nav_object_database.h"
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


wxString GetErrorText(RestServerResult result) {
  switch (result) {
    case RestServerResult::GenericError:
      return _("Server Generic Error");
    case RestServerResult::ObjectRejected:
      return _("Peer rejected object");
    case RestServerResult::DuplicateRejected:
      return _("Peer rejected duplicate object");
    case RestServerResult::RouteInsertError:
      return _("Peer internal error (insert)");
    default:
      return _("Server Unknown Error");
  }
}

PINConfirmDlg::PINConfirmDlg() {
  m_OKButton = NULL;
  m_CancelButton = NULL;
  premtext = NULL;
}

PINConfirmDlg::PINConfirmDlg(wxWindow* parent, wxWindowID id,
                             const wxString& caption,
                             const wxString& hint, const wxPoint& pos,
                             const wxSize& size, long style) {
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont(*pif);
  Create(parent, id, caption, hint, pos, size, style);
}

PINConfirmDlg::~PINConfirmDlg() {
  delete m_OKButton;
  delete m_CancelButton;
}

bool PINConfirmDlg::Create(wxWindow* parent, wxWindowID id,
                              const wxString& caption, const wxString& hint,
                              const wxPoint& pos, const wxSize& size,
                              long style) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls(hint);
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();
  return TRUE;
}

void PINConfirmDlg::CreateControls(const wxString&) {
  PINConfirmDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer2);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText(
      this, -1, "A loooooooooooooooooooooooooooooooooooooooooooooong line\n");
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  m_pText1 = new wxTextCtrl(this, wxID_ANY, "        ", wxDefaultPosition,
                            wxDefaultSize, wxTE_CENTRE);
  itemBoxSizer2->Add(m_pText1, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);

  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_PCD_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
   m_CancelButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                        [&](wxCommandEvent e)  {OnCancelClick(e); });
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_PCD_OK, "OK", wxDefaultPosition,
                            wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();
  m_OKButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                   [&](wxCommandEvent e)  {OnOKClick(e); });
}

void PINConfirmDlg::SetMessage(const wxString& msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void PINConfirmDlg::SetText1Message(const wxString& msg) {
  m_pText1->ChangeValue(msg);
  m_pText1->Show();
  GetSizer()->Fit(this);
}

void PINConfirmDlg::OnOKClick(wxCommandEvent&) {
  SetReturnCode(ID_PCD_OK);
  EndModal(ID_PCD_OK);
}

void PINConfirmDlg::OnCancelClick(wxCommandEvent&) {
  EndModal(ID_PCD_CANCEL);
}
