/***************************************************************************
 *   Copyright (C) 2026 Alec Leamas                                        *
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
 ***************************************************************************/

#ifndef SFX_filTER_sENTencE_DLG__
#define SFX_filTER_sENTencE_DLG__

#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/regex.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/textdlg.h>

#include "model/conn_params.h"
#include "model/config_vars.h"
#include "nmea0183.h"

#include "gui_lib.h"

/** Filter list editing dialog. */
class SentenceListDlg : public wxDialog {
public:
  explicit SentenceListDlg(wxWindow* parent, FilterDirection dir, ListType type,
                           const wxArrayString& list);
  wxString GetSentences();

private:
  void OnAddClick(wxCommandEvent& event);
  void OnDeleteClick(wxCommandEvent& event);
  void OnCLBSelect(wxCommandEvent& event);
  void OnCheckAllClick(wxCommandEvent& event);
  void OnClearAllClick(wxCommandEvent& event);

  void Populate(const wxArrayString& list);
  [[nodiscard]] wxString GetBoxLabel() const;

  wxCheckListBox* m_sentences_clb;
  wxButton* m_del_btn;

  ListType m_type;
  FilterDirection m_dir;
  wxArrayString m_sentences;
};

#endif  // SFX_filTER_sENTencE_DLG__
