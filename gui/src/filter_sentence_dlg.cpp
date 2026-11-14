
/**************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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
 * Implement filter_sentence_dlg.h
 */

#include "filter_sentence_dlg.h"

#include <cstddef>

#include <wx/arrstr.h>
#include <wx/string.h>

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

static wxString StringArrayToString(const wxArrayString& arr) {
  wxString ret = wxEmptyString;
  for (size_t i = 0; i < arr.Count(); i++) {
    if (i > 0) ret.Append(",");
    ret.Append(arr[i]);
  }
  return ret;
}

SentenceListDlg::SentenceListDlg(wxWindow* parent, FilterDirection dir,
                                 ListType type, const wxArrayString& list)
    : wxDialog(parent, wxID_ANY, _("Sentence Filter"), wxDefaultPosition,
               wxSize(280, 420)),
      m_type(type),
      m_dir(dir),
      m_sentences(NMEA0183().GetRecognizedArray()) {
  m_sentences.Sort();
  auto* mainSizer = new wxBoxSizer(wxVERTICAL);
  auto* secondSizer = new wxBoxSizer(wxHORIZONTAL);
  auto* pclbBox = new wxStaticBox(this, wxID_ANY, GetBoxLabel());
  auto* stcSizer = new wxStaticBoxSizer(pclbBox, wxVERTICAL);
  m_sentences_clb = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition,
                                       wxDefaultSize, m_sentences);
  auto* btnEntrySizer = new wxBoxSizer(wxVERTICAL);
  auto* btnCheckAll = new wxButton(this, wxID_ANY, _("Select All"));
  auto* btnClearAll = new wxButton(this, wxID_ANY, _("Clear All"));
  auto* btnAdd = new wxButton(this, wxID_ANY, _("Add"));
  m_del_btn = new wxButton(this, wxID_ANY, _("Delete"));
  m_del_btn->Disable();
  auto* btnSizer = new wxStdDialogButtonSizer();
  auto* btnOK = new wxButton(this, wxID_OK);
  auto* btnCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));

  secondSizer->Add(stcSizer, 1, wxALL | wxEXPAND, 5);
  stcSizer->Add(m_sentences_clb, 1, wxALL | wxEXPAND, 5);
  btnEntrySizer->Add(btnCheckAll, 0, wxALL, 5);
  btnEntrySizer->Add(btnClearAll, 0, wxALL, 5);
  btnEntrySizer->AddSpacer(1);
  btnEntrySizer->Add(btnAdd, 0, wxALL, 5);
  btnEntrySizer->Add(m_del_btn, 0, wxALL, 5);
  secondSizer->Add(btnEntrySizer, 0, wxALL | wxEXPAND, 5);
  mainSizer->Add(secondSizer, 1, wxEXPAND, 5);
  btnSizer->AddButton(btnOK);
  btnSizer->AddButton(btnCancel);
  btnSizer->Realize();
  mainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

  SetSizer(mainSizer);
  mainSizer->SetSizeHints(this);
  Centre();

  btnAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                  wxCommandEventHandler(SentenceListDlg::OnAddClick), nullptr,
                  this);
  m_del_btn->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(SentenceListDlg::OnDeleteClick),
                     nullptr, this);
  m_sentences_clb->Connect(wxEVT_COMMAND_LISTBOX_SELECTED,
                           wxCommandEventHandler(SentenceListDlg::OnCLBSelect),
                           nullptr, this);
  btnCheckAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(SentenceListDlg::OnCheckAllClick),
                       nullptr, this);
  btnClearAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(SentenceListDlg::OnClearAllClick),
                       nullptr, this);
  Populate(list);
}

wxString SentenceListDlg::GetBoxLabel() const {
  if (m_dir == FILTER_OUTPUT)
    return m_type == WHITELIST ? _("Transmit sentences") : _("Drop sentences");
  else
    return m_type == WHITELIST ? _("Accept only sentences")
                               : _("Ignore sentences");
}

