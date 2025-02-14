/***************************************************************************
 *   Copyright (C) 2025 by NoCodeHummel                                    *
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

#include <sstream>
#include <wx/wx.h>
#include <wx/display.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include "dialog_base.h"

BaseDialog::BaseDialog(wxWindow* parent, const std::string& title, long style)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
               style) {
  m_layout = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_layout);

  // Add content sizer to layout
  m_content = new wxBoxSizer(wxVERTICAL);
  m_layout->Add(m_content, wxSizerFlags().Border(
                               wxALL, GUI::GetSpacing(this, kDialogPadding)));

  // Handle layout resize event
  Bind(EVT_LAYOUT_RESIZE, [&](wxCommandEvent&) { Layout(); });
}

void BaseDialog::SetInitialSize() {
  wxDisplay display(wxDisplay::GetFromWindow(this));
  wxRect rect(display.GetGeometry());
  GUI::Breakpoint breakpoint(GUI::GetScreenSize(&rect));

  wxSize size;
  rect = display.GetClientArea();
  switch (breakpoint) {
    case GUI::Breakpoint::kExtraSmall:
      size = wxSize(rect.GetWidth(), -1);
      break;
    case GUI::Breakpoint::kSmall:
    case GUI::Breakpoint::kMedium:
      size = wxSize((rect.GetWidth() / 2), -1);
      break;
    default:
      size = wxSize((rect.GetWidth() / 3), -1);
      break;
  }

  wxDialog::SetInitialSize(size);
}

int BaseDialog::ShowModal() {
  Fit();
  Center(wxBOTH | wxCENTER_FRAME);

  return wxDialog::ShowModal();
}

void BaseDialog::AddHtmlContent(const std::stringstream& html) {
  std::string html_str = html.str();
  auto* html_window =
      new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, GetClientSize());
  bool result = html_window->SetPage(html_str.c_str());
  assert(result && "BaseDialog: HTML page not added");

  int html_width, html_height;
  html_window->GetVirtualSize(&html_width, &html_height);
  html_window->SetMinSize(
      wxSize(html_width, html_height));  // Fit() needs this size!
  html_window->SetBackgroundColour(GetBackgroundColour());
  m_content->Prepend(html_window, wxSizerFlags(1).Expand());
}
