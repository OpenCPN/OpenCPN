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

#include "GarminHDControl.h"

PLUGIN_BEGIN_NAMESPACE

#pragma pack(push, 1)

typedef struct {
  uint32_t packet_type;
  uint32_t len1;
  uint8_t parm1;
} rad_ctl_pkt_9;

typedef struct {
  uint32_t packet_type;
  uint32_t len1;
  uint16_t parm1;
} rad_ctl_pkt_10;

typedef struct {
  uint32_t packet_type;
  uint32_t len1;
  uint32_t parm1;
} rad_ctl_pkt_12;

typedef struct {
  uint32_t packet_type;
  uint32_t len1;
  uint8_t parm1;
  uint8_t parm2;
  uint8_t parm3;
  uint8_t parm4;
} rad_ctl_pkt_12_4b;

#pragma pack(pop)

GarminHDControl::GarminHDControl(NetworkAddress sendAddress) {
  m_addr = sendAddress.GetSockAddrIn();  // Overwritten by actual radar addr

  m_radar_socket = INVALID_SOCKET;
  m_name = wxT("Navico radar");

  m_pi = 0;
  m_ri = 0;
  m_name = wxT("GarminHD");
}

GarminHDControl::~GarminHDControl() {
  if (m_radar_socket != INVALID_SOCKET) {
    closesocket(m_radar_socket);
    LOG_TRANSMIT(wxT("radar_pi: %s transmit socket closed"), m_name.c_str());
  }
}

bool GarminHDControl::Init(radar_pi *pi, RadarInfo *ri, NetworkAddress &ifadr, NetworkAddress &radaradr) {
  int r;
  int one = 1;

  m_addr.sin_addr = radaradr.addr;

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

  LOG_TRANSMIT(wxT("radar_pi: %s transmit socket open"), m_name);
  return true;
}

void GarminHDControl::logBinaryData(const wxString &what, const void *data, int size) {
  wxString explain;
  const uint8_t *d = (const uint8_t *)data;
  int i = 0;

  explain.Alloc(size * 3 + 50);
  explain += wxT("radar_pi: ") + m_name + wxT(" ");
  explain += what;
  explain += wxString::Format(wxT(" %d bytes: "), size);
  for (i = 0; i < size; i++) {
    explain += wxString::Format(wxT(" %02X"), d[i]);
  }
  LOG_TRANSMIT(explain);
}

bool GarminHDControl::TransmitCmd(const void *msg, int size) {
  if (m_radar_socket == INVALID_SOCKET) {
    wxLogError(wxT("radar_pi: Unable to transmit command to unknown radar"));
    return false;
  }
  if (sendto(m_radar_socket, (char *)msg, size, 0, (struct sockaddr *)&m_addr, sizeof(m_addr)) < size) {
    wxLogError(wxT("radar_pi: Unable to transmit command to %s: %s"), m_name.c_str(), SOCKETERRSTR);
    return false;
  }
  IF_LOG_AT(LOGLEVEL_TRANSMIT, logBinaryData(wxString::Format(wxT("%s transmit"), m_name), msg, size));
  return true;
}

void GarminHDControl::RadarTxOff() {
  IF_LOG_AT(LOGLEVEL_VERBOSE | LOGLEVEL_TRANSMIT, wxLogMessage(wxT("radar_pi: %s transmit: turn off"), m_name));

  rad_ctl_pkt_10 packet;
  packet.packet_type = 0x2b2;
  packet.len1 = sizeof(packet.parm1);
  packet.parm1 = 1;  // 1 for "off"

  TransmitCmd(&packet, sizeof(packet));
}

void GarminHDControl::RadarTxOn() {
  IF_LOG_AT(LOGLEVEL_VERBOSE | LOGLEVEL_TRANSMIT, wxLogMessage(wxT("radar_pi: %s transmit: turn on"), m_name));

  rad_ctl_pkt_10 packet;
  packet.packet_type = 0x2b2;
  packet.len1 = sizeof(packet.parm1);
  packet.parm1 = 2;  // 2 for "on"

  TransmitCmd(&packet, sizeof(packet));
}

bool GarminHDControl::RadarStayAlive() {
  // Garmin radars don't need a ping
  return true;
}