void SentenceListDlg::Populate(const wxArrayString& list) {
  if (m_dir == FILTER_OUTPUT) {
    wxString s;
    m_sentences.Add(g_TalkerIdText + wxString("RMB"));
    m_sentences.Add(g_TalkerIdText + wxString("RMC"));
    m_sentences.Add(g_TalkerIdText + wxString("APB"));
    m_sentences.Add(g_TalkerIdText + wxString("XTE"));
  }
  m_sentences.Add("AIVDM");
  m_sentences.Add("AIVDO");
  m_sentences.Add("FRPOS");
  m_sentences.Add(g_TalkerIdText);
  m_sentences.Add("CD");
  m_sentences.Sort();
  m_sentences_clb->Clear();
  m_sentences_clb->InsertItems(m_sentences, 0);

  wxArrayString new_strings;
  if (list.Count() == 0) {
    for (size_t i = 0; i < m_sentences_clb->GetCount(); ++i)
      m_sentences_clb->Check(i, m_type == WHITELIST);
  } else {
    for (size_t i = 0; i < list.Count(); ++i) {
      int item = m_sentences_clb->FindString(list[i]);
      if (item != wxNOT_FOUND)
        m_sentences_clb->Check(item);
      else
        new_strings.Add(list[i]);
    }
    if (new_strings.GetCount()) {
      m_sentences_clb->InsertItems(new_strings, m_sentences_clb->GetCount());
      for (size_t i = 0; i < new_strings.GetCount(); ++i) {
        int item = m_sentences_clb->FindString(new_strings[i]);
        if (item != wxNOT_FOUND) m_sentences_clb->Check(item);
      }
    }
  }
}

wxString SentenceListDlg::GetSentences() {
  wxArrayString retString;
  for (size_t i = 0; i < m_sentences_clb->GetCount(); i++) {
    if (m_sentences_clb->IsChecked(i))
      retString.Add(m_sentences_clb->GetString(i));
  }
  return StringArrayToString(retString);
}

void SentenceListDlg::OnCLBSelect(wxCommandEvent& e) {
  // Only activate the "Delete" button if the selection is not in the standard
  // list
  m_del_btn->Enable(m_sentences.Index(e.GetString()) == wxNOT_FOUND);
}

void SentenceListDlg::OnAddClick(wxCommandEvent& event) {
#ifdef __ANDROID__
  androidDisableRotation();
#endif

  wxTextEntryDialog textdlg(
      this,
      _("Enter the NMEA sentence (2, 3 or 5 characters)\n  or a valid REGEX "
        "expression (6 characters or longer)"),
      _("Enter the NMEA sentence"));

  textdlg.SetTextValidator(wxFILTER_ASCII);
  int result = textdlg.ShowModal();

#ifdef __ANDROID__
  androidEnableRotation();
#endif

  if (result == wxID_CANCEL) return;
  wxString stc = textdlg.GetValue();

  if (stc.Length() == 2 || stc.Length() == 3 || stc.Length() == 5) {
    m_sentences_clb->Append(stc);
    m_sentences_clb->Check(m_sentences_clb->FindString(stc));
    return;
  } else if (stc.Length() < 2) {
    OCPNMessageBox(
        this,
        _("An NMEA sentence is generally 3 characters long (like RMC, GGA etc.)\n \
            It can also have a two letter prefix identifying the source, or TALKER, of the message.\n \
            The whole sentences then looks like GPGGA or AITXT.\n \
            You may filter out all the sentences with certain TALKER prefix (like GP, AI etc.).\n \
            The filter also accepts Regular Expressions (REGEX) with 6 or more characters. \n\n"),
        _("OpenCPN Info"));
    return;
  }

  else {
    // Verify that a longer text entry is a valid RegEx
    wxRegEx r(stc);
    if (r.IsValid()) {
      m_sentences_clb->Append(stc);
      m_sentences_clb->Check(m_sentences_clb->FindString(stc));
      return;
    } else {
      OCPNMessageBox(this, _("REGEX syntax error: \n") + stc,
                     _("OpenCPN Info"));
      return;
    }
  }
}

void SentenceListDlg::OnDeleteClick(wxCommandEvent& event) {
  m_sentences_clb->Delete(m_sentences_clb->GetSelection());
}

void SentenceListDlg::OnClearAllClick(wxCommandEvent& event) {
  for (size_t i = 0; i < m_sentences_clb->GetCount(); i++)
    m_sentences_clb->Check(i, false);
}

void SentenceListDlg::OnCheckAllClick(wxCommandEvent& event) {
  for (size_t i = 0; i < m_sentences_clb->GetCount(); i++)
    m_sentences_clb->Check(i, true);
}
