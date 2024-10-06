
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

#ifndef TTYSCROLL_H_
#define TTYSCROLL_H_

#include <wx/scrolwin.h>
#include <wx/textctrl.h>

/** NMEA log scrolled TTY-like window. */
class NmeaScrollwin : public wxScrolledWindow {
public:
  NmeaScrollwin(wxWindow *parent, int n_lines, wxTextCtrl &tFilter);
  virtual ~NmeaScrollwin();
  virtual void OnDraw(wxDC &dc);
  virtual void Add(const wxString &line);
  void Pause(bool pause) { m_pause = pause; }
  void Copy();

protected:
  wxCoord m_line;    // the height of one line on screen
  size_t m_n_lines;  // the number of lines we draw

  wxArrayString *m_line_array;
  wxTextCtrl &m_filter;
  bool m_pause;
};

#endif
