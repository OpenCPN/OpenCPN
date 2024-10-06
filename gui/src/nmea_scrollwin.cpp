
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

/** \file tty_scrollwin.cpp Implement tty_scrollwin.h */

#include <wx/textctrl.h>
#include <wx/dcclient.h>
#include <wx/clipbrd.h>

#include "nmea_scrollwin.h"

NmeaScrollwin::NmeaScrollwin(wxWindow *parent, int n_lines, wxTextCtrl &tFilter)
    : wxScrolledWindow(parent), m_n_lines(n_lines), m_filter(tFilter) {
  m_pause = false;
  wxClientDC dc(this);
  dc.GetTextExtent(_T("Line Height"), NULL, &m_line);

  SetScrollRate(0, m_line);
  SetVirtualSize(-1, (m_n_lines + 1) * m_line);
  m_line_array = new wxArrayString;
  for (unsigned int i = 0; i < m_n_lines; i++) m_line_array->Add(_T(""));
}

NmeaScrollwin::~NmeaScrollwin() { delete m_line_array; }

void NmeaScrollwin::Add(const wxString &line) {
  wxString filter = m_filter.GetValue();
  if (!m_pause && (filter.IsEmpty() || line.Contains(filter))) {
    if (m_line_array->GetCount() > m_n_lines - 1) {  // shuffle the arraystring
      wxArrayString *p_newArray = new wxArrayString;

      for (unsigned int i = 1; i < m_line_array->GetCount(); i++)
        p_newArray->Add(m_line_array->Item(i));

      delete m_line_array;
      m_line_array = p_newArray;
    }

    m_line_array->Add(line);
    Refresh(true);
  }
}

void NmeaScrollwin::OnDraw(wxDC &dc) {
  // update region is always in device coords, translate to logical ones
  wxRect rectUpdate = GetUpdateRegion().GetBox();
  CalcUnscrolledPosition(rectUpdate.x, rectUpdate.y, &rectUpdate.x,
                         &rectUpdate.y);

  size_t lineFrom = rectUpdate.y / m_line,
         lineTo = rectUpdate.GetBottom() / m_line;

  if (lineTo > m_n_lines - 1) lineTo = m_n_lines - 1;

  wxCoord y = lineFrom * m_line;
  wxString lss;
  for (size_t line = lineFrom; line <= lineTo; line++) {
    wxCoord yPhys;
    CalcScrolledPosition(0, y, NULL, &yPhys);

    wxString ls = m_line_array->Item(line);
    if (ls.Mid(0, 7) == _T("<GREEN>")) {
      dc.SetTextForeground(wxColour(_T("DARK GREEN")));
      lss = ls.Mid(7);
    } else if (ls.Mid(0, 7) == _T("<GOLD>")) {
      dc.SetTextForeground(wxColour(_T("GOLD")));
      lss = ls.Mid(7);
    } else if (ls.Mid(0, 6) == _T("<BLUE>")) {
      dc.SetTextForeground(wxColour(_T("BLUE")));
      lss = ls.Mid(6);
    } else if (ls.Mid(0, 5) == _T("<RED>")) {
      dc.SetTextForeground(wxColour(_T("RED")));
      lss = ls.Mid(5);
    } else if (ls.Mid(0, 7) == _T("<BROWN>")) {
      dc.SetTextForeground(wxColour(_T("BROWN")));
      lss = ls.Mid(7);
    } else if (ls.Mid(0, 8) == _T("<SIENNA>")) {
      dc.SetTextForeground(wxColour(_T("SIENNA")));
      lss = ls.Mid(8);
    } else if (ls.Mid(0, 8) == _T("<MAROON>")) {
      dc.SetTextForeground(wxColour(_T("MAROON")));
      lss = ls.Mid(8);
    } else if (ls.Mid(0, 7) == _T("<CORAL>")) {
      dc.SetTextForeground(wxColour(_T("CORAL")));
      lss = ls.Mid(7);
    }
    dc.DrawText(lss, 0, y);
    y += m_line;
  }
}

void NmeaScrollwin::Copy() {
  wxString theText;
  for (unsigned int i = 0; i < m_line_array->GetCount(); i++) {
    theText.append(m_line_array->Item(i));
    theText.append("\n");
  }
  // Write scrolled text to the clipboard
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(theText));
    wxTheClipboard->Close();
  }
}
