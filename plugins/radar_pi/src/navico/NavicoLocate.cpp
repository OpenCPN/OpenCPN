/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Hakan Svensson
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

#include "NavicoLocate.h"

PLUGIN_BEGIN_NAMESPACE

//
// Radars at and above 4G report their addresses here, including the version #.
//
static const NetworkAddress reportNavicoCommon(236, 6, 7, 5, 6878);

#define SECONDS_PER_SELECT (1)
#define PERIOD_UNTIL_CARD_REFRESH (60)
#define PERIOD_UNTIL_WAKE_RADAR (30)

void NavicoLocate::CleanupCards() {
  if (m_interface_addr) {
    delete[] m_interface_addr;
    m_interface_addr = 0;
  }
  if (m_socket) {
    for (size_t i = 0; i < m_interface_count; i++) {
      if (m_socket[i] != INVALID_SOCKET) {
        closesocket(m_socket[i]);
      }
    }
    delete[] m_socket;
    m_socket = 0;
  }
  m_interface_count = 0;
}

void NavicoLocate::UpdateEthernetCards() {
  struct ifaddrs *addr_list;
  struct ifaddrs *addr;
  size_t i = 0;
  wxString error;

  CleanupCards();

  if (!getifaddrs(&addr_list)) {
    // Count the # of active IPv4 cards
    for (addr = addr_list; addr; addr = addr->ifa_next) {
      if (VALID_IPV4_ADDRESS(addr)) {
        m_interface_count++;
      }
    }

    // If there are any fill packed array (m_socket, m_interface_addr) with them.
    if (m_interface_count > 0) {
      m_socket = new SOCKET[m_interface_count];
      m_interface_addr = new NetworkAddress[m_interface_count];

      for (addr = addr_list; addr; addr = addr->ifa_next) {
        if (VALID_IPV4_ADDRESS(addr)) {
          struct sockaddr_in *sa = (struct sockaddr_in *)addr->ifa_addr;
          m_interface_addr[i].addr = sa->sin_addr;
          m_interface_addr[i].port = 0;
          m_socket[i] = startUDPMulticastReceiveSocket(m_interface_addr[i], reportNavicoCommon, error);
          LOG_VERBOSE(wxT("radar_pi: NavicoLocate scanning interface %s for radars"), m_interface_addr[i].FormatNetworkAddress());
          i++;
        }
      }
    }

    freeifaddrs(addr_list);
  }

  WakeRadar();
}

/*
 * Entry
 *
 * Called by wxThread when the new thread is running.
 * It should remain running until Shutdown is called.
 */
void *NavicoLocate::Entry(void) {
  int r = 0;
  int rescan_network_cards = 0;
  int wake_timeout = 0;
  union {
    sockaddr_storage addr;
    sockaddr_in ipv4;
  } rx_addr;
  socklen_t rx_len;

  uint8_t data[1500];

  LOG_VERBOSE(wxT("radar_pi: NavicoLocate thread starting"));

  m_is_shutdown = false;

  UpdateEthernetCards();

  while (!m_shutdown) {
    struct timeval tv = { (long)1, (long)(0) };
    fd_set fdin;
    FD_ZERO(&fdin);

    int maxFd = INVALID_SOCKET;
    for (size_t i = 0; i < m_interface_count; i++) {
      if (m_socket[i] != INVALID_SOCKET) {
        FD_SET(m_socket[i], &fdin);
        maxFd = MAX(m_socket[i], maxFd);
      }
    }

    r = select(maxFd + 1, &fdin, 0, 0, &tv);
    if (r == -1 && errno != 0) {
      int err = errno;
      UpdateEthernetCards();
      rescan_network_cards = 0;
    }
    if (r > 0) {
      for (size_t i = 0; i < m_interface_count; i++) {
        if (m_socket[i] != INVALID_SOCKET && FD_ISSET(m_socket[i], &fdin)) {
          rx_len = sizeof(rx_addr);
          r = recvfrom(m_socket[i], (char *)data, sizeof(data), 0, (struct sockaddr *)&rx_addr, &rx_len);
          if (r > 2) {   // we are not interested in 2 byte messages
            NetworkAddress radar_address;
            radar_address.addr = rx_addr.ipv4.sin_addr;
            radar_address.port = rx_addr.ipv4.sin_port;

            if (ProcessReport(radar_address, m_interface_addr[i], data, (size_t)r)) {
              rescan_network_cards = -PERIOD_UNTIL_CARD_REFRESH;  // Give double time until we rescan
              wake_timeout = -PERIOD_UNTIL_WAKE_RADAR;
            }
          }
        }
      }
    }
    else {  // no data received -> select timeout
      if (++rescan_network_cards >= PERIOD_UNTIL_CARD_REFRESH) {
        UpdateEthernetCards();
        rescan_network_cards = 0;
        wake_timeout = PERIOD_UNTIL_WAKE_RADAR - 2;  // Wake radar soon, but not immediately
      }

      if (++wake_timeout >= PERIOD_UNTIL_WAKE_RADAR) {
        WakeRadar();
        wake_timeout = 0;
      }
    }

  }  // endless loop until thread destroy

  CleanupCards();

  LOG_VERBOSE(wxT("radar_pi: NavicoLocate thread stopping"));
  m_is_shutdown = true;
  return 0;
}

