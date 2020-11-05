/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin, Arpa partition
 *           Target tracking
 * Authors:  Douwe Fokkema
 *           Kees Verruijt
 *           Håkan Svensson
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
 *   Copyright (C) 2013-2016 by Douwe Fokkkema             df@percussion.nl*
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

#include "RadarMarpa.h"
#include "GuardZone.h"
#include "RadarCanvas.h"
#include "RadarInfo.h"
#include "drawutil.h"
#include "radar_pi.h"

PLUGIN_BEGIN_NAMESPACE

static int target_id_count = 0;

RadarArpa::RadarArpa(radar_pi* pi, RadarInfo* ri) {
  m_ri = ri;
  m_pi = pi;
  m_number_of_targets = 0;
  CLEAR_STRUCT(m_targets);
}

ArpaTarget::~ArpaTarget() {
  if (m_kalman) {
    delete m_kalman;
    m_kalman = 0;
  }
}

RadarArpa::~RadarArpa() {
  int n = m_number_of_targets;
  m_number_of_targets = 0;
  for (int i = 0; i < n; i++) {
    if (m_targets[i]) {
      delete m_targets[i];
      m_targets[i] = 0;
    }
  }
}

ExtendedPosition ArpaTarget::Polar2Pos(Polar pol, ExtendedPosition own_ship) {
  // The "own_ship" in the function call can be the position at an earlier time than the current position
  // converts in a radar image angular data r ( 0 - max_spoke_len ) and angle (0 - max_spokes) to position (lat, lon)
  // based on the own ship position own_ship
  ExtendedPosition pos;

  pos.pos.lat = own_ship.pos.lat + ((double)pol.r / m_ri->m_pixels_per_meter)  // Scale to fraction of distance from radar
                                       * cos(deg2rad(SCALE_SPOKES_TO_DEGREES(pol.angle))) / 60. / 1852.;
  pos.pos.lon = own_ship.pos.lon + ((double)pol.r / m_ri->m_pixels_per_meter)  // Scale to fraction of distance to radar
                                       * sin(deg2rad(SCALE_SPOKES_TO_DEGREES(pol.angle))) / cos(deg2rad(own_ship.pos.lat)) / 60. /
                                       1852.;
  return pos;
}

Polar ArpaTarget::Pos2Polar(ExtendedPosition p, ExtendedPosition own_ship) {
  // converts in a radar image a lat-lon position to angular data relative to position own_ship
  Polar pol;
  double dif_lat = p.pos.lat;
  dif_lat -= own_ship.pos.lat;
  double dif_lon = (p.pos.lon - own_ship.pos.lon) * cos(deg2rad(own_ship.pos.lat));
  pol.r = (int)(sqrt(dif_lat * dif_lat + dif_lon * dif_lon) * 60. * 1852. * m_ri->m_pixels_per_meter + 1);
  pol.angle = (int)((atan2(dif_lon, dif_lat)) * (double)m_ri->m_spokes / (2. * PI) + 1);  // + 1 to minimize rounding errors
  if (pol.angle < 0) pol.angle += m_ri->m_spokes;
  return pol;
}

bool RadarArpa::Pix(int ang, int rad) {
  if (rad <= 0 || rad >= (int)m_ri->m_spoke_len_max) {
    return false;
  }
  return ((m_ri->m_history[MOD_SPOKES(ang)].line[rad] & 128) != 0);
}

bool ArpaTarget::Pix(int ang, int rad) {
  if (rad <= 0 || rad >= (int)m_ri->m_spoke_len_max) {
    return false;
  }
  if (m_check_for_duplicate) {
    // check bit 1
    return ((m_ri->m_history[MOD_SPOKES(ang)].line[rad] & 64) != 0);
  } else {
    // check bit 0
    return ((m_ri->m_history[MOD_SPOKES(ang)].line[rad] & 128) != 0);
  }
}

bool ArpaTarget::MultiPix(int ang, int rad) {  // checks if the blob has a contour of at least length pixels
  // pol must start on the contour of the blob
  // false if not
  // if false clears out pixels of the blob in hist
  wxCriticalSectionLocker lock(ArpaTarget::m_ri->m_exclusive);
  int length = m_ri->m_min_contour_length;
  Polar start;
  start.angle = ang;
  start.r = rad;
  if (!Pix(start.angle, start.r)) {
    return false;
  }
  Polar current = start;  // the 4 possible translations to move from a point on the contour to the next
  Polar max_angle;
  Polar min_angle;
  Polar max_r;
  Polar min_r;
  Polar transl[4];  //   = { 0, 1,   1, 0,   0, -1,   -1, 0 };
  transl[0].angle = 0;
  transl[0].r = 1;
  transl[1].angle = 1;
  transl[1].r = 0;
  transl[2].angle = 0;
  transl[2].r = -1;
  transl[3].angle = -1;
  transl[3].r = 0;
  int count = 0;
  int aa;
  int rr;
  bool succes = false;
  int index = 0;
  max_r = current;
  max_angle = current;
  min_r = current;
  min_angle = current;  // check if p inside blob
  if (start.r >= (int)m_ri->m_spoke_len_max) {
    return false;  //  r too large
  }
  if (start.r < 3) {
    return false;  //  r too small
  }
  // first find the orientation of border point p
  for (int i = 0; i < 4; i++) {
    index = i;
    aa = current.angle + transl[index].angle;
    rr = current.r + transl[index].r;
    succes = !Pix(aa, rr);
    if (succes) break;
  }
  if (!succes) {
    // single pixel blob
    return false;
  }
  index += 1;  // determines starting direction
  if (index > 3) index -= 4;
  while (current.r != start.r || current.angle != start.angle || count == 0) {  // try all translations to find the next point
    // start with the "left most" translation relative to the
    // previous one
    index += 3;  // we will turn left all the time if possible
    for (int i = 0; i < 4; i++) {
      if (index > 3) index -= 4;
      aa = current.angle + transl[index].angle;
      rr = current.r + transl[index].r;
      succes = Pix(aa, rr);
      if (succes) {  // next point found
        break;
      }
      index += 1;
    }
    if (!succes) {
      return false;  // no next point found (this happens when the blob consists of one single pixel)
    }                // next point found
    current.angle = aa;
    current.r = rr;
    if (count >= length) {
      return true;
    }
    count++;
    if (current.angle > max_angle.angle) {
      max_angle = current;
    }
    if (current.angle < min_angle.angle) {
      min_angle = current;
    }
    if (current.r > max_r.r) {
      max_r = current;
    }
    if (current.r < min_r.r) {
      min_r = current;
    }
  }  // contour length is less than m_min_contour_length
     // before returning false erase this blob so we do not have to check this one again
  if (min_angle.angle < 0) {
    min_angle.angle += m_ri->m_spokes;
    max_angle.angle += m_ri->m_spokes;
  }
  for (int a = min_angle.angle; a <= max_angle.angle; a++) {
    for (int r = min_r.r; r <= max_r.r; r++) {
      m_ri->m_history[MOD_SPOKES(a)].line[r] &= 63;
    }
  }
  return false;
}

