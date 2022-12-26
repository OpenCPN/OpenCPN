/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  SVG Utility functions
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 ***************************************************************************
 *
 *
 */

#ifndef OPENCPN_SVG_UTILS_H
#define OPENCPN_SVG_UTILS_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <algorithm>
#include <string>
#include <sstream>
#include <mutex>
#include <unordered_map>

#define SVG_IN_TO_PT 72
#define SVG_IN_TO_PX 96
#define SVG_PT_TO_IN 1 / 72
#define SVG_PX_TO_IN 1 / 96
#define SVG_PT_TO_PX 96 / 72
#define SVG_MM_TO_PX 3.7795275591
#define SVG_PX_TO_MM 0.2645833333
#define SVG_MM_TO_PT 2.8346456693
#define SVG_PT_TO_MM 0.3527777778
#define SVG_CM_TO_PX 37.795275591
#define SVG_CM_TO_PT 28.346456693
#define SVG_MM_TO_IN 25.4

// Load SVG file and return it's bitmap representation of requested size
// In case file can't be loaded an empty bitmap of desired size or a provided
// default bitmap is returned
wxBitmap LoadSVG(const wxString filename, const unsigned int width,
                 const unsigned int height, wxBitmap* default_bitmap = NULL,
                 bool use_cache = true);

// Return the size of the SVG document in standard 96 DPI pixels
// https://developer.mozilla.org/en-US/docs/Web/SVG/Content_type#length
// Percentage, em and ex make no sense without being able to relate them
// to something, so we just return the numerical value as we do for px
bool SVGDocumentPixelSize(const wxString filename, unsigned int& width,
                          unsigned int& height);

// Recalculate the length in standard 96 DPI pixels to actual display pixels
unsigned int SVGPixelsToDisplay(unsigned int svg_px);

// Manage memory and disk cache for rendered bitmaps
class SVGBitmapCache {
private:
  SVGBitmapCache();
  wxString cache_directory;
  std::mutex sync;
  std::unordered_map<std::string, wxBitmap> items;

public:
  static SVGBitmapCache& GetInstance() {
    static SVGBitmapCache instance;
    return instance;
  }

  void Clear(bool disk = true) {
    items.clear(); /*TODO: clear the disk cache as well*/
  };

  std::string MakeKey(wxString file_path, const int width, const int height);

  bool HasKey(const wxString key);

  wxBitmap Get(const wxString key);

  void Add(const wxString, const wxBitmap bmp);

public:
  SVGBitmapCache(SVGBitmapCache const&) = delete;
  void operator=(SVGBitmapCache const&) = delete;
};

#endif  // OPENCPN_SVG_UTILS_H
