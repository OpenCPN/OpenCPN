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

#include "NavicoControl.h"

PLUGIN_BEGIN_NAMESPACE

static const uint8_t COMMAND_TX_OFF_A[3] = {0x00, 0xc1, 0x01};  // OFF part 1, note same as TX ON part 1
static const uint8_t COMMAND_TX_OFF_B[3] = {0x01, 0xc1, 0x00};  // OFF part 1, note same as TX ON part 1

static const uint8_t COMMAND_TX_ON_A[3] = {0x00, 0xc1, 0x01};  // ON part 1
static const uint8_t COMMAND_TX_ON_B[3] = {0x01, 0xc1, 0x01};  // ON part 2

static const uint8_t COMMAND_STAY_ON_A[2] = {0xA0, 0xc1};
static const uint8_t COMMAND_STAY_ON_B[2] = {0x03, 0xc2};
static const uint8_t COMMAND_STAY_ON_C[2] = {0x04, 0xc2};
static const uint8_t COMMAND_STAY_ON_D[2] = {0x05, 0xc2};

NavicoControl::NavicoControl() {
  m_radar_socket = INVALID_SOCKET;
  m_name = wxT("Navico radar");
}

NavicoControl::~NavicoControl() {
  if (m_radar_socket != INVALID_SOCKET) {
    closesocket(m_radar_socket);
    LOG_TRANSMIT(wxT("radar_pi: %s transmit socket closed"), m_name.c_str());
  }
}

void NavicoControl::SetMultiCastAddress(NetworkAddress sendMultiCastAddress) { m_addr = sendMultiCastAddress.GetSockAddrIn(); }

bool NavicoControl::Init(radar_pi *pi, RadarInfo *ri, NetworkAddress &ifadr, NetworkAddress &radaradr) {
  int r;
  int one = 1;

  // The radar IP address is not used for Navico BR/Halo radars
  if (radaradr.port != 0) {
    // Null
  }

  m_pi = pi;
  m_ri = ri;
  m_name = ri->m_name;

  if (m_radar_socket != INVALID_SOCKET) {
    closesocket(m_radar_socket);
  }
  m_radar_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_radar_socket == INVALID_SOCKET) {
    r = -1;
  } else {
    r = setsockopt(m_radar_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
  }

  if (!r) {
    struct sockaddr_in s = ifadr.GetSockAddrIn();

    r = ::bind(m_radar_socket, (struct sockaddr *)&s, sizeof(s));
  }

  if (r) {
    wxLogError(wxT("radar_pi: Unable to create UDP sending socket"));
    // Might as well give up now
    return false;
  }

  LOG_TRANSMIT(wxT("radar_pi: %s transmit socket open"), m_name.c_str());
  return true;
}

void NavicoControl::logBinaryData(const wxString &what, const uint8_t *data, int size) {
  wxString explain;
  int i = 0;

  explain.Alloc(size * 3 + 50);
  explain += wxT("radar_pi: ") + m_name.c_str() + wxT(" ");
  explain += what;
  explain += wxString::Format(wxT(" %d bytes: "), size);
  for (i = 0; i < size; i++) {
    explain += wxString::Format(wxT(" %02X"), data[i]);
  }
  LOG_TRANSMIT(explain);
}

bool NavicoControl::TransmitCmd(const uint8_t *msg, int size) {
  if (m_radar_socket == INVALID_SOCKET) {
    wxLogError(wxT("radar_pi: Unable to transmit command to unknown radar"));
    return false;
  }
  if (sendto(m_radar_socket, (char *)msg, size, 0, (struct sockaddr *)&m_addr, sizeof(m_addr)) < size) {
    wxLogError(wxT("radar_pi: Unable to transmit command to %s: %s"), m_name.c_str(), SOCKETERRSTR);
    return false;
  }
  IF_LOG_AT(LOGLEVEL_TRANSMIT, logBinaryData(wxT("transmit"), msg, size));
  return true;
}

