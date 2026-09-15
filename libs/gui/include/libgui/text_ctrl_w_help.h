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
 **************************************************************************/

/**
 * A wxTextCtrl with an initial italics help text, removed when user starts
 * typing.   \image html ./text_entry_help.png
 */
class TextCtrlWithHelp : public wxTextCtrl {
public:
  TextCtrlWithHelp(wxWindow* parent, const std::string& help_text);

  /** Ignored if pristine, use ChangeValue if need be. */
  void SetValue(const wxString& value) override;

  /** Falsify pristine state. */
  void ChangeValue(const wxString& value) override;

  /** Restore help text to initial value, enter pristine state */
  void RestoreHelp() ;

  /** Set help text, enter pristine state */
  void SetHelp(const std::string& help_text);

  /** Return true if user has not entered anything. */
  bool IsPristine() const { return !m_is_inited; }

private:
  const wxFont m_font;
  bool m_is_inited;
  std::string m_help;

  void OnKeypress(wxCommandEvent& ev);

};