/*
 RADAR REPORTS

 The radars send various reports. The first 2 bytes indicate what the report type is.
 The types seen on a BR24 are:

 2nd byte C4:   01 02 03 04 05 07 08
 2nd byte F5:   08 0C 0D 0F 10 11 12 13 14

 Not definitive list for
 4G radars only send the C4 data.

 */

 //
 // The following is the received radar state. It sends this regularly
 // but especially after something sends it a state change.
 //
#pragma pack(push, 1)

/*

 BR24:   N/A
 3G:     N/A
             Serial______________            Addr____Port                        Addr____Port        Addr____Port Addr____Port
Addr____Port                    Addr____Port        Addr____Port        Addr____Port                    Addr____Port Addr____Port
Addr____Port                    Addr____Port        Addr____Port        Addr____Port                    Addr____Port Addr____Port
Addr____Port 4G:
01B2313430333330323030300000000000000A0043D901010600FDFF20010200100000000A0043D9176011000000EC0607161A261F002001020010000000EC0607171A1C11000000EC0607181A1D10002001030010000000EC0607081A1611000000EC06070A1A1812000000EC0607091A1710002002030010000000EC06070D1A0111000000EC06070E1A0212000000EC06070F1A0312002001030010000000EC0607121A2011000000EC0607141A2212000000EC0607131A2112002002030010000000EC06070C1A0411000000EC06070D1A0512000000EC06070E1A06
 HALO:
01B231353039303332303030000000000000C0A800F014300600FDFF2001020010000000EC06076117F111000000EC0607161A261F002001020010000000EC06076217F211000000EC06076317F310002001030010000000EC06076417F411000000EC06076517F512000000EC06076617F610002002030010000000EC06076717F711000000EC06076817F812000000EC06076917F912002001030010000000EC06076A17FA11000000EC06076B17FB12000000EC06076C17FC12002002030010000000EC06076D17FD11000000EC06076E17FE12000000EC06076F17FF
 HALO24:
01B2313930323530313030300000000000000A0043C620310600FDFF2001020010000000EC060820197011000000EC0607161A261F002001020010000000EC060821197111000000EC060822197210002001030010000000EC060823197311000000EC060824197412000000EC060823197510002002030010000000EC060825197611000000EC060826197712000000EC060825197812002001030010000000EC060823197911000000EC060827197A12000000EC060823197B12002002030010000000EC060825197C11000000EC060828197D12000000EC060825197E

0A 00 43 D9 01 01 06 00 FD FF 20 01 02 00 10 00 00 00
0A 00 43 D9 17 60 11 00 00 00
EC 06 07 16 1A 26 1F 00 20 01 02 00 10 00 00 00
EC 06 07 17 1A 1C 11 00 00 00
EC 06 07 18 1A 1D 10 00 20 01 03 00 10 00 00 00
EC 06 07 08 1A 16 11 00 00 00
EC 06 07 0A 1A 18 12 00 00 00
EC 06 07 09 1A 17 10 00 20 02 03 00 10 00 00 00
EC 06 07 0D 1A 01 11 00 00 00
EC 06 07 0E 1A 02 12 00 00 00
EC 06 07 0F 1A 03 12 00 20 01 03 00 10 00 00 00
EC 06 07 12 1A 20 11 00 00 00
EC 06 07 14 1A 22 12 00 00 00
EC 06 07 13 1A 21 12 00 20 02 03 00 10 00 00 00
EC 06 07 0C 1A 04 11 00 00 00
EC 06 07 0D 1A 05 12 00 00 00
EC 06 07 0E 1A 06
*/

