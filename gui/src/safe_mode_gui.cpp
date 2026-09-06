/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Safe start without plugins, basic check and dialog
 */

#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/html/htmlwin.h>

#include "std_filesystem.h"

#include "model/safe_mode.h"

static const char* const kLastRunErrorMsg =
    // clang-format off
  _(R"( <p>The last opencpn run seems to have failed. Do <br/>
        you want to run in safe mode without plugins and <br/>
        other possibly problematic features? </p>
        <br/><br/>
        <p> You may consider visiting the OpenCPN-5.16 <br/>
        <a href="http://repo.opencpn.org/known-issues-5.16.html">
        list of known issues</a>.</p>)");  // clang-format on

namespace {

class HtmlWindow : public wxHtmlWindow {
public:
  explicit HtmlWindow(wxWindow* parent) : wxHtmlWindow(parent, wxID_ANY) {
    std::stringstream html;
    html << "<html><body>" << kLastRunErrorMsg << "</body></html>";
    wxHtmlWindow::SetPage(html.str());
    wxHtmlWindow::Layout();
  }
};

class ButtonSizer : public wxStdDialogButtonSizer {
public:
  explicit ButtonSizer(wxWindow* parent) : wxStdDialogButtonSizer() {
    auto ok_btn = new wxButton(parent, wxID_OK);
    ok_btn->SetLabel(_("Safe restart"));
    SetAffirmativeButton(ok_btn);
    auto cancel_btn = new wxButton(parent, wxID_CANCEL);
    cancel_btn->SetLabel(_("Normal start"));
    SetCancelButton(cancel_btn);
    Realize();
  }
};

/** Close a modal dialog when timeout is reached. */
class ModalDialogTimer : public wxTimer {
public:
  /**
   * Create a timer closing a modal dialog using EndModal() on timeout.
   * @param dialog Client dialog to be closed
   * @param seconds Timeout (seconds)
   * @param exit_value Used as argument to EndModal(exit_value) when
   *     closing dialog.
   */
  ModalDialogTimer(wxDialog* dialog, unsigned seconds, int exit_value)
      : wxTimer(dialog), m_dialog(dialog), m_exit_value(exit_value) {
    StartOnce(static_cast<int>(seconds) * 1000);
  }

  void Notify() override { m_dialog->EndModal(m_exit_value); }

private:
  wxDialog* const m_dialog;
  const int m_exit_value;
};

class SafeModeDialog : public wxDialog {
public:
  SafeModeDialog(wxWindow* parent, unsigned timeout)
      : wxDialog(parent, wxID_ANY, _("Safe Restart")),
        m_timer(new ModalDialogTimer(this, 15, wxID_CANCEL)) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(new HtmlWindow(this), wxSizerFlags(1).Expand());
    vbox->Add(new ButtonSizer(this), wxSizerFlags().Expand().Border());
    SetSizer(vbox);
    wxDialog::Layout();
    wxDialog::Show();

    Bind(wxEVT_CHAR_HOOK, [&](wxKeyEvent& ev) { OnKeyPressed(ev); });
  }

  void StopTimer() const { m_timer->Stop(); }

private:
  wxTimer* m_timer;

  void OnKeyPressed(wxKeyEvent& ev) {
    if (ev.GetKeyCode() == WXK_RETURN) EndModal(wxID_CANCEL);
    ev.DoAllowNextEvent();
    ev.Skip();
  };
};

}  // namespace

namespace safe_mode {
/**
 * Check if the last start failed, possibly invoke user dialog and set
 * global safe mode state.
 */
void CheckLastStart() {
  std::string path = CheckFilePath();
  if (fs::exists(path)) {
    SafeModeDialog dlg(nullptr, 15);
    int result = dlg.ShowModal();
    dlg.StopTimer();
    safe_mode = result != wxID_CANCEL;
  } else {
    std::ofstream dest(path, std::ios::binary);
    dest << "Internal opencpn use\n";
  }
}

}  // namespace safe_mode
