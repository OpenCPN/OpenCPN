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
#ifndef BUTTON_SWITCH_H
#define BUTTON_SWITCH_H

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/graphics.h>

/**
 * On/Off switch button.
 */
class SwitchButton : public wxControl {
public:
  /**
   * Toggle key on/off.
   * @param parent Parent window.
   * @param key Key identifier.
   * @param value Initial state.
   */
  SwitchButton(wxWindow* parent, int key, bool value = true);

  // Get the key identifier.
  int GetKey();

  // Return active flag.
  bool IsActive();

private:
  int m_key;
  bool m_flag;

  void OnToggle(wxMouseEvent& event);
  void OnPaint(wxPaintEvent& event);
};

#endif  // BUTTON_SWITCH_H
