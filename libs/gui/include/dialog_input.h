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
#ifndef DIALOG_INPUT_H
#define DIALOG_INPUT_H

#include <set>
#include <wx/panel.h>

#include "dialog_alert.h"
#include "field_switch.h"
#include "ui_utils.h"

/**
 * Input dialog with panel and flex grid sizer.
 */
class InputDialog : public AlertDialog {
public:
  InputDialog(wxWindow* parent, const std::string& title,
              const std::string& action = _("Continue").ToStdString());

  /**
   * Add key selection switch.
   * @param key Key identifier.
   * @param label Selection label.
   * @param value Default value.
   * @return Selection field.
   */
  SwitchField* AddSelection(int key, const std::string& label, bool value);

  /**
   * Get selected keys.
   */
  GUI::KeySet GetSelected();

private:
  wxPanel* m_grid;
};

#endif  // DIALOG_INPUT_H
