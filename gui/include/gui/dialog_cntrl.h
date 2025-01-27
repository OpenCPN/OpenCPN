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

/**
 * \file
 * Dialog control classes and validator base classes.
 */
#ifndef DIALOG_CNTRL_H
#define DIALOG_CNTRL_H

#include <wx/wx.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>

/**
 * Text field with validator and error handler.
 */
class TextField : public wxTextCtrl {
public:
  TextField(wxWindow* parent, wxWindowID id = wxID_ANY,
            const wxString& value = "", const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0);

  /**
   * Shows an error below the text field.
   * @param msg Error message.
   */
  void OnError(const wxString& msg);

  void SetValidator(const wxValidator& validator = wxDefaultValidator) override;

  /**
   * Text changed event handler.
   * @param event Change event.
   */
  void OnTextChanged(wxCommandEvent& event);

private:
  wxSizer* m_sizer;
  wxStaticText* m_error_text;
};

/**
 * Base class for member to implement validation rules.
 */
class TextValidator : public wxTextValidator {
public:
  bool Validate(wxWindow* parent) override;

  /**
   * Member to implement the instance clone.
   */
  virtual wxValidator* Clone() const override = 0;

  /**
   * Member to implement the validation rules.
   */
  virtual wxString IsValid(const wxString& val) const override = 0;
};

#endif  // DIALOG_CNTRL_H
