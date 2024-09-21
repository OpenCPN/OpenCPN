/**************************************************************************
 *   Copyright (C) 2024 Alec Leamas                                        *
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
 **************************************************************************/

#include <string>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "ocpn_plugin.h"

#include "hotkeys_dlg.h"
#include "manual.h"

static const char* kArrowLeft = "\u2190";
static const char* kArrowRight = "\u2192";
static const char* kArrowUp = "\u2191";
static const char* kArrowDown = "\u2193";

static const char* const kMessage = _(R"---(<tt>
Zoom in             +, PgUp             Zoom out             -, PgDown
Fine zoom in        Alt +               Fine zoom out        Alt +
Fine zoom           Ctrl scroll-wheel
Panning             < > ^ !             Slow panning:        Alt < > ^ !
Larger scale chart  Ctrl <, F7          Smaller scale chart  Ctrl >, F8

Toggle quilting     Q, F9               Toggle auto-follow   Ctrl A, F2
Toggle outlines     O, F12              Toggle range rings   R
Toggle quilting     Q, F9               Toggle chart bar     Ctrl-B
Toggle full screen  F11                 Change color scheme  Ctrl-G, F5

Start measure mode  F4                  Stop measure mode    Esc
Drop mark           Ctrl O, space bar
</tt>

More keys are available in the manual.)---");

static void replace(std::string& str, const std::string& from,
                    const std::string to) {
  size_t it = 0;
  for (it = str.find(from); it != std::string::npos; it = str.find(from, it))
    str.replace(it, from.length(), to);
}

/** Two buttons at bottom */
class ButtonsSizer : public wxStdDialogButtonSizer {
public:
  /** The "Browse manual" button */
  class ManualButton : public wxButton {
  public:
    ManualButton(wxWindow* parent)
        : wxButton(parent, wxID_OK, _("Browse manual")) {
      Bind(wxEVT_COMMAND_BUTTON_CLICKED, [](wxCommandEvent) {
        wxString datadir = GetPluginDataDir("manual_pi");
        Manual(datadir.ToStdString()).Launch("Hotkeys");
      });
    }
  };

  /** The Close button */
  class CloseButton : public wxButton {
  public:
    CloseButton(wxDialog* parent) : wxButton(parent, wxID_CLOSE) {
      Bind(wxEVT_COMMAND_BUTTON_CLICKED,
           [parent](wxCommandEvent) { parent->EndModal(wxID_OK); });
    };
  };

  ButtonsSizer(wxDialog* parent) : wxStdDialogButtonSizer() {
    AddButton(new ManualButton(parent));
    AddButton(new CloseButton(parent));
    Realize();
  };
};

class Message : public wxStaticText {
public:
  Message(wxWindow* parent, const char* text)
      : wxStaticText(parent, wxID_ANY, "") {
    std::string msg(text);
    replace(msg, "<tt>", "%tt%");
    replace(msg, "</tt>", "%/tt%");
    replace(msg, "<", kArrowLeft);
    replace(msg, ">", kArrowRight);
    replace(msg, "^", kArrowUp);
    replace(msg, "!", kArrowDown);
    replace(msg, "%tt%", "<tt>");
    replace(msg, "%/tt%", "</tt>");
    SetLabelMarkup(msg);
  }
};

HotkeysDlg::HotkeysDlg(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Hotkeys") {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto flags = wxSizerFlags().Expand().DoubleBorder();
  vbox->Add(new Message(this, kMessage), flags);
  vbox->Add(new ButtonsSizer(this), flags);
  SetSizer(vbox);
  Fit();
  Layout();
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent&) { EndModal(wxID_OK); });
}
