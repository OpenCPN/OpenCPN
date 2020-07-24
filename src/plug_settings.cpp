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

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "plug_settings.h"


class CustomCatalogCtrl: public wxTextCtrl
{
    public:
        CustomCatalogCtrl(wxWindow* parent): wxTextCtrl(parent, wxID_ANY, "")
        {
        }

};


class PlatformChoice: public wxChoice
{
    public:
        PlatformChoice(wxWindow* parent): wxChoice()
        {
            wxArrayString labels;
            labels.Add("ubuntu-x86_64");
            labels.Add("Debian-x86_64");
            labels.Add("Raspbian-armhf");
            Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, labels);
            Layout();
        }
};


class CatalogListBox: public wxChoice
{
    public:
        CatalogListBox(wxWindow* parent): wxChoice()
        {
            wxArrayString labels;
            labels.Add("Master");
            labels.Add("Beta");
            labels.Add("Alpha");
            labels.Add("Custom");
            Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, labels);
            Layout();
        }
};


class CatalogSizer: public wxStaticBoxSizer
{
    public:
        CatalogSizer(wxWindow* parent)
           : wxStaticBoxSizer(wxHORIZONTAL, parent, _("Active catalog"))
        {
            auto flags = wxSizerFlags().Border();
            Add(new wxStaticText(parent, wxID_ANY, _("Select plugin catalog")),
                flags);
            Add(new CatalogListBox(parent),flags);
            Add(new CustomCatalogCtrl(parent), flags.Expand().Proportion(1));
            Layout();
        }
};


class CompatSizer: public wxStaticBoxSizer
{
    public:
        CompatSizer(wxWindow* parent)
           : wxStaticBoxSizer(wxHORIZONTAL, parent, _("Compatibility"))
        {
            auto flags = wxSizerFlags().Border();
            Add(new wxStaticText(parent, wxID_ANY, _("Plugin active flavour")),
                flags);
            Add(new wxStaticText(parent, wxID_ANY, "ubuntu-x86_64"), flags);
            Add(new PlatformChoice(parent), flags);
        }
};


/** The OK and Cancel buttons. */
class ButtonsSizer: public wxStdDialogButtonSizer
{
    public:
        ButtonsSizer(wxWindow* parent): wxStdDialogButtonSizer()
        {
            SetAffirmativeButton(new wxButton(parent, wxID_OK));
            SetCancelButton(new wxButton(parent, wxID_CANCEL));
            Realize();
        }
};


/** Top-level plugin settings dialog.  */
PluginSettingsDialog::PluginSettingsDialog(wxWindow* parent)
    :wxDialog(parent, wxID_ANY, _("Plugin Catalog Settings"),
              wxDefaultPosition , wxDefaultSize,
              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    auto vbox = new wxBoxSizer(wxVERTICAL);

    //vbox->Add(new GridSizer(this), wxSizerFlags().Expand().DoubleBorder());
    vbox->Add(new CatalogSizer(this), wxSizerFlags().Expand().Border());
    vbox->Add(new CompatSizer(this), wxSizerFlags().Expand().DoubleBorder());
    vbox->Add(new ButtonsSizer(this), wxSizerFlags().Expand().DoubleBorder());
 
    SetSizer(vbox);
    Fit();
    Layout();
    SetMinSize(GetSize());
}
