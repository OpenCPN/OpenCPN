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
  return NavAddr::Bus::Undef;
}

static std::string CharToString(unsigned char c) {
  using namespace std;
  stringstream ss;
  ss << setfill('0') << hex << setw(2) << (c & 0x00ff);
  return ss.str();
}

std::string Nmea2000Msg::to_string() const {
  std::string s;
  std::for_each(payload.begin(), payload.end(),
                [&s](unsigned char c) { s.append(CharToString(c)); });

  return NavMsg::to_string() + " " + PGN.to_string() + " " + s;
}

std::string Nmea0183Msg::to_string() const {
  std::stringstream ss;
  ss << NavAddr::BusToString(bus) << " " << key() << " " << talker << type
     << " " << ocpn::printable(payload);
  return ss.str();
}

std::string PluginMsg::to_string() const {
  return name + ": " + ocpn::rtrim(message);
}

/**
length (1):      0x13
 * priority (1);    0x02
 * PGN (3):         0x01 0xF8 0x01
 * destination(1):  0xFF
 * source (1):      0x01
 * time (4):        0x76 0xC2 0x52 0x00
 * len (1):         0x08
 * data (len):      08 70 EB 14 E8 8E 52 D2
 * packet CRC:      0xBB
 *
11FB1000 00 3E 78 6C 21 50 25 21  -> PGN 1FB10
 https://www.thehulltruth.com/marine-electronics-forum/
 1147866-reading-raw-nmea-2000-data.html

 Fake PGN 65392 65395
**/

std::string Nmea0183Msg::to_candump() const {
  std::stringstream ss;
  ss << "10FF7000 ";  // 1 -> misc, 0FF70 -> PGN 65392, 00-> misc
  for (auto c : payload) {
    ss << std::hex << static_cast<int>(c) / 16 << static_cast<int>(c) % 16;
    ss << " ";
  }
  return ss.str();
}