bool RadarArpa::MultiPix(int ang, int rad) {
  // checks the blob has a contour of at least length pixels
  // pol must start on the contour of the blob
  // false if not
  // if false clears out pixels of th blob in hist
  //    wxCriticalSectionLocker lock(ArpaTarget::m_ri->m_exclusive);
  int length = m_ri->m_min_contour_length;
  Polar start;
  start.angle = ang;
  start.r = rad;
  if (!Pix(start.angle, start.r)) {
    return false;
  }
  Polar current = start;  // the 4 possible translations to move from a point on the contour to the next
  Polar max_angle;
  Polar min_angle;
  Polar max_r;
  Polar min_r;
  Polar transl[4];  //   = { 0, 1,   1, 0,   0, -1,   -1, 0 };
  transl[0].angle = 0;
  transl[0].r = 1;
  transl[1].angle = 1;
  transl[1].r = 0;
  transl[2].angle = 0;
  transl[2].r = -1;
  transl[3].angle = -1;
  transl[3].r = 0;
  int count = 0;
  int aa;
  int rr;
  bool succes = false;
  int index = 0;
  max_r = current;
  max_angle = current;
  min_r = current;
  min_angle = current;  // check if p inside blob
  if (start.r >= (int)m_ri->m_spoke_len_max) {
    return false;  //  r too large
  }
  if (start.r < 3) {
    return false;  //  r too small
  }
  // first find the orientation of border point p
  for (int i = 0; i < 4; i++) {
    index = i;
    aa = current.angle + transl[index].angle;
    rr = current.r + transl[index].r;
    succes = !Pix(aa, rr);
    if (succes) break;
  }
  if (!succes) {
    return false;
  }
  index += 1;  // determines starting direction
  if (index > 3) index -= 4;
  while (current.r != start.r || current.angle != start.angle ||
         count == 0) {  // try all translations to find the next point  // start with the "left most" translation relative to the
    // previous one
    index += 3;  // we will turn left all the time if possible
    for (int i = 0; i < 4; i++) {
      if (index > 3) index -= 4;
      aa = current.angle + transl[index].angle;
      rr = current.r + transl[index].r;
      succes = Pix(aa, rr);
      if (succes) {  // next point found
        break;
      }
      index += 1;
    }
    if (!succes) {
      return false;  // no next point found
    }                // next point found
    current.angle = aa;
    current.r = rr;
    if (count >= length) {
      return true;
    }
    count++;
    if (current.angle > max_angle.angle) {
      max_angle = current;
    }
    if (current.angle < min_angle.angle) {
      min_angle = current;
    }
    if (current.r > max_r.r) {
      max_r = current;
    }
    if (current.r < min_r.r) {
      min_r = current;
    }
  }  // contour length is less than m_min_contour_length
  // before returning false erase this blob so we do not have to check this one again
  if (min_angle.angle < 0) {
    min_angle.angle += m_ri->m_spokes;
    max_angle.angle += m_ri->m_spokes;
  }
  for (int a = min_angle.angle; a <= max_angle.angle; a++) {
    for (int r = min_r.r; r <= max_r.r; r++) {
      m_ri->m_history[MOD_SPOKES(a)].line[r] &= 63;
    }
  }
  return false;
}

void RadarArpa::AcquireNewMARPATarget(ExtendedPosition target_pos) { AcquireOrDeleteMarpaTarget(target_pos, ACQUIRE0); }

void RadarArpa::DeleteTarget(ExtendedPosition target_pos) { AcquireOrDeleteMarpaTarget(target_pos, FOR_DELETION); }

void RadarArpa::AcquireOrDeleteMarpaTarget(ExtendedPosition target_pos, int status) {
  // acquires new target from mouse click position
  // no contour taken yet
  // target status acquire0
  // returns in X metric coordinates of click
  // constructs Kalman filter
  // make new target
  int i_target;
  if (m_number_of_targets < MAX_NUMBER_OF_TARGETS - 1 ||
      (m_number_of_targets == MAX_NUMBER_OF_TARGETS - 1 && status == FOR_DELETION)) {
    if (m_targets[m_number_of_targets] == 0) {
      m_targets[m_number_of_targets] = new ArpaTarget(m_pi, m_ri);
    }
    i_target = m_number_of_targets;
    m_number_of_targets++;
  } else {
    LOG_INFO(wxT("radar_pi: RadarArpa:: Error, max targets exceeded "));
    return;
  }

  LOG_ARPA(wxT("radar_pi: Adding (M)ARPA target at position %f / %f"), target_pos.pos.lat, target_pos.pos.lon);

  ArpaTarget* target = m_targets[i_target];
  target->m_position = target_pos;  // Expected position
  target->m_position.time = 0;
  target->m_position.dlat_dt = 0.;
  target->m_position.dlon_dt = 0.;
  target->m_status = status;

  target->m_max_angle.angle = 0;
  target->m_min_angle.angle = 0;
  target->m_max_r.r = 0;
  target->m_min_r.r = 0;

  if (!target->m_kalman) {
    target->m_kalman = new KalmanFilter(m_ri->m_spokes);
  }
  target->m_automatic = false;
  return;
}

