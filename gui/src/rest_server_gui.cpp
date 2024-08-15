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

#include "FontMgr.h"
#include "rest_server_gui.h"
#include "routemanagerdialog.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

extern RouteManagerDialog* pRouteManagerDialog;
extern MyFrame* gFrame;

static wxDialog* DisplayDlg(const std::string& msg, const std::string& txt1) {
  auto dlg = new PINCreateDialog(
      dynamic_cast<wxWindow*>(gFrame), wxID_ANY, _("OpenCPN Server Message"),
      "", wxDefaultPosition, wxDefaultSize, SYMBOL_STG_STYLE);
  dlg->SetMessage(msg);
  dlg->SetText1Message(txt1);
  dlg->Show();
  return dlg;
}

static void UpdateRouteMgr() {
  if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
    pRouteManagerDialog->UpdateTrkListCtrl();
    pRouteManagerDialog->UpdateWptListCtrl();
    pRouteManagerDialog->UpdateRouteListCtrl();
  }
}

static AcceptObjectDlgResult RunAcceptObjectDlg(const wxString& msg,
                                                const wxString& check1msg) {
  AcceptObjectDialog dlg(NULL, _("OpenCPN Server Message"), msg, check1msg);
  int result = dlg.ShowModal();
  bool check1 = dlg.GetCheck1Value();
  return AcceptObjectDlgResult(result, check1);
}

RestServerDlgCtx PINCreateDialog::GetDlgCtx() {
  RestServerDlgCtx ctx;
  ctx.run_pincode_dlg = [](const std::string& msg, const std::string& text1) {
    return DisplayDlg(msg, text1);
  };
  ctx.update_route_mgr = []() { UpdateRouteMgr(); };
  ctx.run_accept_object_dlg = [](const wxString& msg,
                                 const wxString& check1msg) {
    return RunAcceptObjectDlg(msg, check1msg);
  };
  ctx.top_level_refresh = []() { dynamic_cast<wxWindow*>(gFrame)->Refresh(); };
  return ctx;
}

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

AcceptObjectDialog::AcceptObjectDialog(wxWindow* parent,
                                       const wxString& caption,
                                       const wxString& msg1,
                                       const wxString msg2)
    : AcceptObjectDialog(parent, 0, caption, "", wxDefaultPosition,
                         wxDefaultSize, SYMBOL_STG_STYLE, msg1, msg2) {}

AcceptObjectDialog::AcceptObjectDialog(wxWindow* parent, wxWindowID id,
                                       const wxString& caption,
                                       const wxString& hint, const wxPoint& pos,
                                       const wxSize& size, long style,
                                       const wxString& msg1,
                                       const wxString& msg2) {
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont(*pif);
  m_checkbox1_msg = msg2;
  Create(parent, id, caption, hint, pos, size, style, msg1, msg2);
#ifdef __ANDROID__
  androidDisableRotation();
#endif
}

AcceptObjectDialog::~AcceptObjectDialog() {
  delete m_OKButton;
  delete m_CancelButton;
#ifdef __ANDROID__
  androidEnableRotation();
#endif
}

bool AcceptObjectDialog::Create(wxWindow* parent, wxWindowID id,
                                const wxString& caption, const wxString& hint,
                                const wxPoint& pos, const wxSize& size,
                                long style, const wxString& msg1,
                                const wxString& msg2) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls(hint, msg1, msg2);
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  return TRUE;
}

void AcceptObjectDialog::CreateControls(const wxString& hint,
                                        const wxString& msg1,
                                        const wxString& msg2) {
  AcceptObjectDialog* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer2);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText(this, -1, msg1);
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  m_pCheck1 = new wxCheckBox(this, ID_STG_CHECK1, m_checkbox1_msg);
  itemBoxSizer2->Add(m_pCheck1, 0, wxEXPAND | wxALL, 10);

  if (!m_checkbox1_msg.Length()) m_pCheck1->Hide();

  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_STG_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_STG_OK, "OK", wxDefaultPosition,
                            wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();
}

void AcceptObjectDialog::SetMessage(const wxString& msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void AcceptObjectDialog::SetCheck1Message(const wxString& msg) {
  m_checkbox1_msg = msg;
  m_pCheck1->SetLabel(msg);
  m_pCheck1->Show();
  GetSizer()->Fit(this);
}

void AcceptObjectDialog::OnOKClick(wxCommandEvent& event) {
  EndModal(ID_STG_OK);
  SetReturnCode(ID_STG_OK);
}

void AcceptObjectDialog::OnCancelClick(wxCommandEvent& event) {
  EndModal(ID_STG_CANCEL);
#ifdef __ANDROID__
  androidDisableRotation();
#endif
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
#ifdef __ANDROID__
  androidEnableRotation();
#endif
}

PINCreateDialog::PINCreateDialog(wxWindow* parent, wxWindowID id,
                                 const wxString& caption, const wxString& hint,
                                 const wxPoint& pos, const wxSize& size,
                                 long style) {
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont(*pif);
  Create(parent, id, caption, hint, pos, size, style);
#ifdef __ANDROID__
  androidDisableRotation();
#endif
}

PINCreateDialog::~PINCreateDialog() {
  delete m_OKButton;
  delete m_CancelButton;
#ifdef __ANDROID__
  androidEnableRotation();
#endif
}

wxDialog* PINCreateDialog::Initiate(const std::string& msg,
                                    const std::string& text1) {
  auto dlg = new PINCreateDialog(
      dynamic_cast<wxWindow*>(gFrame), wxID_ANY, _("OpenCPN Server Message"),
      "", wxDefaultPosition, wxDefaultSize, SYMBOL_STG_STYLE);
  dlg->SetMessage(msg);
  dlg->SetText1Message(text1);
  dlg->Show();
  return dlg;
}

void PINCreateDialog::DeInit() {
  Close();
  Destroy();
}

bool PINCreateDialog::Create(wxWindow* parent, wxWindowID id,
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

void PINCreateDialog::CreateControls(const wxString& hint) {
  PINCreateDialog* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer2);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText(
      this, -1, "A loooooooooooooooooooooooooooooooooooooooooooooong line\n");
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  m_pText1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                            wxDefaultSize, wxTE_READONLY | wxTE_CENTRE);
  itemBoxSizer2->Add(m_pText1, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  m_pText1->SetMinSize(wxSize(7 * GetCharWidth(), -1));

  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_STG_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_STG_OK, "OK", wxDefaultPosition,
                            wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();
}

void PINCreateDialog::SetMessage(const wxString& msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void PINCreateDialog::SetText1Message(const wxString& msg) {
  m_pText1->ChangeValue(msg);
  m_pText1->Show();
  GetSizer()->Fit(this);
}

void PINCreateDialog::OnOKClick(wxCommandEvent& event) { Close(); }

void PINCreateDialog::OnCancelClick(wxCommandEvent& event) { Close(); }
