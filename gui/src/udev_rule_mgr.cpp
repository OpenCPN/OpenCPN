/******************************************************************************
 *
 * Project:  OpenCPN
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

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dcclient.h>
#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "udev_rule_mgr.h"
#include "linux_devices.h"
#include "model/logger.h"
#include "gui_lib.h"
#include "model/ocpn_utils.h"

static bool hide_dongle_dialog;
static bool hide_device_dialog;

static const char* const DONGLE_INTRO = _(R"(
An OpenCPN dongle is detected but cannot be used due to missing permissions.

This problem can be fixed by installing a udev rules file. Once installed,
it will ensure that the dongle permissions are OK.
)");

static const char* const FLATPAK_INTRO_TRAILER = _(R"(

On flatpak, this must be done using the manual command instructions below
)");

static const char* const DEVICE_INTRO = _(R"(
The device @DEVICE@ exists but cannot be used due to missing permissions.

This problem can be fixed by installing a udev rules file. Once installed,
the rules file will fix the permissions problem.

It will also create a new device called @SYMLINK@. It is recommended to use
@SYMLINK@ instead of @DEVICE@ to avoid problems with changing device names,
in particular on laptops.
)");

static const char* const HIDE_DIALOG_LABEL =
    _("Do not show this dialog next time");

static const char* const RULE_SUCCESS_MSG = _(R"(
Rule successfully installed. To activate the new rule:
- Exit opencpn.
- Unplug and re-insert the USB device.
- Restart opencpn
)");

static const char* const FLATPAK_INSTALL_MSG = _(R"(
To do after installing the rule according to instructions:
- Exit opencpn.
- Unplug and re-insert the USB device.
- Restart opencpn
)");

static const char* const DEVICE_NOT_FOUND =
    _("The device @device@ can not be found (disconnected?)");

/** The "Dont show this message next time" checkbox. */
struct HideCheckbox : public wxCheckBox {
  HideCheckbox(wxWindow* parent, const char* label, bool* state)
      : wxCheckBox(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize,
                   wxALIGN_LEFT),
        m_state(state) {
    SetValue(*state);
    Bind(wxEVT_CHECKBOX,
         [&](wxCommandEvent& ev) { *m_state = ev.IsChecked(); });
  }

private:
  bool* m_state;
};

/**  Line with  "Don't show this message..." checkbox  */
struct HidePanel : wxPanel {
  HidePanel(wxWindow* parent, const char* label, bool* state)
      : wxPanel(parent) {
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    //hbox->Add(1, 1, 100, wxEXPAND);  // Expanding spacer
    hbox->Add(new HideCheckbox(this, label, state), wxSizerFlags().Expand());
    SetSizer(hbox);
    Fit();
    Show();
  }
};

static const char* const INSTRUCTIONS = "@pkexec@ cp @PATH@ /etc/udev/rules.d";

/** A clickable triangle which controls child window hide/show. */
class HideShowPanel : public wxPanel {
public:
  HideShowPanel(wxWindow* parent, wxWindow* child)
      : wxPanel(parent), m_show(true), m_child(child) {
    m_arrow = new wxStaticText(this, wxID_ANY, "");
    m_arrow->Bind(wxEVT_LEFT_DOWN, [&](wxMouseEvent& ev) { toggle(); });
    if (m_child) {
      toggle();
    }
  }

protected:
  bool m_show;
  wxWindow* m_child;
  wxStaticText* m_arrow;

  void toggle() {
    static const auto ARROW_DOWN = L"\u25BC";
    static const auto ARROW_RIGHT = L"\u25BA";

    m_show = !m_show;
    m_child->Show(m_show);
    m_arrow->SetLabel(m_show ? ARROW_DOWN : ARROW_RIGHT);
    GetGrandParent()->Fit();
    GetGrandParent()->Layout();
  }
};

/**  Manual instructions dynamic display. */
class ManualInstructions : public HideShowPanel {
public:
  ManualInstructions(wxWindow* parent, const char* cmd)
      : HideShowPanel(parent, 0) {
    m_child = get_cmd(parent, cmd);
    toggle();
    auto flags = wxSizerFlags().Expand().Right();

    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    const char* label = _("Manual command line instructions");
    hbox->Add(new wxStaticText(this, wxID_ANY, label), flags);
    hbox->Add(m_arrow);

    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox);
    //auto indent = parent->GetTextExtent("aaa").GetWidth();
    flags = flags.Border(wxLEFT);
    vbox->Add(m_child, flags.ReserveSpaceEvenIfHidden());

