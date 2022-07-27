/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
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

#ifndef __COMMDRIVERBASE_H__
#define __COMMDRIVERBASE_H__

#include <memory>
#include <string>
#include <vector>

enum class CommStatus {ok, not_implemented, not_supported, name_in_use};

enum class NavBus {nmea0183, signalK, n2k, onenet, test_if, undefined};

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
  N2kName(uint64_t name);
  uint32_t get_number() const;           /**< 21 bits */
  uint16_t get_manufacturer() const;     /**< 9 bits */
  uint8_t get_dev_instance_low() const;  /**< 3 bits */
  uint8_t get_dev_instance_high() const; /**< 5 bits */
  uint8_t get_dev_func() const;          /**< 8 bits */
  uint8_t get_dev_class() const;         /**< 7 bits */
  uint8_t get_sys_instance() const;      /**< 4 bits */
  uint8_t get_industry_group() const;    /**< 4 bits */
};

/**
 * The n2k message id as defined by the J/1939 standard. See
 * https://www.kvaser.com/about-can/higher-layer-protocols/j1939-introduction/
 */
struct N2kId {
  N2kId() {};
  N2kId(uint64_t id);
  N2kId(std::vector<unsigned char> *raw_data){
    // build pgn
    uint32_t pgn = 0;
    unsigned char *t = (unsigned char *)&pgn;
    *t++ = raw_data->at(3);
    *t++ = raw_data->at(4);
    *t++ = raw_data->at(5);
    id = (pgn << 8) + raw_data->at(7);
    id += raw_data->at(2) << 26;
  };

  uint8_t get_prio() const;   /**< 3 bits */
  uint32_t get_png() const;   /**< a. k. a. PNG, 18 bits */
  uint32_t get_source() const;   /**< Source address,  8 bits */

  private:
    uint64_t id;

};

/** Where messages are sent to or received from. */
typedef struct {
   NavBus bus;
   std::string interface;         /**< Physical device for 0183, else a
                                       unique string */
    union {
//       const DataStream* nmea0183; /**< A specific RS485/nmea01831 interface  */
//       struct in_addr sk_ipv4;     /**< signalK message over ipv4 */
//       struct in6_addr onenet;     /**< FIXME Probably too simplified. */
       N2kName name;
    } address;
} nav_addr_t;

typedef struct {
  std::string id;       /**<  For example 'GPGGA'  */
  std::string payload;  /**< Remaining data after first ',' */
} Nmea0183_msg;

typedef struct {
  N2kId id;
  std::vector<unsigned char> payload;
} Nmea2000_msg;

/** A parsed SignalK message over ipv4 */
typedef struct {
//   struct in_addr dest;
//   struct in_addr src;
//   wxJSONValue* root;
  const int depth;
  std::vector<std::string> errors;
  std::vector<std::string> warnings;
} SignalK_ipv4_msg;


/** Actual data sent between application and transport layer */
typedef struct {
  NavBus bus;
  union {
    Nmea2000_msg nmea2000;
    Nmea0183_msg nmea0183;
    SignalK_ipv4_msg sk_ipv4;
  };
} nav_msg;

class AbstractCommDriver;   // forward

/**
 * Interface implemented by transport layer and possible other parties
 * like test code which should handle incoming messages
 */
class DriverListener {
public:
  /** Handle a received message. */
  virtual void notify(const nav_msg& message) = 0;

  /** Handle driver status change. */
  virtual void notify(const AbstractCommDriver& driver) = 0;
};

/** Common interface for all drivers.  */
class AbstractCommDriver {
public:
  AbstractCommDriver();
  virtual ~AbstractCommDriver();

  NavBus bus;
  const std::string interface;  /**< Physical device for 0183, else a
                                     unique string */

  virtual void send_message(const nav_msg& msg, const nav_addr_t& addr) = 0;

  //virtual void set_listener(DriverListener listener) = 0;

  /**
   * Create a new virtual interface using a new instance of this driver.
   * A successful return guarantees that the new driver is registered in
   * the device registry and activated.
   *
   * @return <CommStatus::ok, interface> on success else <error_code, message>.
   */
  virtual std::pair<CommStatus, std::string> clone() {
    // FIXME: Requires some unique interface support in DriverRegistry.
    return std::pair<CommStatus, std::string>(CommStatus::not_implemented, "");
  }
};

#endif    //guard