bool ArpaTarget::FindContourFromInside(Polar* pol) {  // moves pol to contour of blob
  // true if success
  // false when failed
  int ang = pol->angle;
  int rad = pol->r;
  if (rad >= (int)m_ri->m_spoke_len_max || rad < 3) {
    return false;
  }
  if (!(Pix(ang, rad))) {
    return false;
  }
  while (Pix(ang, rad)) {
    ang--;
  }
  ang++;
  pol->angle = ang;
  // check if the blob has the required min contour length
  if (MultiPix(ang, rad)) {
    return true;
  } else {
    return false;
  }
}

/**
 * Find a contour from the given start position on the edge of a blob.
 *
 * Follows the contour in a clockwise manner.
 *
 * Returns 0 if ok, or a small integer on error (but nothing is done with this)
 */
int ArpaTarget::GetContour(Polar* pol) {
  wxCriticalSectionLocker lock(ArpaTarget::m_ri->m_exclusive);
  // the 4 possible translations to move from a point on the contour to the next
  Polar transl[4];  //   = { 0, 1,   1, 0,   0, -1,   -1, 0 };
  transl[0].angle = 0;
  transl[0].r = 1;

  transl[1].angle = 1;
  transl[1].r = 0;

  transl[2].angle = 0;
  transl[2].r = -1;

  transl[3].angle = -1;
  transl[3].r = 0;

  int count = 0;
  Polar start = *pol;
  Polar current = *pol;
  int aa;
  int rr;

  bool succes = false;
  int index = 0;
  m_max_r = current;
  m_max_angle = current;
  m_min_r = current;
  m_min_angle = current;
  // check if p inside blob
  if (start.r >= (int)m_ri->m_spoke_len_max) {
    return 1;  // return code 1, r too large
  }
  if (start.r < 4) {
    return 2;  // return code 2, r too small
  }
  if (!Pix(start.angle, start.r)) {
    return 3;  // return code 3, starting point outside blob
  }
  // first find the orientation of border point p
  for (int i = 0; i < 4; i++) {
    index = i;
    aa = current.angle + transl[index].angle;
    rr = current.r + transl[index].r;
    //  if (rr > RETURNS_PER_LINE - 1) return 13;  // r too large
    succes = !Pix(aa, rr);
    if (succes) break;
  }
  if (!succes) {
    return 4;  // return code 4, starting point not on contour
  }
  index += 1;  // determines starting direction
  if (index > 3) index -= 4;

  while (current.r != start.r || current.angle != start.angle || count == 0) {
    // try all translations to find the next point
    // start with the "left most" translation relative to the previous one
    index += 3;  // we will turn left all the time if possible
    for (int i = 0; i < 4; i++) {
      if (index > 3) index -= 4;
      aa = current.angle + transl[index].angle;
      rr = current.r + transl[index].r;
      succes = Pix(aa, rr);
      if (succes) {
        // next point found

        break;
      }
      index += 1;
    }
    if (!succes) {
      LOG_INFO(wxT("radar_pi::RadarArpa::GetContour no next point found count= %i"), count);
      return 7;  // return code 7, no next point found
    }
    // next point found
    current.angle = aa;
    current.r = rr;
    if (count < MAX_CONTOUR_LENGTH - 2) {
      m_contour[count] = current;
    }
    if (count == MAX_CONTOUR_LENGTH - 2) {
      m_contour[count] = start;  // shortcut to the beginning for drawing the contour
      current = start;           // this will cause the while to terminate
    }
    if (count < MAX_CONTOUR_LENGTH - 1) {
      count++;
    }
    if (current.angle > m_max_angle.angle) {
      m_max_angle = current;
    }
    if (current.angle < m_min_angle.angle) {
      m_min_angle = current;
    }
    if (current.r > m_max_r.r) {
      m_max_r = current;
    }
    if (current.r < m_min_r.r) {
      m_min_r = current;
    }
  }
  m_contour_length = count;
  //  CalculateCentroid(*target);    we better use the real centroid instead of the average, todo
  if (m_min_angle.angle < 0) {
    m_min_angle.angle += m_ri->m_spokes;
    m_max_angle.angle += m_ri->m_spokes;
  }
  pol->angle = (m_max_angle.angle + m_min_angle.angle) / 2;
  if (m_max_r.r >= (int)m_ri->m_spoke_len_max || m_min_r.r >= (int)m_ri->m_spoke_len_max) {
    return 10;  // return code 10 r too large
  }
  if (m_max_r.r < 2 || m_min_r.r < 2) {
    return 11;  // return code 11 r too small
  }
  if (pol->angle >= (int)m_ri->m_spokes) {
    pol->angle -= m_ri->m_spokes;
  }
  pol->r = (m_max_r.r + m_min_r.r) / 2;
  pol->time = m_ri->m_history[MOD_SPOKES(pol->angle)].time;
  m_radar_pos = m_ri->m_history[MOD_SPOKES(pol->angle)].pos;

  double poslat = m_radar_pos.lat;
  double poslon = m_radar_pos.lon;
  if (poslat > 90. || poslat < -90. || poslon > 180. || poslon < -180.) {
    // some additional logging, to be removed later
    LOG_INFO(wxT("**error wrong target pos, poslat = %f, poslon = %f"), poslat, poslon);
  }
  return 0;  //  success, blob found
}

