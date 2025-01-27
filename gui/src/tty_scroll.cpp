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

static const auto kUtfCheckMark = wxString::FromUTF8(u8"\u2713");
static const auto kUtfCircledDivisionSlash = wxString::FromUTF8(u8"\u2298");
static const auto kUtfFallingDiagonal = wxString::FromUTF8(u8"\u269F");
static const auto kUtfLeftArrow = wxString::FromUTF8(u8"\u2190");
static const auto kUtfLeftRightArrow = wxString::FromUTF8(u8"\u2194");
static const auto kUtfLeftwardsArrowToBar = wxString::FromUTF8(u8"\u21E4");
static const auto kUtfMultiplicationX = wxString::FromUTF8(u8"\u2716");
static const auto kUtfRightArrow = wxString::FromUTF8(u8"\u2192");

TtyScroll::TtyScroll(wxWindow* parent, int n_lines, wxTextCtrl& filter)
    : wxScrolledWindow(parent),
      m_n_lines(n_lines),
      m_filter(filter),
      m_is_paused(false) {
  wxClientDC dc(this);
  dc.GetTextExtent("Line Height", NULL, &m_line_height);

  SetScrollRate(0, m_line_height);
  SetVirtualSize(-1, (m_n_lines + 1) * m_line_height);
  for (unsigned i = 0; i < m_n_lines; i++) m_lines.push_back(Logline());
  Bind(wxEVT_SIZE, [&](wxSizeEvent& ev) { OnSize(ev); });
}

void TtyScroll::OnSize(wxSizeEvent& ev) {
  m_n_lines = ev.GetSize().y / GetCharHeight();
  while (m_lines.size() < m_n_lines) m_lines.push_back(Logline());
  SetVirtualSize(-1, (m_n_lines + 1) * m_line_height);
  ev.Skip();
}

void TtyScroll::Add(struct Logline ll) {
  wxString filter = m_filter.GetValue();
  if (!m_is_paused) {
    while (m_lines.size() > m_n_lines - 1) m_lines.pop_front();
    m_lines.push_back(ll);
    Refresh(true);
  }
}

void TtyScroll::OnDraw(wxDC& dc) {
  // update region is always in device coords, translate to logical ones
  wxRect rect_update = GetUpdateRegion().GetBox();
  CalcUnscrolledPosition(rect_update.x, rect_update.y, &rect_update.x,
                         &rect_update.y);

  size_t line_from = rect_update.y / m_line_height;
  size_t line_to = rect_update.GetBottom() / m_line_height;
  if (line_to > m_n_lines - 1) line_to = m_n_lines - 1;

  wxCoord y = line_from * m_line_height;
  for (size_t line = line_from; line <= line_to; line++) {
    wxString ws;
    auto l = m_lines[line];
#ifndef __WXQT__  //  Date/Time on Qt are broken, at least for android
    ws << wxDateTime::Now().FormatISOTime() << " ";
#endif
    if (l.state.direction == NavmsgStatus::Direction::kOutput)
      ws << " " << kUtfRightArrow << " ";
    else if (l.state.direction == NavmsgStatus::Direction::kInput)
      ws << " " << kUtfLeftwardsArrowToBar << " ";
    else if (l.state.direction == NavmsgStatus::Direction::kInternal)
      ws << " " << kUtfLeftRightArrow << " ";
    else
      ws << " " << kUtfLeftArrow << " ";
    wxCoord y_phys;
    CalcScrolledPosition(0, y, NULL, &y_phys);
    if (l.state.status != NavmsgStatus::State::kOk) {
      dc.SetTextForeground(wxColour("RED"));
      ws << kUtfMultiplicationX;
    } else if (l.state.accepted == NavmsgStatus::Accepted::kFilteredNoOutput) {
      dc.SetTextForeground(wxColour("CORAL"));
      ws << kUtfFallingDiagonal;
    } else if (l.state.accepted == NavmsgStatus::Accepted::kFilteredDropped) {
      ws << kUtfCircledDivisionSlash;
      dc.SetTextForeground(wxColour("MAROON"));
    } else {
      ws << kUtfCheckMark;
      dc.SetTextForeground(wxColour("GREEN"));
    }
    std::stringstream error_msg;
    if (l.state.status != NavmsgStatus::State::kOk) {
      error_msg << " - "
                << (l.error_msg.size() > 0 ? l.error_msg : "Unknown  errror");
    }
    std::string msg = l.navmsg ? l.navmsg->to_string() : "";
    ws << " (" << l.stream_name << ") " << l.prefix << msg << error_msg.str()
       << "\n";
    dc.DrawText(ws, 0, y);
    y += m_line_height;
  }
}

void TtyScroll::Copy(bool n0183) {
  wxString the_text;
  for (auto& line : m_lines) {
    std::string s = line.navmsg->to_string();
    if (n0183) {
      size_t pos = 0;
      if ((pos = s.find("$")) != std::string::npos) {
        the_text.append(s.substr(pos) + "\n");
      } else if ((pos = s.find("!")) != std::string::npos) {
        the_text.append(s.substr(pos) + "\n");
      }
    } else {
      the_text += s + "\n";
    }
  }
  // Write scrolled text to the clipboard
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(the_text));
    wxTheClipboard->Close();
  }
}
