/******************************************************************************
 * Project:  OpenCPN
 * Purpose:  AIS info GUI parts
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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

#include "ais_target_data.h"
#include "OCPN_Sound.h"

class AisInfoGui: public wxEvtHandler {
  AisInfoGui();

  void ShowAisInfo(AisTargetData* palert_target);
  bool AIS_AlertPlaying(void) { return m_bAIS_AlertPlaying; };

  bool m_bAIS_Audio_Alert_On;
  bool m_bAIS_AlertPlaying;
  OcpnSound* m_AIS_Sound;
  ObservedVarListener ais_info_listener;
  ObservedVarListener ais_touch_listener;
  ObservedVarListener ais_wp_listener;
  ObservedVarListener ais_new_track_listener;
  ObservedVarListener ais_del_track_listener;
};
