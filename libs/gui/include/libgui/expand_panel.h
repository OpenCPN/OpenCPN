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

#ifndef SHOW_HIDE_WIDGET__H_
#define SHOW_HIDE_WIDGET__H_

#include <functional>

#include <wx/panel.h>

#include "expand_icon.h"

/**
 * An ExpandIcon together with a child window. The child window is
 * hidden or shown depending ong the ExpandIcon state.
 */
class ExpandablePanel : public wxPanel {
public:
  /**
   * Single step constructor.
   * @param parent  Containing window
   * @param child Managed window, hidden/showń depending on ExpandIcon state
   * @param on_resize Callback invoked when child becomes hidden or shown.
   */
  ExpandablePanel(wxWindow* parent, wxWindow* child,
                  std::function<void()> on_resize = [] {});
  /**
   * First part of two step create.
   * @param parent Containing window.
   */
  ExpandablePanel(wxWindow* parent);

  /**
   * Second part of two step creation.
   * @param child  Window to be hidden/shown
   * @param on_resize  Callback invoked when child becomes hidden or shown
   */
  void Create(wxWindow* child, std::function<void()> on_resize = [] {} );

  /** Return the ExpandableIcon reflecting expanded/collapsed state. */
  wxWindow* GetIcon() const { return m_expand_icon; }

  /** Return the managed window which is shown or hidden. */
  wxWindow* GetChild() const { return m_child; }

protected:
  void OnClick(bool collapse);

private:
  bool m_show;
  wxWindow* m_child;
  ExpandableIcon* m_expand_icon;
  std::function<void()> m_on_resize;
};
#endif  //  SHOW_HIDE_WIDGET__H_
