/******************************************************************************
 *
 * Project:  OpenCPN
 *
 * Purpose: Warning message for platforms almost without plugins
 *
 ***************************************************************************
 *   Copyright (C) 2021 Alec Leamas                                        *
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

#include "config.h"

#include <string>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "check_platform.h"

extern bool g_hide_no_plugins_dlg;

static const char* const MESSAGE = _(R"""(
Unfortunately, there are not many plugins
available for this OpenCPN package. However,
by installing OpenCPN using Flatpak instead
more plugins are supported. See the Getting
Started chapter in the User Manual.
)""");

static const char* const HIDE_DIALOG_LABEL =
    _("Do not show this dialog next time");

/** We warn if number of plugins (including system ones) is below this. */
static const unsigned SANE_PLUGIN_COUNT = 10;

/** The "Dont show this message next time" checkbox. */
struct HideCheckbox : public wxCheckBox {
  HideCheckbox(wxWindow* parent, const char* label)
      : wxCheckBox(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize,
                   wxALIGN_RIGHT) {
    SetValue(g_hide_no_plugins_dlg);
    Bind(wxEVT_CHECKBOX,
         [&](wxCommandEvent& ev) { g_hide_no_plugins_dlg = ev.IsChecked(); });
  }
};

/**  Line with  "Don't show this message..." checkbox  */
struct HidePanel : wxPanel {
  HidePanel(wxWindow* parent, const char* label)
      : wxPanel(parent) {
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(1, 1, 100, wxEXPAND);  // Expanding spacer
    hbox->Add(new HideCheckbox(this, label), wxSizerFlags().Expand());
    SetSizer(hbox);
    Fit();
    Show();
  }
};

/** Bottom panel with OK button. */
struct Buttons : public wxPanel  {
  Buttons(wxWindow* parent): wxPanel(parent) {
    // The natural height seems broken on GTK:
    auto size = GetTextExtent("OK");
    size.SetHeight(size.GetHeight() * 3);
    SetMinSize(size);

    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    auto flags = wxSizerFlags().Right().Bottom().Border();
    hbox->Add(1, 1, 100, wxEXPAND);  // Expanding spacer
    auto ok = new wxButton(this, wxID_OK);
    hbox->Add(ok, flags);
    SetSizer(hbox);
    Fit();
    Show();
  }
};


/** Main top-level dialog. */
class NoPluginsDialog : public wxDialog {
public:
  NoPluginsDialog(wxWindow* parent)
      : wxDialog(parent, wxID_ANY, _("Plugins unavailable")) {
    // Make window at least 50 char wide to accomodate MESSAGE
    auto size = GetTextExtent("abcdeFGHIJ0");
    size.SetWidth(size.GetWidth() * 4);
    SetMinSize(size);

    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Expand().Border();
    sizer->Add(new wxStaticText(this, wxID_ANY, MESSAGE), flags);
    sizer->Add(new wxStaticLine(this), flags);
    sizer->Add(new HidePanel(this, HIDE_DIALOG_LABEL), flags.Right());
    sizer->Add(new wxStaticLine(this), wxSizerFlags().Expand());
    sizer->Add(new Buttons(this), wxSizerFlags().Border().Expand());
    SetSizer(sizer);
    Fit();
    Layout();
    Show();
  }
};

#ifdef __linux__
void check_platform(wxWindow* parent, unsigned nr_of_plugins) {
  using namespace std;

  if (nr_of_plugins >= SANE_PLUGIN_COUNT) return;
  if (g_hide_no_plugins_dlg) return;   // "Do not show this message again"
  static const string t(PKG_TARGET);
  if (t.find("flatpak") != string::npos) return;

  // Are we on an architecture supported by Flatpak?
  bool fp_arch = t.find("aarch64") != string::npos;
  fp_arch |= t.find("arm64") != string::npos;
  fp_arch |= t.find("x86_64") != string::npos;
  if (!fp_arch) return;

  NoPluginsDialog dlg(parent);
  dlg.ShowModal();
}

#else
void check_platform(wxWindow* w, unsigned u) {}
#endif
