/**************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include <iomanip>
#include <sstream>
#include <string>

#include <wx/clipbrd.h>
#include <wx/dcclient.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include "tty_scroll.h"
#include "observable.h"

// Recursive include of winsock2.h -> redefined DrawText.
// See: https://forums.wxwidgets.org/viewtopic.php?f=19&t=51849
#ifdef _WIN32
#include <wx/msw/winundef.h>
#endif

/**
 * \file
 * Implement tty_scroll.h
 */

const wxString kUtfCheckMark = wxString::FromUTF8(u8"\u2713");
const wxString kUtfCircledDivisionSlash = wxString::FromUTF8(u8"\u2298");
const wxString kUtfFallingDiagonal = wxString::FromUTF8(u8"\u269F");
const wxString kUtfIdenticalTo = wxString::FromUTF8(u8"\u2261");
const wxString kUtfLeftArrow = wxString::FromUTF8(u8"\u2190");
const wxString kUtfLeftRightArrow = wxString::FromUTF8(u8"\u2194");
const wxString kUtfLeftwardsArrowToBar = wxString::FromUTF8(u8"\u21E4");
const wxString kUtfMultiplicationX = wxString::FromUTF8(u8"\u2716");
const wxString kUtfRightArrow = wxString::FromUTF8(u8"\u2192");

wxColor StdColorsByState::operator()(NavmsgStatus ns) {
  wxColour color;
  if (ns.status != NavmsgStatus::State::kOk)
    color = wxColour("RED");
  else if (ns.accepted == NavmsgStatus::Accepted::kFilteredNoOutput)
    color = wxColour("CORAL");
  else if (ns.accepted == NavmsgStatus::Accepted::kFilteredDropped)
    color = wxColour("MAROON");
  else if (ns.direction == NavmsgStatus::Direction::kOutput)
    color = wxColour("BLUE");
  else if (ns.direction == NavmsgStatus::Direction::kInput)
    color = wxColour("ORANGE");
  else
    color = wxColour("GREEN");
  return color;
}

/** Draw a single line in the log window. */
void TtyScroll::DrawLine(wxDC& dc, Logline ll, int data_pos, int y) {
  wxString ws;
#ifndef __WXQT__  //  Date/Time on Qt are broken, at least for android
  if (!ll.message.empty()) ws << wxDateTime::Now().FormatISOTime() << " ";
#endif
  if (ll.state.direction == NavmsgStatus::Direction::kOutput)
    ws << " " << kUtfRightArrow << " ";  // BLUE
  else if (ll.state.direction == NavmsgStatus::Direction::kInput)
    ws << " " << kUtfLeftwardsArrowToBar << " ";  // ORANGE
  else if (ll.state.direction == NavmsgStatus::Direction::kInternal)
    ws << " " << kUtfLeftRightArrow << " ";
  else
    ws << " " << kUtfLeftArrow << " ";

  if (ll.state.status != NavmsgStatus::State::kOk)
    ws << kUtfMultiplicationX;
  else if (ll.state.accepted == NavmsgStatus::Accepted::kFilteredNoOutput)
    ws << kUtfFallingDiagonal;
  else if (ll.state.accepted == NavmsgStatus::Accepted::kFilteredDropped)
    ws << kUtfCircledDivisionSlash;
  else
    ws << kUtfCheckMark;

  std::stringstream error_msg;
  if (ll.state.status != NavmsgStatus::State::kOk) {
    error_msg << " - "
              << (ll.error_msg.empty() ? "Unknown  errror" : ll.error_msg);
  }
  std::string iface(ll.navmsg ? ll.navmsg->source->iface : "");
  if (iface.size() > 20) iface = iface.substr(0, 17) + "...";
  ws << iface << " ";
  ws << (ll.navmsg ? NavAddr::BusToString(ll.navmsg->bus) : "-") << " ";

  dc.DrawText(ws, 0, y);
  ws = "";
  ws << ll.message << error_msg.str();
  dc.DrawText(ws, data_pos, y);
}

TtyScroll::TtyScroll(wxWindow* parent, int n_lines)
    : wxScrolledWindow(parent), m_n_lines(n_lines), m_is_paused(false) {
  SetName("TtyScroll");
  wxClientDC dc(this);
  dc.GetTextExtent("Line Height", NULL, &m_line_height);
  SetScrollRate(0, m_line_height);
  SetVirtualSize(-1, (m_n_lines + 1) * m_line_height);
  for (unsigned i = 0; i < m_n_lines; i++) m_lines.push_back(Logline());
  SetColors(std::make_unique<StdColorsByState>());
  Bind(wxEVT_SIZE, [&](wxSizeEvent& ev) { OnSize(ev); });
}

void TtyScroll::OnSize(wxSizeEvent& ev) {
  m_n_lines = ev.GetSize().y / GetCharHeight();
  while (m_lines.size() < m_n_lines) m_lines.push_back(Logline());
  SetVirtualSize(-1, (m_n_lines + 1) * m_line_height);
  ev.Skip();
}

void TtyScroll::Add(struct Logline ll) {
  if (m_is_paused || !m_filter.Pass(ll.state, ll.navmsg)) return;
  const auto& filter = m_quick_filter;
  if (!filter.empty() && ll.message.find(filter) == std::string::npos) return;
  while (m_lines.size() > m_n_lines - 1) m_lines.pop_front();
  m_lines.push_back(ll);
  Refresh(true);
}

void TtyScroll::SetColors(std::unique_ptr<ColorByState> color_by_state) {
  m_color_by_state = std::move(color_by_state);
}

void TtyScroll::OnDraw(wxDC& dc) {
  // Update region is always in device coords, translate to logical ones
  wxRect rect_update = GetUpdateRegion().GetBox();
  CalcUnscrolledPosition(rect_update.x, rect_update.y, &rect_update.x,
                         &rect_update.y);
  size_t line_from = rect_update.y / m_line_height;
  size_t line_to = rect_update.GetBottom() / m_line_height;
  if (line_to > m_n_lines - 1) line_to = m_n_lines - 1;

  wxCoord y = line_from * m_line_height;
  for (size_t line = line_from; line <= line_to; line++) {
    wxString ws;
    dc.SetTextForeground((*m_color_by_state)(m_lines[line].state));
    DrawLine(dc, m_lines[line], 40 * GetCharWidth(), y);
    y += m_line_height;
  }
}

void TtyScroll::CopyToClipboard() const {
  std::stringstream ss;
  for (auto& line : m_lines) ss << line.message << "\n";
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(ss.str()));
    wxTheClipboard->Close();
  }
}