void RadarArpa::DrawContour(ArpaTarget* target) {
  if (target->m_lost_count > 0) {
    return;  // don't draw targets that were not seen last sweep
  }
  wxColor arpa = m_pi->m_settings.arpa_colour;
  glColor4ub(arpa.Red(), arpa.Green(), arpa.Blue(), arpa.Alpha());
  glLineWidth(3.0);

  glEnableClientState(GL_VERTEX_ARRAY);

  Point vertex_array[MAX_CONTOUR_LENGTH + 1];
  for (int i = 0; i < target->m_contour_length; i++) {
    int angle = target->m_contour[i].angle + (DEGREES_PER_ROTATION + OPENGL_ROTATION) * m_ri->m_spokes / DEGREES_PER_ROTATION;
    int radius = target->m_contour[i].r;
    if (radius <= 0 || radius >= (int)m_ri->m_spoke_len_max) {
      LOG_INFO(wxT("radar_pi: wrong values in DrawContour"));
      return;
    }
    vertex_array[i] = m_ri->m_polar_lookup->GetPoint(angle, radius);
    vertex_array[i].x = vertex_array[i].x / m_ri->m_pixels_per_meter;
    vertex_array[i].y = vertex_array[i].y / m_ri->m_pixels_per_meter;
  }

  glVertexPointer(2, GL_FLOAT, 0, vertex_array);
  glDrawArrays(GL_LINE_STRIP, 0, target->m_contour_length);

  glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
}

void RadarArpa::DrawArpaTargetsOverlay(double scale, double arpa_rotate) {
  wxPoint boat_center;
  GeoPosition radar_pos;
  if (!m_pi->m_settings.drawing_method && m_ri->GetRadarPosition(&radar_pos)) {
    for (int i = 0; i < m_number_of_targets; i++) {
      if (!m_targets[i]) {
        continue;
      }
      if (m_targets[i]->m_status == LOST) {
        continue;
      }
      double poslat = m_targets[i]->m_radar_pos.lat;
      double poslon = m_targets[i]->m_radar_pos.lon;
      // some additional logging, to be removed later
      if (poslat > 90. || poslat < -90. || poslon > 180. || poslon < -180.) {
        LOG_INFO(wxT("**error wrong target pos, nr = %i, poslat = %f, poslon = %f"), i, poslat, poslon);
        continue;
      }

      GetCanvasPixLL(m_ri->m_pi->m_vp, &boat_center, m_targets[i]->m_radar_pos.lat, m_targets[i]->m_radar_pos.lon);
      glPushMatrix();
      glTranslated(boat_center.x, boat_center.y, 0);
      glRotated(arpa_rotate, 0.0, 0.0, 1.0);
      glScaled(scale, scale, 1.);
      DrawContour(m_targets[i]);
      glPopMatrix();
    }
  } else {
    m_ri->GetRadarPosition(&radar_pos);
    GetCanvasPixLL(m_ri->m_pi->m_vp, &boat_center, radar_pos.lat, radar_pos.lon);
    glPushMatrix();
    glTranslated(boat_center.x, boat_center.y, 0);
    glRotated(arpa_rotate, 0.0, 0.0, 1.0);
    glScaled(scale, scale, 1.);
    for (int i = 0; i < m_number_of_targets; i++) {
      if (!m_targets[i]) {
        continue;
      }
      if (m_targets[i]->m_status != LOST) {
        DrawContour(m_targets[i]);
      }
    }
    glPopMatrix();
  }
}

void RadarArpa::DrawArpaTargetsPanel(double scale, double arpa_rotate) {
  wxPoint boat_center;
  GeoPosition radar_pos, target_pos;
  double offset_lat = 0.;
  double offset_lon = 0.;

  if (!m_pi->m_settings.drawing_method && m_ri->GetRadarPosition(&radar_pos)) {
    m_ri->GetRadarPosition(&radar_pos);
    for (int i = 0; i < m_number_of_targets; i++) {
      if (!m_targets[i]) {
        continue;
      }
      if (m_targets[i]->m_status == LOST) {
        continue;
      }
      target_pos = m_targets[i]->m_radar_pos;
      offset_lat = (radar_pos.lat - target_pos.lat) * 60. * 1852. * m_ri->m_panel_zoom / m_ri->m_range.GetValue();
      offset_lon = (radar_pos.lon - target_pos.lon) * 60. * 1852. * cos(deg2rad(target_pos.lat)) * m_ri->m_panel_zoom /
                   m_ri->m_range.GetValue();
      glPushMatrix();
      glRotated(arpa_rotate, 0.0, 0.0, 1.0);
      glTranslated(-offset_lon, offset_lat, 0);
      glScaled(scale, scale, 1.);
      DrawContour(m_targets[i]);
      glPopMatrix();
    }
  }

  else {
    glPushMatrix();
    glTranslated(0., 0., 0.);
    glRotated(arpa_rotate, 0.0, 0.0, 1.0);
    glScaled(scale, scale, 1.);
    for (int i = 0; i < m_number_of_targets; i++) {
      if (!m_targets[i]) {
        continue;
      }
      if (m_targets[i]->m_status == LOST) {
        continue;
      }
      DrawContour(m_targets[i]);
    }
    glPopMatrix();
  }
}

void RadarArpa::CleanUpLostTargets() {
  // remove targets with status LOST and put them at the end
  // adjust m_number_of_targets
  int ii = 0;
  while (ii < m_number_of_targets) {
    if (m_targets[ii]) {
      if (m_targets[ii]->m_status == LOST) {
        // we keep the lost target for later use, destruction and construction is expensive
        ArpaTarget* lost = m_targets[ii];
        int len = sizeof(ArpaTarget*);
        // move rest of larget list up to keep them in sequence
        memmove(&m_targets[ii], &m_targets[ii] + 1, (m_number_of_targets - ii) * len);
        m_number_of_targets--;
        // set the lost target at the last position
        m_targets[m_number_of_targets] = lost;
      } else {
        ii++;
      }
    }
  }
}