bool GarminHDControl::SetRange(int meters) {
  if (meters >= 200 && meters <= 48 * 1852) {
    rad_ctl_pkt_12 packet;

    packet.packet_type = 0x2b3;
    packet.len1 = sizeof(packet.parm1);
    packet.parm1 = meters-1;
    LOG_VERBOSE(wxT("radar_pi: %s transmit: range %d meters"), m_name.c_str(), meters-1);
    return TransmitCmd(&packet, sizeof(packet));
  }
  return false;
}

bool GarminHDControl::SetControlValue(ControlType controlType, RadarControlItem &item, RadarControlButton *button) {
  bool r = false;
  int value = item.GetValue();
  RadarControlState state = item.GetState();

  rad_ctl_pkt_9 pck_9;
  rad_ctl_pkt_10 pck_10;
  rad_ctl_pkt_12 pck_12;
  rad_ctl_pkt_12_4b pck_12_4b;

  pck_9.len1 = sizeof(pck_9.parm1);
  pck_10.len1 = sizeof(pck_10.parm1);
  pck_12.len1 = sizeof(pck_12.parm1);
  pck_12_4b.len1 = sizeof(pck_12_4b.parm1);

  switch (controlType) {
    // The following are settings that are not radar commands. Made them explicit so the
    // compiler can catch missing control types.
    case CT_NONE:
    case CT_RANGE:
    case CT_TIMED_IDLE:
    case CT_TIMED_RUN:
    case CT_TRANSPARENCY:
    case CT_REFRESHRATE:
    case CT_TARGET_TRAILS:
    case CT_TRAILS_MOTION:
    case CT_MAIN_BANG_SIZE:
    case CT_MAX:
    case CT_ANTENNA_FORWARD:
    case CT_ANTENNA_STARBOARD:
    case CT_ORIENTATION:
    case CT_CENTER_VIEW:
    case CT_OVERLAY_CANVAS:
    case CT_TARGET_ON_PPI:

    // The following are settings not supported by Garmin HD.
    case CT_SIDE_LOBE_SUPPRESSION:
    case CT_TARGET_EXPANSION:
    case CT_TARGET_BOOST:
    case CT_LOCAL_INTERFERENCE_REJECTION:
    case CT_NOISE_REJECTION:
    case CT_TARGET_SEPARATION:
    case CT_DOPPLER:
    case CT_ANTENNA_HEIGHT:
    case CT_NO_TRANSMIT_END:
    case CT_NO_TRANSMIT_START:

      break;

      // Ordering the radar commands by the first byte value.
      // Some interesting holes here, seems there could be more commands!

    case CT_BEARING_ALIGNMENT: {
      if (value < 0) {
        value += 360;
      }

      pck_10.packet_type = 0x2b7;
      pck_10.parm1 = value;

      LOG_VERBOSE(wxT("radar_pi: %s Bearing alignment: %d"), m_name.c_str(), value);
      r = TransmitCmd(&pck_10, sizeof(pck_10));
      break;
    }

      /*    case CT_NO_TRANSMIT_START: {
            // value is already in range -180 .. +180 which is what I think radar wants...
            if (state == RCS_OFF) {  // OFF
              pck_9.packet_type = 0x93f;
              pck_9.parm1 = 0;
              r = TransmitCmd(&pck_9, sizeof(pck_9));
            } else {
              pck_9.packet_type = 0x93f;
              pck_9.parm1 = 1;
              r = TransmitCmd(&pck_9, sizeof(pck_9));
              pck_12.packet_type = 0x940;
              pck_12.parm1 = value * 32;
              r = TransmitCmd(&pck_12, sizeof(pck_12));
              m_ri->m_no_transmit_start.Update(value);  // necessary because we hacked "off" as auto value
            }
            LOG_VERBOSE(wxT("radar_pi: %s No Transmit Start: value=%d state=%d"), m_name.c_str(), value, (int)state);
            break;
          }

          case CT_NO_TRANSMIT_END: {
            // value is already in range -180 .. +180 which is what I think radar wants...
            if (state == RCS_OFF) {  // OFF
              pck_9.packet_type = 0x93f;
              pck_9.parm1 = 0;
              r = TransmitCmd(&pck_9, sizeof(pck_9));
            } else {
              pck_9.packet_type = 0x93f;
              pck_9.parm1 = 1;
              r = TransmitCmd(&pck_9, sizeof(pck_9));
              pck_12.packet_type = 0x941;
              pck_12.parm1 = value * 32;
              r = TransmitCmd(&pck_12, sizeof(pck_12));
            }
            LOG_VERBOSE(wxT("radar_pi: %s No Transmit End: value=%d state=%d"), m_name.c_str(), value, (int)state);
            break;
          } */

    case CT_GAIN: {
      LOG_VERBOSE(wxT("radar_pi: %s Gain: value=%d state=%d"), m_name.c_str(), value, (int)state);
      if (state >= RCS_AUTO_1) {
        pck_12.packet_type = 0x2b4;
        pck_12.parm1 = 344;
        r = TransmitCmd(&pck_12, sizeof(pck_12));
      } else if (state == RCS_MANUAL) {
        pck_12.packet_type = 0x2b4;
        pck_12.parm1 = value;
        r = TransmitCmd(&pck_12, sizeof(pck_12));
      }
      break;
    }

    case CT_SEA: {
      LOG_VERBOSE(wxT("radar_pi: %s Sea: value=%d state=%d"), m_name.c_str(), value, (int)state);
      pck_12_4b.parm3 = 0;
      pck_12_4b.parm4 = 0;

      if (state == RCS_AUTO_1) {
        pck_12_4b.packet_type = 0x2b5;
        pck_12_4b.parm1 = 33;  // calm
        pck_12_4b.parm2 = 1;   // auto
        r = TransmitCmd(&pck_12, sizeof(pck_12));
      } else if (state == RCS_AUTO_2) {
        pck_12_4b.packet_type = 0x2b5;
        pck_12_4b.parm1 = 67;  // medium
        pck_12_4b.parm2 = 2;   // auto
        r = TransmitCmd(&pck_12, sizeof(pck_12));
      } else if (state == RCS_AUTO_3) {
        pck_12_4b.packet_type = 0x2b5;
        pck_12_4b.parm1 = 100;  // rough
        pck_12_4b.parm2 = 2;    // auto
        r = TransmitCmd(&pck_12, sizeof(pck_12));
      } else if (state == RCS_OFF) {
        pck_12_4b.packet_type = 0x2b5;
        pck_12_4b.parm1 = 0;  // off
        pck_12_4b.parm2 = 0;  // off
        r = TransmitCmd(&pck_12_4b, sizeof(pck_12_4b));
      } else if (state == RCS_MANUAL) {
        pck_12_4b.packet_type = 0x2b5;
        pck_12_4b.parm1 = value;
        pck_12_4b.parm1 = 0;  // manual
        r = TransmitCmd(&pck_12_4b, sizeof(pck_12_4b));
      }
      break;
    }

    case CT_FTC: {
      LOG_VERBOSE(wxT("radar_pi: %s FTC: value=%d"), m_name.c_str(), value);
      pck_9.packet_type = 0x2b8;
      pck_9.parm1 = value;
      r = TransmitCmd(&pck_9, sizeof(pck_9));
      break;
    }

    case CT_RAIN: {  // Rain Clutter
      LOG_VERBOSE(wxT("radar_pi: %s Rain: value=%d state=%d"), m_name.c_str(), value, (int)state);

      if (state == RCS_OFF) {
        pck_12.packet_type = 0x2b6;
        pck_12.parm1 = 0;  // off
        r = TransmitCmd(&pck_12, sizeof(pck_12));
      } else if (state == RCS_MANUAL) {
        pck_12.packet_type = 0x2b6;
        pck_12.parm1 = value;
        r = TransmitCmd(&pck_12, sizeof(pck_12));
      }
      break;
    }

    case CT_INTERFERENCE_REJECTION: {
      LOG_VERBOSE(wxT("radar_pi: %s Interference Rejection / Crosstalk: %d"), m_name.c_str(), value);
      pck_9.packet_type = 0x2b9;
      pck_9.parm1 = value;
      r = TransmitCmd(&pck_9, sizeof(pck_9));
      break;
    }

    case CT_SCAN_SPEED: {
      LOG_VERBOSE(wxT("radar_pi: %s Scan speed: %d"), m_name.c_str(), value);
      pck_9.packet_type = 0x2be;
      pck_9.parm1 = value;
      r = TransmitCmd(&pck_9, sizeof(pck_9));
      break;
    }
  }

  return r;
}

PLUGIN_END_NAMESPACE
