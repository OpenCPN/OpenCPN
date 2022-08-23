/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  navmsg -- Raw, undecoded messages definitions.
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

#include <memory>
#include <sstream>
#include <vector>
#include <string>

#ifndef _MSC_VER
#include <netinet/in.h>
#endif

#include <wx/jsonreader.h>

#ifndef _DRIVER_NAVMSG_H
#define _DRIVER_NAVMSG_H

/**
 * N2k uses CAN which defines the basic properties of messages.
 * The NAME is an unique identifier for a node. CAN standardizes
 * an address claim protocol. The net effect is that upper layers
 * sees a stable NAME even if the address chnages.
 *
 * The structure of the NAME is defined in the J/1939 standard, see
 * https://www.kvaser.com/about-can/higher-layer-protocols/j1939-introduction/
 */
struct N2kName {
  N2kName(uint64_t name) : value(name) {}

  std::string to_string() const {
    std::stringstream ss;
    ss << value;
    return ss.str();
  }

  static uint64_t Parse(const std::string& s) {
    std::stringstream ss;
    uint64_t id;
    ss << s;
    ss >> id;
    return id;
  }

  uint64_t value;
  uint32_t GetNumber() const;         /**< 21 bits */
  uint16_t GetManufacturer() const;   /**< 9 bits */
  uint8_t GetDevInstanceLow() const;  /**< 3 bits */
  uint8_t GetDevInstanceHigh() const; /**< 5 bits */
  uint8_t GetDevFunc() const;         /**< 8 bits */
  uint8_t GetDevClass() const;        /**< 7 bits */
  uint8_t GetSysInstance() const;     /**< 4 bits */
  uint8_t GetIndustryGroup() const;   /**< 4 bits */
};

/** Where messages are sent to or received from. */
class NavAddr {
public:
  enum class Bus { N0183, Signalk, N2000, Onenet, TestBus, Undef };

  NavAddr(Bus b, const std::string& i) : bus(b), iface(i){};
  NavAddr() : bus(Bus::Undef), iface(""){};

  std::string to_string() const {
    return NavAddr::BusToString(bus) + " " + iface;
  }
  static std::string BusToString(Bus b);
  static Bus StringToBus(const std::string& s);

  Bus bus;
  const std::string iface; /**< Physical device for 0183, else a unique
                                string */
};

class NavAddr0183 : public NavAddr {
public:
  NavAddr0183(const std::string iface) : NavAddr(NavAddr::Bus::N0183, iface){};

  std::string to_string() const { return iface; }
};

class NavAddr2000 : public NavAddr {
public:
  NavAddr2000(const std::string& iface, const N2kName& _name)
      : NavAddr(NavAddr::Bus::N2000, iface), name(_name){};

  std::string to_string() const { return name.to_string(); }

  const N2kName name;
};

/** There is only support for a single signalK bus. */
class NavAddrSignalK : public NavAddr {
public:
  NavAddrSignalK() : NavAddr(NavAddr::Bus::Signalk, "signalK"){};
};

class NavAddrTest : public NavAddr {
public:
  NavAddrTest(std::string output_path)
      : NavAddr(NavAddr::Bus::TestBus, "Test"), name(output_path){};

  const std::string name;
};

/** Actual data sent between application and transport layer */
class NavMsg {
public:
  NavMsg() = delete;

  virtual std::string key() const = 0;

  virtual std::string to_string() const {
    return NavAddr::BusToString(bus) + " " + key();
  }

  const NavAddr::Bus bus;

  std::shared_ptr<const NavAddr> source;

protected:
  NavMsg(const NavAddr::Bus& _bus, std::shared_ptr<const NavAddr> src)
      : bus(_bus), source(src){};
};

/**
 * See: https://github.com/OpenCPN/OpenCPN/issues/2729#issuecomment-1179506343
 */
class Nmea2000Msg : public NavMsg {
public:
  Nmea2000Msg(const N2kName& n)
      : NavMsg(NavAddr::Bus::N2000, std::make_shared<NavAddr>()), name(n) {}
  Nmea2000Msg(const N2kName& n, std::shared_ptr<const NavAddr> src)
      : NavMsg(NavAddr::Bus::N2000, src), name(n) {}
  Nmea2000Msg(const N2kName& n, const std::vector<unsigned char>& _payload,
              std::shared_ptr<const NavAddr> src)
      : NavMsg(NavAddr::Bus::N2000, src), name(n), payload(_payload) {}

  std::string key() const { return std::string("n2000-") + name.to_string(); };

  /** Print "bus key id payload" */
  std::string to_string() const;

  N2kName name;
  std::vector<unsigned char> payload;
};

/** A regular Nmea0183 message. */
class Nmea0183Msg : public NavMsg {
public:
  Nmea0183Msg(const std::string& id, const std::string& _payload,
              std::shared_ptr<const NavAddr> src)
      : NavMsg(NavAddr::Bus::N0183, src),
        talker(id.substr(0, 2)),
        type(id.substr(2)),
        payload(_payload) {}

  Nmea0183Msg()
      : NavMsg(NavAddr::Bus::Undef, std::make_shared<const NavAddr>()) {}

  Nmea0183Msg(const std::string& id)
      : Nmea0183Msg(id.size() <= 3 ? std::string("??") + id : id, "",
                    std::make_shared<const NavAddr>()) {}

  Nmea0183Msg(const Nmea0183Msg& other, const std::string& t)
      : NavMsg(NavAddr::Bus::N0183, other.source),
        talker(other.talker),
        type(t),
        payload(other.payload) {}

  std::string key() const { return Nmea0183Msg::MessageKey(type.c_str()); };

  std::string to_string() const {
    return NavMsg::to_string() + " " + talker + type + " " + payload;
  }

  /** Return key which should be used to listen to given message type. */
  static std::string MessageKey(const char* type = "ALL") {
    static const char* const prefix = "n0183-";
    return std::string(prefix) + type;
  }

  const std::string talker;  /**< For example 'GP' */
  const std::string type;    /**< For example 'GGA' */
  const std::string payload; /**< Complete NMEA0183 sentence, also prefix */
};

/** A parsed SignalK message over ipv4 */
class SignalkMsg : public NavMsg {
public:
  SignalkMsg(int _depth)
      : NavMsg(NavAddr::Bus::Signalk, std::make_shared<const NavAddr>()),
        depth(_depth) {}

  struct in_addr dest;
  struct in_addr src;
  wxJSONValue* root;
  const int depth;
  std::vector<std::string> errors;
  std::vector<std::string> warnings;
  std::string key() const { return std::string("signalK"); };
};

/** An invalid message, error return value. */
class NullNavMsg : public NavMsg {
public:
  NullNavMsg()
      : NavMsg(NavAddr::Bus::Undef, std::make_shared<const NavAddr>()) {}

  std::string key() const { return "navmsg-undef"; }
};

#endif  // DRIVER_NAVMSG_H