void RadarArpa::RefreshArpaTargets() {
  CleanUpLostTargets();
  int target_to_delete = -1;
  // find a target with status FOR_DELETION if it is there
  for (int i = 0; i < m_number_of_targets; i++) {
    if (!m_targets[i]) continue;
    if (m_targets[i]->m_status == FOR_DELETION) {
      target_to_delete = i;
    }
  }
  if (target_to_delete != -1) {
    // delete the target that is closest to the target with status FOR_DELETION
    ExtendedPosition* deletePosition = &m_targets[target_to_delete]->m_position;
    double min_dist = 1000;
    int del_target = -1;
    for (int i = 0; i < m_number_of_targets; i++) {
      if (!m_targets[i]) continue;
      if (i == target_to_delete || m_targets[i]->m_status == LOST) continue;
      double dif_lat = deletePosition->pos.lat - m_targets[i]->m_position.pos.lat;
      double dif_lon = (deletePosition->pos.lon - m_targets[i]->m_position.pos.lon) * cos(deg2rad(deletePosition->pos.lat));
      double dist2 = dif_lat * dif_lat + dif_lon * dif_lon;
      if (dist2 < min_dist) {
        min_dist = dist2;
        del_target = i;
      }
    }
    // del_target is the index of the target closest to target with index target_to_delete
    if (del_target != -1) {
      m_targets[del_target]->SetStatusLost();
    }
    m_targets[target_to_delete]->SetStatusLost();
    // now first clean up the lost targets again
    CleanUpLostTargets();
  }

  ArpaTarget t;
  KalmanFilter k(m_ri->m_spokes);
  // main target refresh loop

  // pass 1 of target refresh
  int dist = TARGET_SEARCH_RADIUS1;
  for (int i = 0; i < m_number_of_targets; i++) {
    if (!m_targets[i]) {
      LOG_INFO(wxT("radar_pi:  error target non existent i=%i"), i);
      continue;
    }
    m_targets[i]->m_pass_nr = PASS1;
    if (m_targets[i]->m_pass1_result == NOT_FOUND_IN_PASS1) continue;
    m_targets[i]->RefreshTarget(dist);
    if (m_targets[i]->m_pass1_result == NOT_FOUND_IN_PASS1) {
    }
  }

  // pass 2 of target refresh
  dist = TARGET_SEARCH_RADIUS2;
  for (int i = 0; i < m_number_of_targets; i++) {
    if (!m_targets[i]) {
      LOG_INFO(wxT("radar_pi: error target non existent i=%i"), i);
      continue;
    }
    if (m_targets[i]->m_pass1_result == UNKNOWN) continue;
    m_targets[i]->m_pass_nr = PASS2;
    m_targets[i]->RefreshTarget(dist);
  }

  for (int i = 0; i < GUARD_ZONES; i++) m_ri->m_guard_zone[i]->SearchTargets();
}

