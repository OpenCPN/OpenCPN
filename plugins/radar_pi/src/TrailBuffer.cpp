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

#include "TrailBuffer.h"

#undef M_SETTINGS
#define M_SETTINGS m_ri->m_pi->m_settings

PLUGIN_BEGIN_NAMESPACE

// Allocated arrays are not two dimensional, so we make
// up a macro that makes it look that way. Note the 'stride'
// which is the length of the 2nd dimension, not the 1st.
// Striding the first dimension makes for better locality because
// we generally iterate over the range (process one spoke) so those
// values are now closer together in memory.
#define M_TRUE_TRAILS_STRIDE m_trail_size
#define M_TRUE_TRAILS(x, y) m_true_trails[x * M_TRUE_TRAILS_STRIDE + y]
#define M_RELATIVE_TRAILS_STRIDE m_max_spoke_len
#define M_RELATIVE_TRAILS(x, y) m_relative_trails[x * M_RELATIVE_TRAILS_STRIDE + y]

TrailBuffer::TrailBuffer(RadarInfo *ri, size_t spokes, size_t max_spoke_len) {
  m_ri = ri;
  m_spokes = spokes;
  m_max_spoke_len = (int)max_spoke_len;
  m_previous_pixels_per_meter = 0.;
  m_trail_size = max_spoke_len * 2 + MARGIN * 2;
  m_true_trails = (TrailRevolutionsAge *)calloc(sizeof(TrailRevolutionsAge), m_trail_size * m_trail_size);
  m_relative_trails = (TrailRevolutionsAge *)calloc(sizeof(TrailRevolutionsAge), m_spokes * m_max_spoke_len);
  m_copy_true_trails = (TrailRevolutionsAge *)calloc(sizeof(TrailRevolutionsAge), m_trail_size * m_trail_size);
  m_copy_relative_trails = (TrailRevolutionsAge *)calloc(sizeof(TrailRevolutionsAge), m_spokes * m_max_spoke_len);

  if (!m_true_trails || !m_relative_trails || !m_copy_true_trails || !m_copy_relative_trails) {
    wxLogError(wxT("radar_pi: Out Of Memory, fatal!"));
    wxAbort();
  }
  ClearTrails();
}

TrailBuffer::~TrailBuffer() {
  free(m_true_trails);
  free(m_relative_trails);
  free(m_copy_relative_trails);
  free(m_copy_true_trails);
}

void TrailBuffer::UpdateTrueTrails(SpokeBearing bearing, uint8_t *data, size_t len) {
  int motion = m_ri->m_trails_motion.GetValue();
  RadarControlState trails = m_ri->m_target_trails.GetState();
  bool update_targets_true = trails != RCS_OFF && motion == TARGET_MOTION_TRUE;

  uint8_t weak_target = M_SETTINGS.threshold_blue;
  uint8_t strong_target = M_SETTINGS.threshold_red;
  size_t radius = 0;

  for (; radius < len - 1; radius++) {  //  len - 1 : no trails on range circle
    PointInt point = m_ri->m_polar_lookup->GetPointInt(bearing, radius);

    point.x += m_trail_size / 2 + m_offset.lat;
    point.y += m_trail_size / 2 + m_offset.lon;

    if (point.x >= 0 && point.x < (int)m_trail_size && point.y >= 0 && point.y < (int)m_trail_size) {
      uint8_t *trail = &M_TRUE_TRAILS(point.x, point.y);
      // when ship moves north, offset.lat > 0. Add to move trails image in opposite direction
      // when ship moves east, offset.lon > 0. Add to move trails image in opposite direction
      if (data[radius] >= strong_target) {
        *trail = 1;
      } else if (*trail > 0 && *trail < TRAIL_MAX_REVOLUTIONS) {
        (*trail)++;
      }

      if (update_targets_true && (data[radius] < weak_target)) {
        data[radius] = m_ri->m_trail_colour[*trail];
      }
    }
  }

  // Now process the rest of the spoke from len to m_spoke_len_max.
  // This will only be called when the current spoke length is smaller than the max.
  // we need to update the trail 'age' for those points.
  for (; radius < m_ri->m_spoke_len_max; radius++) {
    PointInt point = m_ri->m_polar_lookup->GetPointInt(bearing, radius);

    point.x += m_trail_size / 2 + m_offset.lat;
    point.y += m_trail_size / 2 + m_offset.lon;

    if (point.x >= 0 && point.x < (int)m_trail_size && point.y >= 0 && point.y < (int)m_trail_size) {
      uint8_t *trail = &M_TRUE_TRAILS(point.x, m_trail_size + point.y);
      // when ship moves north, offset.lat > 0. Add to move trails image in opposite direction
      // when ship moves east, offset.lon > 0. Add to move trails image in opposite direction
      if (*trail > 0 && *trail < TRAIL_MAX_REVOLUTIONS) {
        (*trail)++;
      }
    }
  }
}

