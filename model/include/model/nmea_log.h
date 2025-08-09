/**************************************************************************
 *   Copyright (C) 2024 - 2025  Alec Leamas                                *
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

/**
 * \file
 *
 * Basic DataMonitor logging interface: LogLine (reflects a line in the log)
 * and NmeaLog, basic interface to add lines to the log.
 */

#ifndef ABSTRACT_NMEA_LOG_
#define ABSTRACT_NMEA_LOG_

#include "model/comm_navmsg.h"
#include "model/navmsg_filter.h"

/** Item in the log window. */
struct Logline {
  const std::shared_ptr<const NavMsg> navmsg;
  const NavmsgStatus state;
  const std::string message;
  std::string error_msg;
  std::string prefix;

  Logline() = default;

  Logline(const std::shared_ptr<const NavMsg>& navmsg, NavmsgStatus sts)
      : navmsg(navmsg),
        state(sts),
        message(navmsg ? navmsg->to_string() : ""),
        error_msg("Unknown error") {}

  explicit Logline(const std::shared_ptr<const NavMsg>& navmsg)
      : Logline(navmsg, [navmsg] {
          if (navmsg->state == NavMsg::State::kOk) return NavmsgStatus();

          NavmsgStatus navmsg_status;
          if (navmsg->state == NavMsg::State::kBadChecksum)
            navmsg_status.status = NavmsgStatus::State::kChecksumError;
          else if (navmsg->state == NavMsg::State::kCannotParse)
            navmsg_status.status = NavmsgStatus::State::kMalformed;
          else if (navmsg->state == NavMsg::State::kFiltered)
            navmsg_status.accepted = NavmsgStatus::Accepted::kFilteredDropped;
          return navmsg_status;
        }()) {}
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
  virtual ~NmeaLog() = default;

  /** Add a formatted string to log output. */
  virtual void Add(const Logline& l) = 0;

  /** Return true if log is visible i.e., if it's any point using Add(). */
  [[nodiscard]] virtual bool IsVisible() const = 0;
};

#endif  // ABSTRACT_NMEA_LOG_
