
/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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

/** \file manual_dlg.cpp Implement manual_dlg.h */

#include <wx/button.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/window.h>

#include "manual_dlg.h"

const char* const kMessage = _(R"--(
The  off-line manual is not installed. To make it
available install the "manual" plugin.

If you are connected to the Internet you might want
to try the on-line manual instead
)--");

/** The two "browse on-line" and "Close" buttons at bottom */
class ButtonsSizer : public wxStdDialogButtonSizer {
public:

  /** The "Browse on-line manual" button */
  class OnLineButton : public wxButton {
  public:
    OnLineButton(wxWindow* parent, const std::string& url)
        : wxButton(parent, wxID_OK, _("Browse on-line manual")) {
      Bind(wxEVT_COMMAND_BUTTON_CLICKED,
           [url](wxCommandEvent) { wxLaunchDefaultBrowser(url); });
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

  ButtonsSizer(wxDialog* parent, const std::string& url)
      : wxStdDialogButtonSizer() {
    AddButton(new OnLineButton(parent, url));
    AddButton(new CloseButton(parent));
    Realize();
  }
};

/** Overall "Local manual not found" window" */
ManualDlg::ManualDlg(wxWindow* parent, const std::string& url)
    : wxDialog(parent, wxID_ANY, "Manual not found"), m_url(url) {

  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto flags = wxSizerFlags().Expand().DoubleBorder();
  vbox->Add(new wxStaticText(this, wxID_ANY, kMessage), flags);
  vbox->Add(new ButtonsSizer(this, url), flags);
  SetSizer(vbox);
  Fit();
  Layout();
}
