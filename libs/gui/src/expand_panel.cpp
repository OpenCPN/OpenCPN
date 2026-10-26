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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 * Implement expand_panel.h
 */


#include "expand_panel.h"

ExpandablePanel::ExpandablePanel(wxWindow* parent, wxWindow* child,
                                 std::function<void()> on_resize)
    : wxPanel(parent),
      m_show(false),
      m_child(child),
      m_expand_icon(new ExpandableIcon(
          this, [&](bool collapsed) { OnClick(collapsed); })),
      m_on_resize(std::move(on_resize)) {
  if (m_child) OnClick(true);
}

ExpandablePanel::ExpandablePanel(wxWindow* parent)
   : ExpandablePanel(parent, nullptr) {}

void ExpandablePanel::Create(wxWindow* child,
                             std::function<void()> on_resize) {
   m_child = child;
   m_show = false;
   m_on_resize = std::move(on_resize);
   if (m_child) m_child->Hide();
}

void ExpandablePanel::OnClick(bool collapsed) const {
  m_child->Show(!collapsed);
  m_on_resize();
}
