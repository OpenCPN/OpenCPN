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

#include <array>
#include <string>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include "ocpn_plugin.h"

#include "hotkeys_dlg.h"
#include "manual.h"

static inline wxString U8(const char* s) { return wxString::FromUTF8(s); }

/** Two buttons at bottom */
class ButtonsSizer : public wxStdDialogButtonSizer {
public:
  /** The "Browse manual" button */
  class ManualButton : public wxButton {
  public:
    ManualButton(wxWindow* parent)
        : wxButton(parent, wxID_OK, _("Browse manual")) {
      Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent) {
        wxString datadir = GetPluginDataDir("manual_pi");
        Manual(this, datadir.ToStdString()).Launch("Hotkeys");
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

/** Overall help message: key functions and bindings in a string matrix */
class GridSizer : public wxGridSizer {
private:
  static constexpr int kGridSize{4};
  static constexpr int kNumMsgs{12};
  using MsgLine = std::array<wxString, kGridSize>;
  using Messages = std::array<MsgLine, kNumMsgs>;

public:
  GridSizer(wxWindow* parent) : wxGridSizer(kGridSize) {
    const auto osSystemId = wxPlatformInfo::Get().GetOperatingSystemId();
    const Messages& kMessages =
        (osSystemId & wxOS_MAC) ? kMacMessages : kWinLinuxMessages;

    for (const MsgLine& line : kMessages)
      for (const wxString& word : line)
        Add(new wxStaticText(parent, wxID_ANY, word),
            wxSizerFlags().DoubleBorder());
  }

private:
  // It's unclear whether _() actually works in this context or
  // if wxTRANSLATE is needed instead...
  const Messages kWinLinuxMessages{
      // clang-format off
      {{_("Zoom in"), "+, PgUp",
                                   _("Zoom out"), "-, PgDown"},
       {_("Fine zoom in"), "Alt +",
                                   _("Fine zoom out"), "Alt -"},
       {_("Fine zoom"), _("Ctrl scroll-wheel"),
                                   "", ""},
       {_("Panning"), U8("→ ← ↑ ↓")
                                 , _("Slow panning"), U8("Alt → ← ↑ ↓")},
       {_("Larger scale chart"), U8("Ctrl ←, F7"),
                                   _("Smaller scale chart"), U8("Ctrl →, F8")},
       {_("Toggle quilting "), "Q, F9",
                                   _("Toggle auto-follow"), "Ctrl A, F2"},
       {_("Toggle outlines"), "O, F12",
                                   _("Toggle range rings"), "R"},
       {_("Toggle chart bar"), "Ctrl B",
                                   _("Change color scheme"), "Ctrl-G, F5"},
       {_("Toggle full screen"), "F11",
                                   "", ""},
       {"", "", "", ""},
       {_("Start measure mode"), "M, F4",
                                   _("Stop measure mode"), "Esc"},
       {_("Drop mark"), _("Ctrl O, space bar"),
                                   _("Open Data Monitor"), "E"}}};

  const Messages kMacMessages{
      {{_("Zoom in"), "+, PgUp",
                                   _("Zoom out"), "-, PgDown"},
       {_("Fine zoom in"), "Alt +",
                                   _("Fine zoom out"), "Alt -"},
       {_("Fine zoom"), _("Ctrl scroll-wheel"),
                                   "", ""},
       {_("Panning"), U8("→ ← ↑ ↓")
                                 , _("Slow panning"), U8("Alt → ← ↑ ↓")},
       {_("Larger scale chart"), U8("Cmd ←, F7"),
                                   _("Smaller scale chart"), U8("Cmd →, F8")},
       {_("Toggle quilting "), "Q, F9",
                                   _("Toggle auto-follow"), "Cmd A"},
       {_("Toggle outlines"), "O, F12",
                                   _("Toggle range rings"), "R"},
       {_("Toggle chart bar"), "Ctrl B",
                                   _("Change color scheme"), "Ctrl-G, F5"},
       {_("Toggle full screen"), "Ctrl Cmd F",
                                   "", ""},
       {"", "", "", ""},
       {_("Start measure mode"), "F4",
                                   _("Stop measure mode"), "Esc"},
       {_("Drop mark"), _("Ctrl O, space bar"),
                                   _("Open Data Monitor"), "E"}}};  // clang-format on
};

HotkeysDlg::HotkeysDlg(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, _("Keyboard Shortcuts")) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto flags = wxSizerFlags().DoubleBorder();
  vbox->Add(new GridSizer(this), flags.Expand());
  vbox->Add(new wxStaticText(this, wxID_ANY,
                             _("More keys are available in the manual.")),
            wxSizerFlags().DoubleBorder().Centre());
  vbox->Add(new ButtonsSizer(this), flags.Expand());
  SetSizer(vbox);
  Fit();
  Layout();
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent&) { EndModal(wxID_OK); });
}
