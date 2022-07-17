/* API handling communication "drivers". */

#include <memory>
#include <vector>

#include <wx/event.h>
#include <wx/jsonreader.h>

#include "observable.h"
#include "datastream.h"

#ifndef _DRIVER_API_H
#define _DRIVER_API_H

enum class CommStatus {ok, not_implemented, not_supported, name_in_use};

enum class NavBus {nmea0183, signalK, n2k, onenet, test_if};

typedef uint64_t n2k_name_t;      // FIXME: Add a proper type.

/** Where messages are sent to or received from. */
typedef struct {
   NavBus bus;
   std::string interface;         /**< Physical device for 0183, else a
                                       unique string */
   union {
      const DataStream* nmea0183; /**< A specific RS485/nmea01831 interface  */
      struct in_addr sk_ipv4;     /**< signalK message over ipv4 */
      struct in6_addr onenet;     /**< FIXME Probably too simplified. */
      n2k_name_t  name;
   } address;
} nav_addr_t;

typedef struct {
  std::string id;       /**<  For example 'GPGGA'  */
  std::string payload;  /**< Remaining data after first ',' */
} Nmea0183_msg;


/**
 * See: https://github.com/OpenCPN/OpenCPN/issues/2729#issuecomment-1179506343
 */
typedef struct {
  std::vector<unsigned char> payload;
} Nmea2000_msg;

/** A parsed SignalK message over ipv4 */
typedef struct {
  struct in_addr dest;
  struct in_addr src;
  wxJSONValue* root;
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

typedef const std::shared_ptr<nav_msg> nav_msg_ptr;


class AbstractDriver;   // forward

/**
 * Interface implemented by transport layer and possible other parties
 * like test code which should handle incoming messages
 */
class DriverListener {
public:
  /** Handle a received message. */
  virtual void notify(const nav_msg& message) = 0;

  /** Handle driver status change. */
  virtual void notify(const AbstractDriver& driver) = 0;
};


/**
 * Common interface for all drivers.
 *
 * nmea2000 drivers are responsible for address claiming, exposing a stable
 * n2k_name. It also handles fast packages fragmentation/defragmentation.
 *
 * Handling of list of attached devices and their human readable attributes
 * are NOT part of the driver.
  */
class AbstractDriver {
public:
  const NavBus bus;
  const std::string interface;  /**< Physical device for 0183, else a
                                 *   unique string
                                 */
  virtual void send_message(const nav_msg& msg, const nav_addr_t& addr) = 0;

  virtual void set_listener(DriverListener listener) = 0;

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


/**
 * The global driver registry, a singleton. Drivers register here when
 * activated, transport layer finds them.
 */
class DriverRegistry {
public:
  void activate(const AbstractDriver& driver);
  void deactivate(const AbstractDriver& driver);

  /** Notified by all driverlist updates. */
  EventVar evt_driverlist_change;

  /** @return List of all activated drivers. */
  const std::vector<AbstractDriver>& get_drivers();

  static DriverRegistry* getInstance();
};

#endif  // DRIVER_API_H
