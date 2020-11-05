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

#ifndef _RADAR_CONTROL_ITEM_H_
#define _RADAR_CONTROL_ITEM_H_

#include "pi_common.h"

PLUGIN_BEGIN_NAMESPACE

class radar_pi;

//
// a RadarControlItem encapsulates a particular control, for instance
// sea clutter or gain.
//
// Some controls are always only just a value.
// Some other controls have state as well.

enum RadarControlState {
  RCS_OFF = -1,
  RCS_MANUAL = 0,
  RCS_AUTO_1,
  RCS_AUTO_2,
  RCS_AUTO_3,
  RCS_AUTO_4,
  RCS_AUTO_5,
  RCS_AUTO_6,
  RCS_AUTO_7,
  RCS_AUTO_8,
  RCS_AUTO_9
};

class RadarControlItem {
 public:
  static const int VALUE_NOT_SET = -10000;

  RadarControlItem() {
    m_value = 0;
    m_state = RCS_OFF;
    m_button_v = VALUE_NOT_SET;  // Unlikely value so that first actual set sets proper value + mod
    m_button_s = RCS_OFF;
    m_mod = true;
  }

  // The copy constructor
  RadarControlItem(const RadarControlItem &other) { Update(other.m_value, other.m_state); }

  // The assignment constructor
  RadarControlItem &operator=(const RadarControlItem &other) {
    if (this != &other) {  // self-assignment check expected
      Update(other.m_value, other.m_state);
    }
    return *this;
  }

  // The assignment constructor to allow "item = value"
  RadarControlItem &operator=(int v) {
    Update(v, RCS_MANUAL);
    return *this;
  }

  void Update(int v, RadarControlState s) {
    wxCriticalSectionLocker lock(m_exclusive);

    if (v != m_button_v || s != m_button_s) {
      m_mod = true;
      m_button_v = v;
      m_button_s = s;
    }
    m_value = v;
    m_state = s;
  };

  void UpdateState(RadarControlState s) {
    wxCriticalSectionLocker lock(m_exclusive);

    if (s != m_button_s) {
      m_mod = true;
      m_button_s = s;
    }
    m_state = s;
  };

  void Update(int v) { Update(v, RCS_MANUAL); };

  bool GetButton(int *value, RadarControlState *state) {
    wxCriticalSectionLocker lock(m_exclusive);
    if (value) {
      *value = this->m_button_v;
    }
    if (state) {
      *state = this->m_button_s;
    }

    bool changed = m_mod;
    m_mod = false;
    return changed;
  }

  bool GetButton(int *value) {
    wxCriticalSectionLocker lock(m_exclusive);
    if (value) {
      *value = this->m_button_v;
    }

    bool changed = m_mod;
    m_mod = false;
    return changed;
  }

  int GetButton() {
    wxCriticalSectionLocker lock(m_exclusive);

    m_mod = false;
    return m_button_v;
  }

  int GetValue() {
    wxCriticalSectionLocker lock(m_exclusive);

    return m_value;
  }

  RadarControlState GetState() {
    wxCriticalSectionLocker lock(m_exclusive);

    return m_state;
  }

  bool IsModified() {
    wxCriticalSectionLocker lock(m_exclusive);

    return m_mod;
  }

 protected:
  wxCriticalSection m_exclusive;
  int m_value;
  int m_button_v;
  RadarControlState m_state;
  RadarControlState m_button_s;
  bool m_mod;
};

/*
 * RadarRange is a little different. Since the 'Auto' range is computed
 * by the plugin and not a state coming from the radar any range change
 * coming from the radar should not be interpreted as being a change back
 * to manual state (RCS_MANUAL.)
 */
class RadarRangeControlItem : public RadarControlItem {
 public:
  RadarRangeControlItem() {
    m_value = 0;
    m_state = RCS_OFF;
    m_button_v = VALUE_NOT_SET;  // Unlikely value so that first actual set sets proper value + mod
    m_button_s = RCS_OFF;
    m_mod = true;
  }

  void Update(int v) {
    wxCriticalSectionLocker lock(m_exclusive);

    if (v != m_button_v) {
      m_mod = true;
      m_button_v = v;
    }
    m_value = v;
  };
};

PLUGIN_END_NAMESPACE

#endif
