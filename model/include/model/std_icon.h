/***************************************************************************
 *   Copyright (C) 2025  Alec Leamas                                       *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/   *
 **************************************************************************/

/**
 * \file
 *
 * Class StdIcon, typically a small monochrome svg icon
 */

#ifndef STD_ICON_H
#define STD_ICON_H

#include <string>

#include <wx/bitmap.h>

#include "observable_evtvar.h"

/**
 * Small monochrome SVG icon scaled to the height of a char. Aware of
 * color map changes and adapts to night mode.
 */
class StdIcon {
public:
  /**
   * Create an icon
   * @param parent  Window used to compute size
   * @param svg_file  File to render
   * @param color_change_evt Color map change event, governs day/night mode
   * @param touch  If true, ensure icon is "big enough" for touch screens
   */
  StdIcon(const wxWindow* parent, const std::string& svg_file,
          EventVar& color_change_evt, bool touch);

  /** Return bitmap, either day or inverted night variant. */
  const wxBitmap& GetBitmap();

private:
  ObsListener m_color_change_lstnr;
  EventVar& m_color_change_evt;
  bool m_is_night;
  wxBitmap m_day_bitmap;
  wxBitmap m_night_bitmap;
  wxBitmap& m_bitmap;
};

#endif  //  STD_ICON_H
