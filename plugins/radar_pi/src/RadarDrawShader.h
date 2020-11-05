/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _RADARDRAWSHADER_H_
#define _RADARDRAWSHADER_H_

#include "RadarDraw.h"

PLUGIN_BEGIN_NAMESPACE

#define SHADER_COLOR_CHANNELS (4)  // RGB + Alpha

class RadarDrawShader : public RadarDraw {
 public:
  RadarDrawShader(RadarInfo* ri) {
    m_ri = ri;
    m_start_line = -1;  // No spokes received since last draw
    m_lines = 0;
    m_texture = 0;
    m_fragment = 0;
    m_vertex = 0;
    m_program = 0;
    m_format = GL_RGBA;
    m_channels = SHADER_COLOR_CHANNELS;
    m_data = 0;
    m_spokes = 0;
    m_spoke_len_max = 0;
  }

  ~RadarDrawShader();

  bool Init(size_t spokes, size_t spoke_len_max);
  void DrawRadarOverlayImage(double radar_scale, double panel_rotate);
  void DrawRadarPanelImage(double panel_scale, double panel_rotate);
  void ProcessRadarSpoke(int transparency, SpokeBearing angle, uint8_t* data, size_t len, GeoPosition spoke_pos);

 private:
  RadarInfo* m_ri;

  wxCriticalSection m_exclusive;  // protects the following data structures
  unsigned char* m_data;          // [SHADER_COLOR_CHANNELS * m_spokes * m_spoke_len_max];
  size_t m_spokes;
  size_t m_spoke_len_max;

  int m_start_line;  // First line received since last draw, or -1
  int m_lines;       // # of lines received since last draw

  int m_format;
  int m_channels;

  GLuint m_texture;
  GLuint m_fragment;
  GLuint m_vertex;
  GLuint m_program;

  void Reset();
};

PLUGIN_END_NAMESPACE

#endif /* _RADARDRAWSHADER_H_ */
