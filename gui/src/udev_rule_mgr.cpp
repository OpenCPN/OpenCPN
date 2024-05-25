
/**************************************************************************
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
 ***************************************************************************/
/** \file udev_rule_mgr.cpp Implement udev_rule_mgr.h */

#include "config.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <vector>

#include <stdlib.h>

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

#include "model/linux_devices.h"
#include "model/logger.h"
#include "model/ocpn_utils.h"

#include "gui_lib.h"
#include "udev_rule_mgr.h"

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
)");

static const char* const DEVICE_LINK_INTRO = _(R"(

It will also create a new device called @SYMLINK@. It is recommended to use
@SYMLINK@ instead of @DEVICE@ to avoid problems with changing device names,
in particular on laptops.
)");

static const char* const HIDE_DIALOG_LABEL =
    _("Do not show this dialog next time");

static const char* const RULE_SUCCESS_TTYS_MSG = _(R"(
Rule successfully installed. To activate the new rule restart the system.
)");

static const char* const RULE_SUCCESS_MSG = _(R"(
Rule successfully installed. To activate the new rule restart system or:
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

static const char* const INSTRUCTIONS = "@pkexec@ cp @PATH@ /etc/udev/rules.d";

/** The modeless "Device not found" dialog. */
class DeviceNotFoundDlg : public wxFrame {
public:
  /** Construct and show a dialog for given device. */
  static void Create(wxWindow* parent, const std::string& device) {
    wxWindow* dlg = new DeviceNotFoundDlg(parent, device);
    dlg->Show();
  }

  /** Destroy all open dialog windows, overall destructor helper. */
  static void DestroyOpenWindows() {
    for (const auto& name : open_windows) {
      auto window = wxWindow::FindWindowByName(name);
      if (window) window->Destroy();
    }
    open_windows.clear();
  }

private:
  static std::vector<std::string> open_windows;

  class ButtonsSizer : public wxStdDialogButtonSizer {
  public:
    ButtonsSizer(DeviceNotFoundDlg* parent) : wxStdDialogButtonSizer() {
      auto button = new wxButton(parent, wxID_OK);
      AddButton(button);
      Realize();
    }
  };

  DeviceNotFoundDlg(wxWindow* parent, const std::string& device)
      : wxFrame(parent, wxID_ANY, _("Opencpn: device not found"),
                wxDefaultPosition, wxDefaultSize,
                wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT) {
    std::stringstream ss;
    ss << "dlg-id-" << rand();
    SetName(ss.str());
    open_windows.push_back(ss.str());

    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& e) {
      OnClose();
      e.Skip();
    });
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent&) { OnClose(); });

    auto vbox = new wxBoxSizer(wxVERTICAL);
    SetSizer(vbox);
    auto flags = wxSizerFlags().Expand().Border();
    std::string txt(DEVICE_NOT_FOUND);
    ocpn::replace(txt, "@device@", device);
    vbox->Add(0, 0, 1);  // vertical space
    vbox->Add(new wxStaticText(this, wxID_ANY, txt), flags);
    vbox->Add(0, 0, 1);
    vbox->Add(new wxStaticLine(this), wxSizerFlags().Expand());
    vbox->Add(new ButtonsSizer(this), flags);
    Layout();
    CenterOnScreen();
    SetFocus();
  }

  void OnClose() {
    const std::string name(GetName().ToStdString());
    auto found =
        std::find_if(open_windows.begin(), open_windows.end(),
                     [name](const std::string& s) { return s == name; });
    assert(found != std::end(open_windows) &&
           "Cannot find dialog in window list");
    open_windows.erase(found);
    Destroy();
  }
};

std::vector<std::string> DeviceNotFoundDlg::open_windows;

void DestroyDeviceNotFoundDialogs() { DeviceNotFoundDlg::DestroyOpenWindows(); }

/** The "Dont show this message next time" checkbox. */
class HideCheckbox : public wxCheckBox {
public:
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
class HidePanel : public wxPanel {
public:
  HidePanel(wxWindow* parent, const char* label, bool* state)
      : wxPanel(parent) {
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new HideCheckbox(this, label, state), wxSizerFlags().Expand());
    SetSizer(hbox);
    Fit();
    Show();
  }
};

/** A clickable triangle which controls child window hide/show. */
class HideShowPanel : public wxPanel {
public:
  HideShowPanel(wxWindow* parent, wxWindow* child)
      : wxPanel(parent), m_show(true), m_child(child) {
    m_arrow = new wxStaticText(this, wxID_ANY, "");
    m_arrow->Bind(wxEVT_LEFT_DOWN, [&](wxMouseEvent& ev) { Toggle(); });
    if (m_child) {
      Toggle();
    }
  }

protected:
  bool m_show;
  wxWindow* m_child;
  wxStaticText* m_arrow;

  void Toggle() {
    static const auto ARROW_DOWN = L"\u25BC";
    static const auto ARROW_RIGHT = L"\u25BA";

    m_show = !m_show;
    m_child->Show(m_show);
    m_arrow->SetLabel(std::string(" ") + (m_show ? ARROW_DOWN : ARROW_RIGHT));
    GetGrandParent()->Fit();
    GetGrandParent()->Layout();
  }
};

