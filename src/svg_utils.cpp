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

#include "svg_utils.h"

#ifdef ocpnUSE_SVG
#ifndef ocpnUSE_wxBitmapBundle
#include "wxSVG/svg.h"
#else
#include <wx/bmpbndl.h>
#endif
#endif  // ocpnUSE_SVG
#include <wx/filename.h>
#include <wx/dir.h>

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

#include "pugixml.hpp"
#include "base_platform.h"

wxBitmap LoadSVG(const wxString filename, const unsigned int width,
                 const unsigned int height, wxBitmap* default_bitmap,
                 bool use_cache) {
#ifdef ocpnUSE_SVG
#ifndef ocpnUSE_wxBitmapBundle
#ifdef __OCPN__ANDROID__
  return loadAndroidSVG(filename, width, height);
#else
  wxSVGDocument svgDoc;
  if (svgDoc.Load(filename))
    return wxBitmap(svgDoc.Render(width, height, NULL, true, true));
  else
    return wxBitmap(width, height);
#endif
#else
#ifdef __OCPN__ANDROID__
  return loadAndroidSVG(filename, width, height);
#else
  wxSize s(width, height);
  if (wxFileExists(filename)) {
    wxBitmap bmp;
    std::string key;
    if (use_cache && SVGBitmapCache::GetInstance().HasKey(
                         key = SVGBitmapCache::GetInstance().MakeKey(
                             filename, width, height))) {
      bmp = SVGBitmapCache::GetInstance().Get(key);
    } else {
      bmp = wxBitmapBundle::FromSVGFile(filename, s).GetBitmap(s);
      if (use_cache) {
        SVGBitmapCache::GetInstance().Add(key, bmp);
      }
    }
    if (bmp.IsOk()) {
      return bmp;
    }
  }
  if (default_bitmap) {
    return *default_bitmap;
  } else {
    return wxNullBitmap;  // Or wxBitmap(width, height);?
  }
#endif
#endif
#else
  return wxBitmap(width, height);
#endif  // ocpnUSE_SVG
}

/*  returns 1 if str ends with suffix  */
int str_ends_with(const char* str, const char* suffix) {
  if (str == NULL || suffix == NULL) return 0;

  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  if (suffix_len > str_len) return 0;

  return 0 == strncmp(str + str_len - suffix_len, suffix, suffix_len);
}

// Convert the provided value to the standard 96 DPI pixels
// if such a conversion is doable. If not doable or not necessary (px, em, ex,
// %), numerical part of the value is returned In case of an error, 0 is
// returned
unsigned int get_px_length(const char* val) {
  int num;
  try {
    num = std::stoi(val);
  } catch (std::invalid_argument const& ex) {
    return 0;
  } catch (std::out_of_range const& ex) {
    return 0;
  }
  if (num < 0) {
    return 0;
  }

  if (str_ends_with(val, "mm")) {
    return (unsigned int)((float)num * SVG_MM_TO_PX);
  } else if (str_ends_with(val, "cm")) {
    return (unsigned int)((float)num * SVG_CM_TO_PX);
  } else if (str_ends_with(val, "in")) {
    return (unsigned int)((float)num * SVG_CM_TO_PX);
  } else if (str_ends_with(val, "pt")) {
    return (unsigned int)((float)num * SVG_PT_TO_PX);
  }
  return num;
}

bool SVGDocumentPixelSize(const wxString filename, unsigned int& width,
                          unsigned int& height) {
  pugi::xml_document svgDoc;
  if (svgDoc.load_file(filename.fn_str())) {
    pugi::xml_node svgNode = svgDoc.child("svg");
    for (pugi::xml_attribute attr = svgNode.first_attribute(); attr;
         attr = attr.next_attribute()) {
      const char* pca = attr.name();
      if (!strcmp(pca, "width")) {
        width = get_px_length(attr.as_string());
      } else if (!strcmp(pca, "height")) {
        height = get_px_length(attr.as_string());
      }
    }
  }
  return false;
}

extern BasePlatform* g_BasePlatform;
extern float g_ChartScaleFactorExp;

unsigned int SVGPixelsToDisplay(unsigned int svg_px) {
  return g_BasePlatform->GetDisplayDPmm() * SVG_MM_TO_IN / SVG_IN_TO_PX * svg_px *
         g_ChartScaleFactorExp;
}

SVGBitmapCache::SVGBitmapCache() {
  wxFileName iconcachedir;
  iconcachedir.SetName("iconCacheSVG");
  iconcachedir.SetPath(g_BasePlatform->GetPrivateDataDir());
  //  Create the cache dir here if necessary
  if (!wxDir::Exists(iconcachedir.GetFullPath())) {
    wxFileName::Mkdir(iconcachedir.GetFullPath());
  }
  cache_directory = iconcachedir.GetFullPath();
}

std::string SVGBitmapCache::MakeKey(wxString file_path, const int width,
                                    const int height) {
  std::replace(file_path.begin(), file_path.end(), ':', '_');
  std::replace(file_path.begin(), file_path.end(), '/', '_');
  std::replace(file_path.begin(), file_path.end(), '\\', '_');
  std::replace(file_path.begin(), file_path.end(), '>', '_');
  std::replace(file_path.begin(), file_path.end(), '<', '_');
  std::replace(file_path.begin(), file_path.end(), '"', '_');
  std::replace(file_path.begin(), file_path.end(), '|', '_');
  std::replace(file_path.begin(), file_path.end(), '?', '_');
  std::replace(file_path.begin(), file_path.end(), '*', '_');

  std::ostringstream ss;
  ss << file_path << "_" << width << "x" << height;
  return ss.str();
}

void SVGBitmapCache::Add(const wxString key, const wxBitmap bmp) {
  if (!bmp.IsOk()) {
    return;
  }
  sync.lock();
  items.emplace(key, bmp);
  wxFileName fn;
  fn.SetName(key);
  fn.SetPath(cache_directory);
  bmp.SaveFile(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  sync.unlock();
}

wxBitmap SVGBitmapCache::Get(const wxString key) {
  wxBitmap bmp = wxNullBitmap;
  sync.lock();
  std::unordered_map<std::string, wxBitmap>::const_iterator i =
      items.find(key.ToStdString());
  if (i != items.end()) {
    bmp = i->second;
  } else {
    wxFileName fn;
    fn.SetName(key);
    fn.SetPath(cache_directory);
    if (fn.FileExists()) {
      bmp.LoadFile(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
      if (bmp.IsOk()) {
        items.emplace(key, bmp);
      } else {
        bmp = wxNullBitmap;
      }
    }
  }
  sync.unlock();
  return bmp;
}

bool SVGBitmapCache::HasKey(const wxString key) {
  bool res = false;
  sync.lock();
  if (items.find(key.ToStdString()) != items.end()) {
    res = true;
  } else {
    wxFileName fn;
    fn.SetName(key);
    fn.SetPath(cache_directory);
    if (fn.FileExists()) {
      // We proactively also load it here if it exists
      wxBitmap bmp;
      bmp.LoadFile(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
      if (bmp.IsOk()) {
        items.emplace(key, bmp);
        res = true;
      }
    }
  }
  sync.unlock();
  return res;
}