    SetSizer(vbox);
    SetAutoLayout(true);
    Show();
  }

private:
  wxTextCtrl* get_cmd(wxWindow* parent, const char* tmpl) {
    std::string cmd(tmpl);
    ocpn::replace(cmd, "@PATH@", get_dongle_rule());
    auto ctrl = new wxTextCtrl(this, wxID_ANY, cmd);
    ctrl->SetEditable(false);
    ctrl->SetMinSize(parent->GetTextExtent(cmd + "aaa"));
    return ctrl;
  }
  wxWindow* m_parent;
};

/** Review rule dynamic display. */
class ReviewRule : public HideShowPanel {
public:
  ReviewRule(wxWindow* parent, const std::string& rule)
      : HideShowPanel(parent, 0) {
    int from = rule[0] == '\n' ? 1 : 0;
    m_child = new wxStaticText(this, wxID_ANY, rule.substr(from));
    toggle();

    auto flags = wxSizerFlags().Expand().Right();
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(this, wxID_ANY, _("Review rule")), flags);
    hbox->Add(m_arrow);

    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox);
    auto indent = parent->GetTextExtent("ABCDE").GetWidth();
    flags = flags.Border(wxLEFT, indent);
    vbox->Add(m_child, flags.ReserveSpaceEvenIfHidden());
    SetSizer(vbox);
    SetAutoLayout(true);
    Show();
  }
};

/** Read and return contents of file with given path. */
static std::string get_rule(const std::string& path) {
  std::ifstream input(path.c_str());
  std::ostringstream buf;
  buf << input.rdbuf();
  input.close();
  if (input.bad()) {
    WARNING_LOG << "Cannot open rule file: " << path;
  }
  return buf.str();
}

/** The dongle "manual instructions" and "Review rule" stuff. */
class DongleInfoPanel : public wxPanel {
public:
  DongleInfoPanel(wxWindow* parent) : wxPanel(parent) {
    std::string cmd(INSTRUCTIONS);
    std::string rule_path(get_dongle_rule());
    ocpn::replace(cmd, "@PATH@", rule_path.c_str());
    ocpn::replace(cmd, "@pkexec@", "sudo");
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(new ManualInstructions(this, cmd.c_str()));
    std::string rule_text = get_rule(rule_path);
    vbox->Add(new ReviewRule(this, rule_text.c_str()));
    SetAutoLayout(true);
    SetSizer(vbox);
  }
};

/** The device "manual instructions" and "Review rule" stuff. */
class DeviceInfoPanel : public wxPanel {
public:
  DeviceInfoPanel(wxWindow* parent, const std::string rule_path)
      : wxPanel(parent) {
    std::string cmd(INSTRUCTIONS);
    ocpn::replace(cmd, "@PATH@", rule_path.c_str());
    ocpn::replace(cmd, "@pkexec@", "sudo");
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(new ManualInstructions(this, cmd.c_str()));
    vbox->Add(new ReviewRule(this, get_rule(rule_path)));
    SetAutoLayout(true);
    SetSizer(vbox);
  }
};

/** Install/Quit buttons bottom-right */
struct Buttons : public wxPanel {
  Buttons(wxWindow* parent, const char* rule_path)
      : wxPanel(parent), m_rule_path(rule_path) {
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    auto flags = wxSizerFlags().Right().Bottom();
    sizer->Add(1, 1, 100, wxEXPAND);  // Expanding spacer
    auto install = new wxButton(this, wxID_ANY, _("Install rule"));
    install->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                  [&](wxCommandEvent& ev) { do_install(); });
    install->Enable(getenv("FLATPAK_ID") == NULL);
    sizer->Add(install, flags);
    auto quit = new wxButton(this, wxID_EXIT, _("Quit"));
    quit->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent& ev) {
      if (getenv("FLATPAK_ID")) {
        auto flags = wxOK | wxICON_INFORMATION;
        auto msg = FLATPAK_INSTALL_MSG;
        OCPNMessageBox(this, msg, _("OpenCPN"), flags);
      }
      dynamic_cast<wxDialog*>(GetParent())->EndModal(0);
    });
    sizer->Add(quit, flags);
    SetSizer(sizer);
    Fit();
    Show();
  }

  void do_install() {
    std::string cmd(INSTRUCTIONS);
    ocpn::replace(cmd, "@PATH@", m_rule_path);
    ocpn::replace(cmd, "@pkexec@", "pkexec");
    int sts = system(cmd.c_str());
    int flags = wxOK | wxICON_WARNING;
    const char* msg = _("Errors encountered installing rule.");
    if (WIFEXITED(sts) && WEXITSTATUS(sts) == 0) {
      msg = RULE_SUCCESS_MSG;
      flags = wxOK | wxICON_INFORMATION;
    }
    OCPNMessageBox(this, msg, _("OpenCPN Info"), flags);
  }

  std::string m_rule_path;
};

