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

#ifndef _DRAWUTIL_H_
#define _DRAWUTIL_H_

#include "pi_common.h"

PLUGIN_BEGIN_NAMESPACE

extern void DrawArc(float cx, float cy, float r, float start_angle, float arc_angle, int num_segments);
extern void DrawOutlineArc(double r1, double r2, double a1, double a2, bool stippled);
extern void DrawFilledArc(double r1, double r2, double a1, double a2);
extern void CheckOpenGLError(const wxString &after);

typedef struct {
  float x;
  float y;
} Point;

typedef struct {
  int16_t x;
  int16_t y;
} PointInt;

// Allocated arrays are not two dimensional, so we make
// up a macro that makes it look that way. Note the 'stride'
// which is the length of the 2nd dimension, not the 1st.
#define M_XY_STRIDE m_spoke_len
#define M_XY(x, y) m_xy[x * M_XY_STRIDE + y]
#define M_XYI(x, y) m_xyi[x * M_XY_STRIDE + y]

class PolarToCartesianLookup {
 private:
  size_t m_spokes;
  size_t m_spoke_len;
  Point *m_xy;
  PointInt *m_xyi;

 public:
  PolarToCartesianLookup(size_t spokes, size_t spoke_len) {
    m_spokes = spokes;
    m_spoke_len = spoke_len + 1;

    m_xy = (Point *)malloc(sizeof(Point) * m_spokes * m_spoke_len);
    m_xyi = (PointInt *)malloc(sizeof(PointInt) * m_spokes * m_spoke_len);

    if (!m_xy || !m_xyi) {
      wxLogError(wxT("radar_pi: Out Of Memory, fatal!"));
      wxAbort();
    }

    for (size_t arc = 0; arc < m_spokes; arc++) {
      float sine = sinf((float)arc * PI * 2 / m_spokes);
      float cosine = cosf((float)arc * PI * 2 / m_spokes);
      for (size_t radius = 0; radius < m_spoke_len; radius++) {
        float x = (float)radius * cosine;
        float y = (float)radius * sine;
        M_XY(arc, radius).x = x;
        M_XY(arc, radius).y = y;
        M_XYI(arc, radius).x = (int16_t)x;
        M_XYI(arc, radius).y = (int16_t)y;
      }
    }
  }

  ~PolarToCartesianLookup() {
    free(m_xy);
    free(m_xyi);
  }

  // We trust that the optimizer will inline this
  Point GetPoint(size_t angle, size_t radius) { return M_XY((angle + m_spokes) % m_spokes, radius); }
  PointInt GetPointInt(size_t angle, size_t radius) { return M_XYI((angle + m_spokes) % m_spokes, radius); };
};

extern void DrawRoundRect(float x, float y, float width, float height, float radius = 0.0);

PLUGIN_END_NAMESPACE

#endif
