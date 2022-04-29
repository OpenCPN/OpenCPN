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

#include <map>
#include <sstream>
#include <string>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/log.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "cat_settings.h"
#include "config_var.h"
#include "ocpn_utils.h"
#include "plugin_cache.h"
#include "PluginHandler.h"

extern wxString g_catalog_channel;
extern wxString g_catalog_custom_url;
extern wxString g_compatOS;
extern wxString g_compatOsVersion;

/** The custom URL text entry. */
class CustomCatalogCtrl : public wxTextCtrl {
public:
  CustomCatalogCtrl(wxWindow* parent) : wxTextCtrl(parent, wxID_ANY, "") {
    SetValue(g_catalog_custom_url);
    Bind(wxEVT_TEXT,
         [&](wxCommandEvent&) { g_catalog_custom_url = GetValue(); });
  }
};

/** Select compatibility drop-down. */
class PlatformChoice : public wxChoice {
public:
  PlatformChoice(wxWindow* parent, wxStaticText* selected)
      : wxChoice(), m_selected(selected) {
    Bind(wxEVT_CHOICE, &PlatformChoice::OnChoice, this);
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, getLabels());
    SetSelection(0);
    Layout();
  }

private:
  wxStaticText* m_selected;

  void OnChoice(wxCommandEvent&) {
    ocpn::GlobalVar<wxString> compat_os(&g_compatOS);
    if (GetSelection() == 0) {
      // "Select new flavour"
      return;
    }
    if (GetSelection() == 1) {
      // "Default Setting"
      g_compatOsVersion = "";
      compat_os.set("");
      auto newOS = CompatOs::getInstance();
      m_selected->SetLabel(newOS->name() + ":" + newOS->version());
    } else {
      auto current = GetString(GetSelection());
      auto os = ocpn::split(current, " ")[0];
      m_selected->SetLabel(os);
      compat_os.set(ocpn::split(os.c_str(), ":")[0]);
      g_compatOsVersion = ocpn::split(os.c_str(), ":")[1];
    }
  }

  wxArrayString getLabels() {
    auto plug_handler = PluginHandler::getInstance();
    wxArrayString labels;
    labels.Add(_("Select new flavour"));
    labels.Add(_("Default setting"));
    for (const auto& c : plug_handler->getCountByTarget()) {
      std::stringstream ss;
      ss << c.first << "   (" << c.second << ")";
      labels.Add(ss.str());
    }
    return labels;
  }
};

/** Select master, beta, alpha, custom drop-down. */
class CatalogChoice : public wxChoice {
public:
  CatalogChoice(wxWindow* parent, wxTextCtrl* custom_ctrl)
      : wxChoice(), m_custom_ctrl(custom_ctrl) {
    static const std::vector<std::string> labels(
        {"master", "Beta", "Alpha", "custom"});
    wxArrayString wxLabels;
    for (const auto& l : labels) wxLabels.Add(l);
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLabels);

    m_custom_ctrl->Enable(false);
    for (auto l = labels.begin(); l != labels.end(); l++) {
      if (g_catalog_channel == *l) {
        SetSelection(l - labels.begin());
      }
    }
    wxCommandEvent ev;
    OnChoice(ev);
    Layout();
    Bind(wxEVT_CHOICE, &CatalogChoice::OnChoice, this);
  }

private:
  wxTextCtrl* m_custom_ctrl;

  void OnChoice(wxCommandEvent&) {
    auto selected = GetString(GetSelection());
    m_custom_ctrl->Enable(selected == "custom");
    if (selected == "custom") {
      m_custom_ctrl->Show();
      GetParent()->Layout();
      m_custom_ctrl->SetFocus();
      g_catalog_custom_url = m_custom_ctrl->GetValue();
    } else {
      m_custom_ctrl->Hide();
    }
    ocpn::GlobalVar<wxString> catalog(&g_catalog_channel);
    catalog.set(selected);
    Layout();
  }
};

