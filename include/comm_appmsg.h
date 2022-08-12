/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Decoded messages definitions. These messages are handled by the
 *           ApgMsgBus defined in comm_appmsg_bus.h.
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

#ifndef _APP_MSG_H
#define _APP_MSG_H

#include <memory>
#include <iomanip>

#include <wx/event.h>

#include "comm_driver.h"


double PosPartsToDegrees(float degrees, float minutes, float percent_of_minute);

std::string DegreesToString(double degrees);

std::string TimeToString(const time_t time);

class Position{
public:
  enum class Type {NE, NW, SE, SW, Undef};

  const Type type;
  double lat;
  double lon;

  std::string to_string() const {
    std::stringstream buf;
    const std::string NE(TypeToStr(type));
    auto  lat_s = DegreesToString(lat);
    auto  lon_s = DegreesToString(lon);
    buf << lat_s << NE[0] << " " << lon_s << NE[1];
    return buf.str();
  }

  Position(double _lat, double _lon, Type t = Type::Undef)
    : type(t), lat(_lat), lon(_lon) {}
  Position() : type(Type::Undef), lat(0), lon(0) {};

  std::string TypeToStr(const Type t) const {
    switch (t) {
      case Type::NE:  return "NE"; break;
      case Type::NW:  return "NW"; break;
      case Type::SE:  return "SE"; break;
      case Type::SW:  return "SW"; break;
      default: return "??"; break;
    }
  }
};


class AppMsg {
public:
  enum class Type;
  const Type type;
  const std::string name;  // Must be unique, probably using TypeToString().
  NavAddr source;
  unsigned short prio;     // Initially 0, modified using set_priority

  virtual std::string key() const { return std::string("@!appmsg-") + name; }

  std::string TypeToString(const Type t) const;

  AppMsg(AppMsg::Type t)
    : type(t), name(TypeToString(t)), source(NavAddr()), prio(0) {};

protected:
  AppMsg(AppMsg::Type tp, const std::string& nm, NavAddr src)
    : type(tp), name(nm), source(src), prio(0) {};

  AppMsg& operator=(const AppMsg&) = default;
};

enum class AppMsg::Type {BasicNavData, GPSWatchdog, GnssFix, AisData, DataPrioNeeded, CustomMsg, Undef};


/**
 * Issued when there are multiple sources providing 'what' with priority == 0.
 * Should result in GUI actions eventually calling set_priority()
 */
class DataPrioNeeded: public AppMsg {
public:
  AppMsg::Type what;
  std::vector<NavAddr> sources;
};

/** GPS, Galileo, etc. position data point. */
class GnssFix: public AppMsg {
public:
  enum class Quality {none, gnss, differential };
  Position pos;
  const time_t time;
  Quality quality;
  int satellites_used;
  GnssFix(Position p, time_t t, Quality q = Quality::none, int s_used = -1)
    : AppMsg(AppMsg::Type::GnssFix, "gnss-fix", NavAddr()),
    pos(p), time(t), quality(q), satellites_used(s_used) {};

  std::string to_string() const {
    std::stringstream buf;
    buf << pos.to_string() << " " << TimeToString(time);
    return  buf.str();
  }
};

class BasicNavDataMsg: public AppMsg {
public:
  Position pos;
  double sog;
  double cog;
  double var;
  double hdt;
  time_t time;

  BasicNavDataMsg(double lat, double lon, double SOG, double COG, double VAR,
               double HDT, time_t t )
    : AppMsg(AppMsg::Type::BasicNavData, "basic-nav-data", NavAddr()),
    pos(lat, lon), sog(SOG), cog(COG), var(VAR), hdt(HDT), time(t){};

};

class GPSWatchdogMsg: public AppMsg {
public:
  int gps_watchdog;

  GPSWatchdogMsg(int value)
  : AppMsg(AppMsg::Type::GPSWatchdog, "gps-watchdog", NavAddr()),
  gps_watchdog(value) {};
};

/** AIS data point for a vessel. */
class AisData: public AppMsg {
public:
  time_t time;
  Position pos;
  float sog;             // Speed over ground, knots.
  float cog;             // Course over ground, 0..360 degrees.
  float heading;         // Magnetic sensor, 0..360 degrees.
  float rate_of_turn;    // Degrees per minute, "-" means bow turns to port.
  uint8_t type;          // https://api.vtexplorer.com/docs/ref-aistypes.html
  std::string name;
  std::string callsign;
  std::string dest;      // Destination port
  int length;
  int beam;
  int draft;
  uint8_t status;        // https://api.vtexplorer.com/docs/ref-navstat.html
};


/**
 * A generic message containing a const pointer to basically anything, the
 * pointer neds to be casted to the proper type on the receiving side.
 */
class CustomMsg: public AppMsg {
  const std::string id;   // Must be unique.
  std::shared_ptr<const void> payload;

  std::string key() const override {
    return std::string("@##_appmsg-custom-") + id;
  }
  CustomMsg(const std::string s, std::shared_ptr<const void> ptr)
    : AppMsg(Type::CustomMsg, "custom", NavAddr()), id(s), payload(ptr) {}
};

#endif  // APP_MSG_H
