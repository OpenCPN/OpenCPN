/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
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

#include <wx/sizer.h>

#include "plug_settings.h"
#include "OCPNPlatform.h"

extern OCPNPlatform*            g_Platform;

/** Top-level plugin settings dialog. */
PluginSettingsDialog::PluginSettingsDialog(wxWindow* parent)
    :wxDialog(parent, wxID_ANY, _("Plugin Catalog Settings"),
              wxDefaultPosition , wxDefaultSize,
              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    auto vbox = new wxBoxSizer(wxVERTICAL);

    // The list has no natural height. Allocate 8 lines of text so some
    // items are displayed initially in Layout()
    int min_height = GetTextExtent("abcdefghijklmnopqrst").GetHeight() * 10;

    // There seem to be no way have dynamic, wrapping text:
    // https://forums.wxwidgets.org/viewtopic.php?f=1&t=46662
    //int width = GetParent()->GetClientSize().GetWidth();
    //  SetMinClientSize(wxSize(width, min_height));
    int width = GetTextExtent("abcdefghijklmnopqrst").GetWidth() * 10;
    width = wxMin(width, g_Platform->getDisplaySize().x);
    SetMinSize(wxSize(width, min_height));
 
    SetSizer(vbox);
    Fit();
    Layout();
}
