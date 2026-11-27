/***************************************************************************
 *   Copyright (C) 2026  Alec Leamas                                       *
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

#include "help_tools.h"

#include <cassert>
#include <string>
#include <functional>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/popupwin.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace {

class InfoFrame : public wxFrame {
public:
  InfoFrame(wxWindow* parent, const std::string& title,
            const std::string& info)
      : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, 0) {
    auto flags = wxSizerFlags().Expand();
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(new wxStaticText(this, wxID_ANY, info), flags.Border());
    SetSizer(vbox);
    wxTopLevelWindowBase::Layout();
    Hide();
  }
};

}  // anonymous namespace

libgui::ModalHelpBtn::ModalHelpBtn(wxWindow* parent, const std::string& header,
                                   const std::string& info)
    : wxButton(parent, wxID_HELP, _("Toggle help")),
      m_info_frame(new InfoFrame(nullptr, header, info)) {
  m_info_frame->Hide();
  Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent&) {
    if (m_info_frame->IsShown()) {
      m_info_frame->Hide();
    } else {
      m_info_frame->Fit();
      m_info_frame->Enable();
      m_info_frame->Show();
    }
  });
}
