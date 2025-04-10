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

#include "button_switch.h"
#include "ui_utils.h"

SwitchButton::SwitchButton(wxWindow* parent, int key, bool value)
    : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                wxBORDER_NONE),
      m_key(key),
      m_flag(value) {
  SetInitialSize(wxSize(GUI::GetSpacing(this, 6), GUI::GetSpacing(this, 3)));
  Bind(wxEVT_PAINT, &SwitchButton::OnPaint, this);
  Bind(wxEVT_LEFT_DOWN, &SwitchButton::OnToggle, this);
}

int SwitchButton::GetKey() { return m_key; }

bool SwitchButton::IsActive() { return m_flag; }

void SwitchButton::OnToggle(wxMouseEvent& event) {
  m_flag = !m_flag;
  Refresh();
}

void SwitchButton::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(this);

  int slider_width = GetSize().x;
  int slider_height = GetSize().y;
  int button_size = slider_height - 4;

  wxColour borderColor = wxColour(150, 150, 150);
  wxColour shadowColor = wxColour(200, 200, 200);

  // Draw the background with state colour.
  dc.SetBrush(m_flag ? *wxBLUE_BRUSH : *wxGREY_BRUSH);
  dc.SetPen(wxPen(borderColor, 1));
  dc.DrawRoundedRectangle(0, 0, slider_width, slider_height, button_size / 2);

  // Draw the switch knob
  int x_offset = m_flag ? slider_width - button_size - 2 : 2;
  dc.SetBrush(wxBrush(wxColour(255, 255, 255)));
  dc.SetPen(wxPen(shadowColor, 1));
  dc.DrawRoundedRectangle(x_offset, 2, button_size, button_size,
                          button_size / 2);
}
