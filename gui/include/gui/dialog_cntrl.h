/***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

/**
 * \file
 * Dialog control classes and validator base classes.
 */
#ifndef DIALOG_CNTRL_H
#define DIALOG_CNTRL_H

#include <wx/wx.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>

/**
 * Text field with support for error messages.
 */
class TextField : public wxTextCtrl {
public:
  TextField(wxWindow* parent, wxWindowID id = wxID_ANY,
            const wxString& value = "", const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0);

  int GetSizerIndex(wxSizer* sizer);
  void onError(const wxString& msg);
  void SetValidator(const wxValidator& validator) override;

  // Text changed event handler
  void OnTextChanged(wxCommandEvent& event);

private:
  wxStaticText* m_errorText;
};

/**
 * Text validator base class. Member must classes implement the IsValid method,
 * and the Clone method to return a new instance of the class.
 */
class TextValidator : public wxTextValidator {
public:
  bool Validate(wxWindow* parent) override;
  virtual wxValidator* Clone() const override = 0;

  // Add validation rules by member
  virtual wxString IsValid(const wxString& val) const override = 0;
};

#endif  // DIALOG_CNTRL_H