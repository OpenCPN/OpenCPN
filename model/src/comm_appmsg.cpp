/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_appmsg.h -- Decoded application messages.
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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <sstream>
#include <iomanip>

#include "comm_appmsg.h"
#include "ocpn_utils.h"

/* Free functions. */

std::string TimeToString(const time_t t) {
  char buff[30];
#ifdef _MSC_VER
  errno_t e = ctime_s(buff, sizeof(buff), &t);
  assert(e == 0 && "Huh? ctime_s returned an error");
  return std::string(buff);
#else
  const char* r = ctime_r(&t, buff);
  assert(r != NULL && "ctime_r failed...");
  return std::string(buff);
#endif
}

std::string DegreesToString(double degrees) {
  using namespace std;
  std::stringstream buf;
  buf << setw(2) << static_cast<int>(trunc(degrees)) << "\u00B0"
      << static_cast<int>(trunc(degrees * 100)) % 100 << "," << setw(2)
      << (static_cast<int>(trunc(degrees * 10000)) % 10000) % 100;
  return buf.str();
}

double PosPartsToDegrees(float degrees, float minutes,
                         float percent_of_minute) {
  return degrees + minutes / 60 + percent_of_minute / 6000;
}


/* Position implementation */

Position::Position(double _lat, double _lon, Type t)
    : lat(TypeToLat(t, _lat)), lon(TypeToLong(t, _lon)), type(t) {}

Position::Position(double _lat, double _lon)
    : lat(_lat), lon(_lon), type(LatLongToType(_lat, _lon)) {};

Position::Position() : lat(0), lon(0), type(Type::Undef) {};

std::string Position::to_string() const {
  std::stringstream buf;
  const std::string NE(TypeToStr(type));
  auto lat_s = DegreesToString(abs(lat));
  auto lon_s = DegreesToString(abs(lon));
  buf << lat_s << NE[0] << " " << lon_s << NE[1];
  return buf.str();
}

std::string  Position::TypeToStr(const Type t) const {
  switch (t) {
    case Type::NE:
      return "NE";
      break;
    case Type::NW:
      return "NW";
      break;
    case Type::SE:
      return "SE";
      break;
    case Type::SW:
      return "SW";
      break;
    case Type::Undef:
      return "Undefined";
      break;
   }
  return "??";     // Not reached, but compiler complains.
}

Position::Type Position::LatLongToType(double lat, double lon) {
  if (lat >= 0)
    return lon >= 0 ? Type::NE : Type::NW;
  else
    return lon >= 0 ? Type::SE : Type::SW;
}

double Position::TypeToLat(Type t, double lat) {
  return t == Type::SW || t == Type::SE ? -lat : lat;
}

double Position::TypeToLong(Type t, double lon) {
  return t == Type::NE || t == Type::SE ? lon : -lon;
}

/** Parse string like 5800.588 which is 58 degrees 00.588 minutes. */
static double GgaPartToDouble(const std::string& s) {
  size_t dotpos = s.find('.');
  if (dotpos < 2) return nan("");
  auto degrees = s.substr(0, dotpos - 2);
  auto minutes = s.substr(dotpos - 2);
  return std::stod(degrees) + std::stod(minutes)/60;

}

Position Position::ParseGGA(const std::string gga) {
  auto parts = ocpn::split(gga.c_str(), ",");
  if (parts.size() != 4) {
    return Position();
  }
  double lat = GgaPartToDouble(parts[0]);
  if (parts[1] == "S")
    lat = -lat;
  else if (parts[1] != "N")
    lat = nan("");
  double lon = GgaPartToDouble(parts[2]);
  if (parts[3] == "W")
    lon = -lon;
  else if (parts[3] != "E")
    lon = nan("");

  return lat != nan("") && lon != nan("") ? Position(lat, lon) : Position();
}


/* Appmsg implementation */

std::string AppMsg::TypeToString(const AppMsg::Type t) const {
  switch (t) {
    case AppMsg::Type::AisData:
      return "ais-data";
      break;
    case AppMsg::Type::BasicNavData:
      return "basic-nav-data";
      break;
    case AppMsg::Type::CustomMsg:
      return "custom-msg";
      break;
    case AppMsg::Type::DataPrioNeeded:
      return "data-prio-needed";
      break;
    case AppMsg::Type::GnssFix:
      return "gnss-fix";
      break;
    case AppMsg::Type::GPSWatchdog:
      return "gps-watchdog";
      break;
    case AppMsg::Type::Undef:
      return "??";
      break;
  }
  return "????";  // Not reached, for the compiler.
}
