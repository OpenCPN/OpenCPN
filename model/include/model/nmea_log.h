#ifndef _ABSTRACT_NMEA_LOG__
#define _ABSTRACT_NMEA_LOG__

#include <wx/string.h>

#include "model/comm_navmsg.h"
#include "model/navmsg_filter.h"

/** Item in the log window. */
struct Logline {
  const std::shared_ptr<const NavMsg> navmsg;
  const NavmsgStatus state;
  const std::string message;
  std::string error_msg;
  std::string prefix;

  Logline() {}
  Logline(const std::shared_ptr<const NavMsg>& navmsg, NavmsgStatus sts)
      : navmsg(navmsg),
        state(sts),
        message(navmsg ? navmsg->to_string() : ""),
        error_msg("Unknown error") {}
  Logline(const std::shared_ptr<const NavMsg>& navmsg)
      : Logline(navmsg, NavmsgStatus()) {}
};

/**
 * NMEA Log Interface
 *
 * Provides interface for interacting with NMEA log.
 *
 * @interface NmeaLog nmea_log.h "model/nmea_log.h"
 *
 */
class NmeaLog {
public:
  /**
   * Destroy the NMEA Log object.
   */
  virtual ~NmeaLog() = default;

  /** Add an formatted string to log output. */
  virtual void Add(const Logline& l) = 0;

  /** Return true if log is visible i. e., if it's any point using Add(). */
  virtual bool IsVisible() const = 0;
};

#endif  // _ABSTRACT_NMEA_LOG__
