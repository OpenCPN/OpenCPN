/***************************************************************************
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

#ifndef __TTYSCROLL_H__
#define __TTYSCROLL_H__

#include <deque>

#include <wx/scrolwin.h>
#include <wx/textctrl.h>

#include "model/nmea_log.h"

/**
 * \file
 * Scrolled TTY-like window for logging and utilities
 */

/** Scrolled TTY-like window for logging, etc. */
class TtyScroll : public wxScrolledWindow {
public:
  /**
   * Create a TtyScroll instance
   * @param parent Parent window
   * @param n_lines Number of visible lines i. e., window height.
   * @param filter Used by Add() to discard lines. If filter is empty
   * or added lines contains filter.GetValue() lines are used; otherwise
   * lines are discarded.
   */
  TtyScroll(wxWindow* parent, int n_lines, wxTextCtrl& filter);

  virtual ~TtyScroll() = default;

  /**
   * Add a line to bottom of window, typically discarding top-most line.
   * Subject to checks with respect to paused state and filter possibly
   * discarding argument line.
   */
  virtual void Add(struct Logline line);

  /** Set the window to ignore Add() or not depending on pause. */
  void Pause(bool pause) { m_is_paused = pause; }

  /**
   *  Copy visible content to clipboard.
   *  @param n0183 If true, copy cleaned up data excluding time stamps etc.
   */
  void Copy(bool n183);

protected:
  wxCoord m_line_height;  // the height of one line on screen
  size_t m_n_lines;       // the number of lines we draw

  std::deque<Logline> m_lines;
  wxTextCtrl& m_filter;
  bool m_is_paused;

  virtual void OnDraw(wxDC& dc);
  void OnSize(wxSizeEvent& event);
};

#endif
