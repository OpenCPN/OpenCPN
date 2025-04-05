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
#include <wx/panel.h>

#include "dialog_input.h"
#include "form_grid.h"

InputDialog::InputDialog(wxWindow* parent, const std::string& title,
                         const std::string& action)
    : AlertDialog(parent, title, action) {
  FormGrid* sizer = new FormGrid(this);
  sizer->SetHGap(GUI::GetSpacing(this, 4));
  m_grid = new wxPanel(this);
  m_grid->SetSizer(sizer);
  m_content->Add(m_grid);
}

SwitchField* InputDialog::AddSelection(int key, const std::string& label,
                                       bool value) {
  return new SwitchField(m_grid, key, label, value);
}

GUI::KeySet InputDialog::GetSelected() {
  wxWindowList children = m_grid->GetChildren();
  GUI::KeySet selected;

  int numChildren = children.GetCount();
  for (int i = 0; i < numChildren; ++i) {
    auto* select = dynamic_cast<SwitchField*>(children[i]);
    if (select && select->IsActive()) selected.addKey(select->GetKey());
  }
  return selected;
}
