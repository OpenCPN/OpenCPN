/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/dynarray.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/window.h>

// #include "conn_params.h"
// #include "OCPNPlatform.h"
// #include "route_gui.h"
// #include "route.h"
// #include "route_point_gui.h"
// #include "route_point.h"
// #include "ser_ports.h"

#include "REST_server_gui.h"
#include "FontMgr.h"

IMPLEMENT_DYNAMIC_CLASS(AcceptObjectDialog, wxDialog)

BEGIN_EVENT_TABLE(AcceptObjectDialog, wxDialog)
 EVT_BUTTON(ID_STG_CANCEL, AcceptObjectDialog::OnCancelClick)
 EVT_BUTTON(ID_STG_OK, AcceptObjectDialog::OnOKClick)
END_EVENT_TABLE()

AcceptObjectDialog::AcceptObjectDialog() {
  m_OKButton = NULL;
  m_CancelButton = NULL;
  premtext = NULL;
}

AcceptObjectDialog::AcceptObjectDialog(wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxString& hint,
                           const wxPoint& pos, const wxSize& size, long style) {
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont( *pif );
  Create(parent, id, caption, hint, pos, size, style);
}

AcceptObjectDialog::~AcceptObjectDialog() {
  delete m_OKButton;
  delete m_CancelButton;
}

bool AcceptObjectDialog::Create(wxWindow* parent, wxWindowID id,
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

void AcceptObjectDialog::CreateControls(const wxString& hint) {
  AcceptObjectDialog* itemDialog1 = this;

   wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
   SetSizer(itemBoxSizer2);


  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText( this, -1, "A loooooooooooooooooooooooooooooooooooooooooooooong line\n");
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  //    Create a progress gauge
//   wxStaticBox* prog_box = new wxStaticBox(this, wxID_ANY, _("Progress..."));
//
//   wxStaticBoxSizer* prog_box_sizer = new wxStaticBoxSizer(prog_box, wxVERTICAL);
//   itemBoxSizer2->Add(prog_box_sizer, 0, wxEXPAND | wxALL, 5);
//
//   m_pgauge = new wxGauge(this, -1, 100);
//   prog_box_sizer->Add(m_pgauge, 0, wxEXPAND | wxALL, 5);

  m_pCheck1 = new wxCheckBox(this, ID_STG_CHECK1, m_checkbox1_msg);
  itemBoxSizer2->Add(m_pCheck1, 0, wxEXPAND | wxALL, 10);

  if(!m_checkbox1_msg.Length())
    m_pCheck1->Hide();


  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_STG_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_STG_OK, "OK",
                              wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();
}

void AcceptObjectDialog::SetMessage(const wxString &msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void AcceptObjectDialog::SetCheck1Message(const wxString &msg) {
  m_checkbox1_msg = msg;
  m_pCheck1->SetLabel(msg);
  m_pCheck1->Show();
  GetSizer()->Fit(this);
}

void AcceptObjectDialog::OnOKClick(wxCommandEvent& event) {
  EndModal(ID_STG_OK);
}

void AcceptObjectDialog::OnCancelClick(wxCommandEvent& event) {
  EndModal(ID_STG_CANCEL);
}


IMPLEMENT_DYNAMIC_CLASS(PINCreateDialog, wxDialog)

BEGIN_EVENT_TABLE(PINCreateDialog, wxDialog)
 EVT_BUTTON(ID_STG_CANCEL, PINCreateDialog::OnCancelClick)
 EVT_BUTTON(ID_STG_OK, PINCreateDialog::OnOKClick)
END_EVENT_TABLE()

PINCreateDialog::PINCreateDialog() {
  m_OKButton = NULL;
  m_CancelButton = NULL;
  premtext = NULL;
}

PINCreateDialog::PINCreateDialog(wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxString& hint,
                           const wxPoint& pos, const wxSize& size, long style) {
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont( *pif );
  Create(parent, id, caption, hint, pos, size, style);
}

PINCreateDialog::~PINCreateDialog() {
  delete m_OKButton;
  delete m_CancelButton;
}

bool PINCreateDialog::Create(wxWindow* parent, wxWindowID id,
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

void PINCreateDialog::CreateControls(const wxString& hint) {
  PINCreateDialog* itemDialog1 = this;

   wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
   SetSizer(itemBoxSizer2);


  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText( this, -1, "A loooooooooooooooooooooooooooooooooooooooooooooong line\n");
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  m_pText1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_CENTRE);
  itemBoxSizer2->Add(m_pText1, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  m_pText1->SetMinSize(wxSize(7 * GetCharWidth(), -1));

  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_STG_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_STG_OK, "OK",
                              wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();
}

void PINCreateDialog::SetMessage(const wxString &msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void PINCreateDialog::SetText1Message(const wxString &msg) {
  m_pText1->ChangeValue(msg);
  m_pText1->Show();
  GetSizer()->Fit(this);
}

void PINCreateDialog::OnOKClick(wxCommandEvent& event) {
  Close();
}

void PINCreateDialog::OnCancelClick(wxCommandEvent& event) {
  Close();
}