void NavicoControl::RadarTxOff() {
  IF_LOG_AT(LOGLEVEL_VERBOSE | LOGLEVEL_TRANSMIT, wxLogMessage(wxT("radar_pi: %s transmit: turn Off"), m_name.c_str()));
  TransmitCmd(COMMAND_TX_OFF_A, sizeof(COMMAND_TX_OFF_A));
  TransmitCmd(COMMAND_TX_OFF_B, sizeof(COMMAND_TX_OFF_B));
}

void NavicoControl::RadarTxOn() {
  IF_LOG_AT(LOGLEVEL_VERBOSE | LOGLEVEL_TRANSMIT, wxLogMessage(wxT("radar_pi: %s transmit: turn on"), m_name.c_str()));
  TransmitCmd(COMMAND_TX_ON_A, sizeof(COMMAND_TX_ON_A));
  TransmitCmd(COMMAND_TX_ON_B, sizeof(COMMAND_TX_ON_B));
}

bool NavicoControl::RadarStayAlive() {
  LOG_TRANSMIT(wxT("radar_pi: %s transmit: stay alive"), m_name.c_str());

  TransmitCmd(COMMAND_STAY_ON_A, sizeof(COMMAND_STAY_ON_A));
  TransmitCmd(COMMAND_STAY_ON_B, sizeof(COMMAND_STAY_ON_B));
  TransmitCmd(COMMAND_STAY_ON_C, sizeof(COMMAND_STAY_ON_C));
  return TransmitCmd(COMMAND_STAY_ON_D, sizeof(COMMAND_STAY_ON_D));
}

bool NavicoControl::SetRange(int meters) {
  if (meters >= 50 && meters <= 72704 && m_radar_socket != INVALID_SOCKET) {
    unsigned int decimeters = (unsigned int)meters * 10;
    uint8_t pck[] = {0x03,
                     0xc1,
                     (uint8_t)((decimeters >> 0) & 0XFFL),
                     (uint8_t)((decimeters >> 8) & 0XFFL),
                     (uint8_t)((decimeters >> 16) & 0XFFL),
                     (uint8_t)((decimeters >> 24) & 0XFFL)};
    LOG_VERBOSE(wxT("radar_pi: %s transmit: range %d meters"), m_name.c_str(), meters);
    return TransmitCmd(pck, sizeof(pck));
  }
  return false;
}

