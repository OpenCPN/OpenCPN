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
 **************************************************************************/

#include <iomanip>
#include <sstream>
#include <string>

#ifdef _WIN32
#define NOMINMAX  // Disable min/max compiler nacros.
#endif

#include <wx/clipbrd.h>
#include <wx/dcclient.h>
#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include "tty_scroll.h"
#include "model/config_vars.h"
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

/** Return true if s matches ll's source interface or message. */
static bool IsFilterMatch(const struct Logline& ll, const std::string& s) {
  if (ll.navmsg->source->iface.find(s) != std::string::npos) return true;
  return ll.message.find(s) != std::string::npos;
}

static void UpdateColor(wxColour& colour, unsigned rgb) {
  if (rgb == kUndefinedColor) {
    colour = wxNullColour;
  } else {
    colour.SetRGB(rgb);
  }
}

static std::string Timestamp(const NavmsgTimePoint& when) {
  using namespace std;
  using namespace std::chrono;

  auto now = when.time_since_epoch();
  auto _hours = duration_cast<hours>(now);
  now -= _hours;
  auto _minutes = duration_cast<minutes>(now);
  now -= _minutes;
  auto _seconds = duration_cast<seconds>(now);
  now -= _seconds;
  auto ms = duration_cast<milliseconds>(now);
#ifdef CSTDIO_TTYSCROLL_TIMESTAMP
  // Perhaps faster, but not type safe.  Needs <cstdio>
  char buf[128];
  snprintf(buf, sizeof(buf), "%02ld:%02ld:%02ld.%03ld", _hours.count() % 24,
           _minutes.count(), _seconds.count(), ms.count());
  return buf;
#else
  stringstream ss;
  ss << setw(2) << setfill('0') << _hours.count() % 24 << ":" << setw(2)
     << _minutes.count() << ":" << setw(2) << _seconds.count() << "." << setw(3)
     << ms.count();
  return ss.str();
#endif
}

wxColor StdColorsByState::operator()(NavmsgStatus ns) {
  wxColour color;
  static const wxColor kDarkGreen(30, 72, 56);
  if (ns.status != NavmsgStatus::State::kOk)
    color = wxColour("RED");
  else if (ns.accepted == NavmsgStatus::Accepted::kFilteredNoOutput)
    color = wxColour("CORAL");
  else if (ns.accepted == NavmsgStatus::Accepted::kFilteredDropped)
    color = wxColour("MAROON");
  else if (ns.direction == NavmsgStatus::Direction::kOutput)
    color = wxColour("BLUE");
  else if (ns.direction == NavmsgStatus::Direction::kHandled)
    color = kDarkGreen;
  else  // input event
    color = wxColour("ORANGE");
  return color;
}

wxColor UserColorsByState::operator()(NavmsgStatus ns) {
  wxColour color;
  if (ns.status != NavmsgStatus::State::kOk)
    UpdateColor(color, g_dm_not_ok);
  else if (ns.accepted == NavmsgStatus::Accepted::kFilteredNoOutput)
    UpdateColor(color, g_dm_filtered);
  else if (ns.accepted == NavmsgStatus::Accepted::kFilteredDropped)
    UpdateColor(color, g_dm_dropped);
  else if (ns.direction == NavmsgStatus::Direction::kOutput)
    UpdateColor(color, g_dm_output);
  else if (ns.direction == NavmsgStatus::Direction::kInput)
    UpdateColor(color, g_dm_input);
  else
    UpdateColor(color, g_dm_ok);
  return color.IsOk() ? color : defaults(ns);
}

/** Draw a single line in the log window. */
void TtyScroll::DrawLine(wxDC& dc, const Logline& ll, int data_pos, int y) {
  wxString ws;
  if (!ll.message.empty()) ws << Timestamp(ll.navmsg->created_at) << " ";
  if (ll.message.empty())
    ;
  else if (ll.state.direction == NavmsgStatus::Direction::kOutput)
    ws << " " << kUtfRightArrow << " ";
  else if (ll.state.direction == NavmsgStatus::Direction::kInput)
    ws << " " << kUtfLeftwardsArrowToBar << " ";
  else if (ll.state.direction == NavmsgStatus::Direction::kInternal)
    ws << " " << kUtfLeftRightArrow << " ";
  else
    ws << " " << kUtfLeftArrow << " ";

  if (ll.message.empty())
    ;
  else if (ll.state.status != NavmsgStatus::State::kOk)
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
  if (iface.size() > 30)
    iface = std::string("...") + iface.substr(iface.size() - 27);
  ws << iface << " ";

  dc.DrawText(ws, 0, y);
  ws = "";
  ws << ll.message << error_msg.str();
  dc.DrawText(ws, data_pos, y);
  m_text_width =
      std::max(m_text_width, GetTextExtent(ws).GetWidth() + data_pos);
}

TtyScroll::TtyScroll(wxWindow* parent, int n_lines)
    : wxScrolledWindow(parent), m_n_lines(n_lines), m_is_paused(false) {
  SetName("TtyScroll");
  wxClientDC dc(this);
  dc.GetTextExtent("Line Height", NULL, &m_line_height);
  SetScrollRate(m_line_height, m_line_height);
  for (unsigned i = 0; i < m_n_lines; i++) m_lines.push_back(Logline());
  SetColors(std::make_unique<UserColorsByState>());
  Bind(wxEVT_SIZE, [&](wxSizeEvent& ev) { OnSize(ev); });
}

void TtyScroll::OnSize(wxSizeEvent& ev) {
  m_n_lines = ev.GetSize().y / GetCharHeight();
  while (m_lines.size() < m_n_lines) m_lines.push_back(Logline());
  ev.Skip();
}

void TtyScroll::Clear() {
  m_lines.clear();
  for (size_t i = 0; i < m_n_lines; ++i) m_lines.push_back(Logline());
  Refresh(true);
}

void TtyScroll::Add(const Logline& ll) {
  if (m_is_paused || !m_filter.Pass(ll.state, ll.navmsg)) return;
  if (!m_quick_filter.empty() && !IsFilterMatch(ll, m_quick_filter)) return;
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
  m_text_width = 0;
  for (size_t line = line_from; line <= line_to; line++) {
    wxString ws;
    dc.SetTextForeground((*m_color_by_state)(m_lines[line].state));
    DrawLine(dc, m_lines[line], 40 * GetCharWidth(), y);
    y += m_line_height;
  }
  SetVirtualSize(m_text_width, (m_n_lines + 1) * m_line_height);
}

void TtyScroll::CopyToClipboard() const {
  std::stringstream ss;
  for (auto& line : m_lines) ss << line.message << "\n";
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(ss.str()));
    wxTheClipboard->Close();
  }
}
