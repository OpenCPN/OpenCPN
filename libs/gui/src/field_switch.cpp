
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
#include "field_switch.h"
#include "form_grid.h"

SwitchField::SwitchField(wxWindow* parent, int key, const std::string& label,
                         bool value)
    : SwitchButton(parent, key, value) {
  auto* grid = dynamic_cast<FormGrid*>(parent->GetSizer());
  assert(grid && "SwitchField: Invalid parent sizer");
  wxStaticText* text_label = new wxStaticText(parent, wxID_ANY, label);
  grid->Add(text_label, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));
  grid->Add(this);
}
