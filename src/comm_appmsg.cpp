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

#include <sstream>
#include <iomanip>

#include "comm_appmsg.h"

double PosPartsToDegrees(float degrees, float minutes, float percent_of_minute) {
  return degrees + minutes / 60 + percent_of_minute / 6000;
}

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
  buf << setw(2) <<  static_cast<int>(trunc(degrees)) << "\u00B0"
      << static_cast<int>(trunc(degrees * 100)) % 100 << ","
      << setw(2) << (static_cast<int>(trunc(degrees * 10000))% 10000) % 100;
  return buf.str();
}


std::string AppMsg::TypeToString(const AppMsg::Type t) const {
  switch (t) {
    case AppMsg::Type::GnssFix: return "gnss-fix"; break;
    case AppMsg::Type::AisData: return "ais-data"; break;
    case AppMsg::Type::DataPrioNeeded: return "data-prio-needed"; break;
    case AppMsg::Type::Undef: return "??"; break;
    default: return "????";
  }
}
