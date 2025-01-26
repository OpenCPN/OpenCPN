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

/**
 * OpenCPN standard dialog layout with content sizer.
 */
class BaseDialog : public wxDialog {
public:
  static const int kScaling = 6;  // UI guideline default

  BaseDialog(wxWindow* parent, const std::string& title,
             long style = wxDEFAULT_DIALOG_STYLE);

  void AddHtmlContent(const wxString& html);

protected:
  wxBoxSizer* m_layout;
  wxBoxSizer* m_content;

  static const int kDialogPadding = 12;
};

#endif  // DIALOG_BASE_H