void ArpaTarget::RefreshTarget(int dist) {
  ExtendedPosition prev_X;
  ExtendedPosition prev2_X;
  ExtendedPosition own_pos;
  Polar pol;
  double delta_t;
  LocalPosition x_local;
  wxLongLong prev_refresh = m_refresh;
  // refresh may be called from guard directly, better check
  if (m_status == LOST || !m_ri->GetRadarPosition(&own_pos.pos)) {
    return;
  }
  pol = Pos2Polar(m_position, own_pos);
  wxLongLong time1 = m_ri->m_history[MOD_SPOKES(pol.angle)].time;
  int margin = SCAN_MARGIN;
  if (m_pass_nr == PASS2) margin += 100;
  wxLongLong time2 = m_ri->m_history[MOD_SPOKES(pol.angle + margin)].time;
  // check if target has been refreshed since last time (at least SCAN_MARGIN2 later)
  // and if the beam has passed the target location with SCAN_MARGIN spokes
  // the beam sould have passed our "angle" AND a point SCANMARGIN further
  // always refresh when status == 0
  if ((time1 < (m_refresh + SCAN_MARGIN2) || time2 < time1) && m_status != 0) {
    wxLongLong now = wxGetUTCTimeMillis();  // millis
    int diff = now.GetLo() - m_refresh.GetLo();
    if (diff > 8000) {
      LOG_ARPA(wxT("radar_pi: target not refreshed, missing spokes, set lost, status= %i, target_id= %i timediff= %i"), m_status,
               m_target_id, diff);
      SetStatusLost();
    }
    return;
  }
  // set new refresh time
  m_refresh = time1;
  prev2_X = prev_X;
  prev_X = m_position;  // save the previous target position

  // for test only
  /* if (status == 0) {
       target_id_count++;
       if (target_id_count >= 10000) target_id_count = 1;
       target_id = target_id_count;
   }*/

  // PREDICTION CYCLE

  m_position.time = time1;                                                // estimated new target time
  delta_t = ((double)((m_position.time - prev_X.time).GetLo())) / 1000.;  // in seconds
  if (m_status == 0) {
    delta_t = 0.;
  }
  if (m_position.pos.lat > 90.) {
    SetStatusLost();
    return;
  }
  x_local.pos.lat = (m_position.pos.lat - own_pos.pos.lat) * 60. * 1852.;                                  // in meters
  x_local.pos.lon = (m_position.pos.lon - own_pos.pos.lon) * 60. * 1852. * cos(deg2rad(own_pos.pos.lat));  // in meters
  x_local.dlat_dt = m_position.dlat_dt;                                                                    // meters / sec
  x_local.dlon_dt = m_position.dlon_dt;                                                                    // meters / sec
  m_kalman->Predict(&x_local, delta_t);  // x_local is new estimated local position of the target
                                         // now set the polar to expected angular position from the expected local position
  pol.angle = (int)(atan2(x_local.pos.lon, x_local.pos.lat) * m_ri->m_spokes / (2. * PI));
  if (pol.angle < 0) pol.angle += m_ri->m_spokes;
  pol.r = (int)(sqrt(x_local.pos.lat * x_local.pos.lat + x_local.pos.lon * x_local.pos.lon) * m_ri->m_pixels_per_meter);
  // zooming and target movement may  cause r to be out of bounds
  if (pol.r >= (int)m_ri->m_spoke_len_max || pol.r <= 0) {
    SetStatusLost();
    return;
  }
  m_expected = pol;  // save expected polar position

  // MEASUREMENT CYCLE

  // now search for the target at the expected polar position in pol
  int dist1 = dist;
  Polar back = pol;
  if (GetTarget(&pol, dist1)) {
    ResetPixels();
    // target too large? (land masses?) get rid of it
    if (abs(back.r - pol.r) > MAX_TARGET_DIAMETER || abs(m_max_r.r - m_min_r.r) > MAX_TARGET_DIAMETER ||
        abs(m_min_angle.angle - m_max_angle.angle) > MAX_TARGET_DIAMETER) {
      SetStatusLost();
      return;
    }
    // target refreshed, measured position in pol
    // check if target has a new later time than previous target
    if (pol.time <= prev_X.time && m_status > 1) {
      // found old target again, reset what we have done
      LOG_INFO(wxT("radar_pi: Error Gettarget same time found"));
      m_position = prev_X;
      prev_X = prev2_X;
      return;
    }
    m_lost_count = 0;
    if (m_status == ACQUIRE0) {
      // as this is the first measurement, move target to measured position
      ExtendedPosition p_own;
      p_own.pos = m_ri->m_history[MOD_SPOKES(pol.angle)].pos;  // get the position at receive time
      m_position = Polar2Pos(pol, p_own);                      // using own ship location from the time of reception
      m_position.dlat_dt = 0.;
      m_position.dlon_dt = 0.;
      m_expected = pol;
      m_position.sd_speed_kn = 0.;
    }
    m_status++;
    // target gets an id when status  == STATUS_TO_OCPN
    if (m_status == STATUS_TO_OCPN) {
      target_id_count++;
      if (target_id_count >= 10000) target_id_count = 1;
      m_target_id = target_id_count;
    }
    // Kalman filter to  calculate the apostriori local position and speed based on found position (pol)
    if (m_status > 1) {
      m_kalman->Update_P();
      m_kalman->SetMeasurement(&pol, &x_local, &m_expected,
                               m_ri->m_pixels_per_meter);  // pol is measured position in polar coordinates
    }

    // x_local expected position in local coordinates

    m_position.time = pol.time;  // set the target time to the newly found time
  }                              // end of target found

  // target not found
  else {
    // target not found
    if (m_pass_nr == PASS1) m_kalman->Update_P();
    // check if the position of the target has been taken by another target, a duplicate
    // if duplicate, handle target as not found but don't do pass 2 (= search in the surroundings)
    bool duplicate = false;
    m_check_for_duplicate = true;
    if (m_pass_nr == PASS1 && GetTarget(&pol, dist1)) {
      m_pass1_result = UNKNOWN;
      duplicate = true;
    }
    m_check_for_duplicate = false;

    // not found in pass 1
    // try again later in pass 2 with a larger distance
    if (m_pass_nr == PASS1 && !duplicate) {
      m_pass1_result = NOT_FOUND_IN_PASS1;
      // reset what we have done
      pol.time = prev_X.time;
      m_refresh = prev_refresh;
      m_position = prev_X;
      prev_X = prev2_X;
      return;
    }

    // delete low status targets immediately when not found
    if (m_status == ACQUIRE0 || m_status == ACQUIRE1 || m_status == 2) {
      SetStatusLost();
      return;
    }

    m_lost_count++;

    // delete if not found too often
    if (m_lost_count > MAX_LOST_COUNT) {
      SetStatusLost();
      return;
    }
  }  // end of target not found
  // set pass1_result ready for next sweep
  m_pass1_result = UNKNOWN;
  if (m_status != ACQUIRE1) {
    // if status == 1, then this was first measurement, keep position at measured position
    m_position.pos.lat = own_pos.pos.lat + x_local.pos.lat / 60. / 1852.;
    m_position.pos.lon = own_pos.pos.lon + x_local.pos.lon / 60. / 1852. / cos(deg2rad(own_pos.pos.lat));
    m_position.dlat_dt = x_local.dlat_dt;  // meters / sec
    m_position.dlon_dt = x_local.dlon_dt;  // meters /sec
    m_position.sd_speed_kn = x_local.sd_speed_m_s * 3600. / 1852.;
  }

  // set refresh time to the time of the spoke where the target was found
  m_refresh = m_position.time;
  if (m_status >= 1) {
    if (m_status == 2) {
      // avoid extreme start-up speeds
      if (m_position.dlat_dt > START_UP_SPEED) m_position.dlat_dt = START_UP_SPEED;
      if (m_position.dlat_dt < -START_UP_SPEED) m_position.dlat_dt = -START_UP_SPEED;
      if (m_position.dlon_dt > START_UP_SPEED) m_position.dlon_dt = START_UP_SPEED;
      if (m_position.dlon_dt < -START_UP_SPEED) m_position.dlon_dt = -START_UP_SPEED;
    }
    if (m_status == 3) {
      // avoid extreme start-up speeds
      if (m_position.dlat_dt > 2 * START_UP_SPEED) m_position.dlat_dt = 2 * START_UP_SPEED;
      if (m_position.dlat_dt < -2 * START_UP_SPEED) m_position.dlat_dt = -2 * START_UP_SPEED;
      if (m_position.dlon_dt > 2 * START_UP_SPEED) m_position.dlon_dt = 2 * START_UP_SPEED;
      if (m_position.dlon_dt < -2 * START_UP_SPEED) m_position.dlon_dt = -2 * START_UP_SPEED;
    }
    double s1 = m_position.dlat_dt;                          // m per second
    double s2 = m_position.dlon_dt;                          // m  per second
    m_speed_kn = (sqrt(s1 * s1 + s2 * s2)) * 3600. / 1852.;  // and convert to nautical miles per hour
    m_course = rad2deg(atan2(s2, s1));
    if (m_course < 0) m_course += 360.;
    if (m_speed_kn > 20.) {
      pol = Pos2Polar(m_position, own_pos);
    }

    if (m_speed_kn < (double)TARGET_SPEED_DIV_SDEV * m_position.sd_speed_kn) {
      m_speed_kn = 0.;
      m_course = 0.;
      if (m_stationary < 2) {
        m_stationary++;
      }
    } else if (m_stationary > 0) {
      m_stationary--;
    }

    // send target data to OCPN
    pol = Pos2Polar(m_position, own_pos);
    if (m_status >= STATUS_TO_OCPN) {
      OCPN_target_status s;
      if (m_status >= Q_NUM) s = Q;
      if (m_status > T_NUM) s = T;
      if (m_lost_count > 0) {
        // if target was not seen last sweep, color yellow
        s = Q;
      }
      // Check for AIS target at (M)ARPA position
      double dist2target = pol.r / m_ri->m_pixels_per_meter;
      if (m_pi->FindAIS_at_arpaPos(m_position.pos, dist2target)) s = L;
      PassARPAtoOCPN(&pol, s);
    }
  }
  return;
}

