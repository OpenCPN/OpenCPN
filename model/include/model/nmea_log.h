#ifndef _ABSTRACT_NMEA_LOG__
#define _ABSTRACT_NMEA_LOG__

#include <wx/string.h>

#include "model/comm_navmsg.h"
#include "model/navmsg_filter.h"

/** Item in the log window. */
struct Logline {
  const std::shared_ptr<const NavMsg> navmsg;
  const NavmsgStatus state;
  const std::string stream_name;
  std::string error_msg;
  std::string prefix;

  Logline() {}
  Logline(const std::shared_ptr<const NavMsg>& msg, NavmsgStatus sts,
          const std::string& stream)
      : navmsg(msg),
        state(sts),
        stream_name(stream),
        error_msg("Unknown error") {}
  Logline(const std::shared_ptr<const NavMsg>& msg) : navmsg(msg), state() {}
};

class NmeaLog {
public:
  /** Add an formatted string to log output. */
  virtual void Add(const Logline& l) = 0;

  /** Return true if log is visible i. e., if it's any point using Add(). */
  virtual bool IsActive() const = 0;
};

#endif  // _ABSTRACT_NMEA_LOG__
