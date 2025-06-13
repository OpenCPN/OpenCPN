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
 * Implement expand_icon.h
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
#include "expand_icon.h"

#ifdef ANDROID
wxBitmap loadAndroidSVG(const char* svg, unsigned int width,
                        unsigned int height);
#endif

static wxBitmap LoadSvgBitmap(const char* svg, wxWindow* parent) {
#ifndef ANDROID
  char buffer[2048];  // Needs to be larger than any svg icon...
  assert(strlen(svg) < sizeof(buffer));
  std::strcpy(buffer, svg);
#if wxCHECK_VERSION(3, 2, 0) || defined(ocpnUSE_wxBitmapBundle)
  auto icon_size = wxSize(parent->GetCharHeight(), parent->GetCharHeight());
  auto bundle = wxBitmapBundle::FromSVG(buffer, icon_size);
  assert(bundle.IsOk() && "Cannot load svg icon");
  return bundle.GetBitmap(icon_size);
#else
  wxStringInputStream wis(buffer);
  wxSVGDocument svg_doc(wis);
  wxImage image =
      svg_doc.Render(parent->GetCharHeight(), parent->GetCharHeight());
  assert(wxBitmap(image).IsOk() && "Cannot load svg icon");
  return wxBitmap(image);
#endif
#else
  return loadAndroidSVG(svg, parent->GetCharHeight(), parent->GetCharHeight());
#endif
}

ExpandableIcon::ExpandableIcon(wxWindow* parent,
                                 std::function<void(bool)> on_toggle)
  : wxPanel(parent, wxID_ANY),
    m_is_collapsed(true),
    m_collapse_bitmap(LoadSvgBitmap(gui_icons::kCollapseSvg, parent)),
    m_expand_bitmap(LoadSvgBitmap(gui_icons::kExpandSvg, parent)),
    m_static_bitmap(new wxStaticBitmap(this, wxID_ANY, m_expand_bitmap)),
    m_on_toggle(on_toggle) {

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(m_static_bitmap);
  SetSizer(vbox);
  m_static_bitmap->Bind(wxEVT_LEFT_DOWN, [&](wxMouseEvent&) { Toggle(); });
}

void ExpandableIcon::Toggle() {
  m_is_collapsed = !m_is_collapsed;
  m_static_bitmap->SetBitmap(
      m_is_collapsed ? m_expand_bitmap : m_collapse_bitmap);
  m_on_toggle(m_is_collapsed);
}
