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

#include <regex>
#include <sstream>
#include <wx/wx.h>
#include <wx/display.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/string.h>

#include "dialog_base.h"

BaseDialog::BaseDialog(wxWindow* parent, const std::string& title, long style)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
               style) {
  m_layout = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_layout);

  // Add content sizer to layout
  m_content = new wxBoxSizer(wxVERTICAL);
  auto spacing = GUI::GetSpacing(this, kDialogPadding);
  m_layout->Add(m_content, wxSizerFlags().Border(wxALL, spacing).Expand());

  auto separator = new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
                                    wxDefaultSize, wxLI_HORIZONTAL);

  m_layout->Add(separator, wxSizerFlags().Border(wxALL, spacing).Expand());

  Bind(EVT_LAYOUT_RESIZE, [&](wxCommandEvent&) { Layout(); });
  Bind(wxEVT_HTML_LINK_CLICKED, [this](wxHtmlLinkEvent& event) {
    wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
  });
}

void BaseDialog::SetInitialSize() {
  wxDisplay display(wxDisplay::GetFromWindow(this));
  wxRect rect(display.GetGeometry());
  GUI::Breakpoint breakpoint(GUI::GetScreenSize(&rect));

  wxSize size;
  rect = display.GetClientArea();
  switch (breakpoint) {
    case GUI::Breakpoint::kExtraSmall:
      size = wxSize(rect.GetWidth() * 9 / 10, -1);
      break;
    case GUI::Breakpoint::kSmall:
    case GUI::Breakpoint::kMedium:
      size = wxSize((rect.GetWidth() / 2), -1);
      break;
    default:
      size = wxSize((rect.GetWidth() / 4), -1);
      break;
  }

  wxDialog::SetInitialSize(size);
}

void BaseDialog::AddHtmlContent(const std::stringstream& html) {
  std::string html_str = html.str();

  // Inject the font color to support dark mode.
  std::string fg_color =
      GetForegroundColour().GetAsString(wxC2S_HTML_SYNTAX).ToStdString();
  std::regex body_regex(R"(<body[^>]*>)");
  std::regex body_end_regex(R"(</body>)");
  std::string font_tag = "<font color='" + fg_color + "'>";
  html_str = std::regex_replace(html_str, body_regex, "$&" + font_tag);
  html_str = std::regex_replace(html_str, body_end_regex, "</font>$&");

  auto* html_window =
      new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, GetClientSize());
  bool result = html_window->SetPage(html_str.c_str());
  assert(result && "BaseDialog: HTML page not added");

  int html_width, html_height;
  html_window->SetBorders(0);
  html_window->GetVirtualSize(&html_width, &html_height);
  html_width += GUI::GetSpacing(this, kDialogPadding * 2);  // prevent scrollbar
  html_window->SetMinSize(
      wxSize(html_width, html_height));  // Fit() needs this size!
  html_window->SetBackgroundColour(GetBackgroundColour());
  m_content->Prepend(html_window, wxSizerFlags(1).Expand());
}