#define PIX(aa, rr)                                   \
  if (rr >= (int)m_ri->m_spoke_len_max - 1) continue; \
  if (MultiPix(aa, rr)) {                             \
    pol->angle = aa;                                  \
    pol->r = rr;                                      \
    return true;                                      \
  }

bool ArpaTarget::FindNearestContour(Polar* pol, int dist) {
  // make a search pattern along a square
  // returns the position of the nearest blob found in pol
  // dist is search radius (1 more or less) in radial pixels
  int a = pol->angle;
  int r = pol->r;
  if (dist < 2) dist = 2;
  for (int j = 1; j <= dist; j++) {
    int dist_r = j;
    int dist_a = (int)(326. / (double)r * j);  // 326/r: conversion factor to make squares
                                               // if r == 326 circle would be 28 * PI * 326 = 2048
    if (dist_a == 0) dist_a = 1;
    for (int i = 0; i <= dist_a; i++) {  // "upper" side
      PIX(a - i, r + dist_r);            // search starting from the middle
      PIX(a + i, r + dist_r);
    }
    for (int i = 0; i < dist_r; i++) {  // "right hand" side
      PIX(a + dist_a, r + i);
      PIX(a + dist_a, r - i);
    }
    for (int i = 0; i <= dist_a; i++) {  // "lower" side
      PIX(a + i, r - dist_r);
      PIX(a - i, r - dist_r);
    }
    for (int i = 0; i < dist_r; i++) {  // "left hand" side
      PIX(a - dist_a, r + i);
      PIX(a - dist_a, r - i);
    }
  }
  return false;
}

void RadarArpa::CalculateCentroid(ArpaTarget* target) {
  // real calculation still to be done
}

ArpaTarget::ArpaTarget(radar_pi* pi, RadarInfo* ri) {
  ArpaTarget::m_ri = ri;
  m_pi = pi;
  m_kalman = 0;
  m_status = LOST;
  m_contour_length = 0;
  m_lost_count = 0;
  m_target_id = 0;
  m_refresh = 0;
  m_automatic = false;
  m_speed_kn = 0.;
  m_course = 0.;
  m_stationary = 0;
  m_position.dlat_dt = 0.;
  m_position.dlon_dt = 0.;
  m_pass1_result = UNKNOWN;
  m_pass_nr = PASS1;
}

ArpaTarget::ArpaTarget() {
  m_kalman = 0;
  m_status = LOST;
  m_contour_length = 0;
  m_lost_count = 0;
  m_target_id = 0;
  m_refresh = 0;
  m_automatic = false;
  m_speed_kn = 0.;
  m_course = 0.;
  m_stationary = 0;
  m_position.dlat_dt = 0.;
  m_position.dlon_dt = 0.;
  m_pass1_result = UNKNOWN;
  m_pass_nr = PASS1;
}

bool ArpaTarget::GetTarget(Polar* pol, int dist1) {
  // general target refresh
  bool contour_found = false;
  int dist = dist1;

  if (m_status == ACQUIRE0 || m_status == ACQUIRE1) {
    dist *= 2;
  }
  if (dist > pol->r - 5) {
    dist = pol->r - 5;  // don't search close to origin
  }

  int a = pol->angle;
  int r = pol->r;

  if (Pix(a, r)) {
    contour_found = FindContourFromInside(pol);
  } else {
    contour_found = FindNearestContour(pol, dist);
  }
  if (!contour_found) {
    return false;
  }
  int cont = GetContour(pol);
  if (cont != 0) {
    // LOG_ARPA(wxT("radar_pi: ARPA contour error %d at %d, %d"), cont, a, r);
    // reset pol in case of error
    pol->angle = a;
    pol->r = r;
    return false;
  }
  return true;
}