void TrailBuffer::UpdateRelativeTrails(SpokeBearing angle, uint8_t *data, size_t len) {
  int motion = m_ri->m_trails_motion.GetValue();
  RadarControlState trails = m_ri->m_target_trails.GetState();
  bool update_relative_motion = trails != RCS_OFF && motion == TARGET_MOTION_RELATIVE;

  uint8_t *trail = &M_RELATIVE_TRAILS(angle, 0);
  uint8_t weak_target = M_SETTINGS.threshold_blue;
  uint8_t strong_target = M_SETTINGS.threshold_red;
  int radius = 0;
  int length = int(len);

  for (; radius < length - 1; radius++, trail++) {  // len - 1 : no trails on range circle
    if (data[radius] >= strong_target) {
      *trail = 1;
    } else if (*trail > 0 && *trail < TRAIL_MAX_REVOLUTIONS) {
      (*trail)++;
    }

    if (update_relative_motion && (data[radius] < weak_target)) {
      data[radius] = m_ri->m_trail_colour[*trail];
    }
  }

  for (; radius < m_max_spoke_len; radius++, trail++)  // And clear out empty bit of spoke when spoke_len < max_spoke_len
  {
    *trail = 0;
  }
}

// Zooms the trailbuffer (containing image of true trails) in and out
// This version assumes m_offset.lon and m_offset.lat to be zero (earlier versions did zoom offset as well)
// zoom_factor > 1 -> zoom in, enlarge image
void TrailBuffer::ZoomTrails(float zoom_factor) {
  uint8_t *flip;
  memset(m_copy_relative_trails, 0, m_spokes * m_max_spoke_len);

  // zoom relative trails

  for (int i = 0; i < (int)m_spokes; i++) {
    for (int j = 0; j < m_max_spoke_len; j++) {
      int index_j = j * zoom_factor;
      if (index_j >= m_max_spoke_len) break;
      if (M_RELATIVE_TRAILS(i, j) != 0) {
        m_copy_relative_trails[i * M_RELATIVE_TRAILS_STRIDE + index_j] = M_RELATIVE_TRAILS(i, j);
      }
    }
  }
  // Now exchange the two
  flip = m_relative_trails;
  m_relative_trails = m_copy_relative_trails;
  m_copy_relative_trails = flip;

  memset(m_copy_true_trails, 0, m_trail_size * m_trail_size);

  // zoom true trails
  for (int i = MARGIN; i < m_trail_size - MARGIN; i++) {
    int index_i = (int)(((double)i - (double)m_trail_size / 2) * zoom_factor + (double)m_trail_size / 2);
    if (index_i >= m_trail_size - 1) {
      break;  // allow adding an additional pixel later
    }
    if (index_i < 0) {
      continue;
    }
    for (int j = MARGIN; j < m_trail_size - MARGIN; j++) {
      int index_j = (int)(((double)j - (double)m_trail_size / 2) * zoom_factor + (double)m_trail_size / 2);
      if (index_j >= (int)m_trail_size - 1) {
        break;
      }
      if (index_j < 0) {
        continue;
      }
      uint8_t pixel = M_TRUE_TRAILS(i, j);
      if (pixel != 0) {  // many to one mapping, prevent overwriting trails with 0
        m_copy_true_trails[index_i * M_TRUE_TRAILS_STRIDE + index_j] = pixel;
        if (zoom_factor > 1.2) {
          // add an extra pixel in the y direction
          m_copy_true_trails[index_i * M_TRUE_TRAILS_STRIDE + index_j + 1] = pixel;
          if (zoom_factor > 1.6) {
            // also add pixels in the x direction
            m_copy_true_trails[(index_i + 1) * M_TRUE_TRAILS_STRIDE + index_j] = pixel;
            m_copy_true_trails[(index_i + 1) * M_TRUE_TRAILS_STRIDE + index_j + 1] = pixel;
          }
        }
      }
    }
  }
  flip = m_true_trails;
  m_true_trails = m_copy_true_trails;
  m_copy_true_trails = flip;
}

