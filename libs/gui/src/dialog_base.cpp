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
#include <wx/dialog.h>
#include <wx/sizer.h>

#include "dialog_base.h"

/**
 * Base dialog constructor implements the layout with vertical sizer.
 */
BaseDialog::BaseDialog(wxWindow* parent, const std::string& title, long style)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
               style) {
  m_layout = new wxBoxSizer(wxVERTICAL);
  m_content = new wxBoxSizer(wxVERTICAL);
  wxSizerFlags flags = wxSizerFlags();
  flags.Border(wxALL, FromDIP(kDialogPadding));
  m_layout->Add(m_content, flags);
  SetSizer(m_layout);
}