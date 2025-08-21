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
 * Implement std_icon.h
 */

#include "model/svg_utils.h"
#include "model/std_icon.h"

static wxBitmap InvertColors(const wxBitmap& original) {
  wxImage image = original.ConvertToImage();
  unsigned char* data = image.GetData();
  unsigned char* p_idata = data;
  for (int i = 0; i < image.GetSize().y; i++) {
    for (int j = 0; j < image.GetSize().x; j++) {
      unsigned char v = *p_idata;
      v = 255 - v;
      *p_idata++ = v;
      v = *p_idata;
      v = 255 - v;
      *p_idata++ = v;
      v = *p_idata;
      v = 255 - v;
      *p_idata++ = v;
    }
  }
  return wxBitmap(image);
}

StdIcon::StdIcon(const wxWindow* parent, const std::string& svg_file,
                 EventVar& color_change_evt, bool touch)
    : m_color_change_evt(color_change_evt),
      m_is_night(false),
      m_day_bitmap(LoadSvgStdIcon(svg_file, parent, touch)),
      m_night_bitmap(InvertColors(m_day_bitmap)),
      m_bitmap(m_day_bitmap) {
  m_color_change_lstnr.Init(m_color_change_evt, [&](const ObservedEvt& ev) {
    m_bitmap = ev.GetInt() == 0 ? m_night_bitmap : m_day_bitmap;
  });
}

const wxBitmap& StdIcon::GetBitmap() { return m_bitmap; }
