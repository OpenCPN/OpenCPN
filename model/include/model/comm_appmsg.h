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

#include "model/comm_driver.h"
#include "observable.h"

double PosPartsToDegrees(float degrees, float minutes, float percent_of_minute);

std::string DegreesToString(double degrees);

std::string TimeToString(const time_t time);

class Position {
public:
  enum class Type { NE, NW, SE, SW, Undef };

  /** Construct a position from positive lat/long values and Position::Type */
  Position(double _lat, double _lon, Type t);

  /** Construct a position with signed lat/long defining Type.*/
  Position(double _lat, double _lon);

  /** Construct a (0,0) position, type == Undef. */
  Position();


  bool IsValid() const { return type != Type::Undef;  }

  /** Return utf string like 65°25,11N 21°12,01E */
  std::string to_string() const;

  const double lat;      // signed value
  const double lon;      // signed value
  const Type type;

  /**
   * Parse a GGA string like "5800.588,N,01145.776,E" as present in
   * GGA and  other n0183 messages.
   * @return Position where IsValid() is false on errors, else true.
   */
  static Position ParseGGA(const std::string gga);

private:
  std::string TypeToStr(const Type t) const;

  /** Deduce type from signed lat/lon. */
  Type LatLongToType(double lat, double lon);

  /** Returned signed latitude deduced from t. */
  double TypeToLat(Type t, double lat);

  /** Returned signed long  deduced from t. */
  double TypeToLong(Type t, double lon);

};


class AppMsg : public KeyProvider {
public:
  enum class Type;
  AppMsg(AppMsg::Type t)
      : type(t), name(TypeToString(t)), source(NavAddr()), prio(0){};

  virtual std::string key() const { return std::string("@!appmsg-") + name; }

  std::string GetKey() const { return key(); }

  std::string TypeToString(const Type t) const;

  const Type type;
  const std::string name;  // Must be unique, probably using TypeToString().
  NavAddr source;
  unsigned short prio;  // Initially 0, modified using set_priority

protected:
  AppMsg(AppMsg::Type tp, const std::string& nm, NavAddr src)
      : type(tp), name(nm), source(src), prio(0){};
};

enum class AppMsg::Type {
  BasicNavData,
  GPSWatchdog,
  GnssFix,
  AisData,
  DataPrioNeeded,
  CustomMsg,
  Undef
};

/**
 * Issued when there are multiple sources providing 'what' with priority == 0.
 * Should result in GUI actions eventually calling set_priority()
 */
class DataPrioNeeded : public AppMsg {
public:
  AppMsg::Type what;
  std::vector<NavAddr> sources;
};

/** GPS, Galileo, etc. position data point. */
class GnssFix : public AppMsg {
public:
  enum class Quality { none, gnss, differential };

  GnssFix(Position p, time_t t, Quality q = Quality::none, int s_used = -1)
      : AppMsg(AppMsg::Type::GnssFix, "gnss-fix", NavAddr()),
        pos(p),
        time(t),
        quality(q),
        satellites_used(s_used){};
  virtual ~GnssFix() = default;

  std::string to_string() const {
    std::stringstream buf;
    buf << pos.to_string() << " " << TimeToString(time);
    return buf.str();
  }

  Position pos;
  const time_t time;
  Quality quality;
  int satellites_used;
};

// bitmask defining update validity of BasicNavDataMsg members
#define POS_UPDATE     (int)(1)
#define COG_UPDATE     (int)(1 << 1)
#define SOG_UPDATE     (int)(1 << 2)
#define VAR_UPDATE     (int)(1 << 3)
#define HDT_UPDATE     (int)(1 << 4)
#define POS_VALID      (int)(1 << 5)

class BasicNavDataMsg : public AppMsg {
public:
  BasicNavDataMsg(double lat, double lon, double SOG, double COG, double VAR,
                  double HDT, int valid_flag, time_t t)
      : AppMsg(AppMsg::Type::BasicNavData, "basic-nav-data", NavAddr()),
        pos(lat, lon),
        sog(SOG),
        cog(COG),
        var(VAR),
        hdt(HDT),
        vflag(valid_flag),
        time(t){};

  BasicNavDataMsg()
      : AppMsg(AppMsg::Type::BasicNavData, "basic-nav-data", NavAddr()),
        sog(0),
        cog(0),
        var(0),
        hdt(0),
        vflag(0),
        time(0){};

  virtual ~BasicNavDataMsg() = default;

  const Position pos;
  const double sog;
  const double cog;
  const double var;
  const double hdt;
  const int vflag;
  const time_t time;
};

class GPSWatchdogMsg : public AppMsg {
public:
  enum class WDSource { position, velocity, heading, var, sats };

  GPSWatchdogMsg(WDSource _source, int value)
      : AppMsg(AppMsg::Type::GPSWatchdog, "gps-watchdog", NavAddr()),
        gps_watchdog(value),
        wd_source(_source){};

  virtual ~GPSWatchdogMsg() = default;

  const int gps_watchdog;
  const WDSource wd_source;
};

/** AIS data point for a vessel. */
class AisData : public AppMsg {
public:
  time_t time;
  Position pos;
  float sog;           // Speed over ground, knots.
  float cog;           // Course over ground, 0..360 degrees.
  float heading;       // Magnetic sensor, 0..360 degrees.
  float rate_of_turn;  // Degrees per minute, "-" means bow turns to port.
  uint8_t type;        // https://api.vtexplorer.com/docs/ref-aistypes.html
  std::string name;
  std::string callsign;
  std::string dest;  // Destination port
  int length;
  int beam;
  int draft;
  uint8_t status;  // https://api.vtexplorer.com/docs/ref-navstat.html
};

/**
 * A generic message containing a const pointer to basically anything, the
 * pointer neds to be casted to the proper type on the receiving side.
 */
class CustomMsg : public AppMsg {
  CustomMsg(const std::string s, std::shared_ptr<const void> ptr)
      : AppMsg(Type::CustomMsg, "custom", NavAddr()), id(s), payload(ptr) {}

  std::string key() const override {
    return std::string("@##_appmsg-custom-") + id;
  }

  const std::string id;  // Must be unique.
  std::shared_ptr<const void> payload;
};

#endif  // APP_MSG_H
