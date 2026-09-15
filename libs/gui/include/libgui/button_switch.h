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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/
/**
 *\defgroup UI-tools
 *@{
 */

#ifndef BUTTON_SWITCH_H
#define BUTTON_SWITCH_H

#include <wx/wx.h>
#include <wx/graphics.h>

/**
 * On/Off switch button. Similar semantics as a wxCheckBox with
 * a different UI.  \image html ./button_switch.png
 */
class SwitchButton : public wxControl {
public:
  /**
   * Construct a SwitchButton
   * @param parent Parent window.
   * @param id Window identifier, often wxID_ANY
   * @param value Initial state.
   */
  SwitchButton(wxWindow* parent, int id, bool value = true);

  /** Return on/off state as visible in UI */
  bool GetValue();

  /** Set the internal value. */
  void SetValue(bool value);

private:
  bool m_flag;

  void OnToggle(wxMouseEvent& event);
  void OnPaint(wxPaintEvent& event);
};

#endif  // BUTTON_SWITCH_H

/** ·@} */