void ArpaTarget::PassARPAtoOCPN(Polar* pol, OCPN_target_status status) {
  wxString s_TargID, s_Bear_Unit, s_Course_Unit;
  wxString s_speed, s_course, s_Dist_Unit, s_status;
  wxString s_bearing;
  wxString s_distance;
  wxString s_target_name;
  wxString nmea;
  char sentence[90];
  char checksum = 0;
  char* p;

  s_Bear_Unit = wxEmptyString;  // Bearing Units  R or empty
  s_Course_Unit = wxT("T");     // Course type R; Realtive T; true
  s_Dist_Unit = wxT("N");       // Speed/Distance Unit K, N, S N= NM/h = Knots
  switch (status) {
    case Q:
      s_status = wxT("Q");  // yellow
      break;
    case T:
      s_status = wxT("T");  // green
      break;
    case L:
      s_status = wxT("L");  // ?
      break;
  }

  double dist = pol->r / m_ri->m_pixels_per_meter / 1852.;
  double bearing = pol->angle * 360. / m_ri->m_spokes;

  if (bearing < 0) bearing += 360;
  s_TargID = wxString::Format(wxT("%2i"), m_target_id);
  s_speed = wxString::Format(wxT("%4.2f"), m_speed_kn);
  s_course = wxString::Format(wxT("%3.1f"), m_course);
  if (m_automatic) {
    s_target_name = wxString::Format(wxT("ARPA%2i"), m_target_id);
  } else {
    s_target_name = wxString::Format(wxT("MARPA%2i"), m_target_id);
  }
  s_distance = wxString::Format(wxT("%f"), dist);
  s_bearing = wxString::Format(wxT("%f"), bearing);

  /* Code for TTM follows. Send speed and course using TTM*/
  snprintf(sentence, sizeof(sentence), "RATTM,%2s,%s,%s,%s,%s,%s,%s, , ,%s,%s,%s, ",
           (const char*)s_TargID.mb_str(),       // 1 target id
           (const char*)s_distance.mb_str(),     // 2 Targ distance
           (const char*)s_bearing.mb_str(),      // 3 Bearing fr own ship.
           (const char*)s_Bear_Unit.mb_str(),    // 4 Brearing unit ( T = true)
           (const char*)s_speed.mb_str(),        // 5 Target speed
           (const char*)s_course.mb_str(),       // 6 Target Course.
           (const char*)s_Course_Unit.mb_str(),  // 7 Course ref T // 8 CPA Not used // 9 TCPA Not used
           (const char*)s_Dist_Unit.mb_str(),    // 10 S/D Unit N = knots/Nm
           (const char*)s_target_name.mb_str(),  // 11 Target name
           (const char*)s_status.mb_str());      // 12 Target Status L/Q/T // 13 Ref N/A

  for (p = sentence; *p; p++) {
    checksum ^= *p;
  }
  nmea.Printf(wxT("$%s*%02X\r\n"), sentence, (unsigned)checksum);
  PushNMEABuffer(nmea);
}

void ArpaTarget::SetStatusLost() {
  m_contour_length = 0;
  m_lost_count = 0;
  if (m_kalman) {
    // reset kalman filter, don't delete it, too  expensive
    m_kalman->ResetFilter();
  }
  if (m_status >= STATUS_TO_OCPN) {
    Polar p;
    p.angle = 0;
    p.r = 0;
    PassARPAtoOCPN(&p, L);
  }
  m_status = LOST;
  m_target_id = 0;
  m_automatic = false;
  m_refresh = 0;
  m_speed_kn = 0.;
  m_course = 0.;
  m_stationary = 0;
  m_position.dlat_dt = 0.;
  m_position.dlon_dt = 0.;
  m_pass_nr = PASS1;
}

void RadarArpa::DeleteAllTargets() {
  for (int i = 0; i < m_number_of_targets; i++) {
    if (!m_targets[i]) continue;
    m_targets[i]->SetStatusLost();
  }
}

int RadarArpa::AcquireNewARPATarget(Polar pol, int status) {
  // acquires new target from mouse click position
  // no contour taken yet
  // target status status, normally 0, if dummy target to delete a target -2
  // returns in X metric coordinates of click
  // constructs Kalman filter
  ExtendedPosition own_pos;
  ExtendedPosition target_pos;
  if (!m_ri->GetRadarPosition(&own_pos.pos)) {
    return -1;
  }
  // make new target or re-use an existing one with status == lost
  int i;
  if (m_number_of_targets < MAX_NUMBER_OF_TARGETS - 1 || (m_number_of_targets == MAX_NUMBER_OF_TARGETS - 1 && status == -2)) {
    if (!m_targets[m_number_of_targets]) {
      m_targets[m_number_of_targets] = new ArpaTarget(m_pi, m_ri);
    }
    i = m_number_of_targets;
    m_number_of_targets++;
  } else {
    LOG_INFO(wxT("radar_pi: RadarArpa:: Error, max targets exceeded %i"), m_number_of_targets);
    return -1;
  }
  ArpaTarget* target = m_targets[i];
  target_pos = target->Polar2Pos(pol, own_pos);

  target->m_position = target_pos;  // Expected position
  target->m_position.time = wxGetUTCTimeMillis();
  target->m_position.dlat_dt = 0.;
  target->m_position.dlon_dt = 0.;
  target->m_position.sd_speed_kn = 0.;
  target->m_status = status;
  target->m_max_angle.angle = 0;
  target->m_min_angle.angle = 0;
  target->m_max_r.r = 0;
  target->m_min_r.r = 0;
  if (!target->m_kalman) {
    target->m_kalman = new KalmanFilter(m_ri->m_spokes);
  }
  target->m_check_for_duplicate = false;
  target->m_automatic = true;
  target->m_target_id = 0;
  target->RefreshTarget(TARGET_SEARCH_RADIUS1);
  return i;
}

void ArpaTarget::ResetPixels() {
  // resets the pixels of the current blob (plus DISTANCE_BETWEEN_TARGETS) so that blob will not be found again in the same sweep
  // We not only reset the blob but all pixels in a radial "square" covering the blob
  for (int r = wxMax(m_min_r.r - DISTANCE_BETWEEN_TARGETS, 0);
       r <= wxMin(m_max_r.r + DISTANCE_BETWEEN_TARGETS, (int)m_ri->m_spoke_len_max - 1); r++) {
    for (int a = wxMax(m_min_angle.angle - DISTANCE_BETWEEN_TARGETS, 0);
         a <= wxMin(m_max_angle.angle + DISTANCE_BETWEEN_TARGETS, (int)m_ri->m_spokes - 1); a++) {
      m_ri->m_history[a].line[r] = m_ri->m_history[a].line[r] & 127;
    }
  }
}

void RadarArpa::ClearContours() {
  for (int i = 0; i < m_number_of_targets; i++) {
    m_targets[i]->m_contour_length = 0;
  }
}

PLUGIN_END_NAMESPACE