/**  Manual instructions dynamic display. */
class ManualInstructions : public HideShowPanel {
public:
  ManualInstructions(wxWindow* parent, const char* cmd)
      : HideShowPanel(parent, 0) {
    m_child = GetCmd(parent, cmd);
    Toggle();
    auto flags = wxSizerFlags().Expand();

    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    const char* label = _("Manual command line instructions");
    hbox->Add(new wxStaticText(this, wxID_ANY, label), flags);
    hbox->Add(m_arrow);

    auto vbox = new wxBoxSizer(wxVERTICAL);

    vbox->Add(hbox);
    flags = flags.Border(wxLEFT);
    vbox->Add(m_child, flags.ReserveSpaceEvenIfHidden());

    SetSizer(vbox);
    SetAutoLayout(true);
    Show();
  }

private:
  wxTextCtrl* GetCmd(wxWindow* parent, const char* tmpl) {
    std::string cmd(tmpl);
    ocpn::replace(cmd, "@PATH@", GetDongleRule());
    auto ctrl = new CopyableText(this, cmd.c_str());
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
    Toggle();

    auto flags = wxSizerFlags().Expand();
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
static std::string GetRule(const std::string& path) {
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
    std::string rule_path(GetDongleRule());
    ocpn::replace(cmd, "@PATH@", rule_path.c_str());
    ocpn::replace(cmd, "@pkexec@", "sudo");
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(new ManualInstructions(this, cmd.c_str()));
    std::string rule_text = GetRule(rule_path);
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
    vbox->Add(new ReviewRule(this, GetRule(rule_path)));
    SetAutoLayout(true);
    SetSizer(vbox);
  }
};

/** Install/Quit buttons bottom-right */
class Buttons : public wxPanel {
public:
  Buttons(wxWindow* parent, const char* rule_path)
      : wxPanel(parent), m_rule_path(rule_path) {
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    auto flags = wxSizerFlags().Bottom().Border(wxLEFT);
    sizer->Add(1, 1, 100, wxEXPAND);  // Expanding spacer
    auto install = new wxButton(this, wxID_ANY, _("Install rule"));
    install->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                  [&](wxCommandEvent& ev) { DoInstall(); });
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

  void DoInstall() {
    using namespace std;
    string cmd(INSTRUCTIONS);
    ocpn::replace(cmd, "@PATH@", m_rule_path);
    ocpn::replace(cmd, "@pkexec@", "sudo");
    ifstream f(m_rule_path);
    auto rule =
        string(istreambuf_iterator<char>(f), istreambuf_iterator<char>());
    int sts = system(cmd.c_str());
    int flags = wxOK | wxICON_WARNING;
    const char* msg = _("Errors encountered installing rule.");
    if (WIFEXITED(sts) && WEXITSTATUS(sts) == 0) {
      if (rule.find("ttyS") != std::string::npos) {
        msg = RULE_SUCCESS_TTYS_MSG;
      } else {
        msg = RULE_SUCCESS_MSG;
      }
      flags = wxOK | wxICON_INFORMATION;
    }
    OCPNMessageBox(this, msg, _("OpenCPN Info"), flags);
  }

private:
  std::string m_rule_path;
};

/** Main, top-level Dongle udev rule dialog. */
class DongleRuleDialog : public wxDialog {
public:
  DongleRuleDialog(wxWindow* parent)
      : wxDialog(parent, wxID_ANY, _("Manage dongle udev rule")) {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Expand().Border();
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
    sizer->Add(new Buttons(this, GetDongleRule().c_str()), flags);
    SetSizer(sizer);
    SetAutoLayout(true);
    Fit();
  }
};

/** Return an intro based on DEVICE_INTRO with proper substitutions. */
static std::string GetDeviceIntro(const char* device, std::string symlink) {
  std::string intro(DEVICE_INTRO);

  std::string dev_name(device);
  ocpn::replace(dev_name, "/dev/", "");
  if (!ocpn::startswith(dev_name, "ttyS")) {
    intro += DEVICE_LINK_INTRO;
  }
  if (getenv("FLATPAK_ID")) {
    intro += FLATPAK_INTRO_TRAILER;
  }
  ocpn::replace(symlink, "/dev/", "");
  while (intro.find("@SYMLINK@") != std::string::npos) {
    ocpn::replace(intro, "@SYMLINK@", symlink);
  }
  while (intro.find("@DEVICE@") != std::string::npos) {
    ocpn::replace(intro, "@DEVICE@", dev_name.c_str());
  }
  return intro;
}

/** Main, top-level device udev rule dialog. */
class DeviceRuleDialog : public wxDialog {
public:
  DeviceRuleDialog(wxWindow* parent, const char* device_path)
      : wxDialog(parent, wxID_ANY, _("Manage device udev rule")) {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Expand().Border();

    std::string symlink(MakeUdevLink());
    auto intro = GetDeviceIntro(device_path, symlink.c_str());
    auto rule_path = GetDeviceRule(device_path, symlink.c_str());
    sizer->Add(new wxStaticText(this, wxID_ANY, intro), flags);
    sizer->Add(new wxStaticLine(this), flags);
    sizer->Add(new DeviceInfoPanel(this, rule_path), flags);
    sizer->Add(new HidePanel(this, HIDE_DIALOG_LABEL, &hide_device_dialog),
               flags);
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
    DeviceNotFoundDlg::Create(parent, device);
    return false;
  }
  int result = 0;
  if (!IsDevicePermissionsOk(device.c_str())) {
    auto dialog = new DeviceRuleDialog(parent, device.c_str());
    result = dialog->ShowModal();
    delete dialog;
  }
  return result == 0;
}

bool CheckDongleAccess(wxWindow* parent) {
  int result = 0;
  if (IsDonglePermissionsWrong() && !hide_dongle_dialog) {
    auto dialog = new DongleRuleDialog(parent);
    result = dialog->ShowModal();
    delete dialog;
  }
  return result == 0;
}
