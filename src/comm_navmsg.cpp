/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implements comm_navmsg -- raw, undecoded messages.
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#include <algorithm>
#include <string>
#include <iomanip>

#include "comm_driver.h"

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
  if (s == "TestBus") return NavAddr::Bus::TestBus;
  return NavAddr::Bus::Undef;
}

static std::string CharToString(unsigned char c) {
  using namespace std;
  stringstream ss;
  ss << setfill('0') << hex << setw(2) << (c & 0x00ff);
  return ss.str();
}

const std::string Nmea0183Msg::key_prefix = "n0183-";

std::string Nmea2000Msg::to_string() const {
  std::string s;
  std::for_each(payload.begin(), payload.end(),
                [&s](unsigned char c) { s.append(CharToString(c)); });
  return NavMsg::to_string() + " " + name.to_string() + " " + s;
}
