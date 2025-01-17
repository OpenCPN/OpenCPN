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

#include <wx/clipbrd.h>
#include <wx/dcclient.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include "tty_scroll.h"

/**
 * \file
 * Implement tty_scroll.h
 */

TtyScroll::TtyScroll(wxWindow* parent, int n_lines, wxTextCtrl& filter)
    : wxScrolledWindow(parent), m_n_lines(n_lines), m_filter(filter) {
  m_is_paused = false;
  wxClientDC dc(this);
  dc.GetTextExtent("Line Height", NULL, &m_line_height);

  SetScrollRate(0, m_line_height);
  SetVirtualSize(-1, (m_n_lines + 1) * m_line_height);
  for (unsigned i = 0; i < m_n_lines; i++) m_lines.push_back("");
  Bind(wxEVT_SIZE, [&](wxSizeEvent& ev) { OnSize(ev); });
}

void TtyScroll::OnSize(wxSizeEvent& ev) {
  m_n_lines = ev.GetSize().y / GetCharHeight();
  while (m_lines.size() < m_n_lines) m_lines.push_back("");
  SetVirtualSize(-1, (m_n_lines + 1) * m_line_height);
  ev.Skip();
}

void TtyScroll::Add(const wxString& line) {
  wxString filter = m_filter.GetValue();
  if (!m_is_paused && (filter.IsEmpty() || line.Contains(filter))) {
    while (m_lines.size() > m_n_lines - 1) m_lines.pop_front();
    m_lines.push_back(line);
    Refresh(true);
  }
}

void TtyScroll::OnDraw(wxDC& dc) {
  // update region is always in device coords, translate to logical ones
  wxRect rect_update = GetUpdateRegion().GetBox();
  CalcUnscrolledPosition(rect_update.x, rect_update.y, &rect_update.x,
                         &rect_update.y);

  size_t line_from = rect_update.y / m_line_height,
         line_to = rect_update.GetBottom() / m_line_height;

  if (line_to > m_n_lines - 1) line_to = m_n_lines - 1;

  wxCoord y = line_from * m_line_height;
  wxString lss;
  for (size_t line = line_from; line <= line_to; line++) {
    wxCoord y_phys;
    CalcScrolledPosition(0, y, NULL, &y_phys);

    wxString ls = m_lines[line];
    if (ls.Mid(0, 7) == "<GREEN>") {
      dc.SetTextForeground(wxColour("DARK GREEN"));
      lss = ls.Mid(7);
    } else if (ls.Mid(0, 6) == ("<BLUE>")) {
      dc.SetTextForeground(wxColour("BLUE"));
      lss = ls.Mid(6);
    } else if (ls.Mid(0, 5) == "<RED>") {
      dc.SetTextForeground(wxColour("RED"));
      lss = ls.Mid(5);
    } else if (ls.Mid(0, 8) == "<MAROON>") {
      dc.SetTextForeground(wxColour("MAROON"));
      lss = ls.Mid(8);
    } else if (ls.Mid(0, 7) == "<CORAL>") {
      dc.SetTextForeground(wxColour("CORAL"));
      lss = ls.Mid(7);
    } else {
      lss = ls;
    }
    dc.DrawText(lss, 0, y);
    y += m_line_height;
  }
}

void TtyScroll::Copy(bool n0183) {
  wxString the_text;
  for (auto& s : m_lines) {
    if (n0183) {
      int pos = 0;
      if ((pos = s.Find("$")) != wxNOT_FOUND) {
        the_text.append(s.Mid(pos) + "\n");
      } else if ((pos = s.Find("!")) != wxNOT_FOUND) {
        the_text.append(s.Mid(pos) + "\n");
      }
    } else {
      the_text.append(s);
      the_text.append("\n");
    }
  }
  // Write scrolled text to the clipboard
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(the_text));
    wxTheClipboard->Close();
  }
}
