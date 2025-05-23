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

/**
 * \file
 * Implement expandable.h
 */

#include <cassert>
#include <cstring>

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/sizer.h>

#if wxCHECK_VERSION(3, 2, 0) || defined(ocpnUSE_wxBitmapBundle)
#include <wx/bmpbndl.h>
#include <iostream>
#else
#include <wx/sstream.h>
#include <wxSVG/svg.h>
#endif

#include "svg_icons.h"
#include "expand_panel.h"

ExpandablePanel::ExpandablePanel(wxWindow* parent, wxWindow* child,
                                 std::function<void()> on_resize)
    : wxPanel(parent),
      m_show(false),
      m_child(child),
      m_expand_icon(new ExpandableIcon(
          this, [&](bool collapsed) { OnClick(collapsed); })),
      m_on_resize(on_resize) {
  if (m_child) OnClick(false);
}

ExpandablePanel::ExpandablePanel(wxWindow* parent)
   : ExpandablePanel(parent, nullptr) {}

void ExpandablePanel::Create(wxWindow* child,
                             std::function<void()> on_resize) {
   m_child = child;
   m_show = false;
   m_on_resize = on_resize;
   if (m_child) m_child->Hide();
}

void ExpandablePanel::OnClick(bool collapsed) {
  m_child->Show(!collapsed);
  m_on_resize();
}
