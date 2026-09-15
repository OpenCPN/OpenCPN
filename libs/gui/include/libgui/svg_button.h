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

#ifndef SVG__BUTTON__H_
#define SVG__BUTTON__H_
#include <fstream>
#include <sstream>

#include <wx/button.h>
#include <wx/sstream.h>

#ifndef ocpnUSE_wxBitmapBundle
#include <wxSVG/svg.h>
#endif

#include "std_filesystem.h"


/**
 * A button capable of loading an svg image. The image is scaled to roughly
 * the size of a character, and the button is made to fit the image.
 *
 * Internally uses wxBitmapBundle available from wxWidgets 3.2 if available,
 * otherwise falling back to the wxSvg library.
 *
 * Examples:   \image{inline} html ./svg-button.png "Example"
 */
class SvgButton : public wxButton {
protected:
  SvgButton(wxWindow* parent)
      : wxButton(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
                 wxDefaultSize, wxBU_EXACTFIT | wxBU_BOTTOM) {}

  /**
   * Load an svg icon available in memory.
   * @param svg SVG icon smaller than 2048 chars.
   */
  void LoadIcon(const char* svg);

  /**
   * Load icon from svg file on disk
   * @param path Path to svg icon less than 2048 chars.
   */
  void LoadIcon(const fs::path& path);

};

#endif  //  SVG__BUTTON__H_
