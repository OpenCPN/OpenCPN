/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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
 ***************************************************************************
 */

#include <wx/textctrl.h>
#include <wx/dcclient.h>
#include <wx/clipbrd.h>

#include "TTYScroll.h"

TTYScroll::TTYScroll(wxWindow *parent, int n_lines, wxTextCtrl &tFilter)
    : wxScrolledWindow(parent), m_nLines(n_lines), m_tFilter(tFilter) {
  bpause = false;
  wxClientDC dc(this);
  dc.GetTextExtent(_T("Line Height"), NULL, &m_hLine);

  SetScrollRate(0, m_hLine);
  SetVirtualSize(-1, (m_nLines + 1) * m_hLine);
  m_plineArray = new wxArrayString;
  for (unsigned int i = 0; i < m_nLines; i++) m_plineArray->Add(_T(""));
}

TTYScroll::~TTYScroll() { delete m_plineArray; }

void TTYScroll::Add(const wxString &line) {
  wxString filter = m_tFilter.GetValue();
  if (!bpause && (filter.IsEmpty() || line.Contains(filter))) {
    if (m_plineArray->GetCount() > m_nLines - 1) {  // shuffle the arraystring
      wxArrayString *p_newArray = new wxArrayString;

      for (unsigned int i = 1; i < m_plineArray->GetCount(); i++)
        p_newArray->Add(m_plineArray->Item(i));

      delete m_plineArray;
      m_plineArray = p_newArray;
    }

    m_plineArray->Add(line);
    Refresh(true);
  }
}

void TTYScroll::OnDraw(wxDC &dc) {
  // update region is always in device coords, translate to logical ones
  wxRect rectUpdate = GetUpdateRegion().GetBox();
  CalcUnscrolledPosition(rectUpdate.x, rectUpdate.y, &rectUpdate.x,
                         &rectUpdate.y);

  size_t lineFrom = rectUpdate.y / m_hLine,
         lineTo = rectUpdate.GetBottom() / m_hLine;

  if (lineTo > m_nLines - 1) lineTo = m_nLines - 1;

  wxCoord y = lineFrom * m_hLine;
  wxString lss;
  for (size_t line = lineFrom; line <= lineTo; line++) {
    wxCoord yPhys;
    CalcScrolledPosition(0, y, NULL, &yPhys);

    wxString ls = m_plineArray->Item(line);
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
    y += m_hLine;
  }
}

void TTYScroll::Copy() {
  wxString theText;
  for (unsigned int i = 0; i < m_plineArray->GetCount(); i++) {
    theText.append(m_plineArray->Item(i));
    theText.append("\n");
  }
  // Write scrolled text to the clipboard
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(theText));
    wxTheClipboard->Close();
  }
}
