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
 * Implement svg_button.h
 */

#include "svg_button.h"
#ifdef ANDROID
wxBitmap loadAndroidSVG(const char* svg, unsigned int width,
                        unsigned int height);
#endif

void SvgButton::LoadIcon(const char* svg) {
#ifndef ANDROID
    char buffer[2048];
    assert(strlen(svg) < sizeof(buffer) && "svg icon too long");
    strcpy(buffer, svg);
#ifdef ocpnUSE_wxBitmapBundle
    auto icon_size = wxSize(GetCharHeight(), GetCharHeight());
    auto bundle = wxBitmapBundle::FromSVG(buffer, icon_size);
    SetBitmap(bundle);
#else
    wxStringInputStream wis(buffer);
    wxSVGDocument svg_doc(wis);
    wxImage image = svg_doc.Render(GetCharHeight(), GetCharHeight());
    SetBitmap(wxBitmap(image));
#endif
#else
    wxBitmap  bm = loadAndroidSVG(svg, GetCharHeight(), GetCharHeight());
    SetBitmap(bm);
#endif
  }

void SvgButton::LoadIcon(const fs::path& path) {
  std::ifstream stream(path);
  if (!stream.is_open()) return;
  std::stringstream ss;
  ss << stream.rdbuf();
  LoadIcon(ss.str());
}