/** Current selected compatibility. */
class CompatText : public wxStaticText {
public:
  CompatText(wxWindow* parent) : wxStaticText(parent, wxID_ANY, "") {
    auto compatOs = CompatOs::getInstance();
    SetLabel(compatOs->name() + ":" + compatOs->version());
  }
};

/** Catalog channel selection panel. */
class CatalogSizer : public wxStaticBoxSizer {
public:
  CatalogSizer(wxWindow* parent)
      : wxStaticBoxSizer(wxHORIZONTAL, parent, _("Active catalog")) {
    auto flags = wxSizerFlags().Border();
    Add(new wxStaticText(parent, wxID_ANY, _("Select plugin catalog")), flags);
    auto custom_ctrl = new CustomCatalogCtrl(parent);
    Add(new CatalogChoice(parent, custom_ctrl), flags);
    Add(custom_ctrl, flags.Expand().Proportion(1));
    Layout();
  }
};

/** Plugin compatibility panel. */
class CompatSizer : public wxStaticBoxSizer {
public:
  CompatSizer(wxWindow* parent)
      : wxStaticBoxSizer(wxHORIZONTAL, parent, _("Compatibility")) {
    auto flags = wxSizerFlags().Border();
    Add(new wxStaticText(parent, wxID_ANY, _("Active setting:")),
        flags.Center());
    auto status_text = new CompatText(parent);
    Add(status_text, flags.Center().Proportion(1));
    Add(new PlatformChoice(parent, status_text), flags);
  }
};

/* Cache panel, size feedback and clear button. */
class CacheSizer : public wxStaticBoxSizer {
public:
  CacheSizer(wxWindow* parent)
      : wxStaticBoxSizer(wxHORIZONTAL, parent, _("Cache")) {
    using CmdEvt = wxCommandEvent;

    auto flags = wxSizerFlags().Border();
    m_label = new wxStaticText(parent, wxID_ANY, "");
    update_label();
    Add(m_label, flags.Center().Proportion(1));

    Add(1, 1, 1, wxEXPAND);  // Expanding spacer
    m_clear_button = new wxButton(parent, wxID_ANY, _("Clear cache"));
    Add(m_clear_button, flags);
    m_clear_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                         [=](CmdEvt& e) { on_clear_btn_clicked(); });
  }

private:
  wxButton* m_clear_button;
  wxStaticText* m_label;

  void on_clear_btn_clicked() {
    ocpn::cache_clear();
    update_label();
  }

  void update_label() {
    char buf[128];
    snprintf(buf, sizeof(buf), _("Size: %d MB in %d files"), ocpn::cache_size(),
             ocpn::cache_file_count());
    m_label->SetLabel(buf);
  }
};

/** The Done button. */
class ButtonsSizer : public wxStdDialogButtonSizer {
public:
  ButtonsSizer(wxWindow* parent) : wxStdDialogButtonSizer() {
    auto button = new wxButton(parent, wxID_OK, "LongLabel", wxDefaultPosition,
                               wxSize(10 * parent->GetCharWidth(), -1));
    button->SetLabel(_("Done"));
    SetAffirmativeButton(button);
    Realize();
  }
};

/** Top-level plugin settings dialog.  */
CatalogSettingsDialog::CatalogSettingsDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, _("Plugin Catalog Settings"),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
#ifdef __OCPN__ANDROID__
  SetBackgroundColour(wxColour(0x7c, 0xb0, 0xe9));  // light blue
#endif
  auto vbox = new wxBoxSizer(wxVERTICAL);

  vbox->Add(new CatalogSizer(this), wxSizerFlags().Expand().DoubleBorder());
#ifndef __OCPN__ANDROID__
  vbox->Add(new CompatSizer(this), wxSizerFlags().Expand().DoubleBorder());
#endif
  vbox->Add(new CacheSizer(this), wxSizerFlags().Expand().DoubleBorder());
  vbox->Add(new ButtonsSizer(this), wxSizerFlags().Expand().DoubleBorder());

  SetSizer(vbox);
  Fit();
  Layout();
  SetMinSize(GetSize());
}