struct RadarReport_01B2 {
  uint16_t id;
  char serialno[16];          // ASCII serial number, zero terminated
  PackedAddress addr0;        // 0A 00 43 D9 01 01
  uint8_t u1[12];             // 11000000
  PackedAddress addr1;        // EC0608201970
  uint8_t u2[4];              // 11000000
  PackedAddress addr2;        // EC0607161A26
  uint8_t u3[10];             // 1F002001020010000000
  PackedAddress addr3;        // EC0608211971
  uint8_t u4[4];              // 11000000
  PackedAddress addr4;        // EC0608221972
  uint8_t u5[10];             // 10002001030010000000
  PackedAddress addrDataA;    // EC0608231973
  uint8_t u6[4];              // 11000000
  PackedAddress addrSendA;    // EC0608241974
  uint8_t u7[4];              // 12000000
  PackedAddress addrReportA;  // EC0608231975
  uint8_t u8[10];             // 10002002030010000000
  PackedAddress addrDataB;    // EC0608251976
  uint8_t u9[4];              // 11000000
  PackedAddress addrSendB;    // EC0608261977
  uint8_t u10[4];             // 12000000
  PackedAddress addrReportB;  // EC0608251978
  uint8_t u11[10];            // 12002001030010000000
  PackedAddress addr11;       // EC0608231979
  uint8_t u12[4];             // 11000000
  PackedAddress addr12;       // EC060827197A
  uint8_t u13[4];             // 12000000
  PackedAddress addr13;       // EC060823197B
  uint8_t u14[10];            // 12002002030010000000
  PackedAddress addr14;       // EC060825197C
  uint8_t u15[4];             // 11000000
  PackedAddress addr15;       // EC060828197D
  uint8_t u16[4];             // 12000000
  PackedAddress addr16;       // EC060825197E
};

#pragma pack(pop)

