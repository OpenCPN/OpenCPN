/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 * Extended icon definition
 */

#ifndef MARKICON_H_
#define MARKICON_H_

#include <wx/string.h>
#include <wx/bitmap.h>

class MarkIcon {
public:
  MarkIcon() {
    m_blistImageOK = false;
    piconBitmap = NULL;
    icon_texture = 0;
    preScaled = false;
    listIndex = 0;
  }
  wxBitmap *piconBitmap;
  wxString icon_name;
  wxString icon_description;
  bool preScaled;

  unsigned int icon_texture, tex_w, tex_h;
  wxImage iconImage;
  bool m_blistImageOK;
  int listIndex;
};

#endif
