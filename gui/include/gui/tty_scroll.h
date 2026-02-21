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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Scrolled tty like window for logging.
 */

#ifndef TTYSCROLL_H_
#define TTYSCROLL_H_

#include <deque>
#include <memory>

#include <wx/scrolwin.h>
#include <wx/textctrl.h>

#include "model/gui_vars.h"
#include "model/nmea_log.h"
#include "model/navmsg_filter.h"

/**
 * \file
 * Scrolled TTY-like window for logging, related utilities
 */

/** Functor returning log foreground color for given state. */
class ColorByState {
public:
  virtual wxColor operator()(NavmsgStatus) = 0;
  virtual ~ColorByState() = default;
};

/** Functor returning default color for all states. */
class NoColorsByState : public ColorByState {
public:
  NoColorsByState(wxColor color) : m_color(color) {}

  wxColor operator()(NavmsgStatus ns) { return m_color; }

private:
  wxColor m_color;
};

/** The standard colors handler functor. */
class StdColorsByState : public ColorByState {
public:
  StdColorsByState() {};

  wxColor operator()(NavmsgStatus ns);
};

class UserColorsByState : public ColorByState {
public:
  UserColorsByState() {};

  wxColor operator()(NavmsgStatus ns);

private:
  StdColorsByState defaults;
};

/** Scrolled TTY-like window for logging, etc. */
class TtyScroll : public wxScrolledWindow {
public:
  /**
   * Create a TtyScroll instance
   * @param parent Parent window
   * @param n_lines Number of visible lines i. e., window height.
   */
  TtyScroll(wxWindow* parent, int n_lines);

  virtual ~TtyScroll() = default;

  /**
   * Add a line to bottom of window, typically discarding top-most line.
   * Subject to checks with respect to paused state and filter possibly
   * discarding argument line.
   */
  virtual void Add(const Logline& line);

  /** Clear the log window */
  virtual void Clear();

  /** Set the window to ignore Add() or not depending on pause. */
  void Pause(bool pause) { m_is_paused = pause; }

  /**  Copy message contents to clipboard.  */
  void CopyToClipboard() const;

  /** Apply a display filter */
  void SetFilter(const NavmsgFilter& filter) { m_filter = filter; }

  /** Return current display filter */
  const NavmsgFilter& GetFilter() { return m_filter; }

  /** Apply a quick filter directly matched against lines */
  void SetQuickFilter(const std::string s) { m_quick_filter = s; }

  /** Set color scheme */
  void SetColors(std::unique_ptr<ColorByState> color_by_state);

protected:
  wxCoord m_line_height;  // height of one line on screen
  size_t m_n_lines;       // number of lines we draw
  wxCoord m_text_width;   // Width of widest line displayed

  std::deque<Logline> m_lines;
  NavmsgFilter m_filter;
  bool m_is_paused;
  std::unique_ptr<ColorByState> m_color_by_state;
  std::string m_quick_filter;

  void DrawLine(wxDC& dc, const Logline& ll, int data_pos, int y);
  virtual void OnDraw(wxDC& dc);
  void OnSize(wxSizeEvent& event);
};

#endif  // TTYSCROLL_H_