void TrailBuffer::UpdateTrailPosition() {
  GeoPosition radar;
  GeoPositionPixels shift;
  // When position changes the trail image is not moved, only the pointer to the center
  // of the image (offset) is changed.
  // So we move the image around within the m_trails.true_trails buffer (by moving the pointer).
  // But when there is no room anymore (margin used) the whole trails image is shifted
  // and the offset is reset
  if (m_offset.lon >= MARGIN || m_offset.lon <= -MARGIN || m_offset.lat >= MARGIN || m_offset.lat <= -MARGIN) {
    LOG_INFO(wxT("radar_pi: offset lat %d or lon too large %d"), m_offset.lat, m_offset.lon);
    ClearTrails();
    return;
  }

  // zooming of trails required? First check conditions
  if (m_previous_pixels_per_meter == 0. || m_ri->m_pixels_per_meter == 0.) {
    ClearTrails();
    if (m_ri->m_pixels_per_meter == 0.) {
      return;
    }
    m_previous_pixels_per_meter = m_ri->m_pixels_per_meter;
  } else if (m_previous_pixels_per_meter != m_ri->m_pixels_per_meter && m_previous_pixels_per_meter != 0.) {
    // zoom trails
    double zoom_factor = m_ri->m_pixels_per_meter / m_previous_pixels_per_meter;

    if (zoom_factor < 0.25 || zoom_factor > 4.00) {
      ClearTrails();
      return;
    }
    m_previous_pixels_per_meter = m_ri->m_pixels_per_meter;
    // center the image before zooming
    // otherwise the offset might get too large
    ShiftImageLatToCenter();
    ShiftImageLonToCenter();
    ZoomTrails(zoom_factor);
  }

  if (!m_ri->GetRadarPosition(&radar) || m_ri->m_pi->GetHeadingSource() == HEADING_NONE) {
    return;
  }

  // Did the ship move? No, return.
  if (m_pos.lat == radar.lat && m_pos.lon == radar.lon) {
    return;
  }
  // Check the movement of the ship
  double dif_lat = radar.lat - m_pos.lat;  // going north is positive
  double dif_lon = radar.lon - m_pos.lon;  // moving east is positive

  m_pos = radar;
  // get (floating point) shift of the ship in radar pixels
  double fshift_lat = dif_lat * 60. * 1852. * m_ri->m_pixels_per_meter;
  double fshift_lon = dif_lon * 60. * 1852. * m_ri->m_pixels_per_meter;
  fshift_lon *= cos(deg2rad(radar.lat));  // at higher latitudes a degree of longitude is fewer meters
  // Get the integer pixel shift, first add previous rounding error
  shift.lat = (int)(fshift_lat + m_dif.lat);
  shift.lon = (int)(fshift_lon + m_dif.lon);

  // Check for changes in the direction of movement, part of the image buffer has to be erased

  if (shift.lat > 0 && m_ri->m_dir_lat <= 0) {
    // change of direction of movement, moving north now
    // clear space in trailbuffer above image (this area might not be empty)
    uint8_t *start_of_area_to_clear = m_true_trails + (m_trail_size - MARGIN + m_offset.lat) * m_trail_size;
    int number_of_pixels_to_clear = (MARGIN - m_offset.lat) * m_trail_size;
    memset(start_of_area_to_clear, 0, number_of_pixels_to_clear);
    m_ri->m_dir_lat = 1;
  }

  if (shift.lat < 0 && m_ri->m_dir_lat >= 0) {
    // change of direction of movement, moving south now
    // clear space in true_trails below image
    uint8_t *start_of_area_to_clear = m_true_trails;
    int number_of_pixels_to_clear = (MARGIN + m_offset.lat) * m_trail_size;
    memset(start_of_area_to_clear, 0, number_of_pixels_to_clear);
    m_ri->m_dir_lat = -1;
  }

  if (shift.lon > 0 && m_ri->m_dir_lon <= 0) {
    // change of direction of movement, moving east now
    // clear space in true_trails to the right of image
    int number_of_pixels_to_clear = MARGIN - m_offset.lon;
    for (int i = 0; i < m_trail_size; i++) {
      uint8_t *start_of_area_to_clear = m_true_trails + m_trail_size * i + m_trail_size - MARGIN + m_offset.lon;
      memset(start_of_area_to_clear, 0, number_of_pixels_to_clear);
    }
    m_ri->m_dir_lon = 1;
  }

  if (shift.lon < 0 && m_ri->m_dir_lon >= 0) {
    // change of direction of movement, moving west now
    // clear space in true_trails outside image in that direction
    int number_of_pixels_to_clear = MARGIN + m_offset.lon;
    for (int i = 0; i < m_trail_size; i++) {
      uint8_t *start_of_area_to_clear = m_true_trails + m_trail_size * i;
      memset(start_of_area_to_clear, 0, number_of_pixels_to_clear);
    }
    m_ri->m_dir_lon = -1;
  }

  // save the rounding fraction and appy it next time
  m_dif.lat = fshift_lat + m_dif.lat - (double)shift.lat;
  m_dif.lon = fshift_lon + m_dif.lon - (double)shift.lon;

  if (shift.lat >= MARGIN || shift.lat <= -MARGIN || shift.lon >= MARGIN || shift.lon <= -MARGIN) {  // huge shift, reset trails

    LOG_INFO(wxT("radar_pi: %s Large movement trails reset, shift.lat= %f, shift.lon=%f"), m_ri->m_name.c_str(), shift.lat,
             shift.lon);
    ClearTrails();
    return;
  }

  // offset lon too large: shift image
  if (abs(m_offset.lon + shift.lon) >= MARGIN) {
    ShiftImageLonToCenter();
  }

  // offset lat too large: shift image in lat direction
  if (abs(m_offset.lat + shift.lat) >= MARGIN) {
    ShiftImageLatToCenter();
  }
  // apply the shifts to the offset
  m_offset.lat += shift.lat;
  m_offset.lon += shift.lon;
}

