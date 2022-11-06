/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
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

#ifndef _PEERCLIENT_H
#define _PEERCLIENT_H

#include <string>
#include "route.h"

int SendRoute(std::string dest_ip_address, std::string server_name, Route *route, bool overwrite = false);


 #define ID_PCDDIALOG 10005
 #define SYMBOL_PCD_STYLE                                      \
   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX

 #define SYMBOL_PCD_TITLE _("Send to GPS")
 #define SYMBOL_PCD_IDNAME ID_PCDDIALOG
 #define SYMBOL_PCD_SIZE wxSize(500, 500)
 #define SYMBOL_PCD_POSITION wxDefaultPosition

enum { ID_PCD_CANCEL = 10000, ID_PCD_OK, ID_PCD_CHECK1 };


class PINConfirmDialog : public wxDialog {
  DECLARE_DYNAMIC_CLASS(PINConfirmDialog)
  DECLARE_EVENT_TABLE()

public:
  PINConfirmDialog();
  PINConfirmDialog(wxWindow* parent, wxWindowID id, const wxString& caption,
               const wxString& hint, const wxPoint& pos, const wxSize& size,
               long style);
  ~PINConfirmDialog();

  bool Create(wxWindow* parent, wxWindowID id = SYMBOL_PCD_IDNAME,
              const wxString& caption = SYMBOL_PCD_TITLE,
              const wxString& hint = SYMBOL_PCD_TITLE,
              const wxPoint& pos = SYMBOL_PCD_POSITION,
              const wxSize& size = SYMBOL_PCD_SIZE,
              long style = SYMBOL_PCD_STYLE);
  void SetMessage(const wxString &message);
  void SetText1Message(const wxString &message);

  wxString GetText1Value(){ return m_pText1->GetValue(); }

  void OnCancelClick(wxCommandEvent& event);
  void OnOKClick(wxCommandEvent& event);

private:
  void CreateControls(const wxString& hint);


  wxButton* m_CancelButton;
  wxButton* m_OKButton;
  wxStaticText* premtext;
  wxTextCtrl *m_pText1;
  wxString m_checkbox1_msg;
};

#endif  // guard