bool NavicoControl::SetControlValue(ControlType controlType, RadarControlItem &item, RadarControlButton *button) {
  bool r = false;

  int value = item.GetValue();
  RadarControlState state = item.GetState();
  int autoValue = 0;
  if (state > RCS_MANUAL) {
    autoValue = state - RCS_MANUAL;
  }

  switch (controlType) {
    case CT_NONE:
    case CT_RANGE:
    case CT_TIMED_IDLE:
    case CT_TIMED_RUN:
    case CT_TRANSPARENCY:
    case CT_REFRESHRATE:
    case CT_TARGET_ON_PPI:
    case CT_TARGET_TRAILS:
    case CT_TRAILS_MOTION:
    case CT_MAIN_BANG_SIZE:
    case CT_MAX:
    case CT_ORIENTATION:
    case CT_CENTER_VIEW:
    case CT_OVERLAY_CANVAS:
    case CT_ANTENNA_FORWARD:
    case CT_ANTENNA_STARBOARD:
    case CT_NO_TRANSMIT_START:
    case CT_NO_TRANSMIT_END:
    case CT_FTC:
      // The above are not settings that are not radar commands or not supported by Navico radar.
      // Made them explicit so the compiler can catch missing control types.
      break;

      // Ordering the radar commands by the first byte value.
      // Some interesting holes here, seems there could be more commands!

    case CT_BEARING_ALIGNMENT: {  // to be consistent with the local bearing alignment of the pi
                                  // this bearing alignment works opposite to the one an a Lowrance display
      if (value < 0) {
        value += 360;
      }
      int v = value * 10;
      int v1 = v / 256;
      int v2 = v & 255;
      uint8_t cmd[4] = {0x05, 0xc1, (uint8_t)v2, (uint8_t)v1};
      LOG_VERBOSE(wxT("radar_pi: %s Bearing alignment: %d"), m_name.c_str(), v);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_GAIN: {
      int v = (value + 1) * 255 / 100;
      if (v > 255) {
        v = 255;
      }
      uint8_t cmd[] = {0x06, 0xc1, 0, 0, 0, 0, (uint8_t)autoValue, 0, 0, 0, (uint8_t)v};
      LOG_VERBOSE(wxT("radar_pi: %s Gain: %d auto %d"), m_name.c_str(), value, autoValue);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_SEA: {
      int v = (value + 1) * 255 / 100;
      if (v > 255) {
        v = 255;
      }
      uint8_t cmd[] = {0x06, 0xc1, 0x02, 0, 0, 0, (uint8_t)autoValue, 0, 0, 0, (uint8_t)v};
      LOG_VERBOSE(wxT("radar_pi: %s Sea: %d auto %d"), m_name.c_str(), value, autoValue);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_RAIN: {  // Rain Clutter - Manual. Range is 0x01 to 0x50
      int v = (value + 1) * 255 / 100;
      if (v > 255) {
        v = 255;
      }
      uint8_t cmd[] = {0x06, 0xc1, 0x04, 0, 0, 0, 0, 0, 0, 0, (uint8_t)v};
      LOG_VERBOSE(wxT("radar_pi: %s Rain: %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_SIDE_LOBE_SUPPRESSION: {
      int v = value * 256 / 100;
      if (v > 255) {
        v = 255;
      }
      uint8_t cmd[] = {0x6, 0xc1, 0x05, 0, 0, 0, (uint8_t)autoValue, 0, 0, 0, (uint8_t)v};
      LOG_VERBOSE(wxT("radar_pi: %s command Tx CT_SIDE_LOBE_SUPPRESSION: %d auto %d"), m_name.c_str(), value, autoValue);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

      // What would command 7 be?

    case CT_INTERFERENCE_REJECTION: {
      uint8_t cmd[] = {0x08, 0xc1, (uint8_t)value};
      LOG_VERBOSE(wxT("radar_pi: %s Rejection: %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_TARGET_EXPANSION: {
      uint8_t cmd[] = {0x09, 0xc1, (uint8_t)value};
      LOG_VERBOSE(wxT("radar_pi: %s Target expansion: %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_TARGET_BOOST: {
      uint8_t cmd[] = {0x0a, 0xc1, (uint8_t)value};
      LOG_VERBOSE(wxT("radar_pi: %s Target boost: %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

      // What would command b through d be?

    case CT_LOCAL_INTERFERENCE_REJECTION: {
      if (value < 0) value = 0;
      if (value > 3) value = 3;
      uint8_t cmd[] = {0x0e, 0xc1, (uint8_t)value};
      LOG_VERBOSE(wxT("radar_pi: %s local interference rejection %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_SCAN_SPEED: {
      uint8_t cmd[] = {0x0f, 0xc1, (uint8_t)value};
      LOG_VERBOSE(wxT("radar_pi: %s Scan speed: %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

      // What would command 10 through 20 be?

    case CT_NOISE_REJECTION: {
      uint8_t cmd[] = {0x21, 0xc1, (uint8_t)value};
      LOG_VERBOSE(wxT("radar_pi: %s Noise rejection: %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_TARGET_SEPARATION: {
      uint8_t cmd[] = {0x22, 0xc1, (uint8_t)value};
      LOG_VERBOSE(wxT("radar_pi: %s Target separation: %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

    case CT_DOPPLER: {
      uint8_t cmd[] = {0x23, 0xc1, (uint8_t)value};
      LOG_VERBOSE(wxT("radar_pi: %s Doppler state: %d"), m_name.c_str(), value);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }

      // What would command 24 through 2f be?

    case CT_ANTENNA_HEIGHT: {
      int v = value * 1000;  // radar wants millimeters, not meters :-)
      int v1 = v / 256;
      int v2 = v & 255;
      uint8_t cmd[10] = {0x30, 0xc1, 0x01, 0, 0, 0, (uint8_t)v2, (uint8_t)v1, 0, 0};
      LOG_VERBOSE(wxT("radar_pi: %s Antenna height: %d"), m_name.c_str(), v);
      r = TransmitCmd(cmd, sizeof(cmd));
      break;
    }
  }

  return r;
}

PLUGIN_END_NAMESPACE
