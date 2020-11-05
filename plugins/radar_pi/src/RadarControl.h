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

#ifndef _RADARCONTROL_H_
#define _RADARCONTROL_H_

#include "RadarInfo.h"
#include "pi_common.h"

PLUGIN_BEGIN_NAMESPACE

class RadarControl {
 public:
  RadarControl(){};
  virtual ~RadarControl(){};

  /*
   * Initialize any local data structures.
   *
   * @param pi                   The calling radar plugin instance
   * @param ri                   The parent RadarInfo instance
   * @param interfaceAddress     The IPv4 address of the ethernet card on this machine
   * @param radarAddress         The IPV4 address of the radar scanner
   */
  virtual bool Init(radar_pi *pi, RadarInfo *ri, NetworkAddress &interfaceAddress, NetworkAddress &radarAddress) = 0;

  /*
   * Ask the radar to switch off.
   */
  virtual void RadarTxOff() = 0;

  /*
   * Ask the radar to switch on.
   */
  virtual void RadarTxOn() = 0;

  /*
   * Send a 'keepalive' message to the radar.
   * This can be a null implementation if the radar does not need this.
   *
   * @returns   true on success, false on failure.
   */
  virtual bool RadarStayAlive() { return true; };

  /*
   * Set the range to the given range in meters.
   *
   * @param     meters  Requested range in meters.
   * @returns   true on success, false on failure.
   */
  virtual bool SetRange(int meters) = 0;

  /*
   * Modify a radar setting.
   *
   * @param     controlType     Control such as CT_GAIN, etc.
   * @param     item            Requested value and state.
   * @param     button          Button that this originates from.
   * @returns   true on success, false on failure.
   */
  virtual bool SetControlValue(ControlType controlType, RadarControlItem &item, RadarControlButton *button) = 0;
};

PLUGIN_END_NAMESPACE

#endif /* _RADARCONTROL_H_ */
