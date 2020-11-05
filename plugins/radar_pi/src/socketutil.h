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
 *   the Free Software Foundation; either verion 2 of the License, or     *
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

#ifndef _SOCKETUTIL_H_
#define _SOCKETUTIL_H_

#include <wx/tokenzr.h>
#include "pi_common.h"

PLUGIN_BEGIN_NAMESPACE

#define VALID_IPV4_ADDRESS(i)                                                                                                    \
  (i && i->ifa_addr && i->ifa_addr->sa_family == AF_INET && (i->ifa_flags & IFF_UP) > 0 && (i->ifa_flags & IFF_LOOPBACK) == 0 && \
   (i->ifa_flags & IFF_MULTICAST) > 0)

// easy define of mcast addresses. Note that these are in network order already.
#define IPV4_ADDR(a, b, c, d) ((uint32_t)(((a)&0xff) << 24) | (((b)&0xff) << 16) | (((c)&0xff) << 8) | ((d)&0xff))

#define IPV4_PORT(p) (htons(p))

#pragma pack(push, 1)
struct PackedAddress {
  struct in_addr addr;
  uint16_t port;
};
#pragma pack(pop)

class NetworkAddress {
 public:
  NetworkAddress() {
    addr.s_addr = 0;
    port = 0;
  }

  NetworkAddress(PackedAddress packed) {
    addr.s_addr = packed.addr.s_addr;
    port = packed.port;
  }

  NetworkAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t p) {
    uint8_t *paddr = (uint8_t *)&addr;

    paddr[0] = a;
    paddr[1] = b;
    paddr[2] = c;
    paddr[3] = d;

    port = htons(p);
  }

  NetworkAddress(const wxString str) {
    uint8_t *paddr = (uint8_t *)&addr;
    wxStringTokenizer tokenizer(str, wxT(".:"));

    addr.s_addr = 0;
    port = 0;

    if (tokenizer.HasMoreTokens()) {
      paddr[0] = wxAtoi(tokenizer.GetNextToken());
    }
    if (tokenizer.HasMoreTokens()) {
      paddr[1] = wxAtoi(tokenizer.GetNextToken());
    }
    if (tokenizer.HasMoreTokens()) {
      paddr[2] = wxAtoi(tokenizer.GetNextToken());
    }
    if (tokenizer.HasMoreTokens()) {
      paddr[3] = wxAtoi(tokenizer.GetNextToken());
    }
    if (tokenizer.HasMoreTokens()) {
      port = htons(wxAtoi(tokenizer.GetNextToken()));
    }
  }

  bool operator<(const NetworkAddress &other) const {
    if (other.addr.s_addr < this->addr.s_addr) {
      return true;
    }

    return other.port < this->port;
  }

  bool operator==(const NetworkAddress &other) const { return other.addr.s_addr == this->addr.s_addr && other.port == this->port; }

  NetworkAddress &operator=(const NetworkAddress &other) {
    if (this != &other) {
      addr.s_addr = other.addr.s_addr;
      port = other.port;
    }

    return *this;
  }

  wxString to_string() const {
    if (addr.s_addr != 0) {
      uint8_t *a = (uint8_t *)&addr;  // sin_addr is in network layout
      return wxString::Format(wxT("%u.%u.%u.%u:%u"), a[0], a[1], a[2], a[3], ntohs(port));
    }
    return wxT("");
  }

  wxString FormatNetworkAddress() const {
    uint8_t *a = (uint8_t *)&addr;  // sin_addr is in network layout
    return wxString::Format(wxT("%u.%u.%u.%u"), a[0], a[1], a[2], a[3]);
  }

  wxString FormatNetworkAddressPort() const {
    uint8_t *a = (uint8_t *)&addr;  // sin_addr is in network layout
    return wxString::Format(wxT("%u.%u.%u.%u port %u"), a[0], a[1], a[2], a[3], ntohs(port));
  }

  struct sockaddr_in GetSockAddrIn() const {
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr = this->addr;
    sin.sin_port = this->port;
#ifdef __WX_MAC__
    sin.sin_len = sizeof(sockaddr_in);
#endif

    return sin;
  }

  bool IsNull() const { return (addr.s_addr == 0); }

  struct in_addr addr;
  uint16_t port;
};

extern wxString FormatPackedAddress(const PackedAddress &addr);

extern bool socketReady(SOCKET sockfd, int timeout);

extern int radar_inet_aton(const char *cp, struct in_addr *addr);
extern SOCKET startUDPMulticastReceiveSocket(const NetworkAddress &addr, const NetworkAddress &mcast_address,
                                             wxString &error_message);
extern SOCKET GetLocalhostServerTCPSocket();
extern SOCKET GetLocalhostSendTCPSocket(SOCKET receive_socket);
extern bool socketAddMembership(SOCKET socket, const NetworkAddress &interface_address, const NetworkAddress &mcast_address);

#ifndef __WXMSW__

// Mac and Linux have ifaddrs.
#include <ifaddrs.h>
#include <net/if.h>

#else

// Emulate (just enough of) ifaddrs on Windows
// Thanks to
// https://code.google.com/p/openpgm/source/browse/trunk/openpgm/pgm/getifaddrs.c?r=496&spec=svn496
// Although that file has interesting new APIs the old ioctl works fine with XP and W7, and does
// enough
// for what we want to do.

struct ifaddrs {
  struct ifaddrs *ifa_next;
  struct sockaddr *ifa_addr;
  struct sockaddr *ifa_netmask;
  ULONG ifa_flags;
};

struct ifaddrs_storage {
  struct ifaddrs ifa;
  struct sockaddr_storage addr;
  struct sockaddr_storage netmask;
};

extern int getifaddrs(struct ifaddrs **ifap);
extern void freeifaddrs(struct ifaddrs *ifa);

#endif

PLUGIN_END_NAMESPACE

#endif