// shifts the true trails image in lat direction to center
void TrailBuffer::ShiftImageLatToCenter() {
  int image_size = m_trail_size * 2 * m_max_spoke_len;  // number of pixels to shift up / down

  if (m_offset.lat >= MARGIN || m_offset.lat <= -MARGIN) {  // abs not ok
    LOG_INFO(wxT("radar_pi: offset lat too large %i"), m_offset.lat);
    ClearTrails();
    return;
  }
  // current starting location of shifted image
  uint8_t *source_address = m_true_trails + (MARGIN + m_offset.lat) * m_trail_size;
  // location where centered image should be
  uint8_t *destination_address = m_true_trails + MARGIN * m_trail_size;
  // size of image to be shifted, extended to the full width of trailbuffer
  image_size = m_trail_size * 2 * m_max_spoke_len;
  memmove(destination_address, source_address, image_size);
  uint8_t *start_of_area_to_clear;
  int number_of_pixels_to_clear = MARGIN * m_trail_size;
  if (m_offset.lat > 0) {
    // clear upper area of trailbuffer which is now outside the image
    start_of_area_to_clear = m_true_trails + (m_trail_size - MARGIN) * m_trail_size;
  } else {
    // clear lower area of trailbuffer which is now outside the image
    start_of_area_to_clear = m_true_trails;
  }
  memset(start_of_area_to_clear, 0, number_of_pixels_to_clear);
  m_offset.lat = 0;
}

// shifts the true trails image in lon direction to center
void TrailBuffer::ShiftImageLonToCenter() {
  if (m_offset.lon >= MARGIN || m_offset.lon <= -MARGIN) {  // abs no good
    LOG_INFO(wxT("radar_pi: offset lon too large %i"), m_offset.lon);
    ClearTrails();
    return;
  }
  // number of pixels to shift right / left
  int line_of_image_size = 2 * m_max_spoke_len;
  // MARGIN is where the centered line should start
  // shift per line, rigth / left
  for (int i = 0; i < m_trail_size; i++) {
    // current starting location of image
    uint8_t *source_address = m_true_trails + i * m_trail_size + MARGIN + m_offset.lon;
    // location where centered image should be
    uint8_t *destination_address = m_true_trails + i * m_trail_size + MARGIN;
    memmove(destination_address, source_address, line_of_image_size);

    uint8_t *start_of_area_to_clear;
    // offset > 0, we shifted to the left, so clear area to the right of image
    if (m_offset.lon > 0) {
      // start clear at end of the line minus current margin
      start_of_area_to_clear = m_true_trails + i * m_trail_size + m_trail_size - MARGIN;
    }
    // offset <= 0, we shifted to the right, so clear area to the left of image
    else {
      // start clear at start of the line
      start_of_area_to_clear = m_true_trails + i * m_trail_size;
    }
    memset(start_of_area_to_clear, 0, MARGIN);
  }
  m_offset.lon = 0;
}

void TrailBuffer::ClearTrails() {
  m_offset.lat = 0;
  m_offset.lon = 0;
  m_dif.lat = 0.;
  m_dif.lon = 0.;
  // prevent zooming of trails in next trail update
  m_previous_pixels_per_meter = m_ri->m_pixels_per_meter;
  if (m_true_trails) {
    memset(m_true_trails, 0, m_trail_size * m_trail_size);
  }
  if (m_relative_trails) {
    memset(m_relative_trails, 0, m_spokes * m_max_spoke_len);
  }
  if (!m_ri->GetRadarPosition(&m_pos)) {
    m_pos.lat = 0.;
    m_pos.lon = 0.;
  }
}

PLUGIN_END_NAMESPACE
