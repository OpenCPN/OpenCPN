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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef LIBS_GUI_EXPAND_ICON__H_
#define LIBS_GUI_EXPAND_ICON__H_

#include <functional>

#include <wx/bitmap.h>
#include <wx/panel.h>
#include <wx/statbmp.h>

/**
 * Simple panel showing either an "expand" or "collapse" icon,
 * state switches when clicked.
 *
 * The panel is small, roughly the size of one or two characters.
 */
class ExpandableIcon : public wxPanel {
public :
  /**
   * Create a new expandable icon.
   * @param parent Containing window
   * @param on_toggle Callback invoked when user clicks the panel. The bool
   *    argument reflects the new state, true if it is collapsed.
   */
  ExpandableIcon(wxWindow* parent, std::function<void(bool)> on_toggle);

  bool IsCollapsed() const { return m_is_collapsed; }

protected:
  bool m_is_collapsed;
  wxBitmap m_collapse_bitmap;
  wxBitmap m_expand_bitmap;
  wxStaticBitmap* m_static_bitmap;
  std::function<void(bool)> m_on_toggle;

  void Toggle();
};

#endif  //  LIBS_GUI_EXPAND_ICON__H_
