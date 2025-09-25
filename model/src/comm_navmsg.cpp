/**************************************************************************
 *   Copyright (C) 2022  David Register                                    *
 *   Copyright (C) 2022 - 2024  Alec Leamas                                *
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

/**
 * \file
 * Implement comm_navmsg.h
 */

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <algorithm>
#include <string>
#include <iomanip>

#include "model/comm_driver.h"
#include "model/ocpn_utils.h"

std::string NavAddr::BusToString(NavAddr::Bus b) {
  switch (b) {
    case NavAddr::Bus::N0183:
      return "nmea0183";
      break;
    case NavAddr::Bus::N2000:
      return "nmea2000";
      break;
    case NavAddr::Bus::Signalk:
      return "SignalK";
      break;
    case NavAddr::Bus::Onenet:
      return "Onenet";
      break;
    case NavAddr::Bus::Plugin:
      return "Plugin";
      break;
    case NavAddr::Bus::TestBus:
      return "TestBus";
      break;
    case NavAddr::Bus::AppMsg:
      return "AppMsg";
      break;
    case NavAddr::Bus::Undef:
      return "??";
      break;
  }
  return "????";
}

NavAddr::Bus NavAddr::StringToBus(const std::string& s) {
  if (s == "nmea0183") return NavAddr::Bus::N0183;
  if (s == "nmea2000") return NavAddr::Bus::N2000;
  if (s == "SignalK") return NavAddr::Bus::Signalk;
  if (s == "Onenet") return NavAddr::Bus::Onenet;
  if (s == "Plugin") return NavAddr::Bus::Plugin;
  if (s == "TestBus") return NavAddr::Bus::TestBus;
  if (s == "AppMsg") return NavAddr::Bus::AppMsg;
  return NavAddr::Bus::Undef;
}

static std::string CharToString(unsigned char c) {
  using namespace std;
  stringstream ss;
  ss << setfill('0') << hex << setw(2) << (c & 0x00ff);
  return ss.str();
}

static std::string CharToString(char c) {
  return CharToString(static_cast<unsigned char>(c));
}

std::string Nmea2000Msg::to_string() const {
  // Arrange to "pretty-print" the N2K message payload

  // extract PGN from payload
  uint64_t pgn = 0;
  unsigned char* c = (unsigned char*)&pgn;
  *c++ = payload.at(3);
  *c++ = payload.at(4);
  *c++ = payload.at(5);

  std::string st;
  size_t data_start = 12;
  if (payload.at(0) == 0x94) data_start = 7;
  size_t i = 0;
  while (i < payload.size() - 1) {
    if (i > data_start) {
      for (int j = 0; j < 8; j++) {
        st.append(CharToString(payload[i]));
        st.append(" ");
        i++;
        if (i >= payload.size() - 1) {
          break;
        }
      }
      st.append(" ");
    } else
      i++;
  }

  std::stringstream ss1;
  std::string spgn = "    ";
  if (PGN.pgn == 1) spgn = "ALL ";
  ss1 << "n2000-" << spgn << "PGN: " << pgn << " [ " << st << " ]";
  return ss1.str();
}

std::string Nmea2000Msg::to_vdr() const {
  std::string s;
  for (auto c : payload) s.append(CharToString(c) + " ");
  return s;
}

std::string Nmea0183Msg::to_string() const {
  std::stringstream ss;
  ss << key() << " " << talker << type << " " << ocpn::printable(payload);
  return ss.str();
}

std::string Nmea0183Msg::to_vdr() const { return ocpn::printable(payload); }

std::string PluginMsg::to_string() const {
  /** Avoid messing with utf string. */
  if (name == "position-fix") return name + ":" + ocpn::rtrim(message);

  std::stringstream ss;
  for (char c : ocpn::rtrim(message)) {
    if (c >= ' ' && c <= '~')
      ss << c;
    else
      ss << CharToString(c);
  }
  return name + ": " + ss.str();
}

NavAddr::Bus NavMsg::GetBusByKey(const std::string& key) {
  if (key.find("::") == std::string::npos) return NavAddr::Bus::Undef;
  auto key_parts = ocpn::split(key, "::");
  return NavAddr::StringToBus(key_parts[0]);
}
