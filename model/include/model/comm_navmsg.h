/**************************************************************************
 *   Copyright (C) 2022 - 2024 by David Register, Alec Leamas              *
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

/** \file navmsg.h  Raw, undecoded messages definitions. */

#ifndef _DRIVER_NAVMSG_H
#define _DRIVER_NAVMSG_H

#include <memory>
#include <sstream>
#include <vector>
#include <string>

#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "observable.h"

struct N2kPGN {
  uint64_t pgn;

  N2kPGN(uint64_t _pgn) { pgn = _pgn; }

  std::string to_string() const {
    std::stringstream ss;
    ss << pgn;
    return ss.str();
  }
};

/**
 * N2k uses CAN which defines the basic properties of messages.
 * The NAME is an unique identifier for a node. CAN standardizes
 * an address claim protocol. The net effect is that upper layers
 * sees a stable NAME even if the address changes.
 *
 * The structure of NAME is defined in the J/1939 standard, see
 * https://www.kvaser.com/about-can/higher-layer-protocols/j1939-introduction/
 */
struct N2kName {
  N2kName() {};
  N2kName(uint64_t name) { value.Name = name; }

  std::string to_string() const {
    std::stringstream ss;
    ss << value.Name;
    return ss.str();
  }

  static uint64_t Parse(const std::string& s) {
    std::stringstream ss;
    uint64_t id;
    ss << s;
    ss >> id;
    return id;
  }

  uint32_t GetNumber() const;         /**< 21 bits */
  uint16_t GetManufacturer() const;   /**< 9 bits */
  uint8_t GetDevInstanceLow() const;  /**< 3 bits */
  uint8_t GetDevInstanceHigh() const; /**< 5 bits */
  uint8_t GetDevFunc() const;         /**< 8 bits */
  uint8_t GetDevClass() const;        /**< 7 bits */
  uint8_t GetSysInstance() const;     /**< 4 bits */
  uint8_t GetIndustryGroup() const;   /**< 4 bits */

  typedef union {
    uint64_t Name;
    struct {
      uint32_t UnicNumberAndManCode;  // ManufacturerCode 11 bits , UniqueNumber
                                      // 21 bits
      unsigned char DeviceInstance;
      unsigned char DeviceFunction;
      unsigned char DeviceClass;
      unsigned char IndustryGroupAndSystemInstance;  // 4 bits each
    };
  } tUnionDeviceInformation;

  tUnionDeviceInformation value;

  void SetUniqueNumber(uint32_t _UniqueNumber) {
    value.UnicNumberAndManCode =
        (value.UnicNumberAndManCode & 0xffe00000) | (_UniqueNumber & 0x1fffff);
  }
  void SetManufacturerCode(uint16_t _ManufacturerCode) {
    value.UnicNumberAndManCode =
        (value.UnicNumberAndManCode & 0x1fffff) |
        (((unsigned long)(_ManufacturerCode & 0x7ff)) << 21);
  }
  void SetDeviceInstance(unsigned char _DeviceInstance) {
    value.DeviceInstance = _DeviceInstance;
  }
  void SetDeviceFunction(unsigned char _DeviceFunction) {
    value.DeviceFunction = _DeviceFunction;
  }
  void SetDeviceClass(unsigned char _DeviceClass) {
    value.DeviceClass = ((_DeviceClass & 0x7f) << 1);
  }
  void SetIndustryGroup(unsigned char _IndustryGroup) {
    value.IndustryGroupAndSystemInstance =
        (value.IndustryGroupAndSystemInstance & 0x0f) | (_IndustryGroup << 4) |
        0x80;
  }
  void SetSystemInstance(unsigned char _SystemInstance) {
    value.IndustryGroupAndSystemInstance =
        (value.IndustryGroupAndSystemInstance & 0xf0) |
        (_SystemInstance & 0x0f);
  }

  uint64_t GetName() const { return value.Name; }
};

/** Where messages are sent to or received from. */
class NavAddr {
public:
  enum class Bus { N0183, Signalk, N2000, Onenet, Plugin, TestBus, Undef };

  NavAddr(Bus b, const std::string& i) : bus(b), iface(i) {};
  NavAddr() : bus(Bus::Undef), iface("") {};

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
  NavAddr0183(const std::string iface) : NavAddr(NavAddr::Bus::N0183, iface) {};

  std::string to_string() const { return iface; }
};

class NavAddr2000 : public NavAddr {
public:
  NavAddr2000(const std::string& iface, const N2kName& _name)
      : NavAddr(NavAddr::Bus::N2000, iface), name(_name) {};

  NavAddr2000(const std::string& iface, unsigned char _address)
      : NavAddr(NavAddr::Bus::N2000, iface), name(0), address(_address) {};

  std::string to_string() const { return name.to_string(); }

