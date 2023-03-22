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

#ifndef __RESTSERVERGUI_H__
#define __RESTSERVERGUI_H__

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>

//    Constants for  Dialog
#define ID_STGDIALOG 10005
#define SYMBOL_STG_STYLE                                      \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX

#define SYMBOL_STG_TITLE _("Send to GPS")
#define SYMBOL_STG_IDNAME ID_STGDIALOG
#define SYMBOL_STG_SIZE wxSize(500, 500)
#define SYMBOL_STG_POSITION wxDefaultPosition

/**
 * "Accept Object" Dialog Definition
 */
class AcceptObjectDialog : public wxDialog {
  DECLARE_DYNAMIC_CLASS(AcceptObjectDialog)
  DECLARE_EVENT_TABLE()

public:
  AcceptObjectDialog();
  AcceptObjectDialog(wxWindow* parent, wxWindowID id, const wxString& caption,
               const wxString& hint, const wxPoint& pos, const wxSize& size,
               long style, const wxString& msg1, const wxString& msg2);
  ~AcceptObjectDialog();

  bool Create(wxWindow* parent, wxWindowID id,
              const wxString& caption,
              const wxString& hint,
              const wxPoint& pos,
              const wxSize& size,
              long style,
              const wxString& msg1, const wxString& msg2);

  void SetMessage(const wxString &message);
  void SetCheck1Message(const wxString &message);

  bool GetCheck1Value(){ return m_pCheck1->GetValue(); }

private:
  void CreateControls(const wxString& hint,
                      const wxString& msg1, const wxString& msg);

  void OnCancelClick(wxCommandEvent& event);
  void OnOKClick(wxCommandEvent& event);

  wxButton* m_CancelButton;
  wxButton* m_OKButton;
  wxStaticText* premtext;
  wxCheckBox *m_pCheck1;
  wxString m_checkbox1_msg;
};

class PINCreateDialog : public wxDialog {
  DECLARE_DYNAMIC_CLASS(PINCreateDialog)
  DECLARE_EVENT_TABLE()

public:
  PINCreateDialog();
  PINCreateDialog(wxWindow* parent, wxWindowID id, const wxString& caption,
               const wxString& hint, const wxPoint& pos, const wxSize& size,
               long style);
  ~PINCreateDialog();

  bool Create(wxWindow* parent, wxWindowID id = SYMBOL_STG_IDNAME,
              const wxString& caption = SYMBOL_STG_TITLE,
              const wxString& hint = SYMBOL_STG_TITLE,
              const wxPoint& pos = SYMBOL_STG_POSITION,
              const wxSize& size = SYMBOL_STG_SIZE,
              long style = SYMBOL_STG_STYLE);
  void SetMessage(const wxString &message);
  void SetText1Message(const wxString &message);

  wxString GetText1Value(){ return m_pText1->GetValue(); }

private:
  void CreateControls(const wxString& hint);

  void OnCancelClick(wxCommandEvent& event);
  void OnOKClick(wxCommandEvent& event);

  wxButton* m_CancelButton;
  wxButton* m_OKButton;
  wxStaticText* premtext;
  wxTextCtrl *m_pText1;
  wxString m_checkbox1_msg;
};

#endif

