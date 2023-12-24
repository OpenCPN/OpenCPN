/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenGL text rendering
 * Author:   David Register, Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2020 David Register                                    *
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

#ifndef __DEPTHFONT_H__
#define __DEPTHFONT_H__

#include <wx/font.h>

#define SOUND_MAX_GLYPH 50

struct SoundTexGlyphInfo {
  int x, y, width, height;
  float advance;
};

class DepthFont {
public:
  DepthFont();
  ~DepthFont();

  void Build(wxFont *font, double scale, double dip_factor = 1.0);
  void Delete();

  bool IsBuilt() { return m_built; }
  unsigned int GetTexture() { return texobj; }
  bool GetGLTextureRect(wxRect &texrect, int symIndex);
  wxSize GLTextureSize() { return wxSize(tex_w, tex_h); };
  double GetScale() { return m_scaleFactor; }
  void SetContentScaleFactor(double s){m_ContentScaleFactor = s;}

private:
  wxFont m_font;

  SoundTexGlyphInfo tgi[SOUND_MAX_GLYPH];

  unsigned int texobj;
  int tex_w, tex_h;
  int m_maxglyphw;
  int m_maxglyphh;
  bool m_built;

  float m_dx;
  float m_dy;
  double m_scaleFactor;
  double m_ContentScaleFactor;
  int m_width_one;
  double m_dip_factor;
};

#endif  // guard
