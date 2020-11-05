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

#ifndef _GUARDZONE_H_
#define _GUARDZONE_H_

#include "radar_pi.h"

PLUGIN_BEGIN_NAMESPACE

class GuardZone {
 public:
  GuardZoneType m_type;
  AngleDegrees m_start_bearing;
  AngleDegrees m_end_bearing;
  int m_inner_range;  // start in meters
  int m_outer_range;  // end   in meters
  int m_alarm_on;
  int m_arpa_on;
  time_t m_show_time;
  wxLongLong arpa_update_time[SPOKES_MAX];

  void ResetBogeys() {
    m_bogey_count = -1;
    m_running_count = 0;
    m_last_in_guard_zone = false;
    m_last_angle = 0;
  };

  void SetType(GuardZoneType type) {
    m_type = type;
    if (m_type > (GuardZoneType)1) m_type = (GuardZoneType)0;
    ResetBogeys();
  };
  void SetStartBearing(SpokeBearing start_bearing) {
    m_start_bearing = start_bearing;
    ResetBogeys();
  };
  void SetEndBearing(SpokeBearing end_bearing) {
    m_end_bearing = end_bearing;
    ResetBogeys();
  };
  void SetInnerRange(int inner_range) {
    m_inner_range = inner_range;
    ResetBogeys();
  };
  void SetOuterRange(int outer_range) {
    m_outer_range = outer_range;
    ResetBogeys();
  };
  void SetArpaOn(int arpa) { m_arpa_on = arpa; };
  void SetAlarmOn(int alarm) {
    m_alarm_on = alarm;
    if (m_alarm_on) {
      m_pi->m_guard_bogey_confirmed = false;
    } else {
      ResetBogeys();
    }
  };

  /*
   * Check if data is in this GuardZone, if so update bogeyCount
   */
  void ProcessSpoke(SpokeBearing angle, uint8_t *data, uint8_t *hist, size_t len);

  // Find targets inside the zone
  void SearchTargets();

  int GetBogeyCount() {
    if (m_bogey_count > -1) {
      LOG_GUARD(wxT("%s reporting bogey_count=%d"), m_log_name.c_str(), m_bogey_count);
    }
    return m_bogey_count;
  };

  GuardZone(radar_pi *pi, RadarInfo *ri, int zone);

  ~GuardZone() { LOG_VERBOSE(wxT("%s destroyed"), m_log_name.c_str()); }

 private:
  radar_pi *m_pi;
  RadarInfo *m_ri;

  wxString m_log_name;
  bool m_last_in_guard_zone;
  SpokeBearing m_last_angle;
  int m_bogey_count;    // complete cycle
  int m_running_count;  // current swipe

  void UpdateSettings();
};

PLUGIN_END_NAMESPACE

#endif /* _GUARDZONE_H_ */