/** Main, top-level Dongle udev rule dialog. */
class DongleRuleDialog : public wxDialog {
public:
  DongleRuleDialog(wxWindow* parent)
      : wxDialog(parent, wxID_ANY, _("Manage dongle udev rule")) {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Expand();
    std::string intro(DONGLE_INTRO);
    if (getenv("FLATPAK_ID")) {
      intro += FLATPAK_INTRO_TRAILER;
    }
    sizer->Add(new wxStaticText(this, wxID_ANY, intro), flags);
    sizer->Add(new wxStaticLine(this), flags);
    sizer->Add(new DongleInfoPanel(this), flags);
    sizer->Add(new HidePanel(this, HIDE_DIALOG_LABEL, &hide_dongle_dialog),
               flags.Left());
    sizer->Add(new wxStaticLine(this), flags);
    sizer->Add(new Buttons(this, get_dongle_rule().c_str()), flags);
    SetSizer(sizer);
    SetAutoLayout(true);
    Fit();
  }
};

/** Return an intro based on DEVICE_INTRO with proper substitutions. */
static std::string get_device_intro(const char* device, std::string symlink) {
  std::string intro(DEVICE_INTRO);
  ocpn::replace(symlink, "/dev/", "");
  while (intro.find("@SYMLINK@") != std::string::npos) {
    ocpn::replace(intro, "@SYMLINK@", symlink);
  }
  std::string dev_name(device);
  ocpn::replace(dev_name, "/dev/", "");
  while (intro.find("@DEVICE@") != std::string::npos) {
    ocpn::replace(intro, "@DEVICE@", dev_name.c_str());
  }
  if (getenv("FLATPAK_ID")) {
    intro += FLATPAK_INTRO_TRAILER;
  }
  return intro;
}

/** Main, top-level device udev rule dialog. */
class DeviceRuleDialog : public wxDialog {
public:
  DeviceRuleDialog(wxWindow* parent, const char* device_path)
      : wxDialog(parent, wxID_ANY, _("Manage device udev rule")) {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Expand();

    std::string symlink(make_udev_link());
    auto intro = get_device_intro(device_path, symlink.c_str());
    auto rule_path = get_device_rule(device_path, symlink.c_str());
    sizer->Add(new wxStaticText(this, wxID_ANY, intro), flags);
    sizer->Add(new wxStaticLine(this), flags);
    sizer->Add(new DeviceInfoPanel(this, rule_path), flags);
    sizer->Add(new HidePanel(this, HIDE_DIALOG_LABEL, &hide_device_dialog),
               flags.Right());
    sizer->Add(new wxStaticLine(this), flags);
    sizer->Add(new Buttons(this, rule_path.c_str()), flags);

    SetSizer(sizer);
    SetAutoLayout(true);
    Fit();
  }
};

bool CheckSerialAccess(wxWindow* parent, const std::string device) {
  if (hide_device_dialog) {
    return true;
  }
  if (!ocpn::exists(device)) {
    std::string msg(DEVICE_NOT_FOUND);
    ocpn::replace(msg, "@device@", device);
    OCPNMessageBox(parent, msg, _("OpenCPN device error"));
    return false;
  }
  int result = 0;
  if (!is_device_permissions_ok(device.c_str())) {
    auto dialog = new DeviceRuleDialog(parent, device.c_str());
    result = dialog->ShowModal();
    delete dialog;
  }
  return result == 0;
}

bool CheckDongleAccess(wxWindow* parent) {
  int result = 0;
  if (is_dongle_permissions_wrong() && !hide_dongle_dialog) {
    auto dialog = new DongleRuleDialog(parent);
    result = dialog->ShowModal();
    delete dialog;
  }
  return result == 0;
}