bool NavicoLocate::ProcessReport(const NetworkAddress &radar_address, const NetworkAddress &interface_address, const uint8_t *report, size_t len) {
  if (report[0] == 01 && report[1] == 0xB1) {  // Wake radar
    LOG_VERBOSE(wxT("radar_pi: Wake radar request from %s"), radar_address.FormatNetworkAddress());
  }
  if (report[0] == 01 && report[1] == 0xB2) {  // Common Navico message from 4G++
    if (m_pi->m_settings.verbose >= 2) {
      LOG_BINARY_RECEIVE(wxT("radar_pi: NavicoLocate received RadarReport_01B2"), report, len);
    }
    RadarReport_01B2 *data = (RadarReport_01B2 *)report;
    wxCriticalSectionLocker lock(m_exclusive);

    NavicoRadarInfo infoA;
    infoA.serialNr = wxString::FromAscii(data->serialno);
    infoA.spoke_data_addr = NetworkAddress(data->addrDataA);
    infoA.report_addr = NetworkAddress(data->addrReportA);
    infoA.send_command_addr = NetworkAddress(data->addrSendA);
    NetworkAddress radar_ipA = radar_address;
    radar_ipA.port = htons(RO_PRIMARY);
    if (m_report_count < MAX_REPORT) {
      LOG_INFO(wxT("radar_pi: Located radar IP %s, interface %s [%s]"), radar_ipA.FormatNetworkAddressPort(), interface_address.FormatNetworkAddress(), infoA.to_string());
      m_report_count++;
    }
    else {
      LOG_RECEIVE(wxT("radar_pi: Located radar IP %s, interface %s [%s]"), radar_ipA.FormatNetworkAddressPort(), interface_address.FormatNetworkAddress(), infoA.to_string());
    }
    m_pi->FoundNavicoRadarInfo(radar_ipA, interface_address, infoA);

    if (len > 150) {  // for 3G radar len == 150, no B available
      NavicoRadarInfo infoB;
      infoB.serialNr = wxString::FromAscii(data->serialno);
      infoB.spoke_data_addr = NetworkAddress(data->addrDataB);
      infoB.report_addr = NetworkAddress(data->addrReportB);
      infoB.send_command_addr = NetworkAddress(data->addrSendB);
      NetworkAddress radar_ipB = radar_address;
      radar_ipB.port = htons(RO_SECONDARY);
      if (m_report_count < MAX_REPORT) {
        LOG_INFO(wxT("radar_pi: Located radar IP %s, interface %s [%s]"), radar_ipB.FormatNetworkAddressPort(), interface_address.FormatNetworkAddress(), infoB.to_string());
      } else {
        LOG_RECEIVE(wxT("radar_pi: Located radar IP %s, interface %s [%s]"), radar_ipA.FormatNetworkAddressPort(), interface_address.FormatNetworkAddress(), infoA.to_string());
        m_report_count++;
      }
      m_pi->FoundNavicoRadarInfo(radar_ipB, interface_address, infoB);
    }
#define LOG_ADDR_N(n)                                                                                  \
  LOG_RECEIVE(wxT("radar_pi: NavicoLocate %s addr %s = %s"), radar_address.FormatNetworkAddress(), #n, \
              FormatPackedAddress(data->addr##n));

    IF_LOG_AT_LEVEL(LOGLEVEL_RECEIVE) {
      LOG_ADDR_N(0);
      LOG_ADDR_N(1);
      LOG_ADDR_N(2);
      LOG_ADDR_N(3);
      LOG_ADDR_N(4);
      LOG_ADDR_N(DataA);
      LOG_ADDR_N(SendA);
      LOG_ADDR_N(ReportA);
      LOG_ADDR_N(DataB);
      LOG_ADDR_N(SendB);
      LOG_ADDR_N(ReportB);
      LOG_ADDR_N(11);
      LOG_ADDR_N(12);
      LOG_ADDR_N(13);
      LOG_ADDR_N(14);
      LOG_ADDR_N(15);
      LOG_ADDR_N(16);
    }
    return true;
  }

  LOG_BINARY_RECEIVE(wxT("radar_pi: NavicoLocate received unknown message"), report, len);
  return false;
}

void NavicoLocate::WakeRadar() {
  static const uint8_t WAKE_COMMAND[] = { 0x01, 0xb1 };
  struct sockaddr_in send_addr = NetworkAddress(236, 6, 7, 5, 6878).GetSockAddrIn();

  int one = 1;

  for (size_t i = 0; i < m_interface_count; i++) {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in s = m_interface_addr[i].GetSockAddrIn();

    if (sock != INVALID_SOCKET) {
      if (!setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one)) &&
        !::bind(sock, (struct sockaddr *)&s, sizeof(s)) &&
        sendto(sock, (const char *)WAKE_COMMAND, sizeof WAKE_COMMAND, 0, (struct sockaddr *)&send_addr, sizeof(send_addr)) ==
        sizeof WAKE_COMMAND) {
        LOG_VERBOSE(wxT("radar_pi: Sent wake command to radar on %s"), m_interface_addr[i].FormatNetworkAddress());
      }
      else {
        wxLogError(wxT("radar_pi: Failed to send wake command to radars on %s"), m_interface_addr[i].FormatNetworkAddress());
      }
      closesocket(sock);
    }
  }
}

PLUGIN_END_NAMESPACE
