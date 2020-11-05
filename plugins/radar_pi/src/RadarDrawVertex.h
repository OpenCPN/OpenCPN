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

#ifndef _RADARDRAWVERTEX_H_
#define _RADARDRAWVERTEX_H_

#include "RadarDraw.h"
#include "drawutil.h"

PLUGIN_BEGIN_NAMESPACE

#define BUFFER_SIZE (2000000)

class RadarDrawVertex : public RadarDraw {
 public:
  RadarDrawVertex(RadarInfo* ri) {
    wxCriticalSectionLocker lock(m_exclusive);

    m_ri = ri;
    m_vertices = 0;
    m_count = 0;
    m_oom = false;
    m_spokes = 0;
    m_spoke_len_max = 0;
  }

  bool Init(size_t spokes, size_t spoke_len_max);
  void DrawRadarOverlayImage(double radar_scale, double panel_rotate);
  void DrawRadarPanelImage(double panel_scale, double panel_rotate);
  void ProcessRadarSpoke(int transparency, SpokeBearing angle, uint8_t* data, size_t len, GeoPosition spoke_pos);

  ~RadarDrawVertex() {
    wxCriticalSectionLocker lock(m_exclusive);

    Reset();
  }

 private:
  RadarInfo* m_ri;
  size_t m_spokes;
  size_t m_spoke_len_max;

  static const int VERTEX_PER_TRIANGLE = 3;
  static const int VERTEX_PER_QUAD = 2 * VERTEX_PER_TRIANGLE;
  static const int MAX_BLOBS_PER_LINE = SPOKE_LEN_MAX;

  struct VertexPoint {
    Point xy;
    GLubyte red;
    GLubyte green;
    GLubyte blue;
    GLubyte alpha;
  };

  struct VertexLine {
    VertexPoint* points;
    time_t timeout;
    size_t count;
    size_t allocated;
    GeoPosition spoke_pos;
  };

  void SetBlob(VertexLine* line, int angle_begin, int angle_end, int r1, int r2, GLubyte red, GLubyte green, GLubyte blue,
               GLubyte alpha);

  void Reset();
  wxCriticalSection m_exclusive;  // protects the following
  VertexLine* m_vertices;
  unsigned int m_count;
  bool m_oom;
};

PLUGIN_END_NAMESPACE

#endif /* _RADARDRAWVERTEX_H_ */
