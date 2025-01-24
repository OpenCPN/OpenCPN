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

#include <memory>

#include <wx/wx.h>
#include <wx/string.h>
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

  int GetSizerIndex(wxSizer* sizer);

  /**
   * Error handler shows an error with the text field.
   * When the sizer position is found the error is shown
   * under the text field, otherwise with a popup message.
   * @param msg Error message.
   */
  void onError(const wxString& msg);

  void SetValidator(const wxValidator& validator = wxDefaultValidator) override;

  /**
   * Text changed event handler.
   * @param event Change event.
   */
  void OnTextChanged(wxCommandEvent& event);

private:
  std::unique_ptr<wxStaticText> m_errorText;
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
