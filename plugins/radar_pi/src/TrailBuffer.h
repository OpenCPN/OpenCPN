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

#ifndef _TRAIL_BUFFER_H_
#define _TRAIL_BUFFER_H_

#include "RadarInfo.h"

PLUGIN_BEGIN_NAMESPACE

typedef uint8_t TrailRevolutionsAge;

#define MARGIN (100)

class TrailBuffer {
 public:
  TrailBuffer(RadarInfo *ri, size_t spokes, size_t max_spoke_len);
  ~TrailBuffer();

  void ClearTrails();
  void UpdateTrailPosition();
  void UpdateTrueTrails(SpokeBearing bearing, uint8_t *data, size_t len);
  void UpdateRelativeTrails(SpokeBearing angle, uint8_t *data, size_t len);

  struct GeoPositionPixels {
    int lat;
    int lon;
  };

  GeoPosition m_pos;
  GeoPosition m_dif;  // Fraction of a pixel expressed in lat/lon for True Motion Target Trails
  GeoPositionPixels m_offset;

 private:
  void ShiftImageLonToCenter();
  void ShiftImageLatToCenter();
  void ZoomTrails(float zoom_factor);

  RadarInfo *m_ri;
  size_t m_spokes;
  int m_max_spoke_len;
  int m_trail_size;
  double m_previous_pixels_per_meter;

  TrailRevolutionsAge *m_true_trails;           // m_trails_size * m_trails_size
  TrailRevolutionsAge *m_relative_trails;       // m_spokes * m_max_spoke_len
  TrailRevolutionsAge *m_copy_true_trails;      // m_trails_size * m_trails_size
  TrailRevolutionsAge *m_copy_relative_trails;  // m_spokes * m_max_spoke_len
};

PLUGIN_END_NAMESPACE

#endif