  const N2kName name;
  unsigned char address;
};

class NavAddrPlugin : public NavAddr {
public:
  const std::string id;
  NavAddrPlugin(const std::string& _id)
      : NavAddr(NavAddr::Bus::Plugin, "Plugin"), id(_id) {}
};

class NavAddrSignalK : public NavAddr {
public:
  NavAddrSignalK(std::string iface) : NavAddr(NavAddr::Bus::Signalk, iface) {};

  std::string to_string() const { return NavAddr::to_string(); }
};

class NavAddrTest : public NavAddr {
public:
  NavAddrTest(std::string output_path)
      : NavAddr(NavAddr::Bus::TestBus, "Test"), name(output_path) {};

  const std::string name;
};

/** Actual data sent between application and transport layer */
class NavMsg : public KeyProvider {
public:
  NavMsg() = delete;

  virtual std::string key() const = 0;

  virtual std::string to_string() const {
    return NavAddr::BusToString(bus) + " " + key();
  }

  std::string GetKey() const { return key(); }

  const NavAddr::Bus bus;

  std::shared_ptr<const NavAddr> source;

protected:
  NavMsg(const NavAddr::Bus& _bus, std::shared_ptr<const NavAddr> src)
      : bus(_bus), source(src) {};
};

/**
 * See: https://github.com/OpenCPN/OpenCPN/issues/2729#issuecomment-1179506343
 */
class Nmea2000Msg : public NavMsg {
public:
  Nmea2000Msg(const uint64_t _pgn)
      : NavMsg(NavAddr::Bus::N2000, std::make_shared<NavAddr>()), PGN(_pgn) {}

  Nmea2000Msg(const uint64_t _pgn, std::shared_ptr<const NavAddr> src)
      : NavMsg(NavAddr::Bus::N2000, src), PGN(_pgn) {}

  Nmea2000Msg(const uint64_t _pgn, const std::vector<unsigned char>& _payload,
              std::shared_ptr<const NavAddr> src)
      : NavMsg(NavAddr::Bus::N2000, src), PGN(_pgn), payload(_payload) {}

  Nmea2000Msg(const uint64_t _pgn, const std::vector<unsigned char>& _payload,
              std::shared_ptr<const NavAddr> src, int _priority)
      : NavMsg(NavAddr::Bus::N2000, src),
        PGN(_pgn),
        payload(_payload),
        priority(_priority) {}

  virtual ~Nmea2000Msg() = default;

  std::string key() const { return std::string("n2000-") + PGN.to_string(); };

  /** Print "bus key id payload" */
  std::string to_string() const;

  N2kPGN PGN;  // For TX message, unparsed
  std::vector<unsigned char> payload;
  int priority;
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

  virtual ~Nmea0183Msg() = default;

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

/** A plugin to plugin json message over the REST interface */
class PluginMsg : public NavMsg {
public:
  PluginMsg()
      : NavMsg(NavAddr::Bus::Undef, std::make_shared<const NavAddr>()) {}

  PluginMsg(const std::string& _name, const std::string& _dest_host,
            const std::string& msg)
      : NavMsg(NavAddr::Bus::Plugin,
               std::make_shared<const NavAddr>(NavAddr::Bus::Plugin, "")),
        name(_name),
        message(msg),
        dest_host(_dest_host) {}

  PluginMsg(const std::string& _name, const std::string& msg)
      : PluginMsg(_name, "localhost", msg) {}

  virtual ~PluginMsg() = default;

  const std::string name;
  const std::string message;
  const std::string dest_host;  ///< hostname, ip address or 'localhost'

  std::string key() const { return std::string("plug.json-") + name; };
};

/** A parsed SignalK message over ipv4 */
class SignalkMsg : public NavMsg {
public:
  SignalkMsg()
      : NavMsg(NavAddr::Bus::Undef, std::make_shared<const NavAddr>()) {}

  SignalkMsg(std::string _context_self, std::string _context,
             std::string _raw_message, std::string _iface)
      : NavMsg(NavAddr::Bus::Signalk,
               std::make_shared<const NavAddr>(NavAddr::Bus::Signalk, _iface)),
        context_self(_context_self),
        context(_context),
        raw_message(_raw_message) {};

  virtual ~SignalkMsg() = default;

  struct in_addr dest;
  struct in_addr src;
  std::string context_self;
  std::string context;
  std::string raw_message;

  std::string key() const { return std::string("signalK"); };
};

/** An invalid message, error return value. */
class NullNavMsg : public NavMsg {
public:
  NullNavMsg()
      : NavMsg(NavAddr::Bus::Undef, std::make_shared<const NavAddr>()) {}

  virtual ~NullNavMsg() = default;

  std::string key() const { return "navmsg-undef"; }
};

#endif  // DRIVER_NAVMSG_H
