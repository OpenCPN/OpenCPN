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
#ifndef DIALOG_BASE_H
#define DIALOG_BASE_H

#include <wx/dialog.h>
#include <wx/string.h>

#include "ui_utils.h"

/**
 * Dialog layout with content sizer.
 */
class BaseDialog : public wxDialog {
public:
  BaseDialog(wxWindow* parent, const std::string& title,
             long style = wxDEFAULT_DIALOG_STYLE);

  /**
   * Initialize by display geometry.
   */
  void SetInitialSize();

  /**
   * Show dialog and return response.
   * @return YES/NO response.
   */
  int ShowModal() override;

  /**
   * Display HTML content.
   * @param html HTML document.
   */
  void AddHtmlContent(const std::stringstream& html);

protected:
  wxBoxSizer* m_layout;
  wxBoxSizer* m_content;

  // padding scaling factor
  static const int kDialogPadding = 2;
};

#endif  // DIALOG_BASE_H
