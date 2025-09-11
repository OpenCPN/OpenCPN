/**************************************************************************
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
 * Font description container
 */

#ifndef __FONTDESC_H__
#define __FONTDESC_H__

#include <list>

#include <wx/string.h>
#include <wx/font.h>
#include <wx/colour.h>

class MyFontDesc {
public:
  MyFontDesc(wxString DialogString, wxString ConfigString, wxFont *pFont,
             wxColour color, bool is_default = true);
  ~MyFontDesc();

  /**
   * UI element identifier, e.g., "AISTargetAlert", "StatusBar".
   *
   * Used to identify the font configuration in the list.
   *
   * @see [GetFont]
   */
  wxString m_dialogstring;
  /**
   * Configuration key in "locale-hash" format.
   */
  wxString m_configstring;
  /**
   * Platform-specific font descriptor string.
   */
  wxString m_nativeInfo;
  /**
   * Font object.
   */
  wxFont *m_font;
  /**
   * Text color.
   */
  wxColour m_color;
  /** Indicates if this is the default font entry for the TextElement. */
  bool m_is_default;
};

using FontList = std::list<MyFontDesc *>;

#endif
